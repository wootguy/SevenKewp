#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "CBreakable.h"
#include "decals.h"
#include "explode.h"

class CPushable : public CBreakable
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	Touch(CBaseEntity* pOther);
	void	Move(CBaseEntity* pMover, int push);
	void	KeyValue(KeyValueData* pkvd);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	EXPORT StopSound(void);
	//	virtual void	SetActivator( CBaseEntity *pActivator ) { m_pPusher = pActivator; }

	virtual int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_CONTINUOUS_USE; }
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

	m_maxSpeed = 400 - pev->friction;
	SetBits(pev->flags, FL_FLOAT);
	pev->friction = 0;

	pev->origin.z += 1;	// Pick up off of the floor
	UTIL_SetOrigin(pev, pev->origin);

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

	if (pActivator->pev->velocity != g_vecZero)
		Move(pActivator, 0);
}


void CPushable::Touch(CBaseEntity* pOther)
{
	if (FClassnameIs(pOther->pev, "worldspawn"))
		return;

	Move(pOther, 1);
}


void CPushable::Move(CBaseEntity* pOther, int push)
{
	entvars_t* pevToucher = pOther->pev;
	int playerTouch = 0;

	// Is entity standing on this pushable ?
	if (FBitSet(pevToucher->flags, FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev)
	{
		// Only push if floating
		if (pev->waterlevel > 0)
			pev->velocity.z += pevToucher->velocity.z * 0.1;

		return;
	}


	if (pOther->IsPlayer())
	{
		if (push && !(pevToucher->button & (IN_FORWARD | IN_USE)))	// Don't push unless the player is pushing forward and NOT use (pull)
			return;
		playerTouch = 1;
	}

	float factor;

	if (playerTouch)
	{
		if (!(pevToucher->flags & FL_ONGROUND))	// Don't push away from jumping/falling players unless in water
		{
			if (pev->waterlevel < 1)
				return;
			else
				factor = 0.1;
		}
		else
			factor = 1;
	}
	else
		factor = 0.25;

	pev->velocity.x += pevToucher->velocity.x * factor;
	pev->velocity.y += pevToucher->velocity.y * factor;

	float length = sqrt(pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y);
	if (push && (length > MaxSpeed()))
	{
		pev->velocity.x = (pev->velocity.x * MaxSpeed() / length);
		pev->velocity.y = (pev->velocity.y * MaxSpeed() / length);
	}
	if (playerTouch)
	{
		pevToucher->velocity.x = pev->velocity.x;
		pevToucher->velocity.y = pev->velocity.y;
		if ((gpGlobals->time - m_soundTime) > 0.7)
		{
			m_soundTime = gpGlobals->time;
			if (length > 0 && FBitSet(pev->flags, FL_ONGROUND))
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
