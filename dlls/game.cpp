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
#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "game.h"
#include "cbase.h"
#include "CBaseMonster.h"
#include "skill.h"

cvar_t	displaysoundlist = {"displaysoundlist","0", 0, 0, 0};

// multiplayer server rules
cvar_t	fragsleft = { "mp_fragsleft","0", FCVAR_SERVER | FCVAR_UNLOGGED, 0, 0 }; // Don't spam console/log files/users with this changing
cvar_t	timeleft = { "mp_timeleft","0" , FCVAR_SERVER | FCVAR_UNLOGGED, 0, 0 }; // "      "

// multiplayer server rules
cvar_t	teamplay	= {"mp_teamplay","0", FCVAR_SERVER, 0, 0 };
cvar_t	fraglimit	= {"mp_fraglimit","0", FCVAR_SERVER, 0, 0 };
cvar_t	timelimit	= {"mp_timelimit","0", FCVAR_SERVER, 0, 0 };
cvar_t	friendlyfire= {"mp_friendlyfire","0", FCVAR_SERVER, 0, 0 };
cvar_t	falldamage	= {"mp_falldamage","1", FCVAR_SERVER, 0, 0 };
cvar_t	weaponstay	= {"mp_weaponstay","0", FCVAR_SERVER, 0, 0 };
cvar_t	item_despawn_time = {"mp_itemdespawntime","120", FCVAR_SERVER, 0, 0 };
cvar_t	item_repick_time = {"mp_itemrepicktime","10", FCVAR_SERVER, 0, 0 };
cvar_t	max_item_drops = {"mp_maxitemdrops","16", FCVAR_SERVER, 0, 0 };
cvar_t	forcerespawn= {"mp_forcerespawn","1", FCVAR_SERVER, 0, 0 };
cvar_t	flashlight	= {"mp_flashlight","1", FCVAR_SERVER, 0, 0 };
cvar_t	aimcrosshair= {"mp_autocrosshair","1", FCVAR_SERVER, 0, 0 };
cvar_t	decalfrequency = {"decalfrequency","30", FCVAR_SERVER, 0, 0 };
cvar_t	teamlist = {"mp_teamlist","hgrunt;scientist", FCVAR_SERVER, 0, 0 };
cvar_t	teamoverride = {"mp_teamoverride","1", 0, 0, 0 };
cvar_t	defaultteam = {"mp_defaultteam","0", 0, 0, 0 };
cvar_t	allowmonsters={"mp_allowmonsters","1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_nextmap={"mp_nextmap","", FCVAR_SERVER, 0, 0 };
cvar_t	mp_prefer_server_maxspeed={"mp_prefer_server_maxspeed","1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_objectboost ={"mp_objectboost","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_respawndelay ={"mp_respawndelay","3", FCVAR_SERVER, 0, 0 };
cvar_t	mp_debugmsg ={"mp_debugmsg","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_starthealth ={"starthealth","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_startarmor ={"startarmor","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_bulletsponges ={"mp_bulletsponges","1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_bulletspongemax ={"mp_bulletspongemax","4", FCVAR_SERVER, 0, 0 };
cvar_t	mp_maxmonsterrespawns ={"mp_maxmonsterrespawns","-1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_edictsorting ={"mp_edictsorting","1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_shitcode ={"mp_shitcode","0", FCVAR_SERVER, 0, 0 };

cvar_t	soundvariety={"mp_soundvariety","0", FCVAR_SERVER, 0, 0 };

cvar_t	mp_npckill = { "mp_npckill", "1", FCVAR_SERVER, 0, 0 };
cvar_t	killnpc = { "killnpc", "1", FCVAR_SERVER, 0, 0 };

cvar_t  allow_spectators = { "allow_spectators", "0.0", FCVAR_SERVER, 0, 0 }; // 0 prevents players from being spectators

cvar_t  mp_chattime = {"mp_chattime","10", FCVAR_SERVER, 0, 0 };

// Engine Cvars
cvar_t 	*g_psv_gravity = NULL;
cvar_t	*g_psv_aim = NULL;
cvar_t	*g_footsteps = NULL;
cvar_t	*g_developer = NULL;
cvar_t	*sv_max_client_edicts = NULL;
cvar_t	*sv_stepsize = NULL;
cvar_t	*sv_lowercase = NULL;

// END Cvars for Skill Level settings

std::map<std::string, std::string> g_modelReplacementsMod;
std::map<std::string, std::string> g_modelReplacementsMap;
std::map<std::string, std::string> g_modelReplacements;

std::map<std::string, std::string> g_soundReplacementsMod;
std::map<std::string, std::string> g_soundReplacementsMap;
std::map<std::string, std::string> g_soundReplacements;

std::set<std::string> g_mapWeapons;

std::map<std::string, const char*> g_itemNameRemap = {
	{"weapon_9mmar", "weapon_9mmAR"},
	{"weapon_mp5", "weapon_9mmAR"},
	{"weapon_uzi", "weapon_9mmAR"},
	{"weapon_uziakimbo", "weapon_9mmAR"},
	{"weapon_m16", "weapon_9mmAR"},
	{"weapon_m249", "weapon_9mmAR"},
	{"weapon_saw", "weapon_9mmAR"},
	{"weapon_minigun", "weapon_9mmAR"},
	{"weapon_pipewrench", "weapon_crowbar"},
	{"weapon_eagle", "weapon_357"},
	{"weapon_python", "weapon_357"},
	{"weapon_sniperrifle", "weapon_crossbow"},
	{"weapon_displacer", "weapon_egon"},
	{"weapon_shockrifle", "weapon_hornetgun"},
	{"weapon_glock", "weapon_9mmhandgun"},

	{"ammo_9mmar", "ammo_9mmAR"},
	{"ammo_mp5clip", "ammo_9mmAR"},
	{"ammo_556clip", "ammo_9mmAR"},
	{"ammo_uziclip", "ammo_9mmAR"},
	{"ammo_556", "ammo_9mmbox"},
	{"ammo_glockclip", "ammo_9mmclip"},
	{"ammo_9mm", "ammo_9mmclip"},
	{"ammo_egonclip", "ammo_gaussclip"},
	{"ammo_mp5grenades", "ammo_ARgrenades"},
	{"ammo_spore", "ammo_ARgrenades"},
	{"ammo_argrenades", "ammo_ARgrenades"},
	{"weapon_sporelauncher", "ammo_ARgrenades"},
	{"ammo_sporeclip", "ammo_ARgrenades"},
	{"ammo_spore", "ammo_ARgrenades"},
	{"ammo_762", "ammo_crossbow"},
};

void AddPrecacheWeapon(std::string wepName) {
	if (g_itemNameRemap.find(wepName) != g_itemNameRemap.end()) {
		g_mapWeapons.insert(g_itemNameRemap[wepName]);
	}
	else {
		g_mapWeapons.insert(wepName);
	}
}

NerfStats g_nerfStats;
TextureTypeStats g_textureStats;
bool g_cfgsExecuted;

void test_command() {
}

void cfg_exec_finished() {
	g_cfgsExecuted = true;
}

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit( void )
{
	g_engfuncs.pfnAddServerCommand("test", test_command);
	g_engfuncs.pfnAddServerCommand("cfg_exec_finished", cfg_exec_finished);
	g_engfuncs.pfnAddServerCommand("edicts", PrintEntindexStats);
	// Register cvars here:

	g_psv_gravity = CVAR_GET_POINTER( "sv_gravity" );
	g_psv_aim = CVAR_GET_POINTER( "sv_aim" );
	g_footsteps = CVAR_GET_POINTER( "mp_footsteps" );
	g_developer = CVAR_GET_POINTER( "developer" );
	sv_max_client_edicts = CVAR_GET_POINTER( "sv_max_client_edicts" );
	sv_stepsize = CVAR_GET_POINTER( "sv_stepsize" );
	sv_lowercase = CVAR_GET_POINTER( "sv_lowercase" );

	CVAR_REGISTER (&displaysoundlist);
	CVAR_REGISTER( &allow_spectators );

	CVAR_REGISTER (&teamplay);
	CVAR_REGISTER (&fraglimit);
	CVAR_REGISTER (&timelimit);

	CVAR_REGISTER (&fragsleft);
	CVAR_REGISTER (&timeleft);

	CVAR_REGISTER (&friendlyfire);
	CVAR_REGISTER (&falldamage);
	CVAR_REGISTER (&weaponstay);
	CVAR_REGISTER (&item_despawn_time);
	CVAR_REGISTER (&item_repick_time);
	CVAR_REGISTER (&max_item_drops);
	CVAR_REGISTER (&forcerespawn);
	CVAR_REGISTER (&flashlight);
	CVAR_REGISTER (&aimcrosshair);
	CVAR_REGISTER (&decalfrequency);
	CVAR_REGISTER (&teamlist);
	CVAR_REGISTER (&teamoverride);
	CVAR_REGISTER (&defaultteam);
	CVAR_REGISTER (&allowmonsters);
	CVAR_REGISTER (&soundvariety);
	CVAR_REGISTER (&mp_npckill);
	CVAR_REGISTER (&killnpc);
	CVAR_REGISTER (&mp_nextmap);
	CVAR_REGISTER (&mp_prefer_server_maxspeed);
	CVAR_REGISTER (&mp_objectboost);
	CVAR_REGISTER (&mp_respawndelay);
	CVAR_REGISTER (&mp_debugmsg);
	CVAR_REGISTER (&mp_starthealth);
	CVAR_REGISTER (&mp_startarmor);
	CVAR_REGISTER (&mp_bulletsponges);
	CVAR_REGISTER (&mp_bulletspongemax);
	CVAR_REGISTER (&mp_maxmonsterrespawns);
	CVAR_REGISTER (&mp_edictsorting);
	CVAR_REGISTER (&mp_shitcode);

	CVAR_REGISTER (&mp_chattime);

	RegisterSkillCvars();

// END REGISTER CVARS FOR SKILL LEVEL STUFF

	SERVER_COMMAND( "exec skill.cfg\n" );
}

