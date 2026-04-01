#include "CBasePlayerAmmo.h"

class CGaussAmmo : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_gaussammo.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_GAUSSAMMO; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_URANIUMBOX_GIVE, "uranium", gSkillData.sk_ammo_max_uranium ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( ammo_gaussclip, CGaussAmmo)
LINK_ENTITY_TO_CLASS( ammo_egonclip, CGaussAmmo)