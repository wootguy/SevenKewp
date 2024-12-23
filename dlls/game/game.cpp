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
#include "rehlds.h"
#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "game.h"
#include "CBaseMonster.h"
#include "skill.h"
#include "PluginManager.h"
#include "user_messages.h"

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
cvar_t	mp_explosionbug ={"mp_explosionbug","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_respawndelay ={"mp_respawndelay","3", FCVAR_SERVER, 0, 0 };
cvar_t	mp_debugmsg ={"mp_debugmsg","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_starthealth ={"starthealth","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_startarmor ={"startarmor","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_bulletsponges ={"mp_bulletsponges","1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_bulletspongemax ={"mp_bulletspongemax","4", FCVAR_SERVER, 0, 0 };
cvar_t	mp_maxmonsterrespawns ={"mp_maxmonsterrespawns","-1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_edictsorting ={"mp_edictsorting","1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_shitcode ={"mp_shitcode","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_mergemodels ={"mp_mergemodels","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_killfeed ={"mp_killfeed","1", FCVAR_SERVER, 0, 0 };
cvar_t	pluginlistfile ={"pluginlistfile","plugins.txt", FCVAR_SERVER, 0, 0 };
cvar_t	adminlistfile ={"adminlistfile","admins.txt", FCVAR_SERVER, 0, 0 };
cvar_t	pluginupdatepath ={"plugin_update_path","valve_pending/", FCVAR_SERVER, 0, 0 };
cvar_t	pluginautoupdate ={"plugin_auto_update", "0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_skill_allow ={"mp_skill_allow", "1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_default_medkit ={"mp_default_medkit", "0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_rpg_laser_mode ={"mp_rpg_laser_mode", "1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_series_intermission ={"mp_series_intermission", "2", FCVAR_SERVER, 0, 0 };
cvar_t	mp_score_mode ={"mp_score_mode", "0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_damage_points ={"mp_damage_points", "0.01", FCVAR_SERVER, 0, 0 };
cvar_t	mp_antiblock ={"mp_antiblock", "1", FCVAR_SERVER, 0, 0 };
cvar_t	mp_antiblock_cooldown ={"mp_antiblock_cooldown", "3", FCVAR_SERVER, 0, 0 };
cvar_t	mp_min_score_mult ={"mp_min_score_mult", "20", FCVAR_SERVER, 0, 0 };
cvar_t	mp_hevsuit_voice ={"mp_hevsuit_voice", "0", FCVAR_SERVER, 0, 0 };

cvar_t	soundvariety={"mp_soundvariety","0", FCVAR_SERVER, 0, 0 };
cvar_t	mp_npcidletalk={"mp_npcidletalk","1", FCVAR_SERVER, 0, 0 };

cvar_t	mp_npckill = { "mp_npckill", "1", FCVAR_SERVER, 0, 0 };
cvar_t	killnpc = { "killnpc", "1", FCVAR_SERVER, 0, 0 };

cvar_t  allow_spectators = { "allow_spectators", "0.0", FCVAR_SERVER, 0, 0 }; // 0 prevents players from being spectators

cvar_t  mp_chattime = {"mp_chattime","10", FCVAR_SERVER, 0, 0 };

cvar_t  mp_survival_supported = { "mp_survival_supported", "0", FCVAR_SERVER, 0, 0 };
cvar_t  mp_survival_starton = { "mp_survival_starton", "0", FCVAR_SERVER, 0, 0 };
cvar_t  mp_survival_restart = { "mp_survival_restart", "0", FCVAR_SERVER, 0, 0 };

// Engine Cvars
cvar_t 	*g_psv_gravity = NULL;
cvar_t	*g_psv_aim = NULL;
cvar_t* g_psv_allow_autoaim = NULL;
cvar_t	*g_footsteps = NULL;
cvar_t	*g_developer = NULL;
cvar_t	*sv_max_client_edicts = NULL;
cvar_t	*sv_voiceenable = NULL;
cvar_t	*sv_stepsize = NULL;
cvar_t	*sv_friction = NULL;
cvar_t	*sv_stopspeed = NULL;
cvar_t	*sv_maxspeed = NULL;
cvar_t	*sv_lowercase = NULL;

// END Cvars for Skill Level settings

std::unordered_map<std::string, std::string> g_modelReplacementsMod;
std::unordered_map<std::string, std::string> g_modelReplacementsMap;
std::unordered_map<std::string, std::string> g_modelReplacements;

std::unordered_map<std::string, std::string> g_soundReplacementsMod;
std::unordered_map<std::string, std::string> g_soundReplacementsMap;
std::unordered_map<std::string, std::string> g_soundReplacements;

std::unordered_set<std::string> g_mapWeapons;

std::unordered_map<uint64_t, player_score_t> g_playerScores;
std::unordered_map<uint64_t, player_score_t> g_oldPlayerScores;

std::unordered_map<std::string, const char*> g_itemNameRemap = {
	{"weapon_9mmar", "weapon_9mmAR"},
	{"weapon_mp5", "weapon_9mmAR"},
	{"weapon_uzi", "weapon_9mmAR"},
	{"weapon_uziakimbo", "weapon_9mmAR"},
	{"weapon_m16", "weapon_9mmAR"},
	{"weapon_m249", "weapon_9mmAR"},
	{"weapon_saw", "weapon_9mmAR"},
	{"weapon_minigun", "weapon_9mmAR"},
	{"weapon_eagle", "weapon_357"},
	{"weapon_python", "weapon_357"},
	{"weapon_sniperrifle", "weapon_crossbow"},
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
	{"ammo_argrenades", "ammo_ARgrenades"},
	{"ammo_762", "ammo_crossbow"},

	// keyvalues that should be ignored
	{"equipmode", "<keyvalue>"},
	{"delay", "<keyvalue>"},
	{"inventorymode", "<keyvalue>"},
	{"master", "<keyvalue>"},
	{"killtarget", "<keyvalue>"},
	{"ondestroyfn", "<keyvalue>"},
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

#include <algorithm>
#include <fstream>

void dump_missing_files() {
	bool dumpMissing = !strcmp(CMD_ARGV(0), "dmiss");

	std::vector<std::string> resList;

	std::unordered_set<std::string> allPrecacheFiles;
	allPrecacheFiles.insert(g_tryPrecacheModels.begin(), g_tryPrecacheModels.end());
	allPrecacheFiles.insert(g_missingModels.begin(), g_missingModels.end());
	allPrecacheFiles.insert(g_tryPrecacheGeneric.begin(), g_tryPrecacheGeneric.end());
	allPrecacheFiles.insert(g_tryPrecacheEvents.begin(), g_tryPrecacheEvents.end());

	for (std::string item : g_tryPrecacheSounds) {
		if (item.size() > 1) {
			if (item[0] == '*' || item[0] == '!') {
				item = item.substr(1); // client will ignore this character and load the path after this
			}
		}

		allPrecacheFiles.insert("sound/" + item);
	}

	for (std::string item : allPrecacheFiles) {
		std::string lowerItem = normalize_path(toLowerCase(item));

		if (getGameFilePath(lowerItem.c_str()).empty() == dumpMissing) {
			resList.push_back(lowerItem);
		}
	}

	if (resList.empty()) {
		g_engfuncs.pfnServerPrint(dumpMissing ? "No missing files\n" : "No precached files\n");
		return;
	}

	sort(resList.begin(), resList.end());

	std::ofstream resfile;
	const char* suffix = dumpMissing ? ".miss" : ".res";
	std::string fname = std::string("res/") + STRING(gpGlobals->mapname) + suffix;
	resfile.open(fname, std::ios_base::trunc);

	if (!resfile.is_open()) {
		g_engfuncs.pfnServerPrint("Failed to open file in res/ folder (does it exist?)\n");
		return;
	}

	for (int i = 0; i < (int)resList.size(); i++) {
		std::string& item = resList[i];

		if (i < 10)
			g_engfuncs.pfnServerPrint(UTIL_VarArgs("Missing: %s\n", item.c_str()));

		resfile << item + "\n";
	}

	if (resList.size() > 10) {
		g_engfuncs.pfnServerPrint(UTIL_VarArgs("%d more items...\n", resList.size() - 10));
	}

	resfile.close();

	g_engfuncs.pfnServerPrint(UTIL_VarArgs("Wrote %d %s files to %s\n",
		(int)resList.size(), dumpMissing ? "missing" : "precached", fname.c_str()));
}

void reload_plugins() {
	g_pluginManager.ReloadPlugins();
}

void list_plugins() {
	g_pluginManager.ListPlugins(NULL);
}

void remove_plugin() {
	if (CMD_ARGC() < 2) {
		return;
	}

	g_pluginManager.RemovePlugin(CMD_ARGV(1));
}

void reload_plugin() {
	if (CMD_ARGC() < 2) {
		return;
	}

	g_pluginManager.ReloadPlugin(CMD_ARGV(1));
}

void update_plugin() {
	if (CMD_ARGC() < 2) {
		return;
	}

	if (g_pluginManager.UpdatePlugin(CMD_ARGV(1))) {
		g_engfuncs.pfnServerPrint("Plugin updated\n");
	}
}

void update_plugins() {
	g_pluginManager.UpdatePluginsFromList();

	g_engfuncs.pfnServerPrint(UTIL_VarArgs("Searching update path \"%s\"\n", pluginupdatepath.string));
	if (!g_pluginManager.UpdatePlugins()) {
		g_engfuncs.pfnServerPrint("Plugins are up-to-date\n");
	}
}

void freespace_command() {
	std::string path = CMD_ARGC() > 1 ? CMD_ARGS() : "";

	if (path.empty()) {
		static char gameDir[MAX_PATH];
		GET_GAME_DIR(gameDir);

		path = gameDir;
	}

	uint64_t bytes = getFreeSpace(path);
	uint32_t gb = bytes / (1024ULL * 1024ULL * 1024ULL);

	ALERT(at_console, "Free space at %s is %.2f GB\n", path.c_str(), (float)gb);
}

void list_precached_sounds() {
	std::vector<std::string> allSounds;

	for (std::string item : g_precachedSounds) {
		allSounds.push_back(item);
	}

	sort(allSounds.begin(), allSounds.end());

	for (std::string item : allSounds) {
		g_engfuncs.pfnServerPrint(UTIL_VarArgs("    %s\n", item.c_str()));
	}
}

void test_command() {
	int id = GetUserMsgInfo("VoiceMask", NULL);

	MESSAGE_BEGIN(MSG_ALL, id);
	int dw;
	for (dw = 0; dw < VOICE_MAX_PLAYERS_DW; dw++)
	{
		WRITE_LONG(0);
		WRITE_LONG(0);
	}
	MESSAGE_END();
}

void cfg_exec_finished() {
	g_cfgsExecuted = true;
}

// Register your console variables here
// This gets called one time when the game is initialied
void GameDLLInit( void )
{
	g_engfuncs.pfnAddServerCommand("test", test_command);
	g_engfuncs.pfnAddServerCommand("dcache", dump_missing_files);
	g_engfuncs.pfnAddServerCommand("dmiss", dump_missing_files);
	g_engfuncs.pfnAddServerCommand("cfg_exec_finished", cfg_exec_finished);
	g_engfuncs.pfnAddServerCommand("edicts", PrintEntindexStats);
	g_engfuncs.pfnAddServerCommand("reloadplugins", reload_plugins);
	g_engfuncs.pfnAddServerCommand("listplugins", list_plugins);
	g_engfuncs.pfnAddServerCommand("removeplugin", remove_plugin);
	g_engfuncs.pfnAddServerCommand("reloadplugin", reload_plugin);
	g_engfuncs.pfnAddServerCommand("updateplugin", update_plugin);
	g_engfuncs.pfnAddServerCommand("updateplugins", update_plugins);
	g_engfuncs.pfnAddServerCommand("freespace", freespace_command);
	g_engfuncs.pfnAddServerCommand("listsounds", list_precached_sounds);
	
	// Register cvars here:
	g_psv_gravity = CVAR_GET_POINTER( "sv_gravity" );
	g_psv_aim = CVAR_GET_POINTER( "sv_aim" );
	g_psv_allow_autoaim = CVAR_GET_POINTER("sv_allow_autoaim");
	g_footsteps = CVAR_GET_POINTER( "mp_footsteps" );
	g_developer = CVAR_GET_POINTER( "developer" );
	sv_max_client_edicts = CVAR_GET_POINTER( "sv_max_client_edicts" );
	sv_voiceenable = CVAR_GET_POINTER( "sv_voiceenable" );
	sv_stepsize = CVAR_GET_POINTER( "sv_stepsize" );
	sv_friction = CVAR_GET_POINTER( "sv_friction" );
	sv_stopspeed = CVAR_GET_POINTER( "sv_stopspeed" );
	sv_maxspeed = CVAR_GET_POINTER( "sv_maxspeed" );
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
	CVAR_REGISTER (&mp_explosionbug);
	CVAR_REGISTER (&mp_respawndelay);
	CVAR_REGISTER (&mp_debugmsg);
	CVAR_REGISTER (&mp_starthealth);
	CVAR_REGISTER (&mp_startarmor);
	CVAR_REGISTER (&mp_bulletsponges);
	CVAR_REGISTER (&mp_bulletspongemax);
	CVAR_REGISTER (&mp_maxmonsterrespawns);
	CVAR_REGISTER (&mp_edictsorting);
	CVAR_REGISTER (&mp_shitcode);
	CVAR_REGISTER (&mp_mergemodels);
	CVAR_REGISTER (&mp_killfeed);
	CVAR_REGISTER (&pluginlistfile);
	CVAR_REGISTER (&adminlistfile);
	CVAR_REGISTER (&pluginupdatepath);
	CVAR_REGISTER (&pluginautoupdate);
	CVAR_REGISTER (&mp_skill_allow);
	CVAR_REGISTER (&mp_default_medkit);
	CVAR_REGISTER (&mp_rpg_laser_mode);
	CVAR_REGISTER (&mp_npcidletalk);
	CVAR_REGISTER (&mp_series_intermission);
	CVAR_REGISTER (&mp_score_mode);
	CVAR_REGISTER (&mp_damage_points);
	CVAR_REGISTER (&mp_antiblock);
	CVAR_REGISTER (&mp_antiblock_cooldown);
	CVAR_REGISTER (&mp_min_score_mult);
	CVAR_REGISTER (&mp_hevsuit_voice);

	CVAR_REGISTER (&mp_chattime);

	CVAR_REGISTER (&mp_survival_supported);
	CVAR_REGISTER (&mp_survival_starton);
	CVAR_REGISTER (&mp_survival_restart);

	RegisterSkillCvars();

// END REGISTER CVARS FOR SKILL LEVEL STUFF

	SERVER_COMMAND( "exec skill.cfg\n" );

	if (IS_DEDICATED_SERVER()) {
		RehldsApi_Init();
		RegisterRehldsHooks();
	}
}

