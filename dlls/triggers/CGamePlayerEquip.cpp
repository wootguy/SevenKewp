#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "CRuleEntity.h"
#include "weapons.h"

//
// CGamePlayerEquip / game_playerequip	-- Sets the default player equipment
// Flag: USE Only

#define SF_PLAYEREQUIP_USEONLY			0x0001
#define MAX_EQUIP		32

class CGamePlayerEquip : public CRulePointEntity
{
public:
	void		KeyValue(KeyValueData* pkvd);
	void		Touch(CBaseEntity* pOther);
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	inline BOOL	UseOnly(void) { return (pev->spawnflags & SF_PLAYEREQUIP_USEONLY) ? TRUE : FALSE; }

private:

	void		EquipPlayer(CBaseEntity* pPlayer);

	string_t	m_weaponNames[MAX_EQUIP];
	int			m_weaponCount[MAX_EQUIP];
};

LINK_ENTITY_TO_CLASS(game_player_equip, CGamePlayerEquip);


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

		// Ammo logic is duplicated here to reduce the amount of spawned items.
		// Players getting spawnkilled rapidly in a full server leads to crashes and overflows.
		// This also reduces noise when spawning.

		const char* itemName = STRING(m_weaponNames[i]);
		if (!strcmp(itemName, "ammo_357")) {
			pPlayer->GiveAmmo(AMMO_357BOX_GIVE * m_weaponCount[i], "357", _357_MAX_CARRY);
		}
		else if (!strcmp(itemName, "ammo_crossbow")) {
			pPlayer->GiveAmmo(AMMO_CROSSBOWCLIP_GIVE * m_weaponCount[i], "bolts", BOLT_MAX_CARRY);
		}
		else if (!strcmp(itemName, "ammo_gaussclip") || !strcmp(itemName, "ammo_egonclip")) {
			pPlayer->GiveAmmo(AMMO_URANIUMBOX_GIVE * m_weaponCount[i], "uranium", URANIUM_MAX_CARRY);
		}
		else if (!strcmp(itemName, "ammo_glockclip") || !strcmp(itemName, "ammo_9mmclip")) {
			pPlayer->GiveAmmo(AMMO_GLOCKCLIP_GIVE * m_weaponCount[i], "9mm", _9MM_MAX_CARRY);
		}
		else if (!strcmp(itemName, "ammo_mp5clip") || !strcmp(itemName, "ammo_9mmAR")) {
			pPlayer->GiveAmmo(AMMO_MP5CLIP_GIVE * m_weaponCount[i], "9mm", _9MM_MAX_CARRY);
		}
		else if (!strcmp(itemName, "ammo_9mmbox")) {
			pPlayer->GiveAmmo(AMMO_CHAINBOX_GIVE * m_weaponCount[i], "9mm", _9MM_MAX_CARRY);
		}
		else if (!strcmp(itemName, "ammo_mp5grenades") || !strcmp(itemName, "ammo_ARgrenades")) {
			pPlayer->GiveAmmo(AMMO_M203BOX_GIVE * m_weaponCount[i], "ARgrenades", M203_GRENADE_MAX_CARRY);
		}
		else if (!strcmp(itemName, "ammo_rpgclip")) {
			pPlayer->GiveAmmo(AMMO_RPGCLIP_GIVE * m_weaponCount[i], "rockets", ROCKET_MAX_CARRY);
		}
		else if (!strcmp(itemName, "ammo_buckshot")) {
			pPlayer->GiveAmmo(AMMO_BUCKSHOTBOX_GIVE * m_weaponCount[i], "buckshot", BUCKSHOT_MAX_CARRY);
		}
		else if (!strcmp(itemName, "weapon_tripmine")) {
			int giveAmount = m_weaponCount[i];
			if (!pPlayer->HasNamedPlayerItem(itemName)) {
				pPlayer->GiveNamedItem(itemName);
				giveAmount -= 1;
			}
			if (giveAmount > 0)
				pPlayer->GiveAmmo(TRIPMINE_DEFAULT_GIVE * giveAmount, "Trip Mine", TRIPMINE_MAX_CARRY);
		}
		else if (!strcmp(itemName, "weapon_snark")) {
			int giveAmount = m_weaponCount[i];
			if (!pPlayer->HasNamedPlayerItem(itemName)) {
				pPlayer->GiveNamedItem(itemName);
				giveAmount -= 1;
			}
			if (giveAmount > 0)
				pPlayer->GiveAmmo(SNARK_DEFAULT_GIVE * giveAmount, "Snarks", SNARK_MAX_CARRY);
		}
		else if (!strcmp(itemName, "weapon_satchel")) {
			int giveAmount = m_weaponCount[i];
			if (!pPlayer->HasNamedPlayerItem(itemName)) {
				pPlayer->GiveNamedItem(itemName);
				giveAmount -= 1;
			}
			if (giveAmount > 0)
				pPlayer->GiveAmmo(SATCHEL_DEFAULT_GIVE * giveAmount, "Satchel Charge", SATCHEL_MAX_CARRY);
		}
		else if (!strcmp(itemName, "weapon_handgrenade")) {
			int giveAmount = m_weaponCount[i];
			if (!pPlayer->HasNamedPlayerItem(itemName)) {
				pPlayer->GiveNamedItem(itemName);
				giveAmount -= 1;
			}
			if (giveAmount > 0)
				pPlayer->GiveAmmo(HANDGRENADE_DEFAULT_GIVE * giveAmount, "Hand Grenade", HANDGRENADE_MAX_CARRY);
		}
		else {
			for (int j = 0; j < m_weaponCount[i]; j++) {
				pPlayer->GiveNamedItem(itemName);
			}
		}
	}
}


void CGamePlayerEquip::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	EquipPlayer(pActivator);
}
