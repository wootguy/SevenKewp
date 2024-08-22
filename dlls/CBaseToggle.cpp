#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "nodes.h"
#include "CBaseDoor.h"

// Global Savedata for Toggle
TYPEDESCRIPTION	CBaseToggle::m_SaveData[] =
{
	DEFINE_FIELD(CBaseToggle, m_toggle_state, FIELD_INTEGER),
	DEFINE_FIELD(CBaseToggle, m_flActivateFinished, FIELD_TIME),
	DEFINE_FIELD(CBaseToggle, m_flMoveDistance, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_flWait, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_flLip, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_flTWidth, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_flTLength, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_vecPosition1, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseToggle, m_vecPosition2, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseToggle, m_vecAngle1, FIELD_VECTOR),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD(CBaseToggle, m_vecAngle2, FIELD_VECTOR),		// UNDONE: Position could go through transition, but also angle?
	DEFINE_FIELD(CBaseToggle, m_cTriggersLeft, FIELD_INTEGER),
	DEFINE_FIELD(CBaseToggle, m_flHeight, FIELD_FLOAT),
	DEFINE_FIELD(CBaseToggle, m_hActivator, FIELD_EHANDLE),
	DEFINE_FIELD(CBaseToggle, m_pfnCallWhenMoveDone, FIELD_FUNCTION),
	DEFINE_FIELD(CBaseToggle, m_vecFinalDest, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseToggle, m_vecFinalAngle, FIELD_VECTOR),
	DEFINE_FIELD(CBaseToggle, m_sMaster, FIELD_STRING),
	DEFINE_FIELD(CBaseToggle, m_bitsDamageInflict, FIELD_INTEGER),	// damage type inflicted
};
IMPLEMENT_SAVERESTORE(CBaseToggle, CBaseAnimating);


void CBaseToggle::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_flMoveDistance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

/*
=============
LinearMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
===============
*/
void CBaseToggle::LinearMove(Vector	vecDest, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "LinearMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "LinearMove: no post-move function defined");

	m_vecFinalDest = vecDest;

	// Already there?
	if (vecDest == pev->origin)
	{
		LinearMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - pev->origin;

	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to LinearMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::LinearMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->velocity = vecDestDelta / flTravelTime;
}


/*
============
After moving, set origin to exact final destination, call "move done" function
============
*/
void CBaseToggle::LinearMoveDone(void)
{
	Vector delta = m_vecFinalDest - pev->origin;
	float error = delta.Length();
	if (error > 0.03125)
	{
		LinearMove(m_vecFinalDest, 100);
		return;
	}

	UTIL_SetOrigin(pev, m_vecFinalDest);
	pev->velocity = g_vecZero;
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

BOOL CBaseToggle::IsLockedByMaster(void)
{
	if (m_sMaster && !UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
		return TRUE;
	else
		return FALSE;
}

/*
=============
AngularMove

calculate pev->velocity and pev->nextthink to reach vecDest from
pev->origin traveling at flSpeed
Just like LinearMove, but rotational.
===============
*/
void CBaseToggle::AngularMove(Vector vecDestAngle, float flSpeed)
{
	ASSERTSZ(flSpeed != 0, "AngularMove:  no speed is defined!");
	//	ASSERTSZ(m_pfnCallWhenMoveDone != NULL, "AngularMove: no post-move function defined");

	m_vecFinalAngle = vecDestAngle;

	// Already there?
	if (vecDestAngle == pev->angles)
	{
		AngularMoveDone();
		return;
	}

	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDestAngle - pev->angles;

	// divide by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to AngularMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::AngularMoveDone);

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = vecDestDelta / flTravelTime;
}


/*
============
After rotating, set angle to exact final angle, call "move done" function
============
*/
void CBaseToggle::AngularMoveDone(void)
{
	pev->angles = m_vecFinalAngle;
	pev->avelocity = g_vecZero;
	pev->nextthink = -1;
	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}


float CBaseToggle::AxisValue(int flags, const Vector& angles)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angles.z;
	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angles.x;

	return angles.y;
}


void CBaseToggle::AxisDir(entvars_t* pev)
{
	if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_Z))
		pev->movedir = Vector(0, 0, 1);	// around z-axis
	else if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_X))
		pev->movedir = Vector(1, 0, 0);	// around x-axis
	else
		pev->movedir = Vector(0, 1, 0);		// around y-axis
}


float CBaseToggle::AxisDelta(int flags, const Vector& angle1, const Vector& angle2)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angle1.z - angle2.z;

	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angle1.x - angle2.x;

	return angle1.y - angle2.y;
}


void CBaseToggle::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation)
{
	if (pszSentence && IsAlive())
	{
		if (pszSentence[0] == '!')
			EMIT_SOUND_DYN(edict(), CHAN_VOICE, pszSentence, volume, attenuation, 0, PITCH_NORM);
		else if (pszSentence[0] == '+')
			EMIT_SOUND_DYN(edict(), CHAN_VOICE, pszSentence + 1, volume, attenuation, 0, PITCH_NORM);
		else
			SENTENCEG_PlayRndSz(edict(), pszSentence, volume, attenuation, 0, PITCH_NORM);
	}
}


void CBaseToggle::PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity* pListener)
{
	PlaySentence(pszSentence, duration, volume, attenuation);
}


void CBaseToggle::SentenceStop(void)
{
	EMIT_SOUND(edict(), CHAN_VOICE, "common/null.wav", 1.0, ATTN_IDLE);
}