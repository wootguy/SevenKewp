#include "extdll.h"
#include "util.h"
#include "trains.h"
#include "saverestore.h"
#include "CBasePlatTrain.h"
#include "CFuncPlat.h"
#include "CFuncTrackTrain.h"
#include "path/CPathCorner.h"
#include "path/CPathTrack.h"

class CFuncTrain : public CBasePlatTrain
{
public:
	void Spawn(void);
	void Precache(void);
	void Activate(void);
	void OverrideReset(void);
	virtual const char* DisplayName() { return "Compression"; } // hack for the kill feed

	void Blocked(CBaseEntity* pOther);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void KeyValue(KeyValueData* pkvd);


	void EXPORT Wait(void);
	void EXPORT Next(void);
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	entvars_t* m_pevCurrentTarget;
	EHANDLE m_previousTarget;
	int			m_sounds;
	int		m_activated; // 0 = never activated, 1 = activated, 2 = toggled off after activate
	float m_lastNext;
	int m_nextRecursions;
};

LINK_ENTITY_TO_CLASS(func_train, CFuncTrain)
LINK_ENTITY_TO_CLASS(env_spritetrain, CFuncTrain)

TYPEDESCRIPTION	CFuncTrain::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTrain, m_sounds, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTrain, m_pevCurrentTarget, FIELD_EVARS),
	DEFINE_FIELD(CFuncTrain, m_activated, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CFuncTrain, CBasePlatTrain)


void CFuncTrain::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBasePlatTrain::KeyValue(pkvd);
}


void CFuncTrain::Blocked(CBaseEntity* pOther)

{
	if (gpGlobals->time < m_flActivateFinished)
		return;

	m_flActivateFinished = gpGlobals->time + 0.5;

	if (pOther->IsMonster() && !pOther->IsAlive()) {
		// don't let corpses block anything
		pOther->Killed(pev, GIB_ALWAYS);
	}

	pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);
}


void CFuncTrain::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER)
	{
		// Move toward my target
		pev->spawnflags &= ~SF_TRAIN_WAIT_RETRIGGER;
		Next();
		m_activated = 1;
	}
	else
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		// Pop back to last target if it's available
		if (pev->enemy)
			pev->target = pev->enemy->v.targetname;
		pev->nextthink = 0;
		pev->velocity = g_vecZero;
		if (pev->plat_noiseArrived)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->plat_noiseArrived), m_volume, ATTN_NORM);
		m_activated = 2;
	}
}


void CFuncTrain::Wait(void)
{
	// Fire the pass target if there is one
	if (m_pevCurrentTarget->message)
	{
		FireTargets(STRING(m_pevCurrentTarget->message), this, this, USE_TOGGLE, 0);
		if (FBitSet(m_pevCurrentTarget->spawnflags, SF_CORNER_FIREONCE))
			m_pevCurrentTarget->message = 0;
	}

	// need pointer to LAST target.
	if (FBitSet(m_pevCurrentTarget->spawnflags, SF_TRAIN_WAIT_RETRIGGER) || (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER))
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
		// clear the sound channel.
		if (pev->plat_noiseMoving)
			STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving));
		if (pev->plat_noiseArrived)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->plat_noiseArrived), m_volume, ATTN_NORM);
		pev->nextthink = 0;
		return;
	}

	// ALERT ( at_console, "%f\n", m_flWait );

	if (m_flWait != 0)
	{// -1 wait will wait forever!		
		pev->nextthink = pev->ltime + m_flWait;
		if (pev->plat_noiseMoving)
			STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving));
		if (pev->plat_noiseArrived)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->plat_noiseArrived), m_volume, ATTN_NORM);
		SetThink(&CFuncTrain::Next);
	}
	else
	{
		Next();// do it RIGHT now!
	}
}


//
// Train next - path corner needs to change to next target 
//
void CFuncTrain::Next(void)
{
	CBaseEntity* pTarg;


	// now find our next target
	pTarg = GetNextTarget();

	if (!pTarg)
	{
		if (pev->plat_noiseMoving)
			STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving));
		// Play stop sound
		if (pev->plat_noiseArrived)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->plat_noiseArrived), m_volume, ATTN_NORM);
		return;
	}

	if (m_lastNext == gpGlobals->time) {
		// prevent infinite recursion (path_corner targeting itself with no distance to move)
		// Some recursion is needed for hl_u12_a5. Adding even one frame of delay totally breaks the machine.
		// TODO: which map crashed due to recursion?
		if (m_nextRecursions++ > 32) {
			ALERT(at_console, "Exceeded max func_train Next() recursion! %s -> %s\n", STRING(pev->targetname), STRING(m_pevCurrentTarget->targetname));
			return;
		}
	}
	else {
		m_nextRecursions = 0;
	}
	m_lastNext = gpGlobals->time;

	// Save last target in case we need to find it again
	pev->message = pev->target;

	pev->target = pTarg->pev->target;
	m_flWait = pTarg->GetDelay();

	if (m_pevCurrentTarget && m_pevCurrentTarget->speed != 0)
	{// don't copy speed from target if it is 0 (uninitialized)
		pev->speed = m_pevCurrentTarget->speed;
		ALERT(at_aiconsole, "Train %s speed to %4.2f\n", STRING(pev->targetname), pev->speed);
	}
	m_previousTarget = ENT(m_pevCurrentTarget);
	m_pevCurrentTarget = pTarg->pev;// keep track of this since path corners change our target for us.

	pev->enemy = pTarg->edict();//hack

	if (FBitSet(m_pevCurrentTarget->spawnflags, SF_CORNER_TELEPORT))
	{
		// Path corner has indicated a teleport to the next corner.
		SetBits(pev->effects, EF_NOINTERP);
		UTIL_SetOrigin(pev, pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5);
		Wait(); // Get on with doing the next path corner.
	}
	else
	{
		// Normal linear move.

		// CHANGED this from CHAN_VOICE to CHAN_STATIC around OEM beta time because trains should
		// use CHAN_STATIC for their movement sounds to prevent sound field problems.
		// this is not a hack or temporary fix, this is how things should be. (sjb).
		if (pev->plat_noiseMoving)
			STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving));
		if (pev->plat_noiseMoving)
			EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->plat_noiseMoving), m_volume, ATTN_NORM);
		ClearBits(pev->effects, EF_NOINTERP);
		SetMoveDone(&CFuncTrain::Wait);
		LinearMove(pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5, pev->speed);
	}
}


void CFuncTrain::Activate(void)
{
	// Not yet active, so teleport to first target
	if (!m_activated)
	{
		m_activated = 1;
		entvars_t* pevTarg = VARS(FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target)));

		pev->target = pevTarg->target;
		m_pevCurrentTarget = pevTarg;// keep track of this since path corners change our target for us.

		UTIL_SetOrigin(pev, pevTarg->origin - (pev->mins + pev->maxs) * 0.5);

		if (FStringNull(pev->targetname))
		{	// not triggered, so start immediately
			pev->nextthink = pev->ltime + 0.1;
			SetThink(&CFuncTrain::Next);
		}
		else
			pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
	}
}

/*QUAKED func_train (0 .5 .8) ?
Trains are moving platforms that players can ride.
The targets origin specifies the min point of the train at each corner.
The train spawns at the first target it is pointing at.
If the train is the target of a button or trigger, it will not begin moving until activated.
speed	default 100
dmg		default	2
sounds
1) ratchet metal
*/

void CFuncTrain::Spawn(void)
{
	Precache();
	if (pev->speed == 0)
		pev->speed = 100;

	if (FStringNull(pev->target))
		ALERT(at_console, "FuncTrain with no target\n");

	if (pev->dmg == 0)
		pev->dmg = 2;

	pev->movetype = MOVETYPE_PUSH; // required for LinearMove (ltime)

	if (FClassnameIs(pev, "env_spritetrain")) {
		pev->solid = SOLID_NOT;

		if (pev->rendermode == 0) {
			pev->rendermode = kRenderTransAdd;
		}
		if (pev->renderamt == 0) {
			pev->renderamt = 200; // just a guess, should check exact value in sven
		}
	}
	else {
		if (FBitSet(pev->spawnflags, SF_TRACKTRAIN_PASSABLE))
			pev->solid = SOLID_NOT;
		else
			pev->solid = SOLID_BSP;
	}

	

	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	m_activated = 0;

	if (m_volume == 0)
		m_volume = 0.85;
}


void CFuncTrain::Precache(void)
{
	CBasePlatTrain::Precache();

	if (FClassnameIs(pev, "env_spritetrain")) {
		PRECACHE_MODEL(STRING(pev->model));
	}

#if 0  // obsolete
	// otherwise use preset sound
	switch (m_sounds)
	{
	case 0:
		pev->noise = 0;
		pev->noise1 = 0;
		break;

	case 1:
		PRECACHE_SOUND("plats/train2.wav");
		PRECACHE_SOUND("plats/train1.wav");
		pev->noise = MAKE_STRING("plats/train2.wav");
		pev->noise1 = MAKE_STRING("plats/train1.wav");
		break;

	case 2:
		PRECACHE_SOUND("plats/platmove1.wav");
		PRECACHE_SOUND("plats/platstop1.wav");
		pev->noise = MAKE_STRING("plats/platstop1.wav");
		pev->noise1 = MAKE_STRING("plats/platmove1.wav");
		break;
	}
#endif
}


void CFuncTrain::OverrideReset(void)
{
	CBaseEntity* pTarg;

	// Are we moving?
	if (pev->velocity != g_vecZero && pev->nextthink != 0)
	{
		pev->target = pev->message;
		// now find our next target
		pTarg = GetNextTarget();
		if (!pTarg)
		{
			pev->nextthink = 0;
			pev->velocity = g_vecZero;
		}
		else	// Keep moving for 0.1 secs, then find path_corner again and restart
		{
			SetThink(&CFuncTrain::Next);
			pev->nextthink = pev->ltime + 0.1;
		}
	}
}

