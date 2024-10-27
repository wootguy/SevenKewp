#include "extdll.h"
#include "util.h"
#include "PluginManager.h"
#include "cbase.h"
#include <fstream>

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

bool PluginManager::AddPlugin(const char* fpath, bool isMapPlugin) {

	std::string gamePath = fpath;

	if (isMapPlugin) {
		std::string fullPath = std::string("plugins/maps/") + fpath + PLUGIN_EXT;
		gamePath = getGameFilePath(fullPath.c_str());
	}

	if (gamePath.empty()) {
		ALERT(at_error, "Failed to find plugin '%s'\n", fpath);
		return false;
	}

	fpath = gamePath.c_str();

	Plugin plugin;
	memset(&plugin, 0, sizeof(Plugin));
	plugin.isMapPlugin = isMapPlugin;

	plugin.fpath = fpath;

#ifdef _WIN32
	plugin.h_module = LoadLibraryA(plugin.fpath.c_str());
#else
	plugin.h_module = dlopen(plugin.fpath.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif

	if (!plugin.h_module) {
		ALERT(at_error, "Plugin load failed '%s' (" LOADLIB_FUNC_NAME " error code %d)\n",
			fpath, GetLastError());
		return false;
	}

	PLUGIN_INIT_FUNCTION apiFunc =
		(PLUGIN_INIT_FUNCTION)GetProcAddress((HMODULE)plugin.h_module, "PluginInit");

	if (apiFunc) {
		int apiVersion = HLCOOP_API_VERSION;
		if (apiFunc(&plugin.hooks, apiVersion)) {
			ALERT(at_console, "Loaded plugin '%s'\n", plugin.fpath.c_str());
		} else {
			ALERT(at_error, "PluginInit call failed in plugin '%s'.\n", plugin.fpath.c_str());
			FreeLibrary((HMODULE)plugin.h_module);
			return false;
		}
	}
	else {
		ALERT(at_error, "PluginInit not found in plugin '%s'\n", plugin.fpath.c_str());
		FreeLibrary((HMODULE)plugin.h_module);
		return false;
	}

	plugins.push_back(plugin);
	return true;
}

void PluginManager::UnloadPlugin(const Plugin& plugin) {
	PLUGIN_EXIT_FUNCTION apiFunc =
		(PLUGIN_EXIT_FUNCTION)GetProcAddress((HMODULE)plugin.h_module, "PluginExit");

	if (apiFunc) {
		apiFunc();
	}
	else {
		ALERT(at_console, "PluginExit not found in plugin '%s'\n", plugin.fpath);
	}

	FreeLibrary((HMODULE)plugin.h_module);
	ALERT(at_console, "Removed plugin: '%s'\n", plugin.fpath.c_str());
}

void PluginManager::RemovePlugin(const Plugin& plugin) {
	UnloadPlugin(plugin);

	for (int i = 0; i < plugins.size(); i++) {
		const Plugin& plugin = plugins[i];
		if (&plugin == &plugin) {
			plugins.erase(plugins.begin() + i);
			return;
		}
	}
}

void PluginManager::RemovePlugins(bool mapPluginsOnly) {
	std::vector<Plugin> newPluginList;

	for (const Plugin& plugin : plugins) {
		if (mapPluginsOnly && !plugin.isMapPlugin) {
			newPluginList.push_back(plugin);
			continue;
		}

		UnloadPlugin(plugin);
	}

	plugins = newPluginList;
}

void PluginManager::UpdateServerPlugins(bool forceUpdate) {
	static uint64_t lastEditTime = 0;
	const char* configPath = CVAR_GET_STRING("pluginlist");

	std::string path = getGameFilePath(configPath);

	if (path.empty()) {
		ALERT(at_warning, "Missing plugin config: '%s'\n", configPath);
	}

	uint64_t editTime = getFileModifiedTime(path.c_str());

	if (!forceUpdate && lastEditTime == editTime) {
		return; // no changes made
	}

	lastEditTime = editTime;

	std::ifstream infile(path);

	if (!infile.is_open()) {
		ALERT(at_console, "Failed to open plugin config: '%s'\n", path.c_str());
		return;
	}

	std::vector<std::string> pluginPaths;

	int lineNum = 0;
	std::string line;
	while (std::getline(infile, line))
	{
		lineNum++;

		line = trimSpaces(line);
		if (line.empty() || line[0] == '/' || line[0] == '#') {
			continue;
		}

		std::string pluginPath = normalize_path("plugins/server/" + line + PLUGIN_EXT);
		std::string gamePath = getGameFilePath(pluginPath.c_str());

		if (gamePath.empty()) {
			ALERT(at_console, "Error on line %d of '%s'. Plugin not found: '%s'\n",
				lineNum, path.c_str(), pluginPath.c_str());
			continue;
		}

		pluginPaths.push_back(gamePath);
	}

	// remove plugins that are no longer in the list
	std::vector<Plugin> newPlugins;
	for (Plugin& plugin : plugins) {
		plugin.sortId = 0; // reset id for sorting later

		if (plugin.isMapPlugin) {
			newPlugins.push_back(plugin);
			continue;
		}

		bool found = false;
		for (std::string& path : pluginPaths) {
			if (std::string(plugin.fpath) == path) {
				found = true;
				break;
			}
		}

		if (!found) {
			UnloadPlugin(plugin);
		}
		else {
			newPlugins.push_back(plugin);
		}
	}
	plugins = newPlugins;

	int numLoaded = 0;
	int numFailed = 0;

	// add new plugins
	for (std::string& path : pluginPaths) {
		bool found = false;
		for (const Plugin& plugin : plugins) {
			if (std::string(plugin.fpath) == path) {
				continue;
			}
		}

		if (!found) {
			if (AddPlugin(path.c_str(), false)) {
				numLoaded++;
			}
			else {
				numFailed++;
			}
		}
	}

	// assign ids and sort
	for (int i = 0; i < pluginPaths.size(); i++) {
		std::string& path = pluginPaths[i];

		for (Plugin& plugin : plugins) {
			if (std::string(plugin.fpath) == path) {
				plugin.sortId = i+1;
			}
		}
	}

	std::sort(plugins.begin(), plugins.end(), [](const Plugin& a, const Plugin& b) {
		return a.isMapPlugin*1000 + a.sortId < b.isMapPlugin*1000 + b.sortId;
	});

	if (numFailed) {
		ALERT(at_error, "Loaded %d plugins. %d plugins failed to load.\n", numLoaded, numFailed);
	}
	else {
		ALERT(at_console, "Loaded %d plugins\n", numLoaded);
	}
}

void PluginManager::ReloadPlugins() {
	std::vector<Plugin> loadedMapPlugins;
	
	for (const Plugin& plugin : plugins) {
		if (plugin.isMapPlugin) {
			loadedMapPlugins.push_back(plugin);
			continue;
		}

		UnloadPlugin(plugin);
	}
	plugins = loadedMapPlugins;

	UpdateServerPlugins(true);
}

void PluginManager::ListPlugins(edict_t* plr) {
	std::vector<std::string> lines;

	bool isAdmin = !plr;

	lines.push_back(UTIL_VarArgs("\n    %-20s %-8s %-44s\n", "Name", "Type", isAdmin ? "File path" : ""));
	lines.push_back("--------------------------------------------------------------------------------\n");

	for (int i = 0; i < plugins.size(); i++) {
		const Plugin& plugin = plugins[i];

		std::string name = plugin.fpath;
		int lastSlash = name.find_last_of("/");
		if (lastSlash != -1) {
			name = name.substr(lastSlash + 1);
		}
		int lastDot = name.find_last_of(".");
		if (lastDot != -1) {
			name = name.substr(0, lastDot);
		}

		const char* type = plugin.isMapPlugin ? "MAP" : "SERVER";

		if (isAdmin) {
			lines.push_back(UTIL_VarArgs("%2d) %-20s %-8s %-44s\n", i + 1, name.c_str(), type, plugin.fpath.c_str()));
		}
		else {
			// player shouldn't know how the server files are laid out
			lines.push_back(UTIL_VarArgs("%2d) %-20s %-8s\n", i + 1, name.c_str(), type));
		}
	}
	lines.push_back("--------------------------------------------------------------------------------\n");

	for (std::string& line : lines) {
		if (plr) {
			UTIL_ClientPrint(plr, print_console, line.c_str());
		}
		else {
			g_engfuncs.pfnServerPrint(line.c_str());
		}
	}
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