#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

#define MAX_NORMAL_BATTERY	100

class CItemBattery : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), GetModel());
		CItem::Spawn();
	}
	void Precache(void)
	{
		if (FClassnameIs(pev, "item_armorvest")) {
			m_defaultModel = "models/barney_helmet.mdl";
		} else if (FClassnameIs(pev, "item_armorvest")) {
			m_defaultModel = "models/barney_vest.mdl";
		}
		else {
			m_defaultModel = "models/w_battery.mdl";
		}
		
		PRECACHE_MODEL(GetModel());
		PRECACHE_SOUND("items/gunpickup2.wav");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			return FALSE;
		}

		if ((pPlayer->pev->armorvalue < MAX_NORMAL_BATTERY) &&
			(pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			int pct;
			char szcharge[64];

			pPlayer->pev->armorvalue += pev->health ? pev->health : gSkillData.sk_battery;
			pPlayer->pev->armorvalue = V_min(pPlayer->pev->armorvalue, MAX_NORMAL_BATTERY);

			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();


			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)((float)(pPlayer->pev->armorvalue * 100.0) * (1.0 / MAX_NORMAL_BATTERY) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;

			snprintf(szcharge, 64, "!HEV_%1dP", pct);

			//EMIT_SOUND_SUIT(ENT(pev), szcharge);
			pPlayer->SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery);
LINK_ENTITY_TO_CLASS(item_helmet, CItemBattery);
LINK_ENTITY_TO_CLASS(item_armorvest, CItemBattery);