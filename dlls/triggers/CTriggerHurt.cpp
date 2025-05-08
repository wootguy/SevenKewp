#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"

#define SF_TRIGGER_HURT_TARGETONCE	1// Only fire hurt target once
#define	SF_TRIGGER_HURT_START_OFF	2//spawnflag that makes trigger_push spawn turned OFF
#define	SF_TRIGGER_HURT_NO_CLIENTS	8//spawnflag that makes trigger_push spawn turned OFF
#define SF_TRIGGER_HURT_CLIENTONLYFIRE	16// trigger hurt will only fire its target if it is hurting a client
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32// only clients may touch this trigger.

#define TRIGGER_HURT_DELAY 0.5f // seconds between each hurt trigger

class CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn(void);
	void EXPORT RadiationThink(void);
	virtual const char* DisplayName() { return "Hazard"; }
	void HurtTouch(CBaseEntity* pOther);
};

LINK_ENTITY_TO_CLASS(trigger_hurt, CTriggerHurt)


//
// trigger_hurt - hurts anything that touches it. if the trigger has a targetname, firing it will toggle state
//
//int gfToggleState = 0; // used to determine when all radiation trigger hurts have called 'RadiationThink'

void CTriggerHurt::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

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

// When touched, a hurt trigger does DMG points of damage each half-second
void CTriggerHurt::HurtTouch(CBaseEntity* pOther)
{
	float fldmg;

	if (!pOther->pev->takedamage)
		return;

	if ((pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYTOUCH) && !pOther->IsPlayer())
	{
		// this trigger is only allowed to touch clients, and this ain't a client.
		return;
	}

	if ((pev->spawnflags & SF_TRIGGER_HURT_NO_CLIENTS) && pOther->IsPlayer())
		return;

	if (pOther->IsMonster()) {
		// monsters only Touch() when moving, so force checks while a monster is here
		// TODO: Don't use this evil variable. It causes other entities like trigger_push
		// to launch players at lightning speeds, and trigger_hurt will start spamming noises
		// on weapons and func_breakable.
		//gpGlobals->force_retouch++;
	}

	// rules are inverted for damage triggers.
	// "Need item" = you need this item or else pain is delivered
	// this should only pass if the entity has inventory rules at all
	// otherwise this is always true.
	if (HasInventoryRules() && RunInventoryRules(pOther)) {
		return;
	}

	float timeSinceLastHurt = TRIGGER_HURT_DELAY;
	float lastHurtTime = 0;

	CBaseMonster* mon = pOther->MyMonsterPointer();
	if (mon) {
		lastHurtTime = mon->LastHurtTriggerTime(this);
		timeSinceLastHurt = gpGlobals->time - lastHurtTime;

		if (timeSinceLastHurt < TRIGGER_HURT_DELAY) {
			return;
		}
		
		mon->RememberHurtTrigger(this);
	}

	// don't let laggy players or speedhackers bypass damage ticks by pausing packets
	// TODO: detect when not touching trigger to stop stacking hurts after leaving it
	//int numHurts = lastHurtTime > 0 ? V_max(1, timeSinceLastHurt / TRIGGER_HURT_DELAY) : 1;
	
	int numHurts = 1;

	// If this is time_based damage (poison, radiation), override the pev->dmg with a 
	// default for the given damage type.  Monsters only take time-based damage
	// while touching the trigger.  Player continues taking damage for a while after
	// leaving the trigger

	fldmg = pev->dmg * TRIGGER_HURT_DELAY * numHurts; // 0.5 seconds worth of damage, pev->dmg is damage/second

	if (fldmg < 0)
	{
		BOOL bApplyHeal = TRUE;

		if (g_pGameRules->IsMultiplayer() && pOther->IsPlayer())
		{
			bApplyHeal = pOther->pev->deadflag == DEAD_NO;
		}

		if (bApplyHeal)
		{
			pOther->TakeHealth(-fldmg, m_bitsDamageInflict);
		}
	}
	else {
		pOther->TakeDamage(pev, pev, fldmg, m_bitsDamageInflict);
	}

	if (pev->target)
	{
		// trigger has a target it wants to fire. 
		if (pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYFIRE)
		{
			// if the toucher isn't a client, don't fire the target!
			if (!pOther->IsPlayer())
			{
				return;
			}
		}

		SUB_UseTargets(pOther, USE_TOGGLE, 0);
		if (pev->spawnflags & SF_TRIGGER_HURT_TARGETONCE)
			pev->target = 0;
	}
}
