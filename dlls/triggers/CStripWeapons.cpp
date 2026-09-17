#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "trains.h"
#include "nodes.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"

#define SF_STRIP_SUIT_TOO 1
#define SF_STRIP_LONG_JUMP 2

enum StripWeaponsListMode {
	STRIP_WEPLIST_INCLUSIVE, // strip weapons in weapons list
	STRIP_WEPLIST_EXCLUSIVE, // strip all weapons except for listed weapons
};

enum StripWeaponsAffected {
	STRIP_ACTIVATOR,			// strip the activator
	STRIP_ALL,					// strip all players
	STRIP_ALL_BUT_ACTIVATOR,	// strip all players except for the activator
};

class CStripWeapons : public CPointEntity
{
public:
	void	KeyValue(KeyValueData* pkvd);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	string_t m_weapons_list;
	int m_weapons_list_mode;
	int m_affected;
private:
};

LINK_ENTITY_TO_CLASS(player_weaponstrip, CStripWeapons)

void CStripWeapons::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "weapons_list"))
	{
		m_weapons_list = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "weapons_list_mode"))
	{
		m_weapons_list_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAffected"))
	{
		m_affected = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CStripWeapons::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	std::vector<std::string> weapons;
	std::unordered_set<std::string> weaponSet;
	if (m_weapons_list)
		weapons = splitString(STRING(m_weapons_list), ";");

	for (std::string& str : weapons) {
		weaponSet.insert(str);
	}

	if (m_weapons_list == STRIP_WEPLIST_EXCLUSIVE && weapons.empty()) {
		EALERT(at_error, "Tried to remove everything except everything. List excluded weapons or switch to inclusive mode.\n");
		return;
	}

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

		if (!pPlayer) {
			continue;
		}

		if (m_affected == STRIP_ACTIVATOR && pPlayer != pActivator) {
			continue;
		}
		if (m_affected == STRIP_ALL_BUT_ACTIVATOR && pPlayer == pActivator) {
			continue;
		}

		if (pev->spawnflags & SF_STRIP_LONG_JUMP) {
			pPlayer->m_fLongJump = FALSE;
			g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "slj", "0");
		}

		if (m_weapons_list_mode == STRIP_WEPLIST_INCLUSIVE) {
			if (weapons.size()) {
				for (int k = 0; k < (int)weapons.size(); k++) {
					CBasePlayerItem* item = pPlayer->GetNamedPlayerItem(weapons[i].c_str());
					if (item) {
						pPlayer->RemovePlayerItem(item);
					}
				}
			} else {
				pPlayer->RemoveAllItems(pev->spawnflags & SF_STRIP_SUIT_TOO);
			}
		}
		else if (m_weapons_list_mode == STRIP_WEPLIST_EXCLUSIVE) {
			CBasePlayerItem* pItem = NULL;
			std::vector<CBasePlayerItem*> removeItems;

			for (int k = 0; k < MAX_ITEM_TYPES; k++) {
				pItem = (CBasePlayerItem*)pPlayer->m_rgpPlayerItems[k].GetEntity();

				while (pItem) {
					if (!weaponSet.count(STRING(pItem->pev->classname))) {
						removeItems.push_back(pItem);
					}

					pItem = (CBasePlayerItem*)pItem->m_pNext.GetEntity();
				}
			}

			for (CBasePlayerItem* item : removeItems) {
				pPlayer->RemovePlayerItem(item);
			}
		}
		else {
			EALERT(at_error, "Invalid weapons list mode %d\n", m_weapons_list_mode);
		}
	}
}
