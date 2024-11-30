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

===== weapons.cpp ========================================================

  functions governing the selection/use of weapons for players

*/

#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "env/CSoundEnt.h"
#include "decals.h"
#include "gamerules.h"
#include "CBasePlayerWeapon.h"
#include "explode.h"

extern CGraph	WorldGraph;
extern int gEvilImpulse101;


#define NOT_USED 255

DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
DLL_GLOBAL  const char *g_pModelNameLaser = "sprites/laserbeam.spr";
DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for the initial blood
DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for splattered blood
DLL_GLOBAL	short	g_sModelIndexShrapnelHit;// holds the sprite index for shrapnel impact sprite
DLL_GLOBAL	short	g_sModelIndexShrapnel;// holds the model index for shrapnel gibs

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];
AmmoInfo CBasePlayerItem::AmmoInfoArray[MAX_AMMO_SLOTS];

MULTIDAMAGE gMultiDamage;

#define TRACER_FREQ		4			// Tracers fire every fourth bullet

//=========================================================
// MaxAmmoCarry - pass in a name and this function will tell
// you the maximum amount of that type of ammunition that a 
// player can carry.
//=========================================================
int MaxAmmoCarry( int iszName )
{
	for ( int i = 0;  i < MAX_WEAPONS; i++ )
	{
		if ( CBasePlayerItem::ItemInfoArray[i].pszAmmo1 && !strcmp( STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo1 ) )
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo1;
		if ( CBasePlayerItem::ItemInfoArray[i].pszAmmo2 && !strcmp( STRING(iszName), CBasePlayerItem::ItemInfoArray[i].pszAmmo2 ) )
			return CBasePlayerItem::ItemInfoArray[i].iMaxAmmo2;
	}

	ALERT( at_console, "MaxAmmoCarry() doesn't recognize '%s'!\n", STRING( iszName ) );
	return -1;
}

/*
==============================================================================

MULTI-DAMAGE

Collects multiple small damages into a single damage

==============================================================================
*/

//
// ClearMultiDamage - resets the global multi damage accumulator
//
void ClearMultiDamage(void)
{
	gMultiDamage.pEntity = NULL;
	gMultiDamage.amount	= 0;
	gMultiDamage.type = 0;
}

//
// ApplyMultiDamage - inflicts contents of global multi damage register on gMultiDamage.pEntity
//
// GLOBALS USED:
//		gMultiDamage
void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker )
{
	Vector		vecSpot1;//where blood comes from
	Vector		vecDir;//direction blood should go
	TraceResult	tr;
	
	if ( !gMultiDamage.pEntity )
		return;

	gMultiDamage.pEntity->TakeDamage(pevInflictor, pevAttacker, gMultiDamage.amount, gMultiDamage.type );
}

// GLOBALS USED:
//		gMultiDamage
void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType)
{
	if ( !pEntity )
		return;
	
	gMultiDamage.type |= bitsDamageType;

	if ( pEntity != gMultiDamage.pEntity )
	{
		ApplyMultiDamage(pevInflictor,pevInflictor); // UNDONE: wrong attacker!
		gMultiDamage.pEntity	= pEntity;
		gMultiDamage.amount		= 0;
	}

	gMultiDamage.amount += flDamage;
}

/*
================
SpawnBlood
================
*/
void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage)
{
	UTIL_BloodDrips( vecSpot, g_vecAttackDir, bloodColor, (int)flDamage );
}

int DamageDecal( CBaseEntity *pEntity, int bitsDamageType )
{
	if ( !pEntity )
		return (DECAL_GUNSHOT1 + RANDOM_LONG(0,4));
	
	return pEntity->DamageDecal( bitsDamageType );
}

void DecalGunshot( TraceResult *pTrace, int iBulletType )
{
	// Is the entity valid
	if ( !UTIL_IsValidEntity( pTrace->pHit ) )
		return;

	if ( VARS(pTrace->pHit)->solid == SOLID_BSP || VARS(pTrace->pHit)->movetype == MOVETYPE_PUSHSTEP )
	{
		CBaseEntity *pEntity = NULL;
		// Decal the wall with a gunshot
		if ( !FNullEnt(pTrace->pHit) )
			pEntity = CBaseEntity::Instance(pTrace->pHit);

		switch( iBulletType )
		{
		case BULLET_PLAYER_9MM:
		case BULLET_MONSTER_9MM:
		case BULLET_PLAYER_MP5:
		case BULLET_MONSTER_MP5:
		case BULLET_PLAYER_BUCKSHOT:
		case BULLET_PLAYER_357:
		case BULLET_PLAYER_556:
		default:
			// smoke and decal
			UTIL_GunshotDecalTrace( pTrace, DamageDecal( pEntity, DMG_BULLET ) );
			break;
		case BULLET_MONSTER_12MM:
			// smoke and decal
			UTIL_GunshotDecalTrace( pTrace, DamageDecal( pEntity, DMG_BULLET ) );
			break;
		case BULLET_PLAYER_CROWBAR:
			// wall decal
			UTIL_DecalTrace( pTrace, DamageDecal( pEntity, DMG_CLUB ) );
			break;
		}
	}
}

//
// EjectBrass - tosses a brass shell from passed origin at passed velocity
//
void EjectBrass ( const Vector &vecOrigin, const Vector &vecVelocity, float rotation, int model, int soundtype )
{
	// FIX: when the player shoots, their gun isn't in the same position as it is on the model other players see.

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecOrigin );
		WRITE_BYTE( TE_MODEL);
		WRITE_COORD( vecOrigin.x);
		WRITE_COORD( vecOrigin.y);
		WRITE_COORD( vecOrigin.z);
		WRITE_COORD( vecVelocity.x);
		WRITE_COORD( vecVelocity.y);
		WRITE_COORD( vecVelocity.z);
		WRITE_ANGLE( rotation );
		WRITE_SHORT( model );
		WRITE_BYTE ( soundtype);
		WRITE_BYTE ( 25 );// 2.5 seconds
	MESSAGE_END();
}


#if 0
// UNDONE: This is no longer used?
void ExplodeModel( const Vector &vecOrigin, float speed, int model, int count )
{
	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, vecOrigin );
		WRITE_BYTE ( TE_EXPLODEMODEL );
		WRITE_COORD( vecOrigin.x );
		WRITE_COORD( vecOrigin.y );
		WRITE_COORD( vecOrigin.z );
		WRITE_COORD( speed );
		WRITE_SHORT( model );
		WRITE_SHORT( count );
		WRITE_BYTE ( 15 );// 1.5 seconds
	MESSAGE_END();
}
#endif

CBaseEntity* ShootMortar(edict_t* pentOwner, Vector vecStart, Vector vecVelocity) {
	TraceResult tr;
	UTIL_TraceLine(vecStart, vecStart + Vector(0, 0, -4096), ignore_monsters, pentOwner, &tr);

	CBaseEntity* pMortar = CBaseEntity::Create("monster_mortar", tr.vecEndPos, Vector(0, 0, 0), pentOwner);

	pMortar->pev->nextthink = gpGlobals->time;

	return pMortar;
}

void GetCircularGaussianSpread(float& x, float& y) {
	float z;
	do {
		x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		z = x * x + y * y;
	} while (z > 1);
}

int giAmmoIndex = 0;

// Precaches the ammo and queues the ammo info for sending to clients
void AddAmmoNameToAmmoRegistry( const char *szAmmoname )
{
	// make sure it's not already in the registry
	for ( int i = 0; i < MAX_AMMO_SLOTS; i++ )
	{
		if ( !CBasePlayerItem::AmmoInfoArray[i].pszName)
			continue;

		if ( stricmp( CBasePlayerItem::AmmoInfoArray[i].pszName, szAmmoname ) == 0 )
			return; // ammo already in registry, just quite
	}


	giAmmoIndex++;
	ASSERT( giAmmoIndex < MAX_AMMO_SLOTS );
	if ( giAmmoIndex >= MAX_AMMO_SLOTS )
		giAmmoIndex = 0;

	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].pszName = szAmmoname;
	CBasePlayerItem::AmmoInfoArray[giAmmoIndex].iId = giAmmoIndex;   // yes, this info is redundant
}


bool g_registeringCustomWeps = false;
std::unordered_set<std::string> g_weaponClassnames;

const char* g_filledWeaponSlots[MAX_WEAPON_SLOTS][MAX_WEAPON_POSITIONS];

// Queues the weapon info for sending to clients
ItemInfo UTIL_RegisterWeapon( const char *szClassname )
{
	edict_t	*pent;
	ItemInfo info;
	memset(&info, 0, sizeof(ItemInfo));
	info.iId = -1;

	if (g_weaponClassnames.size() >= MAX_WEAPONS) {
		ALERT(at_error, "Failed to register weapon %s. Too many weapons! (%d)\n",
			szClassname, MAX_WEAPONS);
		return info;
	}

	pent = CREATE_NAMED_ENTITY( MAKE_STRING( szClassname ) );
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_error, UTIL_VarArgs("Failed to register weapon '%s' (entity does not exist)\n", szClassname) );
		return info;
	}

	CBaseEntity *pEntity = CBaseEntity::Instance (VARS( pent ));
	CBasePlayerWeapon* wep = pEntity ? pEntity->GetWeaponPtr() : NULL;
	ItemInfo II;
	memset(&II, 0, sizeof II);

	if (!wep) {
		ALERT(at_error, "Failed to register weapon '%s' (entity is not a weapon)\n", szClassname);
		goto cleanup;
	}

	if (!wep->GetItemInfo(&II)) {
		ALERT(at_error, "Failed to register weapon '%s' (GetItemInfo() returned FALSE)\n", szClassname);
		goto cleanup;
	}

	memcpy(&info, &II, sizeof(ItemInfo));

	if (!info.pszName) {
		ALERT(at_error, "Failed to register weapon '%s' (pszName not set)\n", szClassname);
		goto cleanup;
	}

	if (info.iId < 0 || info.iId >= MAX_WEAPONS) {
		info.iId = g_weaponClassnames.size() + 1;
	}

	if (info.iSlot < 0 || info.iSlot >= MAX_WEAPON_SLOTS) {
		ALERT(at_error, "Failed to register weapon '%s' (invalid slot %d. Max is %d)\n",
			szClassname, info.iSlot, MAX_WEAPON_SLOTS - 1);
		goto cleanup;
	}

	if (info.iPosition < 0) {
		for (int i = 0; i < MAX_WEAPON_POSITIONS; i++) {
			if (!g_filledWeaponSlots[info.iSlot][i]) {
				info.iPosition = i;
				break;
			}
		}

		if (info.iPosition < 0) {
			ALERT(at_error, "Failed to register weapon '%s' (slot %d has too many weapons)\n",
				szClassname, info.iSlot);
			goto cleanup;
		}
	} else if (info.iPosition < 0 || info.iPosition >= MAX_WEAPON_POSITIONS) {
		ALERT(at_error, "Failed to register weapon '%s' (invalid position %d. Max is %d)\n",
			szClassname, info.iPosition, MAX_WEAPON_POSITIONS - 1);
		goto cleanup;
	}

	if (g_filledWeaponSlots[info.iSlot][info.iPosition]) {
		ALERT(at_error, "Failed to register weapon '%s' (slot %d position %d is filled by '%s')\n",
			szClassname, info.iSlot, info.iPosition, g_filledWeaponSlots[info.iSlot][info.iPosition]);
		goto cleanup;
	}

	g_filledWeaponSlots[info.iSlot][info.iPosition] = szClassname;

	CBasePlayerItem::ItemInfoArray[info.iId] = info;

	if (info.pszAmmo1 && *info.pszAmmo1) {
		AddAmmoNameToAmmoRegistry(info.pszAmmo1);
	}

	if (info.pszAmmo2 && *info.pszAmmo2) {
		AddAmmoNameToAmmoRegistry(info.pszAmmo2);
	}

	g_weaponClassnames.insert(info.pszName);

	// events must always be precached, and in the correct order, or else
	// vanilla clients will play the wrong weapon events
	wep->PrecacheEvents();

	if (g_registeringCustomWeps) {
		PRECACHE_HUD_FILES(("sprites/" + std::string(info.pszName) + ".txt").c_str());

		ALERT(at_console, "Registered custom weapon '%s' (ID %d) to slot %d position %d\n",
			szClassname, info.iId, info.iSlot, info.iPosition);
	}

cleanup:
	REMOVE_ENTITY(pent);
	return info;
}

// called by worldspawn
void W_Precache(void)
{
	memset( CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray) );
	memset( CBasePlayerItem::AmmoInfoArray, 0, sizeof(CBasePlayerItem::AmmoInfoArray) );
	giAmmoIndex = 0;

	// custom items...

	// common world objects
	/*
	UTIL_PrecacheOther( "item_suit" );
	UTIL_PrecacheOther( "item_battery" );
	UTIL_PrecacheOther( "item_antidote" );
	UTIL_PrecacheOther( "item_security" );
	UTIL_PrecacheOther( "item_longjump" );
	*/

	for (int i = 0; i < MAX_WEAPON_SLOTS; i++) {
		memset(g_filledWeaponSlots[i], 0, MAX_WEAPON_POSITIONS * sizeof(const char*));
	}

	g_registeringCustomWeps = false;
	UTIL_RegisterWeapon("weapon_shotgun");
	UTIL_RegisterWeapon("weapon_crowbar");
	UTIL_RegisterWeapon("weapon_9mmhandgun");
	UTIL_RegisterWeapon("weapon_9mmAR");
	UTIL_RegisterWeapon("weapon_357");
	UTIL_RegisterWeapon("weapon_gauss");
	UTIL_RegisterWeapon("weapon_rpg");
	UTIL_RegisterWeapon("weapon_crossbow");
	UTIL_RegisterWeapon("weapon_egon");
	UTIL_RegisterWeapon("weapon_tripmine");
	UTIL_RegisterWeapon("weapon_satchel");
	UTIL_RegisterWeapon("weapon_handgrenade");
	UTIL_RegisterWeapon("weapon_snark");
	UTIL_RegisterWeapon("weapon_hornetgun");
	UTIL_RegisterWeapon("weapon_grapple");
	UTIL_RegisterWeapon("weapon_displacer");
	UTIL_RegisterWeapon("weapon_pipewrench");
	UTIL_RegisterWeapon("weapon_shockrifle");
	UTIL_RegisterWeapon("weapon_sporelauncher");
	UTIL_RegisterWeapon("weapon_medkit");
	UTIL_RegisterWeapon("weapon_inventory");
	g_registeringCustomWeps = true; // anything registered from this point on must be from a plugin

	g_sModelIndexFireball = PRECACHE_MODEL ("sprites/zerogxplode.spr");// fireball
	g_sModelIndexWExplosion = PRECACHE_MODEL ("sprites/WXplo1.spr");// underwater fireball
	g_sModelIndexSmoke = PRECACHE_MODEL ("sprites/steam1.spr");// smoke
	g_sModelIndexBubbles = PRECACHE_MODEL ("sprites/bubble.spr");//bubbles
	g_sModelIndexBloodSpray = PRECACHE_MODEL ("sprites/bloodspray.spr"); // initial blood
	g_sModelIndexBloodDrop = PRECACHE_MODEL ("sprites/blood.spr"); // splattered blood 

	g_sModelIndexLaser = PRECACHE_MODEL( (char *)g_pModelNameLaser );
	g_sModelIndexLaserDot = PRECACHE_MODEL("sprites/laserdot.spr");

	// shrapnel effect ("blood" for machines)
	g_sModelIndexShrapnelHit = PRECACHE_MODEL("sprites/shraphit.spr");
	g_sModelIndexShrapnel = PRECACHE_MODEL("models/bigshrapnel.mdl");

	// used by explosions
	PRECACHE_MODEL ("models/grenade.mdl");
	PRECACHE_MODEL ("models/w_grenade.mdl");
	PRECACHE_MODEL ("sprites/explode1.spr");

	PRECACHE_SOUND_ENT(NULL, "weapons/debris1.wav");// explosion aftermaths
	PRECACHE_SOUND_ENT(NULL, "weapons/debris2.wav");// explosion aftermaths
	PRECACHE_SOUND_ENT(NULL, "weapons/debris3.wav");// explosion aftermaths

	PRECACHE_SOUND_ENT(NULL, "weapons/grenade_hit1.wav");//grenade
	PRECACHE_SOUND_ENT(NULL, "weapons/grenade_hit2.wav");//grenade
	PRECACHE_SOUND_ENT(NULL, "weapons/grenade_hit3.wav");//grenade

	PRECACHE_SOUND_ENT(NULL, "weapons/bullet_hit1.wav");	// hit by bullet
	PRECACHE_SOUND_ENT(NULL, "weapons/bullet_hit2.wav");	// hit by bullet
	
	PRECACHE_SOUND_ENT(NULL, "items/weapondrop1.wav");// weapon falls to the ground

}

void FindHullIntersection(const Vector& vecSrc, TraceResult& tr, float* mins, float* maxs, edict_t* pEntity)
{
	int			i, j, k;
	float		distance;
	float* minmaxs[2] = { mins, maxs };
	TraceResult tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &tmpTrace);
	if (tmpTrace.flFraction < 1.0)
	{
		tr = tmpTrace;
		return;
	}

	for (i = 0; i < 2; i++)
	{
		for (j = 0; j < 2; j++)
		{
			for (k = 0; k < 2; k++)
			{
				vecEnd.x = vecHullEnd.x + minmaxs[i][0];
				vecEnd.y = vecHullEnd.y + minmaxs[j][1];
				vecEnd.z = vecHullEnd.z + minmaxs[k][2];

				UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &tmpTrace);
				if (tmpTrace.flFraction < 1.0)
				{
					float thisDistance = (tmpTrace.vecEndPos - vecSrc).Length();
					if (thisDistance < distance)
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}
