#pragma once
#include "CBasePlayerAmmo.h"
#include "wc_params.h"

class EXPORT CAmmoCustom : public CBasePlayerAmmo {
public:
	CustomAmmoParams params;
	const char* configPath;
	int m_mergedModelBody;

	void Precache(void) override;
	void Spawn(void) override;
	BOOL AddAmmo(CBaseEntity* pOther) override;
	virtual int MergedModelBody() override { return m_mergedModelBody; }

	static string_t GetAmmoTypeForPlayer(CustomAmmoParams& params, CBaseEntity* pOther);
};

extern "C" DLLEXPORT void ammo_custom_ini(entvars_t* pev);