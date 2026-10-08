#pragma once
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "CTalkSquadMonster.h"
#include "game.h"
#include "CGrenade.h"
#include "skill.h"

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_ASSASSIN_EXPOSED = LAST_COMMON_SCHEDULE + 1,// cover was blown.
	SCHED_ASSASSIN_JUMP,	// fly through the air
	SCHED_ASSASSIN_JUMP_ATTACK,	// fly through the air and shoot
	SCHED_ASSASSIN_JUMP_LAND, // hit and run away
};

//=========================================================
// monster-specific tasks
//=========================================================

enum
{
	TASK_ASSASSIN_FALL_TO_GROUND = LAST_COMMON_TASK + 1, // falling and waiting to hit ground
};


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		ASSASSIN_AE_SHOOT1	1
#define		ASSASSIN_AE_TOSS1	2
#define		ASSASSIN_AE_JUMP	3


#define bits_MEMORY_BADJUMP		(bits_MEMORY_CUSTOM1)

#define HASS_FOLLOW_SOUND "buttons/blip2.wav"
#define HASS_UNFOLLOW_SOUND "buttons/blip1.wav"

class EXPORT CHAssassin : public CBaseMonster
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void SetYawSpeed ( void ) override;
	int  Classify ( void ) override;
	const char* DisplayName() override;
	int  ISoundMask ( void) override;
	void Shoot( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	Schedule_t* GetSchedule ( void ) override;
	Schedule_t* GetScheduleOfType ( int Type ) override;
	const char* GetTaskName(int taskIdx) override;
	BOOL CheckMeleeAttack1 ( float flDot, float flDist ) override;	// jump
	// BOOL CheckMeleeAttack2 ( float flDot, float flDist );
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override;	// shoot
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) override;	// throw grenade
	void StartTask ( Task_t *pTask ) override;
	void RunAI( void ) override;
	void RunTask ( Task_t *pTask ) override;
	void DeathSound ( void ) override;
	void IdleSound ( void ) override;
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;
	const char* GetDeathNoticeWeapon() override { return "weapon_9mmhandgun"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }
	CUSTOM_SCHEDULES;

	int	Save( CSave &save ) override;
	int Restore( CRestore &restore ) override;
	static TYPEDESCRIPTION m_SaveData[];

	float m_flLastShot;
	float m_flDiviation;

	float m_flNextJump;
	Vector m_vecJumpVelocity;

	float m_flNextGrenadeCheck;
	Vector	m_vecTossVelocity;
	BOOL	m_fThrowGrenade;

	int		m_iTargetRanderamt;

	int		m_iFrustration;

	int		m_iShell;
};
