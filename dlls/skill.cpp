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


float GetDefaultHealth(const char* monstertype) {
	if (g_defaultMonsterHealth.find(monstertype) != g_defaultMonsterHealth.end()) {
		return g_defaultMonsterHealth[monstertype];
	}
	else {
		ALERT(at_console, "Monster type %s has no default health\n", monstertype);
		return 0;
	}
}

