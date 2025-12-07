#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"CTalkSquadMonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	CONTROLLER_AE_HEAD_OPEN		1
#define	CONTROLLER_AE_BALL_SHOOT	2
#define	CONTROLLER_AE_SMALL_SHOOT	3
#define CONTROLLER_AE_POWERUP_FULL	4
#define CONTROLLER_AE_POWERUP_HALF	5

#define CONTROLLER_FLINCH_DELAY			2		// at most one flinch every n secs

class EXPORT CController : public CTalkSquadMonster
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
	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	CUSTOM_SCHEDULES;

	void Stop( void );
	void Move ( float flInterval );
	int  CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist );
	void MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval );
	void SetActivity ( Activity NewActivity );
	BOOL ShouldAdvanceRoute( float flWaypointDist );
	int LookupFloat( );

	float m_flNextFlinch;

	float m_flShootTime;
	float m_flShootEnd;

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

	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	void Killed( entvars_t *pevAttacker, int iGib );
	void GibMonster( void );
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	EHANDLE m_hBall[2];	// hand balls
	int m_iBall[2];			// how bright it should be
	float m_iBallTime[2];	// when it should be that color
	int m_iBallCurrent[2];	// current brightness

	Vector m_vecEstVelocity;

	Vector m_velocity;
	int m_fInCombat;
};


//=========================================================
// Controller bouncy ball attack
//=========================================================
class EXPORT CControllerHeadBall : public CBaseMonster
{
	void Spawn( void );
	void Precache( void );
	void HuntThink( void );
	void DieThink( void );
	void BounceTouch( CBaseEntity *pOther );
	void MovetoTarget( Vector vecTarget );
	void Crawl( void );
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }
	virtual BOOL IsNormalMonster() { return FALSE; }
	int m_iTrail;
	int m_flNextAttack;
	Vector m_vecIdeal;
	EHANDLE m_hOwner;
};


class EXPORT CControllerZapBall : public CBaseMonster
{
	void Spawn( void );
	void Precache( void );
	void AnimateThink( void );
	void ExplodeTouch( CBaseEntity *pOther );
	virtual BOOL IsNormalMonster() { return FALSE; }

	EHANDLE m_hOwner;
};
