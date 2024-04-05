#include "extdll.h"
#include "mod_api.h"
#include "util.h"

edict_t* SpawnEdict(edict_t* pent);

static HLCOOP_FUNCTIONS gFunctionTable =
{
	PRECACHE_MODEL,
	PRECACHE_SOUND_NULLENT,
	PRECACHE_GENERIC,
	PRECACHE_EVENT,
	GET_MODEL,
	SET_MODEL,
	MODEL_INDEX,
	EMIT_SOUND_DYN,
	StartSound,
	UTIL_EmitAmbientSound,
	PLAY_DISTANT_SOUND,
	UTIL_PlayGlobalMp3,
	UTIL_StopGlobalMp3,
	SpawnEdict
};

extern "C" {
	DLLEXPORT int GetModAPI(HLCOOP_FUNCTIONS* pFunctionTable, int interfaceVersion) {
		if (!pFunctionTable) {
			ALERT(at_error, "GetModAPI: The function table you provided was NULL.\n");
			return FALSE;
		}
		if (interfaceVersion != HLCOOP_API_VERSION) {
			ALERT(at_error, "GetModAPI: You wanted API version %d, but this game is running API version %d\n",
				interfaceVersion, HLCOOP_API_VERSION);
			return FALSE;
		}

		memcpy(pFunctionTable, &gFunctionTable, sizeof(HLCOOP_FUNCTIONS));
		return TRUE;
	}
}
