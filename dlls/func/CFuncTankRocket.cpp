#include "extdll.h"
#include "util.h"
#include "effects.h"
#include "explode.h"
#include "CFuncTank.h"

class CFuncTankRocket : public CFuncTank
{
public:
	void Precache(void);
	void Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker);
	virtual const char* DisplayName() { return "Rocket Launcher"; }
	virtual const char* GetDeathNoticeWeapon() { return "rpg_rocket"; };
};
LINK_ENTITY_TO_CLASS(func_tankrocket, CFuncTankRocket)

void CFuncTankRocket::Precache(void)
{
	UTIL_PrecacheOther("rpg_rocket");
	CFuncTank::Precache();
}

void CFuncTankRocket::Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker)
{
	int i;

	if (m_fireLast != 0)
	{
		int bulletCount = (gpGlobals->time - m_fireLast) * m_fireRate;
		if (bulletCount > 0)
		{
			for (i = 0; i < bulletCount; i++)
			{
				CBaseEntity::Create("rpg_rocket", barrelEnd, pev->angles, true, edict());
			}
			CFuncTank::Fire(barrelEnd, forward, pev);
		}
	}
	else
		CFuncTank::Fire(barrelEnd, forward, pev);
}
