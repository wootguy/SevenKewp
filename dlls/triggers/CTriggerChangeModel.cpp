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

	CBaseEntity* ent = NULL;
	while (ent = FindLogicEntity(ent, pev->target)) {
		CBasePlayer* plr = ent->MyPlayerPointer();

		if (plr) {
			char temp[32];
			const char* pmodel = STRING(pev->model);
			if (strstr(pmodel, "models/player/") != pmodel)
				return;

			pmodel += strlen("models/player/");

			memset(temp, 0, sizeof(temp));
			strcpy_safe(temp, pmodel, 32);

			for (int i = 0; i < 32; i++) {
				if (temp[i] == '/') {
					temp[i] = 0;
					break;
				}
			}

			plr->SetMapPlayerModel(temp);
		}
		else {
			ent->pev->skin = pev->skin;
			SET_MODEL(ent->edict(), STRING(pev->model));
		}
	}
}
