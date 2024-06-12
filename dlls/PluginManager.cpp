#include "extdll.h"
#include "util.h"
#include "PluginManager.h"
#include "cbase.h"

PluginManager g_pluginManager;

#ifdef _WIN32
#include "windows.h"
#define LOADLIB_FUNC_NAME "LoadLibrary"
#define PLUGIN_EXT ".dll"
#else
#include <dlfcn.h>
#define PLUGIN_EXT ".so"
#define LOADLIB_FUNC_NAME "dlopen"
#define GetProcAddress dlsym
#define GetLastError dlerror
#define FreeLibrary dlclose
#define HMODULE void*
#endif

typedef int(*PLUGIN_INIT_FUNCTION)(HLCOOP_PLUGIN_HOOKS* pFunctionTable, int interfaceVersion);
typedef void(*PLUGIN_EXIT_FUNCTION)(void);

void PluginManager::AddPlugin(const char* fpath, bool isMapPlugin) {

	std::string fullPath = std::string("plugins/maps/") + fpath + PLUGIN_EXT;
	std::string gamePath = getGameFilePath(fullPath.c_str());

	if (gamePath.empty()) {
		ALERT(at_error, "Failed to find plugin '%s'\n", fpath);
		return;
	}

	fpath = gamePath.c_str();

	Plugin plugin;
	memset(&plugin, 0, sizeof(Plugin));
	plugin.isMapPlugin = isMapPlugin;

	plugin.fpath = STRING(ALLOC_STRING(fpath));

#ifdef _WIN32
	plugin.h_module = LoadLibraryA(plugin.fpath);
#else
	plugin.h_module = dlopen(plugin.fpath, RTLD_NOW | RTLD_LOCAL);
#endif

	if (!plugin.h_module) {
		ALERT(at_error, "Plugin load failed '%s' (" LOADLIB_FUNC_NAME " error code %d)\n",
			fpath, GetLastError());
		return;
	}

	PLUGIN_INIT_FUNCTION apiFunc =
		(PLUGIN_INIT_FUNCTION)GetProcAddress((HMODULE)plugin.h_module, "PluginInit");

	if (apiFunc) {
		int apiVersion = HLCOOP_API_VERSION;
		if (apiFunc(&plugin.hooks, apiVersion)) {
			ALERT(at_console, "Loaded plugin '%s'\n", plugin.fpath);
		} else {
			ALERT(at_error, "PluginInit call failed in plugin '%s'.\n", plugin.fpath);
			return;
		}
	}
	else {
		ALERT(at_error, "PluginInit not found in plugin '%s'\n", plugin.fpath);
		return;
	}

	plugins.push_back(plugin);
}

void PluginManager::RemovePlugins(bool mapPluginsOnly) {
	std::vector<Plugin> newPluginList;

	for (const Plugin& plugin : plugins) {
		if (mapPluginsOnly && !plugin.isMapPlugin) {
			newPluginList.push_back(plugin);
			continue;
		}

		PLUGIN_EXIT_FUNCTION apiFunc =
			(PLUGIN_EXIT_FUNCTION)GetProcAddress((HMODULE)plugin.h_module, "PluginExit");

		if (apiFunc) {
			apiFunc();
		}
		else {
			ALERT(at_console, "PluginExit not found in plugin '%s'\n", plugin.fpath);
		}

		FreeLibrary((HMODULE)plugin.h_module);
	}

	plugins = newPluginList;
}

ENTITYINIT PluginManager::GetCustomEntityInitFunc(const char* pname) {
	for (const Plugin& plugin : plugins) {
		ENTITYINIT initFunc = (ENTITYINIT)GetProcAddress((HMODULE)plugin.h_module, pname);
		
		if (initFunc) {
			return initFunc;
		}
	}

	return NULL;
}

// custom entity loader called by the engine during map load
extern "C" DLLEXPORT void custom(entvars_t * pev) {
	ENTITYINIT initFunc = g_pluginManager.GetCustomEntityInitFunc(STRING(pev->classname));

	if (initFunc) {
		initFunc(pev);
	}
	else {
		ALERT(at_console, "Invalid entity class '%s'\n", STRING(pev->classname));
	}
}