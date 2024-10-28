#pragma once
#include "mod_api.h"

struct HOOK_RETURN_DATA {
	uint32_t code; // what to do after processing this hook
	void* data; // overridden return value for the mod
};

#define HOOKBIT_HANDLED		1 // If set, forbid later plugins from processing the hook
#define HOOKBIT_OVERRIDE	2 // If set, do not call the game function and return the given value

#define HOOK_CONTINUE					{0, 0}
#define HOOK_CONTINUE_OVERRIDE(data)	{HOOKBIT_OVERRIDE, (void*)data}
#define HOOK_HANDLED					{HOOKBIT_HANDLED, 0}
#define HOOK_HANDLED_OVERRIDE(data)		{(HOOKBIT_HANDLED | HOOKBIT_OVERRIDE), (void*)data}

struct HLCOOP_PLUGIN_HOOKS {
	// called when the server starts, before any entities are spawns
	HOOK_RETURN_DATA (*pfnMapInit)();

	// called after map entities are spawned
	HOOK_RETURN_DATA (*pfnMapActivate)();

	// called before the player PreThink function
	HOOK_RETURN_DATA (*pfnPlayerPreThink)(CBasePlayer* pPlayer);

	// called before the player PostThink function
	HOOK_RETURN_DATA (*pfnPlayerPostThink)(CBasePlayer* pPlayer);

	// called before the player Use function is called
	HOOK_RETURN_DATA (*pfnPlayerUse)(CBasePlayer* pPlayer);

	// called before a client command is processed
	HOOK_RETURN_DATA (*pfnClientCommand)(CBasePlayer* pPlayer);

	// called when a client connects to the server. Return 0 to reject the connection with the given reason.
	HOOK_RETURN_DATA (*pfnClientConnect)(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]);
	
	// called when a player is fully connected to the server and is about to spawn
	HOOK_RETURN_DATA (*pfnClientPutInServer)(CBasePlayer* pPlayer);
};

EXPORT cvar_t* RegisterPluginCVar(void* plugin, char* name, char* strDefaultValue, int intDefaultValue, int flags);

EXPORT void RegisterPlugin(void* plugin, HLCOOP_PLUGIN_HOOKS* hooks, const char* name);

// boilerplate for PluginInit functions
// must be inline so that plugins don't reference the game definition of HLCOOP_API_VERSION
inline int InitPluginApi(void* plugin, HLCOOP_PLUGIN_HOOKS* srcApi, int interfaceVersion) {
	if (interfaceVersion != HLCOOP_API_VERSION) {
		ALERT(at_error, "Plugin API version mismatch. Game wanted: %d, Plugin has: %d\n", interfaceVersion, HLCOOP_API_VERSION);
		return 0;
	}

#ifdef PLUGIN_NAME
	RegisterPlugin(plugin, srcApi, PLUGIN_NAME);
#else
	ALERT(at_error, "Plugin was not compiled correctly. PLUGIN_NAME is undefined.\n");
#endif
	return 1;
}