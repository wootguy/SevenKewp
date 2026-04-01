#include "CBasePlayerAmmo.h"
#include "CUzi.h"

class CUziClip : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_uzi_clip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_UZI_CLIP; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo(AMMO_UZICLIP_GIVE, "9mm", gSkillData.sk_ammo_max_9mm) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_uziclip, CUziClip)
