#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"


class CTriggerTeleport : public CBaseTrigger
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);

	void EXPORT TeleportTouch(CBaseEntity* pOther);

	bool m_startInactive;
};
LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerTeleport);

void CTriggerTeleport::Spawn(void)
{
	InitTrigger();

	if (m_startInactive) {
		pev->solid = SOLID_NOT;
	}

	SetTouch(&CTriggerTeleport::TeleportTouch);
	SetUse(&CBaseTrigger::ToggleUse);
}

void CTriggerTeleport::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "teleport_start_inactive"))
	{
		m_startInactive = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseTrigger::KeyValue(pkvd);
}

void CTriggerTeleport::TeleportTouch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;
	edict_t* pentTarget = NULL;

	// Only teleport monsters or clients
	if (!FBitSet(pevToucher->flags, FL_CLIENT | FL_MONSTER))
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	if (!(pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS))
	{// no monsters allowed!
		if (FBitSet(pevToucher->flags, FL_MONSTER))
		{
			return;
		}
	}

	if ((pev->spawnflags & SF_TRIGGER_NOCLIENTS))
	{// no clients allowed
		if (pOther->IsPlayer())
		{
			return;
		}
	}

	pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));
	if (FNullEnt(pentTarget))
		return;

	Vector tmp = VARS(pentTarget)->origin;

	if (pOther->IsPlayer())
	{
		tmp.z -= pOther->pev->mins.z;// make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
	}

	tmp.z++;

	pevToucher->flags &= ~FL_ONGROUND;

	UTIL_SetOrigin(pevToucher, tmp);

	pevToucher->angles = pentTarget->v.angles;

	if (pOther->IsPlayer())
	{
		pevToucher->v_angle = pentTarget->v.angles;
	}

	pevToucher->fixangle = TRUE;
	pevToucher->velocity = pevToucher->basevelocity = g_vecZero;
}

LINK_ENTITY_TO_CLASS(info_teleport_destination, CPointEntity);