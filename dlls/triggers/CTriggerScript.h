#pragma once
#include "extdll.h"
#include "util.h"
#include "PluginManager.h"
#include "CPointEntity.h"

#define TSCRIPT_START_ON 1
#define TSCRIPT_SELF_ACTIVATOR 2 // always pass the trigger_script as the callback activator
#define TSCRIPT_SELF_CALLER 4 // always pass the trigger_script as the callback caller
#define TSCRIPT_KEEP_ACTIVATOR 64 // in think mode, pass activator of the script to the callback

enum TriggerScriptMode {
	TSCRIPT_TRIGGER = 1,
	TSCRIPT_THINK = 2
};

class CTriggerScript : public CPointEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT ConstantModeThink();
	void callPlugin(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	string_t m_iszScriptFile; // unused - map plugins must be specified in map CFGs
	string_t m_iszScriptFunctionName; // name of the plugin function to call
	float m_flThinkDelta; // delay between thinks when in think mode
	TriggerScriptMode m_iMode;
	bool m_isActive;

	EHANDLE m_thinkActivator; // entity that activated the script's think function
	plugin_ent_callback m_callback; // cached callback pointer
};