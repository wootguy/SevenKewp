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
	HOOK_RETURN_DATA (*pfnPlayerPreThink)(CBasePlayer*);

	// called before the player PostThink function
	HOOK_RETURN_DATA (*pfnPlayerPostThink)(CBasePlayer*);

	// called before the player Use function is called
	HOOK_RETURN_DATA (*pfnPlayerUse)(CBasePlayer*);
};

// boilerplate for PluginInit functions
// must be inline so that plugins don't reference the game definition of HLCOOP_API_VERSION
inline int InitPluginApi(HLCOOP_PLUGIN_HOOKS* destApi, HLCOOP_PLUGIN_HOOKS* srcApi, int interfaceVersion) {
	if (interfaceVersion != HLCOOP_API_VERSION) {
		ALERT(at_error, "Plugin API version mismatch. Game wanted: %d, Plugin has: %d\n", interfaceVersion, HLCOOP_API_VERSION);
		return 0;
	}

	memcpy(destApi, srcApi, sizeof(HLCOOP_PLUGIN_HOOKS));

	return 1;
}