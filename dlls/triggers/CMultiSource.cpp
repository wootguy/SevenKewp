#include "extdll.h"
#include "util.h"
#include "CMultiSource.h"
#include "saverestore.h"

TYPEDESCRIPTION CMultiSource::m_SaveData[] =
{
	//!!!BUGBUG FIX
	DEFINE_ARRAY(CMultiSource, m_rgEntities, FIELD_EHANDLE, MS_MAX_TARGETS),
	DEFINE_ARRAY(CMultiSource, m_rgTriggered, FIELD_INTEGER, MS_MAX_TARGETS),
	DEFINE_FIELD(CMultiSource, m_iTotal, FIELD_INTEGER),
	DEFINE_FIELD(CMultiSource, m_globalstate, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CMultiSource, CBaseEntity)

LINK_ENTITY_TO_CLASS(multisource, CMultiSource)
//
// Cache user-entity-field values until spawn is called.
//

void CMultiSource::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "style") ||
		FStrEq(pkvd->szKeyName, "height") ||
		FStrEq(pkvd->szKeyName, "killtarget") ||
		FStrEq(pkvd->szKeyName, "value1") ||
		FStrEq(pkvd->szKeyName, "value2") ||
		FStrEq(pkvd->szKeyName, "value3"))
		pkvd->fHandled = TRUE;
	else if (FStrEq(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

#define SF_MULTI_INIT		1

void CMultiSource::Spawn()
{
	// set up think for later registration

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->nextthink = gpGlobals->time + 0.1;
	pev->spawnflags |= SF_MULTI_INIT;	// Until it's initialized
	SetThink(&CMultiSource::Register);
}

void CMultiSource::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!m_registered) {
		// if a trigger_auto targets a multisource with 0 delay, then the multisource may not have
		// registered its callers yet, depending on entity initialization order
		Register();
	}

	int i = 0;

	// Find the entity in our list
	while (i < m_iTotal)
		if (m_rgEntities[i++] == pCaller)
			break;

	// if we didn't find it, report error and leave
	if (i > m_iTotal)
	{
		ALERT(at_console, "MultiSrc:Used by non member %s.\n", STRING(pCaller->pev->classname));
		return;
	}

	// CONSIDER: a Use input to the multisource always toggles.  Could check useType for ON/OFF/TOGGLE

	if (i > 0)
		m_rgTriggered[i - 1] ^= 1;

	// 
	if (IsTriggered(pActivator))
	{
		ALERT(at_aiconsole, "Multisource %s enabled (%d inputs)\n", STRING(pev->targetname), m_iTotal);
		USE_TYPE outUseType = USE_TOGGLE;
		if (m_globalstate)
			outUseType = USE_ON;
		SUB_UseTargets(NULL, outUseType, 0);
	}
}


BOOL CMultiSource::IsTriggered(CBaseEntity*)
{
	// Is everything triggered?
	int i = 0;

	// Still initializing?
	if (pev->spawnflags & SF_MULTI_INIT)
		return 0;

	while (i < m_iTotal)
	{
		if (m_rgTriggered[i] == 0)
			break;
		i++;
	}

	if (i == m_iTotal)
	{
		if (!m_globalstate || gGlobalState.EntityGetState(m_globalstate) == GLOBAL_ON)
			return 1;
	}

	return 0;
}

void CMultiSource::Register(void)
{
	CBaseEntity* pentTarget = NULL;

	m_iTotal = 0;
	memset(m_rgEntities, 0, MS_MAX_TARGETS * sizeof(EHANDLE));

	SetThink(&CMultiSource::SUB_DoNothing);

	// search for all entities which target this multisource (pev->targetname)

	edict_t* edicts = ENT(0);
	for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities && m_iTotal < MS_MAX_TARGETS; i++)
	{
		if (edicts[i].free)
			continue;

		CBaseEntity* ent = CBaseEntity::Instance(&edicts[i].v);

		if (ent && ent->HasTarget(pev->targetname)) {
			m_rgEntities[m_iTotal++] = ent;
		}
	}

	pev->spawnflags &= ~SF_MULTI_INIT;
	
	m_registered = true;
}
