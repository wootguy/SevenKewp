#include "mod_api.h"

enum HOOK_RETURN_CODE {
	HOOK_CONTINUE,
	HOOK_HANDLED
};

struct HLCOOP_PLUGIN_HOOKS {
	// called when the server starts, before any entities are spawns
	HOOK_RETURN_CODE (*pfnMapInit)();

	// called after map entities are spawned
	HOOK_RETURN_CODE (*pfnMapActivate)();
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