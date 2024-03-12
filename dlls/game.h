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

#include <map>
#include <string>
#include <set>

extern void GameDLLInit( void );

// subfolders created to separate mod files from vanilla HL files
#define MOD_SND_FOLDER "hlcoop/"

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
extern cvar_t	*g_footsteps;
extern cvar_t	*g_developer;

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

extern std::map<std::string, std::string> g_modelReplacementsMap; // model replacements for the current map
extern std::map<std::string, std::string> g_modelReplacementsMod; // model replacements for this mod
extern std::map<std::string, std::string> g_modelReplacements; // combined model replacements

// per-monster sound replacement maps
// should be a class member, but I'm afraid of the bugs that will come from using non-POD class members
extern std::vector<std::map<std::string, std::string>> g_monsterSoundReplacements;
extern std::set<std::string> g_shuffledMonsterSounds; // classes that had their sounds shuffled this map

extern bool g_cfgsExecuted; // set to true after server and map cfgs are executed

#endif		// GAME_H
