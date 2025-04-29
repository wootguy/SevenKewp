#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
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
};

LINK_ENTITY_TO_CLASS(trigger_respawn, CTriggerRespawn)

void CTriggerRespawn::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	bool moveLiving = !(pev->spawnflags & SF_TRESPAWN_DONT_MOVE_LIVING);
	bool respawnDead = pev->spawnflags & SF_TRESPAWN_DEAD_PLAYERS;

	if (pev->spawnflags & SF_TRESPAWN_TARGET) {
		CBaseEntity* ent = NULL;
		while ((ent = UTIL_FindEntityByTargetname(ent, STRING(pev->target)))) {
			UTIL_RespawnPlayer(UTIL_PlayerByIndex(ent->entindex()), moveLiving, respawnDead);
		}
		return;
	}
	
	UTIL_RespawnAllPlayers(moveLiving, respawnDead);
}
