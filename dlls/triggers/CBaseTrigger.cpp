#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"
#include "te_effects.h"

LINK_ENTITY_TO_CLASS(trigger, CBaseTrigger)

/*
================
InitTrigger
================
*/
void CBaseTrigger::InitTrigger()
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	// trigger angles are used for one-way touches.  An angle of 0 is assumed
	// to mean no restrictions, so use a yaw of 360 instead.
	if (pev->angles != g_vecZero)
		SetMovedir(pev);
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model));    // set size and link into world
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		SetBits(pev->effects, EF_NODRAW);
}


//
// Cache user-entity-field values until spawn is called.
//

void CBaseTrigger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "count"))
	{
		m_cTriggersLeft = (int)atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damagetype"))
	{
		m_bitsDamageInflict = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}


void CBaseTrigger::MultiTouch(CBaseEntity* pOther)
{
	m_testTouch = true;

	entvars_t* pevToucher;

	pevToucher = pOther->pev;

	// Only touch clients, monsters, or pushables (depending on flags)
	if (((pevToucher->flags & FL_CLIENT) && !(pev->spawnflags & SF_TRIGGER_NOCLIENTS)) ||
		((pevToucher->flags & FL_MONSTER) && (pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS)) ||
		((pev->spawnflags & SF_TRIGGER_PUSHABLES) && FClassnameIs(pevToucher, "func_pushable")) )
	{

#if 0
		// if the trigger has an angles field, check player's facing direction
		if (pev->movedir != g_vecZero)
		{
			UTIL_MakeVectors(pevToucher->angles);
			if (DotProduct(gpGlobals->v_forward, pev->movedir) < 0)
				return;         // not facing the right way
		}
#endif

		if (!RunInventoryRules(pOther)) {
			return;
		}

		ActivateMultiTrigger(pOther, false);
	}
}


//
// the trigger was just touched/killed/used
// self.enemy should be set to the activator so it can be held through a delay
// so wait for the delay time before firing
//
void CBaseTrigger::ActivateMultiTrigger(CBaseEntity* pActivator, bool isUntouch)
{
	if (isUntouch) {
		if (!(pev->spawnflags & SF_TRIGGER_FIRE_ON_EXIT)) {
			return;
		}

		//ALERT(at_console, "EXIT TOUCH BY %s\n", pActivator->DisplayName());
	}
	else if (pev->spawnflags & (SF_TRIGGER_FIRE_ON_ENTER | SF_TRIGGER_FIRE_ON_EXIT)) {
		int emptySlot = -1;

		for (int i = 0; i < MAX_TOUCHERS; i++) {
			if (m_touchers[i].GetEntity() == pActivator) {
				return;
			}
			else if (emptySlot == -1 && !m_touchers[i]) {
				emptySlot = i;
			}
		}

		if (emptySlot == -1) {
			ALERT(at_error, "%s Exceeded max trigger touch trackers\n", STRING(pev->classname));
			return;
		}

		//ALERT(at_console, "ENTER TOUCH BY %s\n", pActivator->DisplayName());
		m_touchers[emptySlot] = pActivator;

		if (!(pev->spawnflags & SF_TRIGGER_FIRE_ON_ENTER)) {
			return;
		}
	}
	else if (m_nextTouch > gpGlobals->time) {
		return;         // still waiting for reset time
	}
	
	if (!UTIL_IsMasterTriggered(m_sMaster, pActivator))
		return;

	if (FClassnameIs(pev, "trigger_secret"))
	{
		if (pev->enemy == NULL || !FClassnameIs(pev->enemy, "player"))
			return;
		gpGlobals->found_secrets++;
	}

	if (!FStringNull(pev->noise)) {
		UTIL_TempSound(pev->origin, STRING(pev->noise));
	}

	// don't trigger again until reset
	// pev->takedamage = DAMAGE_NO;

	m_hActivator = pActivator;
	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);

	if (pev->message && pActivator->IsPlayer())
	{
		UTIL_ShowMessage(STRING(pev->message), pActivator);
		//		CLIENT_PRINTF( ENT( pActivator->pev ), print_center, STRING(pev->message) );
	}

	if (m_flWait > 0)
	{
		//SetThink(&CBaseTrigger::MultiWaitOver); // replaced by sv_forceretouch in rehlds
		m_nextTouch = gpGlobals->time + m_flWait;
	}
	else
	{
		// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		SetTouch(NULL);
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CBaseTrigger::SUB_Remove);
	}
}


/*
// the wait time has passed, so set back up for another activation
void CBaseTrigger::MultiWaitOver(void)
{
	//	if (pev->max_health)
	//		{
	//		pev->health		= pev->max_health;
	//		pev->takedamage	= DAMAGE_YES;
	//		pev->solid		= SOLID_BBOX;
	//		}
	SetThink(NULL);

	// retouch to check if stationary entity is touching the trigger
	gpGlobals->force_retouch++;
}
*/

// ========================= COUNTING TRIGGER =====================================

//
// GLOBALS ASSUMED SET:  g_eoActivator
//
void CBaseTrigger::CounterUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_cTriggersLeft--;
	m_hActivator = pActivator;

	if (m_cTriggersLeft < 0)
		return;

	BOOL fTellActivator =
		(m_hActivator != 0) &&
		FClassnameIs(m_hActivator->pev, "player") &&
		!FBitSet(pev->spawnflags, SPAWNFLAG_NOMESSAGE);
	if (m_cTriggersLeft != 0)
	{
		if (fTellActivator)
		{
			// UNDONE: I don't think we want these Quakesque messages
			switch (m_cTriggersLeft)
			{
			case 1:		ALERT(at_console, "Only 1 more to go...");		break;
			case 2:		ALERT(at_console, "Only 2 more to go...");		break;
			case 3:		ALERT(at_console, "Only 3 more to go...");		break;
			default:	ALERT(at_console, "There are more to go...");	break;
			}
		}
		return;
	}

	// !!!UNDONE: I don't think we want these Quakesque messages
	if (fTellActivator)
		ALERT(at_console, "Sequence completed!");

	ActivateMultiTrigger(m_hActivator, false);
}

//
// ToggleUse - If this is the USE function for a trigger, its state will toggle every time it's fired
//
void CBaseTrigger::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->solid == SOLID_NOT)
	{// if the trigger is off, turn it on
		pev->solid = SOLID_TRIGGER;

		// Force retouch
		gpGlobals->force_retouch++;
	}
	else
	{// turn the trigger off
		pev->solid = SOLID_NOT;
	}
	UTIL_SetOrigin(pev, pev->origin);
}

void CBaseTrigger::UntouchThink() {
	for (int i = 0; i < MAX_TOUCHERS; i++) {
		if (!m_touchers[i]) {
			continue;
		}
		
		m_testTouch = false;
		UTIL_ForceRetouch(m_touchers[i].GetEdict());

		if (!m_testTouch) {
			// trigger no longer touched
			ActivateMultiTrigger(m_touchers[i], true);
			m_touchers[i] = NULL;
		}
	}

	pev->nextthink = gpGlobals->time + 0.01f;
}