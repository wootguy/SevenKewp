#pragma once

struct custom_muzzle_flash_t {
	char sprite[64];
	uint8_t attachment;
	uint8_t bone;
	uint8_t scale;
	uint8_t rendermode;
	uint8_t r, g, b, a;
	float x, y, z;
	int modelIdx; // client only
};

// SpriteAdv network message flags
#define FL_SPRITEADV_LOOP 1 // loop the animation + effect has life parameter
#define FL_SPRITEADV_FRAME 2 // effect has an start/end frame parameters
#define FL_SPRITEADV_FADE 4 // effect has fade parameters
#define FL_SPRITEADV_MODE 8 // sprite has render mode parameters
#define FL_SPRITEADV_EXPAND 16 // effect has expand/shrink parameters
#define FL_SPRITEADV_COLOR 32 // effect has color parameters
#define FL_SPRITEADV_SPIN 64 // effect has spin parameters
#define FL_SPRITEADV_MOVE 128 // effect has movement parameters

#define FL_SPRITEADV_MOVE_VEL_X 1 // effect has X velocity parameter
#define FL_SPRITEADV_MOVE_VEL_Y 2 // effect has Y velocity parameter
#define FL_SPRITEADV_MOVE_VEL_Z 4 // effect has Z velocity parameter
#define FL_SPRITEADV_MOVE_ACCEL_X 8 // effect has X velocity parameter
#define FL_SPRITEADV_MOVE_ACCEL_Y 16 // effect has Y velocity parameter
#define FL_SPRITEADV_MOVE_ACCEL_Z 32 // effect has Z velocity parameter

struct SpriteAdvArgs {
	int16_t x, y, z;
	int16_t modelIdx;
	uint8_t scale;
	uint8_t framerate;
	uint8_t startFrame;
	uint8_t endFrame; // 0 = automatic
	uint8_t renderamt;

	// Loop
	uint8_t maxLife; // seconds*0.1 to live (implies looping the animation)

	// Fade
	uint8_t fadeMode; // SpriteAdvFadeMode
	uint8_t fadeTime; // seconds*0.1 for fade out

	// Mode
	uint8_t renderMode;
	uint8_t sprMode; // parallel, oriented...
	float rx, ry, rz; // oriented mode rotation

	// Expand
	int16_t expandSpeed;
	uint8_t expandMax; // stop expanding once hitting this scale (0 = no limit)

	// Color
	uint8_t r, g, b;

	// Attachment
	uint16_t attachEntIdx;
	uint8_t attachBone;

	// Spin
	int8_t spinX, spinY, spinZ; // angular velocity

	// Velocity
	int16_t velX, velY, velZ;
	int16_t accelX, accelY, accelZ;
};

enum sprite_modes
{
	SPR_VP_PARALLEL_UPRIGHT,
	SPR_FACING_UPRIGHT,
	SPR_VP_PARALLEL,
	SPR_ORIENTED,
	SPR_VP_PARALLEL_ORIENTED
};

enum SpriteAdvFadeMode {
	SPRADV_FADE_NONE,
	SPRADV_FADE_OUT,
	SPRADV_FADE_IN,
	SPRADV_FADE_BOTH,
};

extern const char* g_waterSplashSounds[3];