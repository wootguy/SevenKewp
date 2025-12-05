#include "extdll.h"
#include "util.h"
#include "CBaseLogic.h"

//
// CTriggerChangeClass / trigger_change_class -- changes an entities classification

class CTriggerChangeClass : public CBaseLogic
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	int m_newclass;
};

LINK_ENTITY_TO_CLASS(trigger_change_class, CTriggerChangeClass)

void CTriggerChangeClass::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "classify"))
	{
		m_newclass = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CBaseLogic::KeyValue(pkvd);
	}
}

void CTriggerChangeClass::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pev->target) {
		return;
	}

	std::vector<CBaseEntity*> targets = FindLogicEntities(STRING(pev->target));

	for (CBaseEntity* ent : targets) {
		ent->SetClassification(m_newclass);
	}
}
