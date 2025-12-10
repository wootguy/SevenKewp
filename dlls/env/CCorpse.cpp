#include "extdll.h"
#include "util.h"
#include "CWorld.h"
#include "bodyque.h"
#include "CBasePlayer.h"

LINK_ENTITY_TO_CLASS(monster_player_corpse, CCorpse) 

//
// make a body que entry for the given ent so the ent can be respawned elsewhere
//
void CreatePlayerCorpse(CBasePlayer* plr)
{
	if ((plr->pev->effects & EF_NODRAW) || plr->pev->modelindex == 0)
		return;

	edict_t* corpse = CREATE_NAMED_ENTITY(MAKE_STRING("monster_player_corpse"));
	if (!corpse) {
		ALERT(at_error, "Failed to create corpse entity\n");
		return;
	}

	entvars_t* pev = plr->pev;
	entvars_t* pevHead = VARS(corpse);

	pevHead->angles = pev->angles;
	pevHead->model = pev->model;
	pevHead->modelindex = pev->modelindex;
	pevHead->frame = pev->frame;
	pevHead->colormap = pev->colormap;
	pevHead->movetype = MOVETYPE_TOSS;
	pevHead->solid = SOLID_NOT;
	pevHead->velocity = pev->velocity;
	pevHead->flags = FL_MONSTER;
	pevHead->deadflag = DEAD_DEAD;
	
	// render player model instead of entity model
	pevHead->renderfx = kRenderFxDeadPlayer;
	pevHead->renderamt = ENTINDEX(ENT(pev));
	
	pevHead->takedamage = DAMAGE_YES;
	pevHead->health = 100;

	pevHead->sequence = pev->sequence;
	pevHead->animtime = pev->animtime;	
	pevHead->effects &= ~EF_NODRAW;

	UTIL_SetOrigin(pevHead, pev->origin);
	
	// TODO: setting a size that isn't g_vecZero causes you to hit an invisible bbox when aiming at the
	// center of the corpse. This bbox is required here or else the model sinks into the floor. A zero
	// bbox can't be set temporarily at the time of the attack because that breaks hit detection
	// entirely, for some reason...
	UTIL_SetSize(pevHead, Vector(-8, -8, -36), Vector(8, 8, 36));

	CBaseEntity* pent = CBaseEntity::Instance(ENT(pevHead));
	if (pent) {
		CBaseMonster* mon = pent->MyMonsterPointer();
		if (mon) {
			mon->m_killedTime = plr->m_killedTime;
			mon->m_bloodColor = BloodColorHuman();
			mon->InitBoneControllers(); // init server version of the player model (disable renderfx to see why)
			mon->CleanupLocalCorpses();
			AddWaterPhysicsEnt(mon, plr->m_waterFriction, plr->m_buoyancy);
		}
	}
}
