#include "CBasePlayerAmmo.h"

class CRpgAmmo : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL_MERGED(ENT(pev), "models/w_rpgammo.mdl", MERGE_MDL_W_RPGAMMO);
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_REPLACEMENT_MODEL ("models/w_rpgammo.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
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