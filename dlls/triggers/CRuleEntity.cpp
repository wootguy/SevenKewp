#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "CRuleEntity.h"

TYPEDESCRIPTION	CRuleEntity::m_SaveData[] =
{
	DEFINE_FIELD(CRuleEntity, m_iszMaster, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CRuleEntity, CBaseEntity)


void CRuleEntity::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = EF_NODRAW;
}

void CRuleEntity::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		SetMaster(ALLOC_STRING(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

BOOL CRuleEntity::CanFireForActivator(CBaseEntity* pActivator)
{
	if (m_iszMaster)
	{
		if (UTIL_IsMasterTriggered(m_iszMaster, pActivator))
			return TRUE;
		else
			return FALSE;
	}

	return TRUE;
}

void CRulePointEntity::Spawn(void)
{
	CRuleEntity::Spawn();
	pev->frame = 0;
	pev->model = 0;
}

void CRuleBrushEntity::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	SET_MODEL(edict(), STRING(pev->model));
	CRuleEntity::Spawn();
}
