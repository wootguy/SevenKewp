#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "customentity.h"
#include "effects.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"

#define SF_FUNNEL_REVERSE			1 // funnel effect repels particles instead of attracting them.

//=========================================================
// FunnelEffect
//=========================================================
class CEnvFunnel : public CBaseDelay
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	int		m_iSprite;	// Don't save, precache
};

void CEnvFunnel::Precache(void)
{
	m_iSprite = PRECACHE_MODEL("sprites/flare6.spr");
}

LINK_ENTITY_TO_CLASS(env_funnel, CEnvFunnel);

void CEnvFunnel::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_LARGEFUNNEL);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(m_iSprite);

	if (pev->spawnflags & SF_FUNNEL_REVERSE)// funnel flows in reverse?
	{
		WRITE_SHORT(1);
	}
	else
	{
		WRITE_SHORT(0);
	}


	MESSAGE_END();

	SetThink(&CEnvFunnel::SUB_Remove);
	pev->nextthink = gpGlobals->time;
}

void CEnvFunnel::Spawn(void)
{
	Precache();
	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
}
