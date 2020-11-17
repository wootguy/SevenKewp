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
#include "cbase.h"
#include "saverestore.h"
#include "nodes.h"
#include "CBaseDoor.h"

extern CGraph WorldGraph;

extern BOOL FEntIsVisible(entvars_t* pev, entvars_t* pevTarget);

extern DLL_GLOBAL int g_iSkillLevel;

// This updates global tables that need to know about entities being removed
void CBaseEntity::UpdateOnRemove( void )
{
	int	i;

	if ( FBitSet( pev->flags, FL_GRAPHED ) )
	{
	// this entity was a LinkEnt in the world node graph, so we must remove it from
	// the graph since we are removing it from the world.
		for ( i = 0 ; i < WorldGraph.m_cLinks ; i++ )
		{
			if ( WorldGraph.m_pLinkPool [ i ].m_pLinkEnt == pev )
			{
				// if this link has a link ent which is the same ent that is removing itself, remove it!
				WorldGraph.m_pLinkPool [ i ].m_pLinkEnt = NULL;
			}
		}
	}
	if ( pev->globalname )
		gGlobalState.EntitySetState( pev->globalname, GLOBAL_DEAD );
}

// Convenient way to delay removing oneself
void CBaseEntity :: SUB_Remove( void )
{
	UpdateOnRemove();
	if (pev->health > 0)
	{
		// this situation can screw up monsters who can't tell their entity pointers are invalid.
		pev->health = 0;
		ALERT( at_aiconsole, "SUB_Remove called on entity with health > 0\n");
	}

	REMOVE_ENTITY(ENT(pev));
}

// Convenient way to explicitly do nothing (passed to functions that require a method)
void CBaseEntity :: SUB_DoNothing( void )
{
}

/*
==============================
SUB_UseTargets

If self.delay is set, a DelayedUse entity will be created that will actually
do the SUB_UseTargets after that many seconds have passed.

Removes all entities with a targetname that match self.killtarget,
and removes them, so some events can remove other triggers.

Search for (string)targetname in all entities that
match (string)self.target and call their .use function (if they have one)

==============================
*/
void CBaseEntity :: SUB_UseTargets( CBaseEntity *pActivator, USE_TYPE useType, float value )
{
	//
	// fire targets
	//
	if (!FStringNull(pev->target))
	{
		FireTargets( STRING(pev->target), pActivator, this, useType, value );
	}
}


void FireTargets( const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	edict_t *pentTarget = NULL;
	if ( !targetName )
		return;

	ALERT( at_aiconsole, "Firing: (%s)\n", targetName );

	for (;;)
	{
		pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, targetName);
		if (FNullEnt(pentTarget))
			break;

		CBaseEntity *pTarget = CBaseEntity::Instance( pentTarget );
		if ( pTarget && !(pTarget->pev->flags & FL_KILLME) )	// Don't use dying ents
		{
			ALERT( at_aiconsole, "Found: %s, firing (%s)\n", STRING(pTarget->pev->classname), targetName );
			pTarget->Use( pActivator, pCaller, useType, value );
		}
	}
}

/*
QuakeEd only writes a single float for angles (bad idea), so up and down are
just constant angles.
*/
void SetMovedir( entvars_t *pev )
{
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


