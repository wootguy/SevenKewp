#include "CBasePlayerAmmo.h"
#include "skill.h"
#include "CBasePlayer.h"
#include "CMP5.h"

extern bool g_hlPlayersCanPickup556;

class CAmmo556Clip : public CBasePlayerAmmo
{
	void Precache( void )
	{
		m_defaultModel = "models/w_saw_clip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	virtual int MergedModelBody() { return MERGE_MDL_W_9MMARCLIP; }

	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult;

		CBasePlayer* plr = pOther ? pOther->MyPlayerPointer() : NULL;

		if (g_hlPlayersCanPickup556 || (plr && plr->UseSevenKewpGuns())) {
			int limit = gSkillData.sk_ammo_max_556;
			if (!plr->UseSevenKewpGuns())
				limit = V_min(limit, 200); // HL clients can't display ammo counts above 255

			bResult = (pOther->GiveAmmo(AMMO_556_CLIP_GIVE, "556", limit) != -1);
		}
		else {
			// HL players don't have any weapons that use 556 ammo in this map, give them 9mm instead
			bResult = (pOther->GiveAmmo(AMMO_MP5CLIP_GIVE, "9mm", gSkillData.sk_ammo_max_9mm) != -1);
		}

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

//LINK_ENTITY_TO_CLASS( ammo_556clip, CAmmo556Clip)