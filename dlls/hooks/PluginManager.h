#pragma once
#include "PluginHooks.h"
#include <functional>
#include <vector>
#include "const.h"

typedef int(*PLUGIN_INIT_FUNCTION)(void);
typedef void(*PLUGIN_EXIT_FUNCTION)(void);
typedef const char* (*PLUGIN_MESSAGE_FUNCTION)(const char*, const char*);

#define APIFUNC extern "C" DLLEXPORT

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
	if (g_pluginManager.CallHooks(&HLCOOP_PLUGIN_HOOKS::__VA_ARGS__).code & HOOKBIT_OVERRIDE) { return; }

#define CALL_HOOKS(type, ...) { \
	HOOK_RETURN_DATA ret = g_pluginManager.CallHooks(&HLCOOP_PLUGIN_HOOKS::__VA_ARGS__); \
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

	bool LoadPlugin(Plugin& plugin);

	void UnloadPlugin(const Plugin& plugin);

	void RemovePlugin(const Plugin& plugin);

	void RemovePlugin(const char* name);

	void ReloadPlugin(const char* name);

	std::string GetUpdatedPluginPath(Plugin& plugin);

	bool UpdatePlugin(const char* name);

	bool UpdatePlugin(Plugin& plugin);

	bool UpdatePlugins();

	void RemovePlugins(bool mapPluginsOnly);

	// will conditionally load/unload plugins if the config has been updated since the last call, unless forceUpdate=true
	void UpdatePluginsFromList(bool forceUpdate=false);

	// reloads server plugins (not map plugins)
	void ReloadPlugins();

	// print loaded server and map plugins to console or client
	void ListPlugins(CBasePlayer* plr);

	Plugin* FindPlugin(int id);

	Plugin* FindPlugin(const char* name);

	// run a client command registered by a plugin
	// returns true if command should be hidden from chat
	bool ClientCommand(CBasePlayer* pPlayer);

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
					DEBUG_MSG(at_console, "%s", "Multiple plugins want to override a function return value\n");
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

	plugin_ent_callback GetEntityCallback(const char* funcName);

	// clear trigger_script callback info. Does not invalidate trigger_script callback pointers
	// call on map change
	void ClearEntityCallbacks();
};

extern PluginManager g_pluginManager;

EXPORT bool CrossPluginFunctionHandle_internal(const char* pluginName, const char* funcName, void*& func, int& pluginId, int& plugin_load_counter);

template <typename Ret, typename... Args>
struct PluginFuncHandle {
	using FuncType = Ret(*)(Args...);

	int pluginId;            // plugin function was loaded from
	void* func;              // raw function pointer
	const char* pluginName;
	const char* funcName;
	int last_plugin_load_counter; // prevent crash from updated plugins (plugins retain ID for commands/cvars)

	PluginFuncHandle(const char* pluginName, const char* funcName)
		: pluginId(-1), func(nullptr), pluginName(pluginName), funcName(funcName) {
		last_plugin_load_counter = -1;
	}

	// For non-void return types
	template <typename R = Ret>
	typename std::enable_if<!std::is_void<R>::value, R>::type
	call(Args... args) {
		if (valid()) {
			return reinterpret_cast<FuncType>(func)(std::forward<Args>(args)...);
		}
		return R(); // default value
	}

	// For void return type
	template <typename R = Ret>
	typename std::enable_if<std::is_void<R>::value, void>::type
	call(Args... args) {
		if (valid()) {
			reinterpret_cast<FuncType>(func)(std::forward<Args>(args)...);
		}
	}

	bool valid() {
		return CrossPluginFunctionHandle_internal(pluginName, funcName, func, pluginId, last_plugin_load_counter);
	}
};

