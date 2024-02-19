#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "effects.h"
#include "explode.h"
#include "CFuncTank.h"

class CFuncTankMortar : public CFuncTank
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker);
};
LINK_ENTITY_TO_CLASS(func_tankmortar, CFuncTankMortar);


void CFuncTankMortar::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "iMagnitude"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CFuncTank::KeyValue(pkvd);
}


void CFuncTankMortar::Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker)
{
	if (m_fireLast != 0)
	{
		int bulletCount = (gpGlobals->time - m_fireLast) * m_fireRate;
		// Only create 1 explosion
		if (bulletCount > 0)
		{
			TraceResult tr;

			// TankTrace needs gpGlobals->v_up, etc.
			UTIL_MakeAimVectors(pev->angles);

			TankTrace(barrelEnd, forward, gTankSpread[m_spread], tr);

			ExplosionCreate(tr.vecEndPos, pev->angles, edict(), pev->impulse, TRUE);

			CFuncTank::Fire(barrelEnd, forward, pev);
		}
	}
	else
		CFuncTank::Fire(barrelEnd, forward, pev);
}
