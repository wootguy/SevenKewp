#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "trains.h"
#include "saverestore.h"
#include "CBasePlatTrain.h"
#include "CFuncPlat.h"
#include "CFuncPlatRot.h"
#include "CFuncTrackChange.h"
#include "path/CPathCorner.h"

// ----------------------------------------------------------
//
//
// pev->speed is the travel speed
// pev->health is current health
// pev->max_health is the amount to reset to each time it starts

#define FGUNTARGET_START_ON			0x0001

class CGunTarget : public CBaseMonster
{
public:
	void			Spawn(void);
	void			Activate(void);
	void EXPORT		Next(void);
	void EXPORT		Start(void);
	void EXPORT		Wait(void);
	void			Stop(void);

	int				BloodColor(void) { return DONT_BLEED; }
	int				Classify(void) { return CLASS_MACHINE; }
	int				TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void			Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	Vector			BodyTarget(const Vector& posSrc) { return pev->origin; }

	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

private:
	BOOL			m_on;
};


LINK_ENTITY_TO_CLASS(func_guntarget, CGunTarget);

TYPEDESCRIPTION	CGunTarget::m_SaveData[] =
{
	DEFINE_FIELD(CGunTarget, m_on, FIELD_BOOLEAN),
};

IMPLEMENT_SAVERESTORE(CGunTarget, CBaseMonster);


void CGunTarget::Spawn(void)
{
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;

	// Don't take damage until "on"
	pev->takedamage = DAMAGE_NO;
	pev->flags |= FL_MONSTER;

	m_on = FALSE;
	pev->max_health = pev->health;

	if (pev->spawnflags & FGUNTARGET_START_ON)
	{
		SetThink(&CGunTarget::Start);
		pev->nextthink = pev->ltime + 0.3;
	}
}


void CGunTarget::Activate(void)
{
	CBaseEntity* pTarg;

	// now find our next target
	pTarg = GetNextTarget();
	if (pTarg)
	{
		m_hTargetEnt = pTarg;
		UTIL_SetOrigin(pev, pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5);
	}
}


void CGunTarget::Start(void)
{
	Use(this, this, USE_ON, 0);
}


void CGunTarget::Next(void)
{
	SetThink(NULL);

	m_hTargetEnt = GetNextTarget();
	CBaseEntity* pTarget = m_hTargetEnt;

	if (!pTarget)
	{
		Stop();
		return;
	}
	SetMoveDone(&CGunTarget::Wait);
	LinearMove(pTarget->pev->origin - (pev->mins + pev->maxs) * 0.5, pev->speed);
}


void CGunTarget::Wait(void)
{
	CBaseEntity* pTarget = m_hTargetEnt;

	if (!pTarget)
	{
		Stop();
		return;
	}

	// Fire the pass target if there is one
	if (pTarget->pev->message)
	{
		FireTargets(STRING(pTarget->pev->message), this, this, USE_TOGGLE, 0);
		if (FBitSet(pTarget->pev->spawnflags, SF_CORNER_FIREONCE))
			pTarget->pev->message = 0;
	}

	m_flWait = pTarget->GetDelay();

	pev->target = pTarget->pev->target;
	SetThink(&CGunTarget::Next);
	if (m_flWait != 0)
	{// -1 wait will wait forever!		
		pev->nextthink = pev->ltime + m_flWait;
	}
	else
	{
		Next();// do it RIGHT now!
	}
}


void CGunTarget::Stop(void)
{
	pev->velocity = g_vecZero;
	pev->nextthink = 0;
	pev->takedamage = DAMAGE_NO;
}


int	CGunTarget::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (pev->health > 0)
	{
		pev->health -= flDamage;
		if (pev->health <= 0)
		{
			pev->health = 0;
			Stop();
			if (pev->message)
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
		}
	}
	return 0;
}


void CGunTarget::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_on))
		return;

	if (m_on)
	{
		Stop();
	}
	else
	{
		pev->takedamage = DAMAGE_AIM;
		m_hTargetEnt = GetNextTarget();
		if (m_hTargetEnt == NULL)
			return;
		pev->health = pev->max_health;
		Next();
	}
}

