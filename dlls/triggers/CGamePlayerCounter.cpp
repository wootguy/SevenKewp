#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "CPointEntity.h"

//
// CGamePlayerCounter / game_player_counter	-- Fires targets depending on player count

class CGamePlayerCounter : public CPointEntity
{
public:
	void Spawn(void);
	void CountThink();
};

LINK_ENTITY_TO_CLASS(game_player_counter, CGamePlayerCounter)

void CGamePlayerCounter::Spawn(void)
{
	// Save off the initial count
	CPointEntity::Spawn();

	SetThink(&CGamePlayerCounter::CountThink);
	pev->nextthink = gpGlobals->time + 1.0f;
}

void CGamePlayerCounter::CountThink() {

	int playerCount = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		edict_t* plr = INDEXENT(i);

		if (plr && (plr->v.flags & (FL_CLIENT | FL_PROXY)) == FL_CLIENT && plr->v.netname && STRING(plr->v.netname)[0] != '\0') {
			
			// filter targetnames
			if (pev->message && (!plr->v.targetname || strcmp(STRING(pev->message), STRING(plr->v.targetname)))) {
				continue;
			}

			playerCount++;
		}
	}

	// player count changed?
	if (playerCount != (int)pev->dmg) {

		// "Min" target
		if (pev->target && playerCount == (int)pev->frags) {
			FireTargets(STRING(pev->target), this, this, USE_ON, 0.0f);
		}

		// "Max" target
		if (pev->netname && playerCount == (int)pev->health) {
			FireTargets(STRING(pev->netname), this, this, USE_ON, 0.0f);
		}

		pev->dmg = playerCount;
	}

	pev->nextthink = gpGlobals->time + 1.0f;
}
