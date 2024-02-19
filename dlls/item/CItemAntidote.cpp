#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

class CItemAntidote : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), GetModel());
		CItem::Spawn();
	}
	void Precache(void)
	{
		m_defaultModel = "models/w_antidote.mdl";
		PRECACHE_MODEL(GetModel());
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		pPlayer->SetSuitUpdate("!HEV_DET4", FALSE, SUIT_NEXT_IN_1MIN);

		pPlayer->m_rgItems[ITEM_ANTIDOTE] += 1;
		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_antidote, CItemAntidote);
