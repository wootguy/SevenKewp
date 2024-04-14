/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
#ifndef TALKSQUADMONSTER_H
#define TALKSQUADMONSTER_H

#ifndef MONSTERS_H
#include "monsters.h"
#endif

//=========================================================
// Combination of the Talk and Squad monster base classes
// Used for scientists, barneys, grunts, op4 grunts, etc.
//=========================================================

#define TALKRANGE_MIN 500.0				// don't talk to anyone farther away than this

#define TLK_STARE_DIST	128				// anyone closer than this and looking at me is probably staring at me.
#define TLK_CFRIENDS	8

#define bit_saidDamageLight		(1<<0)	// bits so we don't repeat key sentences
#define bit_saidDamageMedium	(1<<1)
#define bit_saidDamageHeavy		(1<<2)
#define bit_saidHelloPlayer		(1<<3)
#define bit_saidWoundLight		(1<<4)
#define bit_saidWoundHeavy		(1<<5)
#define bit_saidHeard			(1<<6)
#define bit_saidSmelled			(1<<7)

typedef enum
{
	TLK_ANSWER = 0,
	TLK_QUESTION,
	TLK_IDLE,
	TLK_STARE,
	TLK_USE,
	TLK_UNUSE,
	TLK_STOP,
	TLK_NOSHOOT,
	TLK_HELLO,
	TLK_PHELLO,
	TLK_PIDLE,
	TLK_PQUESTION,
	TLK_PLHURT1,
	TLK_PLHURT2,
	TLK_PLHURT3,
	TLK_SMELL,
	TLK_WOUND,
	TLK_MORTAL,

	TLK_CGROUPS,					// MUST be last entry
} TALKSQUADGROUPNAMES;

enum
{
	TASK_TLK_RESPOND = LAST_COMMON_TASK + 1,		// say my response
	TASK_TLK_SPEAK,			// question or remark
	TASK_TLK_HELLO,			// Try to say hello to player
	TASK_TLK_HEADRESET,		// reset head position
	TASK_TLK_STOPSHOOTING,	// tell player to stop shooting friend
	TASK_TLK_STARE,			// let the player know I know he's staring at me.
	TASK_TLK_LOOK_AT_CLIENT,// faces player if not moving and not talking and in idle.
	TASK_TLK_CLIENT_STARE,	// same as look at client, but says something if the player stares.
	TASK_TLK_EYECONTACT,	// maintain eyecontact with person who I'm talking to
	TASK_TLK_IDEALYAW,		// set ideal yaw to face who I'm talking to

	LAST_TALKMONSTER_TASK,			// MUST be last
};


#define	SF_SQUADMONSTER_LEADER	32

#define bits_NO_SLOT		0

// HUMAN GRUNT SLOTS
#define bits_SLOT_HGRUNT_ENGAGE1	( 1 << 0 )
#define bits_SLOT_HGRUNT_ENGAGE2	( 1 << 1 )
#define bits_SLOTS_HGRUNT_ENGAGE	( bits_SLOT_HGRUNT_ENGAGE1 | bits_SLOT_HGRUNT_ENGAGE2 )

#define bits_SLOT_HGRUNT_GRENADE1	( 1 << 2 ) 
#define bits_SLOT_HGRUNT_GRENADE2	( 1 << 3 ) 
#define bits_SLOTS_HGRUNT_GRENADE	( bits_SLOT_HGRUNT_GRENADE1 | bits_SLOT_HGRUNT_GRENADE2 )

// ALIEN GRUNT SLOTS
#define bits_SLOT_AGRUNT_HORNET1	( 1 << 4 )
#define bits_SLOT_AGRUNT_HORNET2	( 1 << 5 )
#define bits_SLOT_AGRUNT_CHASE		( 1 << 6 )
#define bits_SLOTS_AGRUNT_HORNET	( bits_SLOT_AGRUNT_HORNET1 | bits_SLOT_AGRUNT_HORNET2 )

// HOUNDEYE SLOTS
#define bits_SLOT_HOUND_ATTACK1		( 1 << 7 )
#define bits_SLOT_HOUND_ATTACK2		( 1 << 8 )
#define bits_SLOT_HOUND_ATTACK3		( 1 << 9 )
#define bits_SLOTS_HOUND_ATTACK		( bits_SLOT_HOUND_ATTACK1 | bits_SLOT_HOUND_ATTACK2 | bits_SLOT_HOUND_ATTACK3 )

// global slots
#define bits_SLOT_SQUAD_SPLIT		( 1 << 10 )// squad members don't all have the same enemy

#define NUM_SLOTS			11// update this every time you add/remove a slot.

#define	MAX_SQUAD_MEMBERS	5


class EXPORT CTalkSquadMonster : public CBaseMonster
{
public:
	void			TalkInit( void );				
	CBaseEntity		*FindNearestFriend(BOOL fPlayer);
	float			TargetDistance( void );
	void			StopTalking( void ) { SentenceStop(); }
	
	// Base Monster functions
	virtual void	Precache( void );
	int				TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	void			Killed( entvars_t *pevAttacker, int iGib );
	int				IRelationship ( CBaseEntity *pTarget );
	virtual int		CanPlaySentence( BOOL fDisregardState );
	virtual void	PlaySentence( const char *pszSentence, float duration, float volume, float attenuation );
	void			PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener );
	virtual void	KeyValue( KeyValueData *pkvd );

	// AI functions
	void			SetActivity ( Activity newActivity );
	Schedule_t		*GetScheduleOfType ( int Type );
	void			StartTask( Task_t *pTask );
	void			RunTask( Task_t *pTask );
	virtual const char* GetTaskName(int taskIdx);
	void			HandleAnimEvent( MonsterEvent_t *pEvent );
	void			PrescheduleThink( void );
	

	// Conversations / communication
	int				GetVoicePitch( void );
	void			IdleRespond( void );
	int				FIdleSpeak( void );
	int				FIdleStare( void );
	int				FIdleHello( void );
	void			IdleHeadTurn( Vector &vecFriend );
	int				FOkToSpeak( void );
	void			TrySmellTalk( void );
	CBaseEntity		*EnumFriends( CBaseEntity *pentPrevious, int listNumber, BOOL bTrace );
	void			AlertFriends( void );
	void			ShutUpFriends( void );
	BOOL			IsTalking( void );
	void			Talk( float flDuration );	

	// following
	virtual void StopFollowing(BOOL clearSchedule);
	virtual void StartFollowing(CBaseEntity* pLeader);
	virtual bool CanBePushed();
	
	virtual void	SetAnswerQuestion(CTalkSquadMonster* pSpeaker );
	virtual int		FriendNumber( int arrayNumber )	{ return arrayNumber; }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	
	static const char *m_szFriends[TLK_CFRIENDS];		// array of friend names
	static float g_talkWaitTime;
	
	int			m_bitsSaid;						// set bits for sentences we don't want repeated
	int			m_nSpeak;						// number of times initiated talking
	int			m_voicePitch;					// pitch of voice for this head
	const char	*m_szGrp[TLK_CGROUPS];			// sentence group names
	int			m_iszUse;						// Custom +USE sentence group (follow)
	int			m_iszUnUse;						// Custom +USE sentence group (stop following)

	float		m_flLastSaidSmelled;// last time we talked about something that stinks
	float		m_flStopTalkTime;// when in the future that I'll be done saying this sentence.

	EHANDLE		m_hTalkTarget;	// who to look at while talking
	CUSTOM_SCHEDULES;


	// 
	// squad functions 
	//

	// squad leader info
	EHANDLE	m_hSquadLeader;		// who is my leader
	EHANDLE	m_hSquadMember[MAX_SQUAD_MEMBERS - 1];	// valid only for leader
	int		m_afSquadSlots;
	float	m_flLastEnemySightTime; // last time anyone in the squad saw the enemy
	BOOL	m_fEnemyEluded;

	// squad member info
	int		m_iMySlot;// this is the behaviour slot that the monster currently holds in the squad. 

	int  CheckEnemy(CBaseEntity* pEnemy);
	virtual void StartMonster(void);
	void VacateSlot(void);
	void ScheduleChange(void);
	BOOL OccupySlot(int iDesiredSlot);
	virtual BOOL NoFriendlyFire(void);

	// squad functions still left in base class
	CTalkSquadMonster* MySquadLeader()
	{
		CTalkSquadMonster* pSquadLeader = (CTalkSquadMonster*)((CBaseEntity*)m_hSquadLeader);
		if (pSquadLeader != NULL)
			return pSquadLeader;
		return this;
	}
	CTalkSquadMonster* MySquadMember(int i)
	{
		if (i >= MAX_SQUAD_MEMBERS - 1)
			return this;
		else
			return (CTalkSquadMonster*)((CBaseEntity*)m_hSquadMember[i]);
	}
	int	InSquad(void) { return m_hSquadLeader != NULL; }
	int IsLeader(void) { return m_hSquadLeader == this; }
	int SquadJoin(int searchRadius);
	int SquadRecruit(int searchRadius, int maxMembers);
	int	SquadCount(void);
	void SquadRemove(CTalkSquadMonster* pRemove);
	void SquadUnlink(void);
	BOOL SquadAdd(CTalkSquadMonster* pAdd);
	void SquadDisband(void);
	void SquadAddConditions(int iConditions);
	void SquadMakeEnemy(CBaseEntity* pEnemy);
	void SquadPasteEnemyInfo(void);
	void SquadCopyEnemyInfo(void);
	BOOL SquadEnemySplit(void);
	BOOL SquadMemberInRange(const Vector& vecLocation, float flDist);

	virtual CTalkSquadMonster* MyTalkSquadMonsterPointer(void) { return this; }

	BOOL FValidateCover(const Vector& vecCoverLocation);

	MONSTERSTATE GetIdealState(void);

	//
	// opposing force
	//

	EHANDLE m_hWaitMedic;
	float m_flMedicWaitTime;
	float m_flLastHitByPlayer;
	int m_iPlayerHits;
	float m_flPlayerDamage;

	CTalkSquadMonster* MySquadMedic();

	CTalkSquadMonster* FindSquadMedic(int searchRadius);

	BOOL HealMe(CTalkSquadMonster* pTarget);
};


// Don't see a client right now.
#define		bits_COND_CLIENT_UNSEEN		( bits_COND_SPECIAL2 )


#endif		//TALKMONSTER_H
