#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "trains.h"
#include "nodes.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"

class CStripWeapons : public CPointEntity
{
public:
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

private:
};

LINK_ENTITY_TO_CLASS(player_weaponstrip, CStripWeapons);

void CStripWeapons::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBasePlayer* pPlayer = NULL;

	if (pActivator && pActivator->IsPlayer())
	{
		pPlayer = (CBasePlayer*)pActivator;
	}
	else if (!g_pGameRules->IsDeathmatch())
	{
		pPlayer = (CBasePlayer*)CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
	}

	if (pPlayer)
		pPlayer->RemoveAllItems(FALSE);
}
