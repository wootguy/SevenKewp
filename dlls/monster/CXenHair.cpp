#include "extdll.h"
#include "util.h"
#include "animation.h"
#include "effects.h"
#include "CXenHair.h"

LINK_ENTITY_TO_CLASS(xen_hair, CXenHair)

void CXenHair::Spawn(void)
{
	Precache();
	SET_MODEL(edict(), "models/hair.mdl");
	UTIL_SetSize(pev, Vector(-4, -4, 0), Vector(4, 4, 32));
	pev->sequence = 0;

	if (!(pev->spawnflags & SF_HAIR_SYNC))
	{
		pev->frame = RANDOM_FLOAT(0, 255);
		pev->framerate = RANDOM_FLOAT(0.7, 1.4);
	}
	ResetSequenceInfo();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	
	// wait for solid entities to spawn
	SetThink(&CXenHair::DropThink);
	pev->nextthink = gpGlobals->time;
}

void CXenHair::DropThink(void)
{
	if (FBitSet(pev->spawnflags, SF_XEN_PLANT_DROP_TO_FLOOR))
		DropToFloor();

	SetThink(&CXenHair::AnimateThink);
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.4);	// Load balance these a bit
}

void CXenHair::AnimateThink(void)
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.5;
}


void CXenHair::Precache(void)
{
	PRECACHE_MODEL("models/hair.mdl");
}