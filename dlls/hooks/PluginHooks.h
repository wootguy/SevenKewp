#pragma once
#include "weaponinfo.h"
#include "entity_state.h"
#include "CommandArgs.h"

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

#define FL_CMD_SERVER			1	// command can be sent from server console
#define FL_CMD_CLIENT_CONSOLE	2	// command can be sent from client console
#define FL_CMD_CLIENT_CHAT		4	// command can be sent from client chat
#define FL_CMD_ADMIN			8	// command can only be sent by admins
#define FL_CMD_CASE				16	// command is case sensitive

// client command that can be run from chat or console
#define FL_CMD_CLIENT (FL_CMD_CLIENT_CONSOLE | FL_CMD_CLIENT_CHAT)

// command that can be run from anywhere (chat, client console, server console)
#define FL_CMD_ANY (FL_CMD_CLIENT_CONSOLE | FL_CMD_CLIENT_CHAT | FL_CMD_SERVER)

// Plugin command callback. Handles commands from the server console, client console, and chat.
// pPlayer = player who executed the command, or NULL for the server console
// args = parsed command and arguments with a flag to indicate if the command was sent from chat or console
// return true if the player's chat should be hidden (if the command was sent from chat)
typedef bool (*plugin_cmd_callback)(CBasePlayer* pPlayer, const CommandArgs& args);

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
	HOOK_RETURN_DATA (*pfnClientDisconnect)(CBasePlayer* pEntity);

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
	HOOK_RETURN_DATA (*pfnEmitSound)(edict_t* pEntity, int channel, const char* pszSample, float volume, float attenuation, int fFlags, int pitch, const float* origin, uint32_t recipients);
	
	// called when an ambient sound is about to be played, after the mod has handled sound replacement
	HOOK_RETURN_DATA (*pfnEmitAmbientSound)(edict_t* pEntity, const float* vecPos, const char* pszSample, float vol, float attenuation, int fFlags, int pitch);

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

	// called when a client user command is received (movement, buttons, etc.)
	HOOK_RETURN_DATA(*pfnCmdStart)(const edict_t* player, const struct usercmd_s* cmd, unsigned int random_seed);

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

	// called before the engine sets the model, after model replacement in the mod
	HOOK_RETURN_DATA (*pfnSetModel)(edict_t* edict, const char* model);

	// called immediately after the engine sets a model
	HOOK_RETURN_DATA (*pfnSetModelPost)(edict_t* edict, const char* model);

	// called after the engine precaches the given model
	HOOK_RETURN_DATA (*pfnPrecacheModelPost)(const char* model);

	// called before an event is played
	HOOK_RETURN_DATA (*pfnPlaybackEvent)(int flags, const edict_t* pInvoker, unsigned short eventindex, float delay, float* origin, float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);

	// called before weapon data is updated
	HOOK_RETURN_DATA (*pfnGetWeaponData)(edict_t* player, weapon_data_t* info);

	// called after client data is updated by the mod and before it is returned to the engine
	HOOK_RETURN_DATA (*pfnUpdateClientDataPost)(const edict_t* ent, int sendweapons, clientdata_t* cd);

	// called before voice data is sent to a player. Set mute to true to block the message to the receiver
	HOOK_RETURN_DATA (*pfnSendVoiceData)(int senderidx, int receiveridx, uint8_t* data, int sz, bool& mute);

	// called before the mod processes a newly uploaded customization (tempdecal.wad spray)
	HOOK_RETURN_DATA (*pfnPlayerCustomization)(edict_t* pEntity, customization_t* pCust);

	// called when a client responds to a cvar query
	HOOK_RETURN_DATA (*pfnCvarValue2)(const edict_t* pEntity, int requestID, const char* pszCvarName, const char* pszValue);
	
	// called before a chat message is sent. Update the message pointer to change the message.
	HOOK_RETURN_DATA (*pfnChatMessage)(CBasePlayer* plr, const char** message);
};

// do not call directly, use RegisterPlugin instead
EXPORT int RegisterPlugin_internal( HLCOOP_PLUGIN_HOOKS* hooks, int hookTableSz, const char* name, int ifaceVersion);

// must call this instead of registering cvars directly or else the game crashes when the plugin unloads
// and any cvar is used
EXPORT cvar_t* RegisterPluginCVar(const char* name, const char* strDefaultValue, int intDefaultValue, int flags);

// must call this instead of registering commands directly or else the game crashes when the plugin unloads
// and the registered command is used.
// cmd = text which triggers the command
// callback = function to call when the text is matched
// flags = combination of FL_CMD_*
// cooldown = limits the speed a command can be triggered by players who are not an admins.
//            A non-zero cooldown is recommended so that bad actors can't send hundreds of commands
//            simultaneously in an attempt to cause lag or crashes.
EXPORT void RegisterPluginCommand(const char* cmd, plugin_cmd_callback callback,
	int flags=FL_CMD_SERVER, float cooldown=0.1f);

// boilerplate for PluginInit functions
// must be inline so that plugins don't reference the mod definitions of HLCOOP_API_VERSION, PLUGIN_NAME,
// and the hook table size
inline int RegisterPlugin(HLCOOP_PLUGIN_HOOKS* srcApi) {
#ifdef PLUGIN_NAME
	return RegisterPlugin_internal(srcApi, sizeof(HLCOOP_PLUGIN_HOOKS), PLUGIN_NAME, HLCOOP_API_VERSION);
#else
	ALERT(at_error, "Plugin was not compiled correctly. PLUGIN_NAME is undefined.\n");
	return 0;
#endif
}