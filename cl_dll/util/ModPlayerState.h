#pragma once

// extra player state synced using network messages
struct ModPlayerState {
	int pmodelanim;
	float pmodelfps;
	uint64_t weaponBits; // currently held weapons

	// calculated client-side
	float pmodelAnimTime;
};

extern ModPlayerState g_modPlayerStates[32];

ModPlayerState& GetLocalPlayerState();