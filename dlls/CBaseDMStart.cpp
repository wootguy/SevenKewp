#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "gamerules.h"
#include "game.h"
#include "CBaseDMStart.h"
#include <vector>

#define FL_START_OFF 2

class CBaseDMStart : public CPointEntity
{
public:
	void		Spawn(void);
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	BOOL		IsTriggered(CBaseEntity* pEntity);

private:
	bool isActive;
};

// These are the new entry points to entities. 
LINK_ENTITY_TO_CLASS(info_player_deathmatch, CBaseDMStart)
LINK_ENTITY_TO_CLASS(info_player_start, CBaseDMStart)
LINK_ENTITY_TO_CLASS(info_player_coop, CBaseDMStart)
LINK_ENTITY_TO_CLASS(info_player_dm2, CBaseDMStart)
LINK_ENTITY_TO_CLASS(info_landmark, CPointEntity)

void CBaseDMStart::Spawn()
{
	isActive = !(pev->spawnflags & FL_START_OFF);
}

void CBaseDMStart::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	isActive = useType == USE_TOGGLE ? !isActive : useType == USE_ON;
}

BOOL CBaseDMStart::IsTriggered(CBaseEntity* pEntity)
{
	return isActive;
}

/*
============
EntSelectSpawnPoint

Returns the entity to spawn at
============
*/
edict_t* EntSelectSpawnPoint(CBaseEntity* pPlayer, bool includeDisabledSpawns)
{
	CBaseEntity* pSpot;

	std::vector<CBaseEntity*> enabledSpawns;
	std::vector<CBaseEntity*> clearSpawns;
	std::vector<CBaseEntity*> legacySpawns;

	const int SPAWN_ENT_TYPES = 4;
	const char* spawn_ent_names[SPAWN_ENT_TYPES] = {
		"info_player_start",
		"info_player_deathmatch",
		"info_player_dm2",
		"info_player_coop"
	};

	bool shouldDisableLegacySpawns = false;

	for (int i = 0; i < SPAWN_ENT_TYPES; i++) {
		pSpot = NULL;
		while (!FNullEnt(pSpot = UTIL_FindEntityByClassname(pSpot, spawn_ent_names[i]))) {
			if (pSpot->IsTriggered(pPlayer) || includeDisabledSpawns) {
				if (i == 0) {
					legacySpawns.push_back(pSpot);
					continue;
				}

				enabledSpawns.push_back(pSpot);
				
				if (IsSpawnPointClear(pPlayer, pSpot)) {
					clearSpawns.push_back(pSpot);
				}
			}

			if (i != 0) {
				// if any normal spawn is present, then disable info_player_start
				shouldDisableLegacySpawns = true;
			}
		}
	}

	// Spawn point priority:
	// 1. Any non-legacy spawn point which is both enabled and unblocked
	// 2. Any non-legacy spawn point which is enabled
	// 3. Any legacy spawn point (info_player_start)

	if (clearSpawns.size()) {
		pSpot = clearSpawns[RANDOM_LONG(0, clearSpawns.size()-1)];
	}
	else if (enabledSpawns.size()) {
		pSpot = enabledSpawns[RANDOM_LONG(0, enabledSpawns.size() - 1)];
	}
	else if (!shouldDisableLegacySpawns && legacySpawns.size()) {
		pSpot = legacySpawns[RANDOM_LONG(0, legacySpawns.size() - 1)];
	}
	else {
		return INDEXENT(0);
	}

	return pSpot->edict();
}


// checks if the spot is clear of players
BOOL IsSpawnPointClear(CBaseEntity* pPlayer, CBaseEntity* pSpot)
{
	CBaseEntity* ent = NULL;

	while ((ent = UTIL_FindEntityInSphere(ent, pSpot->pev->origin, 128)) != NULL)
	{
		// if ent is a client, don't spawn on 'em
		if (ent->IsPlayer() && ent != pPlayer)
			return FALSE;
	}

	return TRUE;
}
