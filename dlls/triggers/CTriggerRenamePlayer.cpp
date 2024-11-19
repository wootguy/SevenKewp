#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "CRuleEntity.h"

//
// CTriggerRenamePlayer / trigger_renameplayer -- sets a targetname on the triggering player

#define SF_RPLR_REUSABLE 1

class CTriggerRenamePlayer : public CPointEntity
{
public:
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(trigger_renameplayer, CTriggerRenamePlayer)

void CTriggerRenamePlayer::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pActivator && pActivator->IsPlayer()) {
		pActivator->pev->targetname = pev->netname;

		if (!(pev->spawnflags & SF_RPLR_REUSABLE)) {
			REMOVE_ENTITY(edict());
		}
	}
}
