#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "CBasePlayer.h"
#include "weapons.h"
#include "CGamePlayerEquip.h"
#include "string"
#include "map"
#include "CCrossbow.h"
#include "CPython.h"
#include "CGlock.h"
#include "CRpg.h"
#include "CMP5.h"
#include "CTripmine.h"
#include "CSatchel.h"
#include "CHandGrenade.h"
#include "CSqueak.h"
#include "ammo.h"
#include "skill.h"
#include "user_messages.h"
#include "CAmmoCustom.h"

LINK_ENTITY_TO_CLASS(game_player_equip, CGamePlayerEquip)

extern bool g_hlPlayersCanPickup556;

bool g_mapCfgExists;
bool g_noSuit;
bool g_noMedkit;
int g_mapEquipIdx;
EquipItem g_mapEquipment[MAX_EQUIP];

void CGamePlayerEquip::KeyValue(KeyValueData* pkvd)
{
	CRulePointEntity::KeyValue(pkvd);

	if (!pkvd->fHandled)
	{
		for (int i = 0; i < MAX_EQUIP; i++)
		{
			if (!m_weaponNames[i])
			{
				char tmp[128];

				UTIL_StripToken(pkvd->szKeyName, tmp, sizeof(tmp));

				m_weaponNames[i] = ALLOC_STRING(tmp);
				m_weaponCount[i] = atoi(pkvd->szValue);
				m_weaponCount[i] = V_max(1, m_weaponCount[i]);
				pkvd->fHandled = TRUE;

				UTIL_PrecacheOther(STRING(m_weaponNames[i]));
				break;
			}
		}
	}
}


void CGamePlayerEquip::Touch(CBaseEntity* pOther)
{
	if (!CanFireForActivator(pOther))
		return;

	if (UseOnly())
		return;

	EquipPlayer(pOther);
}

void CGamePlayerEquip::EquipPlayer(CBaseEntity* pEntity)
{
	CBasePlayer* pPlayer = NULL;

	if (pEntity && pEntity->IsPlayer())
	{
		pPlayer = (CBasePlayer*)pEntity;
	}

	if (!pPlayer || !pPlayer->IsAlive())
		return;

	if (pev->spawnflags & SF_PLAYEREQUIP_FILTER_NAME) {
		if (strcmp(STRING(pEntity->pev->targetname), STRING(pev->target))) {
			return;
		}
	}

	for (int i = 0; i < MAX_EQUIP; i++)
	{
		if (!m_weaponNames[i])
			break;

		equipPlayerWithItem(pPlayer, STRING(m_weaponNames[i]), m_weaponCount[i]);
	}
}

void CGamePlayerEquip::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	EquipPlayer(pActivator);
}

void equipPlayerWithItem(CBasePlayer* pPlayer, const char* itemName, int count) {
	// Ammo logic is duplicated here to reduce the amount of spawned items.
	// Players getting spawnkilled rapidly in a full server leads to crashes and overflows.
	// This also reduces noise when spawning.

	std::string itemLower = toLowerCase(itemName);
	
	const char* remap = g_itemNameRemap.get(itemLower.c_str());
	if (remap) {
		itemName = remap;
	}

	if (!pPlayer->UseSevenKewpGuns()) {
		const char* hlremap = g_itemNameRemapHL.get(itemName);
		if (hlremap) {
			itemName = hlremap;
		}
	}

	if (!strcmp(itemName, "<keyvalue>")) {
		return;
	}

	if (!strcmp(itemName, "weapon_tripmine")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(TRIPMINE_DEFAULT_GIVE * giveAmount, "Trip Mine");
	}
	else if (!strcmp(itemName, "weapon_snark")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(SNARK_DEFAULT_GIVE * giveAmount, "Snarks");
	}
	else if (!strcmp(itemName, "weapon_satchel")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(SATCHEL_DEFAULT_GIVE * giveAmount, "Satchel Charge");
	}
	else if (!strcmp(itemName, "weapon_handgrenade")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(HANDGRENADE_DEFAULT_GIVE * giveAmount, "Hand Grenade");
	}
	else if (g_customAmmoConfigs.get(itemName)) {
		CustomAmmoParams params;
		UTIL_ParseCustomAmmoConfig(g_customAmmoConfigs.get(itemName), params);

		string_t ammoType = CAmmoCustom::GetAmmoTypeForPlayer(params, pPlayer);

		pPlayer->GiveAmmo(params.ammoGiven * count, STRING(ammoType));
	}
	else if (!strcmp(itemName, "item_longjump")) {
		pPlayer->m_fLongJump = TRUE;
		g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "slj", "1");
	}
	else {
		for (int j = 0; j < count; j++) {
			if (!pPlayer->HasNamedPlayerItem(itemName))
				pPlayer->GiveNamedItem(itemName);
		}
	}
}