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
#include <cstdint>
#include "extdll.h"
#include "util.h"
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
#include "PluginManager.h"
#include "TextMenu.h"
#include "debug.h"
#include "hlds_hooks.h"

#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <chrono>
#include <mutex>

using namespace std::chrono;

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#define mkdir(path, perms) _mkdir(path)
#define stat _stat
#else
#include <unistd.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/statvfs.h>
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

std::string g_mp3Command;

std::unordered_map<std::string, int> g_admins;

std::thread::id g_main_thread_id = std::this_thread::get_id();
ThreadSafeQueue<AlertMsgCall> g_thread_prints;

TYPEDESCRIPTION	gEntvarsDescription[] =
{
	DEFINE_ENTITY_FIELD(classname, FIELD_STRING),
	DEFINE_ENTITY_GLOBAL_FIELD(globalname, FIELD_STRING),

	DEFINE_ENTITY_FIELD(origin, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_FIELD(oldorigin, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_FIELD(velocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(basevelocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(clbasevelocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(movedir, FIELD_VECTOR),

	DEFINE_ENTITY_FIELD(angles, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(avelocity, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(punchangle, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(v_angle, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(endpos, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(startpos, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(impacttime, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(starttime, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(fixangle, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(idealpitch, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(pitch_speed, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(ideal_yaw, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(yaw_speed, FIELD_FLOAT),

	DEFINE_ENTITY_FIELD(modelindex, FIELD_INTEGER),
	DEFINE_ENTITY_GLOBAL_FIELD(model, FIELD_MODELNAME),

	DEFINE_ENTITY_FIELD(viewmodel, FIELD_MODELNAME),
	DEFINE_ENTITY_FIELD(weaponmodel, FIELD_MODELNAME),

	DEFINE_ENTITY_FIELD(absmin, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_FIELD(absmax, FIELD_POSITION_VECTOR),
	DEFINE_ENTITY_GLOBAL_FIELD(mins, FIELD_VECTOR),
	DEFINE_ENTITY_GLOBAL_FIELD(maxs, FIELD_VECTOR),
	DEFINE_ENTITY_GLOBAL_FIELD(size, FIELD_VECTOR),

	DEFINE_ENTITY_FIELD(ltime, FIELD_TIME),
	DEFINE_ENTITY_FIELD(nextthink, FIELD_TIME),

	DEFINE_ENTITY_FIELD(solid, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(movetype, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(skin, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(body, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(effects, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(gravity, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(friction, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(light_level, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(gaitsequence, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(frame, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(scale, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(sequence, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(animtime, FIELD_TIME),
	DEFINE_ENTITY_FIELD(framerate, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(controller, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(blending, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(rendermode, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(renderamt, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(rendercolor, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(renderfx, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(health, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(frags, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(weapons, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(takedamage, FIELD_FLOAT),

	DEFINE_ENTITY_FIELD(deadflag, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(view_ofs, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(button, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(impulse, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(chain, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(dmg_inflictor, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(enemy, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(aiment, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(owner, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(groundentity, FIELD_EDICT),

	DEFINE_ENTITY_FIELD(spawnflags, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(flags, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(colormap, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(team, FIELD_INTEGER),

	DEFINE_ENTITY_FIELD(max_health, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(teleport_time, FIELD_TIME),
	DEFINE_ENTITY_FIELD(armortype, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(armorvalue, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(waterlevel, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(watertype, FIELD_INTEGER),

	// Having these fields be local to the individual levels makes it easier to test those levels individually.
	DEFINE_ENTITY_GLOBAL_FIELD(target, FIELD_STRING),
	DEFINE_ENTITY_GLOBAL_FIELD(targetname, FIELD_STRING),
	DEFINE_ENTITY_FIELD(netname, FIELD_STRING),
	DEFINE_ENTITY_FIELD(message, FIELD_STRING),

	DEFINE_ENTITY_FIELD(dmg_take, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(dmg_save, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(dmg, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(dmgtime, FIELD_TIME),

	DEFINE_ENTITY_FIELD(noise, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(noise1, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(noise2, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(noise3, FIELD_SOUNDNAME),
	DEFINE_ENTITY_FIELD(speed, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(air_finished, FIELD_TIME),
	DEFINE_ENTITY_FIELD(pain_finished, FIELD_TIME),
	DEFINE_ENTITY_FIELD(radsuit_finished, FIELD_TIME),

	DEFINE_ENTITY_FIELD(pContainingEntity, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(playerclass, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(maxspeed, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(fov, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(weaponanim, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(pushmsec, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(bInDuck, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(flTimeStepSound, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(flSwimTime, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(flDuckTime, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(iStepLeft, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(flFallVelocity, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(gamestate, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(oldbuttons, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(groupinfo, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(iuser1, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(iuser2, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(iuser3, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(iuser4, FIELD_INTEGER),
	DEFINE_ENTITY_FIELD(fuser1, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(fuser2, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(fuser3, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(fuser4, FIELD_FLOAT),
	DEFINE_ENTITY_FIELD(vuser1, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(vuser2, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(vuser3, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(vuser4, FIELD_VECTOR),
	DEFINE_ENTITY_FIELD(euser1, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(euser2, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(euser3, FIELD_EDICT),
	DEFINE_ENTITY_FIELD(euser4, FIELD_EDICT),
};

const int ENTVARS_COUNT = (sizeof(gEntvarsDescription) / sizeof(gEntvarsDescription[0]));

int g_groupmask = 0;
int g_groupop = 0;

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
	pent->v.oldorigin = pent->v.origin; // for func_clip

	float rgfl[3];
	vecGoal.CopyToArray(rgfl);
//		return MOVE_TO_ORIGIN ( pent, rgfl, flDist, iMoveType ); 
	MOVE_TO_ORIGIN ( pent, rgfl, flDist, iMoveType ); 
}


int UTIL_EntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs,
	int flagMask, bool ignoreDead, bool collisionOnly )
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

		Vector edMin;
		Vector edMax;
		if (collisionOnly) {
			edMin = pEdict->v.origin - pEdict->v.mins;
			edMax = pEdict->v.origin - pEdict->v.maxs;
		}
		else {
			edMin = pEdict->v.absmin;
			edMax = pEdict->v.absmin;
		}
			
		if (mins.x > edMax.x ||
			mins.y > edMax.y ||
			mins.z > edMax.z ||
			maxs.x < edMin.x ||
			maxs.y < edMin.y ||
			maxs.z < edMin.z)
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
CBasePlayer* UTIL_PlayerByIndex( int playerIndex )
{
	if (playerIndex < 0 && playerIndex > gpGlobals->maxClients) {
		return NULL;
	}

	edict_t* pPlayerEdict = INDEXENT( playerIndex );

	return IsValidPlayer(pPlayerEdict) ? (CBasePlayer*)CBaseEntity::Instance( pPlayerEdict ) : NULL;
}

CBasePlayer* UTIL_PlayerByUserId(int userid)
{
	if (userid > 0)
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

			if (pPlayer && g_engfuncs.pfnGetPlayerUserId(pPlayer->edict()) == userid)
				return pPlayer;
		}
	}

	return NULL;
}

CBasePlayer* UTIL_PlayerBySteamId(const char* id) {
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		if (pPlayer && !strcmp(id, pPlayer->GetSteamID())) {
			return pPlayer;
		}
	}

	return NULL;
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
	return ent && ent->m_pvsPlayers;
}

bool IsValidPlayer(edict_t* edict) {
	return edict && !edict->free && (edict->v.flags & FL_CLIENT) && STRING(edict->v.netname)[0] != '\0';
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

// This logic makes sure that a network message does not get sent to
// a client that can't handle a high entity index, if one is given.
// entindex is assumed to be the entity used for PVS/PAS tests, if using that message mode.
// Required variadic args:
//    int msgMode = the network message mode
//    const float* msgOrigin = origin used by the engine for some message send modes (PVS/PAS)
//    edict_t* targetEnt = the target entity of the network message (NULL for broadcasts)
// NOTE: This code is mostly duplicated in UTIL_BeamEnts, which checks multiple indexes
#define SAFE_MESSAGE_ENT_LOGIC(msg_func, entindex, ...) \
	if (entindex < MAX_LEGACY_CLIENT_ENTS) { \
		msg_func(entindex, __VA_ARGS__); \
		return; \
	} \
	\
	if (msgMode == MSG_ONE || msgMode == MSG_ONE_UNRELIABLE) { \
		if (UTIL_isSafeEntIndex(targetEnt, entindex, __FUNCTION__)) { \
			msg_func(entindex, __VA_ARGS__); \
		} \
		return; \
	} \
	\
	int originalMsgMode = msgMode; /* saved in case PVS/PAS was passed, for testing later */ \
	msgMode = GetIndividualNetMessageMode(msgMode); /* sending individual messages instead of a broadcast */\
	msgOrigin = NULL; /* don't send this to the engine because PVS/PAS testing will be done here */ \
	for (int i = 1; i <= gpGlobals->maxClients; i++) { \
		targetEnt = INDEXENT(i); \
	\
		if (TestMsgVis(targetEnt, entindex, originalMsgMode) && UTIL_isSafeEntIndex(targetEnt, entindex, __FUNCTION__)) { \
			msg_func(entindex, __VA_ARGS__); \
		} \
	}

// tests if a player would receive a message given a send mode
// and the entity which is emitting the message
bool TestMsgVis(edict_t* plr, int testEntIdx, int netMsgMode) {
	CBaseEntity* ent = CBaseEntity::Instance(INDEXENT(testEntIdx));

	if (!ent) {
		return false;
	}

	switch (netMsgMode) {
	case MSG_PVS:
	case MSG_PVS_R:
		return ent->InPVS(plr);
	case MSG_PAS:
	case MSG_PAS_R:
		return ent->InPAS(plr);
	default:
		return true;
	}
}

// converts a broadcast message mode to an individual mode
// while preserving the reliable/unreliable channel selection
int GetIndividualNetMessageMode(int msgMode) {
	switch (msgMode) {
	case MSG_ALL:
	case MSG_PVS_R:
	case MSG_PAS_R:
	case MSG_ONE:
		return MSG_ONE;
	default:
		return MSG_ONE_UNRELIABLE;
	}
}

void UTIL_BeamFollow_msg(int entindex, int modelIdx, int life, int width, RGBA color, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_BEAMFOLLOW);
	WRITE_SHORT(entindex);
	WRITE_SHORT(modelIdx);
	WRITE_BYTE(life);
	WRITE_BYTE(width);
	WRITE_BYTE(color.r);
	WRITE_BYTE(color.g);
	WRITE_BYTE(color.b);
	WRITE_BYTE(color.a);
	MESSAGE_END();
}
void UTIL_BeamFollow(int entindex, int modelIdx, int life, int width, RGBA color, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_BeamFollow_msg, entindex, modelIdx, life, width, color, msgMode, msgOrigin, targetEnt);
}

void UTIL_Fizz_msg(int entindex, int modelIdx, uint8_t density, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_FIZZ);
	WRITE_SHORT(entindex);
	WRITE_SHORT(modelIdx);
	WRITE_BYTE(density);
	MESSAGE_END();
}
void UTIL_Fizz(int entindex, int modelIdx, uint8_t density, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_Fizz_msg, entindex, modelIdx, density, msgMode, msgOrigin, targetEnt);
}

void UTIL_ELight_msg(int entindex, int attachment, Vector origin, float radius, RGBA color, int life, float decay, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_ELIGHT);
	WRITE_SHORT(entindex + (attachment << 12));
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_COORD(radius);
	WRITE_BYTE(color.r);
	WRITE_BYTE(color.g);
	WRITE_BYTE(color.b);
	WRITE_BYTE(life);
	WRITE_COORD(decay);
	MESSAGE_END();
}
void UTIL_ELight(int entindex, int attachment, Vector origin, float radius, RGBA color, int life, float decay, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_ELight_msg, entindex, attachment, origin, radius, color, life, decay, msgMode, msgOrigin, targetEnt);
}

void UTIL_BeamEntPoint_msg(int entindex, int attachment, Vector point, int modelIdx, uint8_t frameStart,
	uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, 
	int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_BEAMENTPOINT);
	WRITE_SHORT(entindex + (attachment << 12));
	WRITE_COORD(point.x);
	WRITE_COORD(point.y);
	WRITE_COORD(point.z);
	WRITE_SHORT(modelIdx);
	WRITE_BYTE(frameStart);
	WRITE_BYTE(framerate);
	WRITE_BYTE(life);
	WRITE_BYTE(width);
	WRITE_BYTE(noise);
	WRITE_BYTE(color.r);
	WRITE_BYTE(color.g);
	WRITE_BYTE(color.b);
	WRITE_BYTE(color.a);
	WRITE_BYTE(speed);
	MESSAGE_END();
}
void UTIL_BeamEntPoint(int entindex, int attachment, Vector point, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_BeamEntPoint_msg, entindex, attachment, point, modelIdx, frameStart, framerate, life, width, noise, color, speed, msgMode, msgOrigin, targetEnt);
}

void UTIL_BeamEnts_msg(int entindex, int attachment, int entindex2, int attachment2, bool ringMode, int modelIdx, uint8_t frameStart,
	uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed,
	int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(ringMode ? TE_BEAMRING : TE_BEAMENTS);
	WRITE_SHORT(entindex + (attachment << 12));
	WRITE_SHORT(entindex2 + (attachment2 << 12));
	WRITE_SHORT(g_sModelIndexLaser);
	WRITE_BYTE(frameStart);
	WRITE_BYTE(framerate);
	WRITE_BYTE(life);
	WRITE_BYTE(width);
	WRITE_BYTE(noise);
	WRITE_BYTE(color.r);
	WRITE_BYTE(color.g);
	WRITE_BYTE(color.b);
	WRITE_BYTE(color.a);
	WRITE_BYTE(speed);
	MESSAGE_END();
}
void UTIL_BeamEnts(int startEnt, int startAttachment, int endEnt, int endAttachment, bool ringMode, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	// NOTE: This code is mostly duplicated in the SAFE_MESSAGE_ENT_LOGIC macro
	if (startEnt < MAX_LEGACY_CLIENT_ENTS && endEnt < MAX_LEGACY_CLIENT_ENTS) {
		UTIL_BeamEnts_msg(startEnt, startAttachment, endEnt, endAttachment, ringMode, modelIdx, frameStart,
			framerate, life, width, noise, color, speed, msgMode, msgOrigin, targetEnt);
		return;
	}

	if (msgMode == MSG_ONE || msgMode == MSG_ONE_UNRELIABLE) {
		if (UTIL_isSafeEntIndex(targetEnt, startEnt, __FUNCTION__) && UTIL_isSafeEntIndex(targetEnt, endEnt, __FUNCTION__)) {
			UTIL_BeamEnts_msg(startEnt, startAttachment, endEnt, endAttachment, ringMode, modelIdx, frameStart,
				framerate, life, width, noise, color, speed, msgMode, msgOrigin, targetEnt);
		}
		return;
	}

	int originalMsgMode = msgMode; /* saved in case PVS/PAS was passed, for testing later */
	msgMode = GetIndividualNetMessageMode(msgMode); /* sending individual messages instead of a broadcast */
	msgOrigin = NULL; /* don't send this to the engine because PVS/PAS testing will be done here */
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		targetEnt = INDEXENT(i);

		bool isVisible = TestMsgVis(targetEnt, startEnt, originalMsgMode) || TestMsgVis(targetEnt, endEnt, originalMsgMode);
		bool isSafeIndex = UTIL_isSafeEntIndex(targetEnt, startEnt, __FUNCTION__) && UTIL_isSafeEntIndex(targetEnt, endEnt, __FUNCTION__);

		if (isVisible && isSafeIndex) {
			UTIL_BeamEnts_msg(startEnt, startAttachment, endEnt, endAttachment, ringMode, modelIdx, frameStart,
				framerate, life, width, noise, color, speed, msgMode, msgOrigin, targetEnt);
		}
	}
}

void UTIL_BeamPoints(Vector start, Vector end, int modelIdx, uint8_t frameStart, uint8_t framerate, uint8_t life, uint8_t width, uint8_t noise, RGBA color, uint8_t speed, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(start.x);
	WRITE_COORD(start.y);
	WRITE_COORD(start.z);
	WRITE_COORD(end.x);
	WRITE_COORD(end.y);
	WRITE_COORD(end.z);
	WRITE_SHORT(modelIdx);
	WRITE_BYTE(frameStart);
	WRITE_BYTE(framerate);
	WRITE_BYTE(life);
	WRITE_BYTE(width);
	WRITE_BYTE(noise);
	WRITE_BYTE(color.r);
	WRITE_BYTE(color.g);
	WRITE_BYTE(color.b);
	WRITE_BYTE(color.a);
	WRITE_BYTE(speed);
	MESSAGE_END();
}

void UTIL_BSPDecal_msg(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_BSPDECAL);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_SHORT(decalIdx);
	WRITE_SHORT(entindex);
	if (entindex)
		WRITE_SHORT(ENT(entindex)->v.modelindex);
	MESSAGE_END();
}
void UTIL_BSPDecal(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_BSPDecal_msg, entindex, origin, decalIdx, msgMode, msgOrigin, targetEnt);
}

void UTIL_PlayerDecal_msg(int entindex, int playernum, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_PLAYERDECAL);
	WRITE_BYTE(playernum);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_SHORT(entindex);
	WRITE_BYTE(decalIdx);
	MESSAGE_END();
}
void UTIL_PlayerDecal(int entindex, int playernum, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_PlayerDecal_msg, entindex, playernum, origin, decalIdx, msgMode, msgOrigin, targetEnt);
}

void UTIL_GunshotDecal_msg(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(TE_GUNSHOTDECAL);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_SHORT(entindex);
	WRITE_BYTE(decalIdx);
	MESSAGE_END();
}
void UTIL_GunshotDecal(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_GunshotDecal_msg, entindex, origin, decalIdx, msgMode, msgOrigin, targetEnt);
}

void UTIL_Decal_msg(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	int mode = TE_DECAL;

	if (entindex) {
		mode = decalIdx > 255 ? TE_DECALHIGH : TE_DECAL;
	}
	else {
		mode = decalIdx > 255 ? TE_WORLDDECALHIGH : TE_WORLDDECAL;
	}
	
	MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, msgOrigin, targetEnt);
	WRITE_BYTE(mode);
	WRITE_COORD(origin.x);
	WRITE_COORD(origin.y);
	WRITE_COORD(origin.z);
	WRITE_BYTE(decalIdx);
	if (entindex)
		WRITE_SHORT(entindex);
	MESSAGE_END();
}
void UTIL_Decal(int entindex, Vector origin, int decalIdx, int msgMode, const float* msgOrigin, edict_t* targetEnt) {
	SAFE_MESSAGE_ENT_LOGIC(UTIL_Decal_msg, entindex, origin, decalIdx, msgMode, msgOrigin, targetEnt);
}


void WRITE_FLOAT(float f) {
	WRITE_LONG(*(uint32_t*)&f);
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

	char name[32];
	if (samp && *samp == '!') {
		if (SENTENCEG_Lookup(samp, name, 32) >= 0) {
			samp = name;
		}
		else {
			ALERT(at_error, "Bad sentence: %s\n", samp);
			return;
		}
	}

	if (dest) {
		if (UTIL_isSafeEntIndex(dest, ENTINDEX(entity), "play ambient sound")) {
			ambientsound_msg(entity, rgfl, samp, vol, attenuation, fFlags, pitch, MSG_ONE, dest);
		}
	}
	else if (ENTINDEX(entity) >= MAX_LEGACY_CLIENT_ENTS) {
		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			edict_t* plr = INDEXENT(i);

			if (IsValidPlayer(plr) && UTIL_isSafeEntIndex(plr, ENTINDEX(entity), "play ambient sound")) {
				ambientsound_msg(entity, rgfl, samp, vol, attenuation, fFlags, pitch, MSG_ONE, plr);
			}
		}
	}
	else {
		EMIT_AMBIENT_SOUND(entity, rgfl, samp, vol, attenuation, fFlags, pitch);
	}
}

void UTIL_PlayGlobalMp3(const char* path, bool loop, edict_t* target) {
	// surround with ; to prevent multiple commands being joined when sent in the same frame(?)
	// this fixes music sometimes not loading/starting/stopping
	std::string mp3Path = normalize_path(UTIL_VarArgs("sound/%s", path));
	std::string mp3Command = UTIL_VarArgs(";mp3 %s %s;", (loop ? "loop" : "play"), mp3Path.c_str());

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

unsigned short FixedUnsigned16( float value, float scale )
{
	int output;

	output = value * scale;
	if ( output < 0 )
		output = 0;
	if ( output > 0xFFFF )
		output = 0xFFFF;

	return (unsigned short)output;
}

short FixedSigned16( float value, float scale )
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

void UTIL_HudMessage( CBaseEntity *pEntity, const hudtextparms_t &textparms, const char *pMessage, int msgMode)
{
	bool isIndividual = msgMode == MSG_ONE || msgMode == MSG_ONE_UNRELIABLE;

	if (isIndividual && (!pEntity || !pEntity->IsNetClient()))
		return;

	pMessage = BreakupLongLines(pMessage);

	if (textparms.channel == -1) {
		int sz = 7 * sizeof(long) + 3 + strlen(pMessage);
		uint32_t color = (textparms.r1 << 16) | (textparms.g1 << 8) | textparms.b1;

		MESSAGE_BEGIN(msgMode, SVC_DIRECTOR, NULL, pEntity ? pEntity->edict() : NULL);
		WRITE_BYTE(sz); // message size
		WRITE_BYTE(6); // director message
		WRITE_BYTE(textparms.effect); // effect
		WRITE_LONG(color); // color
		WRITE_FLOAT(textparms.x); // x
		WRITE_FLOAT(textparms.y); // y
		WRITE_FLOAT(textparms.fadeinTime); // fade in
		WRITE_FLOAT(textparms.fadeoutTime); // fade out
		WRITE_FLOAT(textparms.holdTime); // hold time
		WRITE_FLOAT(textparms.fxTime); // fx time
		WRITE_STRING(pMessage); // fx time
		MESSAGE_END();
	}
	else {
		MESSAGE_BEGIN(msgMode, SVC_TEMPENTITY, NULL, pEntity ? pEntity->edict() : NULL);
		WRITE_BYTE(TE_TEXTMESSAGE);
		WRITE_BYTE(textparms.channel & 0xFF);

		WRITE_SHORT(FixedSigned16(textparms.x, 1 << 13));
		WRITE_SHORT(FixedSigned16(textparms.y, 1 << 13));
		WRITE_BYTE(textparms.effect);

		WRITE_BYTE(textparms.r1);
		WRITE_BYTE(textparms.g1);
		WRITE_BYTE(textparms.b1);
		WRITE_BYTE(textparms.a1);

		WRITE_BYTE(textparms.r2);
		WRITE_BYTE(textparms.g2);
		WRITE_BYTE(textparms.b2);
		WRITE_BYTE(textparms.a2);

		WRITE_SHORT(FixedUnsigned16(textparms.fadeinTime, 1 << 8));
		WRITE_SHORT(FixedUnsigned16(textparms.fadeoutTime, 1 << 8));
		WRITE_SHORT(FixedUnsigned16(textparms.holdTime, 1 << 8));

		if (textparms.effect == 2)
			WRITE_SHORT(FixedUnsigned16(textparms.fxTime, 1 << 8));

		WRITE_STRING(pMessage);
		MESSAGE_END();
	}	
}

void UTIL_HudMessageAll(const hudtextparms_t& textparms, const char* pMessage, int msgMode)
{
	UTIL_HudMessage(NULL, textparms, pMessage, MSG_ALL);
}

					 
extern int gmsgTextMsg, gmsgSayText;
void UTIL_ClientPrintAll(PRINT_TYPE print_type, const char *msg )
{
	if (print_type == print_chat) {
		MESSAGE_BEGIN(MSG_ALL, gmsgSayText, NULL);
		WRITE_BYTE(0);
		WRITE_STRING(msg);
		MESSAGE_END();
	}
	else {
		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			edict_t* ent = INDEXENT(i);
			if (IsValidPlayer(ent))
				CLIENT_PRINTF(ent, print_type, msg);
		}
	}
}

void UTIL_ClientPrint(CBaseEntity* client, PRINT_TYPE print_type, const char * msg)
{
	if (!client) {
		g_engfuncs.pfnServerPrint(msg);
		return;
	}
	if (!client->IsPlayer()) {
		return;
	}

	if (print_type == print_chat) {
		MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, client->edict());
		WRITE_BYTE(0);
		WRITE_STRING(msg);
		MESSAGE_END();
	}
	else {
		CLIENT_PRINTF(client->edict(), print_type, msg);
	}
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
	static std::mutex m; // only allow one thread at a time to access static buffers
	std::lock_guard<std::mutex> lock(m);

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
	
	UTIL_Decal(entityIndex, pTrace->vecEndPos, index);
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

	UTIL_PlayerDecal(ENTINDEX(pTrace->pHit), playernum, pTrace->vecEndPos, index);
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

	UTIL_GunshotDecal(ENTINDEX(pTrace->pHit), pTrace->vecEndPos, index, MSG_PAS, pTrace->vecEndPos);
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
	tempString[sizeof(tempString) - 1] = '\0';
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
	tempString[sizeof(tempString) - 1] = '\0';
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


void UTIL_PrecacheOther( const char *szClassname, std::unordered_map<std::string, std::string> keys)
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
void UTIL_LogPlayerEvent(const edict_t* plr, const char* fmt, ...)
{
	va_list			argptr;
	static char		string[1024];
	
	va_start ( argptr, fmt );
	vsnprintf ( string, 1024, fmt, argptr );
	va_end   ( argptr );

	// Print to server console
	g_engfuncs.pfnAlertMessage( at_logged, "\\%s\\%s\\ %s",
		plr->v.netname ? STRING(plr->v.netname) : "",
		GETPLAYERAUTHID(plr),
		string );
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
void UTIL_StripToken( const char *pKey, char *pDest, int nLen)
{
	int i = 0;

	while (i < nLen - 1 && pKey[i] && pKey[i] != '#')
	{
		pDest[i] = pKey[i];
		i++;
	}
	pDest[i] = 0;
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

std::unordered_map<std::string, std::string> loadReplacementFile(const char* path) {
	std::unordered_map<std::string, std::string> replacements;

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

		int comments = line.find("//");
		if (comments != -1) {
			line = line.substr(0, comments);
		}

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

		//PrintEntindexStats();
	}
}

void PrintEntindexStats() {
	int totalFreeLowPrio = 0;
	int totalFreeNormalPrio = 0;
	int totalFreeHighPrio = 0;
	int lowPrioMin = sv_max_client_edicts ? sv_max_client_edicts->value : MAX_LEGACY_CLIENT_ENTS;
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
	int lowPrioMin = sv_max_client_edicts ? sv_max_client_edicts->value : MAX_LEGACY_CLIENT_ENTS;
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
		for (int i = normalPrioMin; i < lowPrioMin; i++) {
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

	if (!ed) {
		ENTITYINIT initFunc = g_pluginManager.GetCustomEntityInitFunc(STRING(cname));

		if (initFunc) {
			ed = CREATE_ENTITY();
			ed->v.classname = cname;
			initFunc(&ed->v);
		}
		else {
			ALERT(at_console, "Invalid entity class '%s'\n", STRING(cname));
			return NULL;
		}
	}

	CBaseEntity* pEntity = CBaseEntity::Instance(ed);

	return pEntity ? RelocateEntIdx(pEntity)->edict() : ed;
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

std::string steamid64_to_steamid(uint64_t steam64) {
	steam64 -= 76561197960265728;

	if (steam64 & 1) {
		return "STEAM_0:1:" + std::to_string((steam64 - 1) / 2);
	}

	return "STEAM_0:0:" + std::to_string(steam64 / 2);
}

bool UTIL_isSafeEntIndex(edict_t* ent, int idx, const char* action) {
	CBasePlayer* plr = UTIL_PlayerByIndex(ENTINDEX(ent));
	if (!plr) {
		return false;
	}

	int maxEdicts = plr->GetMaxClientEdicts();

	if (idx >= maxEdicts) {
		ALERT(at_error, "Can't %s for edict %d '%s' (max edicts for \"%s\" is %d)\n",
			action, idx, STRING(INDEXENT(idx)->v.classname), plr->DisplayName(), maxEdicts);
		plr->SendLegacyClientWarning();
		return false;
	}

	return true;
}

std::vector<std::string> splitString(std::string str, const char* delimiters)
{
	std::vector<std::string> split;

	const char* c_str = str.c_str();
	size_t str_len = strlen(c_str);

	size_t start = strspn(c_str, delimiters);

	while (start < str_len)
	{
		size_t end = strcspn(c_str + start, delimiters);
		split.push_back(str.substr(start, end));
		start += end + strspn(c_str + start + end, delimiters);
	}

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

	std::string lowerPath = toLowerCase(path);

	std::string searchPaths[] = {
		normalize_path(gameDir + std::string("_addon/") + path),
		normalize_path(gameDir + std::string("_addon/") + lowerPath),
		normalize_path(gameDir + std::string("/") + path),
		normalize_path(gameDir + std::string("/") + lowerPath),
		normalize_path(gameDir + std::string("_downloads/") + path),
		normalize_path(gameDir + std::string("_downloads/") + lowerPath),
	};

	for (int i = 0; i < (int)ARRAY_SZ(searchPaths); i++) {
		if (fileExists(searchPaths[i].c_str())) {
			return searchPaths[i];
		}
	}

	return "";
}

void te_debug_beam(Vector start, Vector end, uint8_t life, RGBA c, int msgType, edict_t* dest)
{
	UTIL_BeamPoints(start, end, MODEL_INDEX("sprites/laserbeam.spr"), 0, 0, life, 16, 0,
		c, 0, msgType, NULL, dest);
}

std::string lastMapName;

void DEBUG_MSG(ALERT_TYPE target, const char* format, ...) {
	if (target < at_warning && g_developer->value == 0) {
		return;
	}

	static std::mutex m; // only allow one thread at a time to access static buffers
	std::lock_guard<std::mutex> lock(m);

	static char log_line[4096];

	va_list vl;
	va_start(vl, format);
	vsnprintf(log_line, 4096, format, vl);
	va_end(vl);

	if (std::this_thread::get_id() != g_main_thread_id) {
		g_thread_prints.enqueue({target, log_line}); // only the main thread can call engine functions
		return;
	}

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

void handleThreadPrints() {
	AlertMsgCall msg;

	for (int failsafe = 0; failsafe < 128; failsafe++) {
		if (g_thread_prints.dequeue(msg)) {
			ALERT(msg.atype, msg.msg.c_str());
		}
		else {
			break;
		}
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

std::string sanitize_cvar_value(std::string val) {
	val = replaceString(val, ";", "");
	val.erase(std::remove(val.begin(), val.end(), '"'), val.end());
	return val;
}

const char* getActiveWeapon(entvars_t* pev) {
	CBaseEntity* ent = CBaseEntity::Instance(pev);

	if (!ent || !ent->IsPlayer()) {
		return "";
	}

	CBasePlayer* plr = (CBasePlayer*)ent;
	
	return  plr->m_pActiveItem ? STRING(plr->m_pActiveItem->pev->classname) : "";
}

uint64_t getEpochMillis() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

double TimeDifference(uint64_t start, uint64_t end) {
	if (end > start) {
		return (end - start) / 1000.0;
	}
	else {
		return -((start - end) / 1000.0);
	}
}

void LoadAdminList(bool forceUpdate) {
	const char* ADMIN_LIST_FILE = CVAR_GET_STRING("adminlistfile");

	static uint64_t lastEditTime = 0;

	std::string fpath = getGameFilePath(ADMIN_LIST_FILE);

	if (fpath.empty()) {
		g_engfuncs.pfnServerPrint(UTIL_VarArgs("Missing admin list: '%s'\n", ADMIN_LIST_FILE));
		return;
	}

	uint64_t editTime = getFileModifiedTime(fpath.c_str());

	if (!forceUpdate && lastEditTime == editTime) {
		return; // no changes made
	}
	
	std::ifstream infile(fpath);

	if (!infile.is_open()) {
		ALERT(at_console, "Failed to open admins file: %s\n", ADMIN_LIST_FILE);
		return;
	}

	lastEditTime = editTime;

	g_admins.clear();

	std::string line;
	while (std::getline(infile, line)) {
		if (line.empty()) {
			continue;
		}

		// strip comments
		int endPos = line.find("//");
		if (endPos != -1)
			line = trimSpaces(line.substr(0, endPos));

		if (line.length() < 1) {
			continue;
		}

		int adminLevel = ADMIN_YES;

		if (line[0] == '*') {
			adminLevel = ADMIN_OWNER;
			line = line.substr(1);
		}

		g_admins[line] = adminLevel;
	}

	g_engfuncs.pfnServerPrint(UTIL_VarArgs("Loaded %d admin(s) from file\n", g_admins.size()));
}

int AdminLevel(CBasePlayer* plr) {
	if (!plr) {
		return ADMIN_OWNER; // probably the server console (called by command callback)
	}

	std::string steamId = (*g_engfuncs.pfnGetPlayerAuthId)(plr->edict());

	if (!IS_DEDICATED_SERVER()) {
		if (plr->entindex() == 1) {
			return ADMIN_OWNER; // listen server owner is always the first player to join (I hope)
		}
	}

	auto adminStatus = g_admins.find(steamId);
	if (adminStatus != g_admins.end()) {
		return adminStatus->second;
	}

	return ADMIN_NO;
}

void winPath(std::string& path) {
	for (int i = 0, size = path.size(); i < size; i++) {
		if (path[i] == '/')
			path[i] = '\\';
	}
}

std::vector<std::string> getDirFiles(std::string path, std::string extension, std::string startswith, bool onlyOne)
{
	std::vector<std::string> results;

#if defined(WIN32) || defined(_WIN32)
	path = path + startswith + "*." + extension;
	winPath(path);
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	//ALERT(at_console, "Target file is " + path);
	hFind = FindFirstFile(path.c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		//ALERT(at_console, "FindFirstFile failed " + str((int)GetLastError()) + " " + path);
		return results;
	}
	else
	{
		results.push_back(FindFileData.cFileName);

		while (FindNextFile(hFind, &FindFileData) != 0)
		{
			results.push_back(FindFileData.cFileName);
			if (onlyOne)
				break;
		}

		FindClose(hFind);
	}
#else
	extension = toLowerCase(extension);
	startswith = toLowerCase(startswith);
	startswith.erase(std::remove(startswith.begin(), startswith.end(), '*'), startswith.end());
	DIR* dir = opendir(path.c_str());

	if (!dir)
		return results;

	while (true)
	{
		dirent* entry = readdir(dir);

		if (!entry)
			break;

		if (entry->d_type == DT_DIR)
			continue;

		std::string name = std::string(entry->d_name);
		std::string lowerName = toLowerCase(name);

		if (extension.size() > name.size() || startswith.size() > name.size())
			continue;

		if (extension == "*" || std::equal(extension.rbegin(), extension.rend(), lowerName.rbegin()))
		{
			if (startswith.size() == 0 || std::equal(startswith.begin(), startswith.end(), lowerName.begin()))
			{
				results.push_back(name);
				if (onlyOne)
					break;
			}
		}
	}

	closedir(dir);
#endif

	return results;
}

void KickPlayer(edict_t* ent, const char* reason) {
	if (!ent || (ent->v.flags & FL_CLIENT) == 0) {
		return;
	}
	int userid = g_engfuncs.pfnGetPlayerUserId(ent);
	g_engfuncs.pfnServerCommand(UTIL_VarArgs("kick #%d %s\n", userid, reason));
	g_engfuncs.pfnServerExecute();
}

// https://stackoverflow.com/questions/1628386/normalise-orientation-between-0-and-360
float normalizeRangef(const float value, const float start, const float end)
{
	const float width = end - start;
	const float offsetValue = value - start;   // value relative to 0

	return (offsetValue - (floorf(offsetValue / width) * width)) + start;
	// + start to reset back to start of original range
}

bool createFolder(const std::string& path) {
	if (mkdir(path.c_str(), 0777) == 0) {
		return true;
	}

	return false;
}

bool folderExists(const std::string& path) {
	struct stat info;

	if (stat(path.c_str(), &info) != 0) {
		return false;
	}

	return (info.st_mode & S_IFDIR) != 0;
}


uint64_t getFreeSpace(const std::string& path) {
#if defined(_WIN32)
	ULARGE_INTEGER freeBytesAvailable;
	if (GetDiskFreeSpaceEx(path.c_str(), &freeBytesAvailable, NULL, NULL)) {
		return freeBytesAvailable.QuadPart;
	}
	else {
		ALERT(at_console, "Error getting free space.\n");
		return 0;
	}
#else
	struct statvfs stat;
	if (statvfs(path.c_str(), &stat) != 0) {
		ALERT(at_console, "Error getting free space.\n");
		return 0;
	}

	return (uint64_t)stat.f_bavail * (uint64_t)stat.f_bsize;
#endif
}
