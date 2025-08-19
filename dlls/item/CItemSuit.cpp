#include "extdll.h"
#include "util.h"
#include "weapons.h"
#include "CBasePlayer.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

#define SF_SUIT_SHORTLOGON		0x0001

class CItemSuit : public CItem
{
	virtual int MergedModelBody() { return MERGE_MDL_W_SUIT; }

	void Spawn(void)
	{
		Precache();
		SetItemModel();
		CItem::Spawn();
	}
	void Precache(void)
	{
		m_defaultModel = "models/w_suit.mdl";
		PRECACHE_MODEL(GetModel());
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->m_weaponBits & (1ULL << WEAPON_SUIT))
			return FALSE;

		if (pev->spawnflags & SF_SUIT_SHORTLOGON)
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0");		// short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx");	// long version of suit logon

		pPlayer->m_weaponBits |= (1ULL << WEAPON_SUIT);
		pPlayer->m_iHideHUD &= ~HIDEHUD_HEALTH;

		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit)