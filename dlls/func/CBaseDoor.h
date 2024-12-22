#pragma once
#include "CBaseToggle.h"

// doors
#define SF_DOOR_ROTATE_Y			0
#define	SF_DOOR_START_OPEN			1
#define SF_DOOR_ROTATE_BACKWARDS	2
#define SF_DOOR_PASSABLE			8
#define SF_DOOR_ONEWAY				16
#define	SF_DOOR_NO_AUTO_RETURN		32
#define SF_DOOR_ROTATE_Z			64
#define SF_DOOR_ROTATE_X			128
#define SF_DOOR_USE_ONLY			256	// door must be opened by player's use button.
#define SF_DOOR_NOMONSTERS			512	// Monster can't open
#define SF_DOOR_SILENT				0x80000000

#define door_noiseMoving noise1
#define door_noiseArrived noise2

typedef struct locksounds			// sounds that doors and buttons make when locked/unlocked
{
	string_t	sLockedSound;		// sound a door makes when it's locked
	string_t	sLockedSentence;	// sentence group played when door is locked
	string_t	sUnlockedSound;		// sound a door makes when it's unlocked
	string_t	sUnlockedSentence;	// sentence group played when door is unlocked

	int		iLockedSentence;		// which sentence in sentence group to play next
	int		iUnlockedSentence;		// which sentence in sentence group to play next

	float	flwaitSound;			// time delay between playing consecutive 'locked/unlocked' sounds
	float	flwaitSentence;			// time delay between playing consecutive sentences
	BYTE	bEOFLocked;				// true if hit end of list of locked sentences
	BYTE	bEOFUnlocked;			// true if hit end of list of unlocked sentences
} locksound_t;

enum ObeyTriggerMode {
	DOOR_OBEY_NO,			// ignore trigger mode
	DOOR_OBEY_YES,			// obey trigger mode
	DOOR_OBEY_YES_MOVING,	// obey trigger mode even if moving
};

void PlayLockSounds(entvars_t* pev, locksound_t* pls, int flocked, int fbutton);

class CBaseDoor : public CBaseToggle
{
public:
	void Spawn(void);
	void Precache(void);
	virtual void KeyValue(KeyValueData* pkvd);
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual void Blocked(CBaseEntity* pOther);
	virtual const char* DisplayName() { return "Door"; }
	void InitDoorTriggers();


	virtual int	ObjectCaps(void)
	{
		if (pev->spawnflags & SF_ITEM_USE_ONLY)
			return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE;
		else
			return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION);
	};
	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	virtual void SetToggleState(int state);

	// used to selectivly override defaults
	void EXPORT DoorTouch(CBaseEntity* pOther);

	// local functions
	int DoorActivate(USE_TYPE useType);
	void EXPORT DoorGoUp(void);
	void EXPORT DoorGoDown(void);
	virtual void EXPORT DoorHitTop(void);
	void EXPORT DoorHitBottom(void);

	BYTE	m_bHealthValue;// some doors are medi-kit doors, they give players health

	BYTE	m_bMoveSnd;			// sound a door makes while moving
	BYTE	m_bStopSnd;			// sound a door makes when it stops

	locksound_t m_ls;			// door lock sounds

	BYTE	m_bLockedSound;		// ordinals from entity selection
	BYTE	m_bLockedSentence;
	BYTE	m_bUnlockedSound;
	BYTE	m_bUnlockedSentence;

	// TODO: was 12 keyvalues really necessary for this? I think it can be done with 4.
	// what about the multi_manager style with "#1", or using a multi_manager for things that
	// always trigger at the same time as the door. Much ripenting needed.
	string_t m_fireOnOpenStart;
	string_t m_fireOnOpenEnd;
	string_t m_fireOnCloseStart;
	string_t m_fireOnCloseEnd;
	string_t m_fireOnStart;
	string_t m_fireOnStop;
	USE_TYPE m_fireOnOpenStartMode;
	USE_TYPE m_fireOnOpenEndMode;
	USE_TYPE m_fireOnCloseStartMode;
	USE_TYPE m_fireOnCloseEndMode;
	USE_TYPE m_fireOnStartMode;
	USE_TYPE m_fireOnStopMode;

	ObeyTriggerMode m_iObeyTriggerMode;

	float lastDamage;
};