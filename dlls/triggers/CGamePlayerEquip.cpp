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
#include "skill.h"
#include "user_messages.h"

LINK_ENTITY_TO_CLASS(game_player_equip, CGamePlayerEquip)

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

	if (pEntity->IsPlayer())
	{
		pPlayer = (CBasePlayer*)pEntity;
	}

	if (!pPlayer || !pPlayer->IsAlive())
		return;

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
	
	if (g_itemNameRemap.find(itemLower) != g_itemNameRemap.end()) {
		itemName = g_itemNameRemap[itemLower];
	}

	if (!strcmp(itemName, "<keyvalue>")) {
		return;
	}

	if (!strcmp(itemName, "ammo_357")) {
		pPlayer->GiveAmmo(AMMO_357BOX_GIVE * count, "357", gSkillData.sk_ammo_max_357);
	}
	else if (!strcmp(itemName, "ammo_crossbow")) {
		pPlayer->GiveAmmo(AMMO_CROSSBOWCLIP_GIVE * count, "bolts", gSkillData.sk_ammo_max_bolts);
	}
	else if (!strcmp(itemName, "ammo_gaussclip")) {
		pPlayer->GiveAmmo(AMMO_URANIUMBOX_GIVE * count, "uranium", gSkillData.sk_ammo_max_uranium);
	}
	else if (!strcmp(itemName, "ammo_9mmclip")) {
		pPlayer->GiveAmmo(AMMO_GLOCKCLIP_GIVE * count, "9mm", gSkillData.sk_ammo_max_9mm);
	}
	else if (!strcmp(itemName, "ammo_9mmAR")) {
		pPlayer->GiveAmmo(AMMO_MP5CLIP_GIVE * count, "9mm", gSkillData.sk_ammo_max_9mm);
	}
	else if (!strcmp(itemName, "ammo_9mmbox")) {
		pPlayer->GiveAmmo(AMMO_CHAINBOX_GIVE * count, "9mm", gSkillData.sk_ammo_max_9mm);
	}
	else if (!strcmp(itemName, "ammo_ARgrenades")) {
		pPlayer->GiveAmmo(AMMO_M203BOX_GIVE * count, "ARgrenades", gSkillData.sk_ammo_max_argrenades);
	}
	else if (!strcmp(itemName, "ammo_rpgclip")) {
		pPlayer->GiveAmmo(AMMO_RPGCLIP_GIVE * count, "rockets", gSkillData.sk_ammo_max_rockets);
	}
	else if (!strcmp(itemName, "ammo_buckshot")) {
		pPlayer->GiveAmmo(AMMO_BUCKSHOTBOX_GIVE * count, "buckshot", gSkillData.sk_ammo_max_buckshot);
	}
	else if (!strcmp(itemName, "ammo_medkit")) {
		pPlayer->GiveAmmo(AMMO_MEDKIT_GIVE * count, "health", gSkillData.sk_ammo_max_medkit);
	}
	else if (!strcmp(itemName, "weapon_tripmine")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(TRIPMINE_DEFAULT_GIVE * giveAmount, "Trip Mine", gSkillData.sk_ammo_max_tripmines);
	}
	else if (!strcmp(itemName, "weapon_snark")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(SNARK_DEFAULT_GIVE * giveAmount, "Snarks", gSkillData.sk_ammo_max_snarks);
	}
	else if (!strcmp(itemName, "weapon_satchel")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(SATCHEL_DEFAULT_GIVE * giveAmount, "Satchel Charge", gSkillData.sk_ammo_max_satchels);
	}
	else if (!strcmp(itemName, "weapon_handgrenade")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(HANDGRENADE_DEFAULT_GIVE * giveAmount, "Hand Grenade", gSkillData.sk_ammo_max_grenades);
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