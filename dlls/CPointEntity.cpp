#include "extdll.h"
#include "util.h"
#include "cbase.h"

// Landmark class
void CPointEntity::Spawn(void)
{
	pev->solid = SOLID_NOT;
	//	UTIL_SetSize(pev, g_vecZero, g_vecZero);
}

// Lightning target, just alias landmark
LINK_ENTITY_TO_CLASS(info_target, CPointEntity);
