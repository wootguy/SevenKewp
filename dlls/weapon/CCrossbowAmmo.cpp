#include "CBasePlayerAmmo.h"
#include "CCrossbow.h"
#include "skill.h"

class CCrossbowAmmo : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_crossbow_clip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_CROSSBOW_CLIP; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_CROSSBOWCLIP_GIVE, "bolts", gSkillData.sk_ammo_max_bolts ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};
LINK_ENTITY_TO_CLASS( ammo_crossbow, CCrossbowAmmo )