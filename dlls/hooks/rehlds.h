#pragma once
#include "extdll.h"

#define REHLDS_LEAN_AND_MEAN // handle conflicts between rehlds and hlsdk headers
#include "rehlds/public/rehlds/rehlds_api.h"

bool RehldsApi_Init();
void RegisterRehldsHooks();
void UnregisterRehldsHooks();

EXPORT extern IRehldsApi* g_RehldsApi;
EXPORT extern const RehldsFuncs_t* g_RehldsFuncs;
EXPORT extern IRehldsServerData* g_RehldsData;
EXPORT extern IRehldsHookchains* g_RehldsHookchains;
EXPORT extern IRehldsServerStatic* g_RehldsSvs;

// can send network messages larger than 512 bytes (SVC_VOICEDATA)
// msgMode = MSG_BROADCAST or MSG_ONE_UNRELIABLE
// entindex = 1-based player index
EXPORT void rehlds_SendBigMessage(int msgMode, int msgType, void* data, int sz, int playerindex);