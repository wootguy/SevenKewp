#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

#define MAX_NORMAL_BATTERY	100

class CItemBattery : public CItem
{
	float m_healthcap;

	virtual int MergedModelBody() { return MERGE_MDL_W_BATTERY; }

	void Spawn(void)
	{
		Precache();
		SetItemModel();
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

	void KeyValue(KeyValueData* pkvd)
	{
		if (FStrEq(pkvd->szKeyName, "healthcap"))
		{
			m_healthcap = atof(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else
			CItem::KeyValue(pkvd);
	}

	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->pev->deadflag != DEAD_NO)
		{
			return FALSE;
		}

		float maxArmor = mp_startarmor.value > 100 ? mp_startarmor.value : 100;

		float healthcap = m_healthcap > 0 ? m_healthcap : maxArmor;

		if ((pPlayer->pev->armorvalue < healthcap) &&
			(pPlayer->m_weaponBits & (1ULL << WEAPON_SUIT)))
		{
			int pct;
			char szcharge[64];

			pPlayer->pev->armorvalue += pev->health ? pev->health : gSkillData.sk_battery;
			pPlayer->pev->armorvalue = V_min(pPlayer->pev->armorvalue, healthcap);

			EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();


			// Suit reports new power level
			// For some reason this wasn't working in release build -- round it.
			pct = (int)((float)(pPlayer->pev->armorvalue * 100.0) * (1.0 / maxArmor) + 0.5);
			pct = (pct / 5);
			if (pct > 0)
				pct--;

			if (pct > 0)
				snprintf(szcharge, 64, "!HEV_%1dP", pct);

			//EMIT_SOUND_SUIT(ENT(pev), szcharge);
			pPlayer->SetSuitUpdate(szcharge, FALSE, SUIT_NEXT_IN_30SEC);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_battery, CItemBattery)
LINK_ENTITY_TO_CLASS(item_helmet, CItemBattery)
LINK_ENTITY_TO_CLASS(item_armorvest, CItemBattery)