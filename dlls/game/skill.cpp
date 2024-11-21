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
// skill.cpp - code for skill level concerns
//=========================================================
#include	"extdll.h"
#include	"util.h"
#include	"skill.h"


skilldata_t	gSkillData;
std::unordered_map<std::string, float> g_defaultMonsterHealthMap; // default hp for the map
std::unordered_map<std::string, float> g_defaultMonsterHealthServer; // default hp for the server
std::unordered_map<std::string, skill_cvar_t*> g_skillCvars;

#define DECL_SKILL_CVAR(name, type) {{#name, "0", 0, 0, 0}, offsetof(skilldata_t, name), type}

skill_cvar_t skill_cvars[] = {
	// Agrunt
	DECL_SKILL_CVAR(sk_agrunt_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_agrunt_dmg_punch, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_agrunt_hornet_mode, CVAR_TYPE_AI),

	// Apache
	DECL_SKILL_CVAR(sk_apache_health, CVAR_TYPE_HEALTH),

	// Osprey
	DECL_SKILL_CVAR(sk_osprey_health, CVAR_TYPE_HEALTH),

	// Barney
	DECL_SKILL_CVAR(sk_barney_health, CVAR_TYPE_HEALTH),

	// Barnacle
	DECL_SKILL_CVAR(sk_barnacle_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_barnacle_pullspeed, CVAR_TYPE_AI),

	// Bullsquid
	DECL_SKILL_CVAR(sk_bullsquid_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_bullsquid_dmg_bite, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_bullsquid_dmg_whip, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_bullsquid_dmg_spit, CVAR_TYPE_DAMAGE),

	// Big Momma
	DECL_SKILL_CVAR(sk_bigmomma_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_bigmomma_dmg_slash, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_bigmomma_dmg_blast, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_bigmomma_radius_blast, CVAR_TYPE_DAMAGE),

	// Gargantua
	DECL_SKILL_CVAR(sk_gargantua_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_gargantua_dmg_slash, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_gargantua_dmg_fire, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_gargantua_dmg_stomp, CVAR_TYPE_DAMAGE),

	// Hassassin
	DECL_SKILL_CVAR(sk_hassassin_health, CVAR_TYPE_HEALTH),

	// Headcrab
	DECL_SKILL_CVAR(sk_headcrab_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_headcrab_dmg_bite, CVAR_TYPE_DAMAGE),

	// Hgrunt
	DECL_SKILL_CVAR(sk_hgrunt_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_hgrunt_kick, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_hgrunt_pellets, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_hgrunt_gspeed, CVAR_TYPE_AI),

	// HWGrunt
	DECL_SKILL_CVAR(sk_hwgrunt_health, CVAR_TYPE_HEALTH),

	// Houndeye
	DECL_SKILL_CVAR(sk_houndeye_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_houndeye_dmg_blast, CVAR_TYPE_DAMAGE),

	// ISlave
	DECL_SKILL_CVAR(sk_islave_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_islave_dmg_claw, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_islave_dmg_clawrake, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_islave_dmg_zap, CVAR_TYPE_DAMAGE),

	// Icthyosaur
	DECL_SKILL_CVAR(sk_ichthyosaur_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_ichthyosaur_shake, CVAR_TYPE_DAMAGE),

	// Leech
	DECL_SKILL_CVAR(sk_leech_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_leech_dmg_bite, CVAR_TYPE_DAMAGE),

	// Controller
	DECL_SKILL_CVAR(sk_controller_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_controller_dmgzap, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_controller_speedball, CVAR_TYPE_AI),
	DECL_SKILL_CVAR(sk_controller_dmgball, CVAR_TYPE_DAMAGE),

	// Nihilanth
	DECL_SKILL_CVAR(sk_nihilanth_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_nihilanth_zap, CVAR_TYPE_DAMAGE),

	// Scientist
	DECL_SKILL_CVAR(sk_scientist_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_scientist_heal, CVAR_TYPE_DAMAGE),

	// Snark
	DECL_SKILL_CVAR(sk_snark_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_snark_dmg_bite, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_snark_dmg_pop, CVAR_TYPE_DAMAGE),

	// Zombie
	DECL_SKILL_CVAR(sk_zombie_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_zombie_dmg_one_slash, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_zombie_dmg_both_slash, CVAR_TYPE_DAMAGE),

	// Turrets
	DECL_SKILL_CVAR(sk_turret_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_miniturret_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_sentry_health, CVAR_TYPE_HEALTH),

	// Gonome
	DECL_SKILL_CVAR(sk_gonome_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_gonome_dmg_one_slash, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_gonome_dmg_one_bite, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_gonome_dmg_guts, CVAR_TYPE_DAMAGE),

	// Voltigore
	DECL_SKILL_CVAR(sk_voltigore_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_voltigore_dmg_punch, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_voltigore_dmg_beam, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_voltigore_dmg_explode, CVAR_TYPE_DAMAGE),

	// Tor
	DECL_SKILL_CVAR(sk_tor_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_tor_punch, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_tor_energybeam, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_tor_sonicblast, CVAR_TYPE_DAMAGE),

	// Baby garg
	DECL_SKILL_CVAR(sk_babygargantua_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_babygargantua_dmg_slash, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_babygargantua_dmg_fire, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_babygargantua_dmg_stomp, CVAR_TYPE_DAMAGE),

	// Pit drone
	DECL_SKILL_CVAR(sk_pitdrone_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_pitdrone_dmg_bite, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_pitdrone_dmg_whip, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_pitdrone_dmg_spit, CVAR_TYPE_DAMAGE),

	// Shock trooper
	DECL_SKILL_CVAR(sk_shocktrooper_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_shocktrooper_kick, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_shocktrooper_maxcharge, CVAR_TYPE_AI),
	DECL_SKILL_CVAR(sk_shocktrooper_rechargespeed, CVAR_TYPE_AI),
	DECL_SKILL_CVAR(sk_shocktrooper_grenadespeed, CVAR_TYPE_AI),

	// Shock roach
	DECL_SKILL_CVAR(sk_shockroach_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_shockroach_dmg_bite, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_shockroach_lifespan, CVAR_TYPE_AI),

	// Otis
	DECL_SKILL_CVAR(sk_otis_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_otis_bullet, CVAR_TYPE_DAMAGE),

	// Male assassin
	DECL_SKILL_CVAR(sk_massassin_health, CVAR_TYPE_HEALTH),
	DECL_SKILL_CVAR(sk_massassin_grenadespeed, CVAR_TYPE_AI),
	DECL_SKILL_CVAR(sk_massassin_kick, CVAR_TYPE_DAMAGE),

	// Player weapons
	DECL_SKILL_CVAR(sk_plr_crowbar, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_9mm_bullet, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_357_bullet, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_9mmAR_bullet, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_9mmAR_grenade, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_buckshot, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_xbow_bolt_client, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_xbow_bolt_monster, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_xbow_sniper_bullet, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_rpg, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_gauss, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_secondarygauss, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_egon_narrow, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_egon_wide, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_hand_grenade, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_satchel, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_tripmine, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_spore, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_shockrifle, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_hornet, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_pipewrench, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_pipewrench_full_damage, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_displacer_other, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_plr_displacer_radius, CVAR_TYPE_DAMAGE),

	// Player max ammo
	DECL_SKILL_CVAR(sk_ammo_max_uranium, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_9mm, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_357, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_buckshot, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_bolts, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_rockets, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_grenades, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_satchels, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_tripmines, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_snarks, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_hornets, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_argrenades, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_ammo_max_spores, CVAR_TYPE_ITEM),

	// World weapons
	DECL_SKILL_CVAR(sk_12mm_bullet, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_9mmAR_bullet, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_9mm_bullet, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_762_bullet, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_hornet_dmg, CVAR_TYPE_DAMAGE),
	DECL_SKILL_CVAR(sk_556_bullet, CVAR_TYPE_DAMAGE),

	// Health/Charge
	DECL_SKILL_CVAR(sk_suitcharger, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_battery, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_healthcharger, CVAR_TYPE_ITEM),
	DECL_SKILL_CVAR(sk_healthkit, CVAR_TYPE_ITEM),

	// Monster damage adjusters
	DECL_SKILL_CVAR(sk_monster_head, CVAR_TYPE_HITGROUP),
	DECL_SKILL_CVAR(sk_monster_chest, CVAR_TYPE_HITGROUP),
	DECL_SKILL_CVAR(sk_monster_stomach, CVAR_TYPE_HITGROUP),
	DECL_SKILL_CVAR(sk_monster_arm, CVAR_TYPE_HITGROUP),
	DECL_SKILL_CVAR(sk_monster_leg, CVAR_TYPE_HITGROUP),

	// Player damage adjusters
	DECL_SKILL_CVAR(sk_player_head, CVAR_TYPE_HITGROUP),
	DECL_SKILL_CVAR(sk_player_chest, CVAR_TYPE_HITGROUP),
	DECL_SKILL_CVAR(sk_player_stomach, CVAR_TYPE_HITGROUP),
	DECL_SKILL_CVAR(sk_player_arm, CVAR_TYPE_HITGROUP),
	DECL_SKILL_CVAR(sk_player_leg, CVAR_TYPE_HITGROUP),

	// Mosnter turn speed multiplier
	DECL_SKILL_CVAR(sk_yawspeed_mult, CVAR_TYPE_AI),
};

void RegisterSkillCvars() {
	for (int i = 0; i < (int)(sizeof(skill_cvars) / sizeof(skill_cvar_t)); i++) {
		CVAR_REGISTER(&skill_cvars[i].cvar);
		g_skillCvars[skill_cvars[i].cvar.name] = &skill_cvars[i];
	}
}

cvar_t* GetSkillCvar(const char* cvar) {
	for (int i = 0; i < (int)(sizeof(skill_cvars) / sizeof(skill_cvar_t)); i++) {
		if (FStrEq(cvar, skill_cvars[i].cvar.name)) {
			return &skill_cvars[i].cvar;
		}
	}

	return NULL;
}

void RefreshSkillData(bool mapSkills) {
	for (int i = 0; i < (int)(sizeof(skill_cvars) / sizeof(skill_cvar_t)); i++) {
		float* structMember = (float*)((byte*)&gSkillData.firstMember + skill_cvars[i].structMemberOffset);
		*structMember = skill_cvars[i].cvar.value;
	}

	std::unordered_map<std::string, float>& hpDict = mapSkills ?
		g_defaultMonsterHealthMap : g_defaultMonsterHealthServer;

	hpDict.clear();

	hpDict["cycler"] = 80000;
	hpDict["hornet"] = 1;
	hpDict["monster_alien_babyvoltigore"] = 80;
	hpDict["monster_alien_controller"] = gSkillData.sk_controller_health;
	hpDict["monster_alien_grunt"] = gSkillData.sk_agrunt_health;
	hpDict["monster_alien_slave"] = gSkillData.sk_islave_health;
	hpDict["monster_alien_tor"] = gSkillData.sk_tor_health;
	hpDict["monster_alien_voltigore"] = gSkillData.sk_voltigore_health;
	hpDict["monster_apache"] = gSkillData.sk_apache_health;
	hpDict["monster_assassin_repel"] = gSkillData.sk_massassin_health;
	hpDict["monster_babycrab"] = gSkillData.sk_headcrab_health * 0.25;
	hpDict["monster_babygarg"] = gSkillData.sk_babygargantua_health;
	hpDict["monster_barnacle"] = gSkillData.sk_barnacle_health;
	hpDict["monster_barney"] = gSkillData.sk_barney_health;
	hpDict["monster_bigmomma"] = gSkillData.sk_bigmomma_health;
	hpDict["monster_blkop_osprey"] = gSkillData.sk_osprey_health;
	hpDict["monster_blkop_apache"] = gSkillData.sk_apache_health;
	hpDict["monster_bloater"] = 40;
	hpDict["monster_bodyguard"] = gSkillData.sk_hgrunt_health;
	hpDict["monster_bullchicken"] = gSkillData.sk_bullsquid_health;
	hpDict["monster_chumtoad"] = 100;
	hpDict["monster_cleansuit_scientist"] = gSkillData.sk_scientist_health;
	hpDict["monster_cockroach"] = 1;
	hpDict["monster_flyer_flock"] = 100;
	hpDict["monster_furniture"] = 80000;
	hpDict["monster_gargantua"] = gSkillData.sk_gargantua_health;
	hpDict["monster_generic"] = 8;
	hpDict["monster_gman"] = 100;
	hpDict["monster_gonome"] = gSkillData.sk_gonome_health;
	hpDict["monster_grunt_ally_repel"] = gSkillData.sk_hgrunt_health;
	hpDict["monster_grunt_repel"] = gSkillData.sk_hgrunt_health;
	hpDict["monster_headcrab"] = gSkillData.sk_headcrab_health;
	hpDict["monster_houndeye"] = gSkillData.sk_houndeye_health;
	hpDict["monster_human_assassin"] = gSkillData.sk_hassassin_health;
	hpDict["monster_human_grunt"] = gSkillData.sk_hgrunt_health;
	hpDict["monster_human_grunt_ally"] = gSkillData.sk_hgrunt_health;
	hpDict["monster_human_medic_ally"] = gSkillData.sk_hgrunt_health;
	hpDict["monster_human_torch_ally"] = gSkillData.sk_hgrunt_health;
	hpDict["monster_hwgrunt"] = gSkillData.sk_hwgrunt_health;
	hpDict["monster_hwgrunt_repel"] = gSkillData.sk_hwgrunt_health;
	hpDict["monster_ichthyosaur"] = gSkillData.sk_ichthyosaur_health;
	hpDict["monster_kingpin"] = gSkillData.sk_tor_health;
	hpDict["monster_leech"] = gSkillData.sk_leech_health;
	hpDict["monster_male_assassin"] = gSkillData.sk_massassin_health;
	hpDict["monster_medic_ally_repel"] = gSkillData.sk_hgrunt_health;
	hpDict["monster_miniturret"] = gSkillData.sk_miniturret_health;
	hpDict["monster_nihilanth"] = gSkillData.sk_nihilanth_health;
	hpDict["monster_osprey"] = gSkillData.sk_osprey_health;
	hpDict["monster_otis"] = gSkillData.sk_otis_health;
	hpDict["monster_pitdrone"] = gSkillData.sk_pitdrone_health;
	hpDict["monster_rat"] = 8;
	hpDict["monster_robogrunt"] = gSkillData.sk_hgrunt_health * 5;
	hpDict["monster_robogrunt_repel"] = gSkillData.sk_hgrunt_health * 5;
	hpDict["monster_scientist"] = gSkillData.sk_scientist_health;
	hpDict["monster_sentry"] = gSkillData.sk_sentry_health;
	hpDict["monster_shockroach"] = gSkillData.sk_shockroach_health;
	hpDict["monster_shocktrooper"] = gSkillData.sk_shocktrooper_health;
	hpDict["monster_sitting_scientist"] = gSkillData.sk_scientist_health;
	hpDict["monster_snark"] = gSkillData.sk_snark_health;
	hpDict["monster_sqknest"] = 100;
	hpDict["monster_stukabat"] = gSkillData.sk_controller_health;
	hpDict["monster_tentacle"] = 4000;
	hpDict["monster_torch_ally_repel"] = gSkillData.sk_hgrunt_health;
	hpDict["monster_turret"] = gSkillData.sk_turret_health;
	hpDict["monster_zombie"] = gSkillData.sk_zombie_health;
	hpDict["monster_zombie_barney"] = gSkillData.sk_zombie_health;
	hpDict["monster_zombie_soldier"] = gSkillData.sk_zombie_health;
}

float GetDefaultHealth(const char* monstertype, bool mapSkills) {
	std::unordered_map<std::string, float>& hpDict = mapSkills ?
		g_defaultMonsterHealthMap : g_defaultMonsterHealthServer;

	auto defaultHp = hpDict.find(monstertype);

	if (defaultHp != hpDict.end()) {
		return defaultHp->second;
	}
	else {
		ALERT(at_console, "Monster type %s has no default health\n", monstertype);
		return 0;
	}
}

