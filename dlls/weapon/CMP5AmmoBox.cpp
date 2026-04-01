#include "CBasePlayerAmmo.h"

class CMP5AmmoBox : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_chainammo.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_CHAINAMMO; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_CHAINBOX_GIVE, "9mm", gSkillData.sk_ammo_max_9mm) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_9mmbox, CMP5AmmoBox)