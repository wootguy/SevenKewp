#include "extdll.h"
#include "util.h"
#include "trains.h"
#include "saverestore.h"
#include "CBasePlatTrain.h"
#include "CFuncPlat.h"
#include "CFuncPlatRot.h"
#include "CFuncTrackChange.h"

LINK_ENTITY_TO_CLASS(func_trackchange, CFuncTrackChange)

TYPEDESCRIPTION	CFuncTrackChange::m_SaveData[] =
{
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_hTrackTop, FIELD_EHANDLE),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_hTrackBottom, FIELD_EHANDLE),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_hTrain, FIELD_EHANDLE),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackTopName, FIELD_STRING),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trackBottomName, FIELD_STRING),
	DEFINE_GLOBAL_FIELD(CFuncTrackChange, m_trainName, FIELD_STRING),
	DEFINE_FIELD(CFuncTrackChange, m_code, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTrackChange, m_targetState, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTrackChange, m_use, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CFuncTrackChange, CFuncPlatRot)

void CFuncTrackChange::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	Setup();
	if (FBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
		m_vecPosition2.z = pev->origin.z;

	SetupRotation();

	if (FBitSet(pev->spawnflags, SF_TRACK_STARTBOTTOM))
	{
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_toggle_state = TS_AT_BOTTOM;
		pev->angles = m_start;
		m_targetState = TS_AT_TOP;
	}
	else
	{
		UTIL_SetOrigin(pev, m_vecPosition1);
		m_toggle_state = TS_AT_TOP;
		pev->angles = m_end;
		m_targetState = TS_AT_BOTTOM;
	}

	EnableUse();
	pev->nextthink = pev->ltime + 2.0;
	SetThink(&CFuncTrackChange::Find);
	Precache();
}

void CFuncTrackChange::Precache(void)
{
	// Can't trigger sound
	PRECACHE_SOUND("buttons/button11.wav");

	CFuncPlatRot::Precache();
}


// UNDONE: Filter touches before re-evaluating the train.
void CFuncTrackChange::Touch(CBaseEntity* pOther)
{
#if 0
	TRAIN_CODE code;
	entvars_t* pevToucher = pOther->pev;
#endif
}



void CFuncTrackChange::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "train"))
	{
		m_trainName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "toptrack"))
	{
		m_trackTopName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bottomtrack"))
	{
		m_trackBottomName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CFuncPlatRot::KeyValue(pkvd);		// Pass up to base class
	}
}


void CFuncTrackChange::OverrideReset(void)
{
	pev->nextthink = pev->ltime + 1.0;
	SetThink(&CFuncTrackChange::Find);
}

void CFuncTrackChange::Find(void)
{
	// Find track entities
	CBaseEntity* target;

	target = UTIL_FindEntityByTargetname(NULL, STRING(m_trackTopName));
	if (target)
	{
		CPathTrack* top = CPathTrack::Instance(target->edict());
		m_hTrackTop = top;
		target = UTIL_FindEntityByTargetname(NULL, STRING(m_trackBottomName));
		if (target)
		{
			CPathTrack* bottom = CPathTrack::Instance(target->edict());
			m_hTrackBottom = bottom;
			target = UTIL_FindEntityByTargetname(NULL, STRING(m_trainName));
			if (target)
			{
				m_hTrain = CFuncTrackTrain::Instance(target->edict());
				if (!m_hTrain)
				{
					ALERT(at_error, "Can't find train for track change! %s\n", STRING(m_trainName));
					return;
				}
				Vector center = (pev->absmin + pev->absmax) * 0.5;
				m_hTrackBottom = bottom->Nearest(center);
				m_hTrackTop = top->Nearest(center);
				UpdateAutoTargets(m_toggle_state);
				SetThink(NULL);
				return;
			}
			else
			{
				ALERT(at_error, "Can't find train for track change! %s\n", STRING(m_trainName));
				target = UTIL_FindEntityByTargetname(NULL, STRING(m_trainName));
			}
		}
		else
			ALERT(at_error, "Can't find bottom track for track change! %s\n", STRING(m_trackBottomName));
	}
	else
		ALERT(at_error, "Can't find top track for track change! %s\n", STRING(m_trackTopName));
}



TRAIN_CODE CFuncTrackChange::EvaluateTrain(CPathTrack* pcurrent)
{
	CFuncTrackTrain* m_train = (CFuncTrackTrain*)m_hTrain.GetEntity();

	// Go ahead and work, we don't have anything to switch, so just be an elevator
	if (!pcurrent || !m_train)
		return TRAIN_SAFE;

	if (m_train->m_hPath.GetEntity() == pcurrent || 
		(pcurrent->m_hPrevious && m_train->m_hPath.GetEntity() == pcurrent->m_hPrevious.GetEntity()) ||
		(pcurrent->m_hNext && m_train->m_hPath.GetEntity() == pcurrent->m_hNext.GetEntity()))
	{
		if (m_train->pev->speed != 0)
			return TRAIN_BLOCKING;

		Vector dist = pev->origin - m_train->pev->origin;
		float length = dist.Length2D();
		if (length < m_train->m_length)		// Empirically determined close distance
			return TRAIN_FOLLOWING;
		else if (length > (150 + m_train->m_length))
			return TRAIN_SAFE;

		return TRAIN_BLOCKING;
	}

	return TRAIN_SAFE;
}


void CFuncTrackChange::UpdateTrain(Vector& dest)
{
	CFuncTrackTrain* m_train = (CFuncTrackTrain*)m_hTrain.GetEntity();
	if (!m_train) {
		return;
	}

	float time = (pev->nextthink - pev->ltime);

	m_train->pev->velocity = pev->velocity;
	m_train->pev->avelocity = pev->avelocity;
	m_train->NextThink(m_train->pev->ltime + time, FALSE);

	// Attempt at getting the train to rotate properly around the origin of the trackchange
	if (time <= 0)
		return;

	Vector offset = m_train->pev->origin - pev->origin;
	Vector delta = dest - pev->angles;
	// Transform offset into local coordinates
	UTIL_MakeInvVectors(delta, gpGlobals);
	Vector local;
	local.x = DotProduct(offset, gpGlobals->v_forward);
	local.y = DotProduct(offset, gpGlobals->v_right);
	local.z = DotProduct(offset, gpGlobals->v_up);

	local = local - offset;
	m_train->pev->velocity = pev->velocity + (local * (1.0 / time));
}

void CFuncTrackChange::GoDown(void)
{
	CFuncTrackTrain* m_train = (CFuncTrackTrain*)m_hTrain.GetEntity();

	if (m_code == TRAIN_BLOCKING)
		return;

	// HitBottom may get called during CFuncPlat::GoDown(), so set up for that
	// before you call GoDown()

	UpdateAutoTargets(TS_GOING_DOWN);
	// If ROTMOVE, move & rotate
	if (FBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		SetMoveDone(&CFuncTrackChange::CallHitBottom);
		m_toggle_state = TS_GOING_DOWN;
		AngularMove(m_start, pev->speed);
	}
	else
	{
		CFuncPlat::GoDown();
		SetMoveDone(&CFuncTrackChange::CallHitBottom);
		RotMove(m_start, pev->nextthink - pev->ltime);
	}
	// Otherwise, rotate first, move second

	// If the train is moving with the platform, update it
	if (m_code == TRAIN_FOLLOWING && m_train)
	{
		UpdateTrain(m_start);
		m_train->m_hPath = NULL;
	}
}


//
// Platform is at bottom, now starts moving up
//
void CFuncTrackChange::GoUp(void)
{
	CFuncTrackTrain* m_train = (CFuncTrackTrain*)m_hTrain.GetEntity();

	if (m_code == TRAIN_BLOCKING)
		return;

	// HitTop may get called during CFuncPlat::GoUp(), so set up for that
	// before you call GoUp();

	UpdateAutoTargets(TS_GOING_UP);
	if (FBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		m_toggle_state = TS_GOING_UP;
		SetMoveDone(&CFuncTrackChange::CallHitTop);
		AngularMove(m_end, pev->speed);
	}
	else
	{
		// If ROTMOVE, move & rotate
		CFuncPlat::GoUp();
		SetMoveDone(&CFuncTrackChange::CallHitTop);
		RotMove(m_end, pev->nextthink - pev->ltime);
	}

	// Otherwise, move first, rotate second

	// If the train is moving with the platform, update it
	if (m_code == TRAIN_FOLLOWING && m_train)
	{
		UpdateTrain(m_end);
		m_train->m_hPath = NULL;
	}
}


// Normal track change
void CFuncTrackChange::UpdateAutoTargets(int toggleState)
{
	CPathTrack* m_trackTop = (CPathTrack*)m_hTrackTop.GetEntity();
	CPathTrack* m_trackBottom = (CPathTrack*)m_hTrackBottom.GetEntity();

	if (!m_trackTop || !m_trackBottom)
		return;

	if (toggleState == TS_AT_TOP)
		ClearBits(m_trackTop->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_trackTop->pev->spawnflags, SF_PATH_DISABLED);

	if (toggleState == TS_AT_BOTTOM)
		ClearBits(m_trackBottom->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_trackBottom->pev->spawnflags, SF_PATH_DISABLED);
}


void CFuncTrackChange::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CPathTrack* m_trackTop = (CPathTrack*)m_hTrackTop.GetEntity();
	CPathTrack* m_trackBottom = (CPathTrack*)m_hTrackBottom.GetEntity();

	if (m_toggle_state != TS_AT_TOP && m_toggle_state != TS_AT_BOTTOM)
		return;

	// If train is in "safe" area, but not on the elevator, play alarm sound
	if (m_toggle_state == TS_AT_TOP)
		m_code = EvaluateTrain(m_trackTop);
	else if (m_toggle_state == TS_AT_BOTTOM)
		m_code = EvaluateTrain(m_trackBottom);
	else
		m_code = TRAIN_BLOCKING;
	if (m_code == TRAIN_BLOCKING)
	{
		// Play alarm and return
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/button11.wav", 1, ATTN_NORM);
		return;
	}

	// Otherwise, it's safe to move
	// If at top, go down
	// at bottom, go up

	DisableUse();
	if (m_toggle_state == TS_AT_TOP)
		GoDown();
	else
		GoUp();
}


//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncTrackChange::HitBottom(void)
{
	CFuncTrackTrain* m_train = (CFuncTrackTrain*)m_hTrain.GetEntity();
	CPathTrack* m_trackBottom = (CPathTrack*)m_hTrackBottom.GetEntity();

	CFuncPlatRot::HitBottom();
	if (m_code == TRAIN_FOLLOWING && m_train)
	{
		//		UpdateTrain();
		m_train->SetTrack(m_trackBottom);
	}
	SetThink(NULL);
	pev->nextthink = -1;

	UpdateAutoTargets(m_toggle_state);

	EnableUse();
}


//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncTrackChange::HitTop(void)
{
	CFuncTrackTrain* m_train = (CFuncTrackTrain*)m_hTrain.GetEntity();
	CPathTrack* m_trackTop = (CPathTrack*)m_hTrackTop.GetEntity();

	CFuncPlatRot::HitTop();
	if (m_code == TRAIN_FOLLOWING && m_train)
	{
		//		UpdateTrain();
		m_train->SetTrack(m_trackTop);
	}

	// Don't let the plat go back down
	SetThink(NULL);
	pev->nextthink = -1;
	UpdateAutoTargets(m_toggle_state);
	EnableUse();
}
