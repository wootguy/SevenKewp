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
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	void RunAI( void );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );	// balls
	BOOL CheckRangeAttack2 ( float flDot, float flDist );	// head
	BOOL CheckMeleeAttack1 ( float flDot, float flDist );	// block, throw
	Schedule_t* GetSchedule ( void );
	Schedule_t* GetScheduleOfType ( int Type );
	const char* GetTaskName(int taskIdx);
	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	void DiveTouch(CBaseEntity* pOther);
	CUSTOM_SCHEDULES;

	void Stop( void );
	void Move ( float flInterval );
	int  CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist );
	void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );
	void SetActivity ( Activity NewActivity );
	int LookupActivity(int activity);
	BOOL ShouldAdvanceRoute( float flWaypointDist );
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	void PainSound( void );
	void AlertSound( void );
	void IdleSound( void );
	void AttackSound( void );
	void DeathSound( void );
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();

	static const char *pAttackSounds[];
	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pPainSounds[];
	static const char *pDeathSounds[];
	static const char *pAttackHitSounds[];
	static const char *pFlapSounds[];

	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	Vector m_retreatPos; // position flying to to prepare for a dive attack
	Vector m_velocity;
	int m_isDiving; // 1 = no flapping, 2 = flapping
};
