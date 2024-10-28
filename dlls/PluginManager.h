#pragma once
#include "PluginHooks.h"
#include <functional>
#include <vector>
#include "const.h"

struct Plugin {
	HLCOOP_PLUGIN_HOOKS hooks;
	const char* name; // must be unique. Each plugin defines this
	void* h_module; // handle to the library
	bool isMapPlugin;
	int id; // unique per server instance
	int sortId; // not unique
	std::string fpath;
};

typedef void(*ENTITYINIT)(struct entvars_s*);

#define CALL_HOOKS_VOID(...) \
	if (g_pluginManager.CallHooks(__VA_ARGS__).code & HOOKBIT_OVERRIDE) { return; }

#define CALL_HOOKS(type, ...) { \
	HOOK_RETURN_DATA ret = g_pluginManager.CallHooks(__VA_ARGS__); \
	if (ret.code & HOOKBIT_OVERRIDE) { \
		return (type)ret.data; \
	} \
}

class PluginManager {
public:
	std::vector<Plugin> plugins;

	// fpath is relative to a game directory
	// set isMapPlugin to true if the plugin should only run on the current map
	bool AddPlugin(const char* fpath, bool isMapPlugin);

	void UnloadPlugin(const Plugin& plugin);

	void RemovePlugin(const Plugin& plugin);

	void RemovePlugin(const char* name);

	void RemovePlugins(bool mapPluginsOnly);

	// will conditionally load/unload plugins if the config has been updated since the last call, unless forceUpdate=true
	void UpdateServerPlugins(bool forceUpdate=false);

	// reloads server plugins (not map plugins)
	void ReloadPlugins();

	// print loaded server and map plugins to console or client
	void ListPlugins(edict_t* plr);

	template<typename Func, typename... Args>
	HOOK_RETURN_DATA CallHooks(Func hookFunction, Args&&... args) {
		HOOK_RETURN_DATA totalRet = {0, 0};

		for (const Plugin& plugin : plugins) {
			if (!(plugin.hooks.*hookFunction)) {
				continue;
			}

			HOOK_RETURN_DATA ret = (*(plugin.hooks.*hookFunction))(std::forward<Args>(args)...);
			
			if (ret.code & HOOKBIT_OVERRIDE) {
				if (totalRet.code & HOOKBIT_OVERRIDE) {
					ALERT(at_console, "Multiple plugins want to override a function return value\n");
				}
				totalRet.code |= HOOKBIT_OVERRIDE;
				totalRet.data = ret.data;
			}

			if (ret.code & HOOKBIT_HANDLED) {
				break;
			}
		}

		return totalRet;
	}

	ENTITYINIT GetCustomEntityInitFunc(const char* pname);
};

extern PluginManager g_pluginManager;
