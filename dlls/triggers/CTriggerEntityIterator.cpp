#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "CRuleEntity.h"
#include "CBaseLogic.h"

//
// CTriggerEntityIterator / trigger_entity_iterator -- fires targets using iterated entities as the activator

enum status_filter_types {
	FILTER_STATUS_NONE,
	FILTER_STATUS_LIVING,
	FILTER_STATUS_DEAD
};

enum run_modes {
	RUN_MODE_ONCE,
	RUN_MODE_ONCE_MULTITHREADED,
	RUN_MODE_TOGGLE,
	RUN_MODE_ONCE_MULTITHREADED_CHILD = 1337, // this entity is the child of a multithreaded iterator, and should die after running
};

class CTriggerEntityIterator : public CBaseLogic
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void Iterate();

	string_t m_name_filter;
	string_t m_classname_filter;
	string_t m_trigger_after_run;
	int m_status_filter;
	int m_triggerstate;
	int m_run_mode;
	int m_maximum_runs;
	float m_delay_between_triggers;
	float m_delay_between_runs;

	int m_nextIdx;
	int m_runCount;
	bool m_isRunning;
};

LINK_ENTITY_TO_CLASS(trigger_entity_iterator, CTriggerEntityIterator)

void CTriggerEntityIterator::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "name_filter"))
	{
		m_name_filter = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "classname_filter"))
	{
		m_classname_filter = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "trigger_after_run"))
	{
		m_trigger_after_run = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "status_filter"))
	{
		m_status_filter = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		m_triggerstate = atoi(pkvd->szValue);

		if (m_triggerstate < 0 || m_triggerstate >= 2) {
			m_triggerstate = USE_TOGGLE;
		}

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "run_mode"))
	{
		m_run_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "maximum_runs"))
	{
		m_maximum_runs = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "delay_between_triggers"))
	{
		m_delay_between_triggers = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "delay_between_runs"))
	{
		m_delay_between_runs = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CPointEntity::KeyValue(pkvd);
	}
}

void CTriggerEntityIterator::Spawn(void)
{
	CPointEntity::Spawn();
}

void CTriggerEntityIterator::Iterate() {
	const char* name_filter = m_name_filter ? STRING(m_name_filter) : NULL;
	const char* class_filter = m_classname_filter ? STRING(m_classname_filter) : NULL;
	const char* ent_target = STRING(pev->target);
	bool filterLiving = m_status_filter == FILTER_STATUS_LIVING;
	bool iterateAll = m_delay_between_triggers == 0;
	bool foundAnEnt = false;

	edict_t* edicts = ENT(0);

	std::vector<CBaseEntity*> targets;
	if (iterateAll) {
		// only find targets once since they shouldn't be changing mid-loop (faster).
		// It IS possible that new entities are created during the loop, but
		// proper handling for that would mean restarting the scan on every trigger
		// as well as keeping track of which entities have been iterated.
		targets = FindLogicEntities(ent_target);
	}

	int i;
	for (i = m_nextIdx; i < gpGlobals->maxEntities; i++)
	{
		if (edicts[i].free)
			continue;

		CBaseEntity* ent = CBaseEntity::Instance(&edicts[i]);
		if (!ent)
			continue;

		entvars_t& vars = edicts[i].v;

		if (m_status_filter && filterLiving != (vars.deadflag == DEAD_NO)) {
			continue;
		}
		if (name_filter && strcmp(name_filter, STRING(vars.targetname))) {
			continue;
		}
		if (class_filter && strcmp(class_filter, STRING(vars.classname))) {
			continue;
		}

		if (iterateAll) {
			for (CBaseEntity* target : targets) {
				target->Use(ent, this, (USE_TYPE)m_triggerstate, 0.0f);
			}
		}
		else {
			if (!iterateAll && foundAnEnt) {
				// make sure there is another entity to use next time to prevent an Iterate() call doing nothing
				m_nextIdx = i;
				break;
			}

			FireTargets(ent_target, ent, this, (USE_TYPE)m_triggerstate, 0.0f);
			foundAnEnt = true;
		}
	}

	bool finishedRun = i == gpGlobals->maxEntities;

	if (finishedRun) {
		if (m_trigger_after_run) {
			FireTargets(STRING(m_trigger_after_run), h_activator, this, USE_TOGGLE, 0.0f);
		}

		if (m_run_mode == RUN_MODE_ONCE_MULTITHREADED_CHILD) {
			SetThink(NULL);
			pev->nextthink = 0;
			UTIL_Remove(this);
			return;
		}

		if (m_run_mode == RUN_MODE_TOGGLE && (!m_maximum_runs || ++m_runCount < m_maximum_runs)) {
			m_nextIdx = 1;
			pev->nextthink = gpGlobals->time + m_delay_between_runs;
			SetThink(&CTriggerEntityIterator::Iterate);
		}
		else {
			m_isRunning = false;
			SetThink(NULL);
			pev->nextthink = 0;
		}
	}
	else {
		m_isRunning = true;
		SetThink(&CTriggerEntityIterator::Iterate);
		pev->nextthink = gpGlobals->time + m_delay_between_triggers;
	}
}

void CTriggerEntityIterator::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	h_activator = pActivator;

	bool shouldToggleOff = (m_isRunning && useType != USE_ON) || useType == USE_OFF;
	if (shouldToggleOff && m_run_mode != RUN_MODE_ONCE_MULTITHREADED_CHILD) {
		m_isRunning = false;
		SetThink(NULL);
		pev->nextthink = 0;
		return;
	}

	if (!pev->target) {
		ALERT(at_console, "trigger_entity_iterator: no target specified\n");
		return;
	}

	if (m_run_mode == RUN_MODE_ONCE_MULTITHREADED) {
		CTriggerEntityIterator* child = (CTriggerEntityIterator*)CBaseEntity::Create(
			"trigger_entity_iterator", pev->origin, pev->angles);

		if (child) {
			child->pev->target = pev->target;
			child->m_name_filter = m_name_filter;
			child->m_classname_filter = m_classname_filter;
			child->m_trigger_after_run = m_trigger_after_run;
			child->m_status_filter = m_status_filter;
			child->m_triggerstate = m_triggerstate;
			child->m_delay_between_triggers = m_delay_between_triggers;
			child->m_run_mode = RUN_MODE_ONCE_MULTITHREADED_CHILD;

			child->Use(pActivator, pCaller, USE_ON, 0.0f);
		}
	}
	else {
		m_runCount = 0;
		m_nextIdx = 1;
		Iterate();
	}
}
