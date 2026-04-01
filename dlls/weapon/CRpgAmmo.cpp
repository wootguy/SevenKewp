#include "CBasePlayerAmmo.h"

class CRpgAmmo : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_rpgammo.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_RPGAMMO; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{
		if (pOther->GiveAmmo(AMMO_RPGCLIP_GIVE, "rockets", gSkillData.sk_ammo_max_rockets ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( ammo_rpgclip, CRpgAmmo )