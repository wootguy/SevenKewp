#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "CFuncConveyor.h"

#define RAIN_MODEL_SMALL "models/weather/rain0_128.bsp"
#define RAIN_MODEL_BIG "models/weather/rain1_128.bsp"
#define RAIN_MODEL_SMALL_PUDDLES "models/weather/rain2_128.bsp"
#define RAIN_MODEL_BIG_PUDDLES "models/weather/rain3_128.bsp"
#define RAIN_OUT_SOUND "weather/rain.wav"
#define RAIN_GLASS_SOUND "weather/rain_glass.wav"
#define RAIN_SPLASH_SPR "sprites/rain_splash.spr"

#define SNOW_MODEL_SMALL "models/weather/snow0_128.bsp"
#define SNOW_MODEL_BIG "models/weather/snow1_128.bsp"

#define FOG_MODEL "models/weather/fog.mdl"
#define FOG_MAX_DIST 131072 // maximum radius of the fog sphere at the max sequence and frame
#define FOG_LAYERS 32

// an offset is baked into the model so that additive layers don't conflict with 1st-person muzzle flashes
#define FOG_OFFSET_Z 300

// amount of distance the fog can be out of sync due to lag (longjumping at 250 ping). Without this,
// the fog will clip into held weapons, and transparent entities will sometimes be invisible when moving
#define FOG_LAG_DIST 350

// min distance for there to be no transparency glitches
#define FOG_MIN_DIST_COLOR (FOG_OFFSET_Z + FOG_LAG_DIST)

// black fog doesn't use an additive layer, so no Z offset is needed
// no idea why the 100 is needed, but without it transparent ents still sometimes are invisible when moving
#define FOG_MIN_DIST_BLACK (FOG_LAG_DIST + 100)

// Fog ideas tried and failed:
// - using a dither pattern with an alpha-tested model.
//   pros: transparency works fine at any distance
//   cons: effect is horrible and distracting. Tried large/small dither patterns and a cloudy texture
// - compensate low light levels by changing fog color.
//   Failed because light_level is inaccurate and doesn't account for dlight/flashlight.
//   Can only increase brightness so much, especially if fog is already bright.
// - additive render mode
//   pros: full bright without lighting
//   cons: very strange looking if not using white fog
// - use a small offset for the model with EF_BRIGHTLIGHT
//   pros: fixes transparent entities within that offset
//   cons: requires a higher minimum fog distance (1000). light effect is visible sometimes.
// - combine additive color layer and solid black layer in the model
//   Failed because additive textures in models are affected by lighting, unlike the additive render mode.
// - Scaling model normals to trick the renderer into making the model brighter than it should be.
//   Failed. The client engine must be normalizing.

#define MAX_WORLD_DIM 65536
#define RAIN_TEST_DIST 1024
#define WEATHER_SIZE 128 // width of a weather conveyor
#define WEATHER_HEIGHT 960 // height of a weather conveyor
#define WEATHER_VOL_HIST_SZ 25

#define SF_WEATHER_START_OFF 1

// don't auto adjust fog minimum distance, for when you're sure that there will be no issues
// with transparent entities in the map (for instance if fog is only active during a cutscene,
// or if there is no way for a player to move fast from longjumps or falling).
#define SF_WEATHER_FOG_NO_ADJUST 1024

extern "C" uint32_t g_fog_palette[61][256];
extern "C" int g_fog_skins;

bool g_fog_enabled;
int g_fog_start_dist;
int g_fog_end_dist;

EHANDLE g_fog_ents[FOG_LAYERS];

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

void EnvWeatherServerDeactivate() {
	// ehandles might be pointing to ents from the previous map
	for (int i = 0; i < FOG_LAYERS; i++) {
		g_fog_ents[i] = NULL;
	}
}

enum weather_modes {
	WEATHER_NONE,
	WEATHER_RAIN,
	WEATHER_SNOW
};

class CFogLayer : public CBaseAnimating
{
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	void Spawn(void);
	void Precache(void);
	BOOL IsWeather(void) { return TRUE; };
	virtual int AddToFullPack(struct entity_state_s* state, CBasePlayer* player);
};

class CWeatherConveyor : public CFuncConveyor
{
	BOOL IsWeather(void) { return TRUE; };
};

class CEnvWeather : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	void	Spawn(void);
	void	Precache(void);
	void	EXPORT WeatherThink(void);
	void	KeyValue(KeyValueData* pkvd);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	BOOL	IsWeather(void) { return TRUE; };

	bool GetRainPosition(Vector pos, Vector& bottom, Vector& top, bool largeHull, bool debug);
	bool IsSky(TraceResult& tr, Vector traceStart);
	bool TraceToSky(Vector pos, Vector& skyPos); // trace upwards until hitting the sky
	bool CanSeePosition(CBasePlayer* plr, Vector pos);
	void WeatherEntsThink();
	void UpdateFog();
	int UpdateWeatherVisibility(CBasePlayer* plr); // returns number of nearby conveyors

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

	// adjusts the size of a fog sphere
	void SetFogSphereRadius(CBaseAnimating* fog, float radius, bool useOffsetAnims);

	void SetFogColor(RGB color);

	weather_sound_t m_rainSnd_out; // outdoor rain sound
	weather_sound_t m_rainSnd_glass; // rain hitting window sound
	int m_historyIdx;
	int m_rainSplashSpr;
	int m_weatherMode;
	bool m_isActive;

	Vector m_fogColor;
	int m_fogBody; // model texture to use (palette of 256 colors)
	int m_fogSkin; // model uv coordinates for the texture (pixel in the palette)
	int m_fogStartDist;
	int m_fogEndDist;
	bool m_useFog;
};

LINK_ENTITY_TO_CLASS(env_weather, CEnvWeather)
LINK_ENTITY_TO_CLASS(env_rain, CEnvWeather)
LINK_ENTITY_TO_CLASS(env_snow, CEnvWeather)
LINK_ENTITY_TO_CLASS(env_fog, CEnvWeather)

LINK_ENTITY_TO_CLASS(fog_layer, CFogLayer)
LINK_ENTITY_TO_CLASS(weather_conveyor, CWeatherConveyor)

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

void CFogLayer::Spawn() {
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NOCLIP;
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 128;

	SET_MODEL(edict(), FOG_MODEL);

	pev->sequence = 0;
	pev->frame = 0;
	ResetSequenceInfo();
	pev->framerate = FLT_MIN;

	// render everywhere
	static Vector galaxySize = Vector(65536, 65536, 65536);
	UTIL_SetSize(pev, galaxySize * -1, galaxySize);
}

void CFogLayer::Precache() {
	PRECACHE_MODEL(FOG_MODEL);
}

int CFogLayer::AddToFullPack(struct entity_state_s* state, CBasePlayer* player) {
	if (pev->movetype == MOVETYPE_FOLLOW) {
		return 1;
	}

	if (pev->sequence > 10) {
		state->origin = player->GetViewPosition() + Vector(0, 0, -FOG_OFFSET_Z);
	}
	else {
		state->origin = player->GetViewPosition();
	}
	return 1;
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

	//te_debug_beam(bottom, top, 1, RGBA(0, 255, 0));

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
				const char* model = NOT_PRECACHED_MODEL;
				const char* stackModel = NOT_PRECACHED_MODEL;

				if (m_weatherMode == WEATHER_RAIN) {
					model = unevenGround ? RAIN_MODEL_SMALL : RAIN_MODEL_SMALL_PUDDLES;
					stackModel = RAIN_MODEL_SMALL;
				}
				else if (m_weatherMode == WEATHER_SNOW) {
					stackModel = model = SNOW_MODEL_SMALL;
				}

				lastRainZ = bottom.z;
				spots.push_back({ bottom, model, 1, true, shouldFloat, unevenGround });

				int stacks = ((bottom - top).Length() + WEATHER_HEIGHT * 0.5f) / WEATHER_HEIGHT;
				for (int i = 1; i < stacks; i++) {
					spots.push_back({ bottom + Vector(0,0,WEATHER_HEIGHT * i), stackModel, 1, true, true, false });
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
			const char* model = NOT_PRECACHED_MODEL;
			
			if (m_weatherMode == WEATHER_RAIN) {
				model = (spot.isFloating || uneven) ? RAIN_MODEL_BIG : RAIN_MODEL_BIG_PUDDLES;
			}
			else if (m_weatherMode == WEATHER_SNOW) {
				model = SNOW_MODEL_BIG;
			}
			
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
	if (FClassnameIs(pev, "env_snow")) {
		m_weatherMode = WEATHER_SNOW;
	}
	if (FClassnameIs(pev, "env_rain")) {
		m_weatherMode = WEATHER_RAIN;
	}
	if (FClassnameIs(pev, "env_fog")) {
		g_fog_enabled = true;
		m_useFog = true;

		if (pev->iuser2) {
			m_fogStartDist = pev->iuser2;
		}

		if (pev->iuser3) {
			m_fogEndDist = pev->iuser3;
		}

		m_fogColor = pev->rendercolor;
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

	if (m_weatherMode != WEATHER_NONE) {
		if (g_weather_init_done) {
			ALERT(at_console, "Removing %s (%s). Only one can weather entity can exist in the map\n",
				STRING(pev->targetname), STRING(pev->classname));
			UTIL_Remove(this);
			return;
		}

		std::vector<weather_spot_t> spots = FindWeatherSpots();
		int validSpots = spots.size();

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

		ALERT(at_console, "Found %d potential weather locations. Merged to %d\n",
			validSpots, g_weatherEnts.size());

		g_weather_init_done = true;
	}

	m_isActive = !FBitSet(pev->spawnflags, SF_WEATHER_START_OFF);

	if (m_useFog) {
		int minDist = m_fogColor == g_vecZero ? FOG_MIN_DIST_BLACK : FOG_MIN_DIST_COLOR;
		int newMin = V_max(minDist, m_fogStartDist);
		int newMax = V_max(newMin + 100, m_fogEndDist); // fog won't look nice if too compacted

		if (newMin > m_fogStartDist || newMax > m_fogEndDist) {
			if (pev->spawnflags & SF_WEATHER_FOG_NO_ADJUST) {
				newMin = m_fogStartDist;
				newMax = m_fogEndDist;
				ALERT(at_console, "env_weather: fog distance adjustment disabled via flag. Rendering errors may occur.\n");
			}
			else {
				ALERT(at_console, "env_weather: fog distance increased to prevent rendering errors. [%d - %d] -> [%d - %d]\n",
					m_fogStartDist, m_fogEndDist, newMin, newMax);
			}
		}

		m_fogStartDist = newMin;
		m_fogEndDist = newMax;

		// reverse order so fog renders correctly
		for (int k = FOG_LAYERS - 1; k >= 0; k--) {
			if (!g_fog_ents[k]) {
				g_fog_ents[k] = Create("fog_layer", g_vecZero, g_vecZero);
				if (!m_isActive)
					g_fog_ents[k]->pev->effects = EF_NODRAW;;
			}
		}

		if (m_isActive)
			UpdateFog();
	}
}

void CEnvWeather::Precache(void)
{
	if (m_weatherMode == WEATHER_RAIN) {
		PRECACHE_MODEL(RAIN_MODEL_SMALL);
		PRECACHE_MODEL(RAIN_MODEL_BIG);
		PRECACHE_MODEL(RAIN_MODEL_SMALL_PUDDLES);
		PRECACHE_MODEL(RAIN_MODEL_BIG_PUDDLES);
		m_rainSplashSpr = PRECACHE_MODEL(RAIN_SPLASH_SPR);
		PRECACHE_SOUND(RAIN_OUT_SOUND);
		PRECACHE_SOUND(RAIN_GLASS_SOUND);
	}
	else if (m_weatherMode == WEATHER_SNOW) {
		PRECACHE_MODEL(SNOW_MODEL_SMALL);
		PRECACHE_MODEL(SNOW_MODEL_BIG);
	}

	if (m_useFog) {
		UTIL_PrecacheOther("fog_layer");
	}
}

void CEnvWeather::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "mode"))
	{
		m_weatherMode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "startdist"))
	{
		m_fogStartDist = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "enddist"))
	{
		m_fogEndDist = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CEnvWeather::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	m_isActive = useType == USE_TOGGLE ? !m_isActive : useType == USE_ON;
	
	if (m_isActive) {
		// disable all other weather entities
		CBaseEntity* pWeather = NULL;
		while ((pWeather = UTIL_FindEntityByClassname(pWeather, "env_weather")) != NULL) {
			pWeather->Use(pActivator, this, USE_OFF, 0.0f);
		}
	}

	if (m_useFog) {
		g_fog_enabled = m_isActive;

		if (m_isActive) {
			UpdateFog();
		}

		for (int k = 0; k < FOG_LAYERS; k++) {
			CFogLayer* fog = (CFogLayer*)g_fog_ents[k].GetEntity();
			if (!fog) {
				continue;
			}

			if (m_isActive) {
				fog->pev->effects &= ~EF_NODRAW;
			}
			else {
				fog->pev->effects |= EF_NODRAW;
			}
		}
	}

	if (m_weatherMode != WEATHER_NONE && !m_isActive) {
		for (int k = 0; k < (int)g_weatherEnts.size(); k++) {
			weather_ent_t& weather = g_weatherEnts[k];

			weather.visPlayers = 0;
			UTIL_Remove(weather.h_ent);
		}

		// TODO: fade out
		weather_sound_t* sounds[2] = { &m_rainSnd_out, &m_rainSnd_glass };
		for (int k = 0; k < 2; k++) {
			weather_sound_t* snd = sounds[k];
			STOP_SOUND(edict(), snd->channel, snd->file);
		}
	}
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

void CEnvWeather::SetFogSphereRadius(CBaseAnimating* fog, float radius, bool useOffsetAnims) {
	// each sequence has a larger max distance for the sphere, but reduced accuracy
	int sequences[] = { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1 };
	int thresholds[] = { 65536, 32768, 16384, 8192, 4096, 2048, 1024, 512, 256, 0 };

	for (int i = 0; i < 10; ++i) {
		if (radius > thresholds[i]) {
			fog->pev->sequence = sequences[i];
			break;
		}
	}

	float sphereMaxDist = 256 << (fog->pev->sequence-1);

	fog->pev->frame = clampf(roundf((radius / sphereMaxDist) * 256.0f) - 1, 0, 255);

	if (useOffsetAnims) {
		fog->pev->sequence += 10; // use animations with an offset baked into them
	}

	fog->ResetSequenceInfo();
	fog->pev->framerate = FLT_MIN;
}

void CEnvWeather::SetFogColor(RGB color) {
	int bestDist = INT_MAX;
	int bestSkin = 0;
	int bestBody = 0;

	// adjustment is needed to account for the blending of additive and transparent models.
	// This isn't perfect but neither is the color palette in the model.
	float scaleR = ((color.r / 16) * 0.05f) + 0.2f;
	float scaleG = ((color.g / 16) * 0.05f) + 0.2f;
	float scaleB = ((color.b / 16) * 0.05f) + 0.2f;
	color.r = V_min(255, color.r * scaleR);
	color.g = V_min(255, color.g * scaleG);
	color.b = V_min(255, color.b * scaleB);

	for (int i = 0; i < g_fog_skins && bestDist; i++) {
		for (int k = 0; k < 256; k++) {
			RGB p = RGB(g_fog_palette[i][k]);
			int dist = abs(p.r - color.r) + abs(p.g - color.g) + abs(p.b - color.b);

			if (dist < bestDist) {
				bestDist = dist;
				bestSkin = i;
				bestBody = k;

				if (dist == 0) {
					break;
				}
			}
		}
	}

	m_fogSkin = bestSkin;
	m_fogBody = bestBody;
}

void CEnvWeather::UpdateFog() {
	g_fog_start_dist = m_fogStartDist;
	g_fog_end_dist = m_fogEndDist;

	float spanDist = g_fog_end_dist - g_fog_start_dist;

	SetFogColor(m_fogColor);

	// fine tuned values for 16 layers
	float renderamtBlack = 10;
	float scalingFactorBlack = 1.2f;
	float renderamtAdd = 15;
	float scalingFactorAdd = 1.15f;

	bool colorFog = m_fogColor != g_vecZero;

	for (int k = 0; k < FOG_LAYERS; k++) {
		CFogLayer* fog = (CFogLayer*)g_fog_ents[k].GetEntity();
		
		if (!fog || !fog->MyAnimatingPointer()) {
			continue;
		}
		
		if (k % 2 == 1) {
			// black layer as a backdrop for the additive layer, for light removal
			fog->pev->skin = 0;
			fog->pev->body = 0;

			if (k == FOG_LAYERS - 1) {
				// final black layer is fully solid
				fog->pev->rendermode = kRenderNormal;
			}
			else {
				fog->pev->rendermode = kRenderTransTexture;
				fog->pev->renderamt = renderamtBlack;
				renderamtBlack *= scalingFactorBlack;
			}

			float t = k / (float)(FOG_LAYERS - 1);
			SetFogSphereRadius(fog, g_fog_start_dist + spanDist * t, colorFog);
		}
		else {
			// color layer
			fog->pev->skin = m_fogSkin;
			fog->pev->body = m_fogBody;

			if (!colorFog) {
				fog->pev->effects = EF_NODRAW;
			}
			else {
				fog->pev->rendermode = kRenderTransAdd;

				fog->pev->renderamt = renderamtAdd;
				renderamtAdd *= scalingFactorAdd;

				fog->pev->aiment = g_fog_ents[k + 1].GetEdict();
				fog->pev->movetype = MOVETYPE_FOLLOW;

				// use same scale as upcoming black layer, so they perfectly overlap
				float t = (k+1) / (float)(FOG_LAYERS - 1);
				SetFogSphereRadius(fog, g_fog_start_dist + spanDist * t, colorFog);
			}
		}
	}

	//ALERT(at_console, "Fog: %d ents, dist %d -> %d\n", fogCount, g_fog_start_dist, g_fog_end_dist);
}

void CEnvWeather::WeatherEntsThink() {
	int totalVis = 0;

	for (int k = 0; k < (int)g_weatherEnts.size(); k++) {
		weather_ent_t& weather = g_weatherEnts[k];

		if (weather.visPlayers && !weather.h_ent) {
			StringMap keys = {
				{"spawnflags", "3"},
				{"model", weather.model},
			};

			CFuncConveyor* ent = (CFuncConveyor*)Create("weather_conveyor", weather.pos, Vector(0, 0, 0), true, NULL, keys);
			ent->pev->solid = SOLID_NOT;
			ent->pev->movetype = weather.isFloating ? MOVETYPE_NONE : MOVETYPE_TOSS;
			ent->pev->rendermode = kRenderTransAdd;
			ent->pev->renderamt = 50;
			
			if (m_weatherMode == WEATHER_RAIN) {
				ent->UpdateSpeed(RANDOM_LONG(150, 200));
			}
			else if (m_weatherMode == WEATHER_SNOW) {
				ent->UpdateSpeed(48);
				ent->pev->gravity = 0.2f;
				ent->pev->renderamt = 100;
			}

			// each face has a unique texture to prevent particles perfectly lining up
			ent->pev->angles.y = RANDOM_LONG(0, 3)*90;
			
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
			
			// copy renderamt from weather entity, if set
			if (pev->renderamt) {
				ent->pev->renderamt = pev->renderamt;
			}

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

int CEnvWeather::UpdateWeatherVisibility(CBasePlayer* plr) {
	Vector playerOri = plr->GetViewPosition();
	int plrbit = PLRBIT(plr->edict());
	int numVis = 0;

	for (int k = 0; k < (int)g_weatherEnts.size(); k++) {
		weather_ent_t& weather = g_weatherEnts[k];

		Vector delta = (weather.pos + Vector(0, 0, WEATHER_HEIGHT * 0.5f)) - playerOri;

		const float renderDist = 1100;

		if (delta.Length() < renderDist) {
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

	for (int k = 0; k < 2; k++) {
		weather_sound_t* snd = sounds[k];
		int vol = snd->GetAverageLoudness(pidx);

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
	if (g_fog_enabled && m_useFog) {
		uint32_t softwarePlayers = 0;

		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			CBasePlayer* plr = (CBasePlayer*)UTIL_PlayerByIndex(i);
			if (!plr) {
				continue;
			}

			if (plr->m_clientRenderer == CLIENT_RENDERER_SOFTWARE) {
				softwarePlayers |= PLRBIT(i);
			}
		}

		for (int k = 0; k < FOG_LAYERS; k++) {
			CFogLayer* fog = (CFogLayer*)g_fog_ents[k].GetEntity();
			if (fog) {
				// smoothes movement somehow.
				// TODO: HOW? velocity is not sent to the client and the origin will be overridden
				// later with m_fakeFollow.
				fog->pev->velocity = Vector(0, 0, 1) * sinf(gpGlobals->time);

				// overdraw is the software mode killer. Don't even try to render fog.
				fog->m_hidePlayers |= softwarePlayers;
			}
		}
	}

	if (m_weatherMode != WEATHER_NONE && m_isActive) {
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
				numVis = UpdateWeatherVisibility(plr);
			}

			if (m_weatherMode == WEATHER_RAIN) {
				// water splashes
				//int numSplash = 0;
				if (numVis > 0) {
					//numSplash = WaterSplashes(plr);
					WaterSplashes(plr);
				}
				else {
					m_rainSnd_glass.loudHist[pidx][m_historyIdx] = 0;
					m_rainSnd_out.loudHist[pidx][m_historyIdx] = 0;
				}

				PlayWeatherSounds(plr);
			}

			/*
			ALERT(at_console, "%d / %d rain vis, %d|%d loud, %d splash\n",
				numVis, g_weatherEnts.size(),
				m_rainSnd_out.GetAverageLoudness(pidx), m_rainSnd_glass.GetAverageLoudness(pidx), numSplash);
				*/
		}

		m_historyIdx = (m_historyIdx + 1) % WEATHER_VOL_HIST_SZ;
	}

	pev->nextthink = gpGlobals->time + 0.05;
}
