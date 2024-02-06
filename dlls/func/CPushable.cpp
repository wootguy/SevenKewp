#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "CBreakable.h"
#include "decals.h"
#include "explode.h"
#include "CBasePlayer.h"

// how fast players can move pushables by touch/use
#define PUSH_SPEED 100

// how often to apply push vectors
#define PUSH_THINK_DELAY 0.05f

// how fast pushables track the lifting position
#define PUSH_LIFT_SPEED 20

// max movement speed of pushable in lift mode
#define PUSH_LIFT_MAX_SPEED 400

class CPushable : public CBreakable
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	Touch(CBaseEntity* pOther);
	void	Move();
	void	Lift();
	void	StopLift();
	void	StartLift(CBasePlayer* lifter);
	void	UpdatePushDir(CBaseEntity* pMover, int push);
	void	KeyValue(KeyValueData* pkvd);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	EXPORT StopSound(void);
	void	EXPORT MoveThink(void);
	void    PostMove(bool clampSpeed); // clamps velocity and plays movement sounds
	//	virtual void	SetActivator( CBaseEntity *pActivator ) { m_pPusher = pActivator; }
	BOOL	IsBreakable() { return pev->spawnflags & SF_PUSH_BREAKABLE; }

	virtual int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE; }
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	inline float MaxSpeed(void) { return m_maxSpeed; }

	// breakables use an overridden takedamage
	virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	static	TYPEDESCRIPTION m_SaveData[];

	static const char* m_soundNames[3];
	int		m_lastSound;	// no need to save/restore, just keeps the same sound from playing twice in a row
	float	m_maxSpeed;
	float	m_soundTime;
	float   m_lastMove;

	Vector m_playerPushDir[32]; // push direction from every player touching/using the pushable
	Vector m_entPushDir; // push direction from other entities
	bool m_wasPushed;

	bool m_ignoreLiftUse[32]; // true if use key was held while moving
	EHANDLE m_hLifter; // player who is lifting the pushable
};

TYPEDESCRIPTION	CPushable::m_SaveData[] =
{
	DEFINE_FIELD(CPushable, m_maxSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CPushable, m_soundTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CPushable, CBreakable);

LINK_ENTITY_TO_CLASS(func_pushable, CPushable);

const char* CPushable::m_soundNames[3] = { "debris/pushbox1.wav", "debris/pushbox2.wav", "debris/pushbox3.wav" };


void CPushable::Spawn(void)
{
	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		CBreakable::Spawn();
	else
		Precache();

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BBOX;
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->friction > 399)
		pev->friction = 399;

	m_maxSpeed = V_max(10, 400 - pev->friction); // pushables should always be moveable
	SetBits(pev->flags, FL_FLOAT);
	pev->friction = 0;

	pev->origin.z += 1;	// Pick up off of the floor
	UTIL_SetOrigin(pev, pev->origin);

	SetThink(&CPushable::MoveThink);
	pev->nextthink = gpGlobals->time;

	// Multiply by area of the box's cross-section (assume 1000 units^3 standard volume)
	pev->skin = (pev->skin * (pev->maxs.x - pev->mins.x) * (pev->maxs.y - pev->mins.y)) * 0.0005;
	m_soundTime = 0;
}


void CPushable::Precache(void)
{
	for (int i = 0; i < 3; i++)
		PRECACHE_SOUND(m_soundNames[i]);

	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		CBreakable::Precache();
}


void CPushable::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "size"))
	{
		int bbox = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		switch (bbox)
		{
		case 0:	// Point
			UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));
			break;

		case 2: // Big Hull!?!?	!!!BUGBUG Figure out what this hull really is
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN * 2, VEC_DUCK_HULL_MAX * 2);
			break;

		case 3: // Player duck
			UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
			break;

		default:
		case 1: // Player
			UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
			break;
		}

	}
	else if (FStrEq(pkvd->szKeyName, "buoyancy"))
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBreakable::KeyValue(pkvd);
}


// Pull the func_pushable
void CPushable::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pActivator || !pActivator->IsPlayer())
	{
		if (pev->spawnflags & SF_PUSH_BREAKABLE)
			this->CBreakable::Use(pActivator, pCaller, useType, value);
		return;
	}

	if (pev->spawnflags & SF_PUSH_LIFTABLE) {
		int eidx = pActivator->entindex() % 32;
		CBasePlayer* plr = (CBasePlayer*)pActivator;
		bool isMoving = plr->pev->button & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		bool releasedUseKey = value == 0;
		bool notHoldingOtherPushable = !plr->m_pPushable || plr->m_pPushable.GetEntity() == this;
		bool wasMovingWithUse = m_ignoreLiftUse[eidx] || isMoving;
		bool isCurrentLifter = m_hLifter.GetEntity() == plr;

		if (isMoving && !releasedUseKey) {
			// player wants to move the pushable normally, so don't lift after the move finishes
			m_ignoreLiftUse[eidx] = true;
		}

		if (releasedUseKey && (!wasMovingWithUse || isCurrentLifter) && notHoldingOtherPushable) {
			if (m_hLifter) {
				CBasePlayer* oldLifter = (CBasePlayer*)m_hLifter.GetEntity();
				StopLift();

				if (oldLifter != plr) {
					// stealing from someone else
					StartLift(plr);
				}
			}
			else {
				StartLift(plr);
			}
		}

		if (releasedUseKey) {
			m_ignoreLiftUse[eidx] = false;
		}
	}

	if (pActivator->pev->velocity != g_vecZero) {
		UpdatePushDir(pActivator, 0);
	}
}


void CPushable::Touch(CBaseEntity* pOther)
{
	if (FClassnameIs(pOther->pev, "worldspawn"))
		return;

	if (pOther->IsMonster() && pOther->pev->deadflag == DEAD_DEAD) {
		// don't let corpses block pushables
		pOther->Killed(pev, GIB_ALWAYS);
	}

	UpdatePushDir(pOther, 1);
}

void CPushable::MoveThink() {
	if (!m_hLifter && pev->movetype == MOVETYPE_FLY) {
		StopLift();
	}

	if (m_hLifter) {
		Lift();
	}
	else if (m_wasPushed && !mp_objectboost.value)
		Move();

	m_lastMove = gpGlobals->time;

	if (pev->velocity == g_vecZero && (pev->flags & FL_ONGROUND) && !FNullEnt(pev->groundentity)) {
		// don't let velocity stay at 0 in case ground entity is moved out from under the pushable,
		// otherwise it will float in the air.
		pev->velocity.z += 0.000001f;
	}

	// push forces are applied at a constant delay to prevent clients with high FPS
	// launching pushables into space.
	pev->nextthink = gpGlobals->time + PUSH_THINK_DELAY;
}

void CPushable::UpdatePushDir(CBaseEntity* pOther, int push) {
	entvars_t* pevToucher = pOther->pev;
	int playerTouch = 0;

	m_wasPushed = true;

	int eidx = pOther->entindex() % 32;
	Vector& pushDir = pOther->IsPlayer() ? m_playerPushDir[eidx] : m_entPushDir;

	float factor = 0.25f;

	// Is entity standing on this pushable ?
	if (FBitSet(pevToucher->flags, FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev)
	{
		// Only push if floating
		if (pev->waterlevel > 0)
			pushDir.z += pevToucher->velocity.z * 0.1;

		goto objectboost;
	}

	if (pOther->IsPlayer())
	{
		if (push && !(pevToucher->button & (IN_FORWARD | IN_USE)))	// Don't push unless the player is pushing forward and NOT use (pull)
			goto objectboost;
		playerTouch = 1;
	}

	if (playerTouch)
	{
		if (!(pevToucher->flags & FL_ONGROUND))
		{
			// Don't push away from jumping/falling players unless in water or if the
			// pushable glitched up a wall and someone is trying to pull it down
			if (pev->waterlevel >= 1)
				factor = 0.1;
			else if (pevToucher->absmax.z < pev->absmin.z)
				factor = 1;
			else
				goto objectboost;
		}
		else
			factor = 1;
	}

	pushDir.x += pevToucher->velocity.x * factor;
	pushDir.y += pevToucher->velocity.y * factor;

objectboost:
	if (mp_objectboost.value) {
		// classic broken physics which adds uncapped velocity every client frame
		// which gives extreme speed boosts at high FPS, both to the pushable and player.
		pev->velocity = pev->velocity + pushDir;

		if (playerTouch) {
			pevToucher->velocity.x = pev->velocity.x;
			pevToucher->velocity.y = pev->velocity.y;
		}

		pushDir = g_vecZero;
		PostMove(false);
	}
}

void CPushable::Move()
{
	m_wasPushed = false;

	Vector combinedPushDir = m_entPushDir;

	for (int i = 0; i < 32; i++) {
		if (m_playerPushDir[i] == g_vecZero) {
			continue;
		}

		combinedPushDir = combinedPushDir + m_playerPushDir[i].Normalize();
	}

	if (combinedPushDir == g_vecZero) {
		return;
	}

	float deltaTime = clampf(gpGlobals->time - m_lastMove, 0, 0.5f);
	float timeScale = deltaTime / PUSH_THINK_DELAY;

	combinedPushDir = combinedPushDir.Normalize() * PUSH_SPEED * timeScale;

	pev->velocity = pev->velocity + combinedPushDir;

	memset(&m_playerPushDir, 0, sizeof(Vector) * 32);
	m_entPushDir = g_vecZero;

	PostMove(true);
}

void CPushable::PostMove(bool clampSpeed) {
	float length = sqrt(pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y);
	if (clampSpeed && length > MaxSpeed())
	{
		pev->velocity.x = (pev->velocity.x * MaxSpeed() / length);
		pev->velocity.y = (pev->velocity.y * MaxSpeed() / length);
	}

	if ((gpGlobals->time - m_soundTime) > 0.7)
	{
		m_soundTime = gpGlobals->time;
		if (length > 10 && FBitSet(pev->flags, FL_ONGROUND))
		{
			m_lastSound = RANDOM_LONG(0, 2);
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound], 0.5, ATTN_NORM);
			//			SetThink( StopSound );
			//			pev->nextthink = pev->ltime + 0.1;
		}
		else
			STOP_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound]);
	}
}

void CPushable::Lift() {
	CBasePlayer* plr = (CBasePlayer*)m_hLifter.GetEntity();

	Vector min = pev->absmin;
	Vector max = pev->absmax;
	Vector size = max - min;
	Vector center = min + size * 0.5f;

	float maxObjectDiag = (Vector(size.x, size.y, size.z) * 0.5f).Length();
	float maxPlrDiag = Vector(16, 16, 36).Length();
	float liftDist = (maxObjectDiag + maxPlrDiag);

	UTIL_MakeVectors(plr->pev->v_angle + plr->pev->punchangle);
	Vector targetPos = plr->GetGunPosition() + gpGlobals->v_forward * liftDist;
	Vector deltaPos = targetPos - center;

	if (!m_hLifter->IsAlive() || deltaPos.Length() > 64) {
		StopLift();
		return;
	}

	float deltaTime = clampf(gpGlobals->time - m_lastMove, 0, 0.5f);
	float timeScale = deltaTime / PUSH_THINK_DELAY;

	pev->velocity = deltaPos * PUSH_LIFT_SPEED * timeScale;

	if (pev->velocity.Length() > PUSH_LIFT_MAX_SPEED) {
		pev->velocity = pev->velocity.Normalize() * PUSH_LIFT_MAX_SPEED;
	}
}

void CPushable::StopLift() {
	if (m_hLifter) {
		CBasePlayer* plr = (CBasePlayer*)m_hLifter.GetEntity();
		plr->m_pPushable = NULL;
	}
	m_hLifter = NULL;
	pev->movetype = MOVETYPE_PUSHSTEP;
}

void CPushable::StartLift(CBasePlayer* lifter) {
	lifter->m_pPushable = this;
	m_hLifter = lifter;
	pev->movetype = MOVETYPE_FLY;
}

#if 0
void CPushable::StopSound(void)
{
	Vector dist = pev->oldorigin - pev->origin;
	if (dist.Length() <= 0)
		STOP_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound]);
}
#endif

int CPushable::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		return CBreakable::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);

	return 1;
}
