#pragma once
#include <stdint.h>

#pragma pack(push,1)
struct BobPredVars {
	uint16_t speed;			// scale factor applied to bob cl_bobcycle (100ths)
	uint16_t mag;			// scale factor applied to bob cl_bob (100ths)
	uint16_t mag_weapon;	// scale factor applied to bob cl_bob (100ths)
	int8_t hmin;			// min bob height
	int8_t hmax;			// max bob height
	int8_t offset;			// offset bob center
	uint8_t cycle_factor;	// how much the bob cycle contributes to the bob value (0-100)
	uint8_t airtime_factor;	// how much to reduce bobbing while jumping/falling (0-100)
};

struct MovePredVars {
	BobPredVars bob;
	uint8_t step_size;			// sv_stepsize
	uint16_t step_speed;		// how fast view adjusts upward when climbing a stair (UPS)
	uint16_t jump_sound;		// sound index to play when jumping
	uint16_t fall_sound_speed;	// how fast the player needs to be moving to trigger the falling pain sound
	uint16_t fall_tilt_speed;	// how fast the player needs to be moving to trigger the view tilt effect
	uint16_t jump_power;		// how much upward velocity is given by jumping.
};
#pragma pack(pop)

extern MovePredVars g_movecfg;

extern const char* g_prediction_files[];
extern const int g_prediction_files_sz;

// prediction related cvars, used client-side
extern int g_soundvariety;
extern int g_flashlight_size;

extern uint8_t g_predMsgData[190];
extern int g_predMsgLen;

// generate replacement indexes for sounds/models predicted by the client (call after everything is precached)
void GeneratePredicionData();

// called by the client to get the replacement file used by the server
const char* RemapFile(const char* path);

// called by the client
void HookPredictionMessages();

void BuildMovePredData();

void ResetMovePredData();