#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"
#include "CBasePlayer.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

#define SF_SUIT_SHORTLOGON		0x0001

class CItemSuit : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/w_suit.mdl");
		CItem::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/w_suit.mdl");
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->pev->weapons & (1 << WEAPON_SUIT))
			return FALSE;

		if (pev->spawnflags & SF_SUIT_SHORTLOGON)
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A0");		// short version of suit logon,
		else
			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_AAx");	// long version of suit logon

		pPlayer->pev->weapons |= (1 << WEAPON_SUIT);
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);