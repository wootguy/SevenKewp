#pragma once

// handle conflicts between rehlds and hlsdk headers
#ifndef CACHE_USER
typedef enum server_state_e
{
	ss_dead = 0,
	ss_loading = 1,
	ss_active = 2,
} server_state_t;

using cvar_callback_t = void (*)(const char* pszNewValue);

typedef struct cache_user_s
{
	void* data;
} cache_user_t;

#include "rehlds/public/rehlds/osconfig.h"

#define CACHE_USER
#define SYNCTYPE_T
#define DID_VEC3_T_DEFINE
#define EIFACE_H
#define REHLDS_INCLUDE
#endif

#include "extdll.h"
#include "rehlds/public/rehlds/custom.h"
#include "rehlds/public/interface.h"
#include "rehlds/public/FileSystem.h"
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