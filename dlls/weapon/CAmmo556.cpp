#include "CBasePlayerAmmo.h"
#include "skill.h"

extern bool g_using556ammo;

class CAmmo556 : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		//SET_MODEL_MERGED(ENT(pev), "models/w_saw_clip.mdl", MERGE_MDL_W_CHAINAMMO);

		if (g_using556ammo) {
			SET_MODEL(ENT(pev), "models/w_saw_clip.mdl");
		}
		else {
			SET_MODEL_MERGED(ENT(pev), "models/w_chainammo.mdl", MERGE_MDL_W_CHAINAMMO);
		}
		
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		//PRECACHE_REPLACEMENT_MODEL("models/w_saw_clip.mdl");

		if (g_using556ammo) {
			PRECACHE_MODEL("models/w_saw_clip.mdl");
		}
		else {
			PRECACHE_REPLACEMENT_MODEL("models/w_chainammo.mdl");
		}
		
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult;

		if (g_using556ammo) {
			bResult = (pOther->GiveAmmo(AMMO_556_GIVE, "556", gSkillData.sk_ammo_max_556) != -1);
		}
		else {
			bResult = (pOther->GiveAmmo(AMMO_CHAINBOX_GIVE, "9mm", gSkillData.sk_ammo_max_9mm) != -1);
		}

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_556, CAmmo556)