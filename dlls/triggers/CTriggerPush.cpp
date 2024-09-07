#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"

#define	SF_TRIGGER_PUSH_START_OFF	2//spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_PUSH_NO_CLIENTS 8
#define SF_TRIGGER_PUSH_NO_MONSTERS 16

class CTriggerPush : public CBaseTrigger
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Touch(CBaseEntity* pOther);
};
LINK_ENTITY_TO_CLASS(trigger_push, CTriggerPush)


void CTriggerPush::KeyValue(KeyValueData* pkvd)
{
	CBaseTrigger::KeyValue(pkvd);
}


/*QUAKED trigger_push (.5 .5 .5) ? TRIG_PUSH_ONCE
Pushes the player
*/

void CTriggerPush::Spawn()
{
	if (pev->angles == g_vecZero)
		pev->angles.y = 360;
	InitTrigger();

	if (pev->speed == 0)
		pev->speed = 100;

	if (FBitSet(pev->spawnflags, SF_TRIGGER_PUSH_START_OFF))// if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	SetUse(&CTriggerPush::ToggleUse);

	UTIL_SetOrigin(pev, pev->origin);		// Link into the list
}


void CTriggerPush::Touch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;

	// UNDONE: Is there a better way than health to detect things that have physics? (clients/monsters)
	switch (pevToucher->movetype)
	{
	case MOVETYPE_NONE:
	case MOVETYPE_PUSH:
	case MOVETYPE_NOCLIP:
	case MOVETYPE_FOLLOW:
		return;
	}

	if (pOther->IsPlayer()) {
		CBasePlayer* plr = (CBasePlayer*)pOther;
		if ((pev->spawnflags & SF_TRIGGER_PUSH_NO_CLIENTS) || plr->IsOnLadder())
			return;
	}
	else if (pOther->IsMonster()) {
		if (pev->spawnflags & SF_TRIGGER_PUSH_NO_MONSTERS)
			return;
	}
	else if (FClassnameIs(pOther->pev, "func_pushable")) {
		if (!(pev->spawnflags & SF_TRIGGER_PUSHABLES))
			return;
	}

	if (pevToucher->solid != SOLID_NOT && pevToucher->solid != SOLID_BSP)
	{
		// Instant trigger, just transfer velocity and remove
		if (FBitSet(pev->spawnflags, SF_TRIG_PUSH_ONCE))
		{
			pevToucher->velocity = pevToucher->velocity + (pev->speed * pev->movedir);
			if (pevToucher->velocity.z > 0)
				pevToucher->flags &= ~FL_ONGROUND;
			UTIL_Remove(this);
		}
		else
		{	// Push field, transfer to base velocity
			Vector vecPush = (pev->speed * pev->movedir);
			if (pevToucher->flags & FL_BASEVELOCITY)
				vecPush = vecPush + pevToucher->basevelocity;

			pevToucher->basevelocity = vecPush;

			pevToucher->flags |= FL_BASEVELOCITY;
			//			ALERT( at_console, "Vel %f, base %f\n", pevToucher->velocity.z, pevToucher->basevelocity.z );
		}
	}
}

