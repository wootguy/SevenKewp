#include "extdll.h"
#include "util.h"
#include "saverestore.h"

#define SF_GLOBAL_SET			1	// Set global state to initial state on spawn

class CEnvGlobal : public CPointEntity
{
public:
	void	Spawn(void);
	void	KeyValue(KeyValueData* pkvd);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	string_t	m_globalstate;
	int			m_triggermode;
	int			m_initialstate;
};

TYPEDESCRIPTION CEnvGlobal::m_SaveData[] =
{
	DEFINE_FIELD(CEnvGlobal, m_globalstate, FIELD_STRING),
	DEFINE_FIELD(CEnvGlobal, m_triggermode, FIELD_INTEGER),
	DEFINE_FIELD(CEnvGlobal, m_initialstate, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CEnvGlobal, CBaseEntity)

LINK_ENTITY_TO_CLASS(env_global, CEnvGlobal)

void CEnvGlobal::KeyValue(KeyValueData* pkvd)
{
	pkvd->fHandled = TRUE;

	if (FStrEq(pkvd->szKeyName, "globalstate"))		// State name
		m_globalstate = ALLOC_STRING(pkvd->szValue);
	else if (FStrEq(pkvd->szKeyName, "triggermode"))
		m_triggermode = atoi(pkvd->szValue);
	else if (FStrEq(pkvd->szKeyName, "initialstate"))
		m_initialstate = atoi(pkvd->szValue);
	else
		CPointEntity::KeyValue(pkvd);
}

void CEnvGlobal::Spawn(void)
{
	if (!m_globalstate)
	{
		REMOVE_ENTITY(ENT(pev));
		return;
	}
	if (FBitSet(pev->spawnflags, SF_GLOBAL_SET))
	{
		if (!gGlobalState.EntityInTable(m_globalstate))
			gGlobalState.EntityAdd(m_globalstate, gpGlobals->mapname, (GLOBALESTATE)m_initialstate);
	}
}


void CEnvGlobal::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	GLOBALESTATE oldState = gGlobalState.EntityGetState(m_globalstate);
	GLOBALESTATE newState;

	switch (m_triggermode)
	{
	case 0:
		newState = GLOBAL_OFF;
		break;

	case 1:
		newState = GLOBAL_ON;
		break;

	case 2:
		newState = GLOBAL_DEAD;
		break;

	default:
	case 3:
		if (oldState == GLOBAL_ON)
			newState = GLOBAL_OFF;
		else if (oldState == GLOBAL_OFF)
			newState = GLOBAL_ON;
		else
			newState = oldState;
	}

	if (gGlobalState.EntityInTable(m_globalstate))
		gGlobalState.EntitySetState(m_globalstate, newState);
	else
		gGlobalState.EntityAdd(m_globalstate, gpGlobals->mapname, newState);
}

