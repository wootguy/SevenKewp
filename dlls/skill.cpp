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
std::map<std::string, float> g_defaultMonsterHealth;

#define DECL_SKILL_CVAR(name) {{#name, "0", 0, 0, 0}, offsetof(skilldata_t, name)}

struct skill_cvar_t {
	cvar_t cvar;
	int structMemberOffset; // member offset into skill data struct
};

skill_cvar_t skill_cvars[] = {
	// Agrunt
	DECL_SKILL_CVAR(sk_agrunt_health),
	DECL_SKILL_CVAR(sk_agrunt_dmg_punch),
	DECL_SKILL_CVAR(sk_agrunt_hornet_mode),

	// Apache
	DECL_SKILL_CVAR(sk_apache_health),

	// Osprey
	DECL_SKILL_CVAR(sk_osprey_health),

	// Barney
	DECL_SKILL_CVAR(sk_barney_health),

	// Barnacle
	DECL_SKILL_CVAR(sk_barnacle_health),
	DECL_SKILL_CVAR(sk_barnacle_pullspeed),

	// Bullsquid
	DECL_SKILL_CVAR(sk_bullsquid_health),
	DECL_SKILL_CVAR(sk_bullsquid_dmg_bite),
	DECL_SKILL_CVAR(sk_bullsquid_dmg_whip),
	DECL_SKILL_CVAR(sk_bullsquid_dmg_spit),

	// Big Momma
	DECL_SKILL_CVAR(sk_bigmomma_health),
	DECL_SKILL_CVAR(sk_bigmomma_dmg_slash),
	DECL_SKILL_CVAR(sk_bigmomma_dmg_blast),
	DECL_SKILL_CVAR(sk_bigmomma_radius_blast),

	// Gargantua
	DECL_SKILL_CVAR(sk_gargantua_health),
	DECL_SKILL_CVAR(sk_gargantua_dmg_slash),
	DECL_SKILL_CVAR(sk_gargantua_dmg_fire),
	DECL_SKILL_CVAR(sk_gargantua_dmg_stomp),

	// Hassassin
	DECL_SKILL_CVAR(sk_hassassin_health),

	// Headcrab
	DECL_SKILL_CVAR(sk_headcrab_health),
	DECL_SKILL_CVAR(sk_headcrab_dmg_bite),

	// Hgrunt
	DECL_SKILL_CVAR(sk_hgrunt_health),
	DECL_SKILL_CVAR(sk_hgrunt_kick),
	DECL_SKILL_CVAR(sk_hgrunt_pellets),
	DECL_SKILL_CVAR(sk_hgrunt_gspeed),

	// HWGrunt
	DECL_SKILL_CVAR(sk_hwgrunt_health),

	// Houndeye
	DECL_SKILL_CVAR(sk_houndeye_health),
	DECL_SKILL_CVAR(sk_houndeye_dmg_blast),

	// ISlave
	DECL_SKILL_CVAR(sk_islave_health),
	DECL_SKILL_CVAR(sk_islave_dmg_claw),
	DECL_SKILL_CVAR(sk_islave_dmg_clawrake),
	DECL_SKILL_CVAR(sk_islave_dmg_zap),

	// Icthyosaur
	DECL_SKILL_CVAR(sk_ichthyosaur_health),
	DECL_SKILL_CVAR(sk_ichthyosaur_shake),

	// Leech
	DECL_SKILL_CVAR(sk_leech_health),
	DECL_SKILL_CVAR(sk_leech_dmg_bite),

	// Controller
	DECL_SKILL_CVAR(sk_controller_health),
	DECL_SKILL_CVAR(sk_controller_dmgzap),
	DECL_SKILL_CVAR(sk_controller_speedball),
	DECL_SKILL_CVAR(sk_controller_dmgball),

	// Nihilanth
	DECL_SKILL_CVAR(sk_nihilanth_health),
	DECL_SKILL_CVAR(sk_nihilanth_zap),

	// Scientist
	DECL_SKILL_CVAR(sk_scientist_health),

	// Snark
	DECL_SKILL_CVAR(sk_snark_health),
	DECL_SKILL_CVAR(sk_snark_dmg_bite),
	DECL_SKILL_CVAR(sk_snark_dmg_pop),

	// Zombie
	DECL_SKILL_CVAR(sk_zombie_health),
	DECL_SKILL_CVAR(sk_zombie_dmg_one_slash),
	DECL_SKILL_CVAR(sk_zombie_dmg_both_slash),

	// Turrets
	DECL_SKILL_CVAR(sk_turret_health),
	DECL_SKILL_CVAR(sk_miniturret_health),
	DECL_SKILL_CVAR(sk_sentry_health),

	// Gonome
	DECL_SKILL_CVAR(sk_gonome_health),
	DECL_SKILL_CVAR(sk_gonome_dmg_one_slash),
	DECL_SKILL_CVAR(sk_gonome_dmg_one_bite),
	DECL_SKILL_CVAR(sk_gonome_dmg_guts),

	// Voltigore
	DECL_SKILL_CVAR(sk_voltigore_health),
	DECL_SKILL_CVAR(sk_voltigore_dmg_punch),
	DECL_SKILL_CVAR(sk_voltigore_dmg_beam),
	DECL_SKILL_CVAR(sk_voltigore_dmg_explode),

	// Tor
	DECL_SKILL_CVAR(sk_tor_health),
	DECL_SKILL_CVAR(sk_tor_punch),
	DECL_SKILL_CVAR(sk_tor_energybeam),
	DECL_SKILL_CVAR(sk_tor_sonicblast),

	// Baby garg
	DECL_SKILL_CVAR(sk_babygargantua_health),
	DECL_SKILL_CVAR(sk_babygargantua_dmg_slash),
	DECL_SKILL_CVAR(sk_babygargantua_dmg_fire),
	DECL_SKILL_CVAR(sk_babygargantua_dmg_stomp),

	// Pit drone
	DECL_SKILL_CVAR(sk_pitdrone_health),
	DECL_SKILL_CVAR(sk_pitdrone_dmg_bite),
	DECL_SKILL_CVAR(sk_pitdrone_dmg_whip),
	DECL_SKILL_CVAR(sk_pitdrone_dmg_spit),

	// Shock trooper
	DECL_SKILL_CVAR(sk_shocktrooper_health),
	DECL_SKILL_CVAR(sk_shocktrooper_kick),
	DECL_SKILL_CVAR(sk_shocktrooper_maxcharge),
	DECL_SKILL_CVAR(sk_shocktrooper_rechargespeed),
	DECL_SKILL_CVAR(sk_shocktrooper_grenadespeed),

	// Shock roach
	DECL_SKILL_CVAR(sk_shockroach_health),
	DECL_SKILL_CVAR(sk_shockroach_dmg_bite),
	DECL_SKILL_CVAR(sk_shockroach_lifespan),

	// Otis
	DECL_SKILL_CVAR(sk_otis_health),
	DECL_SKILL_CVAR(sk_otis_bullet),

	// Male assassin
	DECL_SKILL_CVAR(sk_massassin_health),
	DECL_SKILL_CVAR(sk_massassin_grenadespeed),
	DECL_SKILL_CVAR(sk_massassin_kick),

	// Player weapons
	DECL_SKILL_CVAR(sk_plr_crowbar),
	DECL_SKILL_CVAR(sk_plr_9mm_bullet),
	DECL_SKILL_CVAR(sk_plr_357_bullet),
	DECL_SKILL_CVAR(sk_plr_9mmAR_bullet),
	DECL_SKILL_CVAR(sk_plr_9mmAR_grenade),
	DECL_SKILL_CVAR(sk_plr_buckshot),
	DECL_SKILL_CVAR(sk_plr_xbow_bolt_client),
	DECL_SKILL_CVAR(sk_plr_xbow_bolt_monster),
	DECL_SKILL_CVAR(sk_plr_xbow_sniper_bullet),
	DECL_SKILL_CVAR(sk_plr_rpg),
	DECL_SKILL_CVAR(sk_plr_gauss),
	DECL_SKILL_CVAR(sk_plr_egon_narrow),
	DECL_SKILL_CVAR(sk_plr_egon_wide),
	DECL_SKILL_CVAR(sk_plr_hand_grenade),
	DECL_SKILL_CVAR(sk_plr_satchel),
	DECL_SKILL_CVAR(sk_plr_tripmine),
	DECL_SKILL_CVAR(sk_plr_spore),
	DECL_SKILL_CVAR(sk_plr_shockrifle),
	DECL_SKILL_CVAR(sk_plr_hornet),
	DECL_SKILL_CVAR(sk_plr_pipewrench),
	DECL_SKILL_CVAR(sk_plr_pipewrench_full_damage),
	DECL_SKILL_CVAR(sk_plr_displacer_other),
	DECL_SKILL_CVAR(sk_plr_displacer_radius),

	// Player max ammo
	DECL_SKILL_CVAR(sk_ammo_max_uranium),
	DECL_SKILL_CVAR(sk_ammo_max_9mm),
	DECL_SKILL_CVAR(sk_ammo_max_357),
	DECL_SKILL_CVAR(sk_ammo_max_buckshot),
	DECL_SKILL_CVAR(sk_ammo_max_bolts),
	DECL_SKILL_CVAR(sk_ammo_max_rockets),
	DECL_SKILL_CVAR(sk_ammo_max_grenades),
	DECL_SKILL_CVAR(sk_ammo_max_satchels),
	DECL_SKILL_CVAR(sk_ammo_max_tripmines),
	DECL_SKILL_CVAR(sk_ammo_max_snarks),
	DECL_SKILL_CVAR(sk_ammo_max_hornets),
	DECL_SKILL_CVAR(sk_ammo_max_argrenades),
	DECL_SKILL_CVAR(sk_ammo_max_spores),

	// World weapons
	DECL_SKILL_CVAR(sk_12mm_bullet),
	DECL_SKILL_CVAR(sk_9mmAR_bullet),
	DECL_SKILL_CVAR(sk_9mm_bullet),
	DECL_SKILL_CVAR(sk_762_bullet),
	DECL_SKILL_CVAR(sk_hornet_dmg),
	DECL_SKILL_CVAR(sk_556_bullet),

	// Health/Charge
	DECL_SKILL_CVAR(sk_suitcharger),
	DECL_SKILL_CVAR(sk_battery),
	DECL_SKILL_CVAR(sk_healthcharger),
	DECL_SKILL_CVAR(sk_healthkit),
	DECL_SKILL_CVAR(sk_scientist_heal),

	// Monster damage adjusters
	DECL_SKILL_CVAR(sk_monster_head),
	DECL_SKILL_CVAR(sk_monster_chest),
	DECL_SKILL_CVAR(sk_monster_stomach),
	DECL_SKILL_CVAR(sk_monster_arm),
	DECL_SKILL_CVAR(sk_monster_leg),

	// Player damage adjusters
	DECL_SKILL_CVAR(sk_player_head),
	DECL_SKILL_CVAR(sk_player_chest),
	DECL_SKILL_CVAR(sk_player_stomach),
	DECL_SKILL_CVAR(sk_player_arm),
	DECL_SKILL_CVAR(sk_player_leg),

	// Mosnter turn speed multiplier
	DECL_SKILL_CVAR(sk_yawspeed_mult),
};

void RegisterSkillCvars() {
	for (int i = 0; i < (int)(sizeof(skill_cvars) / sizeof(skill_cvar_t)); i++) {
		CVAR_REGISTER(&skill_cvars[i].cvar);
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

void RefreshSkillData() {
	int	iSkill;

	iSkill = (int)CVAR_GET_FLOAT("skill");
	g_iSkillLevel = iSkill;

	if (iSkill < 1)
	{
		iSkill = 1;
	}
	else if (iSkill > 3)
	{
		iSkill = 3;
	}

	gSkillData.iSkillLevel = iSkill;

	ALERT(at_console, "\nGAME SKILL LEVEL:%d\n", iSkill);

	for (int i = 0; i < (int)(sizeof(skill_cvars) / sizeof(skill_cvar_t)); i++) {
		float* structMember = (float*)((byte*)&gSkillData.firstMember + skill_cvars[i].structMemberOffset);
		*structMember = skill_cvars[i].cvar.value;
	}

	g_defaultMonsterHealth.clear();

	g_defaultMonsterHealth["cycler"] = 80000;
	g_defaultMonsterHealth["hornet"] = 1;
	g_defaultMonsterHealth["monster_alien_babyvoltigore"] = 80;
	g_defaultMonsterHealth["monster_alien_controller"] = gSkillData.sk_controller_health;
	g_defaultMonsterHealth["monster_alien_grunt"] = gSkillData.sk_agrunt_health;
	g_defaultMonsterHealth["monster_alien_slave"] = gSkillData.sk_islave_health;
	g_defaultMonsterHealth["monster_alien_tor"] = gSkillData.sk_tor_health;
	g_defaultMonsterHealth["monster_alien_voltigore"] = gSkillData.sk_voltigore_health;
	g_defaultMonsterHealth["monster_apache"] = gSkillData.sk_apache_health;
	g_defaultMonsterHealth["monster_assassin_repel"] = gSkillData.sk_massassin_health;
	g_defaultMonsterHealth["monster_babycrab"] = gSkillData.sk_headcrab_health * 0.25;
	g_defaultMonsterHealth["monster_babygarg"] = gSkillData.sk_babygargantua_health;
	g_defaultMonsterHealth["monster_barnacle"] = gSkillData.sk_barnacle_health;
	g_defaultMonsterHealth["monster_barney"] = gSkillData.sk_barney_health;
	g_defaultMonsterHealth["monster_bigmomma"] = gSkillData.sk_bigmomma_health;
	g_defaultMonsterHealth["monster_blkop_osprey"] = gSkillData.sk_osprey_health;
	g_defaultMonsterHealth["monster_blkop_apache"] = gSkillData.sk_apache_health;
	g_defaultMonsterHealth["monster_bloater"] = 40;
	g_defaultMonsterHealth["monster_bodyguard"] = gSkillData.sk_hgrunt_health;
	g_defaultMonsterHealth["monster_bullchicken"] = gSkillData.sk_bullsquid_health;
	g_defaultMonsterHealth["monster_chumtoad"] = 100;
	g_defaultMonsterHealth["monster_cleansuit_scientist"] = gSkillData.sk_scientist_health;
	g_defaultMonsterHealth["monster_cockroach"] = 1;
	g_defaultMonsterHealth["monster_flyer_flock"] = 100;
	g_defaultMonsterHealth["monster_furniture"] = 80000;
	g_defaultMonsterHealth["monster_gargantua"] = gSkillData.sk_gargantua_health;
	g_defaultMonsterHealth["monster_generic"] = 8;
	g_defaultMonsterHealth["monster_gman"] = 100;
	g_defaultMonsterHealth["monster_gonome"] = gSkillData.sk_gonome_health;
	g_defaultMonsterHealth["monster_grunt_ally_repel"] = gSkillData.sk_hgrunt_health;
	g_defaultMonsterHealth["monster_grunt_repel"] = gSkillData.sk_hgrunt_health;
	g_defaultMonsterHealth["monster_headcrab"] = gSkillData.sk_headcrab_health;
	g_defaultMonsterHealth["monster_houndeye"] = gSkillData.sk_houndeye_health;
	g_defaultMonsterHealth["monster_human_assassin"] = gSkillData.sk_hassassin_health;
	g_defaultMonsterHealth["monster_human_grunt"] = gSkillData.sk_hgrunt_health;
	g_defaultMonsterHealth["monster_human_grunt_ally"] = gSkillData.sk_hgrunt_health;
	g_defaultMonsterHealth["monster_human_medic_ally"] = gSkillData.sk_hgrunt_health;
	g_defaultMonsterHealth["monster_human_torch_ally"] = gSkillData.sk_hgrunt_health;
	g_defaultMonsterHealth["monster_hwgrunt"] = gSkillData.sk_hwgrunt_health;
	g_defaultMonsterHealth["monster_hwgrunt_repel"] = gSkillData.sk_hwgrunt_health;
	g_defaultMonsterHealth["monster_ichthyosaur"] = gSkillData.sk_ichthyosaur_health;
	g_defaultMonsterHealth["monster_kingpin"] = gSkillData.sk_tor_health;
	g_defaultMonsterHealth["monster_leech"] = gSkillData.sk_leech_health;
	g_defaultMonsterHealth["monster_male_assassin"] = gSkillData.sk_massassin_health;
	g_defaultMonsterHealth["monster_medic_ally_repel"] = gSkillData.sk_hgrunt_health;
	g_defaultMonsterHealth["monster_miniturret"] = gSkillData.sk_miniturret_health;
	g_defaultMonsterHealth["monster_nihilanth"] = gSkillData.sk_nihilanth_health;
	g_defaultMonsterHealth["monster_osprey"] = gSkillData.sk_osprey_health;
	g_defaultMonsterHealth["monster_otis"] = gSkillData.sk_otis_health;
	g_defaultMonsterHealth["monster_pitdrone"] = gSkillData.sk_pitdrone_health;
	g_defaultMonsterHealth["monster_rat"] = 8;
	g_defaultMonsterHealth["monster_robogrunt"] = gSkillData.sk_hgrunt_health * 5;
	g_defaultMonsterHealth["monster_robogrunt_repel"] = gSkillData.sk_hgrunt_health * 5;
	g_defaultMonsterHealth["monster_scientist"] = gSkillData.sk_scientist_health;
	g_defaultMonsterHealth["monster_sentry"] = gSkillData.sk_sentry_health;
	g_defaultMonsterHealth["monster_shockroach"] = gSkillData.sk_shockroach_health;
	g_defaultMonsterHealth["monster_shocktrooper"] = gSkillData.sk_shocktrooper_health;
	g_defaultMonsterHealth["monster_sitting_scientist"] = gSkillData.sk_scientist_health;
	g_defaultMonsterHealth["monster_snark"] = gSkillData.sk_snark_health;
	g_defaultMonsterHealth["monster_sqknest"] = 100;
	g_defaultMonsterHealth["monster_stukabat"] = gSkillData.sk_controller_health;
	g_defaultMonsterHealth["monster_tentacle"] = 4000;
	g_defaultMonsterHealth["monster_torch_ally_repel"] = gSkillData.sk_hgrunt_health;
	g_defaultMonsterHealth["monster_turret"] = gSkillData.sk_turret_health;
	g_defaultMonsterHealth["monster_zombie"] = gSkillData.sk_zombie_health;
	g_defaultMonsterHealth["monster_zombie_barney"] = gSkillData.sk_zombie_health;
	g_defaultMonsterHealth["monster_zombie_soldier"] = gSkillData.sk_zombie_health;
}

float GetDefaultHealth(const char* monstertype) {
	if (g_defaultMonsterHealth.find(monstertype) != g_defaultMonsterHealth.end()) {
		return g_defaultMonsterHealth[monstertype];
	}
	else {
		ALERT(at_console, "Monster type %s has no default health\n", monstertype);
		return 0;
	}
}

