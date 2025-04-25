#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

class CItemGeneric : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), GetModel());
		CItem::Spawn();
		
		// set smaller bbox so that it drops to the floor when close to walls
		UTIL_SetSize(pev, Vector(-4, -4, 0), Vector(4, 4, 8));

		pev->solid = SOLID_NOT;
	}
	void Precache(void)
	{
		m_defaultModel = "models/w_security.mdl";
		PRECACHE_MODEL(GetModel());
	}
	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		return FALSE;
	}
	BOOL ShouldRespawn() { return FALSE; }
};
LINK_ENTITY_TO_CLASS(item_generic, CItemGeneric)