/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/

/*

	h_tentacle.cpp - silo of death tentacle monster (half life)

*/

#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseMonster.h"

#define ACT_T_IDLE		1010
#define ACT_T_TAP			1020
#define ACT_T_STRIKE		1030
#define ACT_T_REARIDLE	1040

// stike sounds
#define TE_NONE -1
#define TE_SILO 0
#define TE_DIRT 1
#define TE_WATER 2


// animation sequence aliases 
typedef enum
{
	TENTACLE_ANIM_Pit_Idle,

	TENTACLE_ANIM_rise_to_Temp1,
	TENTACLE_ANIM_Temp1_to_Floor,
	TENTACLE_ANIM_Floor_Idle,
	TENTACLE_ANIM_Floor_Fidget_Pissed,
	TENTACLE_ANIM_Floor_Fidget_SmallRise,
	TENTACLE_ANIM_Floor_Fidget_Wave,
	TENTACLE_ANIM_Floor_Strike,
	TENTACLE_ANIM_Floor_Tap,
	TENTACLE_ANIM_Floor_Rotate,
	TENTACLE_ANIM_Floor_Rear,
	TENTACLE_ANIM_Floor_Rear_Idle,
	TENTACLE_ANIM_Floor_to_Lev1,

	TENTACLE_ANIM_Lev1_Idle,
	TENTACLE_ANIM_Lev1_Fidget_Claw,
	TENTACLE_ANIM_Lev1_Fidget_Shake,
	TENTACLE_ANIM_Lev1_Fidget_Snap,
	TENTACLE_ANIM_Lev1_Strike,
	TENTACLE_ANIM_Lev1_Tap,
	TENTACLE_ANIM_Lev1_Rotate,
	TENTACLE_ANIM_Lev1_Rear,
	TENTACLE_ANIM_Lev1_Rear_Idle,
	TENTACLE_ANIM_Lev1_to_Lev2,

	TENTACLE_ANIM_Lev2_Idle,
	TENTACLE_ANIM_Lev2_Fidget_Shake,
	TENTACLE_ANIM_Lev2_Fidget_Swing,
	TENTACLE_ANIM_Lev2_Fidget_Tut,
	TENTACLE_ANIM_Lev2_Strike,
	TENTACLE_ANIM_Lev2_Tap,
	TENTACLE_ANIM_Lev2_Rotate,
	TENTACLE_ANIM_Lev2_Rear,
	TENTACLE_ANIM_Lev2_Rear_Idle,
	TENTACLE_ANIM_Lev2_to_Lev3,

	TENTACLE_ANIM_Lev3_Idle,
	TENTACLE_ANIM_Lev3_Fidget_Shake,
	TENTACLE_ANIM_Lev3_Fidget_Side,
	TENTACLE_ANIM_Lev3_Fidget_Swipe,
	TENTACLE_ANIM_Lev3_Strike,
	TENTACLE_ANIM_Lev3_Tap,
	TENTACLE_ANIM_Lev3_Rotate,
	TENTACLE_ANIM_Lev3_Rear,
	TENTACLE_ANIM_Lev3_Rear_Idle,

	TENTACLE_ANIM_Lev1_Door_reach,

	TENTACLE_ANIM_Lev3_to_Engine,
	TENTACLE_ANIM_Engine_Idle,
	TENTACLE_ANIM_Engine_Sway,
	TENTACLE_ANIM_Engine_Swat,
	TENTACLE_ANIM_Engine_Bob,
	TENTACLE_ANIM_Engine_Death1,
	TENTACLE_ANIM_Engine_Death2,
	TENTACLE_ANIM_Engine_Death3,

	TENTACLE_ANIM_none
} TENTACLE_ANIM;

class EXPORT CTentacle : public CBaseMonster
{
public:
	CTentacle( void );

	void Spawn( );
	void Precache( );
	void KeyValue( KeyValueData *pkvd );

	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	// Don't allow the tentacle to go across transitions!!!
	virtual int	ObjectCaps( void ) { return CBaseMonster :: ObjectCaps() & ~(FCAP_IMPULSE_USE | FCAP_ACROSS_TRANSITION); }

	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector(-400, -400, 0);
		pev->absmax = pev->origin + Vector(400, 400, 850);
	}

	void Cycle( void );
	void CommandUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void Start( void );
	void DieThink( void );

	void Test( void );

	void HitTouch( CBaseEntity *pOther );

	float HearingSensitivity( void ) { return 2.0; };

	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void Killed( entvars_t *pevAttacker, int iGib );

	MONSTERSTATE GetIdealState ( void ) { return MONSTERSTATE_IDLE; };
	int CanPlaySequence( BOOL fDisregardState ) { return TRUE; };

	int Classify( void );
	const char* DisplayName();
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	int Level( float dz );
	int MyLevel( void );
	float MyHeight( void );

	float m_flInitialYaw;
	int m_iGoalAnim;
	int m_iLevel;
	int m_iDir;
	float m_flFramerateAdj;
	float m_flSoundYaw;
	int m_iSoundLevel;
	float m_flSoundTime;
	float m_flSoundRadius;
	int m_iHitDmg;
	float m_flHitTime;

	float m_flTapRadius;

	float m_flNextSong;
	static int g_fFlySound;
	static int g_fSquirmSound;

	float m_flMaxYaw;
	int m_iTapSound;

	Vector m_vecPrevSound;
	float m_flPrevSoundTime;
	float m_spawnHealth;

	static const char *pHitSilo[];
	static const char *pHitDirt[];
	static const char *pHitWater[];
};

class CTentacleMaw : public CBaseMonster
{
public:
	void Spawn( );
	void Precache( );
};
