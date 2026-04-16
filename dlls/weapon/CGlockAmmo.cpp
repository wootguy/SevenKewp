#include "CBasePlayerAmmo.h"
#include "CGlock.h"

class CGlockAmmo : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_9mmclip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	
	virtual int MergedModelBody() { return MERGE_MDL_W_9MMCLIP; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_GLOCKCLIP_GIVE, "9mm" ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( ammo_glockclip, CGlockAmmo )
LINK_ENTITY_TO_CLASS( ammo_9mmclip, CGlockAmmo )
LINK_ENTITY_TO_CLASS( ammo_9mm, CGlockAmmo )
