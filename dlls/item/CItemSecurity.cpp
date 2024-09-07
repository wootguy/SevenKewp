#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

class CItemSecurity : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), GetModel());
		CItem::Spawn();
	}
	void Precache(void)
	{
		m_defaultModel = "models/w_security.mdl";
		PRECACHE_MODEL(GetModel());
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		pPlayer->m_rgItems[ITEM_SECURITY] += 1;
		return TRUE;
	}
	BOOL ShouldRespawn() { return FALSE; }
};

LINK_ENTITY_TO_CLASS(item_security, CItemSecurity)