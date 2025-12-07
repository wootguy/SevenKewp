#pragma once
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "schedule.h"
#include "game.h"
#include "CBaseMonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		HC_AE_JUMPATTACK	( 2 )

class EXPORT CHeadCrab : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void RunTask ( Task_t *pTask );
	void StartTask ( Task_t *pTask );
	void SetYawSpeed ( void );
	void LeapTouch ( CBaseEntity *pOther );
	Vector Center( void );
	Vector BodyTarget( const Vector &posSrc );
	void PainSound( void );
	void DeathSound( void );
	void IdleSound( void );
	void AlertSound( void );
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();
	void PrescheduleThink( void );
	int  Classify ( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	BOOL CheckRangeAttack2 ( float flDot, float flDist );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	virtual float GetDamageAmount( void ) { return gSkillData.sk_headcrab_dmg_bite; }
	virtual int GetVoicePitch( void ) { return 100; }
	virtual float GetSoundVolue( void ) { return 1.0; }
	Schedule_t* GetScheduleOfType ( int Type );
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	CUSTOM_SCHEDULES;

	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pAttackSounds[];
	static const char *pDeathSounds[];
	static const char *pBiteSounds[];
};

class EXPORT CBabyCrab : public CHeadCrab
{
public:
	void Spawn( void );
	void Precache( void );
	int  Classify(void);
	const char* DisplayName();
	void SetYawSpeed ( void );
	float GetDamageAmount( void ) { return gSkillData.sk_headcrab_dmg_bite * 0.3; }
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	Schedule_t* GetScheduleOfType ( int Type );
	virtual int GetVoicePitch( void ) { return PITCH_NORM + RANDOM_LONG(40,50); }
	virtual float GetSoundVolue( void ) { return 0.8; }
};
