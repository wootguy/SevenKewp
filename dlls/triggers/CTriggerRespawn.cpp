#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CRuleEntity.h"
#include "CBasePlayer.h"
#include "CBaseDMStart.h"

// CTriggerRespawn / trigger_respawn -- Respawns dead players and relocates living ones

#define SF_TRESPAWN_TARGET 1
#define SF_TRESPAWN_DEAD_PLAYERS 2
#define SF_TRESPAWN_DONT_MOVE_LIVING 4

class CTriggerRespawn : public CRulePointEntity
{
public:
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void RespawnTarget(CBaseEntity* target);
private:
};

LINK_ENTITY_TO_CLASS(trigger_respawn, CTriggerRespawn)

void CTriggerRespawn::RespawnTarget(CBaseEntity* target) {
	if (!target) {
		return;
	}

	if (target->IsAlive() && (pev->spawnflags & SF_TRESPAWN_DONT_MOVE_LIVING)) {
		return;
	}

	CBasePlayer* plr = (CBasePlayer*)target;
	if (target->IsPlayer() && plr->IsObserver()) {
		return;
	}

	// always move player entity, dead or alive
	edict_t* spawnPoint = EntSelectSpawnPoint(target);
	if (!FNullEnt(spawnPoint)) {
		target->pev->origin = spawnPoint->v.origin;
		target->pev->angles = spawnPoint->v.angles;
		target->pev->fixangle = 1; // force view angles
	}

	if (target->IsAlive()) {
		target->pev->health = target->pev->max_health;
	}
	else if (pev->spawnflags & SF_TRESPAWN_DEAD_PLAYERS) {
		target->Spawn();
	}
}

void CTriggerRespawn::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->spawnflags & SF_TRESPAWN_TARGET) {
		edict_t* ent = NULL;
		while (!FNullEnt(ent = FIND_ENTITY_BY_TARGETNAME(ent, STRING(pev->target)))) {
			RespawnTarget(CBaseEntity::Instance(ent));
		}
		return;
	}

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		edict_t* ent = INDEXENT(i);

		if (!IsValidPlayer(ent)) {
			continue;
		}

		RespawnTarget(CBaseEntity::Instance(ent));
	}
}
