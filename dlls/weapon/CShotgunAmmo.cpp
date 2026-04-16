#include "CBasePlayerAmmo.h"

class CShotgunAmmo : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_shotbox.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_SHOTBOX; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_BUCKSHOTBOX_GIVE, "buckshot" ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( ammo_buckshot, CShotgunAmmo )
