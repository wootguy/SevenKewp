#pragma once

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
// skill.h - skill level concerns
//=========================================================

struct skilldata_t
{
	int firstMember; // dummy var used for addressing other members
	int iSkillLevel; // game skill level

	// Agrunt
	float sk_agrunt_health;
	float sk_agrunt_dmg_punch;
	float sk_agrunt_hornet_mode;

	// Apache
	float sk_apache_health;

	// Osprey
	float sk_osprey_health;

	// Barney
	float sk_barney_health;

	// Barnacle
	float sk_barnacle_health;
	float sk_barnacle_pullspeed;

	// Bullsquid
	float sk_bullsquid_health;
	float sk_bullsquid_dmg_bite;
	float sk_bullsquid_dmg_whip;
	float sk_bullsquid_dmg_spit;

	// Big Momma
	float sk_bigmomma_health;
	float sk_bigmomma_dmg_slash;
	float sk_bigmomma_dmg_blast;
	float sk_bigmomma_radius_blast;

	// Gargantua
	float sk_gargantua_health;
	float sk_gargantua_dmg_slash;
	float sk_gargantua_dmg_fire;
	float sk_gargantua_dmg_stomp;

	// Hassassin
	float sk_hassassin_health;

	// Headcrab
	float sk_headcrab_health;
	float sk_headcrab_dmg_bite;

	// Hgrunt
	float sk_hgrunt_health;
	float sk_hgrunt_kick;
	float sk_hgrunt_pellets;
	float sk_hgrunt_gspeed;

	// HWGrunt
	float sk_hwgrunt_health;

	// Houndeye
	float sk_houndeye_health;
	float sk_houndeye_dmg_blast;

	// ISlave
	float sk_islave_health;
	float sk_islave_dmg_claw;
	float sk_islave_dmg_clawrake;
	float sk_islave_dmg_zap;

	// Icthyosaur
	float sk_ichthyosaur_health;
	float sk_ichthyosaur_shake;

	// Leech
	float sk_leech_health;
	float sk_leech_dmg_bite;

	// Controller
	float sk_controller_health;
	float sk_controller_dmgzap;
	float sk_controller_speedball;
	float sk_controller_dmgball;

	// Nihilanth
	float sk_nihilanth_health;
	float sk_nihilanth_zap;

	// Scientist
	float sk_scientist_health;

	// Snark
	float sk_snark_health;
	float sk_snark_dmg_bite;
	float sk_snark_dmg_pop;

	// Zombie
	float sk_zombie_health;
	float sk_zombie_dmg_one_slash;
	float sk_zombie_dmg_both_slash;

	// Turrets
	float sk_turret_health;
	float sk_miniturret_health;
	float sk_sentry_health;

	// Gonome
	float sk_gonome_health;
	float sk_gonome_dmg_one_slash;
	float sk_gonome_dmg_one_bite;
	float sk_gonome_dmg_guts;

	// Voltigore
	float sk_voltigore_health;
	float sk_voltigore_dmg_punch;
	float sk_voltigore_dmg_beam;
	float sk_voltigore_dmg_explode;

	// Tor
	float sk_tor_health;
	float sk_tor_punch;
	float sk_tor_energybeam;
	float sk_tor_sonicblast;

	// Baby garg
	float sk_babygargantua_health;
	float sk_babygargantua_dmg_slash;
	float sk_babygargantua_dmg_fire;
	float sk_babygargantua_dmg_stomp;

	// Pit drone
	float sk_pitdrone_health;
	float sk_pitdrone_dmg_bite;
	float sk_pitdrone_dmg_whip;
	float sk_pitdrone_dmg_spit;

	// Shock trooper
	float sk_shocktrooper_health;
	float sk_shocktrooper_kick;
	float sk_shocktrooper_maxcharge;
	float sk_shocktrooper_rechargespeed;
	float sk_shocktrooper_grenadespeed;

	// Shock roach
	float sk_shockroach_health;
	float sk_shockroach_dmg_bite;
	float sk_shockroach_lifespan;

	// Otis
	float sk_otis_health;
	float sk_otis_bullet;

	// Male assassin
	float sk_massassin_health;
	float sk_massassin_grenadespeed;
	float sk_massassin_kick;

	// Player weapons
	float sk_plr_crowbar;
	float sk_plr_9mm_bullet;
	float sk_plr_357_bullet;
	float sk_plr_9mmAR_bullet;
	float sk_plr_9mmAR_grenade;
	float sk_plr_buckshot;
	float sk_plr_xbow_bolt_client;
	float sk_plr_xbow_bolt_monster;
	float sk_plr_xbow_sniper_bullet;
	float sk_plr_rpg;
	float sk_plr_gauss;
	float sk_plr_egon_narrow;
	float sk_plr_egon_wide;
	float sk_plr_hand_grenade;
	float sk_plr_satchel;
	float sk_plr_tripmine;
	float sk_plr_spore;
	float sk_plr_shockrifle;
	float sk_plr_hornet;

	// World weapons
	float sk_12mm_bullet;
	float sk_9mmAR_bullet;
	float sk_9mm_bullet;
	float sk_762_bullet;
	float sk_hornet_dmg;
	float sk_556_bullet;

	// Health/Charge
	float sk_suitcharger;
	float sk_battery;
	float sk_healthcharger;
	float sk_healthkit;
	float sk_scientist_heal;

	// Monster damage adjusters
	float sk_monster_head;
	float sk_monster_chest;
	float sk_monster_stomach;
	float sk_monster_arm;
	float sk_monster_leg;

	// Player damage adjusters
	float sk_player_head;
	float sk_player_chest;
	float sk_player_stomach;
	float sk_player_arm;
	float sk_player_leg;

	// Mosnter turn speed multiplier
	float sk_yawspeed_mult;
};

EXPORT extern DLL_GLOBAL skilldata_t gSkillData;

EXPORT extern DLL_GLOBAL int g_iSkillLevel;
EXPORT extern std::map<std::string, float> g_defaultMonsterHealth;

void RefreshSkillData();

void RegisterSkillCvars();

float GetDefaultHealth(const char* monstertype);

cvar_t* GetSkillCvar(const char* cvar);


#define SKILL_EASY		1
#define SKILL_MEDIUM	2
#define SKILL_HARD		3
