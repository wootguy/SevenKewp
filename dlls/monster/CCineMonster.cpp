#include "extdll.h"
#include "util.h"
#include "monsters.h"

#ifndef ANIMATION_H
#include "animation.h"
#endif

#include "schedule.h"
#include "CCineMonster.h"
#include "defaultai.h"

TYPEDESCRIPTION	CCineMonster::m_SaveData[] =
{
	DEFINE_FIELD(CCineMonster, m_iszIdle, FIELD_STRING),
	DEFINE_FIELD(CCineMonster, m_iszPlay, FIELD_STRING),
	DEFINE_FIELD(CCineMonster, m_iszEntity, FIELD_STRING),
	DEFINE_FIELD(CCineMonster, m_fMoveTo, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_flRepeat, FIELD_FLOAT),
	DEFINE_FIELD(CCineMonster, m_flRadius, FIELD_FLOAT),

	DEFINE_FIELD(CCineMonster, m_iDelay, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_startTime, FIELD_TIME),

	DEFINE_FIELD(CCineMonster,	m_saved_movetype, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster,	m_saved_solid, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_saved_effects, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_iFinishSchedule, FIELD_INTEGER),
	DEFINE_FIELD(CCineMonster, m_interruptable, FIELD_BOOLEAN),
};


IMPLEMENT_SAVERESTORE(CCineMonster, CBaseMonster)

LINK_ENTITY_TO_CLASS(scripted_sequence, CCineMonster)

LINK_ENTITY_TO_CLASS(aiscripted_sequence, CCineAI)

void ScriptEntityCancel(edict_t* pentCine)
{
	// make sure they are a scripted_sequence
	if (FClassnameIs(pentCine, "scripted_sequence") || FClassnameIs(pentCine, "aiscripted_sequence"))
	{
		CCineMonster* pCineTarget = GetClassPtr((CCineMonster*)VARS(pentCine));
		// make sure they have a monster in mind for the script
		CBaseEntity* pEntity = pCineTarget->m_hTargetEnt;
		CBaseMonster* pTarget = NULL;
		if (pEntity)
			pTarget = pEntity->MyMonsterPointer();

		if (pTarget)
		{
			// make sure their monster is actually playing a script
			if (pTarget->m_MonsterState == MONSTERSTATE_SCRIPT)
			{
				// tell them do die
				pTarget->m_scriptState = CCineMonster::SCRIPT_CLEANUP;
				// do it now
				pTarget->CineCleanup();
			}
		}
	}
}

//
// Cache user-entity-field values until spawn is called.
//

void CCineMonster::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszIdle"))
	{
		m_iszIdle = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszPlay"))
	{
		m_iszPlay = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszEntity"))
	{
		m_iszEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fMoveTo"))
	{
		m_fMoveTo = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flRepeat"))
	{
		m_flRepeat = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flRadius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iFinishSchedule"))
	{
		m_iFinishSchedule = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseMonster::KeyValue(pkvd);
	}
}


void CCineMonster::Spawn(void)
{
	// pev->solid = SOLID_TRIGGER;
	// UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
	pev->solid = SOLID_NOT;


	// REMOVE: The old side-effect
#if 0
	if (m_iszIdle)
		m_fMoveTo = 4;
#endif

	// if no targetname, start now
	if (FStringNull(pev->targetname) || !FStringNull(m_iszIdle))
	{
		SetThink(&CCineMonster::CineThink);
		pev->nextthink = gpGlobals->time + 1.0;
		// Wait to be used?
		if (pev->targetname)
			m_startTime = gpGlobals->time + 1E6;
	}
	if (pev->spawnflags & SF_SCRIPT_NOINTERRUPT)
		m_interruptable = FALSE;
	else
		m_interruptable = TRUE;
}

//=========================================================
// FCanOverrideState - returns FALSE, scripted sequences 
// cannot possess entities regardless of state.
//=========================================================
BOOL CCineMonster::FCanOverrideState(void)
{
	if (pev->spawnflags & SF_SCRIPT_OVERRIDESTATE)
		return TRUE;
	return FALSE;
}


//
// CineStart
//
void CCineMonster::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// do I already know who I should use
	CBaseEntity* pEntity = m_hTargetEnt;
	CBaseMonster* pTarget = NULL;

	if (pEntity)
		pTarget = pEntity->MyMonsterPointer();

	if (pTarget)
	{
		// am I already playing the script?
		if (pTarget->m_scriptState == SCRIPT_PLAYING)
			return;

		m_startTime = gpGlobals->time + 0.05;
	}
	else
	{
		// if not, try finding them
		SetThink(&CCineMonster::CineThink);
		pev->nextthink = gpGlobals->time;
	}
}


// This doesn't really make sense since only MOVETYPE_PUSH get 'Blocked' events
void CCineMonster::Blocked(CBaseEntity* pOther)
{

}

void CCineMonster::Touch(CBaseEntity* pOther)
{
	/*
		ALERT( at_aiconsole, "Cine Touch\n" );
		if (m_pentTarget && OFFSET(pOther->pev) == OFFSET(m_pentTarget))
		{
			CBaseMonster *pTarget = GetClassPtr((CBaseMonster *)VARS(m_pentTarget));
			pTarget->m_monsterState == MONSTERSTATE_SCRIPT;
		}
	*/
}


/*
	entvars_t *pevOther = VARS( gpGlobals->other );

	if ( !FBitSet ( pevOther->flags , FL_MONSTER ) )
	{// touched by a non-monster.
		return;
	}

	pevOther->origin.z += 1;

	if ( FBitSet ( pevOther->flags, FL_ONGROUND ) )
	{// clear the onground so physics don't bitch
		pevOther->flags -= FL_ONGROUND;
	}

	// toss the monster!
	pevOther->velocity = pev->movedir * pev->speed;
	pevOther->velocity.z += m_flHeight;


	pev->solid = SOLID_NOT;// kill the trigger for now !!!UNDONE
}
*/


//
// ********** Cinematic DIE **********
//
void CCineMonster::Die(void)
{
	SetThink(&CCineMonster::SUB_Remove);
}

//
// ********** Cinematic PAIN **********
//
void CCineMonster::Pain(void)
{

}

//
// ********** Cinematic Think **********
//

// find a viable entity
int CCineMonster::FindEntity(void)
{
	CBaseEntity* pentTarget;

	pentTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEntity));
	m_hTargetEnt = NULL;
	CBaseMonster* pTarget = NULL;

	while (pentTarget)
	{
		if (FBitSet(pentTarget->pev->flags, FL_MONSTER))
		{
			pTarget = pentTarget->MyMonsterPointer();
			if (pTarget && pTarget->CanPlaySequence(FCanOverrideState(), SS_INTERRUPT_BY_NAME))
			{
				m_hTargetEnt = pTarget;
				return TRUE;
			}
			ALERT(at_console, "%s (%s): Found %s, but can't play!\n",
				STRING(pev->targetname), STRING(pev->classname), STRING(m_iszEntity));
		}
		pentTarget = UTIL_FindEntityByTargetname(pentTarget, STRING(m_iszEntity));
		pTarget = NULL;
	}

	if (!pTarget)
	{
		CBaseEntity* pEntity = NULL;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_flRadius)) != NULL)
		{
			if (FClassnameIs(pEntity->pev, STRING(m_iszEntity)))
			{
				if (FBitSet(pEntity->pev->flags, FL_MONSTER))
				{
					pTarget = pEntity->MyMonsterPointer();
					if (pTarget && pTarget->CanPlaySequence(FCanOverrideState(), SS_INTERRUPT_IDLE))
					{
						m_hTargetEnt = pTarget;
						return TRUE;
					}
				}
			}
		}
	}
	pTarget = NULL;
	m_hTargetEnt = NULL;
	return FALSE;
}

// make the entity enter a scripted sequence
void CCineMonster::PossessEntity(void)
{
	CBaseEntity* pEntity = m_hTargetEnt;
	CBaseMonster* pTarget = NULL;
	if (pEntity)
		pTarget = pEntity->MyMonsterPointer();

	if (pTarget)
	{

		// FindEntity() just checked this!
#if 0
		if (!pTarget->CanPlaySequence(FCanOverrideState()))
		{
			ALERT(at_aiconsole, "Can't possess entity %s\n", STRING(pTarget->pev->classname));
			return;
		}
#endif

		pTarget->m_hGoalEnt = this;
		pTarget->m_hCine = this;
		pTarget->m_hTargetEnt = this;

		m_saved_movetype = pTarget->pev->movetype;
		m_saved_solid = pTarget->pev->solid;
		m_saved_effects = pTarget->pev->effects;
		pTarget->pev->effects |= pev->effects;

		switch (m_fMoveTo)
		{
		case 0:
			pTarget->m_scriptState = SCRIPT_WAIT;
			break;

		case 1:
			pTarget->m_scriptState = SCRIPT_WALK_TO_MARK;
			DelayStart(1);
			break;

		case 2:
			pTarget->m_scriptState = SCRIPT_RUN_TO_MARK;
			DelayStart(1);
			break;

		case 4:
			UTIL_SetOrigin(pTarget->pev, pev->origin);
			pTarget->pev->ideal_yaw = pev->angles.y;
			pTarget->pev->avelocity = Vector(0, 0, 0);
			pTarget->pev->velocity = Vector(0, 0, 0);
			pTarget->pev->effects |= EF_NOINTERP;
			pTarget->pev->angles.y = pev->angles.y;
			pTarget->m_scriptState = SCRIPT_WAIT;
			m_startTime = gpGlobals->time + 1E6;
			// UNDONE: Add a flag to do this so people can fixup physics after teleporting monsters
			//			pTarget->pev->flags &= ~FL_ONGROUND;
			break;
		}
		//		ALERT( at_aiconsole, "\"%s\" found and used (INT: %s)\n", STRING( pTarget->pev->targetname ), FBitSet(pev->spawnflags, SF_SCRIPT_NOINTERRUPT)?"No":"Yes" );

		pTarget->m_IdealMonsterState = MONSTERSTATE_SCRIPT;
		if (m_iszIdle)
		{
			StartSequence(pTarget, m_iszIdle, FALSE);
			if (FStrEq(STRING(m_iszIdle), STRING(m_iszPlay)))
			{
				pTarget->pev->framerate = 0;
			}
		}
	}
}


void CCineMonster::CineThink(void)
{
	if (FindEntity())
	{
		PossessEntity();
		ALERT(at_aiconsole, "script \"%s\" using monster \"%s\"\n", STRING(pev->targetname), STRING(m_iszEntity));
	}
	else
	{
		CancelScript();
		ALERT(at_aiconsole, "script \"%s\" can't find monster \"%s\"\n", STRING(pev->targetname), STRING(m_iszEntity));
		pev->nextthink = gpGlobals->time + 1.0;
	}
}


// lookup a sequence name and setup the target monster to play it
BOOL CCineMonster::StartSequence(CBaseMonster* pTarget, int iszSeq, BOOL completeOnEmpty)
{
	if (!iszSeq && completeOnEmpty)
	{
		SequenceDone(pTarget);
		return FALSE;
	}

	pTarget->pev->sequence = pTarget->LookupSequence(STRING(iszSeq));
	if (pTarget->pev->sequence == -1)
	{
		ALERT(at_warning, "%s: unknown scripted sequence \"%s\"\n", STRING(pTarget->pev->targetname), STRING(iszSeq));
		pTarget->pev->sequence = 0;
		// return FALSE;
	}

#if 0
	char* s;
	if (pev->spawnflags & SF_SCRIPT_NOINTERRUPT)
		s = "No";
	else
		s = "Yes";

	ALERT(at_console, "%s (%s): started \"%s\":INT:%s\n", STRING(pTarget->pev->targetname), STRING(pTarget->pev->classname), STRING(iszSeq), s);
#endif

	pTarget->pev->frame = 0;
	pTarget->ResetSequenceInfo();
	return TRUE;
}



//=========================================================
// SequenceDone - called when a scripted sequence animation
// sequence is done playing ( or when an AI Scripted Sequence
// doesn't supply an animation sequence to play ). Expects
// the CBaseMonster pointer to the monster that the sequence
// possesses. 
//=========================================================
void CCineMonster::SequenceDone(CBaseMonster* pMonster)
{
	//ALERT( at_aiconsole, "Sequence %s finished\n", STRING( m_pCine->m_iszPlay ) );

	if (!(pev->spawnflags & SF_SCRIPT_REPEATABLE))
	{
		SetThink(&CCineMonster::SUB_Remove);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	// This is done so that another sequence can take over the monster when triggered by the first

	pMonster->CineCleanup();

	FixScriptMonsterSchedule(pMonster);

	// This may cause a sequence to attempt to grab this guy NOW, so we have to clear him out
	// of the existing sequence
	SUB_UseTargets(NULL, USE_TOGGLE, 0);
}

//=========================================================
// When a monster finishes a scripted sequence, we have to 
// fix up its state and schedule for it to return to a 
// normal AI monster. 
//
// Scripted sequences just dirty the Schedule and drop the
// monster in Idle State.
//=========================================================
void CCineMonster::FixScriptMonsterSchedule(CBaseMonster* pMonster)
{
	if (pMonster->m_IdealMonsterState != MONSTERSTATE_DEAD)
		pMonster->m_IdealMonsterState = MONSTERSTATE_IDLE;
	pMonster->ClearSchedule();
}


void CCineMonster::AllowInterrupt(BOOL fAllow)
{
	if (pev->spawnflags & SF_SCRIPT_NOINTERRUPT)
		return;
	m_interruptable = fAllow;
}


BOOL CCineMonster::CanInterrupt(void)
{
	if (!m_interruptable)
		return FALSE;

	CBaseEntity* pTarget = m_hTargetEnt;

	if (pTarget != NULL && pTarget->pev->deadflag == DEAD_NO)
		return TRUE;

	return FALSE;
}


int	CCineMonster::IgnoreConditions(void)
{
	if (CanInterrupt())
		return 0;
	return SCRIPT_BREAK_CONDITIONS;
}


// find all the cinematic entities with my targetname and stop them from playing
void CCineMonster::CancelScript(void)
{
	ALERT(at_aiconsole, "Cancelling script: %s\n", STRING(m_iszPlay));

	if (!pev->targetname)
	{
		ScriptEntityCancel(edict());
		return;
	}

	CBaseEntity* pentCineTarget = UTIL_FindEntityByTargetname(NULL, STRING(pev->targetname));

	while (pentCineTarget)
	{
		ScriptEntityCancel(pentCineTarget->edict());
		pentCineTarget = UTIL_FindEntityByTargetname(pentCineTarget, STRING(pev->targetname));
	}
}


// find all the cinematic entities with my targetname and tell them to wait before starting
void CCineMonster::DelayStart(int state)
{
	CBaseEntity* pentCine = UTIL_FindEntityByTargetname(NULL, STRING(pev->targetname));

	while (pentCine)
	{
		if (FClassnameIs(pentCine->pev, "scripted_sequence"))
		{
			CCineMonster* pTarget = GetClassPtr((CCineMonster*)VARS(pentCine->pev));
			if (state)
			{
				//pTarget->m_iDelay++;
				pTarget->m_iDelay = 1;
			}
			else
			{
				// TODO: why was this done originally? seems like it was to allow multiple
				// monsters to use it at the same time, preventing the sequence playing until
				// they all arrive. But that isn't possible? they would need to somehow
				// occupy the same space. FindEntity only ever returns the first monster
				// found anyway?? iDelay is never reset if the monster is interrupted, so
				// the sequence breaks when treating this as a counter.
				//pTarget->m_iDelay--;
				//if (pTarget->m_iDelay <= 0)

				pTarget->m_iDelay = 0;
				pTarget->m_startTime = gpGlobals->time + 0.05;
			}
		}
		pentCine = UTIL_FindEntityByTargetname(pentCine, STRING(pev->targetname));
	}
}



// Find an entity that I'm interested in and precache the sounds he'll need in the sequence.
void CCineMonster::Activate(void)
{
	CBaseEntity* pentTarget;
	CBaseMonster* pTarget;

	// The entity name could be a target name or a classname
	// Check the targetname
	pentTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEntity));
	pTarget = NULL;

	while (!pTarget && pentTarget)
	{
		if (FBitSet(pentTarget->pev->flags, FL_MONSTER))
		{
			pTarget = pentTarget->MyMonsterPointer();
		}
		pentTarget = UTIL_FindEntityByTargetname(pentTarget, STRING(m_iszEntity));
	}

	// If no entity with that targetname, check the classname
	if (!pTarget)
	{
		pentTarget = UTIL_FindEntityByClassname(NULL, STRING(m_iszEntity));
		while (!pTarget && pentTarget)
		{
			pTarget = pentTarget->MyMonsterPointer();
			pentTarget = UTIL_FindEntityByTargetname(pentTarget, STRING(m_iszEntity));
		}
	}
	// Found a compatible entity
	if (pTarget)
	{
		void* pmodel;
		pmodel = GET_MODEL_PTR(pTarget->edict());
		if (pmodel)
		{
			// Look through the event list for stuff to precache
			SequencePrecache(pmodel, STRING(m_iszIdle));
			SequencePrecache(pmodel, STRING(m_iszPlay));
		}
	}
}
