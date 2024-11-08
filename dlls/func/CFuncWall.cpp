#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CFuncWall.h"

LINK_ENTITY_TO_CLASS(func_wall, CFuncWall)

void CFuncWall::Spawn(void)
{
	pev->angles = (pev->spawnflags & SF_WALL_USE_ANGLES) ? pev->angles : g_vecZero;
	pev->movetype = MOVETYPE_PUSH;  // so it doesn't get pushed by anything
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	// If it can't move/go away, it's really part of the world
	pev->flags |= FL_WORLDBRUSH;
}


void CFuncWall::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, (int)(pev->frame)))
		pev->frame = 1 - pev->frame;
}
