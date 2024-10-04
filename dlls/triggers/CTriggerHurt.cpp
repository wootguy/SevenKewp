#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"

class CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn(void);
	void EXPORT RadiationThink(void);
	virtual const char* DisplayName() { return "Injury"; }
};

LINK_ENTITY_TO_CLASS(trigger_hurt, CTriggerHurt)


//
// trigger_hurt - hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
//
//int gfToggleState = 0; // used to determine when all radiation trigger hurts have called 'RadiationThink'

void CTriggerHurt::Spawn(void)
{
	InitTrigger();
	SetTouch(&CTriggerHurt::HurtTouch);

	if (!FStringNull(pev->targetname))
	{
		SetUse(&CTriggerHurt::ToggleUse);
	}
	else
	{
		SetUse(NULL);
	}

	if (m_bitsDamageInflict & DMG_RADIATION)
	{
		SetThink(&CTriggerHurt::RadiationThink);
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.0, 0.5);
	}

	if (FBitSet(pev->spawnflags, SF_TRIGGER_HURT_START_OFF))// if flagged to Start Turned Off, make trigger nonsolid.
		pev->solid = SOLID_NOT;

	UTIL_SetOrigin(pev, pev->origin);		// Link into the list
}

// trigger hurt that causes radiation will do a radius
// check and set the player's geiger counter level
// according to distance from center of trigger

void CTriggerHurt::RadiationThink(void)
{
	CBasePlayer* pPlayer = NULL;
	float flRange;
	entvars_t* pevTarget;
	Vector vecSpot1;
	Vector vecSpot2;
	Vector vecRange;
	Vector origin;
	Vector view_ofs;

	// check to see if a player is in pvs
	// if not, continue	

	// set origin to center of trigger so that this check works
	origin = pev->origin;
	view_ofs = pev->view_ofs;

	pev->origin = (pev->absmin + pev->absmax) * 0.5;
	pev->view_ofs = pev->view_ofs * 0.0;

	int numPvsPlayers;
	edict_t* pvsPlayer = UTIL_ClientsInPVS(edict(), numPvsPlayers);

	pev->origin = origin;
	pev->view_ofs = view_ofs;

	// reset origin

	while (!FNullEnt(pvsPlayer))
	{

		pPlayer = GetClassPtr((CBasePlayer*)VARS(pvsPlayer));

		pevTarget = VARS(pvsPlayer);

		// get range to player;

		vecSpot1 = (pev->absmin + pev->absmax) * 0.5;
		vecSpot2 = (pevTarget->absmin + pevTarget->absmax) * 0.5;

		vecRange = vecSpot1 - vecSpot2;
		flRange = vecRange.Length();

		// if player's current geiger counter range is larger
		// than range to this trigger hurt, reset player's
		// geiger counter range 

		if (pPlayer->m_flgeigerRange >= flRange)
			pPlayer->m_flgeigerRange = flRange;

		pvsPlayer = pvsPlayer->v.chain;
	}

	pev->nextthink = gpGlobals->time + 0.25;
}
