#include "CBasePlayerAmmo.h"
#include "skill.h"
#include "CBasePlayer.h"

extern bool g_hlPlayersCanPickup556;

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
		PRECACHE_MODEL("models/w_saw_clip.mdl");
		
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		int bResult;

		CBasePlayer* plr = pOther ? pOther->MyPlayerPointer() : NULL;

		if (g_hlPlayersCanPickup556 || (plr && plr->IsSevenKewpClient())) {
			int limit = gSkillData.sk_ammo_max_556;
			if (!plr->IsSevenKewpClient())
				limit = V_min(limit, 200); // HL clients can't display ammo counts above 255

			bResult = (pOther->GiveAmmo(AMMO_556_GIVE, "556", limit) != -1);
		}
		else {
			// HL players don't have any weapons that use 556 ammo in this map, give them 9mm instead
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