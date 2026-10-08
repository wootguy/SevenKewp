#pragma once
#include	"extdll.h"
#include	"util.h"
#include    "CFlyingMonster.h"
#include    "nodes.h"
#include    "monsters.h"

#define SEARCH_RETRY	16

#define ICHTHYOSAUR_SPEED 150

extern CGraph WorldGraph;

#define EYE_MAD		0
#define EYE_BASE	1
#define EYE_CLOSED	2
#define EYE_BACK	3
#define EYE_LOOK	4

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

// UNDONE: Save/restore here
class EXPORT CIchthyosaur : public CFlyingMonster
{
public:
	void  Spawn( void ) override;
	void  Precache( void ) override;
	void  SetYawSpeed( void ) override;
	int   Classify( void ) override;
	const char* DisplayName() override;
	void  HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	CUSTOM_SCHEDULES;

	int	Save( CSave &save ) override;
	int Restore( CRestore &restore ) override;
	static TYPEDESCRIPTION m_SaveData[];

	Schedule_t *GetSchedule( void ) override;
	Schedule_t *GetScheduleOfType ( int Type ) override;
	const char* GetTaskName(int taskIdx) override;

	void Killed( entvars_t *pevAttacker, int iGib ) override;
	void BecomeDead( void ) override;

	void CombatUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void BiteTouch( CBaseEntity *pOther );

	void  StartTask( Task_t *pTask ) override;
	void  RunTask( Task_t *pTask ) override;

	BOOL  CheckMeleeAttack1 ( float flDot, float flDist ) override;
	BOOL  CheckRangeAttack1 ( float flDot, float flDist ) override;

	float ChangeYaw( int yawSpeed) override;
	Activity GetStoppedActivity( void ) override;

	void  Move( float flInterval ) override;
	void  MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval ) override;
	void  MonsterThink( void ) override;
	void  Stop( void ) override;
	void  Swim( void );
	Vector DoProbe(const Vector &Probe);

	float VectorToPitch( const Vector &vec);
	float FlPitchDiff( void );
	float ChangePitch( int yawSpeed);
	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }

	Vector m_SaveVelocity;
	float m_idealDist;

	float m_flBlink;

	float m_flEnemyTouched;
	BOOL  m_bOnAttack;

	float m_flMaxSpeed;
	float m_flMinSpeed;
	float m_flMaxDist;

	float m_flNextAlert;

	float m_flLastPitchTime;	// Last frame time pitch was changed
	float m_flLastZYawTime;		// Last frame time Z was changed when yaw was changed

	static const char *pIdleSounds[];
	static const char *pAlertSounds[];
	static const char *pAttackSounds[];
	static const char *pBiteSounds[];
	static const char *pDieSounds[];
	static const char *pPainSounds[];

	void IdleSound( void ) override;
	void AlertSound( void ) override;
	void AttackSound( void );
	void BiteSound( void );
	void DeathSound( void ) override;
	void PainSound( void ) override;
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;
};
