#include "extdll.h"
#include "util.h"
#include "gamerules.h"
#include "game.h"
#include "CBaseDMStart.h"
#include <vector>
#include "CBasePlayer.h"

#define FL_START_OFF 2
#define FL_SPAWN_FILTER_TNAME 8
#define FL_SPAWN_FILTER_INVERT 16
#define FL_SPAWN_TRIGGER 32

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
	if (pev->spawnflags & FL_SPAWN_FILTER_TNAME) {
		bool namesShouldMatch = !(pev->spawnflags & FL_SPAWN_FILTER_INVERT);
		bool namesMatch = (!pev->message && !pEntity->pev->targetname)
				|| (pev->message && pEntity->pev->targetname && !strcmp(STRING(pev->message), STRING(pEntity->pev->targetname)));

		if (namesMatch != namesShouldMatch) {
			return false;
		}
	}

	return isActive;
}

void CBaseDMStart::SpawnPlayer(CBasePlayer* plr) {
	plr->pev->origin = pev->origin + Vector(0, 0, 1);
	plr->pev->v_angle = g_vecZero;
	plr->pev->velocity = g_vecZero;
	plr->pev->angles = pev->angles;
	plr->pev->punchangle = g_vecZero;
	plr->pev->fixangle = TRUE;
	
	if (pev->netname) {
		// new player targetname
		plr->pev->targetname = pev->netname;
	}

	// try spawning ducked if there's no room to stand
	TraceResult tr;
	TRACE_HULL(plr->pev->origin, plr->pev->origin, ignore_monsters, human_hull, NULL, &tr);
	if (tr.fStartSolid) {
		plr->pev->flags |= FL_DUCKING;
		plr->pev->view_ofs = VEC_DUCK_VIEW;
	}

	if (pev->spawnflags & FL_SPAWN_TRIGGER) {
		FireTargets(STRING(pev->target), plr, this, (USE_TYPE)triggerState, 0.0f);
	}
}

void CBaseDMStart::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		triggerState = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CPointEntity::KeyValue(pkvd);
	}
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
