#include "CBasePlayerAmmo.h"
#include "skill.h"

class CAmmo556 : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		//SET_MODEL_MERGED(ENT(pev), "models/w_saw_clip.mdl", MERGE_MDL_W_CHAINAMMO);
		SET_MODEL(ENT(pev), "models/w_saw_clip.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		//PRECACHE_REPLACEMENT_MODEL("models/w_saw_clip.mdl");
		PRECACHE_MODEL ("models/w_saw_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult = (pOther->GiveAmmo(AMMO_556_GIVE, "556", gSkillData.sk_ammo_max_556) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_556, CAmmo556)