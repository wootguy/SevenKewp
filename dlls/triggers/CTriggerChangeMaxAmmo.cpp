#include "extdll.h"
#include "util.h"

#define SF_MAXAMMO_ALL_PLAYERS		1	// Apply to all players, not just the activator
#define SF_MAXAMMO_REMOVE_EXCESS	2	// Remove ammo if above the new limit
#define SF_MAXAMMO_REFILL_AMMO		4	// Add ammo if below the new limit

enum MaxAmmoModes {
	MAXAMMO_SET,
	MAXAMMO_RESET,
	MAXAMMO_ADD,
	MAXAMMO_SUBTRACT,
};

class CTriggerChangeMaxAmmo : public CPointEntity
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void AdjustAmmo(int ammoIdx, int newMax, CBasePlayer* plr);

	int m_Mode;
	int m_iMaxAmmo;
};

LINK_ENTITY_TO_CLASS(trigger_changemaxammo, CTriggerChangeMaxAmmo)

void CTriggerChangeMaxAmmo::KeyValue(KeyValueData* pkvd) {
	if (FStrEq(pkvd->szKeyName, "m_Mode"))
	{
		m_Mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iMaxAmmo"))
	{
		m_iMaxAmmo = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue(pkvd);
	}
}

void CTriggerChangeMaxAmmo::AdjustAmmo(int ammoIdx, int newMax, CBasePlayer* plr) {
	if (!plr)
		return;

	if (pev->spawnflags & SF_MAXAMMO_REMOVE_EXCESS) {
		plr->m_rgAmmo[ammoIdx] = V_min(newMax, plr->m_rgAmmo[ammoIdx]);
	}
	if (pev->spawnflags & SF_MAXAMMO_REFILL_AMMO) {
		plr->m_rgAmmo[ammoIdx] = V_max(newMax, plr->m_rgAmmo[ammoIdx]);
	}

	if (!plr->IsSevenKewpClient()) {
		plr->m_rgAmmo[ammoIdx] = V_min(plr->m_rgAmmo[ammoIdx], 255); // HL clients can't display ammo counts above 255
	}
}

void CTriggerChangeMaxAmmo::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	const char* ammoType = STRING(pev->message);

	if (!ammoType[0])
		return;
	
	int current = UTIL_GetMaxAmmo(ammoType);
	int initial = UTIL_GetMaxAmmoInitial(ammoType);
	int newMax;

	switch (m_Mode) {
	default:
	case MAXAMMO_SET: newMax = m_iMaxAmmo; break;
	case MAXAMMO_RESET: newMax = initial; break;
	case MAXAMMO_ADD: newMax = clamp(0, (uint64_t)current + m_iMaxAmmo, INT_MAX); break;
	case MAXAMMO_SUBTRACT: newMax = clamp(0, (uint64_t)current - m_iMaxAmmo, INT_MAX); break;
	}

	UTIL_RegisterAmmoCapacity(ammoType, newMax);

	int ammoIdx = CBasePlayer::GetAmmoIndex(ammoType);

	if (pev->spawnflags & SF_MAXAMMO_ALL_PLAYERS) {
		for (int i = 1; i < gpGlobals->maxClients; i++) {
			AdjustAmmo(ammoIdx, newMax, UTIL_PlayerByIndex(i));
		}
	}
	else {
		AdjustAmmo(ammoIdx, newMax, pActivator->MyPlayerPointer());
	}

	SUB_UseTargets(pActivator, useType, value);
}
