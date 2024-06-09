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
//=========================================================
// GameRules.cpp
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"CBasePlayer.h"
#include	"gamerules.h"
#include	"teamplay_gamerules.h"
#include	"skill.h"
#include	"game.h"
#include "CGamePlayerEquip.h"
#include "CBasePlayerItem.h"
#include "PluginManager.h"

#include <sstream>
#include <string>
#include <fstream>
#include <set>
#include <algorithm>

using namespace std;

extern edict_t *EntSelectSpawnPoint( CBaseEntity *pPlayer );

DLL_GLOBAL CGameRules*	g_pGameRules = NULL;
extern DLL_GLOBAL BOOL	g_fGameOver;
extern int gmsgDeathMsg;	// client dll messages
extern int gmsgMOTD;

int g_teamplay = 0;

//=========================================================
//=========================================================
BOOL CGameRules::CanHaveAmmo( CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry )
{
	int iAmmoIndex;

	if ( pszAmmoName )
	{
		iAmmoIndex = pPlayer->GetAmmoIndex( pszAmmoName );

		if ( iAmmoIndex > -1 )
		{
			if ( pPlayer->AmmoInventory( iAmmoIndex ) < iMaxCarry )
			{
				// player has room for more of this type of ammo
				return TRUE;
			}
		}
	}

	return FALSE;
}

//=========================================================
//=========================================================
edict_t *CGameRules :: GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	return EntSelectSpawnPoint( pPlayer );
}

//=========================================================
//=========================================================
BOOL CGameRules::CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
	// only living players can have items
	if ( pPlayer->pev->deadflag != DEAD_NO )
		return FALSE;

	if ( pWeapon->pszAmmo1() )
	{
		if ( !CanHaveAmmo( pPlayer, pWeapon->pszAmmo1(), pWeapon->iMaxAmmo1() ) )
		{
			// we can't carry anymore ammo for this gun. We can only 
			// have the gun if we aren't already carrying one of this type
			if ( pPlayer->HasPlayerItem( pWeapon ) )
			{
				return FALSE;
			}
		}
	}
	else
	{
		// weapon doesn't use ammo, don't take another if you already have it.
		if ( pPlayer->HasPlayerItem( pWeapon ) )
		{
			return FALSE;
		}
	}

	// note: will fall through to here if GetItemInfo doesn't fill the struct!
	return TRUE;
}

void execMapCfg() {
	// Map CFGs are low trust so only whitelisted commands are allowed.
	// Server owners shouldn't have to check each map for things like "rcon_password HAHA_GOT_YOU"

	static set<string> whitelistCommands = {
		"sv_gravity",
		"sv_friction",
		"sv_accelerate",
		"sv_airaccelerate",
		"sv_bounce",
		"sv_maxspeed",
		"sv_maxvelocity",
		"sv_newunit",
		"sv_rollspeed",
		"sv_stepsize",
		"sv_stopspeed",
		"sv_wateraccelerate",
		"sv_wateramp",
		"sv_waterfriction",
		"mp_decals",
		"mp_falldamage",
		"mp_flashlight",
		"mp_footsteps",
		"mp_forcerespawn",
		"mp_fraglimit",
		"mp_timelimit",
		"mp_weaponstay",
		"mp_friendlyfire",
		"mp_soundvariety",
		"mp_bulletsponges",
		"mp_bulletspongemax",
		"mp_maxmonsterrespawns",
		"killnpc",
		"mp_npckill",
		"startarmor",
		"starthealth",
		"globalmodellist",
		"globalsoundlist",
		"mp_shitcode",
		"map_plugin",
	};

	static set<string> itemNames = {
		"weapon_crossbow",
		"weapon_crowbar",
		"weapon_egon",
		"weapon_gauss",
		"weapon_handgrenade",
		"weapon_hornetgun",
		"weapon_mp5",
		"weapon_9mmar",
		"weapon_python",
		"weapon_357",
		"weapon_rpg",
		"weapon_satchel",
		"weapon_shotgun",
		"weapon_snark",
		"weapon_tripmine",
		"weapon_glock",
		"weapon_9mmhandgun",
		"weapon_uzi",
		"weapon_uziakimbo",
		"weapon_m16",
		"weapon_m249",
		"weapon_saw",
		"weapon_pipewrench",
		"weapon_minigun",
		"weapon_grapple",
		"weapon_medkit",
		"weapon_eagle",
		"weapon_sniperrifle",
		"weapon_displacer",
		"weapon_shockrifle",

		"ammo_crossbow",
		"ammo_egonclip",
		"ammo_gaussclip",
		"ammo_mp5clip",
		"ammo_9mmar",
		"ammo_9mmbox",
		"ammo_mp5grenades",
		"ammo_argrenades",
		"ammo_357",
		"ammo_rpgclip",
		"ammo_buckshot",
		"ammo_glockclip",
		"ammo_9mm",
		"ammo_9mmclip",
		"ammo_556",
		"ammo_556clip",
		"ammo_762",
		"ammo_uziclip",
		"ammo_spore",
		"ammo_sporeclip",

		"item_longjump",
	};

	memset(g_mapEquipment, 0, sizeof(EquipItem) * MAX_EQUIP);

	string cfgPath = "maps/" + string(STRING(gpGlobals->mapname)) + ".cfg";
	int length;
	char* cfgFile = (char*)LOAD_FILE_FOR_ME(cfgPath.c_str(), &length);
	
	g_mapCfgExists = cfgFile;

	if (!cfgFile) {
		// precache default equipment
		AddPrecacheWeapon("weapon_crowbar");
		AddPrecacheWeapon("weapon_9mmhandgun");
		return;
	}

	std::stringstream data_stream(cfgFile);
	string line;

	int equipIdx = 0;

	while (std::getline(data_stream, line))
	{
		vector<string> parts = splitString(line, " \t");

		if (parts.empty()) {
			continue;
		}

		string name = trimSpaces(toLowerCase(parts[0]));
		string value = sanitize_cvar_value(parts.size() > 1 ? trimSpaces(parts[1]) : "");

		if (parts.size() > 1 && whitelistCommands.find(name) != whitelistCommands.end()) {
			if (mp_prefer_server_maxspeed.value == 1 && name == "sv_maxspeed") {
				int maxspeed = atoi(value.c_str());
				if (maxspeed == 270 || maxspeed == 320) { // default speeds for Half-Life (320) and Sven Co-op (270)
					ALERT(at_console, "mp_prefer_server_maxspeed: Ignoring \"sv_maxspeed %d\" set by map cfg.\n", maxspeed);
					continue;
				}
			}

			// model/sound lists must be loaded now or else other entities might precache the wrong files
			if (name == "globalmodellist" || name == "globalsoundlist") {
				KeyValueData dat;
				dat.fHandled = false;
				dat.szClassName = (char*)"worldspawn";
				dat.szKeyName = (char*)name.c_str();
				dat.szValue = (char*)value.c_str();
				DispatchKeyValue(ENT(0), &dat);
				continue;
			}

			// map plugins need to be loaded now in case they define custom entities used in the bsp data
			if (name == "map_plugin") {
				g_pluginManager.AddPlugin(value.c_str(), true);
				continue;
			}

			SERVER_COMMAND(UTIL_VarArgs("%s %s\n", name.c_str(), value.c_str()));
		}
		else if (itemNames.find(name) != itemNames.end()) {
			if (equipIdx >= MAX_EQUIP) {
				ALERT(at_error, "Failed to add equipment '%s'. Max equipment reached.\n", line.c_str());
				continue;
			}

			g_mapEquipment[equipIdx].itemName = ALLOC_STRING(name.c_str());
			g_mapEquipment[equipIdx].count = value.size() ? atoi(value.c_str()) : 1;

			AddPrecacheWeapon(name);

			equipIdx++;
		}
	}

	FREE_FILE(cfgFile);
}

void execServerCfg() {
	int length;
	char* cfgFile = (char*)LOAD_FILE_FOR_ME("server.cfg", &length);

	if (!cfgFile) {
		return;
	}

	std::stringstream data_stream(cfgFile);
	string line;

	// not just doing "exec server.cfg" so that commands remain in order after parsing other CFGs.
	while (std::getline(data_stream, line)) {
		line = trimSpaces(line);
		if (line.empty() || line[0] == '/') {
			continue;
		}

		SERVER_COMMAND(UTIL_VarArgs("%s\n", line.c_str()));
	}

	FREE_FILE(cfgFile);
}

void execCfgs() {
	// CFG execution must be done without mixing "exec asdf.cfg" and individual commands
	// so that commands are executed in the correct order (map.cfg after server.cfg).
	// The engine automatically exec's the "servercfgfile" cfg on the first server start,
	// throwing another wrench into this system. Disable that with '+servercfg ""' on the
	// server command line. The server CFG must run every map change to reset vars from the
	// previous map, and respond to changes without a hard restart.
	// 
	// How do commands run out of order?
	// Whenever "exec file.cfg" is called, it's command are moved to the back of the command list,
	// meaning the order you call SERVER_COMMAND doesn't match the actual execution order. Commands
	// are run one per frame, so you can't hackfix this by adding a delay to an important CFG either.

	execServerCfg();
	execMapCfg();
	SERVER_COMMAND("cfg_exec_finished\n");

	SERVER_EXECUTE();
}

//=========================================================
// instantiate the proper game rules object
//=========================================================

CGameRules *InstallGameRules( void )
{
	execCfgs();

	if ( !gpGlobals->deathmatch )
	{
		// generic half-life
		g_teamplay = 0;
		return new CHalfLifeRules;
	}
	else
	{
		if ( teamplay.value > 0 )
		{
			// teamplay

			g_teamplay = 1;
			return new CHalfLifeTeamplay;
		}
		if ((int)gpGlobals->deathmatch == 1)
		{
			// vanilla deathmatch
			g_teamplay = 0;
			return new CHalfLifeMultiplay;
		}
		else
		{
			// vanilla deathmatch??
			g_teamplay = 0;
			return new CHalfLifeMultiplay;
		}
	}
}

