#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"

// this is a really bad idea.
class CTriggerChangeTarget : public CBaseDelay
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Spawn(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	int ObjectCaps(void) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

private:
	int		m_iszNewTarget;
};
LINK_ENTITY_TO_CLASS(trigger_changetarget, CTriggerChangeTarget);

TYPEDESCRIPTION	CTriggerChangeTarget::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerChangeTarget, m_iszNewTarget, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeTarget, CBaseDelay);

void CTriggerChangeTarget::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszNewTarget"))
	{
		m_iszNewTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerChangeTarget::Spawn(void)
{
}


void CTriggerChangeTarget::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pTarget = UTIL_FindEntityByString(NULL, "targetname", STRING(pev->target));

	if (pTarget)
	{
		pTarget->pev->target = m_iszNewTarget;
		CBaseMonster* pMonster = pTarget->MyMonsterPointer();
		if (pMonster)
		{
			pMonster->m_pGoalEnt = NULL;
		}
	}
}

