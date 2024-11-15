#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "env/CSoundEnt.h"
#include "hlds_hooks.h"
#include "decals.h"
#include "skill.h"
#include "effects.h"
#include "CBasePlayer.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "bodyque.h"
#include "PluginManager.h"

extern CGraph WorldGraph;
extern CSoundEnt* pSoundEnt;
DLL_GLOBAL edict_t* g_pBodyQueueHead;
extern DLL_GLOBAL	int			gDisplayTitle;

bool g_freeRoam;

extern void W_Precache(void);

// moved CWorld class definition to cbase.h
//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================

LINK_ENTITY_TO_CLASS(worldspawn, CWorld)

#define SF_WORLD_DARK		0x0001		// Fade from black at startup
#define SF_WORLD_TITLE		0x0002		// Display game title at startup
#define SF_WORLD_FORCETEAM	0x0004		// Force teams

extern DLL_GLOBAL BOOL		g_fGameOver;
float g_flWeaponCheat;

void CWorld::Spawn(void)
{
	g_fGameOver = FALSE;
	Precache();
	g_flWeaponCheat = CVAR_GET_FLOAT("sv_cheats");  // Is the impulse 101 command allowed?

	g_freeRoam = m_freeRoam;
}

void CWorld::loadReplacementFiles() {
	const char* gmrPath = "hlcoop.gmr";
	const char* gsrPath = "hlcoop.gsr";
	static uint64_t lastEditTimeGmr = 0;
	static uint64_t lastEditTimeGsr = 0;

	std::string mpath = getGameFilePath(gmrPath);
	std::string spath = getGameFilePath(gsrPath);

	if (mpath.empty()) {
		g_modelReplacementsMod.clear();
		ALERT(at_warning, "Missing replacement file: %s\n", gmrPath);
	}
	if (spath.empty()) {
		g_soundReplacementsMod.clear();
		ALERT(at_warning, "Missing replacement file: %s\n", gsrPath);
	}

	uint64_t editTimeGmr = getFileModifiedTime(mpath.c_str());
	uint64_t editTimeGsr = getFileModifiedTime(spath.c_str());

	if (lastEditTimeGmr != editTimeGmr) {
		lastEditTimeGmr = editTimeGmr;
		g_modelReplacementsMod = loadReplacementFile(gmrPath);
	}

	if (lastEditTimeGsr != editTimeGsr) {
		lastEditTimeGsr = editTimeGsr;
		g_soundReplacementsMod = loadReplacementFile(gsrPath);
	}

	g_modelReplacementsMap.clear();
	if (m_globalModelList) {
		const char* mapGmrPath = UTIL_VarArgs("models/%s/%s", STRING(gpGlobals->mapname), STRING(m_globalModelList));
		g_modelReplacementsMap = loadReplacementFile(mapGmrPath);
	}

	g_soundReplacementsMap.clear();
	if (m_globalSoundList) {
		const char* mapGsrPath = UTIL_VarArgs("sound/%s/%s", STRING(gpGlobals->mapname), STRING(m_globalSoundList));
		g_soundReplacementsMap = loadReplacementFile(mapGsrPath);
	}

	// map models/sounds have priority over mod models
	g_modelReplacements.clear();
	g_modelReplacements.insert(g_modelReplacementsMap.begin(), g_modelReplacementsMap.end());
	g_modelReplacements.insert(g_modelReplacementsMod.begin(), g_modelReplacementsMod.end());

	g_soundReplacements.clear();
	g_soundReplacements.insert(g_soundReplacementsMap.begin(), g_soundReplacementsMap.end());
	g_soundReplacements.insert(g_soundReplacementsMod.begin(), g_soundReplacementsMod.end());
}

void CWorld::Precache(void)
{
#if 1
	CVAR_SET_STRING("sv_gravity", "800"); // 67ft/sec
	CVAR_SET_STRING("sv_stepsize", "18");
#else
	CVAR_SET_STRING("sv_gravity", "384"); // 32ft/sec
	CVAR_SET_STRING("sv_stepsize", "24");
#endif

	CVAR_SET_STRING("room_type", "0");// clear DSP

	// Set up game rules
	if (g_pGameRules)
	{
		delete g_pGameRules;
	}

	g_pGameRules = InstallGameRules();
	g_pluginManager.UpdatePluginsFromList();
	if (pluginautoupdate.value) {
		g_pluginManager.UpdatePlugins();
	}
	loadReplacementFiles();

	// init here so sprites can be replaced
	g_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);

	//!!!UNDONE why is there so much Spawn code in the Precache function? I'll just keep it here 

	///!!!LATER - do we want a sound ent in deathmatch? (sjb)
	//pSoundEnt = CBaseEntity::Create( "soundent", g_vecZero, g_vecZero, edict() );
	pSoundEnt = GetClassPtr((CSoundEnt*)NULL);
	pSoundEnt->Spawn();

	if (!pSoundEnt)
	{
		ALERT(at_console, "**COULD NOT CREATE SOUNDENT**\n");
	}

	InitBodyQue();

	// init sentence group playback stuff from sentences.txt.
	// ok to call this multiple times, calls after first are ignored.

	SENTENCEG_Init();

	// init texture type array from materials.txt

	TEXTURETYPE_Init();


	// the area based ambient sounds MUST be the first precache_sounds

	// player precaches     
	W_Precache();									// get weapon precaches

	ClientPrecache();

	const char* skyname = CVAR_GET_STRING("sv_skyname");

	if (strlen(skyname)) {
		// the engine precaches these automatically, this is here for tracking missing files
		PRECACHE_GENERIC(UTIL_VarArgs("gfx/env/%sft.tga", skyname));
		PRECACHE_GENERIC(UTIL_VarArgs("gfx/env/%sbk.tga", skyname));
		PRECACHE_GENERIC(UTIL_VarArgs("gfx/env/%slf.tga", skyname));
		PRECACHE_GENERIC(UTIL_VarArgs("gfx/env/%srt.tga", skyname));
		PRECACHE_GENERIC(UTIL_VarArgs("gfx/env/%sup.tga", skyname));
		PRECACHE_GENERIC(UTIL_VarArgs("gfx/env/%sdn.tga", skyname));

		// bmp files are also precached by the engine, but they're useless?
		// I was once told software mode uses them, but the HL25 client is happy with tga.
	}

	PRECACHE_DETAIL_TEXTURES();

	// wads are precached automatically by the engine but this is needed for tracking missing files
	if (m_wadlist) {
		std::vector<std::string> wads = splitString(STRING(m_wadlist), ";");

		for (std::string wad : wads) {
			int lastSlash = wad.find_last_of("\\/\n");
			if (lastSlash != -1) {
				wad = wad.substr(lastSlash + 1);
			}

			wad = toLowerCase(wad);

			if (wad.find(".wad") != wad.size() - 4) {
				continue;
			}

			if (wad.find("xeno.wad") != std::string::npos || wad.find("halflife.wad") != std::string::npos) {
				// bad logic copied from the engine. This explains why "nwxeno.wad" fails to transfer
				bool unexpected = wad != "xeno.wad" && wad != "halflife.wad";
				ALERT(unexpected ? at_error : at_console, "Engine blacklisted WAD: %s\n", wad.c_str());
				continue;
			}

			PRECACHE_GENERIC(wad.c_str());
		}
	}

	// sounds used from C physics code
	PRECACHE_SOUND("common/null.wav");				// clears sound channels

	PRECACHE_SOUND("items/suitchargeok1.wav");//!!! temporary sound for respawning weapons.
	PRECACHE_SOUND("items/gunpickup2.wav");// player picks up a gun.

	PRECACHE_SOUND("common/bodydrop3.wav");// dead bodies hitting the ground (animation events)
	PRECACHE_SOUND("common/bodydrop4.wav");

	PRECACHE_MODEL(NOT_PRECACHED_MODEL);

	if (mp_mergemodels.value)
		PRECACHE_MODEL(MERGED_ITEMS_MODEL);

	g_Language = (int)CVAR_GET_FLOAT("sv_language");
	if (g_Language == LANGUAGE_GERMAN)
	{
		PRECACHE_MODEL("models/germangibs.mdl");
	}
	else
	{
		PRECACHE_MODEL("models/hgibs.mdl");
		PRECACHE_MODEL("models/agibs.mdl");
	}

	PRECACHE_SOUND("weapons/ric1.wav");
	PRECACHE_SOUND("weapons/ric2.wav");
	PRECACHE_SOUND("weapons/ric3.wav");
	PRECACHE_SOUND("weapons/ric4.wav");
	PRECACHE_SOUND("weapons/ric5.wav");

	PRECACHE_SOUND("weapons/distant/crack_9mm.wav");
	PRECACHE_SOUND("weapons/distant/crack_357.wav");
	PRECACHE_SOUND("weapons/distant/crack_556.wav");
	PRECACHE_SOUND("weapons/distant/explode3.wav");
	//PRECACHE_SOUND("weapons/distant/explode4.wav");
	PRECACHE_SOUND("weapons/distant/explode5.wav");
	//
	// Setup light animation tables. 'a' is total darkness, 'z' is maxbright.
	//

		// 0 normal
	LIGHT_STYLE(0, "m");

	// 1 FLICKER (first variety)
	LIGHT_STYLE(1, "mmnmmommommnonmmonqnmmo");

	// 2 SLOW STRONG PULSE
	LIGHT_STYLE(2, "abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba");

	// 3 CANDLE (first variety)
	LIGHT_STYLE(3, "mmmmmaaaaammmmmaaaaaabcdefgabcdefg");

	// 4 FAST STROBE
	LIGHT_STYLE(4, "mamamamamama");

	// 5 GENTLE PULSE 1
	LIGHT_STYLE(5, "jklmnopqrstuvwxyzyxwvutsrqponmlkj");

	// 6 FLICKER (second variety)
	LIGHT_STYLE(6, "nmonqnmomnmomomno");

	// 7 CANDLE (second variety)
	LIGHT_STYLE(7, "mmmaaaabcdefgmmmmaaaammmaamm");

	// 8 CANDLE (third variety)
	LIGHT_STYLE(8, "mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa");

	// 9 SLOW STROBE (fourth variety)
	LIGHT_STYLE(9, "aaaaaaaazzzzzzzz");

	// 10 FLUORESCENT FLICKER
	LIGHT_STYLE(10, "mmamammmmammamamaaamammma");

	// 11 SLOW PULSE NOT FADE TO BLACK
	LIGHT_STYLE(11, "abcdefghijklmnopqrrqponmlkjihgfedcba");

	// 12 UNDERWATER LIGHT MUTATION
	// this light only distorts the lightmap - no contribution
	// is made to the brightness of affected surfaces
	LIGHT_STYLE(12, "mmnnmmnnnmmnn");

	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	LIGHT_STYLE(63, "a");

	init_decals();

	// init the WorldGraph.
	WorldGraph.InitGraph();

	// make sure the .NOD file is newer than the .BSP file.
	if (!WorldGraph.CheckNODFile((char*)STRING(gpGlobals->mapname)))
	{// NOD file is not present, or is older than the BSP file.
		WorldGraph.AllocNodes();
	}
	else
	{// Load the node graph for this level
		if (!WorldGraph.FLoadGraph((char*)STRING(gpGlobals->mapname)))
		{// couldn't load, so alloc and prepare to build a graph.
			ALERT(at_console, "*Error opening .NOD file\n");
			WorldGraph.AllocNodes();
		}
		else
		{
			ALERT(at_console, "\n*Graph Loaded!\n");
		}
	}

	if (pev->speed > 0)
		CVAR_SET_FLOAT("sv_zmax", pev->speed);
	else
		CVAR_SET_FLOAT("sv_zmax", 4096);

	if (pev->netname)
	{
		ALERT(at_aiconsole, "Chapter title: %s\n", STRING(pev->netname));
		CBaseEntity* pEntity = CBaseEntity::Create("env_message", g_vecZero, g_vecZero, NULL);
		if (pEntity)
		{
			pEntity->SetThink(&CBaseEntity::SUB_CallUseToggle);
			pEntity->pev->message = pev->netname;
			pev->netname = 0;
			pEntity->pev->nextthink = gpGlobals->time + 0.3;
			pEntity->pev->spawnflags = SF_MESSAGE_ONCE;
		}
	}

	if (pev->spawnflags & SF_WORLD_DARK)
		CVAR_SET_FLOAT("v_dark", 1.0);
	else
		CVAR_SET_FLOAT("v_dark", 0.0);

	if (pev->spawnflags & SF_WORLD_TITLE)
		gDisplayTitle = TRUE;		// display the game title if this key is set
	else
		gDisplayTitle = FALSE;

	if (pev->spawnflags & SF_WORLD_FORCETEAM)
	{
		CVAR_SET_FLOAT("mp_defaultteam", 1);
	}
	else
	{
		CVAR_SET_FLOAT("mp_defaultteam", 0);
	}

	CALL_HOOKS_VOID(pfnMapInit);
}


//
// Just to ignore the "wad" field.
//
void CWorld::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "skyname"))
	{
		// Sent over net now.
		CVAR_SET_STRING("sv_skyname", pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wad"))
	{
		m_wadlist = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		gpGlobals->cdAudioTrack = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "WaveHeight"))
	{
		// Sent over net now.
		pev->scale = atof(pkvd->szValue) * (1.0 / 8.0);
		pkvd->fHandled = TRUE;
		CVAR_SET_FLOAT("sv_wateramp", pev->scale);
	}
	else if (FStrEq(pkvd->szKeyName, "MaxRange"))
	{
		pev->speed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "chaptertitle"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "startdark"))
	{
		// UNDONE: This is a gross hack!!! The CVAR is NOT sent over the client/sever link
		// but it will work for single player
		int flag = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
		if (flag)
			pev->spawnflags |= SF_WORLD_DARK;
	}
	else if (FStrEq(pkvd->szKeyName, "newunit"))
	{
		// Single player only.  Clear save directory if set
		if (atoi(pkvd->szValue))
			CVAR_SET_FLOAT("sv_newunit", 1);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "gametitle"))
	{
		if (atoi(pkvd->szValue))
			pev->spawnflags |= SF_WORLD_TITLE;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "mapteams"))
	{
		pev->team = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "defaultteam"))
	{
		if (atoi(pkvd->szValue))
		{
			pev->spawnflags |= SF_WORLD_FORCETEAM;
		}
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "globalmodellist"))
	{
		m_globalModelList = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "globalsoundlist"))
	{
		m_globalSoundList = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "freeroam"))
	{
		m_freeRoam = atoi(pkvd->szValue) == 1;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}
