#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "trains.h"
#include "saverestore.h"
#include "CBasePlatTrain.h"
#include "CFuncPlat.h"
#include "CPlatTrigger.h"

/*QUAKED func_plat (0 .5 .8) ? PLAT_LOW_TRIGGER
speed	default 150

Plats are always drawn in the extended position, so they will light correctly.

If the plat is the target of another trigger or button, it will start out disabled in
the extended position until it is trigger, when it will lower and become a normal plat.

If the "height" key is set, that will determine the amount the plat moves, instead of
being implicitly determined by the model's height.

Set "sounds" to one of the following:
1) base fast
2) chain slow
*/

LINK_ENTITY_TO_CLASS(func_plat, CFuncPlat);

void CFuncPlat::Setup(void)
{
	//pev->plat_noiseMoving = MAKE_STRING("plats/platmove1.wav");
	//pev->plat_noiseArrived = MAKE_STRING("plats/platstop1.wav");

	if (m_flTLength == 0)
		m_flTLength = 80;
	if (m_flTWidth == 0)
		m_flTWidth = 10;

	pev->angles = g_vecZero;

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);		// set size and link into world
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model));

	// vecPosition1 is the top position, vecPosition2 is the bottom
	m_vecPosition1 = pev->origin;
	m_vecPosition2 = pev->origin;
	if (m_flHeight != 0)
		m_vecPosition2.z = pev->origin.z - m_flHeight;
	else
		m_vecPosition2.z = pev->origin.z - pev->size.z + 8;
	if (pev->speed == 0)
		pev->speed = 150;

	if (m_volume == 0)
		m_volume = 0.85;
}


void CFuncPlat::Precache()
{
	CBasePlatTrain::Precache();
	//PRECACHE_SOUND("plats/platmove1.wav");
	//PRECACHE_SOUND("plats/platstop1.wav");
	if (!IsTogglePlat())
		PlatSpawnInsideTrigger(pev);		// the "start moving" trigger
}


void CFuncPlat::Spawn()
{
	Setup();

	Precache();

	// If this platform is the target of some button, it starts at the TOP position,
	// and is brought down by that button.  Otherwise, it starts at BOTTOM.
	if (!FStringNull(pev->targetname))
	{
		UTIL_SetOrigin(pev, m_vecPosition1);
		m_toggle_state = TS_AT_TOP;
		SetUse(&CFuncPlat::PlatUse);
	}
	else
	{
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_toggle_state = TS_AT_BOTTOM;
	}
}


//
// Used by SUB_UseTargets, when a platform is the target of a button.
// Start bringing platform down.
//
void CFuncPlat::PlatUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (IsTogglePlat())
	{
		// Top is off, bottom is on
		BOOL on = (m_toggle_state == TS_AT_BOTTOM) ? TRUE : FALSE;

		if (!ShouldToggle(useType, on))
			return;

		if (m_toggle_state == TS_AT_TOP)
			GoDown();
		else if (m_toggle_state == TS_AT_BOTTOM)
			GoUp();
	}
	else
	{
		SetUse(NULL);

		if (m_toggle_state == TS_AT_TOP)
			GoDown();
	}
}


//
// Platform is at top, now starts moving down.
//
void CFuncPlat::GoDown(void)
{
	if (pev->plat_noiseMoving)
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving), m_volume, ATTN_NORM);

	ASSERT(m_toggle_state == TS_AT_TOP || m_toggle_state == TS_GOING_UP);
	m_toggle_state = TS_GOING_DOWN;
	SetMoveDone(&CFuncPlat::CallHitBottom);
	LinearMove(m_vecPosition2, pev->speed);
}


//
// Platform has hit bottom.  Stops and waits forever.
//
void CFuncPlat::HitBottom(void)
{
	if (pev->plat_noiseMoving)
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving));

	if (pev->plat_noiseArrived)
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, (char*)STRING(pev->plat_noiseArrived), m_volume, ATTN_NORM);

	ASSERT(m_toggle_state == TS_GOING_DOWN);
	m_toggle_state = TS_AT_BOTTOM;
}


//
// Platform is at bottom, now starts moving up
//
void CFuncPlat::GoUp(void)
{
	if (pev->plat_noiseMoving)
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving), m_volume, ATTN_NORM);

	ASSERT(m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_GOING_DOWN);
	m_toggle_state = TS_GOING_UP;
	SetMoveDone(&CFuncPlat::CallHitTop);
	LinearMove(m_vecPosition1, pev->speed);
}


//
// Platform has hit top.  Pauses, then starts back down again.
//
void CFuncPlat::HitTop(void)
{
	if (pev->plat_noiseMoving)
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving));

	if (pev->plat_noiseArrived)
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, (char*)STRING(pev->plat_noiseArrived), m_volume, ATTN_NORM);

	ASSERT(m_toggle_state == TS_GOING_UP);
	m_toggle_state = TS_AT_TOP;

	if (!IsTogglePlat())
	{
		// After a delay, the platform will automatically start going down again.
		SetThink(&CFuncPlat::CallGoDown);
		pev->nextthink = pev->ltime + 3;
	}
}


void CFuncPlat::Blocked(CBaseEntity* pOther)
{
	// Hurt the blocker a little
	if (pev->dmg || FClassnameIs(pOther->pev, "monster_tripmine")) {
		pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);
	}

	if (pOther->IsMonster() && !pOther->IsAlive()) {
		// don't let corpses block anything
		pOther->Killed(pev, GIB_ALWAYS);
		return; // don't bounce back because the path is clear now
	}

	if (pev->plat_noiseMoving)
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving));

	// Send the platform back where it came from
	ASSERT(m_toggle_state == TS_GOING_UP || m_toggle_state == TS_GOING_DOWN);
	if (m_toggle_state == TS_GOING_UP)
		GoDown();
	else if (m_toggle_state == TS_GOING_DOWN)
		GoUp();
}
