#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"CBaseMonster.h"

#define	STUKABAT_AE_FLAP 2
#define	STUKABAT_AE_BITE 3
#define STUKABAT_FLAP_SOUND "stukabat/flap.wav"

class EXPORT CStukabat : public CBaseMonster
{
public:
	virtual int		Save( CSave &save ) override;
	virtual int		Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void ) override;
	void Precache( void ) override;
	void SetYawSpeed( void ) override;
	int  Classify ( void ) override;
	const char* DisplayName() override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;

	void RunAI( void ) override;
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override;	// balls
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) override;	// head
	BOOL CheckMeleeAttack1 ( float flDot, float flDist ) override;	// block, throw
	Schedule_t* GetSchedule ( void ) override;
	Schedule_t* GetScheduleOfType ( int Type ) override;
	const char* GetTaskName(int taskIdx) override;
	void StartTask ( Task_t *pTask ) override;
	void RunTask ( Task_t *pTask ) override;
	void DiveTouch(CBaseEntity* pOther);
	CUSTOM_SCHEDULES;

	void Stop( void ) override;
	void Move ( float flInterval ) override;
	int  CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist ) override;
	void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval ) override;
	void SetActivity ( Activity NewActivity ) override;
	int LookupActivity(int activity) override;
	BOOL ShouldAdvanceRoute( float flWaypointDist ) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	void PainSound( void ) override;
	void AlertSound( void ) override;
	void IdleSound( void ) override;
	void AttackSound( void );
	void DeathSound( void ) override;
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;

	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pDeathSounds[];
	static const char *pAttackHitSounds[];
	static const char *pFlapSounds[];

	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }

	Vector m_retreatPos; // position flying to to prepare for a dive attack
	Vector m_velocity;
	int m_isDiving; // 1 = no flapping, 2 = flapping
};
