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
	gSkillData.agruntHealth = CVAR_GET_FLOAT( "sk_agrunt_health" );
	gSkillData.agruntDmgPunch = CVAR_GET_FLOAT( "sk_agrunt_dmg_punch");

	// Apache 
	gSkillData.apacheHealth = CVAR_GET_FLOAT( "sk_apache_health");

	// Osprey
	gSkillData.ospreyHealth = CVAR_GET_FLOAT( "sk_osprey_health");

	// Barney
	gSkillData.barneyHealth = CVAR_GET_FLOAT( "sk_barney_health");

	// Barnacle
	gSkillData.barnacleHealth = CVAR_GET_FLOAT( "sk_barnacle_health");
	gSkillData.barnaclePullSpeed = CVAR_GET_FLOAT( "sk_barnacle_pullspeed");

	// Big Momma
	gSkillData.bigmommaHealth = CVAR_GET_FLOAT( "sk_bigmomma_health" );
	gSkillData.bigmommaDmgSlash = CVAR_GET_FLOAT( "sk_bigmomma_dmg_slash" );
	gSkillData.bigmommaDmgBlast = CVAR_GET_FLOAT( "sk_bigmomma_dmg_blast" );
	gSkillData.bigmommaRadiusBlast = CVAR_GET_FLOAT( "sk_bigmomma_radius_blast" );

	// Bullsquid
	gSkillData.bullsquidHealth = CVAR_GET_FLOAT( "sk_bullsquid_health");
	gSkillData.bullsquidDmgBite = CVAR_GET_FLOAT( "sk_bullsquid_dmg_bite");
	gSkillData.bullsquidDmgWhip = CVAR_GET_FLOAT( "sk_bullsquid_dmg_whip");
	gSkillData.bullsquidDmgSpit = CVAR_GET_FLOAT( "sk_bullsquid_dmg_spit");

	// Gargantua
	gSkillData.gargantuaHealth = CVAR_GET_FLOAT( "sk_gargantua_health");
	gSkillData.gargantuaDmgSlash = CVAR_GET_FLOAT( "sk_gargantua_dmg_slash");
	gSkillData.gargantuaDmgFire = CVAR_GET_FLOAT( "sk_gargantua_dmg_fire");
	gSkillData.gargantuaDmgStomp = CVAR_GET_FLOAT( "sk_gargantua_dmg_stomp");

	// Hassassin
	gSkillData.hassassinHealth = CVAR_GET_FLOAT( "sk_hassassin_health");

	// Headcrab
	gSkillData.headcrabHealth = CVAR_GET_FLOAT( "sk_headcrab_health");
	gSkillData.headcrabDmgBite = CVAR_GET_FLOAT( "sk_headcrab_dmg_bite");

	// Hgrunt 
	gSkillData.hgruntHealth = CVAR_GET_FLOAT( "sk_hgrunt_health");
	gSkillData.hgruntDmgKick = CVAR_GET_FLOAT( "sk_hgrunt_kick");
	gSkillData.hgruntShotgunPellets = CVAR_GET_FLOAT( "sk_hgrunt_pellets");
	gSkillData.hgruntGrenadeSpeed = CVAR_GET_FLOAT( "sk_hgrunt_gspeed");

	// HWGrunt
	gSkillData.hwgruntHealth = CVAR_GET_FLOAT( "sk_hwgrunt_health");

	// Houndeye
	gSkillData.houndeyeHealth = CVAR_GET_FLOAT( "sk_houndeye_health");
	gSkillData.houndeyeDmgBlast = CVAR_GET_FLOAT( "sk_houndeye_dmg_blast");

	// ISlave
	gSkillData.slaveHealth = CVAR_GET_FLOAT( "sk_islave_health");
	gSkillData.slaveDmgClaw = CVAR_GET_FLOAT( "sk_islave_dmg_claw");
	gSkillData.slaveDmgClawrake = CVAR_GET_FLOAT( "sk_islave_dmg_clawrake");
	gSkillData.slaveDmgZap = CVAR_GET_FLOAT( "sk_islave_dmg_zap");

	// Icthyosaur
	gSkillData.ichthyosaurHealth = CVAR_GET_FLOAT( "sk_ichthyosaur_health");
	gSkillData.ichthyosaurDmgShake = CVAR_GET_FLOAT( "sk_ichthyosaur_shake");

	// Leech
	gSkillData.leechHealth = CVAR_GET_FLOAT( "sk_leech_health");

	gSkillData.leechDmgBite = CVAR_GET_FLOAT( "sk_leech_dmg_bite");

	// Controller
	gSkillData.controllerHealth = CVAR_GET_FLOAT( "sk_controller_health");
	gSkillData.controllerDmgZap = CVAR_GET_FLOAT( "sk_controller_dmgzap");
	gSkillData.controllerSpeedBall = CVAR_GET_FLOAT( "sk_controller_speedball");
	gSkillData.controllerDmgBall = CVAR_GET_FLOAT( "sk_controller_dmgball");

	// Nihilanth
	gSkillData.nihilanthHealth = CVAR_GET_FLOAT( "sk_nihilanth_health");
	gSkillData.nihilanthZap = CVAR_GET_FLOAT( "sk_nihilanth_zap");

	// Scientist
	gSkillData.scientistHealth = CVAR_GET_FLOAT( "sk_scientist_health");

	// Snark
	gSkillData.snarkHealth = CVAR_GET_FLOAT( "sk_snark_health");
	gSkillData.snarkDmgBite = CVAR_GET_FLOAT( "sk_snark_dmg_bite");
	gSkillData.snarkDmgPop = CVAR_GET_FLOAT( "sk_snark_dmg_pop");

	// Zombie
	gSkillData.zombieHealth = CVAR_GET_FLOAT( "sk_zombie_health");
	gSkillData.zombieDmgOneSlash = CVAR_GET_FLOAT( "sk_zombie_dmg_one_slash");
	gSkillData.zombieDmgBothSlash = CVAR_GET_FLOAT( "sk_zombie_dmg_both_slash");

	//Turret
	gSkillData.turretHealth = CVAR_GET_FLOAT( "sk_turret_health");

	// MiniTurret
	gSkillData.miniturretHealth = CVAR_GET_FLOAT( "sk_miniturret_health");
	
	// Sentry Turret
	gSkillData.sentryHealth = CVAR_GET_FLOAT( "sk_sentry_health");

// PLAYER WEAPONS

	// Crowbar whack
	gSkillData.plrDmgCrowbar = CVAR_GET_FLOAT( "sk_plr_crowbar");

	// Glock Round
	gSkillData.plrDmg9MM = CVAR_GET_FLOAT( "sk_plr_9mm_bullet");

	// 357 Round
	gSkillData.plrDmg357 = CVAR_GET_FLOAT( "sk_plr_357_bullet");

	// MP5 Round
	gSkillData.plrDmgMP5 = CVAR_GET_FLOAT( "sk_plr_9mmAR_bullet");

	// M203 grenade
	gSkillData.plrDmgM203Grenade = CVAR_GET_FLOAT( "sk_plr_9mmAR_grenade");

	// Shotgun buckshot
	gSkillData.plrDmgBuckshot = CVAR_GET_FLOAT( "sk_plr_buckshot");

	// Crossbow
	gSkillData.plrDmgCrossbowClient = CVAR_GET_FLOAT( "sk_plr_xbow_bolt_client");
	gSkillData.plrDmgCrossbowMonster = CVAR_GET_FLOAT( "sk_plr_xbow_bolt_monster");
	gSkillData.plrDmgCrossbowSniper = CVAR_GET_FLOAT( "sk_plr_xbow_sniper_bullet");

	// RPG
	gSkillData.plrDmgRPG = CVAR_GET_FLOAT( "sk_plr_rpg");

	// Gauss gun
	gSkillData.plrDmgGauss = CVAR_GET_FLOAT( "sk_plr_gauss");

	// Egon Gun
	gSkillData.plrDmgEgonNarrow = CVAR_GET_FLOAT( "sk_plr_egon_narrow");
	gSkillData.plrDmgEgonWide = CVAR_GET_FLOAT( "sk_plr_egon_wide");

	// Hand Grendade
	gSkillData.plrDmgHandGrenade = CVAR_GET_FLOAT( "sk_plr_hand_grenade");

	// Satchel Charge
	gSkillData.plrDmgSatchel = CVAR_GET_FLOAT( "sk_plr_satchel");

	// Tripmine
	gSkillData.plrDmgTripmine = CVAR_GET_FLOAT( "sk_plr_tripmine");

	// MONSTER WEAPONS
	gSkillData.monDmg12MM = CVAR_GET_FLOAT( "sk_12mm_bullet");
	gSkillData.monDmgMP5 = CVAR_GET_FLOAT ("sk_9mmAR_bullet" );
	gSkillData.monDmg9MM = CVAR_GET_FLOAT( "sk_9mm_bullet");
	gSkillData.monDmg762 = CVAR_GET_FLOAT( "sk_762_bullet");

	// MONSTER HORNET
	gSkillData.monDmgHornet = CVAR_GET_FLOAT( "sk_hornet_dmg");

	// PLAYER HORNET
	gSkillData.plrDmgHornet = CVAR_GET_FLOAT( "sk_plr_hornet" );


	// HEALTH/CHARGE
	gSkillData.suitchargerCapacity = CVAR_GET_FLOAT( "sk_suitcharger" );
	gSkillData.batteryCapacity = CVAR_GET_FLOAT( "sk_battery" );
	gSkillData.healthchargerCapacity = CVAR_GET_FLOAT ( "sk_healthcharger" );
	gSkillData.healthkitCapacity = CVAR_GET_FLOAT ( "sk_healthkit" );
	gSkillData.scientistHeal = CVAR_GET_FLOAT ( "sk_scientist_heal" );

	// monster damage adj
	gSkillData.monHead = CVAR_GET_FLOAT( "sk_monster_head" );
	gSkillData.monChest = CVAR_GET_FLOAT( "sk_monster_chest" );
	gSkillData.monStomach = CVAR_GET_FLOAT( "sk_monster_stomach" );
	gSkillData.monLeg = CVAR_GET_FLOAT( "sk_monster_leg" );
	gSkillData.monArm = CVAR_GET_FLOAT( "sk_monster_arm" );

	// player damage adj
	gSkillData.plrHead = CVAR_GET_FLOAT( "sk_player_head" );
	gSkillData.plrChest = CVAR_GET_FLOAT( "sk_player_chest" );
	gSkillData.plrStomach = CVAR_GET_FLOAT( "sk_player_stomach" );
	gSkillData.plrLeg = CVAR_GET_FLOAT( "sk_player_leg" );
	gSkillData.plrArm = CVAR_GET_FLOAT( "sk_player_arm" );

	gSkillData.gonomeHealth = CVAR_GET_FLOAT( "sk_gonome_health" );
	gSkillData.gonomeDmgOneSlash = CVAR_GET_FLOAT( "sk_gonome_dmg_one_slash" );
	gSkillData.gonomeDmgOneBite = CVAR_GET_FLOAT( "sk_gonome_dmg_one_bite" );
	gSkillData.gonomeDmgGuts = CVAR_GET_FLOAT( "sk_gonome_dmg_guts" );

	gSkillData.voltigoreHealth = CVAR_GET_FLOAT("sk_voltigore_health");
	gSkillData.voltigoreDmgPunch = CVAR_GET_FLOAT("sk_voltigore_dmg_punch");
	gSkillData.voltigoreDmgBeam = CVAR_GET_FLOAT("sk_voltigore_dmg_beam");
	gSkillData.voltigoreDmgExplode = CVAR_GET_FLOAT("sk_voltigore_dmg_explode");

	gSkillData.torHealth = CVAR_GET_FLOAT("sk_tor_health");
	gSkillData.torDmgPunch = CVAR_GET_FLOAT("sk_tor_punch");
	gSkillData.torDmgEnergyBeam = CVAR_GET_FLOAT("sk_tor_energybeam");
	gSkillData.torDmgSonicBlast = CVAR_GET_FLOAT("sk_tor_sonicblast");

	gSkillData.babyGargHealth = CVAR_GET_FLOAT("sk_babygargantua_health");
	gSkillData.babyGargDmgSlash = CVAR_GET_FLOAT("sk_babygargantua_dmg_slash");
	gSkillData.babyGargDmgFire = CVAR_GET_FLOAT("sk_babygargantua_dmg_fire");
	gSkillData.babyGargDmgStomp = CVAR_GET_FLOAT("sk_babygargantua_dmg_stomp");

	gSkillData.pitdroneHealth = CVAR_GET_FLOAT("sk_pitdrone_health");
	gSkillData.pitdroneDmgBite = CVAR_GET_FLOAT("sk_pitdrone_dmg_bite");
	gSkillData.pitdroneDmgWhip = CVAR_GET_FLOAT("sk_pitdrone_dmg_whip");
	gSkillData.pitdroneDmgSpit = CVAR_GET_FLOAT("sk_pitdrone_dmg_spit");

	gSkillData.shocktrooperHealth = CVAR_GET_FLOAT("sk_shocktrooper_health");
	gSkillData.shocktrooperDmgKick = CVAR_GET_FLOAT("sk_shocktrooper_kick");
	gSkillData.shocktrooperMaxCharge = CVAR_GET_FLOAT("sk_shocktrooper_maxcharge");
	gSkillData.shocktrooperRechargeSpeed = CVAR_GET_FLOAT("sk_shocktrooper_rechargespeed");
	gSkillData.shocktrooperGrenadeSpeed = CVAR_GET_FLOAT("sk_shocktrooper_grenadespeed");

	gSkillData.shockroachHealth = CVAR_GET_FLOAT("sk_shockroach_health");
	gSkillData.shockroachDmgBite = CVAR_GET_FLOAT("sk_shockroach_dmg_bite");
	gSkillData.shockroachLifespan = CVAR_GET_FLOAT("sk_shockroach_lifespan");

	gSkillData.plrDmgSpore = CVAR_GET_FLOAT("sk_plr_spore");

	gSkillData.plrDmgShockRifle = CVAR_GET_FLOAT("sk_plr_shockrifle");

	gSkillData.plrDmg556 = CVAR_GET_FLOAT("sk_556_bullet");

	gSkillData.otisHealth = CVAR_GET_FLOAT("sk_otis_health");
	gSkillData.otisDmgBullet = CVAR_GET_FLOAT("sk_otis_bullet");

	gSkillData.massassinHealth = CVAR_GET_FLOAT("sk_massassin_health");
	gSkillData.massassinGrenadeSpeed = CVAR_GET_FLOAT("sk_massassin_grenadespeed");
	gSkillData.massassinDmgKick = CVAR_GET_FLOAT("sk_massassin_kick");

	gSkillData.yawspeedMult = CVAR_GET_FLOAT("sk_yawspeed_mult");

	g_defaultMonsterHealth.clear();

	g_defaultMonsterHealth["cycler"] = 80000;
	g_defaultMonsterHealth["hornet"] = 1;
	g_defaultMonsterHealth["monster_alien_babyvoltigore"] = 80;
	g_defaultMonsterHealth["monster_alien_controller"] = gSkillData.controllerHealth;
	g_defaultMonsterHealth["monster_alien_grunt"] = gSkillData.agruntHealth;
	g_defaultMonsterHealth["monster_alien_slave"] = gSkillData.slaveHealth;
	g_defaultMonsterHealth["monster_alien_tor"] = gSkillData.torHealth;
	g_defaultMonsterHealth["monster_alien_voltigore"] = gSkillData.voltigoreHealth;
	g_defaultMonsterHealth["monster_apache"] = gSkillData.apacheHealth;
	g_defaultMonsterHealth["monster_assassin_repel"] = gSkillData.massassinHealth;
	g_defaultMonsterHealth["monster_babycrab"] = gSkillData.headcrabHealth * 0.25;
	g_defaultMonsterHealth["monster_babygarg"] = gSkillData.babyGargHealth;
	g_defaultMonsterHealth["monster_barnacle"] = gSkillData.barnacleHealth;
	g_defaultMonsterHealth["monster_barney"] = gSkillData.barneyHealth;
	g_defaultMonsterHealth["monster_bigmomma"] = gSkillData.bigmommaHealth;
	g_defaultMonsterHealth["monster_blkop_osprey"] = gSkillData.ospreyHealth;
	g_defaultMonsterHealth["monster_blkop_apache"] = gSkillData.apacheHealth;
	g_defaultMonsterHealth["monster_bloater"] = 40;
	g_defaultMonsterHealth["monster_bodyguard"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_bullchicken"] = gSkillData.bullsquidHealth;
	g_defaultMonsterHealth["monster_chumtoad"] = 100;
	g_defaultMonsterHealth["monster_cleansuit_scientist"] = gSkillData.scientistHealth;
	g_defaultMonsterHealth["monster_cockroach"] = 1;
	g_defaultMonsterHealth["monster_flyer_flock"] = 100;
	g_defaultMonsterHealth["monster_furniture"] = 80000;
	g_defaultMonsterHealth["monster_gargantua"] = gSkillData.gargantuaHealth;
	g_defaultMonsterHealth["monster_generic"] = 8;
	g_defaultMonsterHealth["monster_gman"] = 100;
	g_defaultMonsterHealth["monster_gonome"] = gSkillData.gonomeHealth;
	g_defaultMonsterHealth["monster_grunt_ally_repel"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_grunt_repel"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_headcrab"] = gSkillData.headcrabHealth;
	g_defaultMonsterHealth["monster_houndeye"] = gSkillData.houndeyeHealth;
	g_defaultMonsterHealth["monster_human_assassin"] = gSkillData.hassassinHealth;
	g_defaultMonsterHealth["monster_human_grunt"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_human_grunt_ally"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_human_medic_ally"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_human_torch_ally"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_hwgrunt"] = gSkillData.hwgruntHealth;
	g_defaultMonsterHealth["monster_hwgrunt_repel"] = gSkillData.hwgruntHealth;
	g_defaultMonsterHealth["monster_ichthyosaur"] = gSkillData.ichthyosaurHealth;
	g_defaultMonsterHealth["monster_kingpin"] = gSkillData.torHealth;
	g_defaultMonsterHealth["monster_leech"] = gSkillData.leechHealth;
	g_defaultMonsterHealth["monster_male_assassin"] = gSkillData.massassinHealth;
	g_defaultMonsterHealth["monster_medic_ally_repel"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_miniturret"] = gSkillData.miniturretHealth;
	g_defaultMonsterHealth["monster_nihilanth"] = gSkillData.nihilanthHealth;
	g_defaultMonsterHealth["monster_osprey"] = gSkillData.ospreyHealth;
	g_defaultMonsterHealth["monster_otis"] = gSkillData.otisHealth;
	g_defaultMonsterHealth["monster_pitdrone"] = gSkillData.pitdroneHealth;
	g_defaultMonsterHealth["monster_rat"] = 8;
	g_defaultMonsterHealth["monster_robogrunt"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_robogrunt_repel"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_scientist"] = gSkillData.scientistHealth;
	g_defaultMonsterHealth["monster_sentry"] = gSkillData.sentryHealth;
	g_defaultMonsterHealth["monster_shockroach"] = gSkillData.shockroachHealth;
	g_defaultMonsterHealth["monster_shocktrooper"] = gSkillData.shocktrooperHealth;
	g_defaultMonsterHealth["monster_sitting_scientist"] = gSkillData.scientistHealth;
	g_defaultMonsterHealth["monster_snark"] = gSkillData.snarkHealth;
	g_defaultMonsterHealth["monster_sqknest"] = 100;
	g_defaultMonsterHealth["monster_stukabat"] = gSkillData.controllerHealth;
	g_defaultMonsterHealth["monster_tentacle"] = 4000;
	g_defaultMonsterHealth["monster_torch_ally_repel"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_turret"] = gSkillData.turretHealth;
	g_defaultMonsterHealth["monster_zombie"] = gSkillData.zombieHealth;
	g_defaultMonsterHealth["monster_zombie_barney"] = gSkillData.zombieHealth;
	g_defaultMonsterHealth["monster_zombie_soldier"] = gSkillData.zombieHealth;
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

