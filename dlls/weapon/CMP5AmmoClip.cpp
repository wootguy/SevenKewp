#include "CBasePlayerAmmo.h"
#include "CMP5.h"

class CMP5AmmoClip : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_9mmARclip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_9MMARCLIP; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo( AMMO_MP5CLIP_GIVE, "9mm", gSkillData.sk_ammo_max_9mm) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_mp5clip, CMP5AmmoClip )
LINK_ENTITY_TO_CLASS( ammo_9mmAR, CMP5AmmoClip )
LINK_ENTITY_TO_CLASS( ammo_9mmar, CMP5AmmoClip )
LINK_ENTITY_TO_CLASS(ammo_556clip, CMP5AmmoClip)
