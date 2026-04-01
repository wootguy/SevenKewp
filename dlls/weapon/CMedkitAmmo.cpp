#include "CBasePlayerAmmo.h"

class CMedkitAmmo : public CBasePlayerAmmo
{
	void Precache(void)
	{
		m_defaultModel = "models/w_pmedkit.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_MODEL("models/w_pmedkit.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	//virtual int MergedModelBody() { return MERGE_MDL_W_GAUSSAMMO; }

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