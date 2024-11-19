#include "extdll.h"
#include "util.h"
#include "CBaseDelay.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"

// This trigger will fire when the level spawns (or respawns if not fire once)
// It will check a global state before firing.  It supports delay and killtargets

#define SF_AUTO_FIREONCE		0x0001

class CAutoTrigger : public CBaseDelay
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Spawn(void);
	void Precache(void);
	void Think(void);

	int ObjectCaps(void) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

private:
	int			m_globalstate;
	USE_TYPE	triggerType;
};
LINK_ENTITY_TO_CLASS(trigger_auto, CAutoTrigger)

TYPEDESCRIPTION	CAutoTrigger::m_SaveData[] =
{
	DEFINE_FIELD(CAutoTrigger, m_globalstate, FIELD_STRING),
	DEFINE_FIELD(CAutoTrigger, triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CAutoTrigger, CBaseDelay)

void CAutoTrigger::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);
		switch (type)
		{
		case 0:
			triggerType = USE_OFF;
			break;
		case 2:
			triggerType = USE_TOGGLE;
			break;
		default:
			triggerType = USE_ON;
			break;
		}
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}


void CAutoTrigger::Spawn(void)
{
	Precache();
}


void CAutoTrigger::Precache(void)
{
	pev->nextthink = gpGlobals->time + 0.1;
}


void CAutoTrigger::Think(void)
{
	if (!m_globalstate || gGlobalState.EntityGetState(m_globalstate) == GLOBAL_ON)
	{
		SUB_UseTargets(this, triggerType, 0);
		if (pev->spawnflags & SF_AUTO_FIREONCE)
			UTIL_Remove(this);
	}
}
