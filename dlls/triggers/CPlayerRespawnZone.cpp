#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CRuleEntity.h"
#include "CBaseDMStart.h"

//
// CPlayerRespawnZone / player_respawn_zone -- teleports players to active spawn points
//

enum zone_types {
	ZONE_RESPAWN_OUTSIDE,
	ZONE_RESPAWN_INSIDE
};

class CPlayerRespawnZone : public CRuleBrushEntity
{
public:
	void		KeyValue(KeyValueData* pkvd);
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

private:
	int zoneType;
};

LINK_ENTITY_TO_CLASS(player_respawn_zone, CPlayerRespawnZone);

TYPEDESCRIPTION	CPlayerRespawnZone::m_SaveData[] =
{
	DEFINE_FIELD(CPlayerRespawnZone, zoneType, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CPlayerRespawnZone, CRuleBrushEntity);

void CPlayerRespawnZone::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "zonetype"))
	{
		zoneType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CRuleBrushEntity::KeyValue(pkvd);
}

void CPlayerRespawnZone::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pPlayer = NULL;

	bool respawnInside = zoneType == ZONE_RESPAWN_INSIDE;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		pPlayer = UTIL_PlayerByIndex(i);
		
		if (!pPlayer)
			continue;

		TraceResult trace;
		int	hullNumber = (pPlayer->pev->flags & FL_DUCKING) ? head_hull : human_hull;

		UTIL_TraceModel(pPlayer->pev->origin, pPlayer->pev->origin, hullNumber, edict(), &trace);

		if ((bool)trace.fStartSolid == respawnInside) {
			// Despite the entity name, players are not actually respawned.
			// Players are merely teleported to active spawn points
			edict_t* spawnPoint = EntSelectSpawnPoint(pPlayer);
			if (spawnPoint) {
				pPlayer->pev->origin = spawnPoint->v.origin;
				pPlayer->pev->angles = spawnPoint->v.angles;
				pPlayer->pev->fixangle = 1; // force view angles
			}
		}
	}
}

