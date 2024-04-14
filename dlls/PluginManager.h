#pragma once
#include "PluginHooks.h"
#include <functional>
#include <vector>

struct Plugin {
	const char* fpath;
	void* h_module; // handle to the library
	HLCOOP_PLUGIN_HOOKS hooks;
	bool isMapPlugin;
};

typedef void(*ENTITYINIT)(struct entvars_s*);

class PluginManager {
public:
	std::vector<Plugin> plugins;


	// fpath is relative to a game directory
	// set isMapPlugin to true if the plugin should only run on the current map
	void AddPlugin(const char* fpath, bool isMapPlugin);

	void RemovePlugins(bool mapPluginsOnly);

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