#include "extdll.h"
#include "util.h"
#include "CPointEntity.h"

// Landmark class
void CPointEntity::Spawn(void)
{
	pev->solid = SOLID_NOT;
	//	UTIL_SetSize(pev, g_vecZero, g_vecZero);
}

// Lightning target, just alias landmark
LINK_ENTITY_TO_CLASS(info_target, CPointEntity)

// TODO: implement. This should allow players to click things in the camera view
LINK_ENTITY_TO_CLASS(trigger_cameratarget, CPointEntity)
