#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "ent_globals.h"
#include "bodyque.h"
#include "CBaseMonster.h"

// Body queue class here.... It's really just CBaseEntity
class CCorpse : public CBaseMonster
{
	virtual int ObjectCaps(void) { return FCAP_DONT_SAVE; }
	int Classify() { return CLASS_PLAYER; }
};

LINK_ENTITY_TO_CLASS(bodyque, CCorpse);


//
// make a body que entry for the given ent so the ent can be respawned elsewhere
//
// GLOBALS ASSUMED SET:  g_eoBodyQueueHead
//
void CopyToBodyQue(entvars_t* pev)
{
	if (pev->effects & EF_NODRAW)
		return;

	entvars_t* pevHead = VARS(g_pBodyQueueHead);

	pevHead->angles = pev->angles;
	pevHead->model = pev->model;
	pevHead->modelindex = pev->modelindex;
	pevHead->frame = pev->frame;
	pevHead->colormap = pev->colormap;
	pevHead->movetype = MOVETYPE_TOSS;
	pevHead->solid = SOLID_NOT;
	pevHead->velocity = pev->velocity;
	pevHead->flags = FL_MONSTER;
	pevHead->deadflag = pev->deadflag;
	
	// render player model instead of entity model
	pevHead->renderfx = kRenderFxDeadPlayer;
	pevHead->renderamt = ENTINDEX(ENT(pev));
	
	pevHead->takedamage = DAMAGE_YES;
	pevHead->health = 100;

	pevHead->sequence = pev->sequence;
	pevHead->animtime = pev->animtime;	

	CBaseEntity* pent = CBaseEntity::Instance(ENT(pevHead));
	if (pent) {
		CBaseMonster* mon = pent->MyMonsterPointer();
		if (mon) {
			mon->m_bloodColor = BLOOD_COLOR_RED;
			mon->InitBoneControllers(); // init server version of the player model (disable renderfx to see why)
		}
	}

	UTIL_SetOrigin(pevHead, pev->origin);
	
	// TODO: setting a size that isn't g_vecZero causes you to hit an invisible bbox when aiming at the
	// center of the corpse. This bbox is required here or else the model sinks into the floor. A zero
	// bbox can't be set temporarily at the time of the attack because that breaks hit detection
	// entirely, for some reason...
	UTIL_SetSize(pevHead, Vector(-8, -8, -36), Vector(8, 8, 36));

	g_pBodyQueueHead = pevHead->owner;
}

void InitBodyQue(void)
{
	string_t	istrClassname = MAKE_STRING("bodyque");

	g_pBodyQueueHead = CREATE_NAMED_ENTITY(istrClassname);
	entvars_t* pev = VARS(g_pBodyQueueHead);

	// Reserve 31 more slots for dead bodies
	for (int i = 0; i < 31; i++)
	{
		pev->owner = CREATE_NAMED_ENTITY(istrClassname);
		pev = VARS(pev->owner);
	}

	pev->owner = g_pBodyQueueHead;
}