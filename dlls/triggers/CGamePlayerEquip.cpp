#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "weapons.h"
#include "CGamePlayerEquip.h"
#include "string"
#include "map"

LINK_ENTITY_TO_CLASS(game_player_equip, CGamePlayerEquip);

bool g_mapCfgExists;
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

				UTIL_StripToken(pkvd->szKeyName, tmp);

				m_weaponNames[i] = ALLOC_STRING(tmp);
				m_weaponCount[i] = atoi(pkvd->szValue);
				m_weaponCount[i] = V_max(1, m_weaponCount[i]);
				pkvd->fHandled = TRUE;
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

	if (!pPlayer)
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

	static std::map<std::string, const char*> itemNameRemap = {
		{"weapon_mp5", "weapon_9mmAR"},
		{"weapon_uzi", "weapon_9mmAR"},
		{"weapon_uziakimbo", "weapon_9mmAR"},
		{"weapon_m16", "weapon_9mmAR"},
		{"weapon_m249", "weapon_9mmAR"},
		{"weapon_minigun", "weapon_9mmAR"},
		{"weapon_pipewrench", "weapon_crowbar"},
		{"weapon_grapple", "weapon_crowbar"},
		{"weapon_medkit", "weapon_crowbar"},
		{"weapon_eagle", "weapon_357"},
		{"weapon_python", "weapon_357"},
		{"weapon_sniperrifle", "weapon_crossbow"},
		{"weapon_displacer", "weapon_egon"},
		{"weapon_shockrifle", "weapon_hornetgun"},
		{"weapon_glock", "weapon_9mmhandgun"},

		{"ammo_mp5clip", "ammo_9mmAR"},
		{"ammo_556clip", "ammo_9mmAR"},
		{"ammo_uziclip", "ammo_9mmAR"},
		{"ammo_556", "ammo_9mmbox"},
		{"ammo_glockclip", "ammo_9mmclip"},
		{"ammo_9mm", "ammo_9mmclip"},
		{"ammo_egonclip", "ammo_gaussclip"},
		{"ammo_mp5grenades", "ammo_ARgrenades"},
		{"ammo_spore", "ammo_ARgrenades"},
		{"weapon_sporelauncher", "ammo_ARgrenades"},
		{"ammo_sporeclip", "ammo_ARgrenades"},
		{"ammo_spore", "ammo_ARgrenades"},
		{"ammo_762", "ammo_crossbow"},
	};

	if (itemNameRemap.find(itemName) != itemNameRemap.end()) {
		itemName = itemNameRemap[itemName];
	}

	if (!strcmp(itemName, "ammo_357")) {
		pPlayer->GiveAmmo(AMMO_357BOX_GIVE * count, "357", _357_MAX_CARRY);
	}
	else if (!strcmp(itemName, "ammo_crossbow")) {
		pPlayer->GiveAmmo(AMMO_CROSSBOWCLIP_GIVE * count, "bolts", BOLT_MAX_CARRY);
	}
	else if (!strcmp(itemName, "ammo_gaussclip")) {
		pPlayer->GiveAmmo(AMMO_URANIUMBOX_GIVE * count, "uranium", URANIUM_MAX_CARRY);
	}
	else if (!strcmp(itemName, "ammo_9mmclip")) {
		pPlayer->GiveAmmo(AMMO_GLOCKCLIP_GIVE * count, "9mm", _9MM_MAX_CARRY);
	}
	else if (!strcmp(itemName, "ammo_9mmAR")) {
		pPlayer->GiveAmmo(AMMO_MP5CLIP_GIVE * count, "9mm", _9MM_MAX_CARRY);
	}
	else if (!strcmp(itemName, "ammo_9mmbox")) {
		pPlayer->GiveAmmo(AMMO_CHAINBOX_GIVE * count, "9mm", _9MM_MAX_CARRY);
	}
	else if (!strcmp(itemName, "ammo_ARgrenades")) {
		pPlayer->GiveAmmo(AMMO_M203BOX_GIVE * count, "ARgrenades", M203_GRENADE_MAX_CARRY);
	}
	else if (!strcmp(itemName, "ammo_rpgclip")) {
		pPlayer->GiveAmmo(AMMO_RPGCLIP_GIVE * count, "rockets", ROCKET_MAX_CARRY);
	}
	else if (!strcmp(itemName, "ammo_buckshot")) {
		pPlayer->GiveAmmo(AMMO_BUCKSHOTBOX_GIVE * count, "buckshot", BUCKSHOT_MAX_CARRY);
	}
	else if (!strcmp(itemName, "weapon_tripmine")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(TRIPMINE_DEFAULT_GIVE * giveAmount, "Trip Mine", TRIPMINE_MAX_CARRY);
	}
	else if (!strcmp(itemName, "weapon_snark")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(SNARK_DEFAULT_GIVE * giveAmount, "Snarks", SNARK_MAX_CARRY);
	}
	else if (!strcmp(itemName, "weapon_satchel")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(SATCHEL_DEFAULT_GIVE * giveAmount, "Satchel Charge", SATCHEL_MAX_CARRY);
	}
	else if (!strcmp(itemName, "weapon_handgrenade")) {
		int giveAmount = count;
		if (!pPlayer->HasNamedPlayerItem(itemName)) {
			pPlayer->GiveNamedItem(itemName);
			giveAmount -= 1;
		}
		if (giveAmount > 0)
			pPlayer->GiveAmmo(HANDGRENADE_DEFAULT_GIVE * giveAmount, "Hand Grenade", HANDGRENADE_MAX_CARRY);
	}
	else {
		for (int j = 0; j < count; j++) {
			if (!pPlayer->HasNamedPlayerItem(itemName))
				pPlayer->GiveNamedItem(itemName);
		}
	}
}