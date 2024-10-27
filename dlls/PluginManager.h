#pragma once
#include "PluginHooks.h"
#include <functional>
#include <vector>
#include "const.h"

struct Plugin {
	std::string fpath;
	void* h_module; // handle to the library
	HLCOOP_PLUGIN_HOOKS hooks;
	bool isMapPlugin;
	int sortId; // not unique
};

typedef void(*ENTITYINIT)(struct entvars_s*);

class PluginManager {
public:
	std::vector<Plugin> plugins;

	// fpath is relative to a game directory
	// set isMapPlugin to true if the plugin should only run on the current map
	bool AddPlugin(const char* fpath, bool isMapPlugin);

	void UnloadPlugin(const Plugin& plugin);

	void RemovePlugin(const Plugin& plugin);

	void RemovePlugins(bool mapPluginsOnly);

	// will conditionally load/unload plugins if the config has been updated since the last call, unless forceUpdate=true
	void UpdateServerPlugins(bool forceUpdate=false);

	// reloads server plugins (not map plugins)
	void ReloadPlugins();

	// print loaded server and map plugins to console or client
	void ListPlugins(edict_t* plr);

	template<typename Func, typename... Args>
	void CallHooks(Func hookFunction, Args&&... args) {
		for (const Plugin& plugin : plugins) {
			if (!(plugin.hooks.*hookFunction)) {
				continue;
			}

			HOOK_RETURN_CODE ret = (*(plugin.hooks.*hookFunction))(std::forward<Args>(args)...);

			if (ret == HOOK_HANDLED) {
				break;
			}
		}
	}

	ENTITYINIT GetCustomEntityInitFunc(const char* pname);
};

extern PluginManager g_pluginManager;