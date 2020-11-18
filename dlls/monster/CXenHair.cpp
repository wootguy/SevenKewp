#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "animation.h"
#include "effects.h"
#include "CActAnimating.h"

class CXenHair : public CActAnimating
{
public:
	void		Spawn(void);
	void		Precache(void);
	void		Think(void);
};

LINK_ENTITY_TO_CLASS(xen_hair, CXenHair);

#define SF_HAIR_SYNC		0x0001

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
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1, 0.4);	// Load balance these a bit
}


void CXenHair::Think(void)
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.5;
}


void CXenHair::Precache(void)
{
	PRECACHE_MODEL("models/hair.mdl");
}