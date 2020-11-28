#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CTriggerMultiple.h"


LINK_ENTITY_TO_CLASS(trigger_multiple, CTriggerMultiple);


void CTriggerMultiple::Spawn(void)
{
	if (m_flWait == 0)
		m_flWait = 0.2;

	InitTrigger();

	ASSERTSZ(pev->health == 0, "trigger_multiple with health");
	//	UTIL_SetOrigin(pev, pev->origin);
	//	SET_MODEL( ENT(pev), STRING(pev->model) );
	//	if (pev->health > 0)
	//		{
	//		if (FBitSet(pev->spawnflags, SPAWNFLAG_NOTOUCH))
	//			ALERT(at_error, "trigger_multiple spawn: health and notouch don't make sense");
	//		pev->max_health = pev->health;
	//UNDONE: where to get pfnDie from?
	//		pev->pfnDie = multi_killed;
	//		pev->takedamage = DAMAGE_YES;
	//		pev->solid = SOLID_BBOX;
	//		UTIL_SetOrigin(pev, pev->origin);  // make sure it links into the world
	//		}
	//	else
	{
		SetTouch(&CTriggerMultiple::MultiTouch);
	}
}
