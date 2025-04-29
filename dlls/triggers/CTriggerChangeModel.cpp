#include "extdll.h"
#include "util.h"

//
// CTriggerChangeModel / trigger_changemodel -- change and entity's model

class CTriggerChangeModel : public CPointEntity
{
public:
	void Spawn(void);
	void Precache(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(trigger_changemodel, CTriggerChangeModel)

void CTriggerChangeModel::Spawn(void) {
	CPointEntity::Spawn();
	Precache();
}

void CTriggerChangeModel::Precache() {
	if (pev->model) {
		PRECACHE_MODEL(STRING(pev->model));
	}
}

void CTriggerChangeModel::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pev->target || !pev->model) {
		return;
	}

	CBaseEntity* ent = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
	
	if (ent) {
		ent->pev->skin = pev->skin;
		SET_MODEL(ent->edict(), STRING(pev->model));
	}
}
