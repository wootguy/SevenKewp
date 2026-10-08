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
//=========================================================
// pit drone - medium sized, fires sharp teeth like spikes and swipes with sharp appendages
//=========================================================

#pragma once
#include "extdll.h"
#include "CBaseMonster.h"

#define		SQUID_SPRINT_DIST	256 // how close the squid has to get before starting to sprint and refusing to swerve

int			   iSpikeTrail;
int iPitdroneSpitSprite;
#define PITDRONE_CLIP_SIZE 6
	

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_PITDRONE_HURTHOP = LAST_COMMON_SCHEDULE + 1,
	SCHED_PITDRONE_SMELLFOOD,
	SCHED_PITDRONE_EAT,
	SCHED_PITDRONE_SNIFF_AND_EAT,
	SCHED_PITDRONE_WALLOW,
	SCHED_PITDRONE_COVER_AND_RELOAD,
	SCHED_PITDRONE_WAIT_FACE_ENEMY,
	SCHED_PITDRONE_TAKECOVER_FAILED,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_PITDRONE_HOPTURN = LAST_COMMON_TASK + 1,
};

//=========================================================
// Bullsquid's spit projectile
//=========================================================
class EXPORT CPitdroneSpike : public CBaseEntity
{
public:
	void Precache() override;
	void Spawn( void ) override;

	static void Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, Vector vecAngles );
	void SpikeTouch( CBaseEntity *pOther );

	void StartTrail();
	void FlyThink();

	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }

	virtual int		Save( CSave &save ) override;
	virtual int		Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];

	int  m_maxFrame;
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		PITDRONE_AE_SPIT		( 1 )
#define		PITDRONE_AE_BITE		( 2 )
#define		PITDRONE_AE_TAILWHIP	( 4 )
#define		PITDRONE_AE_HOP		( 5 )
#define		PITDRONE_AE_THROW		( 6 )
#define PITDRONE_AE_RELOAD	7

namespace PitdroneBodygroup
{
enum PitdroneBodygroup
{
	Weapons = 1
};
}

namespace PitdroneWeapon
{
enum PitdroneWeapon
{
	Empty = 0,
	Full,
	Two = 6,
	One = 7
};
}

class EXPORT CPitdrone : public CBaseMonster
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void SetYawSpeed( void ) override;
	int  ISoundMask( void ) override;
	int  Classify ( void ) override;
	const char* DisplayName() override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	void IdleSound( void ) override;
	void PainSound( void ) override;
	void AlertSound ( void ) override;
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;
	void StartTask ( Task_t *pTask ) override;
	void RunTask ( Task_t *pTask ) override;
	BOOL CheckMeleeAttack1 ( float flDot, float flDist ) override;
	BOOL CheckMeleeAttack2 ( float flDot, float flDist ) override;
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override;
	void RunAI( void ) override;
	BOOL FValidateHintType ( short sHint ) override;
	Schedule_t *GetSchedule( void ) override;
	Schedule_t *GetScheduleOfType ( int Type ) override;
	const char* GetTaskName(int taskIdx) override;
	int IRelationship ( CBaseEntity *pTarget ) override;
	int IgnoreConditions ( void ) override;
	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }

	void CheckAmmo() override;
	void Killed(entvars_t* pevAttacker, int iGib) override;
	void GibMonster() override;
	void KeyValue( KeyValueData* pkvd ) override;

	int	Save( CSave &save ) override;
	int Restore( CRestore &restore ) override;

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	float m_flLastHurtTime;// we keep track of this, because if something hurts a squid, it will forget about its love of headcrabs for a while.
	float m_flNextSpikeTime;// last time the pit drone used the spike attack.
	int m_iInitialAmmo;
	float m_flNextEatTime;
	bool m_hasAmmoGroup;
	bool m_hasSpitEvent;
	bool m_didSpit;

private:
	static const char* pAlertSounds[];
	//static const char* pTalkSounds[];
	static const char* pDieSounds[];
	//static const char* pHuntSounds[];
	static const char* pIdleSounds[];
	static const char* pPainSounds[];
	static const char* pBiteSounds[];
};