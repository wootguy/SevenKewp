#include "extdll.h"
#include "util.h"
#include "effects.h"
#include "explode.h"
#include "CFuncTank.h"

class CFuncTankLaser : public CFuncTank
{
public:
	void	Activate(void);
	void	KeyValue(KeyValueData* pkvd);
	void	Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker);
	void	Think(void);
	CLaser* GetLaser(void);
	virtual const char* DisplayName() { return "Laser"; }
	virtual const char* GetDeathNoticeWeapon() { return "weapon_gauss"; };

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

private:
	EHANDLE m_hLaser;
	float	m_laserTime;
};
LINK_ENTITY_TO_CLASS(func_tanklaser, CFuncTankLaser)

TYPEDESCRIPTION	CFuncTankLaser::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTankLaser, m_hLaser, FIELD_EHANDLE),
	DEFINE_FIELD(CFuncTankLaser, m_laserTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CFuncTankLaser, CFuncTank)

void CFuncTankLaser::Activate(void)
{
	CLaser* m_pLaser = GetLaser();
	if (!m_pLaser)
	{
		UTIL_Remove(this);
		ALERT(at_error, "Laser tank with no env_laser!\n");
	}
	else
	{
		m_pLaser->TurnOff();
	}
}


void CFuncTankLaser::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "laserentity"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CFuncTank::KeyValue(pkvd);
}


CLaser* CFuncTankLaser::GetLaser(void)
{
	if (m_hLaser)
		return (CLaser*)m_hLaser.GetEntity();

	CBaseEntity* pentLaser = UTIL_FindEntityByTargetname(NULL, STRING(pev->message));
	while (pentLaser)
	{
		// Found the landmark
		if (FClassnameIs(pentLaser->pev, "env_laser"))
		{
			m_hLaser = (CLaser*)pentLaser;
			return (CLaser*)m_hLaser.GetEntity();
		}
		else
			pentLaser = UTIL_FindEntityByTargetname(pentLaser, STRING(pev->message));
	}

	return NULL;
}


void CFuncTankLaser::Think(void)
{
	CLaser* m_pLaser = (CLaser*)m_hLaser.GetEntity();

	if (m_pLaser && (gpGlobals->time > m_laserTime))
		m_pLaser->TurnOff();

	CFuncTank::Think();
}


void CFuncTankLaser::Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker)
{
	int i;
	TraceResult tr;
	CLaser* m_pLaser = GetLaser();

	if (m_fireLast != 0 && m_pLaser)
	{
		// TankTrace needs gpGlobals->v_up, etc.
		UTIL_MakeAimVectors(pev->angles);

		int bulletCount = (gpGlobals->time - m_fireLast) * m_fireRate;
		if (bulletCount)
		{
			for (i = 0; i < bulletCount; i++)
			{
				m_pLaser->pev->origin = barrelEnd;
				TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);

				m_laserTime = gpGlobals->time;
				m_pLaser->TurnOn();
				m_pLaser->pev->dmgtime = gpGlobals->time - 1.0;
				m_pLaser->FireAtPoint(tr);
				m_pLaser->pev->nextthink = 0;
			}
			CFuncTank::Fire(barrelEnd, forward, pev);
		}
	}
	else
	{
		CFuncTank::Fire(barrelEnd, forward, pev);
	}
}
