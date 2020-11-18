#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "nodes.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"

class CRevertSaved : public CPointEntity
{
public:
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	EXPORT MessageThink(void);
	void	EXPORT LoadThink(void);
	void	KeyValue(KeyValueData* pkvd);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	inline	float	Duration(void) { return pev->dmg_take; }
	inline	float	HoldTime(void) { return pev->dmg_save; }
	inline	float	MessageTime(void) { return m_messageTime; }
	inline	float	LoadTime(void) { return m_loadTime; }

	inline	void	SetDuration(float duration) { pev->dmg_take = duration; }
	inline	void	SetHoldTime(float hold) { pev->dmg_save = hold; }
	inline	void	SetMessageTime(float time) { m_messageTime = time; }
	inline	void	SetLoadTime(float time) { m_loadTime = time; }

private:
	float	m_messageTime;
	float	m_loadTime;
};

LINK_ENTITY_TO_CLASS(player_loadsaved, CRevertSaved);

TYPEDESCRIPTION	CRevertSaved::m_SaveData[] =
{
	DEFINE_FIELD(CRevertSaved, m_messageTime, FIELD_FLOAT),	// These are not actual times, but durations, so save as floats
	DEFINE_FIELD(CRevertSaved, m_loadTime, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CRevertSaved, CPointEntity);

void CRevertSaved::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "messagetime"))
	{
		SetMessageTime(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "loadtime"))
	{
		SetLoadTime(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CRevertSaved::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	UTIL_ScreenFadeAll(pev->rendercolor, Duration(), HoldTime(), pev->renderamt, FFADE_OUT);
	pev->nextthink = gpGlobals->time + MessageTime();
	SetThink(&CRevertSaved::MessageThink);
}


void CRevertSaved::MessageThink(void)
{
	UTIL_ShowMessageAll(STRING(pev->message));
	float nextThink = LoadTime() - MessageTime();
	if (nextThink > 0)
	{
		pev->nextthink = gpGlobals->time + nextThink;
		SetThink(&CRevertSaved::LoadThink);
	}
	else
		LoadThink();
}


void CRevertSaved::LoadThink(void)
{
	if (!gpGlobals->deathmatch)
	{
		SERVER_COMMAND("reload\n");
	}
}
