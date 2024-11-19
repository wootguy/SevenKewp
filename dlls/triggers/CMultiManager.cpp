#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"

//**********************************************************
// The Multimanager Entity - when fired, will fire up to 16 targets 
// at specified times.
// FLAG:		THREAD (create clones when triggered)
// FLAG:		CLONE (this is a clone for a threaded execution)

#define SF_MULTIMAN_CLONE		0x80000000
#define SF_MULTIMAN_THREAD		0x00000001

class CMultiManager : public CBaseToggle
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void KeyValue(KeyValueData* pkvd);
	void Spawn(void);
	void EXPORT ManagerThink(void);
	void EXPORT ManagerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

#if _DEBUG
	void EXPORT ManagerReport(void);
#endif

	BOOL		HasTarget(string_t targetname);

	int ObjectCaps(void) { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	int		m_cTargets;	// the total number of targets in this manager's fire list.
	int		m_index;	// Current target
	float	m_startTime;// Time we started firing
	int		m_iTargetName[MAX_MULTI_TARGETS];// list if indexes into global string array
	float	m_flTargetDelay[MAX_MULTI_TARGETS];// delay (in seconds) from time of manager fire to target fire
	uint8_t	m_triggerMode[MAX_MULTI_TARGETS];// trigger mode for the target (on/off/toggle)
private:
	inline BOOL IsClone(void) { return (pev->spawnflags & SF_MULTIMAN_CLONE) ? TRUE : FALSE; }
	inline BOOL ShouldClone(void)
	{
		if (IsClone())
			return FALSE;

		return (pev->spawnflags & SF_MULTIMAN_THREAD) ? TRUE : FALSE;
	}

	CMultiManager* Clone(void);
};
LINK_ENTITY_TO_CLASS(multi_manager, CMultiManager)

// Global Savedata for multi_manager
TYPEDESCRIPTION	CMultiManager::m_SaveData[] =
{
	DEFINE_FIELD(CMultiManager, m_cTargets, FIELD_INTEGER),
	DEFINE_FIELD(CMultiManager, m_index, FIELD_INTEGER),
	DEFINE_FIELD(CMultiManager, m_startTime, FIELD_TIME),
	DEFINE_ARRAY(CMultiManager, m_iTargetName, FIELD_STRING, MAX_MULTI_TARGETS),
	DEFINE_ARRAY(CMultiManager, m_flTargetDelay, FIELD_FLOAT, MAX_MULTI_TARGETS),
};

IMPLEMENT_SAVERESTORE(CMultiManager, CBaseToggle)

void CMultiManager::KeyValue(KeyValueData* pkvd)
{
	// UNDONE: Maybe this should do something like this:
	//CBaseToggle::KeyValue( pkvd );
	// if ( !pkvd->fHandled )
	// ... etc.

	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else // add this field to the target list
	{
		// this assumes that additional fields are targetnames and their values are delay values.
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			char tmp[128];

			int triggerMode = USE_TOGGLE;
			const char* triggerTypeSep = strstr(pkvd->szValue, "#");
			if (triggerTypeSep && triggerTypeSep != pkvd->szValue + strlen(pkvd->szValue)) {
				triggerMode = atoi(triggerTypeSep + 1);
			}

			UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));
			m_iTargetName[m_cTargets] = ALLOC_STRING(tmp);
			m_flTargetDelay[m_cTargets] = atof(pkvd->szValue);
			m_triggerMode[m_cTargets] = triggerMode;
			m_cTargets++;
			pkvd->fHandled = TRUE;
		}
	}
}


void CMultiManager::Spawn(void)
{
	pev->solid = SOLID_NOT;
	SetUse(&CMultiManager::ManagerUse);
	SetThink(&CMultiManager::ManagerThink);

	// Sort targets
	// Quick and dirty bubble sort
	int swapped = 1;

	while (swapped)
	{
		swapped = 0;
		for (int i = 1; i < m_cTargets; i++)
		{
			if (m_flTargetDelay[i] < m_flTargetDelay[i - 1])
			{
				// Swap out of order elements
				int name = m_iTargetName[i];
				float delay = m_flTargetDelay[i];
				int mode = m_triggerMode[i];
				m_iTargetName[i] = m_iTargetName[i - 1];
				m_flTargetDelay[i] = m_flTargetDelay[i - 1];
				m_triggerMode[i] = m_triggerMode[i - 1];
				m_iTargetName[i - 1] = name;
				m_flTargetDelay[i - 1] = delay;
				m_triggerMode[i - 1] = mode;
				swapped = 1;
			}
		}
	}
}


BOOL CMultiManager::HasTarget(string_t targetname)
{
	for (int i = 0; i < m_cTargets; i++)
		if (FStrEq(STRING(targetname), STRING(m_iTargetName[i])))
			return TRUE;

	return FALSE;
}


// Designers were using this to fire targets that may or may not exist -- 
// so I changed it to use the standard target fire code, made it a little simpler.
void CMultiManager::ManagerThink(void)
{
	float	time;

	time = gpGlobals->time - m_startTime;
	while (m_index < m_cTargets && m_flTargetDelay[m_index] <= time)
	{
		const char* targetName = STRING(m_iTargetName[m_index]);

		if (m_triggerMode[m_index] == 2) { // USE_SET(2) means KILL in this context
			edict_t* pentKillTarget = NULL;
			while (!FNullEnt(pentKillTarget = FIND_ENTITY_BY_TARGETNAME(pentKillTarget, targetName))) {
				UTIL_Remove(CBaseEntity::Instance(pentKillTarget));
			}
		}
		else {
			FireTargets(targetName, m_hActivator, this, (USE_TYPE)m_triggerMode[m_index], 0);
		}

		m_index++;
	}

	if (m_index >= m_cTargets)// have we fired all targets?
	{
		SetThink(NULL);
		if (IsClone())
		{
			UTIL_Remove(this);
			return;
		}
		SetUse(&CMultiManager::ManagerUse);// allow manager re-use 
	}
	else
		pev->nextthink = m_startTime + m_flTargetDelay[m_index];
}

CMultiManager* CMultiManager::Clone(void)
{
	CMultiManager* pMulti = GetClassPtr((CMultiManager*)NULL);

	edict_t* pEdict = pMulti->pev->pContainingEntity;
	memcpy(pMulti->pev, pev, sizeof(*pev));
	pMulti->pev->pContainingEntity = pEdict;

	pMulti->pev->spawnflags |= SF_MULTIMAN_CLONE;
	pMulti->m_cTargets = m_cTargets;
	memcpy(pMulti->m_iTargetName, m_iTargetName, sizeof(m_iTargetName));
	memcpy(pMulti->m_flTargetDelay, m_flTargetDelay, sizeof(m_flTargetDelay));
	memcpy(pMulti->m_triggerMode, m_triggerMode, sizeof(m_triggerMode));

	return pMulti;
}


// The USE function builds the time table and starts the entity thinking.
void CMultiManager::ManagerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// In multiplayer games, clone the MM and execute in the clone (like a thread)
	// to allow multiple players to trigger the same multimanager
	if (ShouldClone())
	{
		CMultiManager* pClone = Clone();
		pClone->ManagerUse(pActivator, pCaller, useType, value);
		return;
	}

	m_hActivator = pActivator;
	m_index = 0;
	m_startTime = gpGlobals->time;

	SetUse(NULL);// disable use until all targets have fired

	SetThink(&CMultiManager::ManagerThink);
	pev->nextthink = gpGlobals->time;
}

#if _DEBUG
void CMultiManager::ManagerReport(void)
{
	int	cIndex;

	for (cIndex = 0; cIndex < m_cTargets; cIndex++)
	{
		ALERT(at_console, "%s %f %d\n", STRING(m_iTargetName[cIndex]), m_flTargetDelay[cIndex], m_triggerMode[cIndex]);
	}
}
#endif
