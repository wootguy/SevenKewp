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
/*

===== subs.cpp ========================================================

  frequently used global functions

*/

#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "nodes.h"
#include "CBaseDoor.h"
#include "skill.h"

string_t g_debug_target; // for breakpointing on triggers

extern BOOL FEntIsVisible(entvars_t* pev, entvars_t* pevTarget);

void FireTargetsDelayed(const char* target, string_t killTarget, CBaseEntity* pActivator, USE_TYPE useType, float delay) {
	// create a temp object to fire at a later time
	CBaseDelay* pTemp = GetClassPtr((CBaseDelay*)NULL);
	pTemp->pev->classname = MAKE_STRING("DelayedUse");
	pTemp->pev->nextthink = gpGlobals->time + delay;
	pTemp->SetThink(&CBaseDelay::DelayThink);

	// Save the useType
	pTemp->pev->button = (int)useType;
	pTemp->m_flDelay = 0; // prevent "recursion"
	pTemp->pev->target = ALLOC_STRING(target);
	pTemp->m_iszKillTarget = killTarget;

	// HACKHACK
	// This wasn't in the release build of Half-Life.  We should have moved m_hActivator into this class
	// but changing member variable hierarchy would break save/restore without some ugly code.
	// This code is not as ugly as that code
	if (pActivator && pActivator->IsPlayer()) {		// If a player activates, then save it
		pTemp->pev->owner = pActivator->edict();
	}
}

void FireTarget(CBaseEntity* pTarget, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	if (!pTarget || (pTarget->pev->flags & FL_KILLME))
		return; // Don't use dying ents
	
	const char* targetName = STRING(pTarget->pev->targetname);
	bool isDebugTrigger = g_debug_target && !strcmp(STRING(g_debug_target), targetName);

	ALERT(at_aiconsole, "Found: %s, firing (%s)\n", STRING(pTarget->pev->classname), targetName);
	if (isDebugTrigger) {
		ALERT(at_aiconsole, "Breakpoint here!\n"); // place a breakpoint here to debug specific triggers
	}

	pTarget->m_hActivator = pActivator;
	pTarget->m_hCaller = pCaller;
	pTarget->Use(pActivator, pCaller, useType, value);
}

void FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value, float delay )
{
	CBaseEntity * pTarget = NULL;
	if ( !targetName )
		return;

	if (delay != 0)
	{
		FireTargetsDelayed(targetName, 0, pActivator, useType, delay);
		return;
	}

	ALERT( at_aiconsole, "Firing: (%s)\n", targetName );

	for (;;)
	{
		pTarget = UTIL_FindEntityByTargetname(pTarget, targetName);
		if (!pTarget)
			break;

		FireTarget(pTarget, pActivator, pCaller, useType, value);
	}
}

/*
QuakeEd only writes a single float for angles (bad idea), so up and down are
just constant angles.
*/
void SetMovedir( entvars_t *pev )
{
	if (pev->movedir != g_vecZero)
		return; // Spawn() called again

	if (pev->angles == Vector(0, -1, 0))
	{
		pev->movedir = Vector(0, 0, 1);
	}
	else if (pev->angles == Vector(0, -2, 0))
	{
		pev->movedir = Vector(0, 0, -1);
	}
	else
	{
		UTIL_MakeVectors(pev->angles);
		pev->movedir = gpGlobals->v_forward;
	}
	
	pev->angles = g_vecZero;
}

/*
=============
FEntIsVisible

returns TRUE if the passed entity is visible to caller, even if not infront ()
=============
*/
	BOOL
FEntIsVisible(
	entvars_t*		pev,
	entvars_t*		pevTarget)
	{
	Vector vecSpot1 = pev->origin + pev->view_ofs;
	Vector vecSpot2 = pevTarget->origin + pevTarget->view_ofs;
	TraceResult tr;

	UTIL_TraceLine(vecSpot1, vecSpot2, ignore_monsters, ENT(pev), &tr);
	
	if (tr.fInOpen && tr.fInWater)
		return FALSE;                   // sight line crossed contents

	if (tr.flFraction == 1)
		return TRUE;

	return FALSE;
	}


