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

===== util.cpp ========================================================

  Utility code.  Really not optional after all.

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include <time.h>
#include "shake.h"
#include "decals.h"
#include "CBasePlayer.h"
#include "weapons.h"
#include "gamerules.h"
#include <algorithm>
#include <fstream>
#include "user_messages.h"
#include <queue>
#include "studio.h"

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#define stat _stat
#endif

float UTIL_WeaponTimeBase( void )
{
#if defined( CLIENT_WEAPONS )
	return 0.0;
#else
	return gpGlobals->time;
#endif
}

static unsigned int glSeed = 0; 

unsigned int seed_table[ 256 ] =
{
	28985, 27138, 26457, 9451, 17764, 10909, 28790, 8716, 6361, 4853, 17798, 21977, 19643, 20662, 10834, 20103,
	27067, 28634, 18623, 25849, 8576, 26234, 23887, 18228, 32587, 4836, 3306, 1811, 3035, 24559, 18399, 315,
	26766, 907, 24102, 12370, 9674, 2972, 10472, 16492, 22683, 11529, 27968, 30406, 13213, 2319, 23620, 16823,
	10013, 23772, 21567, 1251, 19579, 20313, 18241, 30130, 8402, 20807, 27354, 7169, 21211, 17293, 5410, 19223,
	10255, 22480, 27388, 9946, 15628, 24389, 17308, 2370, 9530, 31683, 25927, 23567, 11694, 26397, 32602, 15031,
	18255, 17582, 1422, 28835, 23607, 12597, 20602, 10138, 5212, 1252, 10074, 23166, 19823, 31667, 5902, 24630,
	18948, 14330, 14950, 8939, 23540, 21311, 22428, 22391, 3583, 29004, 30498, 18714, 4278, 2437, 22430, 3439,
	28313, 23161, 25396, 13471, 19324, 15287, 2563, 18901, 13103, 16867, 9714, 14322, 15197, 26889, 19372, 26241,
	31925, 14640, 11497, 8941, 10056, 6451, 28656, 10737, 13874, 17356, 8281, 25937, 1661, 4850, 7448, 12744,
	21826, 5477, 10167, 16705, 26897, 8839, 30947, 27978, 27283, 24685, 32298, 3525, 12398, 28726, 9475, 10208,
	617, 13467, 22287, 2376, 6097, 26312, 2974, 9114, 21787, 28010, 4725, 15387, 3274, 10762, 31695, 17320,
	18324, 12441, 16801, 27376, 22464, 7500, 5666, 18144, 15314, 31914, 31627, 6495, 5226, 31203, 2331, 4668,
	12650, 18275, 351, 7268, 31319, 30119, 7600, 2905, 13826, 11343, 13053, 15583, 30055, 31093, 5067, 761,
	9685, 11070, 21369, 27155, 3663, 26542, 20169, 12161, 15411, 30401, 7580, 31784, 8985, 29367, 20989, 14203,
	29694, 21167, 10337, 1706, 28578, 887, 3373, 19477, 14382, 675, 7033, 15111, 26138, 12252, 30996, 21409,
	25678, 18555, 13256, 23316, 22407, 16727, 991, 9236, 5373, 29402, 6117, 15241, 27715, 19291, 19888, 19847
};

std::map<std::string, std::string> g_precachedModels;
std::set<std::string> g_missingModels;
std::set<std::string> g_precachedSounds;
std::set<std::string> g_precachedGeneric;
std::map<std::string, int> g_precachedEvents;
std::set<std::string> g_tryPrecacheModels;
std::set<std::string> g_tryPrecacheSounds;
std::set<std::string> g_tryPrecacheGeneric;
std::set<std::string> g_tryPrecacheEvents;
std::map<std::string, WavInfo> g_wavInfos;
Bsp g_bsp;

std::string g_mp3Command;

unsigned int U_Random( void ) 
{ 
	glSeed *= 69069; 
	glSeed += seed_table[ glSeed & 0xff ];
 
	return ( ++glSeed & 0x0fffffff ); 
} 

void U_Srand( unsigned int seed )
{
	glSeed = seed_table[ seed & 0xff ];
}

/*
=====================
UTIL_SharedRandomLong
=====================
*/
int UTIL_SharedRandomLong( unsigned int seed, int low, int high )
{
	unsigned int range;

	U_Srand( (int)seed + low + high );

	range = high - low + 1;
	if ( !(range - 1) )
	{
		return low;
	}
	else
	{
		int offset;
		int rnum;

		rnum = U_Random();

		offset = rnum % range;

		return (low + offset);
	}
}

/*
=====================
UTIL_SharedRandomFloat
=====================
*/
float UTIL_SharedRandomFloat( unsigned int seed, float low, float high )
{
	//
	unsigned int range;

	U_Srand( (int)seed + *(int *)&low + *(int *)&high );

	U_Random();
	U_Random();

	range = high - low;
	if ( !range )
	{
		return low;
	}
	else
	{
		int tensixrand;
		float offset;

		tensixrand = U_Random() & 65535;

		offset = (float)tensixrand / 65536.0;

		return (low + offset * range );
	}
}

void UTIL_ParametricRocket( entvars_t *pev, Vector vecOrigin, Vector vecAngles, edict_t *owner )
{	
	pev->startpos = vecOrigin;
	// Trace out line to end pos
	TraceResult tr;
	UTIL_MakeVectors( vecAngles );
	UTIL_TraceLine( pev->startpos, pev->startpos + gpGlobals->v_forward * 8192, ignore_monsters, owner, &tr);
	pev->endpos = tr.vecEndPos;

	// Now compute how long it will take based on current velocity
	Vector vecTravel = pev->endpos - pev->startpos;
	float travelTime = 0.0;
	if ( pev->velocity.Length() > 0 )
	{
		travelTime = vecTravel.Length() / pev->velocity.Length();
	}
	pev->starttime = gpGlobals->time;
	pev->impacttime = gpGlobals->time + travelTime;
}

int g_groupmask = 0;
int g_groupop = 0;

// Normal overrides
void UTIL_SetGroupTrace( int groupmask, int op )
{
	g_groupmask		= groupmask;
	g_groupop		= op;

	ENGINE_SETGROUPMASK( g_groupmask, g_groupop );
}

void UTIL_UnsetGroupTrace( void )
{
	g_groupmask		= 0;
	g_groupop		= 0;

	ENGINE_SETGROUPMASK( 0, 0 );
}

// Smart version, it'll clean itself up when it pops off stack
UTIL_GroupTrace::UTIL_GroupTrace( int groupmask, int op )
{
	m_oldgroupmask	= g_groupmask;
	m_oldgroupop	= g_groupop;

	g_groupmask		= groupmask;
	g_groupop		= op;

	ENGINE_SETGROUPMASK( g_groupmask, g_groupop );
}

UTIL_GroupTrace::~UTIL_GroupTrace( void )
{
	g_groupmask		=	m_oldgroupmask;
	g_groupop		=	m_oldgroupop;

	ENGINE_SETGROUPMASK( g_groupmask, g_groupop );
}

TYPEDESCRIPTION	gEntvarsDescription[] = 
{
	DEFINE_ENTITY_FIELD( classname, FIELD_STRING ),
	DEFINE_ENTITY_GLOBAL_FIELD( globalname, FIELD_STRING ),
	
	DEFINE_ENTITY_FIELD( origin, FIELD_POSITION_VECTOR ),
	DEFINE_ENTITY_FIELD( oldorigin, FIELD_POSITION_VECTOR ),
	DEFINE_ENTITY_FIELD( velocity, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( basevelocity, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( clbasevelocity, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( movedir, FIELD_VECTOR ),

	DEFINE_ENTITY_FIELD( angles, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( avelocity, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( punchangle, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( v_angle, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( endpos, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( startpos, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( impacttime, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( starttime, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( fixangle, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( idealpitch, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( pitch_speed, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( ideal_yaw, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( yaw_speed, FIELD_FLOAT ),

	DEFINE_ENTITY_FIELD( modelindex, FIELD_INTEGER ),
	DEFINE_ENTITY_GLOBAL_FIELD( model, FIELD_MODELNAME ),

	DEFINE_ENTITY_FIELD( viewmodel, FIELD_MODELNAME ),
	DEFINE_ENTITY_FIELD( weaponmodel, FIELD_MODELNAME ),

	DEFINE_ENTITY_FIELD( absmin, FIELD_POSITION_VECTOR ),
	DEFINE_ENTITY_FIELD( absmax, FIELD_POSITION_VECTOR ),
	DEFINE_ENTITY_GLOBAL_FIELD( mins, FIELD_VECTOR ),
	DEFINE_ENTITY_GLOBAL_FIELD( maxs, FIELD_VECTOR ),
	DEFINE_ENTITY_GLOBAL_FIELD( size, FIELD_VECTOR ),

	DEFINE_ENTITY_FIELD( ltime, FIELD_TIME ),
	DEFINE_ENTITY_FIELD( nextthink, FIELD_TIME ),

	DEFINE_ENTITY_FIELD( solid, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( movetype, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( skin, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( body, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( effects, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( gravity, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( friction, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( light_level, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( gaitsequence, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD( frame, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( scale, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( sequence, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( animtime, FIELD_TIME ),
	DEFINE_ENTITY_FIELD( framerate, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( controller, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( blending, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( rendermode, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( renderamt, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( rendercolor, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( renderfx, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( health, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( frags, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( weapons, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( takedamage, FIELD_FLOAT ),

	DEFINE_ENTITY_FIELD( deadflag, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( view_ofs, FIELD_VECTOR ),
	DEFINE_ENTITY_FIELD( button, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( impulse, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( chain, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( dmg_inflictor, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( enemy, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( aiment, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( owner, FIELD_EDICT ),
	DEFINE_ENTITY_FIELD( groundentity, FIELD_EDICT ),

	DEFINE_ENTITY_FIELD( spawnflags, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( flags, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD( colormap, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( team, FIELD_INTEGER ),

	DEFINE_ENTITY_FIELD( max_health, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( teleport_time, FIELD_TIME ),
	DEFINE_ENTITY_FIELD( armortype, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( armorvalue, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( waterlevel, FIELD_INTEGER ),
	DEFINE_ENTITY_FIELD( watertype, FIELD_INTEGER ),

	// Having these fields be local to the individual levels makes it easier to test those levels individually.
	DEFINE_ENTITY_GLOBAL_FIELD( target, FIELD_STRING ),
	DEFINE_ENTITY_GLOBAL_FIELD( targetname, FIELD_STRING ),
	DEFINE_ENTITY_FIELD( netname, FIELD_STRING ),
	DEFINE_ENTITY_FIELD( message, FIELD_STRING ),

	DEFINE_ENTITY_FIELD( dmg_take, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( dmg_save, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( dmg, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( dmgtime, FIELD_TIME ),

	DEFINE_ENTITY_FIELD( noise, FIELD_SOUNDNAME ),
	DEFINE_ENTITY_FIELD( noise1, FIELD_SOUNDNAME ),
	DEFINE_ENTITY_FIELD( noise2, FIELD_SOUNDNAME ),
	DEFINE_ENTITY_FIELD( noise3, FIELD_SOUNDNAME ),
	DEFINE_ENTITY_FIELD( speed, FIELD_FLOAT ),
	DEFINE_ENTITY_FIELD( air_finished, FIELD_TIME ),
	DEFINE_ENTITY_FIELD( pain_finished, FIELD_TIME ),
	DEFINE_ENTITY_FIELD( radsuit_finished, FIELD_TIME ),

	DEFINE_ENTITY_FIELD( pContainingEntity, FIELD_EDICT),
	DEFINE_ENTITY_FIELD( playerclass, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( maxspeed, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD( fov, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD( weaponanim, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( pushmsec, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( bInDuck, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( flTimeStepSound, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( flSwimTime, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( flDuckTime, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( iStepLeft, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( flFallVelocity, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD( gamestate, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( oldbuttons, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( groupinfo, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( iuser1, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( iuser2, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( iuser3, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( iuser4, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD( fuser1, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD( fuser2, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD( fuser3, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD( fuser4, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD( vuser1, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD( vuser2, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD( vuser3, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD( vuser4, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD( euser1, FIELD_EDICT),
	DEFINE_ENTITY_FIELD( euser2, FIELD_EDICT),
	DEFINE_ENTITY_FIELD( euser3, FIELD_EDICT),
	DEFINE_ENTITY_FIELD( euser4, FIELD_EDICT),
};

#define ENTVARS_COUNT		(sizeof(gEntvarsDescription)/sizeof(gEntvarsDescription[0]))


#ifdef	DEBUG
edict_t *DBG_EntOfVars( const entvars_t *pev )
{
	if (pev->pContainingEntity != NULL)
		return pev->pContainingEntity;
	ALERT(at_console, "entvars_t pContainingEntity is NULL, calling into engine");
	edict_t* pent = (*g_engfuncs.pfnFindEntityByVars)((entvars_t*)pev);
	if (pent == NULL)
		ALERT(at_console, "DAMN!  Even the engine couldn't FindEntityByVars!");
	((entvars_t *)pev)->pContainingEntity = pent;
	return pent;
}
#endif //DEBUG


#ifdef	DEBUG
	void
DBG_AssertFunction(
	BOOL		fExpr,
	const char*	szExpr,
	const char*	szFile,
	int			szLine,
	const char*	szMessage)
	{
	if (fExpr)
		return;
	char szOut[512];
	if (szMessage != NULL)
		snprintf(szOut, 512, "ASSERT FAILED:\n %s \n(%s@%d)\n%s", szExpr, szFile, szLine, szMessage);
	else
		snprintf(szOut, 512, "ASSERT FAILED:\n %s \n(%s@%d)", szExpr, szFile, szLine);
	ALERT(at_console, szOut);
	}
#endif	// DEBUG

BOOL UTIL_GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon )
{
	return g_pGameRules->GetNextBestWeapon( pPlayer, pCurrentWeapon );
}

// ripped this out of the engine
float	UTIL_AngleMod(float a)
{
	if (a < 0)
	{
		a = a + 360 * ((int)(a / 360) + 1);
	}
	else if (a >= 360)
	{
		a = a - 360 * ((int)(a / 360));
	}
	// a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	return a;
}

float UTIL_AngleDiff( float destAngle, float srcAngle )
{
	float delta;

	delta = destAngle - srcAngle;
	if ( destAngle > srcAngle )
	{
		if ( delta >= 180 )
			delta -= 360;
	}
	else
	{
		if ( delta <= -180 )
			delta += 360;
	}
	return delta;
}

Vector UTIL_VecToAngles( const Vector &vec )
{
	float rgflVecOut[3];
	VEC_TO_ANGLES(vec, rgflVecOut);
	return Vector(rgflVecOut);
}
	
//	float UTIL_MoveToOrigin( edict_t *pent, const Vector vecGoal, float flDist, int iMoveType )
void UTIL_MoveToOrigin( edict_t *pent, const Vector &vecGoal, float flDist, int iMoveType )
{
	float rgfl[3];
	vecGoal.CopyToArray(rgfl);
//		return MOVE_TO_ORIGIN ( pent, rgfl, flDist, iMoveType ); 
	MOVE_TO_ORIGIN ( pent, rgfl, flDist, iMoveType ); 
}


int UTIL_EntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask, bool ignoreDead )
{
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity *pEntity;
	int			count;

	count = 0;

	if ( !pEdict )
		return count;

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free )	// Not in use
			continue;
		
		if ( flagMask && !(pEdict->v.flags & flagMask) )	// Does it meet the criteria?
			continue;

		if (ignoreDead && pEdict->v.deadflag >= DEAD_DEAD)
			continue;

		if ( mins.x > pEdict->v.absmax.x ||
			 mins.y > pEdict->v.absmax.y ||
			 mins.z > pEdict->v.absmax.z ||
			 maxs.x < pEdict->v.absmin.x ||
			 maxs.y < pEdict->v.absmin.y ||
			 maxs.z < pEdict->v.absmin.z )
			 continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if ( !pEntity )
			continue;

		pList[ count ] = pEntity;
		count++;

		if ( count >= listMax )
			return count;
	}

	return count;
}


int UTIL_MonstersInSphere( CBaseEntity **pList, int listMax, const Vector &center, float radius )
{
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity *pEntity;
	int			count;
	float		distance, delta;

	count = 0;
	float radiusSquared = radius * radius;

	if ( !pEdict )
		return count;

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		if ( pEdict->free )	// Not in use
			continue;
		
		if ( !(pEdict->v.flags & (FL_CLIENT|FL_MONSTER)) )	// Not a client/monster ?
			continue;

		// Use origin for X & Y since they are centered for all monsters
		// Now X
		delta = center.x - pEdict->v.origin.x;//(pEdict->v.absmin.x + pEdict->v.absmax.x)*0.5;
		delta *= delta;

		if ( delta > radiusSquared )
			continue;
		distance = delta;
		
		// Now Y
		delta = center.y - pEdict->v.origin.y;//(pEdict->v.absmin.y + pEdict->v.absmax.y)*0.5;
		delta *= delta;

		distance += delta;
		if ( distance > radiusSquared )
			continue;

		// Now Z
		delta = center.z - (pEdict->v.absmin.z + pEdict->v.absmax.z)*0.5;
		delta *= delta;

		distance += delta;
		if ( distance > radiusSquared )
			continue;

		pEntity = CBaseEntity::Instance(pEdict);
		if ( !pEntity )
			continue;

		pList[ count ] = pEntity;
		count++;

		if ( count >= listMax )
			return count;
	}


	return count;
}


CBaseEntity *UTIL_FindEntityInSphere( CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius )
{
	edict_t	*pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = NULL;

	pentEntity = FIND_ENTITY_IN_SPHERE( pentEntity, vecCenter, flRadius);

	if (!FNullEnt(pentEntity))
		return CBaseEntity::Instance(pentEntity);
	return NULL;
}


CBaseEntity *UTIL_FindEntityByString( CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue )
{
	edict_t	*pentEntity;

	if (pStartEntity)
		pentEntity = pStartEntity->edict();
	else
		pentEntity = NULL;

	pentEntity = FIND_ENTITY_BY_STRING( pentEntity, szKeyword, szValue );

	if (!FNullEnt(pentEntity))
		return CBaseEntity::Instance(pentEntity);
	return NULL;
}

CBaseEntity *UTIL_FindEntityByClassname( CBaseEntity *pStartEntity, const char *szName )
{
	return UTIL_FindEntityByString( pStartEntity, "classname", szName );
}

CBaseEntity *UTIL_FindEntityByTargetname( CBaseEntity *pStartEntity, const char *szName )
{
	return UTIL_FindEntityByString( pStartEntity, "targetname", szName );
}


CBaseEntity *UTIL_FindEntityGeneric( const char *szWhatever, Vector &vecSrc, float flRadius )
{
	CBaseEntity *pEntity = NULL;

	pEntity = UTIL_FindEntityByTargetname( NULL, szWhatever );
	if (pEntity)
		return pEntity;

	CBaseEntity *pSearch = NULL;
	float flMaxDist2 = flRadius * flRadius;
	while ((pSearch = UTIL_FindEntityByClassname( pSearch, szWhatever )) != NULL)
	{
		float flDist2 = (pSearch->pev->origin - vecSrc).Length();
		flDist2 = flDist2 * flDist2;
		if (flMaxDist2 > flDist2)
		{
			pEntity = pSearch;
			flMaxDist2 = flDist2;
		}
	}
	return pEntity;
}


// returns a CBaseEntity pointer to a player by index.  Only returns if the player is spawned and connected
// otherwise returns NULL
// Index is 1 based
CBaseEntity	*UTIL_PlayerByIndex( int playerIndex )
{
	CBaseEntity *pPlayer = NULL;

	if ( playerIndex > 0 && playerIndex <= gpGlobals->maxClients )
	{
		edict_t *pPlayerEdict = INDEXENT( playerIndex );
		if ( IsValidPlayer(pPlayerEdict) && !pPlayerEdict->free )
		{
			pPlayer = CBaseEntity::Instance( pPlayerEdict );
		}
	}
	
	return pPlayer;
}

edict_t* UTIL_ClientsInPVS(edict_t* edict, int& playerCount) {
	// TODO: reimplement engine func so that it only iterates 32 edicts
	edict_t* pvsents = UTIL_EntitiesInPVS(edict);
	playerCount = 0;

	if (FNullEnt(pvsents)) {
		return pvsents;
	}

	edict_t* newList = INDEXENT(0);
	edict_t* lastPlr = newList;
	edict_t* ed = pvsents;

	// unlink edicts which aren't clients
	while (!FNullEnt(ed)) {
		if (ed->v.flags & FL_CLIENT) {
			if (FNullEnt(newList)) {
				newList = ed;
			}
			else {
				lastPlr->v.chain = ed;
			}

			lastPlr = ed;
			ed = ed->v.chain;
			playerCount++;
		}
		else {
			ed = ed->v.chain;
			lastPlr->v.chain = ed;
		}
	}
	lastPlr->v.chain = INDEXENT(0);

	return newList;
}

bool UTIL_IsClientInPVS(edict_t* edict) {
	CBaseEntity* ent = (CBaseEntity*)GET_PRIVATE(edict);
	return ent && ent->m_visiblePlayers;
}

bool IsValidPlayer(edict_t* edict) {
	return edict && (edict->v.flags & FL_CLIENT) && STRING(edict->v.netname)[0] != '\0';
}

void UTIL_MakeVectors( const Vector &vecAngles )
{
	MAKE_VECTORS( vecAngles );
}


void UTIL_MakeAimVectors( const Vector &vecAngles )
{
	float rgflVec[3];
	vecAngles.CopyToArray(rgflVec);
	rgflVec[0] = -rgflVec[0];
	MAKE_VECTORS(rgflVec);
}


#define SWAP(a,b,temp)	((temp)=(a),(a)=(b),(b)=(temp))

void UTIL_MakeInvVectors( const Vector &vec, globalvars_t *pgv )
{
	MAKE_VECTORS(vec);

	float tmp;
	pgv->v_right = pgv->v_right * -1;

	SWAP(pgv->v_forward.y, pgv->v_right.x, tmp);
	SWAP(pgv->v_forward.z, pgv->v_up.x, tmp);
	SWAP(pgv->v_right.z, pgv->v_up.y, tmp);
}

// copied from rehlds
void ambientsound_msg(edict_t* entity, float* pos, const char* samp, float vol, float attenuation, 
	int fFlags, int pitch, int msgDst, edict_t* dest)
{
	int soundnum;

	if (samp[0] == '!')
	{
		fFlags |= SND_SENTENCE;
		soundnum = atoi(samp + 1);
		if (soundnum >= CVOXFILESENTENCEMAX)
		{
			ALERT(at_error, "invalid sentence number: %s", &samp[1]);
			return;
		}
	}
	else
	{
		soundnum = PRECACHE_SOUND_ENT(NULL, samp);
	}

	MESSAGE_BEGIN(msgDst, SVC_SPAWNSTATICSOUND, pos, dest);
	WRITE_COORD(pos[0]);
	WRITE_COORD(pos[1]);
	WRITE_COORD(pos[2]);

	WRITE_SHORT(soundnum);
	WRITE_BYTE(vol * 255.0);
	WRITE_BYTE(attenuation * 64.0);
	WRITE_SHORT(ENTINDEX(entity));
	WRITE_BYTE(pitch);
	WRITE_BYTE(fFlags);
	MESSAGE_END();
}

void WRITE_BYTES(uint8_t* bytes, int count) {
	int longCount = count / 4;
	int byteCount = count % 4;

	int32_t* longPtr = (int32_t*)bytes;
	for (int i = 0; i < longCount; i++) {
		WRITE_LONG(*longPtr);
		longPtr++;
	}

	uint8_t* bytePtr = bytes + longCount*4;
	for (int i = 0; i < byteCount; i++) {
		WRITE_BYTE(*bytePtr);
		bytePtr++;
	}
}

void UTIL_EmitAmbientSound( edict_t *entity, const float* vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch, edict_t* dest)
{
	float rgfl[3];
	memcpy(rgfl, vecOrigin, 3 * sizeof(float));

	int eidx = ENTINDEX(entity);

	if (entity->v.flags & FL_MONSTER) {
		if (g_monsterSoundReplacements[eidx].find(samp) != g_monsterSoundReplacements[eidx].end()) {
			samp = g_monsterSoundReplacements[eidx][samp].c_str();
		}
		else if (g_soundReplacements.find(samp) != g_soundReplacements.end()) {
			samp = g_soundReplacements[samp].c_str();
		}
	}
	else if (g_soundReplacements.find(samp) != g_soundReplacements.end()) {
		samp = g_soundReplacements[samp].c_str();
	}

	if (!UTIL_isSafeEntIndex(eidx, "emit static sound")) {
		return;
	}

	if (samp && *samp == '!')
	{
		char name[32];
		if (SENTENCEG_Lookup(samp, name, 32) >= 0) {
			if (dest) {
				ambientsound_msg(entity, rgfl, name, vol, attenuation, fFlags, pitch, MSG_ONE, dest);
			}
			else {
				EMIT_AMBIENT_SOUND(entity, rgfl, name, vol, attenuation, fFlags, pitch);
			}
			
		}
	}
	else {
		if (dest) {
			ambientsound_msg(entity, rgfl, samp, vol, attenuation, fFlags, pitch, MSG_ONE, dest);
		}
		else {
			EMIT_AMBIENT_SOUND(entity, rgfl, samp, vol, attenuation, fFlags, pitch);
		}
	}
}

void UTIL_PlayGlobalMp3(const char* path, bool loop, edict_t* target) {
	// surround with ; to prevent multiple commands being joined when sent in the same frame(?)
	// this fixes music sometimes not loading/starting/stopping
	std::string mp3Command = UTIL_VarArgs(";mp3 %s sound/%s;", (loop ? "loop" : "play"), path);
	
	MESSAGE_BEGIN(target ? MSG_ONE : MSG_ALL, SVC_STUFFTEXT, NULL, target);
	WRITE_STRING(mp3Command.c_str());
	MESSAGE_END();

	if (!target) {
		g_mp3Command = mp3Command;
		ALERT(at_console, "MP3 Command: '%s'\n", g_mp3Command.c_str());
	}
}

void UTIL_StopGlobalMp3(edict_t* target) {
	const char* cmd = ";mp3 stop;";

	MESSAGE_BEGIN(target ? MSG_ONE : MSG_ALL, SVC_STUFFTEXT, NULL, target);
	WRITE_STRING(cmd);
	//WRITE_STRING(";cd fadeout;"); // blocked by cl_filterstuffcmd
	MESSAGE_END();

	if (!target) {
		g_mp3Command = "";
		ALERT(at_console, "MP3 Command: '%s'\n", cmd);
	}
}

static unsigned short FixedUnsigned16( float value, float scale )
{
	int output;

	output = value * scale;
	if ( output < 0 )
		output = 0;
	if ( output > 0xFFFF )
		output = 0xFFFF;

	return (unsigned short)output;
}

static short FixedSigned16( float value, float scale )
{
	int output;

	output = value * scale;

	if ( output > 32767 )
		output = 32767;

	if ( output < -32768 )
		output = -32768;

	return (short)output;
}

// Shake the screen of all clients within radius
// radius == 0, shake all clients
// UNDONE: Allow caller to shake clients not ONGROUND?
// UNDONE: Fix falloff model (disabled)?
// UNDONE: Affect user controls?
void UTIL_ScreenShake( const Vector &center, float amplitude, float frequency, float duration, float radius )
{
	int			i;
	float		localAmplitude;
	ScreenShake	shake;

	shake.duration = FixedUnsigned16( duration, 1<<12 );		// 4.12 fixed
	shake.frequency = FixedUnsigned16( frequency, 1<<8 );	// 8.8 fixed

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

		if ( !pPlayer || !(pPlayer->pev->flags & FL_ONGROUND) )	// Don't shake if not onground
			continue;

		localAmplitude = 0;

		if ( radius <= 0 )
			localAmplitude = amplitude;
		else
		{
			Vector delta = center - pPlayer->pev->origin;
			float distance = delta.Length();
	
			// Had to get rid of this falloff - it didn't work well
			if ( distance < radius )
				localAmplitude = amplitude;//radius - distance;
		}
		if ( localAmplitude )
		{
			shake.amplitude = FixedUnsigned16( localAmplitude, 1<<12 );		// 4.12 fixed
			
			MESSAGE_BEGIN( MSG_ONE, gmsgShake, NULL, pPlayer->edict() );		// use the magic #1 for "one client"
				
				WRITE_SHORT( shake.amplitude );				// shake amount
				WRITE_SHORT( shake.duration );				// shake lasts this long
				WRITE_SHORT( shake.frequency );				// shake noise frequency

			MESSAGE_END();
		}
	}
}



void UTIL_ScreenShakeAll( const Vector &center, float amplitude, float frequency, float duration )
{
	UTIL_ScreenShake( center, amplitude, frequency, duration, 0 );
}


void UTIL_ScreenFadeBuild( ScreenFade &fade, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags )
{
	fade.duration = FixedUnsigned16( fadeTime, 1<<12 );		// 4.12 fixed
	fade.holdTime = FixedUnsigned16( fadeHold, 1<<12 );		// 4.12 fixed
	fade.r = (int)color.x;
	fade.g = (int)color.y;
	fade.b = (int)color.z;
	fade.a = alpha;
	fade.fadeFlags = flags;
}


void UTIL_ScreenFadeWrite( const ScreenFade &fade, CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsNetClient() )
		return;

	MESSAGE_BEGIN( MSG_ONE, gmsgFade, NULL, pEntity->edict() );		// use the magic #1 for "one client"
		
		WRITE_SHORT( fade.duration );		// fade lasts this long
		WRITE_SHORT( fade.holdTime );		// fade lasts this long
		WRITE_SHORT( fade.fadeFlags );		// fade type (in / out)
		WRITE_BYTE( fade.r );				// fade red
		WRITE_BYTE( fade.g );				// fade green
		WRITE_BYTE( fade.b );				// fade blue
		WRITE_BYTE( fade.a );				// fade blue

	MESSAGE_END();
}


void UTIL_ScreenFadeAll( const Vector &color, float fadeTime, float fadeHold, int alpha, int flags )
{
	int			i;
	ScreenFade	fade;


	UTIL_ScreenFadeBuild( fade, color, fadeTime, fadeHold, alpha, flags );

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
	
		UTIL_ScreenFadeWrite( fade, pPlayer );
	}
}


void UTIL_ScreenFade( CBaseEntity *pEntity, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags )
{
	ScreenFade	fade;

	UTIL_ScreenFadeBuild( fade, color, fadeTime, fadeHold, alpha, flags );
	UTIL_ScreenFadeWrite( fade, pEntity );
}

// client crashes if lines are longer than ~80 characters
// server crashes if sending text > 511 characters
// so truncate and insert newlines to prevent that
const char* BreakupLongLines(const char* pMessage) {
	static char tmp[512];
	static char tmp2[512];

	if (strlen(pMessage) >= 512) {
		strncpy(tmp, pMessage, 511);
		tmp[511] = 0;
		pMessage = tmp;
	}

	int len = strlen(pMessage);

	if (len >= 79) {
		int lastNewline = 0;
		int lastSpace = 0;
		int lastSpaceIdx = 0;
		int idx = 0;

		for (int i = 0; i < len; i++) {
			if (pMessage[i] == '\n') {
				lastNewline = i;
			}
			if (pMessage[i] == ' ' || pMessage[i] == '\t') {
				lastSpace = i;
				lastSpaceIdx = idx;
			}
			if (i - lastNewline >= 79) {
				if (idx + 1 >= 511) {
					break;
				}

				if (i - lastSpaceIdx < 60) {
					// was there a somewhat recent space? Break the line
					// and reiterate from there so that words stay intact
					i = lastSpace;
					lastNewline = i;
					idx = lastSpaceIdx;
					tmp2[idx++] = '\n';
					continue;
				}

				tmp2[idx++] = '\n';
				tmp2[idx++] = pMessage[i];
				lastNewline = i;
			}
			else {
				if (idx >= 511) {
					break;
				}
				tmp2[idx++] = pMessage[i];
			}
			tmp2[idx] = 0;
		}
		pMessage = tmp2;
	}

	return pMessage;
}

void UTIL_HudMessage( CBaseEntity *pEntity, const hudtextparms_t &textparms, const char *pMessage )
{
	if ( !pEntity || !pEntity->IsNetClient() )
		return;

	pMessage = BreakupLongLines(pMessage);

	MESSAGE_BEGIN( MSG_ONE, SVC_TEMPENTITY, NULL, pEntity->edict() );
		WRITE_BYTE( TE_TEXTMESSAGE );
		WRITE_BYTE( textparms.channel & 0xFF );

		WRITE_SHORT( FixedSigned16( textparms.x, 1<<13 ) );
		WRITE_SHORT( FixedSigned16( textparms.y, 1<<13 ) );
		WRITE_BYTE( textparms.effect );

		WRITE_BYTE( textparms.r1 );
		WRITE_BYTE( textparms.g1 );
		WRITE_BYTE( textparms.b1 );
		WRITE_BYTE( textparms.a1 );

		WRITE_BYTE( textparms.r2 );
		WRITE_BYTE( textparms.g2 );
		WRITE_BYTE( textparms.b2 );
		WRITE_BYTE( textparms.a2 );

		WRITE_SHORT( FixedUnsigned16( textparms.fadeinTime, 1<<8 ) );
		WRITE_SHORT( FixedUnsigned16( textparms.fadeoutTime, 1<<8 ) );
		WRITE_SHORT( FixedUnsigned16( textparms.holdTime, 1<<8 ) );

		if ( textparms.effect == 2 )
			WRITE_SHORT( FixedUnsigned16( textparms.fxTime, 1<<8 ) );

		WRITE_STRING( pMessage );
	MESSAGE_END();
}

void UTIL_HudMessageAll( const hudtextparms_t &textparms, const char *pMessage )
{
	int			i;

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer )
			UTIL_HudMessage( pPlayer, textparms, pMessage );
	}
}

					 
extern int gmsgTextMsg, gmsgSayText;
void UTIL_ClientPrintAll( int msg_dest, const char *msg )
{
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		edict_t* ent = INDEXENT(i);
		if (IsValidPlayer(ent))
			CLIENT_PRINTF(ent, (PRINT_TYPE)msg_dest, msg);
	}
}

void UTIL_ClientPrint( edict_t* client, int msg_dest, const char * msg)
{
	CLIENT_PRINTF(client, (PRINT_TYPE)msg_dest, msg);
}

void UTIL_SayText( const char *pText, CBaseEntity *pEntity )
{
	if ( !pEntity->IsNetClient() )
		return;

	MESSAGE_BEGIN( MSG_ONE, gmsgSayText, NULL, pEntity->edict() );
		WRITE_BYTE( pEntity->entindex() );
		WRITE_STRING( pText );
	MESSAGE_END();
}

void UTIL_SayTextAll( const char *pText, CBaseEntity *pEntity )
{
	MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
		WRITE_BYTE( pEntity->entindex() );
		WRITE_STRING( pText );
	MESSAGE_END();
}


char *UTIL_dtos1( int d )
{
	static char buf[8];
	snprintf( buf, 8, "%d", d );
	return buf;
}

char *UTIL_dtos2( int d )
{
	static char buf[8];
	snprintf( buf, 8, "%d", d );
	return buf;
}

char *UTIL_dtos3( int d )
{
	static char buf[8];
	snprintf( buf, 8, "%d", d );
	return buf;
}

char *UTIL_dtos4( int d )
{
	static char buf[8];
	snprintf( buf, 8, "%d", d );
	return buf;
}

void UTIL_ShowMessage( const char *pString, CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsNetClient() )
		return;

	MESSAGE_BEGIN( MSG_ONE, gmsgHudText, NULL, pEntity->edict() );
	WRITE_STRING( BreakupLongLines(pString) );
	MESSAGE_END();
}


void UTIL_ShowMessageAll( const char *pString )
{
	int		i;

	// loop through all players

	for ( i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );
		if ( pPlayer )
			UTIL_ShowMessage( pString, pPlayer );
	}
}

// Overloaded to add IGNORE_GLASS
void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_LINE( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE) | (ignoreGlass?0x100:0), pentIgnore, ptr );
}


void UTIL_TraceLine( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_LINE( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), pentIgnore, ptr );
}


void UTIL_TraceHull( const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr )
{
	TRACE_HULL( vecStart, vecEnd, (igmon == ignore_monsters ? TRUE : FALSE), hullNumber, pentIgnore, ptr );
}

void UTIL_TraceModel( const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr )
{
	g_engfuncs.pfnTraceModel( vecStart, vecEnd, hullNumber, pentModel, ptr );
}


TraceResult UTIL_GetGlobalTrace( )
{
	TraceResult tr;

	tr.fAllSolid		= gpGlobals->trace_allsolid;
	tr.fStartSolid		= gpGlobals->trace_startsolid;
	tr.fInOpen			= gpGlobals->trace_inopen;
	tr.fInWater			= gpGlobals->trace_inwater;
	tr.flFraction		= gpGlobals->trace_fraction;
	tr.flPlaneDist		= gpGlobals->trace_plane_dist;
	tr.pHit			= gpGlobals->trace_ent;
	tr.vecEndPos		= gpGlobals->trace_endpos;
	tr.vecPlaneNormal	= gpGlobals->trace_plane_normal;
	tr.iHitgroup		= gpGlobals->trace_hitgroup;
	return tr;
}

	
void UTIL_SetSize( entvars_t *pev, const Vector &vecMin, const Vector &vecMax )
{
	SET_SIZE( ENT(pev), vecMin, vecMax );
}
	
	
float UTIL_VecToYaw( const Vector &vec )
{
	return VEC_TO_YAW(vec);
}


void UTIL_SetOrigin( entvars_t *pev, const Vector &vecOrigin )
{
	edict_t *ent = ENT(pev);
	if ( ent )
		SET_ORIGIN( ent, vecOrigin );
}

void UTIL_ParticleEffect( const Vector &vecOrigin, const Vector &vecDirection, ULONG ulColor, ULONG ulCount )
{
	PARTICLE_EFFECT( vecOrigin, vecDirection, (float)ulColor, (float)ulCount );
}


float UTIL_Approach( float target, float value, float speed )
{
	float delta = target - value;

	if ( delta > speed )
		value += speed;
	else if ( delta < -speed )
		value -= speed;
	else 
		value = target;

	return value;
}


float UTIL_ApproachAngle( float target, float value, float speed )
{
	target = UTIL_AngleMod( target );
	value = UTIL_AngleMod( target );
	
	float delta = target - value;

	// Speed is assumed to be positive
	if ( speed < 0 )
		speed = -speed;

	if ( delta < -180 )
		delta += 360;
	else if ( delta > 180 )
		delta -= 360;

	if ( delta > speed )
		value += speed;
	else if ( delta < -speed )
		value -= speed;
	else 
		value = target;

	return value;
}


float UTIL_AngleDistance( float next, float cur )
{
	float delta = next - cur;

	if ( delta < -180 )
		delta += 360;
	else if ( delta > 180 )
		delta -= 360;

	return delta;
}


float UTIL_SplineFraction( float value, float scale )
{
	value = scale * value;
	float valueSquared = value * value;

	// Nice little ease-in, ease-out spline-like curve
	return 3 * valueSquared - 2 * valueSquared * value;
}


char* UTIL_VarArgs( const char *format, ... )
{
	va_list		argptr;
	static char		string[1024];
	
	va_start (argptr, format);
	vsnprintf (string, 1024, format, argptr);
	va_end (argptr);

	return string;	
}
	
Vector UTIL_GetAimVector( edict_t *pent, float flSpeed )
{
	Vector tmp;
	GET_AIM_VECTOR(pent, flSpeed, tmp);
	return tmp;
}

int UTIL_IsMasterTriggered(string_t sMaster, CBaseEntity *pActivator)
{
	if (sMaster)
	{
		edict_t *pentTarget = NULL;
	
		while (!FNullEnt(pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(sMaster))))
		{
			CBaseEntity *pMaster = CBaseEntity::Instance(pentTarget);
			if ( pMaster && (pMaster->ObjectCaps() & FCAP_MASTER) )
				return pMaster->IsTriggered( pActivator );
		}

		ALERT(at_console, "Master was null or not a master!\n");
	}

	// if this isn't a master entity, just say yes.
	return 1;
}

BOOL UTIL_ShouldShowBlood( int color )
{
	if ( color != DONT_BLEED )
	{
		if ( color == BLOOD_COLOR_RED )
		{
			if ( CVAR_GET_FLOAT("violence_hblood") != 0 )
				return TRUE;
		}
		else
		{
			if ( CVAR_GET_FLOAT("violence_ablood") != 0 )
				return TRUE;
		}
	}
	return FALSE;
}

int UTIL_PointContents(	const Vector &vec )
{
	return POINT_CONTENTS(vec);
}

void UTIL_BloodStream( const Vector &origin, const Vector &direction, int color, int amount )
{
	if ( !UTIL_ShouldShowBlood( color ) )
		return;

	if ( g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED )
		color = 0;

	
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, origin );
		WRITE_BYTE( TE_BLOODSTREAM );
		WRITE_COORD( origin.x );
		WRITE_COORD( origin.y );
		WRITE_COORD( origin.z );
		WRITE_COORD( direction.x );
		WRITE_COORD( direction.y );
		WRITE_COORD( direction.z );
		WRITE_BYTE( color );
		WRITE_BYTE( V_min( amount, 255 ) );
	MESSAGE_END();
}				

void UTIL_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount )
{
	if ( !UTIL_ShouldShowBlood( color ) )
		return;

	if ( color == DONT_BLEED || amount == 0 )
		return;

	if ( g_Language == LANGUAGE_GERMAN && color == BLOOD_COLOR_RED )
		color = 0;

	if ( g_pGameRules->IsMultiplayer() )
	{
		// scale up blood effect in multiplayer for better visibility
		amount *= 2;
	}

	if ( amount > 255 )
		amount = 255;

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, origin );
		WRITE_BYTE( TE_BLOODSPRITE );
		WRITE_COORD( origin.x);								// pos
		WRITE_COORD( origin.y);
		WRITE_COORD( origin.z);
		WRITE_SHORT( g_sModelIndexBloodSpray );				// initial sprite model
		WRITE_SHORT( g_sModelIndexBloodDrop );				// droplet sprite models
		WRITE_BYTE( color );								// color index into host_basepal
		WRITE_BYTE( V_min( V_max( 3, amount / 10 ), 16 ) );		// size
	MESSAGE_END();
}				

Vector UTIL_RandomBloodVector( void )
{
	Vector direction;

	direction.x = RANDOM_FLOAT ( -1, 1 );
	direction.y = RANDOM_FLOAT ( -1, 1 );
	direction.z = RANDOM_FLOAT ( 0, 1 );

	return direction;
}


void UTIL_BloodDecalTrace( TraceResult *pTrace, int bloodColor )
{
	if ( UTIL_ShouldShowBlood( bloodColor ) )
	{
		if ( bloodColor == BLOOD_COLOR_RED )
			UTIL_DecalTrace( pTrace, DECAL_BLOOD1 + RANDOM_LONG(0,5) );
		else
			UTIL_DecalTrace( pTrace, DECAL_YBLOOD1 + RANDOM_LONG(0,5) );
	}
}


void UTIL_DecalTrace( TraceResult *pTrace, int decalNumber )
{
	short entityIndex;
	int index;
	int message;

	if ( decalNumber < 0 )
		return;

	index = gDecals[ decalNumber ].index;

	if ( index < 0 )
		return;

	if (pTrace->flFraction == 1.0)
		return;

	// Only decal BSP models
	if ( pTrace->pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( pTrace->pHit );
		if ( pEntity && !pEntity->IsBSPModel() )
			return;
		entityIndex = ENTINDEX( pTrace->pHit );
	}
	else 
		entityIndex = 0;

	message = TE_DECAL;
	if ( entityIndex != 0 )
	{
		if ( index > 255 )
		{
			message = TE_DECALHIGH;
			index -= 256;
		}
	}
	else
	{
		message = TE_WORLDDECAL;
		if ( index > 255 )
		{
			message = TE_WORLDDECALHIGH;
			index -= 256;
		}
	}
	
	if (UTIL_isSafeEntIndex(entityIndex, "apply decal")) {
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(message);
		WRITE_COORD(pTrace->vecEndPos.x);
		WRITE_COORD(pTrace->vecEndPos.y);
		WRITE_COORD(pTrace->vecEndPos.z);
		WRITE_BYTE(index);
		if (entityIndex)
			WRITE_SHORT(entityIndex);
		MESSAGE_END();
	}
}

/*
==============
UTIL_PlayerDecalTrace

A player is trying to apply his custom decal for the spray can.
Tell connected clients to display it, or use the default spray can decal
if the custom can't be loaded.
==============
*/
void UTIL_PlayerDecalTrace( TraceResult *pTrace, int playernum, int decalNumber, BOOL bIsCustom )
{
	int index;
	
	if (!bIsCustom)
	{
		if ( decalNumber < 0 )
			return;

		index = gDecals[ decalNumber ].index;
		if ( index < 0 )
			return;
	}
	else
		index = decalNumber;

	if (pTrace->flFraction == 1.0)
		return;

	if (UTIL_isSafeEntIndex(ENTINDEX(pTrace->pHit), "apply player decal")) {
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_PLAYERDECAL);
		WRITE_BYTE(playernum);
		WRITE_COORD(pTrace->vecEndPos.x);
		WRITE_COORD(pTrace->vecEndPos.y);
		WRITE_COORD(pTrace->vecEndPos.z);
		WRITE_SHORT((short)ENTINDEX(pTrace->pHit));
		WRITE_BYTE(index);
		MESSAGE_END();
	}
}

void UTIL_GunshotDecalTrace( TraceResult *pTrace, int decalNumber )
{
	if ( decalNumber < 0 )
		return;

	int index = gDecals[ decalNumber ].index;
	if ( index < 0 )
		return;

	if (pTrace->flFraction == 1.0)
		return;

	if (UTIL_isSafeEntIndex(ENTINDEX(pTrace->pHit), "apply gunshot decal")) {
		MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pTrace->vecEndPos);
		WRITE_BYTE(TE_GUNSHOTDECAL);
		WRITE_COORD(pTrace->vecEndPos.x);
		WRITE_COORD(pTrace->vecEndPos.y);
		WRITE_COORD(pTrace->vecEndPos.z);
		WRITE_SHORT((short)ENTINDEX(pTrace->pHit));
		WRITE_BYTE(index);
		MESSAGE_END();
	}
}


void UTIL_Sparks( const Vector &position )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
		WRITE_BYTE( TE_SPARKS );
		WRITE_COORD( position.x );
		WRITE_COORD( position.y );
		WRITE_COORD( position.z );
	MESSAGE_END();
}


void UTIL_Ricochet( const Vector &position, float scale )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, position );
		WRITE_BYTE( TE_ARMOR_RICOCHET );
		WRITE_COORD( position.x );
		WRITE_COORD( position.y );
		WRITE_COORD( position.z );
		WRITE_BYTE( (int)(scale*10) );
	MESSAGE_END();
}

void UTIL_Shrapnel(Vector pos, Vector dir, float flDamage, int bitsDamageType) {
	Vector sprPos = pos - Vector(0, 0, 10);
	bool isBlast = bitsDamageType & DMG_BLAST;
	int gibCount = V_min(128, flDamage / 10);

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pos);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(sprPos.x);
	WRITE_COORD(sprPos.y);
	WRITE_COORD(sprPos.z);
	WRITE_SHORT(g_sModelIndexShrapnelHit);
	WRITE_BYTE(V_min(8, RANDOM_LONG(3, 4) + (flDamage / 20)));
	WRITE_BYTE(50); // framerate
	WRITE_BYTE(2 | 4 | 8);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pos);
	WRITE_BYTE(TE_BREAKMODEL);
	WRITE_COORD(pos.x);
	WRITE_COORD(pos.y);
	WRITE_COORD(pos.z);
	WRITE_COORD(0);
	WRITE_COORD(0);
	WRITE_COORD(0);
	WRITE_COORD(dir.x);
	WRITE_COORD(dir.y);
	WRITE_COORD(dir.z);
	WRITE_BYTE(isBlast ? 30 : 15); // randomization
	WRITE_SHORT(g_sModelIndexShrapnel); // model id#
	WRITE_BYTE(gibCount);
	WRITE_BYTE(1);// duration 0.1 seconds
	WRITE_BYTE(0); // flags
	MESSAGE_END();

	// saving this in case it's useful for a similar effect. The sounds make more sense for small gibs
	// and they have higher gravity and less bounce. Much higher network usage for lots of gibs though.
	/*
	for (int i = 0; i < gibCount; i++) {
		EjectBrass(pos, dir * 200, RANDOM_LONG(0, 1.0f), MODEL_INDEX("models/shrapnel.mdl"), TE_BOUNCE_SHELL);
	}
	*/
}


BOOL UTIL_TeamsMatch( const char *pTeamName1, const char *pTeamName2 )
{
	// Everyone matches unless it's teamplay
	if ( !g_pGameRules->IsTeamplay() )
		return TRUE;

	// Both on a team?
	if ( *pTeamName1 != 0 && *pTeamName2 != 0 )
	{
		if ( !stricmp( pTeamName1, pTeamName2 ) )	// Same Team?
			return TRUE;
	}

	return FALSE;
}


void UTIL_StringToVector( float *pVector, const char *pString )
{
	char *pstr, *pfront, tempString[128];
	int	j;

	strcpy_safe( tempString, pString, 128 );
	pstr = pfront = tempString;

	for ( j = 0; j < 3; j++ )			// lifted from pr_edict.c
	{
		pVector[j] = atof( pfront );

		while ( *pstr && *pstr != ' ' )
			pstr++;
		if (!*pstr)
			break;
		pstr++;
		pfront = pstr;
	}
	if (j < 2)
	{
		/*
		ALERT( at_error, "Bad field in entity!! %s:%s == \"%s\"\n",
			pkvd->szClassName, pkvd->szKeyName, pkvd->szValue );
		*/
		for (j = j+1;j < 3; j++)
			pVector[j] = 0;
	}
}

bool UTIL_StringIsVector(const char* pString) {
	int j;
	const char *pstr = pString;

	for (j = 0; j < 3; j++)
	{
		while (*pstr && *pstr != ' ')
			pstr++;
		if (!*pstr)
			break;
		pstr++;
	}

	return j > 2;
}

void UTIL_StringToIntArray( int *pVector, int count, const char *pString )
{
	char *pstr, *pfront, tempString[128];
	int	j;

	strcpy_safe( tempString, pString, 128 );
	pstr = pfront = tempString;

	for ( j = 0; j < count; j++ )			// lifted from pr_edict.c
	{
		pVector[j] = atoi( pfront );

		while ( *pstr && *pstr != ' ' )
			pstr++;
		if (!*pstr)
			break;
		pstr++;
		pfront = pstr;
	}

	for ( j++; j < count; j++ )
	{
		pVector[j] = 0;
	}
}

Vector UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize )
{
	Vector sourceVector = input;

	if ( sourceVector.x > clampSize.x )
		sourceVector.x -= clampSize.x;
	else if ( sourceVector.x < -clampSize.x )
		sourceVector.x += clampSize.x;
	else
		sourceVector.x = 0;

	if ( sourceVector.y > clampSize.y )
		sourceVector.y -= clampSize.y;
	else if ( sourceVector.y < -clampSize.y )
		sourceVector.y += clampSize.y;
	else
		sourceVector.y = 0;
	
	if ( sourceVector.z > clampSize.z )
		sourceVector.z -= clampSize.z;
	else if ( sourceVector.z < -clampSize.z )
		sourceVector.z += clampSize.z;
	else
		sourceVector.z = 0;

	return sourceVector.Normalize();
}


float UTIL_WaterLevel( const Vector &position, float minz, float maxz )
{
	Vector midUp = position;
	midUp.z = minz;

	if (UTIL_PointContents(midUp) != CONTENTS_WATER)
		return minz;

	midUp.z = maxz;
	if (UTIL_PointContents(midUp) == CONTENTS_WATER)
		return maxz;

	float diff = maxz - minz;
	while (diff > 1.0)
	{
		midUp.z = minz + diff/2.0;
		if (UTIL_PointContents(midUp) == CONTENTS_WATER)
		{
			minz = midUp.z;
		}
		else
		{
			maxz = midUp.z;
		}
		diff = maxz - minz;
	}

	return midUp.z;
}


extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model

void UTIL_Bubbles( Vector mins, Vector maxs, int count )
{
	Vector mid =  (mins + maxs) * 0.5;

	float flHeight = UTIL_WaterLevel( mid,  mid.z, mid.z + 1024 );
	flHeight = flHeight - mins.z;

	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, mid );
		WRITE_BYTE( TE_BUBBLES );
		WRITE_COORD( mins.x );	// mins
		WRITE_COORD( mins.y );
		WRITE_COORD( mins.z );
		WRITE_COORD( maxs.x );	// maxz
		WRITE_COORD( maxs.y );
		WRITE_COORD( maxs.z );
		WRITE_COORD( flHeight );			// height
		WRITE_SHORT( g_sModelIndexBubbles );
		WRITE_BYTE( count ); // count
		WRITE_COORD( 8 ); // speed
	MESSAGE_END();
}

void UTIL_BubbleTrail( Vector from, Vector to, int count )
{
	float flHeight = UTIL_WaterLevel( from,  from.z, from.z + 256 );
	flHeight = flHeight - from.z;

	if (flHeight < 8)
	{
		flHeight = UTIL_WaterLevel( to,  to.z, to.z + 256 );
		flHeight = flHeight - to.z;
		if (flHeight < 8)
			return;

		// UNDONE: do a ploink sound
		flHeight = flHeight + to.z - from.z;
	}

	if (count > 255) 
		count = 255;

	MESSAGE_BEGIN( MSG_BROADCAST, SVC_TEMPENTITY );
		WRITE_BYTE( TE_BUBBLETRAIL );
		WRITE_COORD( from.x );	// mins
		WRITE_COORD( from.y );
		WRITE_COORD( from.z );
		WRITE_COORD( to.x );	// maxz
		WRITE_COORD( to.y );
		WRITE_COORD( to.z );
		WRITE_COORD( flHeight );			// height
		WRITE_SHORT( g_sModelIndexBubbles );
		WRITE_BYTE( count ); // count
		WRITE_COORD( 8 ); // speed
	MESSAGE_END();
}


void UTIL_Remove( CBaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	pEntity->UpdateOnRemove();
	pEntity->pev->flags |= FL_KILLME;
	pEntity->pev->targetname = 0;
}


BOOL UTIL_IsValidEntity( edict_t *pent )
{
	if ( !pent || pent->free || (pent->v.flags & FL_KILLME) )
		return FALSE;
	return TRUE;
}


void UTIL_PrecacheOther( const char *szClassname, std::map<std::string, std::string> keys)
{
	edict_t	*pent;

	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szClassname ) );
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, UTIL_VarArgs("NULL Ent '%s' in UTIL_PrecacheOther\n", szClassname) );
		return;
	}
	
	CBaseEntity *pEntity = CBaseEntity::Instance (VARS( pent ));

	if (pEntity) {
		for (auto item : keys) {
			KeyValueData dat;
			dat.fHandled = false;
			dat.szClassName = (char*)STRING(pEntity->pev->classname);
			dat.szKeyName = (char*)item.first.c_str();
			dat.szValue = (char*)item.second.c_str();
			DispatchKeyValue(pent, &dat);
		}

		pEntity->Precache();
	}
	REMOVE_ENTITY(pent);
}

//=========================================================
// UTIL_LogPrintf - Prints a logged message to console.
// Preceded by LOG: ( timestamp ) < message >
//=========================================================
void UTIL_LogPrintf( const char *fmt, ... )
{
	va_list			argptr;
	static char		string[1024];
	
	va_start ( argptr, fmt );
	vsnprintf ( string, 1024, fmt, argptr );
	va_end   ( argptr );

	// Print to server console
	ALERT( at_logged, "%s", string );
}

//=========================================================
// UTIL_DotPoints - returns the dot product of a line from
// src to check and vecdir.
//=========================================================
float UTIL_DotPoints ( const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir )
{
	Vector2D	vec2LOS;

	vec2LOS = ( vecCheck - vecSrc ).Make2D();
	vec2LOS = vec2LOS.Normalize();

	return DotProduct (vec2LOS , ( vecDir.Make2D() ) );
}


//=========================================================
// UTIL_StripToken - for redundant keynames
//=========================================================
void UTIL_StripToken( const char *pKey, char *pDest )
{
	int i = 0;

	while ( pKey[i] && pKey[i] != '#' )
	{
		pDest[i] = pKey[i];
		i++;
	}
	pDest[i] = 0;
}


// --------------------------------------------------------------
//
// CSave
//
// --------------------------------------------------------------
static int gSizes[FIELD_TYPECOUNT] = 
{
	sizeof(float),		// FIELD_FLOAT
	sizeof(int),		// FIELD_STRING
	sizeof(int),		// FIELD_ENTITY
	sizeof(int),		// FIELD_CLASSPTR
	sizeof(int),		// FIELD_EHANDLE
	sizeof(int),		// FIELD_entvars_t
	sizeof(int),		// FIELD_EDICT
	sizeof(float)*3,	// FIELD_VECTOR
	sizeof(float)*3,	// FIELD_POSITION_VECTOR
	sizeof(int *),		// FIELD_POINTER
	sizeof(int),		// FIELD_INTEGER
#ifdef GNUC
	sizeof(int *)*2,		// FIELD_FUNCTION
#else
	sizeof(int *),		// FIELD_FUNCTION	
#endif
	sizeof(int),		// FIELD_BOOLEAN
	sizeof(short),		// FIELD_SHORT
	sizeof(char),		// FIELD_CHARACTER
	sizeof(float),		// FIELD_TIME
	sizeof(int),		// FIELD_MODELNAME
	sizeof(int),		// FIELD_SOUNDNAME
};


// Base class includes common SAVERESTOREDATA pointer, and manages the entity table
CSaveRestoreBuffer :: CSaveRestoreBuffer( void )
{
	m_pdata = NULL;
}


CSaveRestoreBuffer :: CSaveRestoreBuffer( SAVERESTOREDATA *pdata )
{
	m_pdata = pdata;
}


CSaveRestoreBuffer :: ~CSaveRestoreBuffer( void )
{
}

int	CSaveRestoreBuffer :: EntityIndex( CBaseEntity *pEntity )
{
	if ( pEntity == NULL )
		return -1;
	return EntityIndex( pEntity->pev );
}


int	CSaveRestoreBuffer :: EntityIndex( entvars_t *pevLookup )
{
	if ( pevLookup == NULL )
		return -1;
	return EntityIndex( ENT( pevLookup ) );
}

int	CSaveRestoreBuffer :: EntityIndex( EOFFSET eoLookup )
{
	return EntityIndex( ENT( eoLookup ) );
}


int	CSaveRestoreBuffer :: EntityIndex( edict_t *pentLookup )
{
	if ( !m_pdata || pentLookup == NULL )
		return -1;

	int i;
	ENTITYTABLE *pTable;

	for ( i = 0; i < m_pdata->tableCount; i++ )
	{
		pTable = m_pdata->pTable + i;
		if ( pTable->pent == pentLookup )
			return i;
	}
	return -1;
}


edict_t *CSaveRestoreBuffer :: EntityFromIndex( int entityIndex )
{
	if ( !m_pdata || entityIndex < 0 )
		return NULL;

	int i;
	ENTITYTABLE *pTable;

	for ( i = 0; i < m_pdata->tableCount; i++ )
	{
		pTable = m_pdata->pTable + i;
		if ( pTable->id == entityIndex )
			return pTable->pent;
	}
	return NULL;
}


int	CSaveRestoreBuffer :: EntityFlagsSet( int entityIndex, int flags )
{
	if ( !m_pdata || entityIndex < 0 )
		return 0;
	if ( entityIndex > m_pdata->tableCount )
		return 0;

	m_pdata->pTable[ entityIndex ].flags |= flags;

	return m_pdata->pTable[ entityIndex ].flags;
}


void CSaveRestoreBuffer :: BufferRewind( int size )
{
	if ( !m_pdata )
		return;

	if ( m_pdata->size < size )
		size = m_pdata->size;

	m_pdata->pCurrentData -= size;
	m_pdata->size -= size;
}

#ifndef _WIN32
extern "C" {
unsigned _rotr ( unsigned val, int shift)
{
        unsigned lobit;        /* non-zero means lo bit set */
        unsigned num = val;    /* number to rotate */

        shift &= 0x1f;                  /* modulo 32 -- this will also make
                                           negative shifts work */

        while (shift--) {
                lobit = num & 1;        /* get high bit */
                num >>= 1;              /* shift right one bit */
                if (lobit)
                        num |= 0x80000000;  /* set hi bit if lo bit was set */
        }

        return num;
}
}
#endif

unsigned int CSaveRestoreBuffer :: HashString( const char *pszToken )
{
	unsigned int	hash = 0;

	while ( *pszToken )
		hash = _rotr( hash, 4 ) ^ *pszToken++;

	return hash;
}

unsigned short CSaveRestoreBuffer :: TokenHash( const char *pszToken )
{
	unsigned short	hash = (unsigned short)(HashString( pszToken ) % (unsigned)m_pdata->tokenCount );
	
#if _DEBUG
	static int tokensparsed = 0;
	tokensparsed++;
	if ( !m_pdata->tokenCount || !m_pdata->pTokens )
		ALERT( at_error, "No token table array in TokenHash()!" );
#endif

	for ( int i=0; i<m_pdata->tokenCount; i++ )
	{
#if _DEBUG
		static qboolean beentheredonethat = FALSE;
		if ( i > 50 && !beentheredonethat )
		{
			beentheredonethat = TRUE;
			ALERT( at_error, "CSaveRestoreBuffer :: TokenHash() is getting too full!" );
		}
#endif

		int	index = hash + i;
		if ( index >= m_pdata->tokenCount )
			index -= m_pdata->tokenCount;

		if ( !m_pdata->pTokens[index] || strcmp( pszToken, m_pdata->pTokens[index] ) == 0 )
		{
			m_pdata->pTokens[index] = (char *)pszToken;
			return index;
		}
	}
		
	// Token hash table full!!! 
	// [Consider doing overflow table(s) after the main table & limiting linear hash table search]
	ALERT( at_error, "CSaveRestoreBuffer :: TokenHash() is COMPLETELY FULL!" );
	return 0;
}

void CSave :: WriteData( const char *pname, int size, const char *pdata )
{
	BufferField( pname, size, pdata );
}


void CSave :: WriteShort( const char *pname, const short *data, int count )
{
	BufferField( pname, sizeof(short) * count, (const char *)data );
}


void CSave :: WriteInt( const char *pname, const int *data, int count )
{
	BufferField( pname, sizeof(int) * count, (const char *)data );
}


void CSave :: WriteFloat( const char *pname, const float *data, int count )
{
	BufferField( pname, sizeof(float) * count, (const char *)data );
}


void CSave :: WriteTime( const char *pname, const float *data, int count )
{
	BufferHeader( pname, sizeof(float) * count );
	for ( int i = 0; i < count; i++ )
	{
		float tmp = data[0];

		// Always encode time as a delta from the current time so it can be re-based if loaded in a new level
		// Times of 0 are never written to the file, so they will be restored as 0, not a relative time
		if ( m_pdata )
			tmp -= m_pdata->time;

		BufferData( (const char *)&tmp, sizeof(float) );
		data ++;
	}
}


void CSave :: WriteString( const char *pname, const char *pdata )
{
#ifdef TOKENIZE
	short	token = (short)TokenHash( pdata );
	WriteShort( pname, &token, 1 );
#else
	BufferField( pname, strlen(pdata) + 1, pdata );
#endif
}


void CSave :: WriteString( const char *pname, const int *stringId, int count )
{
	int i, size;

#ifdef TOKENIZE
	short	token = (short)TokenHash( STRING( *stringId ) );
	WriteShort( pname, &token, 1 );
#else
#if 0
	if ( count != 1 )
		ALERT( at_error, "No string arrays!\n" );
	WriteString( pname, (char *)STRING(*stringId) );
#endif

	size = 0;
	for ( i = 0; i < count; i++ )
		size += strlen( STRING( stringId[i] ) ) + 1;

	BufferHeader( pname, size );
	for ( i = 0; i < count; i++ )
	{
		const char *pString = STRING(stringId[i]);
		BufferData( pString, strlen(pString)+1 );
	}
#endif
}


void CSave :: WriteVector( const char *pname, const Vector &value )
{
	WriteVector( pname, &value.x, 1 );
}


void CSave :: WriteVector( const char *pname, const float *value, int count )
{
	BufferHeader( pname, sizeof(float) * 3 * count );
	BufferData( (const char *)value, sizeof(float) * 3 * count );
}



void CSave :: WritePositionVector( const char *pname, const Vector &value )
{

	if ( m_pdata && m_pdata->fUseLandmark )
	{
		Vector tmp = value - m_pdata->vecLandmarkOffset;
		WriteVector( pname, tmp );
	}

	WriteVector( pname, value );
}


void CSave :: WritePositionVector( const char *pname, const float *value, int count )
{
	BufferHeader( pname, sizeof(float) * 3 * count );
	for ( int i = 0; i < count; i++ )
	{
		Vector tmp( value[0], value[1], value[2] );

		if ( m_pdata && m_pdata->fUseLandmark )
			tmp = tmp - m_pdata->vecLandmarkOffset;

		BufferData( (const char *)&tmp.x, sizeof(float) * 3 );
		value += 3;
	}
}


void CSave :: WriteFunction( const char *pname, void **data, int count )
{
	const char *functionName;

	functionName = NAME_FOR_FUNCTION( (uint32)*data );
	if ( functionName )
		BufferField( pname, strlen(functionName) + 1, functionName );
	else
		ALERT( at_error, "Invalid function pointer in entity!" );
}

CKeyValue GetEntvarsKeyvalue(entvars_t* pev, const char* keyName) {
	CKeyValue keyvalue = g_emptyKeyValue;

	TYPEDESCRIPTION* pField;

	for (int i = 0; i < (int)ENTVARS_COUNT; i++)
	{
		pField = &gEntvarsDescription[i];

		if (!stricmp(pField->fieldName, keyName))
		{
			keyvalue.keyName = pField->fieldName;
			keyvalue.keyOffset = pField->fieldOffset;

			switch (pField->fieldType)
			{
			case FIELD_MODELNAME:
			case FIELD_SOUNDNAME:
			case FIELD_STRING:
				keyvalue.sVal = (*(int*)((char*)pev + pField->fieldOffset));
				keyvalue.keyType = KEY_TYPE_STRING;
				break;

			case FIELD_TIME:
			case FIELD_FLOAT:
				keyvalue.fVal = (*(float*)((char*)pev + pField->fieldOffset));
				keyvalue.keyType = KEY_TYPE_FLOAT;
				break;

			case FIELD_INTEGER:
				keyvalue.iVal = (*(int*)((char*)pev + pField->fieldOffset));
				keyvalue.keyType = KEY_TYPE_INT;
				break;

			case FIELD_POSITION_VECTOR:
			case FIELD_VECTOR:
				memcpy(keyvalue.vVal, (float*)((char*)pev + pField->fieldOffset), 3 * sizeof(float));
				keyvalue.keyType = KEY_TYPE_VECTOR;
				break;

			default:
			case FIELD_EVARS:
			case FIELD_CLASSPTR:
			case FIELD_EDICT:
			case FIELD_ENTITY:
			case FIELD_POINTER:
				break;
			}
			break;
		}
	}

	return keyvalue;
}

void EntvarsKeyvalue( entvars_t *pev, KeyValueData *pkvd )
{
	int i;
	TYPEDESCRIPTION		*pField;

	for ( i = 0; i < (int)ENTVARS_COUNT; i++ )
	{
		pField = &gEntvarsDescription[i];

		if ( !stricmp( pField->fieldName, pkvd->szKeyName ) )
		{
			switch( pField->fieldType )
			{
			case FIELD_MODELNAME:
			case FIELD_SOUNDNAME:
			case FIELD_STRING:
				(*(int *)((char *)pev + pField->fieldOffset)) = ALLOC_STRING( pkvd->szValue );
				break;

			case FIELD_TIME:
			case FIELD_FLOAT:
				(*(float *)((char *)pev + pField->fieldOffset)) = atof( pkvd->szValue );
				break;

			case FIELD_INTEGER:
				(*(int *)((char *)pev + pField->fieldOffset)) = atoi( pkvd->szValue );
				break;

			case FIELD_POSITION_VECTOR:
			case FIELD_VECTOR:
				UTIL_StringToVector( (float *)((char *)pev + pField->fieldOffset), pkvd->szValue );
				break;

			default:
			case FIELD_EVARS:
			case FIELD_CLASSPTR:
			case FIELD_EDICT:
			case FIELD_ENTITY:
			case FIELD_POINTER:
				ALERT( at_error, "Bad field in entity!!\n" );
				break;
			}
			pkvd->fHandled = TRUE;
			return;
		}
	}
}



int CSave :: WriteEntVars( const char *pname, entvars_t *pev )
{
	return WriteFields( pname, pev, gEntvarsDescription, ENTVARS_COUNT );
}



int CSave :: WriteFields( const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount )
{
	int				i, j, actualCount, emptyCount;
	TYPEDESCRIPTION	*pTest;
	int				entityArray[MAX_ENTITYARRAY];

	// Precalculate the number of empty fields
	emptyCount = 0;
	for ( i = 0; i < fieldCount; i++ )
	{
		void *pOutputData;
		pOutputData = ((char *)pBaseData + pFields[i].fieldOffset );
		if ( DataEmpty( (const char *)pOutputData, pFields[i].fieldSize * gSizes[pFields[i].fieldType] ) )
			emptyCount++;
	}

	// Empty fields will not be written, write out the actual number of fields to be written
	actualCount = fieldCount - emptyCount;
	WriteInt( pname, &actualCount, 1 );

	for ( i = 0; i < fieldCount; i++ )
	{
		void *pOutputData;
		pTest = &pFields[ i ];
		pOutputData = ((char *)pBaseData + pTest->fieldOffset );

		// UNDONE: Must we do this twice?
		if ( DataEmpty( (const char *)pOutputData, pTest->fieldSize * gSizes[pTest->fieldType] ) )
			continue;

		switch( pTest->fieldType )
		{
		case FIELD_FLOAT:
			WriteFloat( pTest->fieldName, (float *)pOutputData, pTest->fieldSize );
		break;
		case FIELD_TIME:
			WriteTime( pTest->fieldName, (float *)pOutputData, pTest->fieldSize );
		break;
		case FIELD_MODELNAME:
		case FIELD_SOUNDNAME:
		case FIELD_STRING:
			WriteString( pTest->fieldName, (int *)pOutputData, pTest->fieldSize );
		break;
		case FIELD_CLASSPTR:
		case FIELD_EVARS:
		case FIELD_EDICT:
		case FIELD_ENTITY:
		case FIELD_EHANDLE:
			if ( pTest->fieldSize > MAX_ENTITYARRAY )
				ALERT( at_error, "Can't save more than %d entities in an array!!!\n", MAX_ENTITYARRAY );
			for ( j = 0; j < pTest->fieldSize; j++ )
			{
				switch( pTest->fieldType )
				{
					case FIELD_EVARS:
						entityArray[j] = EntityIndex( ((entvars_t **)pOutputData)[j] );
					break;
					case FIELD_CLASSPTR:
						entityArray[j] = EntityIndex( ((CBaseEntity **)pOutputData)[j] );
					break;
					case FIELD_EDICT:
						entityArray[j] = EntityIndex( ((edict_t **)pOutputData)[j] );
					break;
					case FIELD_ENTITY:
						entityArray[j] = EntityIndex( ((EOFFSET *)pOutputData)[j] );
					break;
					case FIELD_EHANDLE:
						entityArray[j] = EntityIndex( (CBaseEntity *)(((EHANDLE *)pOutputData)[j]) );
					break;
					default:
						break;
				}
			}
			WriteInt( pTest->fieldName, entityArray, pTest->fieldSize );
		break;
		case FIELD_POSITION_VECTOR:
			WritePositionVector( pTest->fieldName, (float *)pOutputData, pTest->fieldSize );
		break;
		case FIELD_VECTOR:
			WriteVector( pTest->fieldName, (float *)pOutputData, pTest->fieldSize );
		break;

		case FIELD_BOOLEAN:
		case FIELD_INTEGER:
			WriteInt( pTest->fieldName, (int *)pOutputData, pTest->fieldSize );
		break;

		case FIELD_SHORT:
			WriteData( pTest->fieldName, 2 * pTest->fieldSize, ((char *)pOutputData) );
		break;

		case FIELD_CHARACTER:
			WriteData( pTest->fieldName, pTest->fieldSize, ((char *)pOutputData) );
		break;

		// For now, just write the address out, we're not going to change memory while doing this yet!
		case FIELD_POINTER:
			WriteInt( pTest->fieldName, (int *)(char *)pOutputData, pTest->fieldSize );
		break;

		case FIELD_FUNCTION:
			WriteFunction( pTest->fieldName, (void **)pOutputData, pTest->fieldSize );
		break;
		default:
			ALERT( at_error, "Bad field type\n" );
		}
	}

	return 1;
}


void CSave :: BufferString( char *pdata, int len )
{
	char c = 0;

	BufferData( pdata, len );		// Write the string
	BufferData( &c, 1 );			// Write a null terminator
}


int CSave :: DataEmpty( const char *pdata, int size )
{
	for ( int i = 0; i < size; i++ )
	{
		if ( pdata[i] )
			return 0;
	}
	return 1;
}


void CSave :: BufferField( const char *pname, int size, const char *pdata )
{
	BufferHeader( pname, size );
	BufferData( pdata, size );
}


void CSave :: BufferHeader( const char *pname, int size )
{
	short	hashvalue = TokenHash( pname );
	if ( size > 1<<(sizeof(short)*8) )
		ALERT( at_error, "CSave :: BufferHeader() size parameter exceeds 'short'!" );
	BufferData( (const char *)&size, sizeof(short) );
	BufferData( (const char *)&hashvalue, sizeof(short) );
}


void CSave :: BufferData( const char *pdata, int size )
{
	if ( !m_pdata )
		return;

	if ( m_pdata->size + size > m_pdata->bufferSize )
	{
		ALERT( at_error, "Save/Restore overflow!" );
		m_pdata->size = m_pdata->bufferSize;
		return;
	}

	memcpy( m_pdata->pCurrentData, pdata, size );
	m_pdata->pCurrentData += size;
	m_pdata->size += size;
}



// --------------------------------------------------------------
//
// CRestore
//
// --------------------------------------------------------------

int CRestore::ReadField( void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount, int startField, int size, char *pName, void *pData )
{
	int i, j, stringCount, fieldNumber, entityIndex;
	TYPEDESCRIPTION *pTest;
	float	time, timeData;
	Vector	position;
	edict_t	*pent;
	char	*pString;

	time = 0;
	position = Vector(0,0,0);

	if ( m_pdata )
	{
		time = m_pdata->time;
		if ( m_pdata->fUseLandmark )
			position = m_pdata->vecLandmarkOffset;
	}

	for ( i = 0; i < fieldCount; i++ )
	{
		fieldNumber = (i+startField)%fieldCount;
		pTest = &pFields[ fieldNumber ];
		if ( !stricmp( pTest->fieldName, pName ) )
		{
			if ( !m_global || !(pTest->flags & FTYPEDESC_GLOBAL) )
			{
				for ( j = 0; j < pTest->fieldSize; j++ )
				{
					void *pOutputData = ((char *)pBaseData + pTest->fieldOffset + (j*gSizes[pTest->fieldType]) );
					void *pInputData = (char *)pData + j * gSizes[pTest->fieldType];

					switch( pTest->fieldType )
					{
					case FIELD_TIME:
						timeData = *(float *)pInputData;
						// Re-base time variables
						timeData += time;
						*((float *)pOutputData) = timeData;
					break;
					case FIELD_FLOAT:
						*((float *)pOutputData) = *(float *)pInputData;
					break;
					case FIELD_MODELNAME:
					case FIELD_SOUNDNAME:
					case FIELD_STRING:
						// Skip over j strings
						pString = (char *)pData;
						for ( stringCount = 0; stringCount < j; stringCount++ )
						{
							while (*pString)
								pString++;
							pString++;
						}
						pInputData = pString;
						if ( strlen( (char *)pInputData ) == 0 )
							*((int *)pOutputData) = 0;
						else
						{
							int string;

							string = ALLOC_STRING( (char *)pInputData );
							
							*((int *)pOutputData) = string;

							if ( !FStringNull( string ) && m_precache )
							{
								if ( pTest->fieldType == FIELD_MODELNAME )
									PRECACHE_MODEL( (char *)STRING( string ) );
								else if ( pTest->fieldType == FIELD_SOUNDNAME )
									PRECACHE_SOUND_ENT(NULL, (char *)STRING( string ) );
							}
						}
					break;
					case FIELD_EVARS:
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						if ( pent )
							*((entvars_t **)pOutputData) = VARS(pent);
						else
							*((entvars_t **)pOutputData) = NULL;
					break;
					case FIELD_CLASSPTR:
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						if ( pent )
							*((CBaseEntity **)pOutputData) = CBaseEntity::Instance(pent);
						else
							*((CBaseEntity **)pOutputData) = NULL;
					break;
					case FIELD_EDICT:
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						*((edict_t **)pOutputData) = pent;
					break;
					case FIELD_EHANDLE:
						// Input and Output sizes are different!
						pOutputData = (char *)pOutputData + j*(sizeof(EHANDLE) - gSizes[pTest->fieldType]);
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						if ( pent )
							*((EHANDLE *)pOutputData) = CBaseEntity::Instance(pent);
						else
							*((EHANDLE *)pOutputData) = NULL;
					break;
					case FIELD_ENTITY:
						entityIndex = *( int *)pInputData;
						pent = EntityFromIndex( entityIndex );
						if ( pent )
							*((EOFFSET *)pOutputData) = OFFSET(pent);
						else
							*((EOFFSET *)pOutputData) = 0;
					break;
					case FIELD_VECTOR:
						((float *)pOutputData)[0] = ((float *)pInputData)[0];
						((float *)pOutputData)[1] = ((float *)pInputData)[1];
						((float *)pOutputData)[2] = ((float *)pInputData)[2];
					break;
					case FIELD_POSITION_VECTOR:
						((float *)pOutputData)[0] = ((float *)pInputData)[0] + position.x;
						((float *)pOutputData)[1] = ((float *)pInputData)[1] + position.y;
						((float *)pOutputData)[2] = ((float *)pInputData)[2] + position.z;
					break;

					case FIELD_BOOLEAN:
					case FIELD_INTEGER:
						*((int *)pOutputData) = *( int *)pInputData;
					break;

					case FIELD_SHORT:
						*((short *)pOutputData) = *( short *)pInputData;
					break;

					case FIELD_CHARACTER:
						*((char *)pOutputData) = *( char *)pInputData;
					break;

					case FIELD_POINTER:
						*((int *)pOutputData) = *( int *)pInputData;
					break;
					case FIELD_FUNCTION:
						if ( strlen( (char *)pInputData ) == 0 )
							*((int *)pOutputData) = 0;
						else
							*((int *)pOutputData) = FUNCTION_FROM_NAME( (char *)pInputData );
					break;

					default:
						ALERT( at_error, "Bad field type\n" );
					}
				}
			}
#if 0
			else
			{
				ALERT( at_console, "Skipping global field %s\n", pName );
			}
#endif
			return fieldNumber;
		}
	}

	return -1;
}


int CRestore::ReadEntVars( const char *pname, entvars_t *pev )
{
	return ReadFields( pname, pev, gEntvarsDescription, ENTVARS_COUNT );
}


int CRestore::ReadFields( const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount )
{
	unsigned short	i, token;
	int		lastField, fileCount;
	HEADER	header;

	i = ReadShort();
	ASSERT( i == sizeof(int) );			// First entry should be an int

	token = ReadShort();

	// Check the struct name
	if ( token != TokenHash(pname) )			// Field Set marker
	{
//		ALERT( at_error, "Expected %s found %s!\n", pname, BufferPointer() );
		BufferRewind( 2*sizeof(short) );
		return 0;
	}

	// Skip over the struct name
	fileCount = ReadInt();						// Read field count

	lastField = 0;								// Make searches faster, most data is read/written in the same order

	// Clear out base data
	for ( i = 0; i < fieldCount; i++ )
	{
		// Don't clear global fields
		if ( !m_global || !(pFields[i].flags & FTYPEDESC_GLOBAL) )
			memset( ((char *)pBaseData + pFields[i].fieldOffset), 0, pFields[i].fieldSize * gSizes[pFields[i].fieldType] );
	}

	for ( i = 0; i < fileCount; i++ )
	{
		BufferReadHeader( &header );
		lastField = ReadField( pBaseData, pFields, fieldCount, lastField, header.size, m_pdata->pTokens[header.token], header.pData );
		lastField++;
	}
	
	return 1;
}


void CRestore::BufferReadHeader( HEADER *pheader )
{
	ASSERT( pheader!=NULL );
	pheader->size = ReadShort();				// Read field size
	pheader->token = ReadShort();				// Read field name token
	pheader->pData = BufferPointer();			// Field Data is next
	BufferSkipBytes( pheader->size );			// Advance to next field
}


short	CRestore::ReadShort( void )
{
	short tmp = 0;

	BufferReadBytes( (char *)&tmp, sizeof(short) );

	return tmp;
}

int	CRestore::ReadInt( void )
{
	int tmp = 0;

	BufferReadBytes( (char *)&tmp, sizeof(int) );

	return tmp;
}

int CRestore::ReadNamedInt( const char *pName )
{
	HEADER header;

	BufferReadHeader( &header );
	return ((int *)header.pData)[0];
}

char *CRestore::ReadNamedString( const char *pName )
{
	HEADER header;

	BufferReadHeader( &header );
#ifdef TOKENIZE
	return (char *)(m_pdata->pTokens[*(short *)header.pData]);
#else
	return (char *)header.pData;
#endif
}


char *CRestore::BufferPointer( void )
{
	if ( !m_pdata )
		return NULL;

	return m_pdata->pCurrentData;
}

void CRestore::BufferReadBytes( char *pOutput, int size )
{
	ASSERT( m_pdata !=NULL );

	if ( !m_pdata || Empty() )
		return;

	if ( (m_pdata->size + size) > m_pdata->bufferSize )
	{
		ALERT( at_error, "Restore overflow!" );
		m_pdata->size = m_pdata->bufferSize;
		return;
	}

	if ( pOutput )
		memcpy( pOutput, m_pdata->pCurrentData, size );
	m_pdata->pCurrentData += size;
	m_pdata->size += size;
}


void CRestore::BufferSkipBytes( int bytes )
{
	BufferReadBytes( NULL, bytes );
}

int CRestore::BufferSkipZString( void )
{
	char *pszSearch;
	int	 len;

	if ( !m_pdata )
		return 0;

	int maxLen = m_pdata->bufferSize - m_pdata->size;

	len = 0;
	pszSearch = m_pdata->pCurrentData;
	while ( *pszSearch++ && len < maxLen )
		len++;

	len++;

	BufferSkipBytes( len );

	return len;
}

int	CRestore::BufferCheckZString( const char *string )
{
	if ( !m_pdata )
		return 0;

	int maxLen = m_pdata->bufferSize - m_pdata->size;
	int len = strlen( string );
	if ( len <= maxLen )
	{
		if ( !strncmp( string, m_pdata->pCurrentData, len ) )
			return 1;
	}
	return 0;
}

std::map<std::string, std::string> loadReplacementFile(const char* path) {
	std::map<std::string, std::string> replacements;

	std::string fpath = getGameFilePath(path);
	std::ifstream infile(fpath);

	if (fpath.empty() || !infile.is_open()) {
		ALERT(at_console, "Failed to load replacement file: %s\n", path);
		return replacements;
	}

	int lineNum = 0;
	std::string line;
	while (std::getline(infile, line))
	{
		lineNum++;
		std::string paths[2];

		line = trimSpaces(line);
		if (line.empty()) {
			continue;
		}

		int quotedPaths = 0;
		for (int i = 0; i < 2; i++) {
			int startQuote = line.find('"');
			if (startQuote == -1) {
				break;
			}

			int endQuote = line.find('"', startQuote + 1);
			if (endQuote != -1) {
				paths[i] = trimSpaces(line.substr(startQuote + 1, (endQuote - startQuote)-1));
				line = line.substr(endQuote + 1);
				quotedPaths++;
			}
			else {
				ALERT(at_warning, "%s line %d: Missing end quote\n", fpath.c_str(), lineNum);
				break;
			}
		}
		
		if (quotedPaths < 2) {
			// one or more paths is not surrounded with quotes
			// assume the paths have no spaces and just get the next X strings
			
			// in case there were an odd number of quotes
			line.erase(std::remove(line.begin(), line.end(), '"'), line.end());
			
			std::vector<std::string> parts = splitString(line, " \t");
			std::vector<std::string> nonemptyParts;
			for (int i = 0; i < (int)parts.size(); i++) {
				if (parts[i].size()) {
					nonemptyParts.push_back(parts[i]);
				}
			}

			if (quotedPaths == 0 && nonemptyParts.size() >= 2) {
				paths[0] = nonemptyParts[0];
				paths[1] = nonemptyParts[1];
			}
			else if (quotedPaths == 1 && nonemptyParts.size() >= 1) {
				paths[1] = nonemptyParts[0];
			}
			else {
				ALERT(at_warning, "%s line %d: Missing path(s)\n", fpath.c_str(), lineNum);
				continue;
			}
		}
		
		replacements[toLowerCase(paths[0])] = toLowerCase(paths[1]);
		//ALERT(at_console, "REP: %s -> %s\n", paths[0].c_str(), paths[1].c_str());
	}

	return replacements;
}

char PM_FindTextureType(char* name);

void LoadBsp() {
	std::string mapPath = getGameFilePath((std::string("maps/") + STRING(gpGlobals->mapname) + ".bsp").c_str());
	g_bsp.load_lumps(mapPath);

	if (g_bsp.textures) {
		for (int i = 0; i < g_bsp.textureCount; i++) {
			int32_t texOffset = ((int32_t*)g_bsp.textures)[i + 1];

			if (texOffset == -1) {
				continue;
			}

			BSPMIPTEX& tex = *((BSPMIPTEX*)(g_bsp.textures + texOffset));

			char* texName = tex.szName;
			int slen = strlen(texName);

			if (slen > 1) {
				if (texName[0] == '{' || texName[0] == '!') {
					texName = texName + 1;
				}
				else if (slen > 2 && (texName[0] == '+' || texName[0] == '-')) {
					texName = texName + 2;
				}
			}

			switch (PM_FindTextureType(texName)) {
				case CHAR_TEX_CONCRETE: g_textureStats.tex_concrete = true; break;
				case CHAR_TEX_METAL: g_textureStats.tex_metal = true; break;
				case CHAR_TEX_DIRT: g_textureStats.tex_dirt = true; break;
				case CHAR_TEX_VENT: g_textureStats.tex_duct = true; break;
				case CHAR_TEX_GRATE: g_textureStats.tex_grate = true; break;
				case CHAR_TEX_TILE: g_textureStats.tex_tile = true; break;
				case CHAR_TEX_SLOSH: g_textureStats.tex_water = true; break;
				case CHAR_TEX_WOOD: g_textureStats.tex_wood = true; break;
				case CHAR_TEX_COMPUTER: g_textureStats.tex_computer = true; break;
				case CHAR_TEX_GLASS: g_textureStats.tex_glass = true; break;
				case CHAR_TEX_FLESH: g_textureStats.tex_flesh = true; break;
				default: break;
			}
		}
	}

	// check for any leaves that would make swimming sounds if the player entered them
	if (g_bsp.leaves) {
		for (int i = 0; i < g_bsp.leafCount; i++) {
			int contents = g_bsp.leaves[i].nContents;
			if (contents <= CONTENTS_WATER && contents > CONTENTS_TRANSLUCENT) {
				g_textureStats.tex_water = true;
			}
		}
	}
}

int PRECACHE_GENERIC(const char* path) {
	std::string lowerPath = toLowerCase(path);
	path = lowerPath.c_str();

	if (g_modelReplacements.find(path) != g_modelReplacements.end()) {
		path = g_modelReplacements[path].c_str();
	}
	if (g_soundReplacements.find(path) != g_soundReplacements.end()) {
		path = g_soundReplacements[path].c_str();
	}

	if (g_serveractive) {
		if (g_precachedGeneric.find(path) != g_precachedGeneric.end()) {
			return g_engfuncs.pfnPrecacheGeneric(path);
		}
		else {
			ALERT(at_warning, "PrecacheGeneric failed: %s\n", path);
			return -1;
		}
	}

	g_tryPrecacheGeneric.insert(path);

	if (g_tryPrecacheGeneric.size() < MAX_PRECACHE) {
		g_precachedGeneric.insert(path);
		return g_engfuncs.pfnPrecacheGeneric(path);
	}
	else {
		return -1;
	}
}

int PRECACHE_SOUND_ENT(CBaseEntity* ent, const char* path) {
	std::string lowerPath = toLowerCase(path);
	path = lowerPath.c_str();

	bool hadMonsterSoundReplacement = false;
	if (ent && ent->IsMonster() && (int)g_monsterSoundReplacements.size() >= ent->entindex()) {
		std::map<std::string, std::string>& replacementMap = g_monsterSoundReplacements[ent->entindex()];
		if (replacementMap.find(path) != replacementMap.end()) {
			path = replacementMap[path].c_str();
			hadMonsterSoundReplacement = true;
		}
	}
	
	if (!hadMonsterSoundReplacement && g_soundReplacements.find(path) != g_soundReplacements.end()) {
		path = g_soundReplacements[path].c_str();
	}

	if (lowerPath.find(" ") != std::string::npos) {
		// files with spaces causes clients to hang at "Verifying resources"
		// and the file doesn't download
		ALERT(at_error, "Precached file with spaces: '%s'\n", path);
		return g_engfuncs.pfnPrecacheSound(NOT_PRECACHED_SOUND);
	}

	if (g_serveractive) {
		if (g_precachedSounds.find(path) != g_precachedSounds.end()) {
			return g_engfuncs.pfnPrecacheSound(path);
		}
		else {
			ALERT(at_warning, "PrecacheSound failed: %s\n", path);
			return -1;
		}
	}

	g_tryPrecacheSounds.insert(path);

	if (g_tryPrecacheSounds.size() <= MAX_PRECACHE_SOUND) {
		g_precachedSounds.insert(path);
		return g_engfuncs.pfnPrecacheSound(path);
	}
	else {
		return g_engfuncs.pfnPrecacheSound(NOT_PRECACHED_SOUND);
	}
}

int PRECACHE_SOUND_NULLENT(const char* path) {
	return PRECACHE_SOUND_ENT(NULL, path);
}

int PRECACHE_MODEL(const char* path) {
	std::string lowerPath = toLowerCase(path);
	path = lowerPath.c_str();

	if (g_modelReplacements.find(path) != g_modelReplacements.end()) {
		path = g_modelReplacements[path].c_str();
	}

	// loading BSP here because ServerActivate is not soon enough and GameDLLInit is only called once
	if (!g_bsp.loaded) {
		LoadBsp();
	}

	bool alreadyPrecached = g_precachedModels.find(path) != g_precachedModels.end();
	if (!alreadyPrecached && getGameFilePath(path).empty()) {
		if (!g_missingModels.count(path)) {
			ALERT(at_error, "Model precache failed. File not found: %s\n", path);
			g_missingModels.insert(path);
		}
		
		return g_engfuncs.pfnPrecacheModel(NOT_PRECACHED_MODEL);
	}

	if (g_serveractive) {
		if (g_tryPrecacheModels.find(path) != g_tryPrecacheModels.end()) {
			return g_engfuncs.pfnPrecacheModel(path);
		}
		else {
			ALERT(at_warning, "PrecacheModel failed: %s\n", path);
			return -1;
		}
	}

	g_tryPrecacheModels.insert(path);

	// not sure what the +2 is for. Tested with sc_darknebula.
	if (g_tryPrecacheModels.size() + g_bsp.modelCount <= MAX_PRECACHE_MODEL) {
		if (g_precachedModels.find(path) == g_precachedModels.end())
			g_precachedModels[path] = path;
		return g_engfuncs.pfnPrecacheModel(path);
	}
	else {
		return g_engfuncs.pfnPrecacheModel(NOT_PRECACHED_MODEL);
	}
	
}

int PRECACHE_EVENT(int id, const char* path) {
	std::string lowerPath = toLowerCase(path);
	path = lowerPath.c_str();

	if (g_serveractive) {
		if (g_precachedEvents.find(path) != g_precachedEvents.end()) {
			return g_precachedEvents[path];
		}
		else {
			ALERT(at_warning, "PrecacheEvent failed: %s\n", path);
			return -1;
		}
	}

	g_tryPrecacheEvents.insert(path);

	if (g_tryPrecacheEvents.size() < MAX_PRECACHE_EVENT) {
		g_precachedEvents[path] = g_engfuncs.pfnPrecacheEvent(id, path);
		return g_precachedEvents[path];
	}
	else {
		return -1;
	}
}

void SET_MODEL(edict_t* edict, const char* model) {
	if (model && model[0] == '*') {
		// BSP model. No special handling.
		g_engfuncs.pfnSetModel(edict, model);
		return;
	}

	std::string lowerPath = toLowerCase(model);
	model = lowerPath.c_str();

	if (g_modelReplacements.find(model) != g_modelReplacements.end()) {
		model = g_modelReplacements[model].c_str();
	}
	
	if (g_precachedModels.find(model) == g_precachedModels.end()) {
		model = NOT_PRECACHED_MODEL;
	}

	g_engfuncs.pfnSetModel(edict, model);
}

const char* GET_MODEL(const char* model) {
	std::string lowerPath = toLowerCase(model);
	model = lowerPath.c_str();

	if (g_modelReplacements.find(model) != g_modelReplacements.end()) {
		model = g_modelReplacements[model].c_str();
	}

	if (g_precachedModels.find(model) == g_precachedModels.end()) {
		model = NOT_PRECACHED_MODEL;
	}
	else {
		model = g_precachedModels[model].c_str();
	}

	return model;
}

int MODEL_INDEX(const char* model) {
	std::string lowerPath = toLowerCase(model);
	model = lowerPath.c_str();
	return g_engfuncs.pfnModelIndex(model);
}

void* GET_MODEL_PTR(edict_t* edict) {
	studiohdr_t* header = (studiohdr_t*)g_engfuncs.pfnGetModelPtr(edict);

	if (!header) {
		return NULL;
	}

	// basic corruption detection
	if (header->id != 1414743113 || header->version != 10) {
		ALERT(at_error, "Model corruption! Model: %s, idx: %d, ID: %d, Version: %d\n",
			STRING(edict->v.model), edict->v.modelindex, header->version, header->id);
		SET_MODEL(edict, NOT_PRECACHED_MODEL); // stop using the broken model and spamming the console
		return NULL;
	}

	return header;
}

void InitEdictRelocations() {
	if (!g_edictsinit) {
		// initialize all edict slots so that ents can be relocated anywhere
		// (unused slots are not marked 'free' until the total edict count requires a new slot)
		// this will hurt performance because the max edict count will always be iterated when scanning ents

		edict_t* startEdict = CREATE_ENTITY();
		int edictsToCreate = (gpGlobals->maxEntities - ENTINDEX(startEdict)) - 1;
		for (int i = 0; i < edictsToCreate; i++) {
			CREATE_ENTITY();
		}
		for (int i = 0; i <= edictsToCreate; i++) {
			REMOVE_ENTITY(startEdict + i);
		}

		g_edictsinit = 1;
		//ALERT(at_console, "Created %d edicts to prep for relocations\n", edictsToCreate);

		PrintEntindexStats();
	}
}

void PrintEntindexStats() {
	int totalFreeLowPrio = 0;
	int totalFreeNormalPrio = 0;
	int totalFreeHighPrio = 0;
	int lowPrioMin = sv_max_client_edicts->value;
	int normalPrioMin = 512;
	int reservedSlots = gpGlobals->maxClients + 1;

	edict_t* edicts = ENT(0);
	for (int i = reservedSlots; i < normalPrioMin; i++) {
		totalFreeHighPrio += edicts[i].free;
	}
	for (int i = normalPrioMin; i < lowPrioMin; i++) {
		totalFreeNormalPrio += edicts[i].free;
	}
	for (int i = lowPrioMin; i < gpGlobals->maxEntities; i++) {
		totalFreeLowPrio += edicts[i].free;
	}

	int totalHighSlots = normalPrioMin;
	int totalNormalSlots = lowPrioMin - normalPrioMin;
	int totalLowSlots = gpGlobals->maxEntities - lowPrioMin;
	int totalFree = totalFreeHighPrio + totalFreeNormalPrio + totalFreeLowPrio;

	g_engfuncs.pfnServerPrint(UTIL_VarArgs("Edict stats: %d/%d TOTAL (%d/%d HIGH, %d/%d NORM, %d/%d LOW)\n",
		gpGlobals->maxEntities - totalFree, gpGlobals->maxEntities,
		totalHighSlots - totalFreeHighPrio, totalHighSlots,
		totalNormalSlots - totalFreeNormalPrio, totalNormalSlots,
		totalLowSlots - totalFreeLowPrio, totalLowSlots));
}

// Moves an entity somewhere else in the edict list, based on its priority.
// Note: This will break EHANDLES, so be certain none of those are created in KeyValue
// before an entity spawns.
CBaseEntity* RelocateEntIdx(CBaseEntity* pEntity) {
	if (!mp_edictsorting.value) {
		return pEntity;
	}

	InitEdictRelocations();

	int iprio = pEntity->GetEntindexPriority();
	int eidx = pEntity->entindex();
	int bestIdx = eidx;
	int lowPrioMin = sv_max_client_edicts->value;
	int normalPrioMin = 512;
	edict_t* edicts = ENT(0);

	if (iprio == ENTIDX_PRIORITY_LOW && eidx < lowPrioMin) {
		// try to find a slot in the low priority area, else a normal slot, else whatever it is now
		for (int i = gpGlobals->maxEntities - 1; i >= normalPrioMin; i--) {
			if (edicts[i].free) {
				bestIdx = i;
				break;
			}
		}
	}
	else if (iprio == ENTIDX_PRIORITY_NORMAL && eidx < normalPrioMin) {
		// try to find a slot in the normal priority area, else keep using the high priority slot
		for (int i = lowPrioMin - 1; i >= normalPrioMin; i--) {
			if (edicts[i].free) {
				bestIdx = i;
				break;
			}
		}
	}

	if (bestIdx != eidx) {
		memcpy(&edicts[bestIdx], &edicts[eidx], sizeof(edict_t));

		edicts[eidx].pvPrivateData = NULL;
		REMOVE_ENTITY(&edicts[eidx]);

		if (!g_customKeyValues.empty()) {
			g_customKeyValues[bestIdx] = g_customKeyValues[eidx];
			g_customKeyValues[eidx].clear();
		}

		if (!g_monsterSoundReplacements.empty()) {
			g_monsterSoundReplacements[bestIdx] = g_monsterSoundReplacements[eidx];
			g_monsterSoundReplacements[eidx].clear();
		}

		pEntity->pev = &edicts[bestIdx].v;
		edicts[bestIdx].v.pContainingEntity = &edicts[bestIdx];
	}

	return pEntity;
}

edict_t* CREATE_NAMED_ENTITY(string_t cname) {
	edict_t* ed = g_engfuncs.pfnCreateNamedEntity(cname);
	CBaseEntity* pEntity = CBaseEntity::Instance(ed);

	return pEntity ? RelocateEntIdx(pEntity)->edict() : ed;
}

const char* getPlayerUniqueId(edict_t* plr) {
	if (plr == NULL) {
		return "STEAM_ID_NULL";
	}

	const char* steamId = (*g_engfuncs.pfnGetPlayerAuthId)(plr);

	if (!strcmp(steamId, "STEAM_ID_LAN") || !strcmp(steamId, "BOT")) {
		steamId = STRING(plr->v.netname);
	}

	return steamId;
}

uint64_t steamid_to_steamid64(const char* steamid) {
	if (strlen(steamid) < 10) {
		return 0;
	}

	uint64_t Y = atoi(steamid + 10);
	uint64_t steam64id = 76561197960265728;
	steam64id += Y * 2;

	if (steamid[8] == '1') {
		steam64id += 1;
	}

	return steam64id;
}

uint64_t getPlayerCommunityId(edict_t* plr) {
	const char* id = getPlayerUniqueId(plr);

	if (!strcmp(id, "STEAM_ID_NULL") || !strcmp(id, "STEAM_ID_LAN") || !strcmp(id, "BOT")) {
		return 0;
	}

	return steamid_to_steamid64(id);
}

enum msg_func_types {
	MFUNC_BYTE,
	MFUNC_CHAR,
	MFUNC_SHORT,
	MFUNC_LONG,
	MFUNC_ANGLE,
	MFUNC_COORD,
	MFUNC_STRING,
	MFUNC_ENTITY,
};

struct msg_part {
	uint16_t type;
	
	union {
		int iValue;
		float fValue;
	};
	const char* sValue;
};

struct msg_info {
	int msg_dest;
	int msg_type;
	bool hasOrigin;
	float pOrigin[3];
	int entIdx;
	char name[32];

	int numMsgParts;
	msg_part parts[512];
};

struct msg_hist_item {
	float time;
	std::string msg;
};

msg_info g_lastMsg;
char g_msgStrPool[512];
int g_nextStrOffset = 0;
std::string g_debugLogName;
std::string g_errorLogName;
std::ofstream g_debugFile;
std::ofstream g_errorFile;
std::queue<msg_hist_item> g_messageHistory;
float g_messageHistoryCutoff = 1.0f; // don't keep more than X seconds of history

void add_msg_part(int mtype, int iValue) {
	if (!mp_debugmsg.value) {
		return;
	}
	int idx = g_lastMsg.numMsgParts++ % 512;
	msg_part& part = g_lastMsg.parts[idx];
	part.type = mtype;
	part.iValue = iValue;
}
void add_msg_part(int mtype, float fValue) {
	if (!mp_debugmsg.value) {
		return;
	}
	int idx = g_lastMsg.numMsgParts++ % 512;
	msg_part& part = g_lastMsg.parts[idx];
	part.type = mtype;
	part.iValue = fValue;
}
void add_msg_part(const char* sValue) {
	if (!mp_debugmsg.value) {
		return;
	}
	int idx = g_lastMsg.numMsgParts++ % 512;
	msg_part& part = g_lastMsg.parts[idx];
	part.type = MFUNC_STRING;
	
	int strLen = sValue ? strlen(sValue) + 1 : 1;
	if (g_nextStrOffset + strLen < 512) {
		if (sValue) {
			memcpy(g_msgStrPool + g_nextStrOffset, sValue, strLen);
			part.sValue = g_msgStrPool + g_nextStrOffset;
			g_nextStrOffset += strLen;
		}
		else {
			part.sValue = g_msgStrPool + g_nextStrOffset;
			g_msgStrPool[g_nextStrOffset++] = 0;
		}
	}
	else {
		ALERT(at_logged, "ERROR: WRITE_STRING exceeded 512 bytes of message\n");
		part.sValue = 0;
	}
}

std::string getLogTimeStr() {
	time_t rawtime;
	struct tm* timeinfo;
	static char buffer[256];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "L %d/%m/%Y - %H:%M:%S: ", timeinfo);

	return buffer;
}

void writeDebugLog(std::ofstream& outFile, std::string lastLogName, std::string prefix, std::string line) {
	std::string curLogName;

	{
		time_t rawtime;
		struct tm* timeinfo;
		static char buffer[256];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, sizeof(buffer), "%Y-%m-%d.log", timeinfo);
		std::string fname = "logs_dbg/" + prefix + "_" + std::string(buffer);

		curLogName = fname;
	}

	if (curLogName != lastLogName) {
		lastLogName = curLogName;
		outFile.close();
		outFile.open(lastLogName, std::ios_base::app);
	}

	outFile << getLogTimeStr() << line << "\n";
}

const char* msgDestStr(int msg_dest) {
	const char* sdst = "";
	switch (msg_dest) {
	case MSG_BROADCAST:
		sdst = "MSG_BROADCAST";
		break;
	case MSG_ONE:
		sdst = "MSG_ONE";
		break;
	case MSG_ALL:
		sdst = "MSG_ALL";
		break;
	case MSG_INIT:
		sdst = "MSG_INIT";
		break;
	case MSG_PVS:
		sdst = "MSG_PVS";
		break;
	case MSG_PAS:
		sdst = "MSG_PAS";
		break;
	case MSG_PVS_R:
		sdst = "MSG_PVS_R";
		break;
	case MSG_PAS_R:
		sdst = "MSG_PAS_R";
		break;
	case MSG_ONE_UNRELIABLE:
		sdst = "MSG_ONE_UNRELIABLE";
		break;
	case MSG_SPEC:
		sdst = "MSG_SPEC";
		break;
	default:
		sdst = UTIL_VarArgs("%d (unkown)", msg_dest);
		break;
	}

	return sdst;
}

const char* msgTypeStr(int msg_type) {
	const char* sdst = "";

	switch (msg_type) {
	case SVC_BAD: sdst = "SVC_BAD"; break;
	case SVC_NOP: sdst = "SVC_NOP"; break;
	case SVC_DISCONNECT: sdst = "SVC_DISCONNECT"; break;
	case SVC_EVENT: sdst = "SVC_EVENT"; break;
	case SVC_VERSION: sdst = "SVC_VERSION"; break;
	case SVC_SETVIEW: sdst = "SVC_SETVIEW"; break;
	case SVC_SOUND: sdst = "SVC_SOUND"; break;
	case SVC_TIME: sdst = "SVC_TIME"; break;
	case SVC_PRINT: sdst = "SVC_PRINT"; break;
	case SVC_STUFFTEXT: sdst = "SVC_STUFFTEXT"; break;
	case SVC_SETANGLE: sdst = "SVC_SETANGLE"; break;
	case SVC_SERVERINFO: sdst = "SVC_SERVERINFO"; break;
	case SVC_LIGHTSTYLE: sdst = "SVC_LIGHTSTYLE"; break;
	case SVC_UPDATEUSERINFO: sdst = "SVC_UPDATEUSERINFO"; break;
	case SVC_DELTADESCRIPTION: sdst = "SVC_DELTADESCRIPTION"; break;
	case SVC_CLIENTDATA: sdst = "SVC_CLIENTDATA"; break;
	case SVC_STOPSOUND: sdst = "SVC_STOPSOUND"; break;
	case SVC_PINGS: sdst = "SVC_PINGS"; break;
	case SVC_PARTICLE: sdst = "SVC_PARTICLE"; break;
	case SVC_DAMAGE: sdst = "SVC_DAMAGE"; break;
	case SVC_SPAWNSTATIC: sdst = "SVC_SPAWNSTATIC"; break;
	case SVC_EVENT_RELIABLE: sdst = "SVC_EVENT_RELIABLE"; break;
	case SVC_SPAWNBASELINE: sdst = "SVC_SPAWNBASELINE"; break;
	case SVC_TEMPENTITY: sdst = "SVC_TEMPENTITY"; break;
	case SVC_SETPAUSE: sdst = "SVC_SETPAUSE"; break;
	case SVC_SIGNONNUM: sdst = "SVC_SIGNONNUM"; break;
	case SVC_CENTERPRINT: sdst = "SVC_CENTERPRINT"; break;
	case SVC_KILLEDMONSTER: sdst = "SVC_KILLEDMONSTER"; break;
	case SVC_FOUNDSECRET: sdst = "SVC_FOUNDSECRET"; break;
	case SVC_SPAWNSTATICSOUND: sdst = "SVC_SPAWNSTATICSOUND"; break;
	case SVC_INTERMISSION: sdst = "SVC_INTERMISSION"; break;
	case SVC_FINALE: sdst = "SVC_FINALE"; break;
	case SVC_CDTRACK: sdst = "SVC_CDTRACK"; break;
	case SVC_RESTORE: sdst = "SVC_RESTORE"; break;
	case SVC_CUTSCENE: sdst = "SVC_CUTSCENE"; break;
	case SVC_WEAPONANIM: sdst = "SVC_WEAPONANIM"; break;
	case SVC_DECALNAME: sdst = "SVC_DECALNAME"; break;
	case SVC_ROOMTYPE: sdst = "SVC_ROOMTYPE"; break;
	case SVC_ADDANGLE: sdst = "SVC_ADDANGLE"; break;
	case SVC_NEWUSERMSG: sdst = "SVC_NEWUSERMSG"; break;
	case SVC_PACKETENTITIES: sdst = "SVC_PACKETENTITIES"; break;
	case SVC_DELTAPACKETENTITIES: sdst = "SVC_DELTAPACKETENTITIES"; break;
	case SVC_CHOKE: sdst = "SVC_CHOKE"; break;
	case SVC_RESOURCELIST: sdst = "SVC_RESOURCELIST"; break;
	case SVC_NEWMOVEVARS: sdst = "SVC_NEWMOVEVARS"; break;
	case SVC_RESOURCEREQUEST: sdst = "SVC_RESOURCEREQUEST"; break;
	case SVC_CUSTOMIZATION: sdst = "SVC_CUSTOMIZATION"; break;
	case SVC_CROSSHAIRANGLE: sdst = "SVC_CROSSHAIRANGLE"; break;
	case SVC_SOUNDFADE: sdst = "SVC_SOUNDFADE"; break;
	case SVC_FILETXFERFAILED: sdst = "SVC_FILETXFERFAILED"; break;
	case SVC_HLTV: sdst = "SVC_HLTV"; break;
	case SVC_DIRECTOR: sdst = "SVC_DIRECTOR"; break;
	case SVC_VOICEINIT: sdst = "SVC_VOICEINIT"; break;
	case SVC_VOICEDATA: sdst = "SVC_VOICEDATA"; break;
	case SVC_SENDEXTRAINFO: sdst = "SVC_SENDEXTRAINFO"; break;
	case SVC_TIMESCALE: sdst = "SVC_TIMESCALE"; break;
	case SVC_RESOURCELOCATION: sdst = "SVC_RESOURCELOCATION"; break;
	case SVC_SENDCVARVALUE: sdst = "SVC_SENDCVARVALUE"; break;
	case SVC_SENDCVARVALUE2: sdst = "SVC_SENDCVARVALUE2"; break;
	default:
		if (msg_type == giPrecacheGrunt) sdst = "giPrecacheGrunt";
		else if (msg_type == giPrecacheGrunt) sdst = "giPrecacheGrunt";
		else if (msg_type == gmsgShake) sdst = "gmsgShake";
		else if (msg_type == gmsgFade) sdst = "gmsgFade";
		else if (msg_type == gmsgSelAmmo) sdst = "gmsgSelAmmo";
		else if (msg_type == gmsgFlashlight) sdst = "gmsgFlashlight";
		else if (msg_type == gmsgFlashBattery) sdst = "gmsgFlashBattery";
		else if (msg_type == gmsgResetHUD) sdst = "gmsgResetHUD";
		else if (msg_type == gmsgInitHUD) sdst = "gmsgInitHUD";
		else if (msg_type == gmsgShowGameTitle) sdst = "gmsgShowGameTitle";
		else if (msg_type == gmsgCurWeapon) sdst = "gmsgCurWeapon";
		else if (msg_type == gmsgHealth) sdst = "gmsgHealth";
		else if (msg_type == gmsgDamage) sdst = "gmsgDamage";
		else if (msg_type == gmsgBattery) sdst = "gmsgBattery";
		else if (msg_type == gmsgTrain) sdst = "gmsgTrain";
		else if (msg_type == gmsgLogo) sdst = "gmsgLogo";
		else if (msg_type == gmsgWeaponList) sdst = "gmsgWeaponList";
		else if (msg_type == gmsgAmmoX) sdst = "gmsgAmmoX";
		else if (msg_type == gmsgHudText) sdst = "gmsgHudText";
		else if (msg_type == gmsgDeathMsg) sdst = "gmsgDeathMsg";
		else if (msg_type == gmsgScoreInfo) sdst = "gmsgScoreInfo";
		else if (msg_type == gmsgTeamInfo) sdst = "gmsgTeamInfo";
		else if (msg_type == gmsgTeamScore) sdst = "gmsgTeamScore";
		else if (msg_type == gmsgGameMode) sdst = "gmsgGameMode";
		else if (msg_type == gmsgMOTD) sdst = "gmsgMOTD";
		else if (msg_type == gmsgServerName) sdst = "gmsgServerName";
		else if (msg_type == gmsgAmmoPickup) sdst = "gmsgAmmoPickup";
		else if (msg_type == gmsgWeapPickup) sdst = "gmsgWeapPickup";
		else if (msg_type == gmsgItemPickup) sdst = "gmsgItemPickup";
		else if (msg_type == gmsgHideWeapon) sdst = "gmsgHideWeapon";
		else if (msg_type == gmsgSetCurWeap) sdst = "gmsgSetCurWeap";
		else if (msg_type == gmsgSayText) sdst = "gmsgSayText";
		else if (msg_type == gmsgTextMsg) sdst = "gmsgTextMsg";
		else if (msg_type == gmsgSetFOV) sdst = "gmsgSetFOV";
		else if (msg_type == gmsgShowMenu) sdst = "gmsgShowMenu";
		else if (msg_type == gmsgGeigerRange) sdst = "gmsgGeigerRange";
		else if (msg_type == gmsgTeamNames) sdst = "gmsgTeamNames";
		else if (msg_type == gmsgStatusText) sdst = "gmsgStatusText";
		else if (msg_type == gmsgStatusValue) sdst = "gmsgStatusValue";
		else if (msg_type == gmsgToxicCloud) sdst = "gmsgToxicCloud";
		else sdst = UTIL_VarArgs("%d", msg_type);
		break;
	}

	return sdst;
}

void log_msg(msg_info& msg) {
	if (!mp_debugmsg.value) {
		return;
	}

	std::string originStr = msg.hasOrigin ? UTIL_VarArgs("(%X %X %X)",
		*(int*)&msg.pOrigin[0], *(int*)&msg.pOrigin[1], *(int*)&msg.pOrigin[2]) : "NULL";
	std::string entStr = msg.name[0] != 0 ? std::string(msg.name) : "NULL";
	std::string argStr = "";
	for (int i = 0; i < msg.numMsgParts; i++) {
		switch (msg.parts[i].type) {
		case MFUNC_BYTE:
			argStr += UTIL_VarArgs(" B-%X", msg.parts[i].iValue);
			break;
		case MFUNC_CHAR:
			argStr += UTIL_VarArgs(" C-%X", msg.parts[i].iValue);
			break;
		case MFUNC_SHORT:
			argStr += UTIL_VarArgs(" S-%X", msg.parts[i].iValue);
			break;
		case MFUNC_LONG:
			argStr += UTIL_VarArgs(" L-%X", msg.parts[i].iValue);
			break;
		case MFUNC_ANGLE:
			argStr += UTIL_VarArgs(" A-%X", *(int*)&msg.parts[i].fValue);
			break;
		case MFUNC_COORD:
			argStr += UTIL_VarArgs(" F-%X", *(int*)&msg.parts[i].fValue);
			break;
		case MFUNC_STRING:
			if (msg.parts[i].sValue >= g_msgStrPool && msg.parts[i].sValue < g_msgStrPool + 512) {
				argStr += UTIL_VarArgs(" \"%s\"", msg.parts[i].sValue);
			}
			else {
				argStr += UTIL_VarArgs(" \"\"");
			}
			break;
		case MFUNC_ENTITY:
			argStr += UTIL_VarArgs(" E-%X", msg.parts[i].iValue);
			break;
		default:
			break;
		}
	}

	float now = g_engfuncs.pfnTime();

	std::string log = UTIL_VarArgs("T%.2f MSG(%s, %s, %s, %s)%s",
		now, msgDestStr(msg.msg_dest), msgTypeStr(msg.msg_type),
		originStr.c_str(), entStr.c_str(), argStr.c_str());

	// forget old messages
	while (!g_messageHistory.empty() && now - g_messageHistory.front().time > g_messageHistoryCutoff) {
		g_messageHistory.pop();
	}
	
	msg_hist_item item;
	item.time = now;
	item.msg = log;
	g_messageHistory.push(item);
}

void writeNetworkMessageHistory(std::string reason) {
	float now = g_engfuncs.pfnTime();
	while (!g_messageHistory.empty() && now - g_messageHistory.front().time > g_messageHistoryCutoff) {
		g_messageHistory.pop();
	}

	while (!g_messageHistory.empty()) {
		msg_hist_item item = g_messageHistory.front();
		g_messageHistory.pop();
		writeDebugLog(g_debugFile, g_debugLogName, "debug", item.msg);
	}
	writeDebugLog(g_debugFile, g_debugLogName, "debug", UTIL_VarArgs("T%.2f ", g_engfuncs.pfnTime()) + reason + "\n");
	g_debugFile.flush();
}

void clearNetworkMessageHistory() {
	while (!g_messageHistory.empty()) {
		g_messageHistory.pop();
	}
}

void MESSAGE_BEGIN(int msg_dest, int msg_type, const float* pOrigin, edict_t* ed) {
	if (mp_debugmsg.value) {
		g_lastMsg.msg_dest = msg_dest;
		g_lastMsg.msg_type = msg_type;
		g_lastMsg.hasOrigin = pOrigin != NULL;
		if (pOrigin) {
			memcpy(g_lastMsg.pOrigin, pOrigin, sizeof(float) * 3);
		}
		g_lastMsg.entIdx = ed ? ENTINDEX(ed) : 0;
		g_lastMsg.numMsgParts = 0;
		g_nextStrOffset = 0;
		if (ed)
			strcpy_safe(g_lastMsg.name, STRING(ed->v.netname), 32);
		else
			g_lastMsg.name[0] = 0;
	}

	(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ed);
}

void WRITE_BYTE(int iValue) {
	add_msg_part(MFUNC_BYTE, iValue);
	g_engfuncs.pfnWriteByte(iValue);
}

void WRITE_CHAR(int iValue) {
	add_msg_part(MFUNC_CHAR, iValue);
	g_engfuncs.pfnWriteChar(iValue);
}

void WRITE_SHORT(int iValue) {
	add_msg_part(MFUNC_SHORT, iValue);
	g_engfuncs.pfnWriteShort(iValue);
}

void WRITE_LONG(int iValue) {
	add_msg_part(MFUNC_LONG, iValue);
	g_engfuncs.pfnWriteLong(iValue);
}

void WRITE_ANGLE(float fValue) {
	add_msg_part(MFUNC_ANGLE, fValue);
	g_engfuncs.pfnWriteAngle(fValue);
}

void WRITE_COORD(float fValue) {
	add_msg_part(MFUNC_COORD, fValue);
	g_engfuncs.pfnWriteCoord(fValue);
}

void WRITE_STRING(const char* sValue) {
	add_msg_part(sValue);
	g_engfuncs.pfnWriteString(sValue);
}

void WRITE_ENTITY(int iValue) {
	add_msg_part(MFUNC_ENTITY, iValue);
	g_engfuncs.pfnWriteEntity(iValue);
}

void MESSAGE_END() {
	log_msg(g_lastMsg);
	g_engfuncs.pfnMessageEnd();
}

bool UTIL_isSafeEntIndex(int idx, const char* action) {
	if (sv_max_client_edicts && idx >= sv_max_client_edicts->value) {
		ALERT(at_error, "Can't %s for edict %d '%s' (sv_max_client_edicts = %d)\n",
			action, idx, STRING(INDEXENT(idx)->v.classname), (int)sv_max_client_edicts->value);
		return false;
	}

	return true;
}

std::vector<std::string> splitString(std::string str, const char* delimitters)
{
	std::vector<std::string> split;
	size_t start = 0;
	size_t end = str.find_first_of(delimitters);

	while (end != std::string::npos)
	{
		split.push_back(str.substr(start, end - start));
		start = end + 1;
		end = str.find_first_of(delimitters, start);
	}

	split.push_back(str.substr(start));

	return split;
}

std::string toLowerCase(std::string str) {
	std::string out = str;

	for (int i = 0; str[i]; i++) {
		out[i] = tolower(str[i]);
	}

	return out;
}

std::string toUpperCase(std::string str) {
	std::string out = str;

	for (int i = 0; str[i]; i++) {
		out[i] = toupper(str[i]);
	}

	return out;
}

std::string trimSpaces(std::string s) {
	size_t start = s.find_first_not_of(" \t\n\r");
	size_t end = s.find_last_not_of(" \t\n\r");
	return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::string replaceString(std::string subject, std::string search, std::string replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

bool boxesIntersect(const Vector& mins1, const Vector& maxs1, const Vector& mins2, const Vector& maxs2) {
	return  (maxs1.x >= mins2.x && mins1.x <= maxs2.x) &&
		(maxs1.y >= mins2.y && mins1.y <= maxs2.y) &&
		(maxs1.z >= mins2.z && mins1.z <= maxs2.z);
}

float clampf(float val, float min, float max) {
	if (val < min) {
		return min;
	}
	if (val > max) {
		return max;
	}
	return val;
}

int clampi(int val, int min, int max) {
	if (val < min) {
		return min;
	}
	if (val > max) {
		return max;
	}
	return val;
}

uint64_t getFileModifiedTime(const char* path) {
	struct stat result;
	if (stat(path, &result) == 0) {
		return result.st_mtime;
	}

	return 0;
}

bool fileExists(const char* path) {
	FILE* file = fopen(path, "r");
	if (file) {
		fclose(file);
		return true;
	}
	return false;
}

std::string normalize_path(std::string s)
{
	if (s.size() == 0)
		return s;

	replace(s.begin(), s.end(), '\\', '/');

	std::vector<std::string> parts = splitString(s, "/");
	int depth = 0;
	for (int i = 0; i < (int)parts.size(); i++)
	{
		depth++;
		if (parts[i] == "..")
		{
			depth--;
			if (depth == 0)
			{
				// can only .. up to game root folder, and not any further
				parts.erase(parts.begin() + i);
				i--;
			}
			else if (i > 0)
			{
				parts.erase(parts.begin() + i);
				parts.erase(parts.begin() + (i - 1));
				i -= 2;
			}
		}
	}
	s = "";
	for (int i = 0; i < (int)parts.size(); i++)
	{
		if (i > 0) {
			s += '/';
		}
		s += parts[i];
	}

	return s;
}

std::string getGameFilePath(const char* path) {
	static char gameDir[MAX_PATH];
	GET_GAME_DIR(gameDir);

	std::string searchPaths[2] = {
		normalize_path(gameDir + std::string("/") + path),
		normalize_path(gameDir + std::string("_downloads/") + path),
	};

	for (int i = 0; i < 2; i++) {
		if (fileExists(searchPaths[i].c_str())) {
			return searchPaths[i];
		}
	}

	return "";
}

void te_debug_beam(Vector start, Vector end, uint8_t life, RGBA c, int msgType, edict_t* dest)
{
	MESSAGE_BEGIN(msgType, SVC_TEMPENTITY, NULL, dest);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(start.x);
	WRITE_COORD(start.y);
	WRITE_COORD(start.z);
	WRITE_COORD(end.x);
	WRITE_COORD(end.y);
	WRITE_COORD(end.z);
	WRITE_SHORT(g_engfuncs.pfnModelIndex("sprites/laserbeam.spr"));
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	WRITE_BYTE(life);
	WRITE_BYTE(16);
	WRITE_BYTE(0);
	WRITE_BYTE(c.r);
	WRITE_BYTE(c.g);
	WRITE_BYTE(c.b);
	WRITE_BYTE(c.a); // actually brightness
	WRITE_BYTE(0);
	MESSAGE_END();
}

// WAVE file header format
#pragma pack(push, 1)
struct RIFF_HEADER {
	char riff[4];		// RIFF string
	uint32_t overall_size;	// overall size of file in bytes
	char wave[4];		// WAVE string
};

struct RIFF_FMT_PCM {
	uint16_t format_type;		// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t channels;			// no.of channels
	uint32_t sample_rate;		// sampling rate (blocks per second)
	uint32_t byterate;			// SampleRate * NumChannels * BitsPerSample/8
	uint16_t block_align;		// NumChannels * BitsPerSample/8
	uint16_t bits_per_sample;	// bits per sample, 8- 8bits, 16- 16 bits etc
};

struct WAVE_CHUNK_FMT {
	// common format fields
	uint16_t format_type;		// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
	uint16_t channels;			// no.of channels
	uint32_t sample_rate;		// sampling rate (blocks per second)
	uint32_t byterate;			// SampleRate * NumChannels * BitsPerSample/8
	uint16_t block_align;		// NumChannels * BitsPerSample/8
	uint16_t bits_per_sample;	// bits per sample, 8- 8bits, 16- 16 bits etc

	// non-pcm format field
	uint16_t cbSize;			// size of the extension

	// extensible format fields
	uint16_t validBitsPerSample;
	uint32_t dwChannelMask;
	uint8_t subFormat[16];
};

struct WAVE_CUE_HEADER {
	uint32_t numCuePoints;
	// WAVE_CUE structs follow
};

struct WAVE_CUE {
	uint32_t id; // A unique number for the point used by other chunks to identify the cue point. For example, a playlist chunk creates a playlist by referring to cue points, which themselves define points somewhere in the file
	uint32_t position; // If there is no playlist chunk, this value is zero. If there is a playlist chunk, this value is the sample at which the cue point should occur
	char dataChunkId[4]; // Either "data" or "slnt" depending on whether the cue occurs in a data chunk or in a silent chunk
	uint32_t chunkStart; // The position of the start of the data chunk that contains the cue point. If there is a wave list chunk, this value is the byte position of the chunk that contains the cue. If there is no wave list chunk, there is only one data chunk in the file and this value is zero
	uint32_t blockStart; // The byte position of the cue in the "data" or "slnt" chunk. If this is an uncompressed PCM file, this is counted from the beginning of the chunk's data. If this is a compressed file, the byte position can be counted from the last byte from which one can start decompressing to find the cue
	uint32_t sampleStart; // The position of the cue in number of bytes from the start of the block
};

struct WAVE_LIST_HEADER {
	char listType[4];
};

struct WAVE_LIST_INFO_HEADER {
	char infoType[4];
	uint32_t textSize;
};

struct WAVE_ADTL_HEADER {
	char adtlType[4];
	uint32_t subSize;
	uint32_t cueId;
	// ascii text follows, if "labl" or "note" chunk
};

// for "ltxt" adtl chunks
struct WAVE_CUE_LABEL {
	uint32_t sampleLength;
	uint32_t purposeId;
	uint16_t country;
	uint16_t lang;
	uint16_t dialect;
	uint16_t codePage;
	// ascii text follows
};

struct WAVE_CHUNK_HEADER {
	uint8_t name[4];
	uint32_t size;
};
#pragma pack(pop)

WavInfo getWaveFileInfo(const char* path) {
	if (g_wavInfos.find(path) != g_wavInfos.end()) {
		return g_wavInfos[path];
	}

	std::string fpath = getGameFilePath(UTIL_VarArgs("sound/%s", path));

	WavInfo info;
	info.durationMillis = 0;
	info.isLooped = false;

	int sampleRate = 0;
	int bytesPerSample = 0;
	int numSamples = 0;
	int read = 0;
	int fsize = 0;
	FILE* file = NULL;

	float durationSeconds = 0;
	bool fatalWave = false;
	float cues[2];
	int numCues = 0;

	if (!fpath.size()) {
		ALERT(at_error, "Missing WAVE file: %s\n", path);
		goto cleanup;
	}

	// open file
	file = fopen(fpath.c_str(), "rb");
	if (file == NULL) {
		ALERT(at_error, "Failed to open WAVE file: %s\n", fpath.c_str());
		goto cleanup;
	}

	fseek(file, 0, SEEK_END);
	fsize = ftell(file);
	fseek(file, 0, SEEK_SET);

	{
		RIFF_HEADER header;
		read = fread(&header, sizeof(RIFF_HEADER), 1, file);

		if (!read || strncmp((const char*)header.riff, "RIFF", 4)
			|| strncmp((const char*)header.wave, "WAVE", 4)) {
			ALERT(at_error, "Invalid WAVE header: %s\n", fpath.c_str());
			goto cleanup;
		}
	}

	//ALERT(at_console, "%s\n", path);

	//static char infoText[512];

	while (1) {
		if (ftell(file) + 8 >= fsize) {
			break;
		}

		WAVE_CHUNK_HEADER chunk;
		read = fread(&chunk, sizeof(WAVE_CHUNK_HEADER), 1, file);
		
		if (!read) {
			break; // end of file
		}

		std::string chunkName = std::string((const char*)chunk.name, 4);
		//ALERT(at_console, "    Read chunk %s (%d)\n", chunkName.c_str(), chunk.size);

		int seekSize = ((chunk.size + 1) / 2) * 2; // round up to nearest word size

		if (chunkName == "fmt ") {

			if (chunk.size == 16 || chunk.size == 18 || chunk.size == 40) {
				WAVE_CHUNK_FMT cdata;
				read = fread(&cdata, chunk.size, 1, file);

				bytesPerSample = cdata.channels * (cdata.bits_per_sample / 8);
				sampleRate = cdata.sample_rate;

				if (!read || !bytesPerSample || !sampleRate) {
					ALERT(at_error, "Invalid WAVE fmt chunk: %s\n", fpath.c_str());
					goto cleanup;
				}
			}
			else {
				ALERT(at_error, "Invalid WAVE fmt chunk: %s\n", fpath.c_str());
				goto cleanup;
			}
		}
		else if (chunkName == "data") {
			if (bytesPerSample && sampleRate && chunk.size) {
				numSamples = chunk.size / bytesPerSample;
				info.durationMillis = ((float)numSamples / sampleRate) * 1000.0f;
			}

			fseek(file, seekSize, SEEK_CUR);
		}
		else if (chunkName == "cue ") {
			WAVE_CUE_HEADER cdata;
			read = fread(&cdata, sizeof(WAVE_CUE_HEADER), 1, file);

			if (!read) {
				ALERT(at_error, "Invalid WAVE cue chunk: %s\n", fpath.c_str());
				goto cleanup;
			}

			// How cue points work:
			// 1  cue point  = loop starts at the cue point and ends at an equal distance from the end
			// 2  cue points = loop starts at the 1st cue and ends at the 2nd cue
			// 3+ cue points = same as 2 and the extras are ignored
			//
			// All fields besides "sampleStart" are ignored
			// inverted cue points will crash the client
			// (possible with 1 cue point if placed after the middle of the file)
			
			//std::string cueString = "        cue points: ";
			for (int i = 0; i < (int)cdata.numCuePoints && i < 2; i++) {
				WAVE_CUE cue;
				read = fread(&cue, sizeof(WAVE_CUE), 1, file);

				std::string dataChunkId = std::string(cue.dataChunkId, 4);
				if (!read || (dataChunkId != "data" && dataChunkId != "slnt")) {
					ALERT(at_error, "Invalid WAVE cue chunk: %s\n", fpath.c_str());
					goto cleanup;
				}

				float sampTime = sampleRate ? cue.sampleStart / (float)sampleRate : 0;
				cues[i] = sampTime;
				//cueString += UTIL_VarArgs("%.2f  ", sampTime);
			}
			//cueString += "\n";
			//ALERT(at_console, cueString.c_str());
			
			info.isLooped = cdata.numCuePoints > 0;
			numCues = cdata.numCuePoints;
		}
		/*
		else if (chunkName == "LIST") {
			WAVE_LIST_HEADER cdata;
			read = fread(&cdata, sizeof(WAVE_LIST_HEADER), 1, file);

			if (!read) {
				ALERT(at_error, "Invalid WAVE list chunk: %s\n", fpath.c_str());
				goto cleanup;
			}

			std::string listType = std::string(cdata.listType, 4);

			ALERT(at_console, "        type: %s\n", listType.c_str());
			if (!strncmp(cdata.listType, "INFO", 4)) {
				WAVE_LIST_INFO_HEADER iheader;
				read = fread(&iheader, sizeof(WAVE_LIST_INFO_HEADER), 1, file);

				if (!read) {
					ALERT(at_error, "Invalid WAVE list chunk: %s\n", fpath.c_str());
					goto cleanup;
				}

				std::string infoType = std::string(iheader.infoType, 4);

				int readSize = V_min(511, iheader.textSize);
				read = fread(infoText, readSize, 1, file);
				infoText[readSize] = '\0';

				ALERT(at_console, "        %s (%d): %s\n", infoType.c_str(), iheader.textSize, infoText);
			}
			else if (!strncmp(cdata.listType, "adtl", 4)) {
				WAVE_ADTL_HEADER aheader;
				read = fread(&aheader, sizeof(WAVE_ADTL_HEADER), 1, file);

				std::string adtlType = std::string(aheader.adtlType, 4);

				if (!strncmp(aheader.adtlType, "labl", 4) || !strncmp(aheader.adtlType, "note", 4)) {
					int headerSize = sizeof(WAVE_ADTL_HEADER) + sizeof(WAVE_LIST_INFO_HEADER) + sizeof(WAVE_LIST_HEADER) + 4;
					int readSize = V_min(511, chunk.size - headerSize);
					read = fread(infoText, readSize, 1, file);
					infoText[readSize] = '\0';
					ALERT(at_console, "        %s for %d (%d): %s\n", adtlType.c_str(), aheader.cueId, readSize, infoText);
				}
				else if (!strncmp(aheader.adtlType, "ltxt", 4)) {
					WAVE_CUE_LABEL lheader;
					read = fread(&aheader, sizeof(WAVE_CUE_LABEL), 1, file);
				}
			}

			fseek(file, chunk.size - sizeof(WAVE_LIST_HEADER), SEEK_CUR);
		}
		*/
		else {
			fseek(file, seekSize, SEEK_CUR);
		}
	}

	durationSeconds = info.durationMillis / 1000.0f;
	fatalWave = false;

	if (numCues == 1 && cues[0] > durationSeconds * 0.495f) { // better safe than sorry here - don't use 0.5 exactly
		ALERT(at_error, "'%s' has 1 cue point and it was placed after the mid point! This file will crash clients.\n", path);
		fatalWave = true;
	}
	else if (numCues >= 2 && cues[0] > cues[1]) {
		ALERT(at_error, "'%s' start/end cue points are inverted! This file will crash clients.\n", path);
		fatalWave = true;
	}

	if (fatalWave) {
		// don't allow anyone to download this file! Not all server ops know that you need to rename
		// a file after updating it. Then if some people got the old file, they will crash and others
		// will be like "idk works for me" and people will have to delete their hl folder to fix this.
		// Or worse, they'll just live with it for years because it's just a few maps they crash on.
		ALERT(at_error, "Server shutting down to prevent distribution of the broken file.\n", path);
		g_engfuncs.pfnServerCommand("quit\n");
		g_engfuncs.pfnServerExecute();
	}

cleanup:
	g_wavInfos[path] = info;
	if (file) {
		fclose(file);
	}
	return info;
}

std::string lastMapName;

void DEBUG_MSG(ALERT_TYPE target, const char* format, ...) {
	static char log_line[4096];

	va_list vl;
	va_start(vl, format);
	vsnprintf(log_line, 4096, format, vl);
	va_end(vl);

#if defined(WIN32) && (_DEBUG)
	OutputDebugString(log_line);
#endif

	g_engfuncs.pfnAlertMessage(target, log_line);

	if (target == at_error) {
		if (lastMapName != STRING(gpGlobals->mapname)) {
			lastMapName = STRING(gpGlobals->mapname);
			writeDebugLog(g_errorFile, g_errorLogName, "error", "--- Errors for map: " + lastMapName);
		}

		std::string line = log_line;
		writeDebugLog(g_errorFile, g_errorLogName, "error", line.substr(0, line.size()-1));
		g_errorFile.flush();

		if (g_developer->value == 0)
			g_engfuncs.pfnServerPrint(log_line);
	}
}

Vector VecBModelOrigin(entvars_t* pevBModel)
{
	return pevBModel->absmin + (pevBModel->size * 0.5);
}

void PlayCDTrack(int iTrack)
{
	edict_t* pClient;

	// manually find the single player. 
	pClient = g_engfuncs.pfnPEntityOfEntIndex(1);

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	if (iTrack < -1 || iTrack > 30)
	{
		ALERT(at_console, "TriggerCDAudio - Track %d out of range\n");
		return;
	}

	if (iTrack == -1)
	{
		CLIENT_COMMAND(pClient, "cd stop\n");
	}
	else
	{
		char string[64];

		snprintf(string, 64, "cd play %3d\n", iTrack);
		CLIENT_COMMAND(pClient, string);
	}
}
