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
// headcrab.cpp - tiny, jumpy alien parasite
//=========================================================

#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"CBaseMonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		SR_AE_JUMPATTACK	( 2 )

class EXPORT COFShockRoach : public CBaseMonster
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void RunTask ( Task_t *pTask ) override;
	void StartTask ( Task_t *pTask ) override;
	void SetYawSpeed ( void ) override;
	void LeapTouch ( CBaseEntity *pOther );
	Vector Center( void ) override;
	Vector BodyTarget( const Vector &posSrc ) override;
	void PainSound( void ) override;
	void DeathSound( void ) override;
	void IdleSound( void ) override;
	void AlertSound( void ) override;
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;
	void PrescheduleThink( void ) override;
	int  Classify ( void ) override;
	const char* DisplayName() override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override;
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) override;
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) override;
	void RifleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual float GetDamageAmount( void ) { return gSkillData.sk_shockroach_dmg_bite; }
	virtual int GetVoicePitch( void ) { return 100; }
	virtual float GetSoundVolue( void ) { return 1.0; }
	Schedule_t* GetScheduleOfType ( int Type ) override;
	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }

	void MonsterThink() override;

	virtual void Revive() override;

	int Save( CSave &save ) override;
	int Restore( CRestore &restore ) override;
	static TYPEDESCRIPTION m_SaveData[];

	float m_flBirthTime;
	BOOL m_fRoachSolid;

	CUSTOM_SCHEDULES;

	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pAttackSounds[];
	static const char *pDeathSounds[];
	static const char *pBiteSounds[];
};
