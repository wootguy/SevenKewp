// Mod API for game plugins (metamod and something like angelscript later).
// The CBase* headers should be enough for most entity-specific plugin needs,
// but hook requests for those entity functions might go here.

#pragma once
#include <stdint.h>

#define HLCOOP_API_VERSION 1

enum distant_sound_types {
	DISTANT_9MM, // light tapping noise
	DISTANT_357, // deeper tap
	DISTANT_556, // deep tap / small explosion
	DISTANT_BOOM // big explosion
};

struct HLCOOP_FUNCTIONS {
	// call this instead of the engine function so that the mod can handle overflows and model replacement
	int (*pfnPrecacheModel)(const char* path);

	// call this instead of the engine function so that the mod can handle overflows and sound replacement
	int (*pfnPrecacheSound)(const char* path);

	// call this instead of the engine function so that the mod can handle overflows
	int (*pfnPrecacheGeneric)(const char* path);

	// call this instead of the engine function so that the mod can handle overflows
	int (*pfnPrecacheEvent)(int type, const char* path);

	// return replacement model if one exists, otherwise the given model
	const char* (*pfnGetModel)(const char* model);

	// same as the engine function, but handles model replacements
	void (*pfnSetModel)(edict_t* edict, const char* model);
	
	// same as the engine function, but handles model replacements
	int (*pfnModelIndex)(const char* model);

	// plays a sound accounting for global and per-entity sound replacment
	void (*pfnEmitSoundDyn)(edict_t* entity, int channel, const char* sample, float volume, 
		float attenuation, int flags, int pitch);

	// lower level sound playback (does not handle sound replacement)
	// plays the sound for players with bits contained in messageTargets
	// a player bit = 1 << (ENTINDEX(player_edict) % 31)
	void (*pfnStartSound)(edict_t* entity, int channel, const char* sample, float volume, float attenuation,
		int fFlags, int pitch, const float* origin, uint32_t messageTargets);

	// same as the engine function but accounts for sound replacement
	void (*pfnEmitAmbientSound)(edict_t* entity, const float* vecOrigin, const char* samp, float vol,
		float attenuation, int fFlags, int pitch, edict_t* dest);

	// conditionally plays a special distant sound clip for very loud sounds that should be heard everywhere
	void (*pfnPlayDistantSound)(edict_t* emitter, int soundType);

	// plays an MP3 for the target, or all players if target is null.
	// Call this so the mod knows when any global mp3s playing.
	void (*pfnPlayGlobalMp3)(const char* path, bool loop, edict_t* target);

	// stops MP3 playback for the target, or all players if target is null.
	// Call this so the mod knows when any global mp3s playing.
	void (*pfnStopGlobalMp3)(edict_t* target);
};