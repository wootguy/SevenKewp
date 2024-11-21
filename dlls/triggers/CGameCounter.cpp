#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "CBaseEntity.h"
#include "CRuleEntity.h"

//
// CGameCounter / game_counter	-- Counts events and fires target
// Flag: Fire once
// Flag: Reset on Fire

#define SF_GAMECOUNT_FIREONCE			0x0001
#define SF_GAMECOUNT_RESET				0x0002

class CGameCounter : public CRulePointEntity
{
public:
	void		Spawn(void);
	void		KeyValue(KeyValueData* pkvd);
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	inline BOOL RemoveOnFire(void) { return (pev->spawnflags & SF_GAMECOUNT_FIREONCE) ? TRUE : FALSE; }
	inline BOOL ResetOnFire(void) { return (pev->spawnflags & SF_GAMECOUNT_RESET) ? TRUE : FALSE; }

	inline void CountUp(void) { pev->frags++; }
	inline void CountDown(void) { pev->frags--; }
	inline void ResetCount(void) { pev->frags = pev->dmg; }
	inline int  CountValue(void) { return pev->frags; }
	inline int	LimitValue(void) { return pev->health; }

	inline BOOL HitLimit(void) { return CountValue() == LimitValue(); }

	string_t m_killtarget;

private:

	inline void SetCountValue(int value) { pev->frags = value; }
	inline void SetInitialValue(int value) { pev->dmg = value; }
};

LINK_ENTITY_TO_CLASS(game_counter, CGameCounter)

void CGameCounter::Spawn(void)
{
	// Save off the initial count
	SetInitialValue(CountValue());
	CRulePointEntity::Spawn();
}

void CGameCounter::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "killtarget"))
	{
		m_killtarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}


void CGameCounter::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	switch (useType)
	{
	case USE_ON:
	case USE_TOGGLE:
		CountUp();
		break;

	case USE_OFF:
		CountDown();
		break;

	case USE_SET:
		SetCountValue((int)value);
		break;
	}

	if (HitLimit())
	{
		SUB_UseTargets(pActivator, USE_TOGGLE, 0);

		if (m_killtarget) {
			SUB_KillTarget(STRING(m_killtarget));
		}

		if (RemoveOnFire())
		{
			UTIL_Remove(this);
		}

		if (ResetOnFire())
		{
			ResetCount();
		}
	}
}



//
// CGameCounterSet / game_counter_set	-- Sets the counter's value
// Flag: Fire once

#define SF_GAMECOUNTSET_FIREONCE			0x0001

class CGameCounterSet : public CRulePointEntity
{
public:
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	inline BOOL RemoveOnFire(void) { return (pev->spawnflags & SF_GAMECOUNTSET_FIREONCE) ? TRUE : FALSE; }

private:
};

LINK_ENTITY_TO_CLASS(game_counter_set, CGameCounterSet)


void CGameCounterSet::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	SUB_UseTargets(pActivator, USE_SET, pev->frags);

	if (RemoveOnFire())
	{
		UTIL_Remove(this);
	}
}
