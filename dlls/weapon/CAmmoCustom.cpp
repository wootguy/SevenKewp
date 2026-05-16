#pragma once
#include "CAmmoCustom.h"
#include "weapons.h"

void CAmmoCustom::Spawn(void) {
	CBasePlayerAmmo::Spawn();

	Vector mins = *(Vector*)params.hullSizeMin;
	Vector maxs = *(Vector*)params.hullSizeMax;
	if (mins != g_vecZero || maxs != g_vecZero)
		UTIL_SetSize(pev, mins, maxs);
}

void CAmmoCustom::Precache(void) {
	configPath = g_customAmmoConfigs.get(STRING(pev->classname));

	if (configPath)
		UTIL_ParseCustomAmmoConfig(configPath, params);

	m_defaultModel = STRING(params.model);

	int* mergedBody = g_merged_models.get(m_defaultModel);
	m_mergedModelBody = mergedBody ? *mergedBody : -1;

	if (params.pickupSound)
		PRECACHE_SOUND(STRING(params.pickupSound));

	CBasePlayerAmmo::Precache();
}

BOOL CAmmoCustom::AddAmmo(CBaseEntity* pOther) {
	string_t ammoType = GetAmmoTypeForPlayer(params, pOther);

	int bResult = pOther->GiveAmmo(params.ammoGiven, STRING(ammoType)) != -1;

	if (bResult && params.pickupSound)
		EMIT_SOUND(ENT(pev), CHAN_ITEM, STRING(params.pickupSound), 1, ATTN_NORM);

	return bResult;
}

string_t CAmmoCustom::GetAmmoTypeForPlayer(CustomAmmoParams& params, CBaseEntity* pOther) {
	if (!pOther)
		return 0;

	// can an HL player use the intended ammo type?
	bool hlPlayersCanUse = g_registeredHlWeaponAmmo.hasKey(STRING(params.ammoType));

	bool isHlClient = pOther->IsPlayer() && !pOther->MyPlayerPointer()->UseSevenKewpGuns();

	string_t ammoTypeHl = params.ammoTypeHl ? params.ammoTypeHl : params.ammoType;
	return isHlClient && !hlPlayersCanUse ? ammoTypeHl : params.ammoType;
}

LINK_ENTITY_TO_CLASS(ammo_custom_ini, CAmmoCustom)