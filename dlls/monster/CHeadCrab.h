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

	virtual float GetDamageAmount( void ) { return gSkillData.sk_headcrab_dmg_bite; }
	virtual int GetVoicePitch( void ) { return 100; }
	virtual float GetSoundVolue( void ) { return 1.0; }
	Schedule_t* GetScheduleOfType ( int Type ) override;
	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }

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
	void Spawn( void ) override;
	void Precache( void ) override;
	int  Classify(void) override;
	const char* DisplayName() override;
	void SetYawSpeed ( void ) override;
	float GetDamageAmount( void ) override { return gSkillData.sk_headcrab_dmg_bite * 0.3; }
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override;
	Schedule_t* GetScheduleOfType ( int Type ) override;
	virtual int GetVoicePitch( void ) override { return PITCH_NORM + RANDOM_LONG(40,50); }
	virtual float GetSoundVolue( void ) override { return 0.8; }
	void MakeGibs(void) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void LeapTouch(CBaseEntity* pOther);
	void SquishTouch(CBaseEntity* pOther);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
};
