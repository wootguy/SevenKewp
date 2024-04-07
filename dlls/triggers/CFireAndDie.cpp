#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CFireAndDie.h"

LINK_ENTITY_TO_CLASS(fireanddie, CFireAndDie);

void CFireAndDie::Spawn(void)
{
	pev->classname = MAKE_STRING("fireanddie");
	// Don't call Precache() - it should be called on restore
}


void CFireAndDie::Precache(void)
{
	// This gets called on restore
	pev->nextthink = gpGlobals->time + m_flDelay;
}


void CFireAndDie::Think(void)
{
	SUB_UseTargets(this, USE_TOGGLE, 0);
	UTIL_Remove(this);
}
