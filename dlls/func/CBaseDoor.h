#pragma once

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

class CBaseDoor : public CBaseToggle
{
public:
	void Spawn(void);
	void Precache(void);
	virtual void KeyValue(KeyValueData* pkvd);
	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual void Blocked(CBaseEntity* pOther);


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
	int DoorActivate();
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
};