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


//=========================================================
// take the name of a cvar, tack a digit for the skill level
// on, and return the value.of that Cvar 
//=========================================================
float GetSkillCvar( const char *pName )
{
	int		iCount;
	float	flValue;
	char	szBuffer[ 64 ];
	
	iCount = sprintf( szBuffer, "%s%d",pName, gSkillData.iSkillLevel );

	flValue = CVAR_GET_FLOAT ( szBuffer );

	if ( flValue <= 0 )
	{
		ALERT ( at_console, "\n\n** GetSkillCVar Got a zero for %s **\n\n", szBuffer );
	}

	return flValue;
}

void UpdateSkillData() {
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
	g_defaultMonsterHealth["monster_babygarg"] = gSkillData.gargantuaHealth * 0.5f;
	g_defaultMonsterHealth["monster_barnacle"] = 50;
	g_defaultMonsterHealth["monster_barney"] = gSkillData.barneyHealth;
	g_defaultMonsterHealth["monster_bigmomma"] = 150 * gSkillData.bigmommaHealthFactor;
	g_defaultMonsterHealth["monster_blkop_osprey"] = 400;
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
	g_defaultMonsterHealth["monster_hwgrunt"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_hwgrunt_repel"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_ichthyosaur"] = gSkillData.ichthyosaurHealth;
	g_defaultMonsterHealth["monster_kingpin"] = gSkillData.torHealth;
	g_defaultMonsterHealth["monster_leech"] = gSkillData.leechHealth;
	g_defaultMonsterHealth["monster_male_assassin"] = gSkillData.massassinHealth;
	g_defaultMonsterHealth["monster_medic_ally_repel"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_miniturret"] = gSkillData.miniturretHealth;
	g_defaultMonsterHealth["monster_nihilanth"] = gSkillData.nihilanthHealth;
	g_defaultMonsterHealth["monster_osprey"] = 400;
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
	g_defaultMonsterHealth["monster_snark"] = 100;
	g_defaultMonsterHealth["monster_sqknest"] = 100;
	g_defaultMonsterHealth["monster_stukabat"] = gSkillData.controllerHealth;
	g_defaultMonsterHealth["monster_tentacle"] = 4000;
	g_defaultMonsterHealth["monster_torch_ally_repel"] = gSkillData.hgruntHealth;
	g_defaultMonsterHealth["monster_turret"] = gSkillData.turretHealth;
	g_defaultMonsterHealth["monster_zombie"] = gSkillData.zombieHealth;
	g_defaultMonsterHealth["monster_zombie_barney"] = gSkillData.zombieHealth;
	g_defaultMonsterHealth["monster_zombie_soldier"] = gSkillData.zombieHealth;
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

