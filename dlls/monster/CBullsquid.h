#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"monsters.h"

#define		SQUID_SPRINT_DIST	256 // how close the squid has to get before starting to sprint and refusing to swerve

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_SQUID_HURTHOP = LAST_COMMON_SCHEDULE + 1,
	SCHED_SQUID_SMELLFOOD,
	SCHED_SQUID_SEECRAB,
	SCHED_SQUID_EAT,
	SCHED_SQUID_SNIFF_AND_EAT,
	SCHED_SQUID_WALLOW,
};

//=========================================================
// monster-specific tasks
//=========================================================
enum 
{
	TASK_SQUID_HOPTURN = LAST_COMMON_TASK + 1,
};

//=========================================================
// Bullsquid's spit projectile
//=========================================================
class EXPORT CSquidSpit : public CBaseEntity
{
public:
	void Spawn( void );

	static void Shoot( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	void Touch( CBaseEntity *pOther );
	void Animate( void );
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int  m_maxFrame;

	static const char* pSpitHitSounds[];
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		BSQUID_AE_SPIT		( 1 )
#define		BSQUID_AE_BITE		( 2 )
#define		BSQUID_AE_BLINK		( 3 )
#define		BSQUID_AE_TAILWHIP	( 4 )
#define		BSQUID_AE_HOP		( 5 )
#define		BSQUID_AE_THROW		( 6 )

class EXPORT CBullsquid : public CBaseMonster
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
	void DeathSound( void ) override;
	void AlertSound ( void ) override;
	void AttackSound( void );
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
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) override;
	int IRelationship ( CBaseEntity *pTarget ) override;
	int IgnoreConditions ( void ) override;
	MONSTERSTATE GetIdealState ( void ) override;

	int	Save( CSave &save ) override;
	int Restore( CRestore &restore ) override;
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_fCanThreatDisplay;// this is so the squid only does the "I see a headcrab!" dance one time. 

	float m_flLastHurtTime;// we keep track of this, because if something hurts a squid, it will forget about its love of headcrabs for a while.
	float m_flNextSpitTime;// last time the bullsquid used the spit attack.

private:
	static const char* pAttackSounds[];
	static const char* pDieSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pGrowlSounds[];
	static const char* pBiteSounds[];
};
