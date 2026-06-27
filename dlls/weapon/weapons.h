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

#include "Platform.h"
#include "ammo.h"
#include "bullet.h"
#include "shared_util.h"

#ifdef CLIENT_DLL
#include "com_weapons.h"
#include "../dlls/util/eng_wrappers.h"
void UTIL_TraceLine(const Vector& vecStart, const Vector& vecEnd, IGNORE_MONSTERS igmon, edict_t* pentIgnore, TraceResult* ptr);
#define LINK_ENTITY_TO_CLASS(...)
#define MAKE_STRING(...) 0
#define UTIL_PrecacheOther(...)
#define UTIL_SetSize(...)
#define UTIL_MakeVectors(...)
#define UTIL_Remove(...)
#define UTIL_SetOrigin(...)
#define lagcomp_begin(...)
#define lagcomp_end(...)
#define PLAY_DISTANT_SOUND(...)
#define EMIT_SOUND(...)
#define EMIT_SOUND_DYN(...)
#define AddWaterPhysicsEnt(...)
#else
#include "util.h"
#include "skill.h"
#include "gamerules.h"
#include "game.h"
#include "eng_wrappers.h"
#include "lagcomp.h"
#include "CGrenade.h"
#endif

#define PLAYBACK_EVENT( flags, who, index ) PLAYBACK_EVENT_FULL( flags, who, index, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );
#define PLAYBACK_EVENT_DELAY( flags, who, index, delay ) PLAYBACK_EVENT_FULL( flags, who, index, delay, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );

class CBaseEntity;
class CBasePlayer;
EXPORT extern int gmsgWeapPickup;

typedef struct
{
	int		iSlot;
	int		iPosition;
	const char* pszAmmo1;	// ammo 1 type
	const char* pszAmmo2;	// ammo 2 type
	const char* pszName;
	int		iMaxClip;
	int		iMaxClip2;
	int		iId;
	int		iFlags;
	int		iWeight;// this value used to determine this weapon's importance in autoselection.
	int		iFlagsEx; // WEP_FLAG_*

	// Dynamic crosshair values for vanilla and server-side weapons only. CWeaponCustom has its own system.
	float	fAccuracyDeg; // degrees of accuracy when standing motionless
	float	fAccuracyDeg2; // degrees of accuracy when standing motionless (secondary)
	float	fAccuracyDegY; // vertical degrees of accuracy
	float	fAccuracyDegY2; // vertical degrees of accuracy (secondary)
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

// leaving a gap here for custom weapons that vanilla HL players can pick up (HL max weapons = 32)

#define WEAPON_SUIT				31	// ?????

#define WEAPON_ALLWEAPONS		(~(1ULL<<WEAPON_SUIT))

#define MAX_WEAPONS			64
#define MAX_WEAPON_SLOTS	6
#define MAX_WEAPON_POSITIONS 8


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

#define WEAPON_IS_ONTARGET 0x40

enum merged_item_bodies {
	MERGE_MDL_CAMERA,
	MERGE_MDL_GRENADE,
	MERGE_MDL_HVR,
	MERGE_MDL_RPGROCKET,
	MERGE_MDL_SHOCK_EFFECT,
	MERGE_MDL_SPORE,
	MERGE_MDL_W_2UZIS,
	MERGE_MDL_W_357,
	MERGE_MDL_W_357AMMOBOX,
	MERGE_MDL_W_9MMAR,
	MERGE_MDL_W_9MMARCLIP,
	MERGE_MDL_W_9MMCLIP,
	MERGE_MDL_W_9MMHANDGUN,
	MERGE_MDL_W_ARGRENADE,
	MERGE_MDL_W_BATTERY,
	MERGE_MDL_W_BGRAP,
	MERGE_MDL_W_CHAINAMMO,
	MERGE_MDL_W_CROSSBOW,
	MERGE_MDL_W_CROSSBOW_CLIP,
	MERGE_MDL_W_CROWBAR,
	MERGE_MDL_W_DESERT_EAGLE,
	MERGE_MDL_W_DISPLACER,
	MERGE_MDL_W_EGON,
	MERGE_MDL_W_GAUSS,
	MERGE_MDL_W_GAUSSAMMO,
	MERGE_MDL_W_GRENADE,
	MERGE_MDL_W_HGUN,
	MERGE_MDL_W_KNIFE,
	MERGE_MDL_W_LONGJUMP,
	MERGE_MDL_W_M16,
	MERGE_MDL_W_M40A1,
	MERGE_MDL_W_M40A1_CLIP,
	MERGE_MDL_W_MEDKIT,
	MERGE_MDL_W_MINIGUN,
	MERGE_MDL_W_PIPE_WRENCH,
	MERGE_MDL_W_PMEDKIT,
	MERGE_MDL_W_RPG,
	MERGE_MDL_W_RPGAMMO,
	MERGE_MDL_W_SATCHEL,
	MERGE_MDL_W_SAW,
	MERGE_MDL_W_SAW_CLIP,
	MERGE_MDL_W_SECURITY,
	MERGE_MDL_W_SHOTBOX,
	MERGE_MDL_W_SHOTGUN,
	MERGE_MDL_W_SHOTSHELL,
	MERGE_MDL_W_SUIT,
	MERGE_MDL_W_UZI,
	MERGE_MDL_W_UZI_CLIP,
	MERGE_MDL_W_WEAPONBOX,
};

EXPORT extern DLL_GLOBAL	short	g_sModelIndexLaser;// holds the index for the laser beam
EXPORT extern DLL_GLOBAL	const char *g_pModelNameLaser;
EXPORT extern DLL_GLOBAL	short g_notPrecachedModelIdx;

EXPORT extern DLL_GLOBAL	short	g_sModelIndexLaserDot;// holds the index for the laser beam dot
EXPORT extern DLL_GLOBAL	short	g_sModelIndexFireball;// holds the index for the fireball
EXPORT extern DLL_GLOBAL	short	g_sModelIndexSmoke;// holds the index for the smoke cloud
EXPORT extern DLL_GLOBAL	short	g_sModelIndexWExplosion;// holds the index for the underwater explosion
EXPORT extern DLL_GLOBAL	short	g_sModelIndexBubbles;// holds the index for the bubbles model
EXPORT extern DLL_GLOBAL	short	g_sModelIndexBloodDrop;// holds the sprite index for blood drops
EXPORT extern DLL_GLOBAL	short	g_sModelIndexBloodSpray;// holds the sprite index for blood spray (bigger)
EXPORT extern DLL_GLOBAL	short	g_sModelIndexShrapnelHit;// holds the sprite index for shrapnel impact sprite
EXPORT extern DLL_GLOBAL	short	g_sModelIndexShrapnel;// holds the model index for shrapnel gibs
EXPORT extern DLL_GLOBAL	short	g_waterSplashSpr; // holds the model index for rain splashes
EXPORT extern DLL_GLOBAL	short	g_waterSplash2Spr; // holds the model index for rain splashes
EXPORT extern DLL_GLOBAL	short	g_waterSplashWakeSpr; // holds the model index for rain splashes
EXPORT extern DLL_GLOBAL	short	g_waterSplashWake2Spr; // holds the model index for rain splashes

extern const char* g_waterSplashSounds[3];
#define WATER_SPLASH2_SND_PATH "water/waterblow.wav"

extern StringMap g_defaultSpriteDirs;
extern StringMap g_customWeaponConfigs; // maps a classname to its config file
extern StringSet g_customWeaponConfigsAlt; // linked configs
extern StringMap g_customAmmoConfigs; // maps a classname to its config file
extern StringSet g_registeredHlWeaponAmmo; // ammo types that are used by currently registered weapons that HL players can pick up
extern HashMap<int> g_ammoCapacities;
extern HashMap<int> g_ammoCapacitiesInitial; // first ammo capacity that was set
extern const char* g_filledWeaponSlots[MAX_WEAPON_SLOTS][MAX_WEAPON_POSITIONS];

EXPORT void ClearMultiDamage(void);
EXPORT void ApplyMultiDamage(entvars_t* pevInflictor, entvars_t* pevAttacker );
EXPORT void AddMultiDamage( entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType);
EXPORT void RadiusDamage( Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType );

EXPORT CBaseEntity* ShootMortar(edict_t* pentOwner, Vector vecStart, Vector vecVelocity);
EXPORT void GetCircularGaussianSpread(float& x, float& y);

// uses GetItemInfo to extract registration info.
// Set iId, iPosition, iMaxAmmo1, or iMaxAmmo2 to -1 to have them be automatically assigned.
// That should be done to prevent conflicts between the game and unrelated plugins.
// Returns an ItemInfo with reassigned id and position.
// szClassname = entity to register. Can be NULL if using a config file.
// configPath = if set, map a classname to CWeaponCustom and load settings from the config
// sevenkewpOnly = auto-assign an ID in the sevenkewp-only range to free up slots in the vanilla HL ID range
EXPORT ItemInfo UTIL_RegisterWeapon(const char* szClassname, const char* configPath=NULL);

EXPORT void UTIL_RegisterAmmo(const char* configPath);

// call this for custom ammo types so custom weapons know what their capacity is.
EXPORT void UTIL_RegisterAmmoCapacity(const char* ammoType, int capacity);

// register an entity for precaching and equipment via map CFG
// Use this with ammo entities and weapons that couldn't be registered normally
EXPORT void UTIL_RegisterEquipmentEntity(const char* szClassname);

// add another classname that a custom weapon to be spawned with (e.g. weapon_glock = weapon_9mmhandgun)
EXPORT void UTIL_RegisterWeaponCustomAlias(const char* classname, const char* alias);

// add another classname that a custom weapon to be spawned with (e.g. weapon_glock = weapon_9mmhandgun)
EXPORT void UTIL_RegisterAmmoCustomAlias(const char* classname, const char* alias);

// set a default sprite dir for a stock weapon
EXPORT void UTIL_SetDefaultWeaponSpriteDir(const char* szClassname, const char* spriteDir);

EXPORT void AddAmmoNameToAmmoRegistry(const char* szAmmoname, bool isSevenKewpGun);

// get current max ammo capacity
EXPORT int UTIL_GetMaxAmmo(const char* ammoName);

// get first capacity value that was set this map
EXPORT int UTIL_GetMaxAmmoInitial(const char* ammoName);

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
