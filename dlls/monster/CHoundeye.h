#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"monsters.h"
#include	"nodes.h"
#include	"CTalkSquadMonster.h"

extern CGraph WorldGraph;

// houndeye does 20 points of damage spread over a sphere 384 units in diameter, and each additional 
// squad member increases the BASE damage by 110%, per the spec.
#define HOUNDEYE_MAX_SQUAD_SIZE			4
#define	HOUNDEYE_MAX_ATTACK_RADIUS		384
#define	HOUNDEYE_SQUAD_BONUS			(float)1.1

#define HOUNDEYE_EYE_FRAMES 3 // how many different switchable maps for the eye

#define HOUNDEYE_SOUND_STARTLE_VOLUME	128 // how loud a sound has to be to badly scare a sleeping houndeye

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_HOUND_CLOSE_EYE = LAST_COMMON_TASK + 1,
	TASK_HOUND_OPEN_EYE,
	TASK_HOUND_THREAT_DISPLAY,
	TASK_HOUND_FALL_ASLEEP,
	TASK_HOUND_WAKE_UP,
	TASK_HOUND_HOP_BACK
};

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_HOUND_AGITATED = LAST_COMMON_SCHEDULE + 1,
	SCHED_HOUND_HOP_RETREAT,
	SCHED_HOUND_FAIL,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		HOUND_AE_WARN			1
#define		HOUND_AE_STARTATTACK	2
#define		HOUND_AE_THUMP			3
#define		HOUND_AE_ANGERSOUND1	4
#define		HOUND_AE_ANGERSOUND2	5
#define		HOUND_AE_HOPBACK		6
#define		HOUND_AE_CLOSE_EYE		7

class EXPORT CHoundeye : public CTalkSquadMonster
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	int  Classify ( void ) override;
	const char* DisplayName() override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	void SetYawSpeed ( void ) override;
	void WarmUpSound ( void );
	void AlertSound( void ) override;
	void DeathSound( void ) override;
	void WarnSound( void );
	void PainSound( void ) override;
	void IdleSound( void ) override;
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;
	void StartTask( Task_t *pTask ) override;
	void RunTask ( Task_t *pTask ) override;
	void SonicAttack( void );
	void PrescheduleThink( void ) override;
	void SetActivity ( Activity NewActivity ) override;
	RGB GetBeamColor ( void );
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override;
	BOOL FValidateHintType ( short sHint ) override;
	BOOL FCanActiveIdle ( void ) override;
	Schedule_t *GetScheduleOfType ( int Type ) override;
	Schedule_t *GetSchedule( void ) override;
	const char* GetTaskName(int taskIdx) override;
	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }

	int	Save( CSave &save ) override;
	int Restore( CRestore &restore ) override;

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	int m_iSpriteTexture;
	BOOL m_fAsleep;// some houndeyes sleep in idle mode if this is set, the houndeye is lying down
	BOOL m_fDontBlink;// don't try to open/close eye if this bit is set!
	Vector	m_vecPackCenter; // the center of the pack. The leader maintains this by averaging the origins of all pack members.

private:
	static const char* pAlertSounds[];
	static const char* pDieSounds[];
	static const char* pIdleSounds[];
	static const char* pHuntSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pBlastSounds[];

};
