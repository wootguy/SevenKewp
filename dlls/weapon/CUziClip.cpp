#include "CBasePlayerAmmo.h"
#include "CUzi.h"

class CUziClip : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL_MERGED(ENT(pev), "models/w_uzi_clip.mdl", MERGE_MDL_W_UZI_CLIP);
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_REPLACEMENT_MODEL ("models/w_uzi_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
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
