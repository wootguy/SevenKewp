#pragma once
#include <string>

struct WeaponConfigCache {
	CustomWeaponParams params;
	uint64_t fileModifiedTime;
};

struct AmmoConfigCache {
	CustomAmmoParams params;
	uint64_t fileModifiedTime;
};

struct SettingsGroup {
	std::string name;
	StringMap keys;
	int lineno;
};

struct HudIconDef {
	std::string name;
	std::string sprite;
	uint16_t resolution;
	uint16_t x, y, w, h;
};

EXPORT extern HashMap<WeaponConfigCache> g_customWeaponCache;
EXPORT extern HashMap<AmmoConfigCache> g_customAmmoCache;

// if true, polls weapon config files for updates and automatically reloads weapons
EXPORT extern bool g_autoConfigReload;

// call on map change
EXPORT void clear_weapon_custom_cache();

EXPORT bool UTIL_ParseCustomWeaponConfig(const char* path, CustomWeaponParams& params);

EXPORT bool UTIL_ParseCustomAmmoConfig(const char* path, CustomAmmoParams& params);

// prettyPrint = if true, organizes configurations and event data into groups.
//               This changes event ordering which could affect weapon behaviors.
EXPORT void UTIL_DumpCustomWeaponConfig(const char* path, CustomWeaponParams& params, bool prettyPrint);

// checks weapon configs for modifications and applies new weapon settings
EXPORT void UTIL_ReloadWeaponConfigs();

EXPORT void UTIL_AutoReloadWeaponConfigs(bool enabled);

EXPORT std::vector<HudIconDef> UTIL_ParseWeaponHudConfig(const char* path);

// migrates weapon configs to a new format
// migrateMode:
// 0 = convert .txt to .dat struct data
// 1 = convert .dat back to .txt
// 2 = rewrite .txt using new schema. Used when no struct changes are needed.
EXPORT void UTIL_MigrateWeaponConfigs(int migrateMode);

EXPORT void UTIL_TestConfig(CWeaponCustom* wep); // validate that config file dumper is accurate