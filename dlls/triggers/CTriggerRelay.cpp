#include "extdll.h"
#include "util.h"
#include "cbase.h"

#define SF_RELAY_FIREONCE		0x0001

class CTriggerRelay : public CBaseDelay
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
	USE_TYPE	triggerType;
};

LINK_ENTITY_TO_CLASS(trigger_relay, CTriggerRelay)

TYPEDESCRIPTION	CTriggerRelay::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerRelay, triggerType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTriggerRelay, CBaseDelay)

void CTriggerRelay::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "triggerstate"))
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


void CTriggerRelay::Spawn(void)
{
}

void CTriggerRelay::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	SUB_UseTargets(this, triggerType, 0);
	if (pev->spawnflags & SF_RELAY_FIREONCE)
		UTIL_Remove(this);
}
