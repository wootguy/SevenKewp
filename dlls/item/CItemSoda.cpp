#include "extdll.h"
#include "util.h"
#include "customentity.h"
#include "effects.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"

//=========================================================
// Soda can
//=========================================================
class CItemSoda : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	EXPORT CanThink(void);
	void	EXPORT CanTouch(CBaseEntity* pOther);
};

void CItemSoda::Precache(void)
{
}

LINK_ENTITY_TO_CLASS(item_sodacan, CItemSoda)

void CItemSoda::Spawn(void)
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_TOSS;

	SET_MODEL(ENT(pev), "models/can.mdl");
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetThink(&CItemSoda::CanThink);
	pev->nextthink = gpGlobals->time + 0.5;
}

void CItemSoda::CanThink(void)
{
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/g_bounce3.wav", 1, ATTN_NORM);

	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-8, -8, 0), Vector(8, 8, 8));
	SetThink(NULL);
	SetTouch(&CItemSoda::CanTouch);
}

void CItemSoda::CanTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{
		return;
	}

	// spoit sound here

	pOther->TakeHealth(1, DMG_GENERIC);// a bit of health.

	if (!FNullEnt(pev->owner))
	{
		// tell the machine the can was taken
		pev->owner->v.frags = 0;
	}

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = EF_NODRAW;
	SetTouch(NULL);
	SetThink(&CItemSoda::SUB_Remove);
	pev->nextthink = gpGlobals->time;
}
