#include "CBasePlayerAmmo.h"

class CMedkitAmmo : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		//SET_MODEL_MERGED(ENT(pev), "models/w_pmedkit.mdl", MERGE_MDL_W_GAUSSAMMO);
		SET_MODEL(ENT(pev), GET_MODEL("models/w_pmedkit.mdl"));
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		//PRECACHE_REPLACEMENT_MODEL("models/w_pmedkit.mdl");
		PRECACHE_MODEL("models/w_pmedkit.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		if (pOther->GiveAmmo(AMMO_MEDKIT_GIVE, "health", gSkillData.sk_ammo_max_medkit) != -1)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
			return TRUE;
		}
		return FALSE;
	}
};

LINK_ENTITY_TO_CLASS(ammo_medkit, CMedkitAmmo)