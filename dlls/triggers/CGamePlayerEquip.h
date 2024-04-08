#pragma once
#include "CRuleEntity.h"

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

struct EquipItem {
	string_t itemName;
	int count;
};

extern bool g_mapCfgExists;
extern EquipItem g_mapEquipment[MAX_EQUIP];

void equipPlayerWithItem(CBasePlayer* pPlayer, const char* itemName, int count);