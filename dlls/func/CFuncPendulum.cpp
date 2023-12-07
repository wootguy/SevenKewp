#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBaseDoor.h"

#define		SF_PENDULUM_SWING		2	// spawnflag that makes a pendulum a rope swing.

class CPendulum : public CBaseEntity
{
public:
	void	Spawn(void);
	void	KeyValue(KeyValueData* pkvd);
	void	EXPORT Swing(void);
	void	EXPORT PendulumUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	EXPORT Stop(void);
	void	Touch(CBaseEntity* pOther);
	void	EXPORT RopeTouch(CBaseEntity* pOther);// this touch func makes the pendulum a rope
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	void	Blocked(CBaseEntity* pOther);

	static	TYPEDESCRIPTION m_SaveData[];

	float	m_accel;			// Acceleration
	float	m_distance;			// 
	float	m_time;
	float	m_damp;
	float	m_maxSpeed;
	float	m_dampSpeed;
	vec3_t	m_center;
	vec3_t	m_start;
};

LINK_ENTITY_TO_CLASS(func_pendulum, CPendulum);

TYPEDESCRIPTION	CPendulum::m_SaveData[] =
{
	DEFINE_FIELD(CPendulum, m_accel, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_distance, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_time, FIELD_TIME),
	DEFINE_FIELD(CPendulum, m_damp, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_maxSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_dampSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CPendulum, m_center, FIELD_VECTOR),
	DEFINE_FIELD(CPendulum, m_start, FIELD_VECTOR),
};

IMPLEMENT_SAVERESTORE(CPendulum, CBaseEntity);



void CPendulum::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_distance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damp"))
	{
		m_damp = atof(pkvd->szValue) * 0.001;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}


void CPendulum::Spawn(void)
{
	// set the axis of rotation
	CBaseToggle::AxisDir(pev);

	if (FBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (m_distance == 0)
		return;

	if (pev->speed == 0)
		pev->speed = 100;

	m_accel = (pev->speed * pev->speed) / (2 * fabs(m_distance));	// Calculate constant acceleration from speed and distance
	m_maxSpeed = pev->speed;
	m_start = pev->angles;
	m_center = pev->angles + (m_distance * 0.5) * pev->movedir;

	if (FBitSet(pev->spawnflags, SF_BRUSH_ROTATE_INSTANT))
	{
		SetThink(&CPendulum::SUB_CallUseToggle);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	pev->speed = 0;
	SetUse(&CPendulum::PendulumUse);

	if (FBitSet(pev->spawnflags, SF_PENDULUM_SWING))
	{
		SetTouch(&CPendulum::RopeTouch);
	}
}


void CPendulum::PendulumUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pev->speed)		// Pendulum is moving, stop it and auto-return if necessary
	{
		if (FBitSet(pev->spawnflags, SF_PENDULUM_AUTO_RETURN))
		{
			float	delta;

			delta = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_start);

			pev->avelocity = m_maxSpeed * pev->movedir;
			pev->nextthink = pev->ltime + (delta / m_maxSpeed);
			SetThink(&CPendulum::Stop);
		}
		else
		{
			pev->speed = 0;		// Dead stop
			SetThink(NULL);
			pev->avelocity = g_vecZero;
		}
	}
	else
	{
		pev->nextthink = pev->ltime + 0.1;		// Start the pendulum moving
		m_time = gpGlobals->time;		// Save time to calculate dt
		SetThink(&CPendulum::Swing);
		m_dampSpeed = m_maxSpeed;
	}
}


void CPendulum::Stop(void)
{
	pev->angles = m_start;
	pev->speed = 0;
	SetThink(NULL);
	pev->avelocity = g_vecZero;
}


void CPendulum::Blocked(CBaseEntity* pOther)
{
	if (pOther->IsMonster() && !pOther->IsAlive()) {
		// don't let corpses block anything
		pOther->Killed(pev, GIB_ALWAYS);
	}
	m_time = gpGlobals->time;
}


void CPendulum::Swing(void)
{
	float delta, dt;

	delta = CBaseToggle::AxisDelta(pev->spawnflags, pev->angles, m_center);
	dt = gpGlobals->time - m_time;	// How much time has passed?
	m_time = gpGlobals->time;		// Remember the last time called

	if (delta > 0 && m_accel > 0)
		pev->speed -= m_accel * dt;	// Integrate velocity
	else
		pev->speed += m_accel * dt;

	if (pev->speed > m_maxSpeed)
		pev->speed = m_maxSpeed;
	else if (pev->speed < -m_maxSpeed)
		pev->speed = -m_maxSpeed;
	// scale the destdelta vector by the time spent traveling to get velocity
	pev->avelocity = pev->speed * pev->movedir;

	// Call this again
	pev->nextthink = pev->ltime + 0.1;

	if (m_damp)
	{
		m_dampSpeed -= m_damp * m_dampSpeed * dt;
		if (m_dampSpeed < 30.0)
		{
			pev->angles = m_center;
			pev->speed = 0;
			SetThink(NULL);
			pev->avelocity = g_vecZero;
		}
		else if (pev->speed > m_dampSpeed)
			pev->speed = m_dampSpeed;
		else if (pev->speed < -m_dampSpeed)
			pev->speed = -m_dampSpeed;

	}
}


void CPendulum::Touch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	if (pev->dmg <= 0)
		return;

	// we can't hurt this thing, so we're not concerned with it
	if (!pevOther->takedamage)
		return;

	// calculate damage based on rotation speed
	float damage = pev->dmg * pev->speed * 0.01;

	if (damage < 0)
		damage = -damage;

	pOther->TakeDamage(pev, pev, damage, DMG_CRUSH);

	pevOther->velocity = (pevOther->origin - VecBModelOrigin(pev)).Normalize() * damage;
}

void CPendulum::RopeTouch(CBaseEntity* pOther)
{
	entvars_t* pevOther = pOther->pev;

	if (!pOther->IsPlayer())
	{// not a player!
		ALERT(at_console, "Not a client\n");
		return;
	}

	if (ENT(pevOther) == pev->enemy)
	{// this player already on the rope.
		return;
	}

	pev->enemy = pOther->edict();
	pevOther->velocity = g_vecZero;
	pevOther->movetype = MOVETYPE_NONE;
}

