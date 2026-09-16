#pragma once
#include "CRuleEntity.h"

//
// CGamePlayerEquip / game_playerequip	-- Sets the default player equipment
// Flag: USE Only

#define SF_PLAYEREQUIP_USEONLY			0x0001
#define SF_PLAYEREQUIP_FILTER_NAME		2
#define MAX_EQUIP		32

enum EquipMode {
	GPEQUIP_GIVE, // equip activator(s) with the given items
	GPEQUIP_CFG, // Modify respawn/cfg equipment. Don't modify currently held items.
};

enum IntentoryMode {
	GPEQUIP_MODE_SET,		// Add weapons and replace ammo
	GPEQUIP_MODE_ADD,		// Add weapons and sum ammo
	GPEQUIP_MODE_SUBTRACT,	// Remove weapons and subtract ammo
	GPEQUIP_MODE_REMOVE,	// Remove weapons and ammo
	GPEQUIP_MODE_RESTOCK,	// Increase ammo to given minimums
	GPEQUIP_MODE_LIMIT,		// Reduce ammo to given maximums
};

class CGamePlayerEquip : public CRulePointEntity
{
public:
	void		KeyValue(KeyValueData* pkvd);
	void		Touch(CBaseEntity* pOther);
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	bool		Equip(CBaseEntity* pActivator, bool isSpawningPlayer); // returns false if activator was not equipped

	inline BOOL	UseOnly(void) { return (pev->spawnflags & SF_PLAYEREQUIP_USEONLY) ? TRUE : FALSE; }

private:

	void		EquipPlayer(CBaseEntity* pPlayer);

	int m_equipMode;
	int m_inventoryMode;

	string_t	m_weaponNames[MAX_EQUIP];
	int			m_weaponCount[MAX_EQUIP];
};

struct EquipItem {
	string_t itemName;
	int count;
};

extern bool g_mapCfgExists;
extern bool g_noSuit;
extern bool g_noMedkit;
extern EquipItem g_mapEquipment[MAX_EQUIP];

void equipPlayerWithItem(CBasePlayer* pPlayer, const char* itemName, int count);