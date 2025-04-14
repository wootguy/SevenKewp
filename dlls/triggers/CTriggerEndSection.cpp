#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"

#define SF_ENDSECTION_USEONLY		0x0001

class CTriggerEndSection : public CBaseTrigger
{
public:
	void Spawn(void);
	void EXPORT EndSectionTouch(CBaseEntity* pOther);
	void KeyValue(KeyValueData* pkvd);
	void EXPORT EndSectionUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};
LINK_ENTITY_TO_CLASS(trigger_endsection, CTriggerEndSection)


void CTriggerEndSection::EndSectionUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Only save on clients
	if (pActivator && !pActivator->IsNetClient())
		return;

	SetUse(NULL);

	if (pev->message)
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove(this);
}

void CTriggerEndSection::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	if (g_pGameRules->IsDeathmatch())
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	InitTrigger();

	SetUse(&CTriggerEndSection::EndSectionUse);
	// If it is a "use only" trigger, then don't set the touch function.
	if (!(pev->spawnflags & SF_ENDSECTION_USEONLY))
		SetTouch(&CTriggerEndSection::EndSectionTouch);
}

void CTriggerEndSection::EndSectionTouch(CBaseEntity* pOther)
{
	// Only save on clients
	if (!pOther->IsNetClient())
		return;

	SetTouch(NULL);

	if (pev->message)
	{
		g_engfuncs.pfnEndSection(STRING(pev->message));
	}
	UTIL_Remove(this);
}

void CTriggerEndSection::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "section"))
	{
		//		m_iszSectionName = ALLOC_STRING( pkvd->szValue );
				// Store this in message so we don't have to write save/restore for this ent
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseTrigger::KeyValue(pkvd);
}
