#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "nodes.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"

//=========================================================
// Multiplayer intermission spots.
//=========================================================
class CInfoIntermission :public CPointEntity
{
	void Spawn(void);
	void Think(void);
};

void CInfoIntermission::Spawn(void)
{
	UTIL_SetOrigin(pev, pev->origin);
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
	pev->v_angle = g_vecZero;

	pev->nextthink = gpGlobals->time + 2;// let targets spawn!

}

void CInfoIntermission::Think(void)
{
	edict_t* pTarget;

	// find my target
	pTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target));

	if (!FNullEnt(pTarget))
	{
		pev->v_angle = UTIL_VecToAngles((pTarget->v.origin - pev->origin).Normalize());
		pev->v_angle.x = -pev->v_angle.x;
	}
}

LINK_ENTITY_TO_CLASS(info_intermission, CInfoIntermission);