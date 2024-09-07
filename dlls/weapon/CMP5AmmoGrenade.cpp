#include "CBasePlayerAmmo.h"

class CMP5AmmoGrenade : public CBasePlayerAmmo
{
	void Spawn( void )
	{ 
		Precache( );
		SET_MODEL(ENT(pev), "models/w_ARgrenade.mdl");
		CBasePlayerAmmo::Spawn( );
	}
	void Precache( void )
	{
		PRECACHE_MODEL ("models/w_ARgrenade.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo( CBaseEntity *pOther ) 
	{ 
		// nerf nade ammo spawned by spores
		int giveAmount = AMMO_M203BOX_GIVE;
		if (!strcmp(STRING(pev->classname), "weapon_sporelauncher") || !strcmp(STRING(pev->classname), "ammo_sporeclip") || !strcmp(STRING(pev->classname), "ammo_spore") ) {
			giveAmount = 1;
		}

		int bResult = (pOther->GiveAmmo(giveAmount, "ARgrenades", gSkillData.sk_ammo_max_argrenades) != -1);

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS( ammo_mp5grenades, CMP5AmmoGrenade )
LINK_ENTITY_TO_CLASS( ammo_ARgrenades, CMP5AmmoGrenade )