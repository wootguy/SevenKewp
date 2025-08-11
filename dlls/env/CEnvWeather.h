#pragma once
#include "CBaseAnimating.h"
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

extern bool g_fog_enabled;
extern int g_fog_start_dist;
extern int g_fog_end_dist;

extern EHANDLE g_fog_ents[FOG_LAYERS];

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

	// for resuming sound after stopsound
	float lastMsg[32];

	const char* file;
	int channel;

	int GetAverageLoudness(int playerindex);
};

// only one active weather type allowed in the map
extern std::vector<weather_ent_t> g_weatherEnts;
extern bool g_weather_init_done;

void EnvWeatherServerDeactivate();

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
	BOOL	IsWeather(void) { return TRUE; }
	virtual CEnvWeather* MyWeatherPointer(void) { return this; }
	void	UpdateOnRemove(void);

	bool GetRainPosition(Vector pos, Vector& bottom, Vector& top, bool largeHull, bool debug);
	bool IsSky(TraceResult& tr, Vector traceStart);
	bool TraceToSky(Vector pos, Vector& skyPos); // trace upwards until hitting the sky
	bool CanSeePosition(CBasePlayer* plr, Vector pos);
	void WeatherEntsThink();
	void UpdateFog();
	void SendFogMessage(CBasePlayer* plr);
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
	int m_fogStartDistHl; // adjusted fog distance for HL to prevent rendering errors
	int m_fogEndDistHl; // adjusted fog distance for HL to prevent rendering errors
	bool m_useFog;
};