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
#ifndef WEAPONS_H
#define WEAPONS_H

#include "ammo.h"

class CBasePlayer;
EXPORT extern int gmsgWeapPickup;

typedef struct
{
	int		iSlot;
	int		iPosition;
	const char* pszAmmo1;	// ammo 1 type
	int		iMaxAmmo1;		// max ammo 1
	const char* pszAmmo2;	// ammo 2 type
	int		iMaxAmmo2;		// max ammo 2
	const char* pszName;
	int		iMaxClip;
	int		iId;
	int		iFlags;
	int		iWeight;// this value used to determine this weapon's importance in autoselection.
} ItemInfo;

typedef struct
{
	const char* pszName;
	int iId;
} AmmoInfo;

EXPORT int MaxAmmoCarry(int iszName);
EXPORT void FindHullIntersection(const Vector& vecSrc, TraceResult& tr, float* mins, float* maxs, edict_t* pEntity);

#define WEAPON_NONE				0
#define WEAPON_CROWBAR			1
#define	WEAPON_GLOCK			2
#define WEAPON_PYTHON			3
#define WEAPON_MP5				4
#define WEAPON_GRAPPLE			5
#define WEAPON_CROSSBOW			6
#define WEAPON_SHOTGUN			7
#define WEAPON_RPG				8
#define WEAPON_GAUSS			9
#define WEAPON_EGON				10
#define WEAPON_HORNETGUN		11
#define WEAPON_HANDGRENADE		12
#define WEAPON_TRIPMINE			13
#define	WEAPON_SATCHEL			14
#define	WEAPON_SNARK			15
#define	WEAPON_DISPLACER		16
#define	WEAPON_PIPEWRENCH		17
#define	WEAPON_SHOCKRIFLE		18
#define	WEAPON_SPORELAUNCHER	19
#define	WEAPON_INVENTORY		20
#define	WEAPON_MEDKIT			21
#define	WEAPON_KNIFE			22

#define WEAPON_ALLWEAPONS		(~(1<<WEAPON_SUIT))

#define WEAPON_SUIT				31	// ?????

#define MAX_WEAPONS			32
#define MAX_WEAPON_SLOTS	5
#define MAX_WEAPON_POSITIONS 5


// weapon weight factors (for auto-switching)   (-1 = noswitch)
#define CROWBAR_WEIGHT			0
#define PIPEWRENCH_WEIGHT		0
#define GRAPPLE_WEIGHT			0
#define INVENTORY_WEIGHT		0
#define MEDKIT_WEIGHT			0
#define KNIFE_WEIGHT			0
#define GLOCK_WEIGHT			10
#define PYTHON_WEIGHT			15
#define MP5_WEIGHT				15
#define SHOTGUN_WEIGHT			15
#define SHOCKRIFLE_WEIGHT		15
#define CROSSBOW_WEIGHT			10
#define RPG_WEIGHT				20
#define GAUSS_WEIGHT			20
#define EGON_WEIGHT				20
#define SPORELAUNCHER_WEIGHT	20
#define HORNETGUN_WEIGHT		15
#define DISPLACER_WEIGHT		10
#define HANDGRENADE_WEIGHT		5
#define SNARK_WEIGHT			5
#define SATCHEL_WEIGHT			-10
#define TRIPMINE_WEIGHT			-10

// bullet types
typedef	enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM, // glock
	BULLET_PLAYER_MP5, // mp5
	BULLET_PLAYER_357, // python
	BULLET_PLAYER_BUCKSHOT, // shotgun
	BULLET_PLAYER_CROWBAR, // crowbar swipe

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,
	BULLET_MONSTER_762,
	BULLET_PLAYER_556
} Bullet;

#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16 // A player can totally exhaust their ammo supply and lose this weapon
#define ITEM_FLAG_NOAUTOSWITCHTO	32

#define WEAPON_IS_ONTARGET 0x40

EXPORT extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
EXPORT extern DLL_GLOBAL	const char *g_pModelNameLaser;

EXPORT extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
EXPORT extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
EXPORT extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
EXPORT extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
EXPORT extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
EXPORT extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
EXPORT extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)
EXPORT extern DLL_GLOBAL	short	g_sModelIndexShrapnelHit;// holds the sprite index for shrapnel impact sprite
EXPORT extern DLL_GLOBAL	short	g_sModelIndexShrapnel;// holds the model index for shrapnel gibs

EXPORT void ClearMultiDamage(void);
EXPORT void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
EXPORT void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);
EXPORT void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

EXPORT CBaseEntity* ShootMortar(edict_t* pentOwner, Vector vecStart, Vector vecVelocity);
EXPORT void GetCircularGaussianSpread(float& x, float& y);

// uses GetItemInfo to extract registration info.
// Set iId and iPosition to -1 to have them be automatically assigned.
// That should be done to prevent conflicts between the game and unrelated plugins.
// Returns an ItemInfo with reassigned id and position.
EXPORT ItemInfo UTIL_RegisterWeapon(const char* szClassname);

EXPORT void AddAmmoNameToAmmoRegistry(const char* szAmmoname);

typedef struct 
{
	CBaseEntity		*pEntity;
	float			amount;
	int				type;
} MULTIDAMAGE;

EXPORT extern MULTIDAMAGE gMultiDamage;

#define LOUD_GUN_VOLUME			1000
#define NORMAL_GUN_VOLUME		600
#define QUIET_GUN_VOLUME		200

#define	BRIGHT_GUN_FLASH		512
#define NORMAL_GUN_FLASH		256
#define	DIM_GUN_FLASH			128

#define BIG_EXPLOSION_VOLUME	2048
#define NORMAL_EXPLOSION_VOLUME	1024
#define SMALL_EXPLOSION_VOLUME	512

#define	WEAPON_ACTIVITY_VOLUME	64

#define VECTOR_CONE_1DEGREES	Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES	Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES	Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES	Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES	Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES	Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES	Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES	Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES	Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES	Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES	Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES	Vector( 0.17365, 0.17365, 0.17365 )

#ifdef CLIENT_DLL
bool bIsMultiplayer ( void );
void LoadVModel ( const char *szViewModel, CBasePlayer *m_pPlayer );
#endif

#endif // WEAPONS_H
