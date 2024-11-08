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

	edict_t* ent = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target));
	
	if (!FNullEnt(ent)) {
		ent->v.skin = pev->skin;
		SET_MODEL(ent, STRING(pev->model));
	}
}
