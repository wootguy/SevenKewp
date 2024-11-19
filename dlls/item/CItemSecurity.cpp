#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

class CItemSecurity : public CItem
{
	virtual int MergedModelBody() { return MERGE_MDL_W_SECURITY; }

	void Spawn(void)
	{
		Precache();
		SetItemModel();
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