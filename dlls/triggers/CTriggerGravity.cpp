#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"

class CTriggerGravity : public CBaseTrigger
{
public:
	void Spawn(void);
	void EXPORT GravityTouch(CBaseEntity* pOther);
};
LINK_ENTITY_TO_CLASS(trigger_gravity, CTriggerGravity)

void CTriggerGravity::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	InitTrigger();
	SetTouch(&CTriggerGravity::GravityTouch);
}

void CTriggerGravity::GravityTouch(CBaseEntity* pOther)
{
	// Only save on clients
	if (!pOther->IsPlayer())
		return;

	CBasePlayer* plr = (CBasePlayer*)pOther;

	plr->m_gravity_modifier = pev->gravity;
	plr->ApplyEffects();
}

