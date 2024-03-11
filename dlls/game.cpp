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

//CVARS FOR SKILL LEVEL SETTINGS
// Agrunt
cvar_t	sk_agrunt_health = {"sk_agrunt_health","0"};
cvar_t	sk_agrunt_dmg_punch = {"sk_agrunt_dmg_punch","0"};

// Apache
cvar_t	sk_apache_health = {"sk_apache_health","0"};

// Osprey
cvar_t	sk_osprey_health = {"sk_osprey_health","0"};

// Barney
cvar_t	sk_barney_health = {"sk_barney_health","0"};

// Bullsquid
cvar_t	sk_bullsquid_health = {"sk_bullsquid_health","0"};
cvar_t	sk_bullsquid_dmg_bite = {"sk_bullsquid_dmg_bite","0"};
cvar_t	sk_bullsquid_dmg_whip = {"sk_bullsquid_dmg_whip","0"};
cvar_t	sk_bullsquid_dmg_spit = {"sk_bullsquid_dmg_spit","0"};


// Big Momma
cvar_t	sk_bigmomma_health = {"sk_bigmomma_health","1.0"};
cvar_t	sk_bigmomma_dmg_slash = {"sk_bigmomma_dmg_slash","50"};
cvar_t	sk_bigmomma_dmg_blast = {"sk_bigmomma_dmg_blast","100"};
cvar_t	sk_bigmomma_radius_blast = {"sk_bigmomma_radius_blast","250"};

// Gargantua
cvar_t	sk_gargantua_health = {"sk_gargantua_health","0"};
cvar_t	sk_gargantua_dmg_slash = {"sk_gargantua_dmg_slash","0"};
cvar_t	sk_gargantua_dmg_fire = {"sk_gargantua_dmg_fire","0"};
cvar_t	sk_gargantua_dmg_stomp = {"sk_gargantua_dmg_stomp","0"};

// Babygarg
cvar_t	sk_babygarg_health = {"sk_babygarg_health","0"};


// Hassassin
cvar_t	sk_hassassin_health = {"sk_hassassin_health","0"};


// Headcrab
cvar_t	sk_headcrab_health = {"sk_headcrab_health","0"};
cvar_t	sk_headcrab_dmg_bite = {"sk_headcrab_dmg_bite","0"};


// Hgrunt 
cvar_t	sk_hgrunt_health = {"sk_hgrunt_health","0"};
cvar_t	sk_hgrunt_kick = {"sk_hgrunt_kick","0"};
cvar_t	sk_hgrunt_pellets = {"sk_hgrunt_pellets","0"};
cvar_t	sk_hgrunt_gspeed = {"sk_hgrunt_gspeed","0"};

// heavy weapons grunt
cvar_t	sk_hwgrunt_health = {"sk_hwgrunt_health","0"};

// Houndeye
cvar_t	sk_houndeye_health = {"sk_houndeye_health","0"};
cvar_t	sk_houndeye_dmg_blast = {"sk_houndeye_dmg_blast","0"};


// ISlave
cvar_t	sk_islave_health = {"sk_islave_health","0"};
cvar_t	sk_islave_dmg_claw = {"sk_islave_dmg_claw","0"};
cvar_t	sk_islave_dmg_clawrake1	= {"sk_islave_dmg_clawrake","0"};
cvar_t	sk_islave_dmg_zap = {"sk_islave_dmg_zap","0"};


// Icthyosaur
cvar_t	sk_ichthyosaur_health = {"sk_ichthyosaur_health","0"};
cvar_t	sk_ichthyosaur_shake = {"sk_ichthyosaur_shake","0"};


// Leech
cvar_t	sk_leech_health = {"sk_leech_health","0"};
cvar_t	sk_leech_dmg_bite = {"sk_leech_dmg_bite","0"};

// Controller
cvar_t	sk_controller_health = {"sk_controller_health","0"};
cvar_t	sk_controller_dmgzap = {"sk_controller_dmgzap","0"};
cvar_t	sk_controller_speedball = {"sk_controller_speedball","0"};
cvar_t	sk_controller_dmgball = {"sk_controller_dmgball","0"};

// Nihilanth
cvar_t	sk_nihilanth_health = {"sk_nihilanth_health","0"};
cvar_t	sk_nihilanth_zap = {"sk_nihilanth_zap","0"};

// Scientist
cvar_t	sk_scientist_health = {"sk_scientist_health","0"};


// Snark
cvar_t	sk_snark_health = {"sk_snark_health","0"};
cvar_t	sk_snark_dmg_bite = {"sk_snark_dmg_bite","0"};
cvar_t	sk_snark_dmg_pop = {"sk_snark_dmg_pop","0"};



// Zombie
cvar_t	sk_zombie_health = {"sk_zombie_health","0"};
cvar_t	sk_zombie_dmg_one_slash = {"sk_zombie_dmg_one_slash","0"};
cvar_t	sk_zombie_dmg_both_slash = {"sk_zombie_dmg_both_slash","0"};


//Turret
cvar_t	sk_turret_health = {"sk_turret_health","0"};


// MiniTurret
cvar_t	sk_miniturret_health = {"sk_miniturret_health","0"};


// Sentry Turret
cvar_t	sk_sentry_health = {"sk_sentry_health","0"};


// PLAYER WEAPONS
cvar_t	sk_plr_crowbar = {"sk_plr_crowbar","0"};
cvar_t	sk_plr_9mm_bullet = {"sk_plr_9mm_bullet","0"};
cvar_t	sk_plr_357_bullet = {"sk_plr_357_bullet","0"};
cvar_t	sk_plr_9mmAR_bullet = {"sk_plr_9mmAR_bullet","0"};
cvar_t	sk_plr_9mmAR_grenade = {"sk_plr_9mmAR_grenade","0"};
cvar_t	sk_plr_buckshot = {"sk_plr_buckshot","0"};
cvar_t	sk_plr_xbow_bolt_client = {"sk_plr_xbow_bolt_client","0"};
cvar_t	sk_plr_xbow_bolt_monster = {"sk_plr_xbow_bolt_monster","0"};
cvar_t	sk_plr_xbow_sniper_bullet = {"sk_plr_xbow_sniper_bullet","0"};
cvar_t	sk_plr_rpg = {"sk_plr_rpg","0"};
cvar_t	sk_plr_gauss = {"sk_plr_gauss","0"};
cvar_t	sk_plr_egon_narrow = {"sk_plr_egon_narrow","0"};
cvar_t	sk_plr_egon_wide = {"sk_plr_egon_wide","0"};
cvar_t	sk_plr_hand_grenade = {"sk_plr_hand_grenade","0"};
cvar_t	sk_plr_satchel = {"sk_plr_satchel","0"};
cvar_t	sk_plr_tripmine = {"sk_plr_tripmine","0"};
cvar_t	sk_plr_spore = { "sk_plr_spore", "100" };
cvar_t	sk_plr_shockrifle = { "sk_plr_shockrifle", "15" };

// WORLD WEAPONS
cvar_t	sk_12mm_bullet = {"sk_12mm_bullet","0"};
cvar_t	sk_9mmAR_bullet = {"sk_9mmAR_bullet","0"};
cvar_t	sk_9mm_bullet = {"sk_9mm_bullet","0"};
cvar_t	sk_762_bullet = {"sk_762_bullet","0"};
cvar_t	sk_hornet_dmg = {"sk_hornet_dmg","0"};
cvar_t	sk_556_bullet = { "sk_556_bullet", "15" };

// HEALTH/CHARGE
cvar_t	sk_suitcharger = { "sk_suitcharger","0" };		
cvar_t	sk_battery = { "sk_battery","0" };	
cvar_t	sk_healthcharger = { "sk_healthcharger","0" };	
cvar_t	sk_healthkit = { "sk_healthkit","0" };
cvar_t	sk_scientist_heal = { "sk_scientist_heal","0" };


// monster damage adjusters
cvar_t	sk_monster_head = { "sk_monster_head","2" };
cvar_t	sk_monster_chest = { "sk_monster_chest","" };
cvar_t	sk_monster_stomach = { "sk_monster_stomach","" };
cvar_t	sk_monster_arm = { "sk_monster_arm","" };
cvar_t	sk_monster_leg = { "sk_monster_leg","" };

// player damage adjusters
cvar_t	sk_player_head = { "sk_player_head","2" };
cvar_t	sk_player_chest = { "sk_player_chest","" };
cvar_t	sk_player_stomach = { "sk_player_stomach","" };
cvar_t	sk_player_arm = { "sk_player_arm","" };
cvar_t	sk_player_leg = { "sk_player_leg","" };

// Gonome
cvar_t	sk_gonome_health = { "sk_gonome_health", "200" };
cvar_t	sk_gonome_dmg_one_slash = { "sk_gonome_dmg_one_slash", "30" };
cvar_t	sk_gonome_dmg_one_bite = { "sk_gonome_dmg_one_bite", "15" };
cvar_t	sk_gonome_dmg_guts = { "sk_gonome_dmg_guts", "15" };

// Voltigore
cvar_t	sk_voltigore_health = { "sk_voltigore_health", "350" };
cvar_t	sk_voltigore_dmg_punch = { "sk_voltigore_dmg_punch", "30" };
cvar_t	sk_voltigore_dmg_beam = { "sk_voltigore_dmg_beam", "40" };
cvar_t	sk_voltigore_dmg_explode = { "sk_voltigore_dmg_explode", "200" };

// Tor
cvar_t	sk_tor_health = { "sk_tor_health", "800" };
cvar_t	sk_tor_punch = { "sk_tor_punch", "55" };
cvar_t	sk_tor_energybeam = { "sk_tor_energybeam", "3" };
cvar_t	sk_tor_sonicblast = { "sk_tor_sonicblast", "15" };

// baby garg
cvar_t	sk_babygargantua_health = { "sk_babygargantua_health", "600" };
cvar_t	sk_babygargantua_dmg_slash = { "sk_babygargantua_dmg_slash", "25" };
cvar_t	sk_babygargantua_dmg_fire = { "sk_babygargantua_dmg_fire", "2" };
cvar_t	sk_babygargantua_dmg_stomp = { "sk_babygargantua_dmg_stomp", "50" };

// pit drone
cvar_t	sk_pitdrone_health = { "sk_pitdrone_health", "60" };
cvar_t	sk_pitdrone_dmg_bite = { "sk_pitdrone_dmg_bite", "25" };
cvar_t	sk_pitdrone_dmg_whip = { "sk_pitdrone_dmg_whip", "25" };
cvar_t	sk_pitdrone_dmg_spit = { "sk_pitdrone_dmg_spit", "15" };

// shock trooper
cvar_t	sk_shocktrooper_health = { "sk_shocktrooper_health", "200" };
cvar_t	sk_shocktrooper_kick = { "sk_shocktrooper_kick", "12" };
cvar_t	sk_shocktrooper_maxcharge = { "sk_shocktrooper_maxcharge", "10" };
cvar_t	sk_shocktrooper_rechargespeed = { "sk_shocktrooper_rechargespeed", "5" };
cvar_t	sk_shocktrooper_grenadespeed = { "sk_shocktrooper_grenadespeed", "500" };

// shock roach
cvar_t	sk_shockroach_health = { "sk_shockroach_health", "20" };
cvar_t	sk_shockroach_dmg_bite = { "sk_shockroach_dmg_bite", "20" };
cvar_t	sk_shockroach_lifespan = { "sk_shockroach_lifespan", "20" };

// otis
cvar_t	sk_otis_health = { "sk_otis_health", "65" };
cvar_t	sk_otis_bullet = { "sk_otis_bullet", "34" };

// male assassin
cvar_t	sk_massassin_health = { "sk_massassin_health", "100" };
cvar_t	sk_massassin_grenadespeed = { "sk_massassin_grenadespeed", "500" };
cvar_t	sk_massassin_kick = { "sk_massassin_kick", "12" };

// turn speed multiplier (all monsters)
cvar_t	sk_yawspeed_mult = { "sk_yawspeed_mult","0" };

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

// REGISTER CVARS FOR SKILL LEVEL STUFF
	// Agrunt
	CVAR_REGISTER ( &sk_agrunt_health );// {"sk_agrunt_health","0"};
	CVAR_REGISTER ( &sk_agrunt_dmg_punch );// {"sk_agrunt_dmg_punch","0"};

	// Apache
	CVAR_REGISTER ( &sk_apache_health );// {"sk_apache_health","0"};

	// Osprey
	CVAR_REGISTER ( &sk_osprey_health);// {"sk_apache_health","0"};

	// Barney
	CVAR_REGISTER ( &sk_barney_health );// {"sk_barney_health","0"};

	// Bullsquid
	CVAR_REGISTER ( &sk_bullsquid_health );// {"sk_bullsquid_health","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_bite );// {"sk_bullsquid_dmg_bite","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_whip );// {"sk_bullsquid_dmg_whip","0"};
	CVAR_REGISTER ( &sk_bullsquid_dmg_spit );// {"sk_bullsquid_dmg_spit","0"};

	// Big Momma
	CVAR_REGISTER ( &sk_bigmomma_health );// {"sk_bigmomma_health_factor","1.0"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_slash );// {"sk_bigmomma_dmg_slash","50"};
	CVAR_REGISTER ( &sk_bigmomma_dmg_blast );// {"sk_bigmomma_dmg_blast","100"};
	CVAR_REGISTER ( &sk_bigmomma_radius_blast );// {"sk_bigmomma_radius_blast","250"};

	// Gargantua
	CVAR_REGISTER ( &sk_gargantua_health );// {"sk_gargantua_health","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_slash );// {"sk_gargantua_dmg_slash","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_fire );// {"sk_gargantua_dmg_fire","0"};
	CVAR_REGISTER ( &sk_gargantua_dmg_stomp );// {"sk_gargantua_dmg_stomp","0"};

	// Babygarg
	CVAR_REGISTER(&sk_babygarg_health);// {"sk_gargantua_health","0"};

	// Hassassin
	CVAR_REGISTER ( &sk_hassassin_health );// {"sk_hassassin_health","0"};

	// Headcrab
	CVAR_REGISTER ( &sk_headcrab_health );// {"sk_headcrab_health","0"};
	CVAR_REGISTER ( &sk_headcrab_dmg_bite );// {"sk_headcrab_dmg_bite","0"};

	// Hgrunt 
	CVAR_REGISTER ( &sk_hgrunt_health );// {"sk_hgrunt_health","0"};
	CVAR_REGISTER ( &sk_hgrunt_kick );// {"sk_hgrunt_kick","0"};
	CVAR_REGISTER ( &sk_hgrunt_pellets );
	CVAR_REGISTER ( &sk_hgrunt_gspeed );

	// hwgrunt
	CVAR_REGISTER ( &sk_hwgrunt_health);

	// Houndeye
	CVAR_REGISTER ( &sk_houndeye_health );// {"sk_houndeye_health","0"};
	CVAR_REGISTER ( &sk_houndeye_dmg_blast );// {"sk_houndeye_dmg_blast","0"};

	// ISlave
	CVAR_REGISTER ( &sk_islave_health );// {"sk_islave_health","0"};
	CVAR_REGISTER ( &sk_islave_dmg_claw );// {"sk_islave_dmg_claw","0"};
	CVAR_REGISTER ( &sk_islave_dmg_clawrake1	);// {"sk_islave_dmg_clawrake","0"};
	CVAR_REGISTER ( &sk_islave_dmg_zap );// {"sk_islave_dmg_zap","0"};

	// Icthyosaur
	CVAR_REGISTER ( &sk_ichthyosaur_health );// {"sk_ichthyosaur_health","0"};
	CVAR_REGISTER ( &sk_ichthyosaur_shake );// {"sk_ichthyosaur_health3","0"};

	// Leech
	CVAR_REGISTER ( &sk_leech_health );// {"sk_leech_health","0"};
	CVAR_REGISTER ( &sk_leech_dmg_bite );// {"sk_leech_dmg_bite","0"};

	// Controller
	CVAR_REGISTER ( &sk_controller_health );
	CVAR_REGISTER ( &sk_controller_dmgzap );
	CVAR_REGISTER ( &sk_controller_speedball );
	CVAR_REGISTER ( &sk_controller_dmgball );

	// Nihilanth
	CVAR_REGISTER ( &sk_nihilanth_health );// {"sk_nihilanth_health","0"};
	CVAR_REGISTER ( &sk_nihilanth_zap );

	// Scientist
	CVAR_REGISTER ( &sk_scientist_health );// {"sk_scientist_health","0"};

	// Snark
	CVAR_REGISTER ( &sk_snark_health );// {"sk_snark_health","0"};
	CVAR_REGISTER ( &sk_snark_dmg_bite );// {"sk_snark_dmg_bite","0"};
	CVAR_REGISTER ( &sk_snark_dmg_pop );// {"sk_snark_dmg_pop","0"};

	// Zombie
	CVAR_REGISTER ( &sk_zombie_health );// {"sk_zombie_health","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_one_slash );// {"sk_zombie_dmg_one_slash","0"};
	CVAR_REGISTER ( &sk_zombie_dmg_both_slash );// {"sk_zombie_dmg_both_slash","0"};

	//Turret
	CVAR_REGISTER ( &sk_turret_health );// {"sk_turret_health","0"};

	// MiniTurret
	CVAR_REGISTER ( &sk_miniturret_health );// {"sk_miniturret_health","0"};

	// Sentry Turret
	CVAR_REGISTER ( &sk_sentry_health );// {"sk_sentry_health","0"};

	// PLAYER WEAPONS
	CVAR_REGISTER ( &sk_plr_crowbar );// {"sk_plr_crowbar","0"};
	CVAR_REGISTER ( &sk_plr_9mm_bullet );// {"sk_plr_9mm_bullet","0"};
	CVAR_REGISTER ( &sk_plr_357_bullet );// {"sk_plr_357_bullet","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_bullet );// {"sk_plr_9mmAR_bullet","0"};
	CVAR_REGISTER ( &sk_plr_9mmAR_grenade );// {"sk_plr_9mmAR_grenade","0"};
	CVAR_REGISTER ( &sk_plr_buckshot );// {"sk_plr_buckshot","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_monster );// {"sk_plr_xbow_bolt","0"};
	CVAR_REGISTER ( &sk_plr_xbow_bolt_client );// {"sk_plr_xbow_bolt","0"};
	CVAR_REGISTER ( &sk_plr_xbow_sniper_bullet);// {"sk_plr_xbow_bolt","0"};
	CVAR_REGISTER ( &sk_plr_rpg );// {"sk_plr_rpg","0"};
	CVAR_REGISTER ( &sk_plr_gauss );// {"sk_plr_gauss","0"};
	CVAR_REGISTER ( &sk_plr_egon_narrow );// {"sk_plr_egon_narrow","0"};
	CVAR_REGISTER ( &sk_plr_egon_wide );// {"sk_plr_egon_wide","0"};
	CVAR_REGISTER ( &sk_plr_hand_grenade );// {"sk_plr_hand_grenade","0"};
	CVAR_REGISTER ( &sk_plr_satchel );// {"sk_plr_satchel","0"};
	CVAR_REGISTER ( &sk_plr_tripmine );// {"sk_plr_tripmine","0"};

	// WORLD WEAPONS
	CVAR_REGISTER ( &sk_12mm_bullet );// {"sk_12mm_bullet","0"};
	CVAR_REGISTER ( &sk_9mmAR_bullet );// {"sk_9mm_bullet","0"};
	CVAR_REGISTER ( &sk_9mm_bullet );// {"sk_9mm_bullet","0"};
	CVAR_REGISTER ( &sk_762_bullet );// {"sk_9mm_bullet3","0"};

	// HORNET
	CVAR_REGISTER ( &sk_hornet_dmg );// {"sk_hornet_dmg","0"};

	// HEALTH/SUIT CHARGE DISTRIBUTION
	CVAR_REGISTER ( &sk_suitcharger );
	CVAR_REGISTER ( &sk_battery );
	CVAR_REGISTER ( &sk_healthcharger );
	CVAR_REGISTER ( &sk_healthkit );
	CVAR_REGISTER ( &sk_scientist_heal );

// monster damage adjusters
	CVAR_REGISTER ( &sk_monster_head );
	CVAR_REGISTER ( &sk_monster_chest );
	CVAR_REGISTER ( &sk_monster_stomach );
	CVAR_REGISTER ( &sk_monster_arm );
	CVAR_REGISTER ( &sk_monster_leg );

// player damage adjusters
	CVAR_REGISTER ( &sk_player_head );
	CVAR_REGISTER ( &sk_player_chest );
	CVAR_REGISTER ( &sk_player_stomach );
	CVAR_REGISTER ( &sk_player_arm );
	CVAR_REGISTER ( &sk_player_leg );

	CVAR_REGISTER(&sk_gonome_health);
	CVAR_REGISTER(&sk_gonome_dmg_one_slash);
	CVAR_REGISTER(&sk_gonome_dmg_guts);
	CVAR_REGISTER(&sk_gonome_dmg_one_bite);

	CVAR_REGISTER(&sk_voltigore_health);
	CVAR_REGISTER(&sk_voltigore_dmg_punch);
	CVAR_REGISTER(&sk_voltigore_dmg_beam);
	CVAR_REGISTER(&sk_voltigore_dmg_explode);

	CVAR_REGISTER(&sk_tor_health);
	CVAR_REGISTER(&sk_tor_punch);
	CVAR_REGISTER(&sk_tor_energybeam);
	CVAR_REGISTER(&sk_tor_sonicblast);

	CVAR_REGISTER(&sk_babygargantua_health);
	CVAR_REGISTER(&sk_babygargantua_dmg_slash);
	CVAR_REGISTER(&sk_babygargantua_dmg_fire);
	CVAR_REGISTER(&sk_babygargantua_dmg_stomp);

	CVAR_REGISTER(&sk_pitdrone_health);
	CVAR_REGISTER(&sk_pitdrone_dmg_bite);
	CVAR_REGISTER(&sk_pitdrone_dmg_whip);
	CVAR_REGISTER(&sk_pitdrone_dmg_spit);

	CVAR_REGISTER(&sk_shocktrooper_health);
	CVAR_REGISTER(&sk_shocktrooper_kick);
	CVAR_REGISTER(&sk_shocktrooper_maxcharge);
	CVAR_REGISTER(&sk_shocktrooper_rechargespeed);
	CVAR_REGISTER(&sk_shocktrooper_grenadespeed);

	CVAR_REGISTER(&sk_shockroach_health);
	CVAR_REGISTER(&sk_shockroach_dmg_bite);
	CVAR_REGISTER(&sk_shockroach_lifespan);

	CVAR_REGISTER(&sk_plr_spore);

	CVAR_REGISTER(&sk_plr_shockrifle);

	CVAR_REGISTER(&sk_556_bullet);

	CVAR_REGISTER(&sk_otis_health);
	CVAR_REGISTER(&sk_otis_bullet);
	
	CVAR_REGISTER(&sk_massassin_health);
	CVAR_REGISTER(&sk_massassin_grenadespeed);
	CVAR_REGISTER(&sk_massassin_kick);

	CVAR_REGISTER(&sk_yawspeed_mult);

// END REGISTER CVARS FOR SKILL LEVEL STUFF

	SERVER_COMMAND( "exec skill.cfg\n" );
}

