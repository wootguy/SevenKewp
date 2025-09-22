#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"

class CTriggerAutoSave : public CBaseTrigger
{
public:
	void Spawn(void);
	void EXPORT SaveTouch(CBaseEntity* pOther);
};
LINK_ENTITY_TO_CLASS(trigger_autosave, CTriggerAutoSave)

void CTriggerAutoSave::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	if (g_pGameRules->IsDeathmatch())
	{
		UTIL_Remove(this);
		return;
	}

	InitTrigger();
	SetTouch(&CTriggerAutoSave::SaveTouch);
}

void CTriggerAutoSave::SaveTouch(CBaseEntity* pOther)
{
	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	// Only save on clients
	if (!pOther->IsPlayer())
		return;

	SetTouch(NULL);
	UTIL_Remove(this);
	SERVER_COMMAND("autosave\n");
}
