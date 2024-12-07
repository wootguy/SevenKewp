#include "extdll.h"
#include "util.h"
#include "CFuncConveyor.h"

LINK_ENTITY_TO_CLASS(func_conveyor, CFuncConveyor)
void CFuncConveyor::Spawn(void)
{
	SetMovedir(pev);
	CFuncWall::Spawn();

	if (!(pev->spawnflags & SF_CONVEYOR_VISUAL))
		SetBits(pev->flags, FL_CONVEYOR);

	// HACKHACK - This is to allow for some special effects
	if (pev->spawnflags & SF_CONVEYOR_NOTSOLID)
	{
		pev->solid = SOLID_NOT;
		pev->skin = 0;		// Don't want the engine thinking we've got special contents on this brush
	}

	if (pev->speed == 0)
		pev->speed = 100;

	UpdateSpeed(pev->speed);
}


// HACKHACK -- This is ugly, but encode the speed in the rendercolor to avoid adding more data to the network stream
void CFuncConveyor::UpdateSpeed(float speed)
{
	// Encode it as an integer with 4 fractional bits
	int speedCode = (int)(fabs(speed) * 16.0);

	if (speed < 0)
		pev->rendercolor.x = 1;
	else
		pev->rendercolor.x = 0;

	pev->rendercolor.y = (speedCode >> 8);
	pev->rendercolor.z = (speedCode & 0xFF);
}


void CFuncConveyor::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	pev->speed = -pev->speed;
	UpdateSpeed(pev->speed);
}
