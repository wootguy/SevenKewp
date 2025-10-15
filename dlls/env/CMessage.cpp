#include "extdll.h"
#include "util.h"
#include "customentity.h"
#include "effects.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"
#include "PluginManager.h"

class CMessage : public CPointEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	void	Spawn(void);
	void	Precache(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	KeyValue(KeyValueData* pkvd);
private:
};

LINK_ENTITY_TO_CLASS(env_message, CMessage)


void CMessage::Spawn(void)
{
	Precache();

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	switch (pev->impulse)
	{
	case 1: // Medium radius
		pev->speed = ATTN_STATIC;
		break;

	case 2:	// Large radius
		pev->speed = ATTN_NORM;
		break;

	case 3:	//EVERYWHERE
		pev->speed = ATTN_NONE;
		break;

	default:
	case 0: // Small radius
		pev->speed = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if (pev->scale <= 0)
		pev->scale = 1.0;
}


void CMessage::Precache(void)
{
	if (pev->noise)
		PRECACHE_SOUND((char*)STRING(pev->noise));
}

void CMessage::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "messagesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "messagevolume"))
	{
		pev->scale = atof(pkvd->szValue) * 0.1;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "messageattenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}


void CMessage::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBaseEntity* pPlayer = NULL;

	bool allPlayers = pev->spawnflags & SF_MESSAGE_ALL;

	CALL_HOOKS_VOID(pfnGameText, this, pActivator, STRING(pev->message), allPlayers, true);

	if (allPlayers) {
		UTIL_ShowMessageAll(STRING(pev->message));
		UTIL_ClientPrintAll(print_console, UTIL_VarArgs("HUD-MSG: \"%s\"\n", STRING(pev->message)));
	}
	else
	{
		if (pActivator && pActivator->IsPlayer())
			pPlayer = pActivator;
		else
		{
			pPlayer = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));
		}
		if (pPlayer) {
			UTIL_ShowMessage(STRING(pev->message), pPlayer);
			UTIL_ClientPrint(pPlayer, print_console, UTIL_VarArgs("HUD-MSG: \"%s\"\n", STRING(pev->message)));
		}
	}
	if (pev->noise)
	{
		EMIT_SOUND(edict(), CHAN_BODY, STRING(pev->noise), pev->scale, pev->speed);
	}
	if (pev->spawnflags & SF_MESSAGE_ONCE)
		UTIL_Remove(this);

	SUB_UseTargets(this, USE_TOGGLE, 0);
}
