#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"

#define SF_TELE_RANDOM_DESTINATION 64
#define SF_TELE_RELATIVE 128
#define SF_TELE_KEEP_ANGLES 256
#define SF_TELE_KEEP_VELOCITY 512
#define SF_TELE_ROTATE_ANGLES 1024

#define SF_TELE_DEST_TRIGGER_ON_ARRIVAL 32

// rotate angles, velocity, and position offset for rotated seamless transitions using relative teleports.
// For example, areas where hallway is shared, but the hallway is rotated 90 degrees so the player would
// run into or get stuck inside a wall if keeping unrotated velocity and position offset.
// This also implies and replaces the following flags: Relative, keep angles, keep velocity
#define SF_TELE_SEAMLESS_TRANSITION 4096

class CTriggerTeleport : public CBaseTrigger
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);

	void EXPORT TeleportTouch(CBaseEntity* pOther);

	bool m_startInactive;
};
LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerTeleport)

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

	edict_t* pentTarget = NULL;
	std::vector<edict_t*> targets;
	edict_t* ent = NULL;
	while (!FNullEnt(ent = FIND_ENTITY_BY_TARGETNAME(ent, STRING(pev->target)))) {
		if (!strcmp(STRING(ent->v.classname), "info_teleport_destination")) {
			targets.push_back(ent);
		}
		else if (!pentTarget) {
			pentTarget = ent;
		}
	}

	if (!targets.empty()) {
		pentTarget = targets[0]; // prefer tp destination entities
		
		if (pev->spawnflags & SF_TELE_RANDOM_DESTINATION)
			pentTarget = targets[RANDOM_LONG(0, targets.size() - 1)];
	}

	if (!pentTarget)
		return;

	Vector tmp = VARS(pentTarget)->origin;

	if (pOther->IsPlayer() && !(pev->spawnflags & (SF_TELE_RELATIVE | SF_TELE_SEAMLESS_TRANSITION)))
	{
		// make origin adjustments in case the teleportee is a player. (origin in center, not at feet)
		// relative/seamless modes already account for this origin difference
		tmp.z -= pOther->pev->mins.z;
	}

	tmp.z++;

	Vector offset = pevToucher->origin - pev->origin;

	if (pev->spawnflags & SF_TELE_SEAMLESS_TRANSITION) {
		// rotate relative offset
		float len = offset.Length();
		Vector vecAngles = UTIL_VecToAngles(offset.Normalize());
		vecAngles.x *= -1;
		UTIL_MakeVectors(vecAngles + pentTarget->v.angles);
		offset = gpGlobals->v_forward * len;
		tmp = tmp + offset;
	}
	else if (pev->spawnflags & SF_TELE_RELATIVE) {
		tmp = tmp + offset;
	}

	UTIL_SetOrigin(pevToucher, tmp);
	pevToucher->flags &= ~FL_ONGROUND;

	if (pev->spawnflags & SF_TELE_SEAMLESS_TRANSITION) {
		// rotate velocity
		float speed = pevToucher->velocity.Length();
		Vector vecAngles = UTIL_VecToAngles(pevToucher->velocity);
		vecAngles.x *= -1;
		UTIL_MakeVectors(vecAngles + pentTarget->v.angles);
		pevToucher->velocity = gpGlobals->v_forward * speed;

		// TODO: players will continue to send movement commands using their old angles which
		// will add some unwanted side velocity. Rotate those commands for some amount of time?
	}
	else if (!(pev->spawnflags & SF_TELE_KEEP_VELOCITY)) {
		pevToucher->velocity = pevToucher->basevelocity = g_vecZero;
	}

	if (pev->spawnflags & (SF_TELE_ROTATE_ANGLES | SF_TELE_SEAMLESS_TRANSITION)) {
		// rotate angles
		pevToucher->angles = pevToucher->v_angle + pentTarget->v.angles;

		if (pOther->IsPlayer()) {
			pevToucher->fixangle = TRUE;
		}
	}
	else if (!(pev->spawnflags & SF_TELE_KEEP_ANGLES)) {
		// copy angles
		pevToucher->angles = pentTarget->v.angles;

		if (pOther->IsPlayer()) {
			pevToucher->fixangle = TRUE;
		}
	}

	if (FClassnameIs(pentTarget, "info_teleport_destination") 
		&& (pentTarget->v.spawnflags & SF_TELE_DEST_TRIGGER_ON_ARRIVAL) && pentTarget->v.target) {
		FireTargets(STRING(pentTarget->v.target), pOther, CBaseEntity::Instance(pentTarget), USE_TOGGLE, 0.0f);
	}
}

LINK_ENTITY_TO_CLASS(info_teleport_destination, CPointEntity)