#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"
#include "CBaseButton.h"
#include "CBaseDoor.h"

class CMomentaryDoor : public CBaseToggle
{
public:
	void	Spawn(void);
	void Precache(void);

	void	KeyValue(KeyValueData* pkvd);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int	ObjectCaps(void) { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	void EXPORT DoorMoveDone(void);

	BYTE	m_bMoveSnd;			// sound a door makes while moving	
};

LINK_ENTITY_TO_CLASS(momentary_door, CMomentaryDoor)

TYPEDESCRIPTION	CMomentaryDoor::m_SaveData[] =
{
	DEFINE_FIELD(CMomentaryDoor, m_bMoveSnd, FIELD_CHARACTER),
};

IMPLEMENT_SAVERESTORE(CMomentaryDoor, CBaseToggle)

void CMomentaryDoor::Spawn(void)
{
	SetMovedir(pev);

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;
	if (pev->dmg == 0)
		pev->dmg = 2;

	m_vecPosition1 = pev->origin;
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));
	ASSERTSZ(m_vecPosition1 != m_vecPosition2, "door start/end positions are equal");

	if (FBitSet(pev->spawnflags, SF_DOOR_START_OPEN))
	{	// swap pos1 and pos2, put door at pos2
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_vecPosition2 = m_vecPosition1;
		m_vecPosition1 = pev->origin;
	}
	SetTouch(NULL);

	Precache();
}

void CMomentaryDoor::Precache(void)
{

	// set the door's "in-motion" sound
	switch (m_bMoveSnd)
	{
	case	0:
		pev->door_noiseMoving = ALLOC_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("doors/doormove1.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove1.wav");
		break;
	case	2:
		PRECACHE_SOUND("doors/doormove2.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove2.wav");
		break;
	case	3:
		PRECACHE_SOUND("doors/doormove3.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove3.wav");
		break;
	case	4:
		PRECACHE_SOUND("doors/doormove4.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove4.wav");
		break;
	case	5:
		PRECACHE_SOUND("doors/doormove5.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove5.wav");
		break;
	case	6:
		PRECACHE_SOUND("doors/doormove6.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove6.wav");
		break;
	case	7:
		PRECACHE_SOUND("doors/doormove7.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove7.wav");
		break;
	case	8:
		PRECACHE_SOUND("doors/doormove8.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove8.wav");
		break;
	default:
		pev->door_noiseMoving = ALLOC_STRING("common/null.wav");
		break;
	}
}

void CMomentaryDoor::KeyValue(KeyValueData* pkvd)
{

	if (FStrEq(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "stopsnd"))
	{
		//		m_bStopSnd = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "healthvalue"))
	{
		//		m_bHealthValue = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CMomentaryDoor::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (useType != USE_SET)		// Momentary buttons will pass down a float in here
		return;

	if (value > 1.0)
		value = 1.0;
	if (value < 0.0)
		value = 0.0;

	Vector move = m_vecPosition1 + (value * (m_vecPosition2 - m_vecPosition1));

	Vector delta = move - pev->origin;
	//float speed = delta.Length() * 10;
	float speed = delta.Length() / 0.1; // move there in 0.1 sec
	if (speed == 0)
		return;

	// This entity only thinks when it moves, so if it's thinking, it's in the process of moving
	// play the sound when it starts moving (not yet thinking)
	if (pev->nextthink < pev->ltime || pev->nextthink == 0)
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseMoving), 1, ATTN_NORM);
	// If we already moving to designated point, return
	else if (move == m_vecFinalDest)
		return;

	SetMoveDone(&CMomentaryDoor::DoorMoveDone);
	LinearMove(move, speed);
}

//
// The door has reached needed position.
//
void CMomentaryDoor::DoorMoveDone(void)
{
	STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseMoving));
	EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseArrived), 1, ATTN_NORM);
}
