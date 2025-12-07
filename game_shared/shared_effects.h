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

// SpriteAdv network message parser flags
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

#define FL_HUDCON_MSG_PERCENT 1			// message contains offset percentage parameters
#define FL_HUDCON_MSG_PIXELS 2			// message contains offset pixel parameters
#define FL_HUDCON_MSG_EM 4				// message contains offset em parameters
#define FL_HUDCON_MSG_WORLD 8			// message contains world coordinates
#define FL_HUDCON_MSG_ENT 16			// message contains an entity attachment

#define FL_HUDCON_WORLD 1				// use world coordinates
#define FL_HUDCON_XRAY 2				// show text even if origin is hidden behind a wall

enum HudConAlignment {
	HUDCON_ALIGN_LEFT,
	HUDCON_ALIGN_RIGHT,
	HUDCON_ALIGN_CENTER,
};

struct hudconparms_t
{
	float		xPercent, yPercent;		// offset in percentage of screen
	float		xEm, yEm;				// offset in percentage of font size (width/height of the letter A)
	int16_t		xPixels, yPixels;		// offset in exact pixels
	int16_t		xWorld, yWorld, zWorld; // absolute position in the world, or offset from attachment ent
	uint16_t	attachEnt;				// entity index to attach to (text will be invisible if the entity isn't in PVS)
	uint8_t		r, g, b;				// font color (0,0,0 = use client default)
	uint16_t	holdTime;				// milliseconds to display
	uint8_t		id;						// IDs are used to update existing elements. 0 = untracked.
	uint8_t		alignment : 2;			// HudConAlignment
	uint8_t		flags : 6;				// FL_HUDCON_
};

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

// hud element network message parser flags
#define HUD_ELEM_MSG_COLOR1 2	// message has a color1 param
#define HUD_ELEM_MSG_COLOR2 4	// message has a color2 param
#define HUD_ELEM_MSG_FADE 16		// message has fade params
#define HUD_ELEM_MSG_FX 32		// message has effect params

#define HUD_SPR_MSG_REGION 1	// message has region params
#define HUD_SPR_MSG_ANIM 2		// message has animation params

// flags shared with all hud elements
#define HUD_ELEM_ABSOLUTE_X		(1<<0)	// X position in pixels.
#define HUD_ELEM_ABSOLUTE_Y		(1<<1)	// Y position in pixels.
#define HUD_ELEM_SCR_CENTER_X	(1<<2)	// X position relative to the center of the screen.
#define HUD_ELEM_SCR_CENTER_Y	(1<<3)	// Y position relative to the center of the screen.
#define HUD_ELEM_NO_BORDER		(1<<4)	// Ignore the client-side HUD border (hud_bordersize).
#define HUD_ELEM_HIDDEN			(1<<5)	// Create a hidden element.
#define HUD_ELEM_EFFECT_ONCE	(1<<6)	// Play the effect only once. (TODO: not implemented/redundant?)
#define HUD_ELEM_DEFAULT_ALPHA	(1<<7)	// Use the default client-side HUD alpha (hud_defaultalpha).
#define HUD_ELEM_DYNAMIC_ALPHA	(1<<8)	// Use the default client-side HUD alpha and flash the element when updated.
#define HUD_ELEM_IS_NUMERIC		(1<<9)	// Hud element is numeric, not a normal sprite

// flags for hud sprites only
#define HUD_SPR_OPAQUE				(1<<10)	// Draw opaque sprite.
#define HUD_SPR_MASKED				(1<<11)	// Draw masked sprite.
#define HUD_SPR_PLAY_ONCE			(1<<12)	// Play the animation only once.
#define HUD_SPR_HIDE_WHEN_STOPPED	(1<<13)	// Hide the sprite when the animation stops.
#define HUD_SPR_USE_CONFIG			(1<<14)	// Sprite name is a icon listed in hud.txt, not a file path (enables HL25 sprite scaling)

// flags for numeric displays only
#define HUD_NUM_RIGHT_ALIGN			(1<<10)	// Draw right aligned element.
#define HUD_NUM_SEPARATOR			(1<<11)	// Draw separator.
#define HUD_NUM_DONT_DRAW_ZERO		(1<<12)	// Hide the element if the value is zero.
#define HUD_NUM_LEADING_ZEROS		(1<<13)	// Draw leading zeros.
#define HUD_NUM_NEGATIVE_NUMBERS	(1<<14)	// Allow negative values. (TODO: not implemented)
#define HUD_NUM_PLUS_SIGN			(1<<15)	// Draw sign for positive values. (TODO: not implemented)

// flags for numeric displays only
#define HUD_TIME_RIGHT_ALIGN		(1<<10)	// Draw right aligned element.
#define HUD_TIME_HOURS				(1<<11)	// Draw hours.
#define HUD_TIME_MINUTES			(1<<12)	// Draw minutes.
#define HUD_TIME_SECONDS			(1<<13)	// Draw seconds.
#define HUD_TIME_MILLISECONDS		(1<<14)	// Draw milliseconds.
#define HUD_TIME_ZERO_HOURS			(1<<15)	// Draw hours even if the value is zero.
#define HUD_TIME_FREEZE				(1<<16)	// Freeze the displayed value.
#define HUD_TIME_COUNT_DOWN			(1<<17)	// Count down.

enum HudEffect {
	HUD_EFFECT_NONE,		// No effect.
	HUD_EFFECT_RAMP_UP,		// Linear ramp up from color1 to color2.
	HUD_EFFECT_RAMP_DOWN,	// Linear ramp down from color2 to color1.
	HUD_EFFECT_TRIANGLE,	// Linear ramp up and ramp down from color1 through color2 back to color1.
	HUD_EFFECT_COSINE_UP,	// Cosine ramp up from color1 to color2.
	HUD_EFFECT_COSINE_DOWN,	// Cosine ramp down from color2 to color1.
	HUD_EFFECT_COSINE,		// Cosine ramp up and ramp down from color1 through color2 back to color1.
	HUD_EFFECT_TOGGLE,		// Toggle between color1 and color2.
	HUD_EFFECT_SINE_PULSE,	// Sine pulse from color1 through zero to color2.
};

// parameters common to all custom hud elements
struct hudelementparams_t {
	uint8_t channel;
	uint32_t flags; // HUD_ELEM_*
	float x, y;
	RGB color1, color2;
	float fadeinTime, fadeoutTime;
	float holdTime;
	float fxTime;
	uint8_t effect;
};

struct hudspriteparams_t {
	uint8_t left, top, width, height; // region of the sprite to draw, if not using a hud.txt icon
	uint8_t frame;
	uint8_t numframes;
	uint8_t framerate;
};

struct hudnumparams_t {
	float value;
	uint8_t defdigits; // Default number of digits (numeric display only)
	uint8_t maxdigits; // Maximum number of digits (numeric display only)
};

struct HUDSpriteParams {
	hudelementparams_t hud;
	hudspriteparams_t spr;
};

struct HUDNumDisplayParams {
	hudelementparams_t hud; // can use HUD_NUM_* flags
	hudnumparams_t num;
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