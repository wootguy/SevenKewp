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