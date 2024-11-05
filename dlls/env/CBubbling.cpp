#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "customentity.h"
#include "effects.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"

class CBubbling : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	KeyValue(KeyValueData* pkvd);

	void	EXPORT FizzThink(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	virtual int		ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static	TYPEDESCRIPTION m_SaveData[];

	int		m_density;
	int		m_frequency;
	int		m_bubbleModel;
	int		m_state;
};

LINK_ENTITY_TO_CLASS(env_bubbles, CBubbling)

TYPEDESCRIPTION	CBubbling::m_SaveData[] =
{
	DEFINE_FIELD(CBubbling, m_density, FIELD_INTEGER),
	DEFINE_FIELD(CBubbling, m_frequency, FIELD_INTEGER),
	DEFINE_FIELD(CBubbling, m_state, FIELD_INTEGER),
	// Let spawn restore this!
	//	DEFINE_FIELD( CBubbling, m_bubbleModel, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE(CBubbling, CBaseEntity)


#define SF_BUBBLES_STARTOFF		0x0001

void CBubbling::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model));		// Set size

	pev->solid = SOLID_NOT;							// Remove model & collisions
	pev->renderamt = 0;								// The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;
	int speed = pev->speed > 0 ? pev->speed : -pev->speed;

	// HACKHACK!!! - Speed in rendercolor
	pev->rendercolor.x = speed >> 8;
	pev->rendercolor.y = speed & 255;
	pev->rendercolor.z = (pev->speed < 0) ? 1 : 0;


	if (!(pev->spawnflags & SF_BUBBLES_STARTOFF))
	{
		SetThink(&CBubbling::FizzThink);
		pev->nextthink = gpGlobals->time + 2.0;
		m_state = 1;
	}
	else
		m_state = 0;
}

void CBubbling::Precache(void)
{
	m_bubbleModel = PRECACHE_MODEL("sprites/bubble.spr");			// Precache bubble sprite
}


void CBubbling::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (ShouldToggle(useType, m_state))
		m_state = !m_state;

	if (m_state)
	{
		SetThink(&CBubbling::FizzThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		SetThink(NULL);
		pev->nextthink = 0;
	}
}


void CBubbling::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "density"))
	{
		m_density = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "frequency"))
	{
		m_frequency = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "current"))
	{
		pev->speed = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}


void CBubbling::FizzThink(void)
{
	UTIL_Fizz(entindex(), m_bubbleModel, m_density, MSG_PAS, VecBModelOrigin(pev));

	if (m_frequency > 19)
		pev->nextthink = gpGlobals->time + 0.5;
	else
		pev->nextthink = gpGlobals->time + 2.5 - (0.1 * m_frequency);
}
