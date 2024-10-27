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
#include <map>
#include <string>
#include <set>

extern void GameDLLInit( void );

// subfolders created to separate mod files from vanilla HL files
#define MOD_MODEL_FOLDER "hlcoop_v2/"
#define MOD_SPRITE_FOLDER "hlcoop/"

#define DEFAULT_TEAM_NAME "Team" // name seen in the scoreboard
#define DEFAULT_TEAM_COLOR 1
#define ENEMY_TEAM_COLOR 2
#define NEUTRAL_TEAM_COLOR 3
#define FRIEND_TEAM_COLOR 4
#define OBSERVER_TEAM_COLOR 6 // spectator color (white on windows, still blue on linux?)

extern cvar_t	displaysoundlist;

// multiplayer server rules
extern cvar_t	teamplay;
extern cvar_t	fraglimit;
extern cvar_t	timelimit;
extern cvar_t	friendlyfire;
extern cvar_t	falldamage;
extern cvar_t	weaponstay;
extern cvar_t	item_despawn_time;
extern cvar_t	item_repick_time;
extern cvar_t	max_item_drops;
extern cvar_t	forcerespawn;
extern cvar_t	flashlight;
extern cvar_t	aimcrosshair;
extern cvar_t	decalfrequency;
extern cvar_t	teamlist;
extern cvar_t	teamoverride;
extern cvar_t	defaultteam;
extern cvar_t	allowmonsters;
extern cvar_t	mp_nextmap; // map which will load after the next intermission
extern cvar_t	mp_starthealth;
extern cvar_t	mp_startarmor;
extern cvar_t	mp_bulletsponges; // prevent mappers setting high npc health for no good reason
extern cvar_t	mp_bulletspongemax; // max health multiplier for any monster
extern cvar_t	mp_maxmonsterrespawns; // limit monster respawns which affect no entity logic
extern cvar_t	mp_edictsorting; // sorts edict list by entity index priority
extern cvar_t	mp_shitcode; // conditionally enables shitty code that fixes critical problems in specific maps, but subtly breaks many others
extern cvar_t	mp_survival_supported;
extern cvar_t	mp_survival_starton;
extern cvar_t	mp_survival_restart;
extern cvar_t	mp_mergemodels; // used merged models to save on model slots
extern cvar_t	mp_killfeed; // 0 = off, 1 = player deaths, 2 = player kills/deaths, 3 = player + monster kills/deaths
extern cvar_t	pluginlist; // name of the plugin list file

// Enables classic func_pushable physics (which is horribly broken, but fun)
// The higher your FPS, the faster you can boost pushables. You also get boosted.
extern cvar_t	mp_objectboost;

// write network messages to log file for debugging
extern cvar_t	mp_debugmsg;

extern cvar_t	mp_respawndelay;

// if a map cfg asks for a default max speed (320 for Half-Life, 270 for Sven Co-op),
// then ignore the command and use whatever was set up by the server.cfg
extern cvar_t	mp_prefer_server_maxspeed;

// limits monster sound variety to save precache slots.
// 0 disables. 1+ = max sounds per action (death/pain/idle/etc.)
extern cvar_t	soundvariety;

extern cvar_t	mp_npckill;
extern cvar_t	killnpc; // legacy setting. When set to 0, makes scientists and barneys invulnerable

// Engine Cvars
extern cvar_t	*g_psv_gravity;
extern cvar_t	*g_psv_aim;
extern cvar_t	*g_psv_allow_autoaim;
extern cvar_t	*g_footsteps;
extern cvar_t	*g_developer;
extern cvar_t	*sv_max_client_edicts;
extern cvar_t	*sv_stepsize;
extern cvar_t	*sv_lowercase;

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

extern NerfStats g_nerfStats;

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

extern TextureTypeStats g_textureStats;

extern std::map<std::string, std::string> g_modelReplacementsMap; // model replacements for the current map
extern std::map<std::string, std::string> g_modelReplacementsMod; // model replacements for this mod
extern std::map<std::string, std::string> g_modelReplacements; // combined model replacements

extern std::map<std::string, std::string> g_soundReplacementsMod; // sound replacements for the current map
extern std::map<std::string, std::string> g_soundReplacementsMap; // sound replacements for this mod
extern std::map<std::string, std::string> g_soundReplacements; // combined sound replacements

extern std::set<std::string> g_mapWeapons; // weapons which should be precached (don't use aliases here)
extern std::map<std::string, const char*> g_itemNameRemap;

// per-monster sound replacement maps
// should be a class member, but I'm afraid of the bugs that will come from using non-POD class members
extern std::vector<std::map<std::string, std::string>> g_monsterSoundReplacements;

// map for each entity, containing custom keyvalues
// using a global vector instead of a class member because the map is not POD
extern std::vector<std::map<std::string, CKeyValue>> g_customKeyValues;

extern CKeyValue g_emptyKeyValue; // a keyvalue initialized with zeroes

extern std::set<std::string> g_shuffledMonsterSounds; // classes that had their sounds shuffled this map

extern bool g_cfgsExecuted; // set to true after server and map cfgs are executed

// mark a palyer weapon for precaching (alias names are ok)
void AddPrecacheWeapon(std::string wepName);

#endif		// GAME_H
