#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"
#include "CFireAndDie.h"
#include "hlds_hooks.h"
#include "CBasePlayer.h"

#define SF_CHANGELEVEL_USEONLY		0x0002

extern DLL_GLOBAL BOOL g_fGameOver;

class CChangeLevel : public CBaseTrigger
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void EXPORT UseChangeLevel(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT TriggerChangeLevel(void);
	void EXPORT ExecuteChangeLevel(void);
	void EXPORT TouchChangeLevel(CBaseEntity* pOther);
	void ChangeLevelNow(CBaseEntity* pActivator);
	int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; } // in case it's made solid by nomaptrans


	static CBaseEntity* FindLandmark(const char* pLandmarkName);
	static int ChangeList(LEVELLIST* pLevelList, int maxList);
	static int AddTransitionToList(LEVELLIST* pLevelList, int listCount, const char* pMapName, const char* pLandmarkName, edict_t* pentLandmark);
	static int InTransitionVolume(CBaseEntity* pEntity, char* pVolumeName);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	char m_szMapName[cchMapNameMost];		// trigger_changelevel only:  next map
	char m_szLandmarkName[cchMapNameMost];		// trigger_changelevel only:  landmark on next map
	int		m_changeTarget;
	float	m_changeTargetDelay;
	bool m_bKeepInventory;
};
LINK_ENTITY_TO_CLASS(trigger_changelevel, CChangeLevel)

// Global Savedata for changelevel trigger
TYPEDESCRIPTION	CChangeLevel::m_SaveData[] =
{
	DEFINE_ARRAY(CChangeLevel, m_szMapName, FIELD_CHARACTER, cchMapNameMost),
	DEFINE_ARRAY(CChangeLevel, m_szLandmarkName, FIELD_CHARACTER, cchMapNameMost),
	DEFINE_FIELD(CChangeLevel, m_changeTarget, FIELD_STRING),
	DEFINE_FIELD(CChangeLevel, m_changeTargetDelay, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CChangeLevel, CBaseTrigger)

//
// Cache user-entity-field values until spawn is called.
//

void CChangeLevel::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "map"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
			ALERT(at_error, "Map name '%s' too long (32 chars)\n", pkvd->szValue);
		strcpy_safe(m_szMapName, pkvd->szValue, 32);

		// lower case map names for linux servers (prevents fastdl/plugin problems)
		for (char* p = m_szMapName; *p; ++p) *p = tolower(*p);

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "landmark"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
			ALERT(at_error, "Landmark name '%s' too long (32 chars)\n", pkvd->szValue);
		strcpy_safe(m_szLandmarkName, pkvd->szValue, 32);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "changetarget"))
	{
		m_changeTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "changedelay"))
	{
		m_changeTargetDelay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "keep_inventory"))
	{
		m_bKeepInventory = atoi(pkvd->szValue) != 0;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseTrigger::KeyValue(pkvd);
}


/*QUAKED trigger_changelevel (0.5 0.5 0.5) ? NO_INTERMISSION
When the player touches this, he gets sent to the map listed in the "map" variable.  Unless the NO_INTERMISSION flag is set, the view will go to the info_intermission spot and display stats.
*/

void CChangeLevel::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	if (FStrEq(m_szMapName, ""))
		ALERT(at_console, "a trigger_changelevel doesn't have a map");

	//if (FStrEq(m_szLandmarkName, ""))
	//	ALERT(at_console, "trigger_changelevel to %s doesn't have a landmark", m_szMapName);

	if (!FStringNull(pev->targetname))
	{
		SetUse(&CChangeLevel::UseChangeLevel);
	}
	InitTrigger();
	if (!(pev->spawnflags & SF_CHANGELEVEL_USEONLY))
		SetTouch(&CChangeLevel::TouchChangeLevel);
	//	ALERT( at_console, "TRANSITION: %s (%s)\n", m_szMapName, m_szLandmarkName );

	if (g_nomaptrans.hasKey(toLowerCase(m_szMapName).c_str())) {
		
		pev->movetype = MOVETYPE_PUSH;
		pev->solid = SOLID_BSP;
		
		// rehlds won't call AddToFullPack if EF_NODRAW is used
		pev->effects = 0;
		pev->renderamt = 0;
		pev->rendermode = kRenderTransTexture;

		SetTouch(NULL);
		UTIL_SetOrigin(pev, pev->origin);
	}
}


void CChangeLevel::ExecuteChangeLevel(void)
{
	MESSAGE_BEGIN(MSG_ALL, SVC_CDTRACK);
	WRITE_BYTE(3);
	WRITE_BYTE(3);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();
}


FILE_GLOBAL char st_szNextMap[cchMapNameMost];
//FILE_GLOBAL char st_szNextSpot[cchMapNameMost];

CBaseEntity* CChangeLevel::FindLandmark(const char* pLandmarkName)
{
	CBaseEntity* pentLandmark = UTIL_FindEntityByTargetname(NULL, pLandmarkName);
	while (pentLandmark)
	{
		// Found the landmark
		if (FClassnameIs(pentLandmark->pev, "info_landmark"))
			return pentLandmark;
		else
			pentLandmark = UTIL_FindEntityByTargetname(pentLandmark, pLandmarkName);
	}
	ALERT(at_error, "Can't find landmark %s\n", pLandmarkName);
	return NULL;
}


//=========================================================
// CChangeLevel :: Use - allows level transitions to be 
// triggered by buttons, etc.
//
//=========================================================
void CChangeLevel::UseChangeLevel(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	ChangeLevelNow(pActivator);
}

void CChangeLevel::ChangeLevelNow(CBaseEntity* pActivator)
{
	//LEVELLIST	levels[16];

	ASSERT(!FStrEq(m_szMapName, ""));

	if (!IS_MAP_VALID(m_szMapName)) {
		UTIL_ClientPrintAll(print_center, UTIL_VarArgs("Next map not found:\n%s", m_szMapName));
		ALERT(at_error, UTIL_VarArgs("Next map not found: %s\n", m_szMapName));
		UTIL_Remove(this); // don't keep checking the disk (lag)
		return;
	}

	// Some people are firing these multiple times in a frame, disable
	if (gpGlobals->time == pev->dmgtime)
		return;

	pev->dmgtime = gpGlobals->time;

	// TODO: this may fail
	CBaseEntity* pPlayer = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
	
	/*
	if (!InTransitionVolume(pPlayer, m_szLandmarkName))
	{
		ALERT(at_aiconsole, "Player isn't in the transition volume %s, aborting\n", m_szLandmarkName);
		return;
	}
	*/

	// Create an entity to fire the changetarget
	if (m_changeTarget)
	{
		CFireAndDie* pFireAndDie = GetClassPtr((CFireAndDie*)NULL);
		if (pFireAndDie)
		{
			// Set target and delay
			pFireAndDie->pev->target = m_changeTarget;
			pFireAndDie->m_flDelay = m_changeTargetDelay;
			if (pPlayer)
				pFireAndDie->pev->origin = pPlayer->pev->origin;
			// Call spawn
			DispatchSpawn(pFireAndDie->edict());
		}
	}
	// This object will get removed in the call to CHANGE_LEVEL, copy the params into "safe" memory
	strcpy_safe(st_szNextMap, m_szMapName, 32);

	m_hActivator = pActivator;
	SUB_UseTargets(pActivator, USE_TOGGLE, 0);
	/*
	st_szNextSpot[0] = 0;	// Init landmark to NULL

	// look for a landmark entity		
	edict_t* pentLandmark = FindLandmark(m_szLandmarkName);
	if (!FNullEnt(pentLandmark))
	{
		strcpy_safe(st_szNextSpot, m_szLandmarkName, 32);
		gpGlobals->vecLandmarkOffset = VARS(pentLandmark)->origin;
	}
	//	ALERT( at_console, "Level touches %d levels\n", ChangeList( levels, 16 ) );
	ALERT(at_console, "CHANGE LEVEL: %s %s\n", st_szNextMap, st_szNextSpot);
	CHANGE_LEVEL(st_szNextMap, st_szNextSpot);
	*/

	if (m_bKeepInventory) {
		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			CBasePlayer* plr = (CBasePlayer*)UTIL_PlayerByIndex(i);
			if (!plr) {
				continue;
			}

			plr->SaveInventory();
		}

		g_clearInventoriesNextMap = false;
	}

	ALERT(at_console, "CHANGE LEVEL: %s\n", st_szNextMap);

	if (mp_series_intermission.value == 2) {
		// show scores and a chat message before executing the level change
		CVAR_SET_STRING("mp_nextmap", st_szNextMap);
		g_pGameRules->EndMultiplayerGame(INTERMISSION_LEVEL_END);
		UTIL_Remove(this);
	}
	else {
		CHANGE_LEVEL(st_szNextMap, NULL);
	}
}

//
// GLOBALS ASSUMED SET:  st_szNextMap
//
void CChangeLevel::TouchChangeLevel(CBaseEntity* pOther)
{
	if (!FClassnameIs(pOther->pev, "player"))
		return;

	if (!RunInventoryRules(pOther)) {
		return;
	}

	ChangeLevelNow(pOther);
}


// Add a transition to the list, but ignore duplicates 
// (a designer may have placed multiple trigger_changelevels with the same landmark)
int CChangeLevel::AddTransitionToList(LEVELLIST* pLevelList, int listCount, const char* pMapName, const char* pLandmarkName, edict_t* pentLandmark)
{
	int i;

	if (!pLevelList || !pMapName || !pLandmarkName || !pentLandmark)
		return 0;

	for (i = 0; i < listCount; i++)
	{
		if (pLevelList[i].pentLandmark == pentLandmark && strcmp(pLevelList[i].mapName, pMapName) == 0)
			return 0;
	}
	strcpy_safe(pLevelList[listCount].mapName, pMapName, 32);
	strcpy_safe(pLevelList[listCount].landmarkName, pLandmarkName, 32);
	pLevelList[listCount].pentLandmark = pentLandmark;
	pLevelList[listCount].vecLandmarkOrigin = VARS(pentLandmark)->origin;

	return 1;
}

int BuildChangeList(LEVELLIST* pLevelList, int maxList)
{
	return CChangeLevel::ChangeList(pLevelList, maxList);
}


int CChangeLevel::InTransitionVolume(CBaseEntity* pEntity, char* pVolumeName)
{
	CBaseEntity* pVolume;


	if (pEntity->ObjectCaps() & FCAP_FORCE_TRANSITION)
		return 1;

	// If you're following another entity, follow it through the transition (weapons follow the player)
	if (pEntity->pev->movetype == MOVETYPE_FOLLOW)
	{
		if (pEntity->pev->aiment != NULL)
			pEntity = CBaseEntity::Instance(pEntity->pev->aiment);
	}

	int inVolume = 1;	// Unless we find a trigger_transition, everything is in the volume

	pVolume = UTIL_FindEntityByTargetname(NULL, pVolumeName);
	while (pVolume)
	{
		if (pVolume && FClassnameIs(pVolume->pev, "trigger_transition"))
		{
			if (pVolume->Intersects(pEntity))	// It touches one, it's in the volume
				return 1;
			else
				inVolume = 0;	// Found a trigger_transition, but I don't intersect it -- if I don't find another, don't go!
		}
		pVolume = UTIL_FindEntityByTargetname(pVolume, pVolumeName);
	}

	return inVolume;
}


// We can only ever move 512 entities across a transition
#define MAX_ENTITY 512

// This has grown into a complicated beast
// Can we make this more elegant?
// This builds the list of all transitions on this level and which entities are in their PVS's and can / should
// be moved across.
int CChangeLevel::ChangeList(LEVELLIST* pLevelList, int maxList)
{
	CBaseEntity* pentChangelevel, *pentLandmark;
	int			i, count;

	count = 0;

	// Find all of the possible level changes on this BSP
	pentChangelevel = UTIL_FindEntityByClassname(NULL, "trigger_changelevel");
	if (!pentChangelevel)
		return 0;
	while (pentChangelevel)
	{
		CChangeLevel* pTrigger;

		pTrigger = (CChangeLevel*)pentChangelevel;
		if (pTrigger)
		{
			// Find the corresponding landmark
			pentLandmark = FindLandmark(pTrigger->m_szLandmarkName);
			if (pentLandmark)
			{
				// Build a list of unique transitions
				if (AddTransitionToList(pLevelList, count, pTrigger->m_szMapName, pTrigger->m_szLandmarkName, pentLandmark->edict()))
				{
					count++;
					if (count >= maxList)		// FULL!!
						break;
				}
			}
		}
		pentChangelevel = UTIL_FindEntityByClassname(pentChangelevel, "trigger_changelevel");
	}

	if (gpGlobals->pSaveData && ((SAVERESTOREDATA*)gpGlobals->pSaveData)->pTable)
	{
		CSave saveHelper((SAVERESTOREDATA*)gpGlobals->pSaveData);

		for (i = 0; i < count; i++)
		{
			int j, entityCount = 0;
			CBaseEntity* pEntList[MAX_ENTITY];
			int			 entityFlags[MAX_ENTITY];

			// Follow the linked list of entities in the PVS of the transition landmark
			edict_t* pent = UTIL_EntitiesInPVS(pLevelList[i].pentLandmark);

			// Build a list of valid entities in this linked list (we're going to use pent->v.chain again)
			while (!FNullEnt(pent))
			{
				CBaseEntity* pEntity = CBaseEntity::Instance(pent);
				if (pEntity)
				{
					//					ALERT( at_console, "Trying %s\n", STRING(pEntity->pev->classname) );
					int caps = pEntity->ObjectCaps();
					if (!(caps & FCAP_DONT_SAVE))
					{
						int flags = 0;

						// If this entity can be moved or is global, mark it
						if (caps & FCAP_ACROSS_TRANSITION)
							flags |= FENTTABLE_MOVEABLE;
						if (pEntity->pev->globalname && !pEntity->IsDormant())
							flags |= FENTTABLE_GLOBAL;
						if (flags)
						{
							pEntList[entityCount] = pEntity;
							entityFlags[entityCount] = flags;
							entityCount++;
							if (entityCount > MAX_ENTITY)
								ALERT(at_error, "Too many entities across a transition!");
						}
						//						else
						//							ALERT( at_console, "Failed %s\n", STRING(pEntity->pev->classname) );
					}
					//					else
					//						ALERT( at_console, "DON'T SAVE %s\n", STRING(pEntity->pev->classname) );
				}
				pent = pent->v.chain;
			}

			for (j = 0; j < entityCount; j++)
			{
				// Check to make sure the entity isn't screened out by a trigger_transition
				if (entityFlags[j] && InTransitionVolume(pEntList[j], pLevelList[i].landmarkName))
				{
					// Mark entity table with 1<<i
					int index = saveHelper.EntityIndex(pEntList[j]);
					// Flag it with the level number
					saveHelper.EntityFlagsSet(index, entityFlags[j] | (1 << i));
				}
				//				else
				//					ALERT( at_console, "Screened out %s\n", STRING(pEntList[j]->pev->classname) );

			}
		}
	}

	return count;
}

/*
go to the next level for deathmatch
only called if a time or frag limit has expired
*/
void NextLevel(void)
{
	CBaseEntity* pent;
	CChangeLevel* pChange;

	// find a trigger_changelevel
	pent = UTIL_FindEntityByClassname(NULL, "trigger_changelevel");

	// go back to start if no trigger_changelevel
	if (!pent)
	{
		gpGlobals->mapname = ALLOC_STRING("start");
		pChange = GetClassPtr((CChangeLevel*)NULL);
		strcpy_safe(pChange->m_szMapName, "start", 32);
	}
	else
		pChange = (CChangeLevel*)pent;

	strcpy_safe(st_szNextMap, pChange->m_szMapName, 32);
	g_fGameOver = TRUE;

	if (pChange->pev->nextthink < gpGlobals->time)
	{
		pChange->SetThink(&CChangeLevel::ExecuteChangeLevel);
		pChange->pev->nextthink = gpGlobals->time + 0.1;
	}
}

