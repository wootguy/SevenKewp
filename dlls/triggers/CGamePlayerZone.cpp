#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CRuleEntity.h"

//
// CGamePlayerZone / game_player_zone -- players in the zone fire my target when I'm fired
//
// Needs master?
class CGamePlayerZone : public CRuleBrushEntity
{
public:
	void		KeyValue(KeyValueData* pkvd);
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

private:
	string_t	m_iszInTarget;
	string_t	m_iszOutTarget;
	string_t	m_iszInCount;
	string_t	m_iszOutCount;
};

LINK_ENTITY_TO_CLASS(game_zone_player, CGamePlayerZone)
TYPEDESCRIPTION	CGamePlayerZone::m_SaveData[] =
{
	DEFINE_FIELD(CGamePlayerZone, m_iszInTarget, FIELD_STRING),
	DEFINE_FIELD(CGamePlayerZone, m_iszOutTarget, FIELD_STRING),
	DEFINE_FIELD(CGamePlayerZone, m_iszInCount, FIELD_STRING),
	DEFINE_FIELD(CGamePlayerZone, m_iszOutCount, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CGamePlayerZone, CRuleBrushEntity)

void CGamePlayerZone::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "intarget"))
	{
		m_iszInTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "outtarget"))
	{
		m_iszOutTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "incount"))
	{
		m_iszInCount = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "outcount"))
	{
		m_iszOutCount = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CRuleBrushEntity::KeyValue(pkvd);
}

void CGamePlayerZone::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int playersInCount = 0;
	int playersOutCount = 0;

	if (!CanFireForActivator(pActivator))
		return;

	CBaseEntity* pPlayer = NULL;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		pPlayer = UTIL_PlayerByIndex(i);
		if (pPlayer)
		{
			TraceResult trace;
			int			hullNumber;

			hullNumber = human_hull;
			if (pPlayer->pev->flags & FL_DUCKING)
			{
				hullNumber = head_hull;
			}

			UTIL_TraceModel(pPlayer->pev->origin, pPlayer->pev->origin, hullNumber, edict(), &trace);

			if (trace.fStartSolid)
			{
				playersInCount++;
				if (m_iszInTarget)
				{
					FireTargets(STRING(m_iszInTarget), pPlayer, pActivator, useType, value);
				}
			}
			else
			{
				playersOutCount++;
				if (m_iszOutTarget)
				{
					FireTargets(STRING(m_iszOutTarget), pPlayer, pActivator, useType, value);
				}
			}
		}
	}

	if (m_iszInCount)
	{
		FireTargets(STRING(m_iszInCount), pActivator, this, USE_SET, playersInCount);
	}

	if (m_iszOutCount)
	{
		FireTargets(STRING(m_iszOutCount), pActivator, this, USE_SET, playersOutCount);
	}
}

