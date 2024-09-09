#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

class CItemLongJump : public CItem
{
	virtual int MergedModelBody() { return MERGE_MDL_W_LONGJUMP; }

	void Spawn(void)
	{
		Precache();
		SetItemModel();
		CItem::Spawn();
	}
	void Precache(void)
	{
		m_defaultModel = "models/w_longjump.mdl";
		PRECACHE_MODEL(GetModel());
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->m_fLongJump)
		{
			return FALSE;
		}

		if ((pPlayer->pev->weapons & (1 << WEAPON_SUIT)))
		{
			pPlayer->m_fLongJump = TRUE;// player now has longjump module

			g_engfuncs.pfnSetPhysicsKeyValue(pPlayer->edict(), "slj", "1");

			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
			WRITE_STRING(STRING(pev->classname));
			MESSAGE_END();

			EMIT_SOUND_SUIT(pPlayer->edict(), "!HEV_A1");	// Play the longjump sound UNDONE: Kelly? correct sound?
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(item_longjump, CItemLongJump)
