#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "decals.h"
#include "CBasePlayer.h"

CGlobalState					gGlobalState;

CGlobalState::CGlobalState(void)
{
	Reset();
}

void CGlobalState::Reset(void)
{
	m_pList = NULL;
	m_listCount = 0;
}

globalentity_t* CGlobalState::Find(string_t globalname)
{
	if (!globalname)
		return NULL;

	globalentity_t* pTest;
	const char* pEntityName = STRING(globalname);


	pTest = m_pList;
	while (pTest)
	{
		if (FStrEq(pEntityName, pTest->name))
			break;

		pTest = pTest->pNext;
	}

	return pTest;
}


// This is available all the time now on impulse 104, remove later
//#ifdef _DEBUG
void CGlobalState::DumpGlobals(void)
{
	static const char* estates[] = { "Off", "On", "Dead" };
	globalentity_t* pTest;

	ALERT(at_console, "-- Globals --\n");
	pTest = m_pList;
	while (pTest)
	{
		ALERT(at_console, "%s: %s (%s)\n", pTest->name, pTest->levelName, estates[pTest->state]);
		pTest = pTest->pNext;
	}
}
//#endif


void CGlobalState::EntityAdd(string_t globalname, string_t mapName, GLOBALESTATE state)
{
	ASSERT(!Find(globalname));

	globalentity_t* pNewEntity = (globalentity_t*)calloc(sizeof(globalentity_t), 1);
	ASSERT(pNewEntity != NULL);
	pNewEntity->pNext = m_pList;
	m_pList = pNewEntity;
	strcpy_safe(pNewEntity->name, STRING(globalname), 64);
	strcpy_safe(pNewEntity->levelName, STRING(mapName), 32);
	pNewEntity->state = state;
	m_listCount++;
}


void CGlobalState::EntitySetState(string_t globalname, GLOBALESTATE state)
{
	globalentity_t* pEnt = Find(globalname);

	if (pEnt)
		pEnt->state = state;
}


const globalentity_t* CGlobalState::EntityFromTable(string_t globalname)
{
	globalentity_t* pEnt = Find(globalname);

	return pEnt;
}


GLOBALESTATE CGlobalState::EntityGetState(string_t globalname)
{
	globalentity_t* pEnt = Find(globalname);
	if (pEnt)
		return pEnt->state;

	return GLOBAL_OFF;
}


// Global Savedata for Delay
TYPEDESCRIPTION	CGlobalState::m_SaveData[] =
{
	DEFINE_FIELD(CGlobalState, m_listCount, FIELD_INTEGER),
};

// Global Savedata for Delay
TYPEDESCRIPTION	gGlobalEntitySaveData[] =
{
	DEFINE_ARRAY(globalentity_t, name, FIELD_CHARACTER, 64),
	DEFINE_ARRAY(globalentity_t, levelName, FIELD_CHARACTER, 32),
	DEFINE_FIELD(globalentity_t, state, FIELD_INTEGER),
};


int CGlobalState::Save(CSave& save)
{
	int i;
	globalentity_t* pEntity;

	if (!save.WriteFields("GLOBAL", this, m_SaveData, ARRAYSIZE(m_SaveData)))
		return 0;

	pEntity = m_pList;
	for (i = 0; i < m_listCount && pEntity; i++)
	{
		if (!save.WriteFields("GENT", pEntity, gGlobalEntitySaveData, ARRAYSIZE(gGlobalEntitySaveData)))
			return 0;

		pEntity = pEntity->pNext;
	}

	return 1;
}

int CGlobalState::Restore(CRestore& restore)
{
	int i, listCount;
	globalentity_t tmpEntity;


	ClearStates();
	if (!restore.ReadFields("GLOBAL", this, m_SaveData, ARRAYSIZE(m_SaveData)))
		return 0;

	listCount = m_listCount;	// Get new list count
	m_listCount = 0;				// Clear loaded data

	for (i = 0; i < listCount; i++)
	{
		if (!restore.ReadFields("GENT", &tmpEntity, gGlobalEntitySaveData, ARRAYSIZE(gGlobalEntitySaveData)))
			return 0;
		EntityAdd(MAKE_STRING(tmpEntity.name), MAKE_STRING(tmpEntity.levelName), tmpEntity.state);
	}
	return 1;
}

void CGlobalState::EntityUpdate(string_t globalname, string_t mapname)
{
	globalentity_t* pEnt = Find(globalname);

	if (pEnt)
		strcpy_safe(pEnt->levelName, STRING(mapname), 32);
}


void CGlobalState::ClearStates(void)
{
	globalentity_t* pFree = m_pList;
	while (pFree)
	{
		globalentity_t* pNext = pFree->pNext;
		free(pFree);
		pFree = pNext;
	}
	Reset();
}


void SaveGlobalState(SAVERESTOREDATA* pSaveData)
{
	CSave saveHelper(pSaveData);
	gGlobalState.Save(saveHelper);
}


void RestoreGlobalState(SAVERESTOREDATA* pSaveData)
{
	CRestore restoreHelper(pSaveData);
	gGlobalState.Restore(restoreHelper);
}


void ResetGlobalState(void)
{
	gGlobalState.ClearStates();
	gInitHUD = TRUE;	// Init the HUD on a new game / load game
}
