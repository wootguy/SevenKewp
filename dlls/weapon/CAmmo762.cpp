#include "CBasePlayerAmmo.h"
#include "skill.h"
#include "CBasePlayer.h"
#include "CCrossbow.h"

class CAmmo762 : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_m40a1clip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_M40A1_CLIP; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult;

		CBasePlayer* plr = pOther ? pOther->MyPlayerPointer() : NULL;

		if (plr->UseSevenKewpGuns()) {
			bResult = (pOther->GiveAmmo(AMMO_762_GIVE, "762", gSkillData.sk_ammo_max_762) != -1);
		}
		else {
			// HL players don't have snipers, use the xbow instead
			bResult = (pOther->GiveAmmo(AMMO_CROSSBOWCLIP_GIVE, "bolts", gSkillData.sk_ammo_max_bolts) != -1);
		}

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS( ammo_762, CAmmo762)