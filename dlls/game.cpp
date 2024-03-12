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

cvar_t	displaysoundlist = {"displaysoundlist","0"};

// multiplayer server rules
cvar_t	fragsleft	= {"mp_fragsleft","0", FCVAR_SERVER | FCVAR_UNLOGGED };	  // Don't spam console/log files/users with this changing
cvar_t	timeleft	= {"mp_timeleft","0" , FCVAR_SERVER | FCVAR_UNLOGGED };	  // "      "

// multiplayer server rules
cvar_t	teamplay	= {"mp_teamplay","0", FCVAR_SERVER };
cvar_t	fraglimit	= {"mp_fraglimit","0", FCVAR_SERVER };
cvar_t	timelimit	= {"mp_timelimit","0", FCVAR_SERVER };
cvar_t	friendlyfire= {"mp_friendlyfire","0", FCVAR_SERVER };
cvar_t	falldamage	= {"mp_falldamage","1", FCVAR_SERVER };
cvar_t	weaponstay	= {"mp_weaponstay","0", FCVAR_SERVER };
cvar_t	item_despawn_time = {"mp_itemdespawntime","120", FCVAR_SERVER };
cvar_t	item_repick_time = {"mp_itemrepicktime","10", FCVAR_SERVER };
cvar_t	max_item_drops = {"mp_maxitemdrops","16", FCVAR_SERVER };
cvar_t	forcerespawn= {"mp_forcerespawn","1", FCVAR_SERVER };
cvar_t	flashlight	= {"mp_flashlight","1", FCVAR_SERVER };
cvar_t	aimcrosshair= {"mp_autocrosshair","1", FCVAR_SERVER };
cvar_t	decalfrequency = {"decalfrequency","30", FCVAR_SERVER };
cvar_t	teamlist = {"mp_teamlist","hgrunt;scientist", FCVAR_SERVER };
cvar_t	teamoverride = {"mp_teamoverride","1" };
cvar_t	defaultteam = {"mp_defaultteam","0" };
cvar_t	allowmonsters={"mp_allowmonsters","1", FCVAR_SERVER };
cvar_t	mp_nextmap={"mp_nextmap","", FCVAR_SERVER };
cvar_t	mp_prefer_server_maxspeed={"mp_prefer_server_maxspeed","1", FCVAR_SERVER };
cvar_t	mp_objectboost ={"mp_objectboost","0", FCVAR_SERVER };
cvar_t	mp_respawndelay ={"mp_respawndelay","3", FCVAR_SERVER };
cvar_t	mp_debugmsg ={"mp_debugmsg","0", FCVAR_SERVER };
cvar_t	mp_starthealth ={"starthealth","0", FCVAR_SERVER };
cvar_t	mp_startarmor ={"startarmor","0", FCVAR_SERVER };
cvar_t	mp_bulletsponges ={"mp_bulletsponges","1", FCVAR_SERVER };
cvar_t	mp_bulletspongemax ={"mp_bulletspongemax","4", FCVAR_SERVER };
cvar_t	mp_maxmonsterrespawns ={"mp_maxmonsterrespawns","-1", FCVAR_SERVER };

cvar_t	soundvariety={"mp_soundvariety","0", FCVAR_SERVER };

cvar_t	mp_npckill = { "mp_npckill", "1", FCVAR_SERVER };
cvar_t	killnpc = { "killnpc", "1", FCVAR_SERVER };

cvar_t  allow_spectators = { "allow_spectators", "0.0", FCVAR_SERVER };		// 0 prevents players from being spectators

cvar_t  mp_chattime = {"mp_chattime","10", FCVAR_SERVER };

// Engine Cvars
cvar_t 	*g_psv_gravity = NULL;
cvar_t	*g_psv_aim = NULL;
cvar_t	*g_footsteps = NULL;
cvar_t	*g_developer = NULL;

// END Cvars for Skill Level settings

std::map<std::string, std::string> g_modelReplacementsMod;
std::map<std::string, std::string> g_modelReplacementsMap;
std::map<std::string, std::string> g_modelReplacements;

NerfStats g_nerfStats;
bool g_cfgsExecuted;

void test_command() {
	int entIdx = 1;
	int channel = 3;
	const char* sample = "hlcoop/weapons/glock_reload.wav";
	uint32_t vol = 0x3F4CCCCD;
	uint32_t attn = 0x3F4CCCCD;
	int flags = 0;
	int pitch = 103;
	uint32_t origin[3] = { 0x448A6000, 0xC5442000, 0xC45D3470 };
	Vector ori = Vector(*(float*)&origin[0], *(float*)&origin[1], *(float*)&origin[2]);
	StartSound(INDEXENT(entIdx), channel, sample, *(float*)&vol, *(float*)&attn, flags, pitch, ori, 0xFFFFFFFF);
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
	// Register cvars here:

	g_psv_gravity = CVAR_GET_POINTER( "sv_gravity" );
	g_psv_aim = CVAR_GET_POINTER( "sv_aim" );
	g_footsteps = CVAR_GET_POINTER( "mp_footsteps" );
	g_developer = CVAR_GET_POINTER( "developer" );

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

	CVAR_REGISTER (&mp_chattime);

	RegisterSkillCvars();

// END REGISTER CVARS FOR SKILL LEVEL STUFF

	SERVER_COMMAND( "exec skill.cfg\n" );
}

