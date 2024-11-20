/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#ifndef GAME_H
#define GAME_H

#include "CKeyValue.h"
#include "voice_gamemgr.h"
#include <unordered_map>
#include <string>
#include <unordered_set>

extern void GameDLLInit( void );

// subfolders created to separate mod files from vanilla HL files
#define MOD_MODEL_FOLDER "hlcoop_v2/"
#define MOD_SPRITE_FOLDER "hlcoop/"

#define DEFAULT_TEAM_NAME "Team" // name seen in the scoreboard
#define ENEMY_TEAM_NAME "Bad Guys" // name seen in the scoreboard
#define DEFAULT_TEAM_COLOR 1
#define ENEMY_TEAM_COLOR 2
#define NEUTRAL_TEAM_COLOR 3
#define FRIEND_TEAM_COLOR 4
#define OBSERVER_TEAM_COLOR 6 // spectator color (white on windows, still blue on linux?)

EXPORT extern cvar_t	displaysoundlist;

// multiplayer server rules
EXPORT extern cvar_t	teamplay;
EXPORT extern cvar_t	fraglimit;
EXPORT extern cvar_t	timelimit;
EXPORT extern cvar_t	friendlyfire;
EXPORT extern cvar_t	falldamage;
EXPORT extern cvar_t	weaponstay;
EXPORT extern cvar_t	item_despawn_time;
EXPORT extern cvar_t	item_repick_time;
EXPORT extern cvar_t	max_item_drops;
EXPORT extern cvar_t	forcerespawn;
EXPORT extern cvar_t	flashlight;
EXPORT extern cvar_t	aimcrosshair;
EXPORT extern cvar_t	decalfrequency;
EXPORT extern cvar_t	teamlist;
EXPORT extern cvar_t	teamoverride;
EXPORT extern cvar_t	defaultteam;
EXPORT extern cvar_t	allowmonsters;
EXPORT extern cvar_t	mp_nextmap; // map which will load after the next intermission
EXPORT extern cvar_t	mp_starthealth;
EXPORT extern cvar_t	mp_startarmor;
EXPORT extern cvar_t	mp_bulletsponges; // prevent mappers setting high npc health for no good reason
EXPORT extern cvar_t	mp_bulletspongemax; // max health multiplier for any monster
EXPORT extern cvar_t	mp_maxmonsterrespawns; // limit monster respawns which affect no entity logic
EXPORT extern cvar_t	mp_edictsorting; // sorts edict list by entity index priority
EXPORT extern cvar_t	mp_shitcode; // conditionally enables shitty code that fixes critical problems in specific maps, but subtly breaks many others
EXPORT extern cvar_t	mp_survival_supported;
EXPORT extern cvar_t	mp_survival_starton;
EXPORT extern cvar_t	mp_survival_restart;
EXPORT extern cvar_t	mp_mergemodels; // used merged models to save on model slots
EXPORT extern cvar_t	mp_killfeed; // 0 = off, 1 = player deaths, 2 = player kills/deaths, 3 = player + monster kills/deaths
EXPORT extern cvar_t	pluginlistfile; // name of the plugin list file
EXPORT extern cvar_t	adminlistfile; // name of the admin list file
EXPORT extern cvar_t	pluginupdatepath; // root path for plugin file updates to be searched
EXPORT extern cvar_t	pluginautoupdate; // attempt to update plugins after every map change
EXPORT extern cvar_t	mp_skill_allow; // 0 = no, 1 = yes

// Enables classic func_pushable physics (which is horribly broken, but fun)
// The higher your FPS, the faster you can boost pushables. You also get boosted.
EXPORT extern cvar_t	mp_objectboost;

// write network messages to log file for debugging
EXPORT extern cvar_t	mp_debugmsg;

EXPORT extern cvar_t	mp_respawndelay;

// if a map cfg asks for a default max speed (320 for Half-Life, 270 for Sven Co-op),
// then ignore the command and use whatever was set up by the server.cfg
EXPORT extern cvar_t	mp_prefer_server_maxspeed;

// limits monster sound variety to save precache slots.
// 0 disables. 1+ = max sounds per action (death/pain/idle/etc.)
EXPORT extern cvar_t	soundvariety;

EXPORT extern cvar_t	mp_npckill;
EXPORT extern cvar_t	killnpc; // legacy setting. When set to 0, makes scientists and barneys invulnerable

// Engine Cvars
EXPORT extern cvar_t	*g_psv_gravity;
EXPORT extern cvar_t	*g_psv_aim;
EXPORT extern cvar_t	*g_psv_allow_autoaim;
EXPORT extern cvar_t	*g_footsteps;
EXPORT extern cvar_t	*g_developer;
EXPORT extern cvar_t	*sv_max_client_edicts;
EXPORT extern cvar_t	*sv_voiceenable;
EXPORT extern cvar_t	*sv_stepsize;
EXPORT extern cvar_t	*sv_friction;
EXPORT extern cvar_t	*sv_stopspeed;
EXPORT extern cvar_t	*sv_lowercase;

struct NerfStats {
	int nerfedMonsterHealth;
	int nerfedMonsterSpawns;
	int nerfedMonsterInfiniSpawns;
	int skippedMonsterSpawns;
	int skippedMonsterHealth;
	int skippedMonsterInfiniSpawns;
	int totalMonsters;
	int totalMonsterHealth;
};

EXPORT extern NerfStats g_nerfStats;

// flags texture types that are present in the current map
// used to conditionally precache step/impact sounds
struct TextureTypeStats {
	bool tex_concrete;
	bool tex_metal;
	bool tex_dirt;
	bool tex_duct;
	bool tex_grate;
	bool tex_tile;
	bool tex_water;
	bool tex_wood;
	bool tex_computer;
	bool tex_glass;
	bool tex_flesh;
};

EXPORT extern TextureTypeStats g_textureStats;

EXPORT extern std::unordered_map<std::string, std::string> g_modelReplacementsMap; // model replacements for the current map
EXPORT extern std::unordered_map<std::string, std::string> g_modelReplacementsMod; // model replacements for this mod
EXPORT extern std::unordered_map<std::string, std::string> g_modelReplacements; // combined model replacements

EXPORT extern std::unordered_map<std::string, std::string> g_soundReplacementsMod; // sound replacements for the current map
EXPORT extern std::unordered_map<std::string, std::string> g_soundReplacementsMap; // sound replacements for this mod
EXPORT extern std::unordered_map<std::string, std::string> g_soundReplacements; // combined sound replacements

EXPORT extern std::unordered_set<std::string> g_mapWeapons; // weapons which should be precached (don't use aliases here)
EXPORT extern std::unordered_map<std::string, const char*> g_itemNameRemap;

// per-monster sound replacement maps
// should be a class member, but I'm afraid of the bugs that will come from using non-POD class members
EXPORT extern std::vector<std::unordered_map<std::string, std::string>> g_monsterSoundReplacements;

// map for each entity, containing custom keyvalues
// using a global vector instead of a class member because the map is not POD
EXPORT extern std::vector<std::unordered_map<std::string, CKeyValue>> g_customKeyValues;

EXPORT extern CKeyValue g_emptyKeyValue; // a keyvalue initialized with zeroes

EXPORT extern std::unordered_set<std::string> g_shuffledMonsterSounds; // classes that had their sounds shuffled this map

EXPORT extern bool g_cfgsExecuted; // set to true after server and map cfgs are executed

// mark a palyer weapon for precaching (alias names are ok)
EXPORT void AddPrecacheWeapon(std::string wepName);

EXPORT extern CVoiceGameMgr g_VoiceGameMgr;
EXPORT extern CMultiplayGameMgrHelper g_GameMgrHelper;

#endif		// GAME_H
