#include "CTriggerScript.h"

//
// CTriggerScript / trigger_script -- calls a plugin function

LINK_ENTITY_TO_CLASS(trigger_script, CTriggerScript)

void CTriggerScript::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszScriptFile"))
	{
		m_iszScriptFile = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszScriptFunctionName"))
	{
		m_iszScriptFunctionName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flThinkDelta"))
	{
		m_flThinkDelta = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iMode"))
	{
		m_iMode = (TriggerScriptMode)atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CPointEntity::KeyValue(pkvd);
	}
}

void CTriggerScript::Spawn(void)
{
	if (m_iMode == TSCRIPT_THINK && (pev->spawnflags & TSCRIPT_START_ON)) {
		m_isActive = true;
		SetThink(&CTriggerScript::ConstantModeThink);
		pev->nextthink = gpGlobals->time;
	}

	CPointEntity::Spawn();
}

void CTriggerScript::callPlugin(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	if (!m_callback && m_iszScriptFunctionName)
		m_callback = g_pluginManager.GetEntityCallback(STRING(m_iszScriptFunctionName));

	if (m_callback) {
		CBaseEntity* activator = (pev->spawnflags & TSCRIPT_SELF_ACTIVATOR) ? this : pActivator;
		CBaseEntity* caller = (pev->spawnflags & TSCRIPT_SELF_CALLER) ? this : pCaller;

		m_callback(activator, caller, useType, value);
	}
	else {
		ALERT(at_console, "%s (trigger_script): callback '%s' not found\n",
			STRING(pev->targetname), STRING(m_iszScriptFunctionName));
	}
}

void CTriggerScript::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_isActive = useType == USE_TOGGLE ? !m_isActive : useType == USE_ON;

	if (m_iMode == TSCRIPT_THINK) {
		if (m_isActive) {
			m_thinkActivator = (pev->spawnflags & TSCRIPT_KEEP_ACTIVATOR) ? pActivator : this;
			SetThink(&CTriggerScript::ConstantModeThink);
			ConstantModeThink();
		}
		else {
			SetThink(NULL);
			pev->nextthink = 0;
		}
	}
	else {
		callPlugin(pActivator, pCaller, useType, value);
	}
}

void CTriggerScript::ConstantModeThink() {
	CBaseEntity* activator = m_thinkActivator ? m_thinkActivator.GetEntity() : this;

	callPlugin(activator, this, USE_TOGGLE, 0.0f);
	pev->nextthink = gpGlobals->time + m_flThinkDelta;
}
