#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"CTalkSquadMonster.h"
#include	"CCineMonster.h"

#define		NUM_SCIENTIST_HEADS		4 // four heads available for scientist model
enum { HEAD_GLASSES = 0, HEAD_EINSTEIN = 1, HEAD_LUTHER = 2, HEAD_SLICK = 3 };

enum
{
	SCHED_HIDE = LAST_COMMON_SCHEDULE + 1,
	SCHED_FEAR,
	SCHED_PANIC,
	SCHED_STARTLE,
	SCHED_TARGET_CHASE_SCARED,
	SCHED_TARGET_FACE_SCARED,
};

enum
{
	TASK_SAY_HEAL = LAST_TALKMONSTER_TASK + 1,
	TASK_HEAL,
	TASK_SAY_FEAR,
	TASK_RUN_PATH_SCARED,
	TASK_SCREAM,
	TASK_RANDOM_SCREAM,
	TASK_MOVE_TO_TARGET_RANGE_SCARED,
};

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		SCIENTIST_AE_HEAL		( 1 )
#define		SCIENTIST_AE_NEEDLEON	( 2 )
#define		SCIENTIST_AE_NEEDLEOFF	( 3 )

//=======================================================
// Scientist
//=======================================================

class EXPORT CScientist : public CTalkSquadMonster
{
public:
	void Spawn( void );
	void Precache( void );

	void SetYawSpeed( void );
	int  Classify ( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual int FriendNumber( int arrayNumber );
	void SetActivity ( Activity newActivity );
	Activity GetStoppedActivity( void );
	int ISoundMask( void );
	void DeclineFollowing( void );

	float	CoverRadius( void ) { return 1200; }		// Need more room for cover because scientists want to get far away!
	BOOL	DisregardEnemy( CBaseEntity *pEnemy ) { return !pEnemy->IsAlive() || (gpGlobals->time - m_fearTime) > 15; }

	BOOL	CanHeal( void );
	void	Heal( void );
	void	Scream( void );

	// Override these to set behavior
	Schedule_t *GetScheduleOfType ( int Type );
	Schedule_t *GetSchedule ( void );
	const char* GetTaskName(int taskIdx);
	MONSTERSTATE GetIdealState ( void );

	void DeathSound( void );
	void PainSound( void );
	
	void TalkInit( void );

	void			Killed( entvars_t *pevAttacker, int iGib );
	
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	CUSTOM_SCHEDULES;

private:	
	float m_painTime;
	float m_healTime;
	float m_fearTime;
	bool m_isCleansuit;

	static const char* pPainSounds[];
};


//=========================================================
// Dead Scientist PROP
//=========================================================
class EXPORT CDeadScientist : public CBaseMonster
{
public:
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }
	void Spawn( void );
	int	Classify ( void ) { return	CLASS_HUMAN_PASSIVE; }

	void KeyValue( KeyValueData *pkvd );
	int	m_iPose;// which sequence to display
	static const char *m_szPoses[7];
};


//=========================================================
// Sitting Scientist PROP
//=========================================================

class EXPORT CSittingScientist : public CScientist // kdb: changed from public CBaseMonster so he can speak
{
public:
	void Spawn( void );
	void Precache( void );

	void DropThink( void );
	void SittingThink( void );
	int	Classify ( void );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	virtual void SetAnswerQuestion( CTalkSquadMonster *pSpeaker );
	int FriendNumber( int arrayNumber );

	int FIdleSpeak ( void );
	int		m_baseSequence;	
	int		m_headTurn;
	float	m_flResponseDelay;
};

// animation sequence aliases 
typedef enum
{
	SITTING_ANIM_sitlookleft,
	SITTING_ANIM_sitlookright,
	SITTING_ANIM_sitscared,
	SITTING_ANIM_sitting2,
	SITTING_ANIM_sitting3
} SITTING_ANIM;
