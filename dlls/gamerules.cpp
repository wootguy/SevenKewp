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

//=========================================================
// load the SkillData struct with the proper values based on the skill level.
//=========================================================
void CGameRules::RefreshSkillData ( void )
{
	int	iSkill;

	iSkill = (int)CVAR_GET_FLOAT("skill");
	g_iSkillLevel = iSkill;

	if ( iSkill < 1 )
	{
		iSkill = 1;
	}
	else if ( iSkill > 3 )
	{
		iSkill = 3; 
	}

	gSkillData.iSkillLevel = iSkill;

	ALERT ( at_console, "\nGAME SKILL LEVEL:%d\n",iSkill );

	//Agrunt		
	gSkillData.agruntHealth = GetSkillCvar( "sk_agrunt_health" );
	gSkillData.agruntDmgPunch = GetSkillCvar( "sk_agrunt_dmg_punch");

	// Apache 
	gSkillData.apacheHealth = GetSkillCvar( "sk_apache_health");

	// Barney
	gSkillData.barneyHealth = GetSkillCvar( "sk_barney_health");

	// Big Momma
	gSkillData.bigmommaHealthFactor = GetSkillCvar( "sk_bigmomma_health_factor" );
	gSkillData.bigmommaDmgSlash = GetSkillCvar( "sk_bigmomma_dmg_slash" );
	gSkillData.bigmommaDmgBlast = GetSkillCvar( "sk_bigmomma_dmg_blast" );
	gSkillData.bigmommaRadiusBlast = GetSkillCvar( "sk_bigmomma_radius_blast" );

	// Bullsquid
	gSkillData.bullsquidHealth = GetSkillCvar( "sk_bullsquid_health");
	gSkillData.bullsquidDmgBite = GetSkillCvar( "sk_bullsquid_dmg_bite");
	gSkillData.bullsquidDmgWhip = GetSkillCvar( "sk_bullsquid_dmg_whip");
	gSkillData.bullsquidDmgSpit = GetSkillCvar( "sk_bullsquid_dmg_spit");

	// Gargantua
	gSkillData.gargantuaHealth = GetSkillCvar( "sk_gargantua_health");
	gSkillData.gargantuaDmgSlash = GetSkillCvar( "sk_gargantua_dmg_slash");
	gSkillData.gargantuaDmgFire = GetSkillCvar( "sk_gargantua_dmg_fire");
	gSkillData.gargantuaDmgStomp = GetSkillCvar( "sk_gargantua_dmg_stomp");

	// Hassassin
	gSkillData.hassassinHealth = GetSkillCvar( "sk_hassassin_health");

	// Headcrab
	gSkillData.headcrabHealth = GetSkillCvar( "sk_headcrab_health");
	gSkillData.headcrabDmgBite = GetSkillCvar( "sk_headcrab_dmg_bite");

	// Hgrunt 
	gSkillData.hgruntHealth = GetSkillCvar( "sk_hgrunt_health");
	gSkillData.hgruntDmgKick = GetSkillCvar( "sk_hgrunt_kick");
	gSkillData.hgruntShotgunPellets = GetSkillCvar( "sk_hgrunt_pellets");
	gSkillData.hgruntGrenadeSpeed = GetSkillCvar( "sk_hgrunt_gspeed");

	// Houndeye
	gSkillData.houndeyeHealth = GetSkillCvar( "sk_houndeye_health");
	gSkillData.houndeyeDmgBlast = GetSkillCvar( "sk_houndeye_dmg_blast");

	// ISlave
	gSkillData.slaveHealth = GetSkillCvar( "sk_islave_health");
	gSkillData.slaveDmgClaw = GetSkillCvar( "sk_islave_dmg_claw");
	gSkillData.slaveDmgClawrake = GetSkillCvar( "sk_islave_dmg_clawrake");
	gSkillData.slaveDmgZap = GetSkillCvar( "sk_islave_dmg_zap");

	// Icthyosaur
	gSkillData.ichthyosaurHealth = GetSkillCvar( "sk_ichthyosaur_health");
	gSkillData.ichthyosaurDmgShake = GetSkillCvar( "sk_ichthyosaur_shake");

	// Leech
	gSkillData.leechHealth = GetSkillCvar( "sk_leech_health");

	gSkillData.leechDmgBite = GetSkillCvar( "sk_leech_dmg_bite");

	// Controller
	gSkillData.controllerHealth = GetSkillCvar( "sk_controller_health");
	gSkillData.controllerDmgZap = GetSkillCvar( "sk_controller_dmgzap");
	gSkillData.controllerSpeedBall = GetSkillCvar( "sk_controller_speedball");
	gSkillData.controllerDmgBall = GetSkillCvar( "sk_controller_dmgball");

	// Nihilanth
	gSkillData.nihilanthHealth = GetSkillCvar( "sk_nihilanth_health");
	gSkillData.nihilanthZap = GetSkillCvar( "sk_nihilanth_zap");

	// Scientist
	gSkillData.scientistHealth = GetSkillCvar( "sk_scientist_health");

	// Snark
	gSkillData.snarkHealth = GetSkillCvar( "sk_snark_health");
	gSkillData.snarkDmgBite = GetSkillCvar( "sk_snark_dmg_bite");
	gSkillData.snarkDmgPop = GetSkillCvar( "sk_snark_dmg_pop");

	// Zombie
	gSkillData.zombieHealth = GetSkillCvar( "sk_zombie_health");
	gSkillData.zombieDmgOneSlash = GetSkillCvar( "sk_zombie_dmg_one_slash");
	gSkillData.zombieDmgBothSlash = GetSkillCvar( "sk_zombie_dmg_both_slash");

	//Turret
	gSkillData.turretHealth = GetSkillCvar( "sk_turret_health");

	// MiniTurret
	gSkillData.miniturretHealth = GetSkillCvar( "sk_miniturret_health");
	
	// Sentry Turret
	gSkillData.sentryHealth = GetSkillCvar( "sk_sentry_health");

// PLAYER WEAPONS

	// Crowbar whack
	gSkillData.plrDmgCrowbar = GetSkillCvar( "sk_plr_crowbar");

	// Glock Round
	gSkillData.plrDmg9MM = GetSkillCvar( "sk_plr_9mm_bullet");

	// 357 Round
	gSkillData.plrDmg357 = GetSkillCvar( "sk_plr_357_bullet");

	// MP5 Round
	gSkillData.plrDmgMP5 = GetSkillCvar( "sk_plr_9mmAR_bullet");

	// M203 grenade
	gSkillData.plrDmgM203Grenade = GetSkillCvar( "sk_plr_9mmAR_grenade");

	// Shotgun buckshot
	gSkillData.plrDmgBuckshot = GetSkillCvar( "sk_plr_buckshot");

	// Crossbow
	gSkillData.plrDmgCrossbowClient = GetSkillCvar( "sk_plr_xbow_bolt_client");
	gSkillData.plrDmgCrossbowMonster = GetSkillCvar( "sk_plr_xbow_bolt_monster");

	// RPG
	gSkillData.plrDmgRPG = GetSkillCvar( "sk_plr_rpg");

	// Gauss gun
	gSkillData.plrDmgGauss = GetSkillCvar( "sk_plr_gauss");

	// Egon Gun
	gSkillData.plrDmgEgonNarrow = GetSkillCvar( "sk_plr_egon_narrow");
	gSkillData.plrDmgEgonWide = GetSkillCvar( "sk_plr_egon_wide");

	// Hand Grendade
	gSkillData.plrDmgHandGrenade = GetSkillCvar( "sk_plr_hand_grenade");

	// Satchel Charge
	gSkillData.plrDmgSatchel = GetSkillCvar( "sk_plr_satchel");

	// Tripmine
	gSkillData.plrDmgTripmine = GetSkillCvar( "sk_plr_tripmine");

	// MONSTER WEAPONS
	gSkillData.monDmg12MM = GetSkillCvar( "sk_12mm_bullet");
	gSkillData.monDmgMP5 = GetSkillCvar ("sk_9mmAR_bullet" );
	gSkillData.monDmg9MM = GetSkillCvar( "sk_9mm_bullet");

	// MONSTER HORNET
	gSkillData.monDmgHornet = GetSkillCvar( "sk_hornet_dmg");

	// PLAYER HORNET
// Up to this point, player hornet damage and monster hornet damage were both using
// monDmgHornet to determine how much damage to do. In tuning the hivehand, we now need
// to separate player damage and monster hivehand damage. Since it's so late in the project, we've
// added plrDmgHornet to the SKILLDATA struct, but not to the engine CVar list, so it's inaccesible
// via SKILLS.CFG. Any player hivehand tuning must take place in the code. (sjb)
	gSkillData.plrDmgHornet = 7;


	// HEALTH/CHARGE
	gSkillData.suitchargerCapacity = GetSkillCvar( "sk_suitcharger" );
	gSkillData.batteryCapacity = GetSkillCvar( "sk_battery" );
	gSkillData.healthchargerCapacity = GetSkillCvar ( "sk_healthcharger" );
	gSkillData.healthkitCapacity = GetSkillCvar ( "sk_healthkit" );
	gSkillData.scientistHeal = GetSkillCvar ( "sk_scientist_heal" );

	// monster damage adj
	gSkillData.monHead = GetSkillCvar( "sk_monster_head" );
	gSkillData.monChest = GetSkillCvar( "sk_monster_chest" );
	gSkillData.monStomach = GetSkillCvar( "sk_monster_stomach" );
	gSkillData.monLeg = GetSkillCvar( "sk_monster_leg" );
	gSkillData.monArm = GetSkillCvar( "sk_monster_arm" );

	// player damage adj
	gSkillData.plrHead = GetSkillCvar( "sk_player_head" );
	gSkillData.plrChest = GetSkillCvar( "sk_player_chest" );
	gSkillData.plrStomach = GetSkillCvar( "sk_player_stomach" );
	gSkillData.plrLeg = GetSkillCvar( "sk_player_leg" );
	gSkillData.plrArm = GetSkillCvar( "sk_player_arm" );

	gSkillData.gonomeHealth = GetSkillCvar( "sk_gonome_health" );
	gSkillData.gonomeDmgOneSlash = GetSkillCvar( "sk_gonome_dmg_one_slash" );
	gSkillData.gonomeDmgOneBite = GetSkillCvar( "sk_gonome_dmg_one_bite" );
	gSkillData.gonomeDmgGuts = GetSkillCvar( "sk_gonome_dmg_guts" );

	gSkillData.voltigoreHealth = GetSkillCvar("sk_voltigore_health");
	gSkillData.voltigoreDmgPunch = GetSkillCvar("sk_voltigore_dmg_punch");
	gSkillData.voltigoreDmgBeam = GetSkillCvar("sk_voltigore_dmg_beam");
	gSkillData.voltigoreDmgExplode = GetSkillCvar("sk_voltigore_dmg_explode");

	gSkillData.torHealth = GetSkillCvar("sk_tor_health");
	gSkillData.torDmgPunch = GetSkillCvar("sk_tor_punch");
	gSkillData.torDmgEnergyBeam = GetSkillCvar("sk_tor_energybeam");
	gSkillData.torDmgSonicBlast = GetSkillCvar("sk_tor_sonicblast");

	gSkillData.babyGargHealth = GetSkillCvar("sk_babygargantua_health");
	gSkillData.babyGargDmgSlash = GetSkillCvar("sk_babygargantua_dmg_slash");
	gSkillData.babyGargDmgFire = GetSkillCvar("sk_babygargantua_dmg_fire");
	gSkillData.babyGargDmgStomp = GetSkillCvar("sk_babygargantua_dmg_stomp");

	gSkillData.pitdroneHealth = GetSkillCvar("sk_pitdrone_health");
	gSkillData.pitdroneDmgBite = GetSkillCvar("sk_pitdrone_dmg_bite");
	gSkillData.pitdroneDmgWhip = GetSkillCvar("sk_pitdrone_dmg_whip");
	gSkillData.pitdroneDmgSpit = GetSkillCvar("sk_pitdrone_dmg_spit");

	gSkillData.shocktrooperHealth = GetSkillCvar("sk_shocktrooper_health");
	gSkillData.shocktrooperDmgKick = GetSkillCvar("sk_shocktrooper_kick");
	gSkillData.shocktrooperMaxCharge = GetSkillCvar("sk_shocktrooper_maxcharge");
	gSkillData.shocktrooperRechargeSpeed = GetSkillCvar("sk_shocktrooper_rechargespeed");
	gSkillData.shocktrooperGrenadeSpeed = GetSkillCvar("sk_shocktrooper_grenadespeed");

	gSkillData.shockroachHealth = GetSkillCvar("sk_shockroach_health");
	gSkillData.shockroachDmgBite = GetSkillCvar("sk_shockroach_dmg_bite");
	gSkillData.shockroachLifespan = GetSkillCvar("sk_shockroach_lifespan");

	gSkillData.plrDmgSpore = GetSkillCvar("sk_plr_spore");

	gSkillData.plrDmgShockRifle = GetSkillCvar("sk_plr_shockrifle");

	gSkillData.plrDmgShockRifle = GetSkillCvar("sk_plr_shockrifle");

	gSkillData.plrDmg556 = GetSkillCvar("sk_556_bullet");

	gSkillData.otisHealth = GetSkillCvar("sk_otis_health");
	gSkillData.otisDmgBullet = GetSkillCvar("sk_otis_bullet");

	gSkillData.massassinHealth = GetSkillCvar("sk_massassin_health");
	gSkillData.massassinGrenadeSpeed = GetSkillCvar("sk_massassin_grenadespeed");
	gSkillData.massassinDmgKick = GetSkillCvar("sk_massassin_kick");

	gSkillData.yawspeedMult = GetSkillCvar("sk_yawspeed_mult");
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
		return;
	}

	std::stringstream data_stream(cfgFile);
	string line;

	int equipIdx = 0;

	while (std::getline(data_stream, line))
	{
		vector<string> parts = splitString(line, " \t");
		string name = trimSpaces(toLowerCase(parts[0]));
		string value = parts.size() > 1 ? trimSpaces(parts[1]) : "";

		// strip quotes from value
		value.erase(std::remove(value.begin(), value.end(), '"'), value.end());

		if (parts.size() > 1 && whitelistCommands.find(name) != whitelistCommands.end()) {
			if (mp_prefer_server_maxspeed.value == 1 && name == "sv_maxspeed") {
				int maxspeed = atoi(value.c_str());
				if (maxspeed == 270 || maxspeed == 320) { // default speeds for Half-Life (320) and Sven Co-op (270)
					ALERT(at_console, "mp_prefer_server_maxspeed: Ignoring \"sv_maxspeed %d\" set by map cfg.\n", maxspeed);
					continue;
				}
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

	int equipIdx = 0;

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

