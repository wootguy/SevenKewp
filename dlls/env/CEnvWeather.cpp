#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "CBaseButton.h"

#define RAIN_MODEL_SMALL "models/weather/rain0_128.bsp"
#define RAIN_MODEL_BIG "models/weather/rain1_128.bsp"
#define RAIN_MODEL_SMALL_PUDDLES "models/weather/rain2_128.bsp"
#define RAIN_MODEL_BIG_PUDDLES "models/weather/rain3_128.bsp"
#define RAIN_OUT_SOUND "weather/rain.wav"
#define RAIN_GLASS_SOUND "weather/rain_glass.wav"
#define RAIN_SPLASH_SPR "sprites/rain_splash.spr"

#define MAX_WORLD_DIM 65536
#define RAIN_TEST_DIST 1024
#define WEATHER_SIZE 128 // width of a weather conveyor
#define WEATHER_HEIGHT 960 // height of a weather conveyor
#define WEATHER_VOL_HIST_SZ 25

struct weather_ent_t {
	EHANDLE h_ent;
	Vector pos;
	const char* model;
	int size;
	bool isFloating;
	bool isUnevenGround;
	uint32_t visPlayers; // like a PVS check but for distance only
};

struct weather_spot_t {
	Vector pos;
	const char* model;
	int size;
	bool valid;
	bool isFloating; // suspended in the air? (for tall high sky ceilings)
	bool isUnevenGround;
};

struct temp_nonsolid_ent_t {
	edict_t* ent;
	int oldSolid;
};

struct weather_sound_t {
	// per-player loudness history for smoothing loudness transitions and randomness
	float loudHist[32][WEATHER_VOL_HIST_SZ];

	// last volume networked to each player
	int lastVol[32];

	const char* file;
	int channel;

	int GetAverageLoudness(int playerindex);
};

// only one active weather type allowed in the map
std::vector<weather_ent_t> g_weatherEnts;
bool g_weather_init_done;

class CEnvWeather : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	void	Spawn(void);
	void	Precache(void);
	void	EXPORT WeatherThink(void);
	void	KeyValue(KeyValueData* pkvd);

	bool GetRainPosition(Vector pos, Vector& bottom, Vector& top, bool largeHull, bool debug);
	bool IsSky(TraceResult& tr, Vector traceStart);
	bool TraceToSky(Vector pos, Vector& skyPos); // trace upwards until hitting the sky
	bool CanSeePosition(CBasePlayer* plr, Vector pos);
	void WeatherEntsThink();
	int UpdateRainVisibility(CBasePlayer* plr); // returns number of nearby rain conveyors

	// find a neighbor suitable for merging
	int FindNeighborSpot(std::vector<weather_spot_t>& spots, Vector pos, weather_spot_t& self);

	// returns number of splashes rendered and sets loudness of rain effects
	int WaterSplashes(CBasePlayer* plr);

	void PlayWeatherSounds(CBasePlayer* plr);

	// scans the BSP for floor positions with a sky above it
	std::vector<weather_spot_t> FindWeatherSpots();

	// merges spots into larger areas to reduce entity count
	void MergeWeatherSpots(std::vector<weather_spot_t>& spots);

	bool IsUnevenGround(Vector pos, float radius);

	weather_sound_t m_rainSnd_out; // outdoor rain sound
	weather_sound_t m_rainSnd_glass; // rain hitting window sound

	int m_historyIdx;

	int m_rainSplashSpr;
};

LINK_ENTITY_TO_CLASS(env_weather, CEnvWeather)
LINK_ENTITY_TO_CLASS(env_rain, CEnvWeather)

int weather_sound_t::GetAverageLoudness(int playerindex) {
	float avg = 0;

	for (int k = 0; k < WEATHER_VOL_HIST_SZ; k++) {
		avg += loudHist[playerindex][k];
	}
	avg /= (float)WEATHER_VOL_HIST_SZ;

	// round to nearest multiple of 5 to reduce network message traffic
	int snapped = avg*100;
	snapped = (snapped / 5) * 5;

	return snapped;
}

bool CEnvWeather::IsSky(TraceResult& tr, Vector traceStart) {
	if (POINT_CONTENTS(tr.vecEndPos) == CONTENTS_SKY) {
		return true;
	}

	// don't use the original end position because this trace ignores glass always(?)
	const char* texName = TRACE_TEXTURE(ENT(0), traceStart, tr.vecEndPos + Vector(0,0,1));

	return texName && !strcmp(texName, "sky");
}

bool CEnvWeather::GetRainPosition(Vector pos, Vector& bottom, Vector& top, bool largeHull, bool debug) {
	Vector worldPosition = pos;

	TraceResult tr;

	Vector start = pos;
	Vector end = pos - Vector(0, 0, RAIN_TEST_DIST);

	if (largeHull) {
		float r = (WEATHER_SIZE * 0.5f);
		Vector altPos[5] = {
			pos + Vector(0, 0, 0),
			pos + Vector(-r, 0, 0),
			pos + Vector(r, 0, 0),
			pos + Vector(0, -r, 0),
			pos + Vector(0, r, 0),
		};

		bool foundBest = false;
		float bestZ = -MAX_WORLD_DIM;
		TraceResult bestTrace;

		for (int i = 0; i < 5; i++) {
			TRACE_LINE(altPos[i], altPos[i] - Vector(0,0,RAIN_TEST_DIST), ignore_monsters, NULL, &tr);

			if (tr.flFraction >= 1.0f) {
				continue;
			}

			if (tr.vecEndPos.z > bestZ) {
				bestZ = tr.vecEndPos.z;
				foundBest = true;
				bestTrace = tr;
			}
		}

		if (!foundBest) {
			return false;
		}

		// realign to center
		tr = bestTrace;
		tr.vecEndPos.x = pos.x;
		tr.vecEndPos.y = pos.y;
	}
	else {
		TRACE_LINE(start, end, ignore_monsters, NULL, &tr);
	}

	if (debug)
		te_debug_beam(start, tr.vecEndPos, 1, RGBA(0, 0, 255));

	if (tr.fAllSolid || (tr.fInOpen && tr.flFraction >= 1.0f)) {
		return false;
	}

	static Vector testPos = Vector(892, 2572, 0);
	Vector epos = tr.vecEndPos;
	epos.z = 0;
	if ((testPos - epos).Length() < 16 && (int)pos.z == 1920) {
		ALERT(at_console, "zomg test it %f\n", pos.z);
	}

	bottom = tr.vecEndPos;

	// don't rain underwater
	if (UTIL_PointInLiquid(bottom)) {
		bottom.z = UTIL_WaterLevel(bottom, bottom.z, bottom.z + 4096);
	}

	top = bottom;

	// check for sky above this floor position
	end = bottom;
	do {
		start = end;
		end = start + Vector(0, 0, RAIN_TEST_DIST);

		TRACE_LINE(start, end, ignore_monsters, NULL, &tr);

		if (tr.flFraction < 1.0f || POINT_CONTENTS(tr.vecEndPos) == CONTENTS_SKY) {
			if (IsSky(tr, start)) {
				top = tr.vecEndPos;
				break;
			}
			else {
				if (debug)
					te_debug_beam(bottom, tr.vecEndPos, 1, RGBA(255, 0, 0));
				return false;
			}
		}

		if (tr.fAllSolid || start.z > MAX_WORLD_DIM) {
			return false;
		}
	}
	while (true);

	// found a floor location with sky above it

	// pull out of the sky brush if floor is sky
	while (POINT_CONTENTS(bottom) == CONTENTS_SKY) {
		bottom.z += 8;
		if (bottom.z >= top.z) {
			return false; // oops all sky
		}
	}

	// check that the full width of the rain sprite can go here
	float radius = WEATHER_SIZE * 0.5f;
	
	for (int i = 0; i < 2; i++) {
		for (int r = -radius; r < radius; r += 8) {

			start = bottom;
			end = top;
			end.z = bottom.z + 8192;

			if (i == 0) {
				end.x += r;
				start.x += r;
			}
			else {
				end.y += r;
				start.y += r;
			}

			// pull out of the sky brush if on a sloped sky brush
			while (POINT_CONTENTS(start) == CONTENTS_SKY) {
				start.z += 8;
				if (start.z >= end.z) {
					break;
				}
			}
			if (start.z >= end.z) {
				continue;
			}

			TRACE_LINE(start, end, ignore_monsters, NULL, &tr);

			if (tr.fAllSolid || IsSky(tr, start)) {
				continue;
			}

			if (tr.fStartSolid) {
				// try to find which part of the trace wasn't solid, there should be a floor
				// that was busted out of in that case. The railings in ghost_buster.bsp will
				// not set fAllSolid yet appear to be all solid when you do this trace
				// (func_wall resting on world brush)
				TRACE_LINE(tr.vecEndPos, start, ignore_monsters, NULL, &tr);
				if (tr.flFraction >= 1.0f) {
					continue;
				}
			}

			// one more try. Sometimes the trace messes up when it's perfectly aligned
			// with a cliff or something (commap4.bsp garg area)
			Vector nudge = Vector(0.5f, 0.5f, 0);
			TRACE_LINE(start + nudge, end + nudge, ignore_monsters, NULL, &tr);
			epos = tr.vecEndPos;

			if (tr.fAllSolid || IsSky(tr, start)) {
				continue;
			}

			if (debug) {
				te_debug_beam(top, start, 1, RGBA(255, 0, 0));
				te_debug_beam(start, epos, 1, RGBA(255, 0, 0));
				
				// in case beam is in all solid region
				//te_debug_beam(start + Vector(0,16,0), epos + Vector(0, 16, 0), 1, RGBA(255, 0, 0));
			}
			return false;
		}
	}

	te_debug_beam(bottom, top, 1, RGBA(0, 255, 0));

	// can be inside world if on a slope, despite point_contents saying it's not
	bottom.z += 1.0f;

	//if (debug)
	//	ALERT(at_console, "Rain height %d\n", (int)(bottom - top).Length());

	return true;
}

int CEnvWeather::FindNeighborSpot(std::vector<weather_spot_t>& spots, Vector pos, weather_spot_t& self) {
	for (int k = 0; k < (int)spots.size(); k++) {
		if (spots[k].valid && spots[k].size == self.size && (spots[k].pos - pos).Length() < 8) {
			if (spots[k].isFloating == self.isFloating)
				return k;
		}
	}
	return -1;
}

bool CEnvWeather::IsUnevenGround(Vector pos, float radius) {
	TraceResult tr;

	for (int x = -radius; x <= radius; x += 16) {
		for (int y = -radius; y <= radius; y += 16) {
			Vector offset = Vector(x, y, 8);
			Vector start = pos + offset;
			Vector end = pos + (offset - Vector(0, 0, 128));
			TRACE_LINE(start, end, ignore_monsters, NULL, &tr);

			if (tr.fStartSolid) {
				continue;
			}

			if (fabs(tr.vecEndPos.z - pos.z) > 4) {
				return true;
			}
		}
	}

	return false;
}

std::vector<weather_spot_t> CEnvWeather::FindWeatherSpots() {
	std::vector<weather_spot_t> spots;
	uint64_t startTime = getEpochMillis();

	Vector worldMins = g_bsp.models[0].nMins;
	Vector worldMaxs = g_bsp.models[0].nMaxs;

	int startX = (int)(worldMins.x / WEATHER_SIZE) * WEATHER_SIZE;
	int startY = (int)(worldMins.y / WEATHER_SIZE) * WEATHER_SIZE;
	int startZ = (int)(worldMins.z / WEATHER_SIZE) * WEATHER_SIZE;
	int endX = ((int)(worldMaxs.x / WEATHER_SIZE)+1) * WEATHER_SIZE;
	int endY = ((int)(worldMaxs.y / WEATHER_SIZE)+1) * WEATHER_SIZE;
	int endZ = ((int)(worldMaxs.z / WEATHER_SIZE)+1) * WEATHER_SIZE;

	int zStep = 128; // small enough that entire outdoor areas aren't stepped over

	g_engfuncs.pfnServerPrint(UTIL_VarArgs("Generating weather..."));
	for (int y = startY; y <= endY; y += WEATHER_SIZE) {
		for (int x = startX; x <= endX; x += WEATHER_SIZE) {
			float lastRainZ = endZ;

			for (int z = endZ; z >= startZ; z -= zStep) {
				if (z > lastRainZ) {
					//continue; // test distance is larger than step distance
				}

				bool shouldFloat = false;
				Vector bottom, top;
				if (!GetRainPosition(Vector(x, y, z), bottom, top, false, false)) {
					if (!GetRainPosition(Vector(x, y, z), bottom, top, true, false)) {
						continue;
					}
					shouldFloat = true; // weather hull is point size so it will fall otherwise
				}
				
				bool isUnique = true;
				for (int i = 0; i < (int)spots.size(); i++) {
					Vector delta = spots[i].pos - bottom;
					Vector flatDelta = delta;
					flatDelta.z = 0;

					if (flatDelta.Length() < WEATHER_SIZE*0.9f && fabs(delta.z) < WEATHER_HEIGHT*0.9f ) {
						isUnique = false;
						break;
					}
				}
				if (!isUnique) {
					continue;
				}

				bool unevenGround = IsUnevenGround(bottom, WEATHER_SIZE * 0.5f);
				const char* model = unevenGround ? RAIN_MODEL_SMALL : RAIN_MODEL_SMALL_PUDDLES;

				lastRainZ = bottom.z;
				spots.push_back({ bottom, model, 1, true, shouldFloat, unevenGround });

				int stacks = ((bottom - top).Length() + WEATHER_HEIGHT * 0.5f) / WEATHER_HEIGHT;
				for (int i = 1; i < stacks; i++) {
					spots.push_back({ bottom + Vector(0,0,WEATHER_HEIGHT * i), RAIN_MODEL_SMALL, 1, true, true, false });
				}
			}
		}
	}
	g_engfuncs.pfnServerPrint(UTIL_VarArgs("DONE (%.2fs)\n", TimeDifference(startTime, getEpochMillis())));

	return spots;
}

void CEnvWeather::MergeWeatherSpots(std::vector<weather_spot_t>& spots) {
	// merge into larger models
	for (int i = 0; i < (int)spots.size(); i++) {
		weather_spot_t& spot = spots[i];

		if (!spot.valid) {
			continue;
		}

		int right = FindNeighborSpot(spots, spot.pos + Vector(WEATHER_SIZE, 0, 0), spot);
		int bottom = FindNeighborSpot(spots, spot.pos + Vector(0, WEATHER_SIZE, 0), spot);
		int botrt = FindNeighborSpot(spots, spot.pos + Vector(WEATHER_SIZE, WEATHER_SIZE, 0), spot);

		if (right != -1 && bottom != -1 && botrt != -1) {
			Vector center = (spot.pos + spots[right].pos + spots[bottom].pos + spots[botrt].pos) / 4.0f;
			bool uneven = spot.isUnevenGround || spots[right].isUnevenGround || spots[bottom].isUnevenGround || spots[botrt].isUnevenGround;
			const char* model = (spot.isFloating || uneven) ? RAIN_MODEL_BIG : RAIN_MODEL_BIG_PUDDLES;
			spots.push_back({ center, model, 4, true, spot.isFloating, uneven });
			spots[i].valid = false;
			spots[right].valid = false;
			spots[bottom].valid = false;
			spots[botrt].valid = false;
		}
	}
}

void CEnvWeather::Spawn(void)
{
	if (g_weather_init_done) {
		ALERT(at_console, "Removing %s (%s). Only one can weather entity can exist in the map\n",
			STRING(pev->targetname), STRING(pev->classname));
		UTIL_Remove(this);
		return;
	}

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;

	// model is needed so AddToFullPack is called by rehlds
	SET_MODEL(edict(), NOT_PRECACHED_MODEL);
	pev->rendermode = kRenderTransTexture;

	// Move entity outside of this solar system so that the client plays sound effects centered.
	// The direction the sound is coming from will be directly upward anywhere in the world.
	// The logical solution of using aiment doesn't work (strafing shifts balance).
	Vector galaxySize = Vector(FLT_MAX, FLT_MAX, FLT_MAX);
	UTIL_SetOrigin(pev, Vector(0, 0, FLT_MAX));
	UTIL_SetSize(pev, galaxySize * -1, galaxySize); // so that the client considers this entity visible

	SetThink(&CEnvWeather::WeatherThink);
	SetUse(NULL);

	pev->nextthink = gpGlobals->time + 0.1f;

	Precache();

	std::vector<weather_spot_t> spots = FindWeatherSpots();
	int validRains = spots.size();

	MergeWeatherSpots(spots);

	g_weatherEnts.clear();

	for (int i = 0; i < (int)spots.size(); i++) {
		weather_spot_t& spot = spots[i];

		if (!spot.valid) {
			continue;
		}

		weather_ent_t weather;
		weather.h_ent = NULL;
		weather.model = spot.model;
		weather.pos = spot.pos;
		weather.size = spot.size;
		weather.isFloating = spot.isFloating;
		weather.visPlayers = 0;
		weather.isUnevenGround = spot.isUnevenGround;
		g_weatherEnts.push_back(weather);
	}

	m_rainSnd_out.file = RAIN_OUT_SOUND;
	m_rainSnd_glass.file = RAIN_GLASS_SOUND;
	m_rainSnd_out.channel = CHAN_BODY;
	m_rainSnd_glass.channel = CHAN_ITEM;

	ALERT(at_console, "Found %d potential rain locations. Merged to %d\n",
		validRains, g_weatherEnts.size());

	g_weather_init_done = true;
}

void CEnvWeather::Precache(void)
{
	PRECACHE_MODEL(RAIN_MODEL_SMALL);
	PRECACHE_MODEL(RAIN_MODEL_BIG);
	PRECACHE_MODEL(RAIN_MODEL_SMALL_PUDDLES);
	PRECACHE_MODEL(RAIN_MODEL_BIG_PUDDLES);
	m_rainSplashSpr = PRECACHE_MODEL(RAIN_SPLASH_SPR);
	PRECACHE_SOUND(RAIN_OUT_SOUND);
	PRECACHE_SOUND(RAIN_GLASS_SOUND);
}

void CEnvWeather::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "asdf"))
	{
		//m_flDelay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

bool CEnvWeather::CanSeePosition(CBasePlayer* plr, Vector pos) {
	std::vector<temp_nonsolid_ent_t> tempNonSolidEdicts;
	TraceResult tr;

	bool canSee = true;
	while (true) {
		UTIL_TraceLine(plr->GetGunPosition(), pos + Vector(0, 0, 8), ignore_monsters, ignore_glass, NULL, &tr);

		if (tr.flFraction < 1.0f) {
			// ok to hit a glass wall/ceiling. func_wall blocks ignore_glass so have to double check
			if (ENTINDEX(tr.pHit) != 0 && tr.pHit->v.renderamt < 255 && tr.pHit->v.rendermode) {
				tempNonSolidEdicts.push_back({ tr.pHit, tr.pHit->v.solid });
				tr.pHit->v.solid = SOLID_NOT;
				continue;
			}
			canSee = false;
			break;
		}

		break;
	}

	for (int e = 0; e < (int)tempNonSolidEdicts.size(); e++) {
		tempNonSolidEdicts[e].ent->v.solid = tempNonSolidEdicts[e].oldSolid;
	}

	return canSee;
}

bool CEnvWeather::TraceToSky(Vector pos, Vector& skyPos) {
	TraceResult tr;

	Vector start = pos;
	Vector end = pos + Vector(0, 0, 4096);

	// keep going up until we hit a sky texture or leave the universe
	while (end.z < MAX_WORLD_DIM) {
		UTIL_TraceLine(start, end, ignore_monsters, dont_ignore_glass, NULL, &tr);

		if (tr.flFraction < 1.0f && IsSky(tr, start)) {
			skyPos = tr.vecEndPos;
			return true;
		}

		// assuming 4k units is too far to be useful for whatever is calling this, save some cpu time
		if (tr.fAllSolid) {
			return false;
		}

		start = tr.vecEndPos + Vector(0,0,1); // enter the blocking object
		end.z = start.z + 4096;
	}

	return false;
}

void CEnvWeather::WeatherEntsThink() {
	int totalVis = 0;

	for (int k = 0; k < (int)g_weatherEnts.size(); k++) {
		weather_ent_t& weather = g_weatherEnts[k];

		if (weather.visPlayers && !weather.h_ent) {
			std::unordered_map<std::string, std::string> keys = {
				{"spawnflags", "3"},
				{"model", weather.model},
				{"speed", UTIL_VarArgs("%d", RANDOM_LONG(150, 200))},
				{"rendermode", "5"},
				{"renderamt", "50"},
			};

			CBaseEntity* ent = Create("func_conveyor", weather.pos, Vector(0, 0, 0), NULL, keys);
			ent->pev->solid = SOLID_NOT;

			ent->pev->movetype = weather.isFloating ? MOVETYPE_NONE : MOVETYPE_TOSS;
			
			// not setting point size because that can fall through tiny cracks 
			// or even solid ents sometimes (wreckhouse2 misaligned glass ceil)
			UTIL_SetSize(ent->pev, Vector(-16, -16, 0), Vector(16, 16, 16));

			weather.h_ent = ent;
		}
		else if (!weather.visPlayers && weather.h_ent) {
			UTIL_Remove(weather.h_ent);
			continue;
		}

		CBaseEntity* ent = weather.h_ent;

		if (ent) {
			ent->pev->flags &= ~FL_ONGROUND;
			weather.pos.z = weather.h_ent->pev->origin.z; // so it isn't re-created hovering in the air
			
			// don't slide down slopes
			ent->pev->origin.x = weather.pos.x;
			ent->pev->origin.y = weather.pos.y;

			if (!weather.isFloating) {
				if (UTIL_PointInLiquid(ent->pev->origin + Vector(0, 0, 1))) {
					Vector ori = ent->pev->origin;
					ent->pev->origin.z = UTIL_WaterLevel(ori, ori.z, ori.z + 4096);
				}

				// don't keep sinking after correcting position, unless no longer above water
				if (UTIL_PointInLiquid(ent->pev->origin - Vector(0, 0, 1))) {
					ent->pev->movetype = MOVETYPE_NONE;
				}
				else {
					ent->pev->movetype = MOVETYPE_TOSS;
				}
			}

			totalVis++;
		}
	}

	//ALERT(at_console, "%d / %d rain vis\n", totalVis, g_weatherEnts.size());
}

int CEnvWeather::WaterSplashes(CBasePlayer* plr) {
	const float splashRadius = 1024;
	const int splashCount = 30;
	int numSplash = 0;
	Vector playerOri = plr->GetViewPosition();
	TraceResult tr;
	float glassLoudness = 0;
	float outLoudness = 0;

	for (int k = 0; k < splashCount; k++) {
		// get a random position in a radius around the player
		Vector pos = plr->pev->origin + plr->pev->mins;
		MAKE_VECTORS(Vector(0, RANDOM_FLOAT(0, 360), 0));
		pos = pos + gpGlobals->v_forward * RANDOM_FLOAT(16, splashRadius);
		
		Vector skyPos;
		if (!TraceToSky(pos, skyPos)) {
			continue; // no sky above this point
		}
		//te_debug_beam(pos, skyPos, 1, RGBA(0, 0, 255));

		pos = skyPos;

		// find the floor position
		TRACE_LINE(pos, pos - Vector(0, 0, 8192), ignore_monsters, NULL, &tr);
		Vector bottom = tr.vecEndPos;
		edict_t* floorEnt = tr.pHit;
		Vector floorNormal = tr.vecPlaneNormal;

		//te_debug_beam(pos, bottom, 1, RGBA(0, 255, 0));

		// move up if inside water when on the floor
		if (UTIL_PointInLiquid(bottom)) {
			bottom.z = UTIL_WaterLevel(bottom, bottom.z, skyPos.z);
		}

		// don't render droplets blocked by the world or an opaque entity
		if (!CanSeePosition(plr, bottom)) {
			//te_debug_beam(playerOri, bottom, 1, RGBA(255, 0, 0));
			continue;
		}

		// reflect droplet velocity
		const Vector rainVel = Vector(0, 0, -1);
		float dot = DotProduct(rainVel, floorNormal);
		Vector bounceDir = rainVel - (floorNormal * dot * 2.0f);
		float bounceSpeed = 1.0f;
		if (dot > -1.0f) {
			bounceSpeed += (dot + 1.0f);
		}

		MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, plr->edict());
		WRITE_BYTE(TE_SPRAY);
		WRITE_COORD(bottom.x);	// pos
		WRITE_COORD(bottom.y);
		WRITE_COORD(bottom.z);
		WRITE_COORD(bounceDir.x);	// dir
		WRITE_COORD(bounceDir.y);
		WRITE_COORD(bounceDir.z);
		WRITE_SHORT(m_rainSplashSpr);	// model
		WRITE_BYTE(1);			// count
		WRITE_BYTE(bounceSpeed * 60);			// speed
		WRITE_BYTE(40);			// noise ( client will divide by 100 )
		WRITE_BYTE(5);	// render mdoe
		MESSAGE_END();

		numSplash++;

		// accumulate loudness
		const float soundDist = 1024;
		float dist = (bottom - playerOri).Length();
		float closeness = V_max(0, ((soundDist - dist) / soundDist)); // 0-1 = far-close

		if (ENTINDEX(floorEnt) != 0 && floorEnt->v.renderamt < 255 && floorEnt->v.rendermode) {
			// hit glass, add volume to the glass sound effect
			glassLoudness += closeness * 1.0f;
		}
		else {
			// hit world. reduce volume if glass is between the droplet and the player
			UTIL_TraceLine(plr->GetGunPosition(), bottom + Vector(0, 0, 8), ignore_monsters, NULL, &tr);
			float mult = tr.flFraction < 1.0f ? 0.5f : 1.0f;
			outLoudness += closeness * 0.5f * mult;
		}
	}

	outLoudness = V_min(1.0f, outLoudness);
	glassLoudness = V_min(1.0f, glassLoudness);

	int plridx = plr->entindex() - 1;
	m_rainSnd_glass.loudHist[plridx][m_historyIdx] = glassLoudness;
	m_rainSnd_out.loudHist[plridx][m_historyIdx] = outLoudness;

	return numSplash;
}

int CEnvWeather::UpdateRainVisibility(CBasePlayer* plr) {
	bool anyNearbyRain = false;
	Vector playerOri = plr->GetViewPosition();
	int plrbit = PLRBIT(plr->edict());
	int numVis = 0;

	for (int k = 0; k < (int)g_weatherEnts.size(); k++) {
		weather_ent_t& weather = g_weatherEnts[k];

		Vector delta = (weather.pos + Vector(0, 0, WEATHER_HEIGHT * 0.5f)) - playerOri;

		const float renderDist = 1100;

		if (delta.Length() < renderDist) {
			anyNearbyRain = true;
			weather.visPlayers |= plrbit;
			numVis++;
		}
	}

	return numVis;
}

void CEnvWeather::PlayWeatherSounds(CBasePlayer* plr) {
	int plrbit = PLRBIT(plr->edict());
	int pidx = plr->entindex() - 1;

	weather_sound_t* sounds[2] = { &m_rainSnd_out, &m_rainSnd_glass };
	int vols[2];

	for (int k = 0; k < 2; k++) {
		weather_sound_t* snd = sounds[k];
		int vol = snd->GetAverageLoudness(pidx);
		vols[k] = vol;

		if (snd->lastVol[pidx] == vol) {
			continue;
		}

		snd->lastVol[pidx] = vol;
		float v = vol * 0.01f * 0.5f;

		//ALERT(at_console, "%d|%d loud\n", m_rainSnd_out.GetAverageLoudness(pidx), m_rainSnd_glass.GetAverageLoudness(pidx));

		StartSound(entindex(), snd->channel, snd->file, v, 0.0f, SND_CHANGE_VOL, 100, g_vecZero, plrbit);
	}
}

void CEnvWeather::WeatherThink(void)
{
	WeatherEntsThink();

	// reset vis
	for (int k = 0; k < (int)g_weatherEnts.size(); k++) {
		weather_ent_t& weather = g_weatherEnts[k];
		weather.visPlayers = 0;
	}

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = (CBasePlayer*)UTIL_PlayerByIndex(i);
		if (!plr) {
			continue;
		}

		int pidx = i - 1;
		bool hideAll = false;

		/*
		static Vector testPos;
		if (plr->pev->button & IN_USE) {
			testPos = plr->pev->origin;
			testPos.x = (int)((testPos.x + 64) / WEATHER_SIZE) * WEATHER_SIZE;
			testPos.y = (int)((testPos.y + 64) / WEATHER_SIZE) * WEATHER_SIZE;
			testPos.z = (int)((testPos.z + 64) / 128) * 128;
		}
		
		hideAll = plr->pev->button & IN_RELOAD;
		Vector bottom, top;
		GetRainPosition(testPos, bottom, top, true, true);
		*/
		
		int numVis = 0;
		if (!hideAll) {
			numVis = UpdateRainVisibility(plr);
		}

		// water splashes
		int numSplash = 0;
		if (numVis > 0) {
			numSplash = WaterSplashes(plr);
		}
		else {
			m_rainSnd_glass.loudHist[pidx][m_historyIdx] = 0;
			m_rainSnd_out.loudHist[pidx][m_historyIdx] = 0;
		}

		PlayWeatherSounds(plr);

		/*
		ALERT(at_console, "%d / %d rain vis, %d|%d loud, %d splash\n",
			numVis, g_weatherEnts.size(),
			m_rainSnd_out.GetAverageLoudness(pidx), m_rainSnd_glass.GetAverageLoudness(pidx), numSplash);
			*/
	}

	m_historyIdx = (m_historyIdx + 1) % WEATHER_VOL_HIST_SZ;
	pev->nextthink = gpGlobals->time + 0.05;
}
