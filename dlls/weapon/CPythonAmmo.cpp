#include "CBasePlayerAmmo.h"
#include "CPython.h"

class CPythonAmmo : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_357ammobox.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_357AMMOBOX; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		if (pOther->GiveAmmo( AMMO_357BOX_GIVE, "357", gSkillData.sk_ammo_max_357 ) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS( ammo_357, CPythonAmmo )