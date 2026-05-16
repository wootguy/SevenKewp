#pragma once
#include "CAmmoCustom.h"
#include "weapons.h"

void CAmmoCustom::Precache(void) {
	configPath = g_customAmmoConfigs.get(STRING(pev->classname));

	if (configPath)
		UTIL_ParseCustomAmmoConfig(configPath, params);

	m_defaultModel = STRING(params.model);
	CBasePlayerAmmo::Precache();

	if (params.pickupSound)
		PRECACHE_SOUND(STRING(params.pickupSound));
}

BOOL CAmmoCustom::AddAmmo(CBaseEntity* pOther) {
	bool isHlClient = pOther->IsPlayer() && !pOther->MyPlayerPointer()->UseSevenKewpGuns();
	string_t ammoTypeHl = params.ammoTypeHl ? params.ammoTypeHl : params.ammoType;
	string_t ammoType = isHlClient ? ammoTypeHl : params.ammoType;

	int bResult = pOther->GiveAmmo(params.ammoGiven, STRING(ammoType)) != -1;

	if (bResult && params.pickupSound)
		EMIT_SOUND(ENT(pev), CHAN_ITEM, STRING(params.pickupSound), 1, ATTN_NORM);

	return bResult;
}

LINK_ENTITY_TO_CLASS(ammo_custom_ini, CAmmoCustom)