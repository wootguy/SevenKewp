#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CBaseDMStart : public CPointEntity
{
public:
	void		KeyValue(KeyValueData* pkvd);
	BOOL		IsTriggered(CBaseEntity* pEntity);

private:
};

// These are the new entry points to entities. 
LINK_ENTITY_TO_CLASS(info_player_deathmatch, CBaseDMStart);
LINK_ENTITY_TO_CLASS(info_player_start, CPointEntity);
LINK_ENTITY_TO_CLASS(info_landmark, CPointEntity);

void CBaseDMStart::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "master"))
	{
		pev->netname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

BOOL CBaseDMStart::IsTriggered(CBaseEntity* pEntity)
{
	BOOL master = UTIL_IsMasterTriggered(pev->netname, pEntity);

	return master;
}
