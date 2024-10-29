#pragma once

#define HLCOOP_API_VERSION 2

class CBasePlayer;

struct HOOK_RETURN_DATA {
	uint32_t code; // what to do after processing this hook
	void* data; // overridden return value for the mod
};

#define HOOKBIT_HANDLED		1 // If set, forbid later plugins from processing the hook
#define HOOKBIT_OVERRIDE	2 // If set, do not call the game function and return the given value

#define HOOK_CONTINUE					{0, 0}
#define HOOK_CONTINUE_OVERRIDE(data)	{HOOKBIT_OVERRIDE, (void*)(data)}
#define HOOK_HANDLED					{HOOKBIT_HANDLED, 0}
#define HOOK_HANDLED_OVERRIDE(data)		{(HOOKBIT_HANDLED | HOOKBIT_OVERRIDE), (void*)(data)}

struct HLCOOP_PLUGIN_HOOKS {
	// called when the server starts, after worldspawn is precached and before any entities spawn
	HOOK_RETURN_DATA (*pfnMapInit)();

	// called when a map is unloaded
	HOOK_RETURN_DATA (*pfnServerDeactivate)();
		
	// called after a map has finished loading
	HOOK_RETURN_DATA (*pfnServerActivate)();

	// called when the server is about to load a new map
	HOOK_RETURN_DATA (*pfnChangeLevel)(const char* pszLevelName, const char* pszLandmarkName);

	// called before the player PreThink function
	HOOK_RETURN_DATA (*pfnPlayerPreThink)(CBasePlayer* pPlayer);

	// called before the player PostThink function
	HOOK_RETURN_DATA (*pfnPlayerPostThink)(CBasePlayer* pPlayer);

	// called before the player Use function is called
	HOOK_RETURN_DATA (*pfnPlayerUse)(CBasePlayer* pPlayer);

	// called before a client command is processed
	HOOK_RETURN_DATA (*pfnClientCommand)(CBasePlayer* pPlayer);

	// called when a client connects to the server. Return 0 to reject the connection with the given reason.
	HOOK_RETURN_DATA (*pfnClientConnect)(edict_t* pEntity, const char* pszName, const char* pszAddress, char szRejectReason[128]);
	
	// called when a client disconnects from the server.
	HOOK_RETURN_DATA (*pfnClientDisconnect)(edict_t* pEntity);

	// called when a player is fully connected to the server and is about to spawn
	HOOK_RETURN_DATA (*pfnClientPutInServer)(CBasePlayer* pPlayer);

	// called when a player changes model, name, colors, etc.
	HOOK_RETURN_DATA (*pfnClientUserInfoChanged)(edict_t* pPlayer, char* infobuffer);

	// called before an entity is spawned
	HOOK_RETURN_DATA (*pfnDispatchSpawn)(edict_t* pent);

	// called before an entity is touched by another
	HOOK_RETURN_DATA (*pfnDispatchTouch)(edict_t* pentTouched, edict_t* pentOther);

	// called before an entity is used
	HOOK_RETURN_DATA (*pfnDispatchUse)(edict_t* pentUsed, edict_t* pentOther);

	// called before an entity thinks
	HOOK_RETURN_DATA (*pfnDispatchThink)(edict_t* pent);

	// called before an entity is blocked by another
	HOOK_RETURN_DATA (*pfnDispatchBlocked)(edict_t* pentBlocked, edict_t* pentOther);

	// called before a keyvalue is sent to an entity
	HOOK_RETURN_DATA (*pfnDispatchKeyValue)(edict_t* pentKeyvalue, KeyValueData* pkvd);

	// called at the start of every server frame
	HOOK_RETURN_DATA (*pfnStartFrame)();

	// called when a sound is about to be played, after the mod has handled sound replacement
	HOOK_RETURN_DATA (*pfnEmitSound)(edict_t* pEntity, int channel, const char* pszSample, float volume, float attenuation, int fFlags, int pitch);

	// called when the mod registers a custom user message
	HOOK_RETURN_DATA (*pfnRegUserMsg)(const char* name, int size);

	// called when the mod changes a player's max movement speed
	HOOK_RETURN_DATA (*pfnSetClientMaxspeed)(const edict_t* pEntity, float maxspeed);

	// called when the mod sets a client key value
	HOOK_RETURN_DATA (*pfnSetClientKeyValue)(int clientIndex, char* pszInfoBuffer, const char* pszKey, const char* pszValue);

	// called when the mod fetches the argument count for a client or server command
	HOOK_RETURN_DATA (*pfnCmd_Argc)();

	// called when the mod fetches the argument count for a client or server command
	HOOK_RETURN_DATA (*pfnCmd_Argv)(int argc);

	// called when the mod fetches the full text for a client or server command
	HOOK_RETURN_DATA (*pfnCmd_Args)();

	// called when constructing a network message
	HOOK_RETURN_DATA (*pfnMessageBegin)(int iMsgType, int iMsgID, const float* pOrigin, edict_t* pEdict);
	HOOK_RETURN_DATA (*pfnWriteByte)(int value);
	HOOK_RETURN_DATA (*pfnWriteChar)(int value);
	HOOK_RETURN_DATA (*pfnWriteShort)(int value);
	HOOK_RETURN_DATA (*pfnWriteLong)(int value);
	HOOK_RETURN_DATA (*pfnWriteAngle)(float value);
	HOOK_RETURN_DATA (*pfnWriteCoord)(float value);
	HOOK_RETURN_DATA (*pfnWriteString)(const char* value);
	HOOK_RETURN_DATA (*pfnWriteEntity)(int value);
	HOOK_RETURN_DATA (*pfnMessageEnd)();
};

EXPORT void RegisterPlugin(void* plugin, HLCOOP_PLUGIN_HOOKS* hooks, const char* name);

// must call this instead of registering cvars directly or else the game crashes when the plugin unloads
// and any cvar is used
EXPORT cvar_t* RegisterPluginCVar(void* plugin, const char* name, const char* strDefaultValue, int intDefaultValue, int flags);

// must call this instead of registering commands directly or else the game crashes when the plugin unloads
// and the registered command is used
EXPORT void RegisterPluginCommand(void* plugin, const char* cmd, void (*function)(void));

// boilerplate for PluginInit functions
// must be inline so that plugins don't reference the game definition of HLCOOP_API_VERSION
inline int InitPluginApi(void* plugin, HLCOOP_PLUGIN_HOOKS* srcApi, int interfaceVersion) {
	if (interfaceVersion != HLCOOP_API_VERSION) {
		ALERT(at_error, "Plugin API version mismatch. Game wanted: %d, Plugin has: %d\n", interfaceVersion, HLCOOP_API_VERSION);
		return 0;
	}

#ifdef PLUGIN_NAME
	RegisterPlugin(plugin, srcApi, PLUGIN_NAME);
#else
	ALERT(at_error, "Plugin was not compiled correctly. PLUGIN_NAME is undefined.\n");
#endif
	return 1;
}