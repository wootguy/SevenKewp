#pragma once
#include "CBasePlayerAmmo.h"
#include "custom_weapon.h"

class CAmmoCustom : public CBasePlayerAmmo
{
	CustomAmmoParams params;
	const char* configPath;

	void Precache(void);
	BOOL AddAmmo(CBaseEntity* pOther);
};

extern "C" DLLEXPORT void ammo_custom_ini(entvars_t* pev);