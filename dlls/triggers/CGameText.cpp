#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "CRuleEntity.h"
#include "PluginManager.h"
#include "CGameText.h"

LINK_ENTITY_TO_CLASS(game_text, CGameText)

// Save parms as a block.  Will break save/restore if the structure changes, but this entity didn't ship with Half-Life, so
// it can't impact saved Half-Life games.
TYPEDESCRIPTION	CGameText::m_SaveData[] =
{
	DEFINE_ARRAY(CGameText, m_textParms, FIELD_CHARACTER, sizeof(hudtextparms_t)),
};

IMPLEMENT_SAVERESTORE(CGameText, CRulePointEntity)


void CGameText::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "channel"))
	{
		m_textParms.channel = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "x"))
	{
		m_textParms.x = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "y"))
	{
		m_textParms.y = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect"))
	{
		m_textParms.effect = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "color"))
	{
		int color[4];
		UTIL_StringToIntArray(color, 4, pkvd->szValue);
		m_textParms.r1 = color[0];
		m_textParms.g1 = color[1];
		m_textParms.b1 = color[2];
		m_textParms.a1 = color[3];
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "color2"))
	{
		int color[4];
		UTIL_StringToIntArray(color, 4, pkvd->szValue);
		m_textParms.r2 = color[0];
		m_textParms.g2 = color[1];
		m_textParms.b2 = color[2];
		m_textParms.a2 = color[3];
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fadein"))
	{
		m_textParms.fadeinTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fadeout"))
	{
		m_textParms.fadeoutTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "holdtime"))
	{
		m_textParms.holdTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "fxtime"))
	{
		m_textParms.fxTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CRulePointEntity::KeyValue(pkvd);
}


void CGameText::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!CanFireForActivator(pActivator))
		return;

	bool consoleEcho = !(pev->spawnflags & SF_NO_CONSOLE_ECHO);

	CALL_HOOKS_VOID(pfnGameText, this, pActivator, MessageGet(), MessageToAll(), consoleEcho);

	if (MessageToAll())
	{
		UTIL_HudMessageAll(m_textParms, MessageGet());

		if (consoleEcho) {
			UTIL_ClientPrintAll(print_console, UTIL_VarArgs("HUD-MSG: \"%s\"\n", MessageGet()));
		}
	}
	else
	{
		if (pActivator && pActivator->IsNetClient())
		{
			UTIL_HudMessage(pActivator, m_textParms, MessageGet());

			if (consoleEcho) {
				UTIL_ClientPrint(pActivator, print_console, UTIL_VarArgs("HUD-MSG: \"%s\"\n", MessageGet()));
			}
		}
	}

	// prevent infinite recursion
	// TODO: should be default behavior.
	//       Can't use for all entities? sven multi_manager can trigger itself in a loop and crash
	string_t oldName = pev->targetname;
	pev->targetname = 0;

	SUB_UseTargets(pActivator, USE_TOGGLE, 0.0f);

	pev->targetname = oldName;
}
