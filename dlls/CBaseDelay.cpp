#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "nodes.h"
#include "CBaseDoor.h"

LINK_ENTITY_TO_CLASS(DelayedUse, CBaseDelay)

// Global Savedata for Delay
TYPEDESCRIPTION	CBaseDelay::m_SaveData[] =
{
	DEFINE_FIELD(CBaseDelay, m_flDelay, FIELD_FLOAT),
	DEFINE_FIELD(CBaseDelay, m_iszKillTarget, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CBaseDelay, CBaseEntity)

void CBaseDelay::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "delay"))
	{
		m_flDelay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "killtarget"))
	{
		m_iszKillTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue(pkvd);
	}
}

void CBaseDelay::SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value)
{
	//
	// exit immediatly if we don't have a target or kill target
	//
	if (FStringNull(pev->target) && !m_iszKillTarget)
		return;

	//
	// check for a delay
	//
	if (m_flDelay != 0)
	{
		FireTargetsDelayed(STRING(pev->target), m_iszKillTarget, pActivator, useType, m_flDelay);
		return;
	}

	//
	// kill the killtargets
	//

	if (m_iszKillTarget)
	{
		SUB_KillTarget(STRING(m_iszKillTarget));
	}

	//
	// fire targets
	//
	if (!FStringNull(pev->target))
	{
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
	}
}

void CBaseDelay::DelayThink(void)
{
	CBaseEntity* pActivator = NULL;

	if (pev->owner != NULL)		// A player activated this on delay
	{
		pActivator = CBaseEntity::Instance(pev->owner);
	}
	// The use type is cached (and stashed) in pev->button
	SUB_UseTargets(pActivator, (USE_TYPE)pev->button, 0);
	REMOVE_ENTITY(ENT(pev));
}
