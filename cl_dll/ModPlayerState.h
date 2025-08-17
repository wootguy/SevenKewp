#pragma once

// extra player state synced using network messages
struct ModPlayerState {
	int pmodelanim;
	float pmodelfps;

	// calculated client-side
	float pmodelAnimTime;
};

extern ModPlayerState g_modPlayerStates[32];