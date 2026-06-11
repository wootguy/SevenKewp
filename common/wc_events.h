#pragma once

#define FLOAT_TO_SFP_10_6(val) (clamp((int)((val) * 64), INT16_MIN, INT16_MAX))
#define SFP_10_6_TO_FLOAT(val) (val / 64.0f)

#define FLOAT_TO_SFP_9_7(val) (clamp((int)((val) * 128), INT16_MIN, INT16_MAX))
#define SFP_9_7_TO_FLOAT(val) (val / 128.0f)

#define FLOAT_TO_SFP_6_10(val) (clamp((int)((val) * 1024), INT16_MIN, INT16_MAX))
#define SFP_6_10_TO_FLOAT(val) (val / 1024.0f)

#define FLOAT_TO_FP_4_12(val) (clamp((int)((val) * 4096), 0, UINT16_MAX))
#define FP_4_12_TO_FLOAT(val) (val / 4096.0f)

#define FLOAT_TO_FP_8_8(val) (clamp((int)((val) * 256), 0, UINT16_MAX))
#define FP_8_8_TO_FLOAT(val) (val / 256.0f)

#define FLOAT_TO_SPREAD(val) (clamp((int)((val) * 65535), 0, UINT16_MAX))
#define SPREAD_TO_FLOAT(val) (val / 65535.0f)

#define MAX_WC_RANDOM_SELECTION 8

#define FL_WC_BULLETS_DYNAMIC_SPREAD 1	// spread widens while moving and tightens while crouching
#define FL_WC_BULLETS_NO_DECAL 2		// don't show gunshot particles and decal at impact point
#define FL_WC_BULLETS_NO_SOUND 4		// don't play texture sound at impact point

#define FL_WC_COOLDOWN_PRIMARY 1
#define FL_WC_COOLDOWN_SECONDARY 2
#define FL_WC_COOLDOWN_TERTIARY 4
#define FL_WC_COOLDOWN_IDLE 8

#define FL_WC_ANIM_NO_RESET 1	// don't restart the animation if the desired anim is already playing
#define FL_WC_ANIM_PMODEL 2		// animate the third-person weapon model, not the first-person one 
#define FL_WC_ANIM_ORDERED 4	// play multiple animations in order, not randomly

#define FL_WC_RECOIL_DUCK 1		// apply punch angles only when ducked
#define FL_WC_RECOIL_STAND 2	// apply punch angles only while standing

#define FL_WC_BEAM_SPIRAL	1		// render the beam as a spiral (egon effect)
#define FL_WC_BEAM_OPAQUE	2		// render the beam without transparency
#define FL_WC_BEAM_SHADEIN	4		// fade the start of the beam
#define FL_WC_BEAM_SHADEOUT	8		// fade the end of the beam
#define FL_WC_BEAM_NO_EVTS	16		// don't trigger impact events or call attack trace logic

#define FL_WC_SOUND_CHARGE_PITCH 1	// Sound pitch increases with chargeup progress

#define FL_WC_DECAL_PARTICLES	1	// create gunshot particles

#define FL_WC_TE_EXPLOSION_OPAQUE		1	// Sprite will be drawn opaque, else additive
#define FL_WC_TE_EXPLOSION_NO_DLIGHT	2	// Don't render the dynamic light
#define FL_WC_TE_EXPLOSION_NO_SOUND		4	// Don't play the default explosion sound
#define FL_WC_TE_EXPLOSION_NO_PARTICLES	8	// Don't draw particles

enum WeaponCustomEventTriggerShootArg {
	WC_TRIG_SHOOT_ARG_ALWAYS,		// always fire the shoot event
	WC_TRIG_SHOOT_ARG_AKIMBO,		// only fire the event when in akimbo mode
	WC_TRIG_SHOOT_ARG_NOT_AKIMBO,	// only fire the event when not in akimbo mode
};

// special clip size conditions
enum WeaponCustomEventTriggerClipSpArg {
	WC_TRIG_CLIP_ARG_ODD,			// fire on odd clip sizes
	WC_TRIG_CLIP_ARG_EVEN,			// fire on even clip sizes
	WC_TRIG_CLIP_ARG_EMPTY,			// fire when the clip is empty
	WC_TRIG_CLIP_ARG_NOT_EMPTY,		// fire on non-zero clip sizes
};

enum WeaponCustomEventTriggerImpactArg {
	WC_TRIG_IMPACT_PRIMARY_ANY,
	WC_TRIG_IMPACT_PRIMARY_WORLD,
	WC_TRIG_IMPACT_PRIMARY_MONSTER,
	WC_TRIG_IMPACT_SECONDARY_ANY,
	WC_TRIG_IMPACT_SECONDARY_WORLD,
	WC_TRIG_IMPACT_SECONDARY_MONSTER,
	WC_TRIG_IMPACT_TERTIARY_ANY,
	WC_TRIG_IMPACT_TERTIARY_WORLD,
	WC_TRIG_IMPACT_TERTIARY_MONSTER,
	WC_TRIG_IMPACT_PRIMARY_ALT_ANY,
	WC_TRIG_IMPACT_PRIMARY_ALT_WORLD,
	WC_TRIG_IMPACT_PRIMARY_ALT_MONSTER,
};

enum WeaponCustomEventIdleArg {
	WC_TRIG_IDLE_ARG_DEFAULT,	// fire when other special conditions are not active
	WC_TRIG_IDLE_ARG_EMPTY,		// fire when the clip is empty
	WC_TRIG_IDLE_ARG_LASER,		// fire when the laser is active
	WC_TRIG_IDLE_ARG_AKIMBO,	// fire when akimbo mode is active
	WC_TRIG_IDLE_ARG_ZOOM,		// fire when the weapon is zoomed
};

enum WeaponCustomEventDeployArg {
	WC_TRIG_DEPLOY_ARG_DEFAULT,		// fire when other special conditions are not active
	WC_TRIG_DEPLOY_ARG_EMPTY,		// fire when the clip is empty
	WC_TRIG_DEPLOY_ARG_LASER,		// fire when the laser is active
	WC_TRIG_DEPLOY_ARG_AKIMBO,		// fire when in akimbo mode
	WC_TRIG_DEPLOY_ARG_FIRST,		// fire when deployed for the first time
};

enum WeaponCustomEventZoomArg {
	WC_TRIG_ZOOM_ARG_OUT,		// trigger at zoom level 0
	WC_TRIG_ZOOM_ARG_IN,		// trigger at zoom level 1
	WC_TRIG_ZOOM_ARG_IN2,		// trigger at zoom level 2
	WC_TRIG_ZOOM_ARG_IN3,		// trigger at zoom level 3
	WC_TRIG_ZOOM_ARG_CHANGED,	// trigger at any zoom level
};

enum WeaponCustomEventTriggers {
	WC_TRIG_PRIMARY,			// trigger arg: WeaponCustomEventTriggerShootArg (does not trigger when alternate fire is active)
	WC_TRIG_SECONDARY,			// trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_TERTIARY,
	WC_TRIG_PRIMARY_ALT,		// triggers on alternate primary fire (laser/zoom)
	WC_TRIG_PRIMARY_CLIPSIZE,	// trigger arg is the clip size to trigger on
	WC_TRIG_PRIMARY_CLIP_SP,	// trigger arg: WeaponCustomEventTriggerClipSpArg
	WC_TRIG_PRIMARY_ALT_CLIPSIZE,// trigger arg: WeaponCustomEventTriggerClipSpArg
	WC_TRIG_PRIMARY_ALT_CLIP_SP,// trigger arg: WeaponCustomEventTriggerClipSpArg
	WC_TRIG_PRIMARY_CHARGE,		// triggers when primary fire begins charging. Delayed events are cancelled when charging stops or fails.
	WC_TRIG_PRIMARY_OVERCHARGE,	// triggers when primary fire charges for too long and is cancelled.
	WC_TRIG_PRIMARY_START,		// triggers when primary fire key is pressed. Delayed events are cancelled when a STOP event starts.
	WC_TRIG_PRIMARY_STOP,		// triggers when primary fire key is released
	WC_TRIG_PRIMARY_FAIL,		// triggers when primary fire fails (no ammo, underwater, ...)
	WC_TRIG_SECONDARY_CLIPSIZE, // trigger arg is the clip size to trigger on
	WC_TRIG_SECONDARY_CLIP_SP,	// trigger arg: WeaponCustomEventTriggerClipSpArg
	WC_TRIG_SECONDARY_CHARGE,	// triggers when secondary fire begins charging. Delayed events are cancelled when charging stops or fails.
	WC_TRIG_SECONDARY_OVERCHARGE,// triggers when primary fire charges for too long and is cancelled.
	WC_TRIG_SECONDARY_START,	// triggers when secondary fire key is pressed. Delayed events are cancelled when a STOP event starts.
	WC_TRIG_SECONDARY_STOP,		// triggers when secondary fire key is released
	WC_TRIG_SECONDARY_FAIL,		// triggers when secondary fire fails (no ammo, underwater, ...)
	WC_TRIG_RELOAD,				// triggers when a simple reload begins, or when a shotgun reloads a single shell. Trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_RELOAD_EMPTY,		// triggers when an empty clip reload begins. Trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_RELOAD_NOT_EMPTY,	// triggers when a non-empty clip reload begins. Trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_RELOAD_FINISH,		// triggers when a shotgun reload finishes
	WC_TRIG_DEPLOY,				// Trigger arg: WeaponCustomEventDeployArg
	WC_TRIG_IDLE,				// Trigger arg: WeaponCustomEventIdleArg
	WC_TRIG_BULLET_FIRED,		// triggered when a bullet is fired
	WC_TRIG_LASER_ON,			// triggered when the laser is enabled
	WC_TRIG_LASER_OFF,			// triggered when the laser is disabled
	WC_TRIG_ZOOM,				// trigger arg: zoom level (0-3)		
	WC_TRIG_IMPACT,				// triggered when an attack trace impacts something. Trigger arg: WeaponCustomEventTriggerImpactArg
	WC_TRIG_RICOCHET,			// triggered when an attack trace ricochets off something. Trigger arg: WeaponCustomEventTriggerImpactArg
};

enum WeaponCustomEventType {
	WC_EVT_IDLE_SOUND,		// simple sound playback for reloads
	WC_EVT_PLAY_SOUND,		// advanced sound playback
	WC_EVT_EJECT_SHELL,
	WC_EVT_RECOIL,			// recoil with logic shared for all angles
	WC_EVT_RECOIL_ADV,		// recoil with unique logic per angle
	WC_EVT_SET_BODY,
	WC_EVT_WEP_ANIM,
	WC_EVT_BULLETS,
	WC_EVT_BEAM,
	WC_EVT_PROJECTILE,		// for slow-moving projectiles that aren't predicted on the client
	WC_EVT_KICKBACK,
	WC_EVT_TOGGLE_STATE,	// toggle some combination of weapon state bits
	WC_EVT_HIDE_LASER,		// temporarily hide the laser
	WC_EVT_COOLDOWN,		// adjust cooldowns (by default every action is cooled down after an attack)
	WC_EVT_SET_GRAVITY,		// change player gravity
	WC_EVT_DLIGHT,			// dynamic light
	WC_EVT_SERVER,			// custom server-side logic.
	WC_EVT_MUZZLEFLASH,

	// impact events
	WC_EVT_SPRITETRAIL,		// TE_SPRITETRAIL
	WC_EVT_DECAL,			// TE_DECAL
	WC_EVT_RADIUS_DAMAGE,	// server-side damage logic
	WC_EVT_EXPLOSION,		// TE_EXPLOSION
	WC_EVT_BEAM_CIRCLE,		// TE_BEAMCYLINDER / TE_BEAMTORUS / TE_BEAMDISK
	WC_EVT_GLOW_SPRITE,		// TE_GLOWSPRITE
	WC_EVT_SPARKS,			// TE_SPARKS
	WC_EVT_ARMOR_RICOCHET,	// TE_ARMOR_RICOCHET
	WC_EVT_QUAKE_EFFECT,	// TE_GUNSHOT / TE_TAREXPLOSION / TE_EXPLOSION2 / TE_LAVASPLASH / TE_TELEPORT / TE_PARTICLE_BURST
	WC_EVT_IMPLOSION,		// TE_IMPLOSION
	WC_EVT_SPRITE_SPRAY,	// TE_SPRITE_SPRAY
	WC_EVT_STREAK_SPLASH,	// TE_STREAK_SPLASH
	WC_EVT_SHAKE,			// screen shake (not predicted)

	WC_EVT_TOTAL,
};

// how loud the sound is for AI (reaction distance) (0 = silent)
enum WeaponCustomAiVol {
	WC_AIVOL_SILENT,
	WC_AIVOL_QUIET,
	WC_AIVOL_NORMAL,
	WC_AIVOL_LOUD,
};

enum WeaponCustomFlashSz {
	WC_FLASH_NONE,
	WC_FLASH_DIM,
	WC_FLASH_NORMAL,
	WC_FLASH_BRIGHT,
};

enum WeaponCustomAnimHand {
	WC_ANIM_BOTH_HANDS,
	WC_ANIM_LEFT_HAND,
	WC_ANIM_RIGHT_HAND,
	WC_ANIM_TRIG_HAND, // play on the hand that triggered this event
};

enum WeaponCustomProjectile
{
	WC_PROJECTILE_ARGRENADE = 1,
	WC_PROJECTILE_BANANA,
	WC_PROJECTILE_BOLT,
	WC_PROJECTILE_DISPLACER,
	WC_PROJECTILE_GRENADE,
	WC_PROJECTILE_HORNET,
	WC_PROJECTILE_HVR,
	WC_PROJECTILE_MORTAR,
	WC_PROJECTILE_RPG,
	WC_PROJECTILE_SHOCK,
	WC_PROJECTILE_WEAPON,
	WC_PROJECTILE_TRIPMINE,
	WC_PROJECTILE_CUSTOM,
	WC_PROJECTILE_OTHER,
};

enum WeaponCustomProjectileAction
{
	WC_PROJ_ACT_IMPACT = 1,
	WC_PROJ_ACT_BOUNCE,
	WC_PROJ_ACT_ATTACH
};

enum WeaponCustomProjectileFollowMode
{
	WC_PROJ_FOLLOW_NONE,
	WC_PROJ_FOLLOW_CROSSHAIRS,
	WC_PROJ_FOLLOW_ENEMIES,
};

enum WeaponCustomToggleStateMode {
	WC_TOGGLE_STATE_OFF,
	WC_TOGGLE_STATE_ON,
	WC_TOGGLE_STATE_TOGGLE,
};

enum WeaponCustomBeamAnimation {
	WC_BEAM_ANIM_DISABLED,
	WC_BEAM_ANIM_TOGGLE,		// toggle between beam styles
	WC_BEAM_ANIM_LINEAR,		// linear ramp between beam style
	WC_BEAM_ANIM_LINEAR_TOGGLE, // linear ramp to alt style, then instant revert to main style
	WC_BEAM_ANIM_EASE_IN_OUT,	// smooth transitions between beam styles
};

enum WeaponCustomTracerColor {
	WC_TRACER_COLOR_WHITE,		// neon white
	WC_TRACER_COLOR_RED,		// neon red
	WC_TRACER_COLOR_GREEN,		// neon green
	WC_TRACER_COLOR_BLUE,		// neon blue
	WC_TRACER_COLOR_DEFAULT,	// colored by tracerred/tracergreen/tracerblue cvars
	WC_TRACER_COLOR_YELLOW,		// similar to the orange values
	WC_TRACER_COLOR_ORANGE2,	// slightly pinkish
	WC_TRACER_COLOR_BLUE2,		// pale purple-ish blue
	WC_TRACER_COLOR_ORANGE3,	// identical to orange2
	WC_TRACER_COLOR_ORANGE4,	// slightly brighter
	WC_TRACER_COLOR_TAN,		// similar to the orange values
	WC_TRACER_COLOR_ORANGE,		// most saturated orange color
};

enum WeaponCustomBeamCircleType {
	WC_BEAM_CIRCLE_TYPE_CYLINDER,	// TE_BEAMCYLINDER
	WC_BEAM_CIRCLE_TYPE_TORUS,		// TE_BEAMTORUS
	WC_BEAM_CIRCLE_TYPE_DISK,		// TE_BEAMDISK
};

enum WeaponCustomQuakeEffectType {
	WC_QUAKE_EFFECT_GUNSHOT,
	WC_QUAKE_EFFECT_EXPLOSION,
	WC_QUAKE_EFFECT_EXPLOSION2,
	WC_QUAKE_EFFECT_LAVASPLASH,
	WC_QUAKE_EFFECT_TELEPORT,
	WC_QUAKE_EFFECT_PARTICLE_BURST,
};

enum WeaponCustomRecoilAngleMode {
	WC_RECOIL_ANGLES_COPY,					// use angles as given
	WC_RECOIL_ANGLES_RANDOM,				// randomize angles between +/- of each value (e.g. 2 = range of [-2,2])
	WC_RECOIL_ANGLES_RANDOM_SIMPLE,			// ramdonly select +/- of each value (e.g. 2 = -2 or 2)
	WC_RECOIL_ANGLES_RANDOM_RANGE,			// randomize between the min and max angles
	WC_RECOIL_ANGLES_RANDOM_RANGE_SIMPLE,	// randomly select a min or max angle
	WC_RECOIL_ANGLES_LINEAR_RAMP,			// interpolate angles from the min to max values. Randomize using the interpolated values.
};

enum WeaponCustomRecoilApplyMode {
	WC_RECOIL_APPLY_PUNCH_SET,	// set punch angle
	WC_RECOIL_APPLY_PUNCH_ADD,	// add to punch angle
	WC_RECOIL_APPLY_ROTATE,		// rotate the player's view
};

#pragma pack(push,1)

struct WepEvtArr8 {
	uint8_t arrSz;
	uint8_t arr[MAX_WC_RANDOM_SELECTION];

	void add(uint8_t val) {
		if (arrSz < MAX_WC_RANDOM_SELECTION) {
			arr[arrSz++] = val;
		}
	}
};

struct WepEvtArr16 {
	uint8_t arrSz;
	uint16_t arr[MAX_WC_RANDOM_SELECTION];

	void add(uint16_t val) {
		if (arrSz < MAX_WC_RANDOM_SELECTION) {
			arr[arrSz++] = val;
		}
	}
};

#define EVT_TYPE_BITS 6
#define EVT_TRIGGER_BITS 6

struct WepEvt {
	uint16_t evtType : EVT_TYPE_BITS;		// which union struct to use
	uint16_t trigger : EVT_TRIGGER_BITS;	// when to trigger the event
	uint16_t hasTrigArg : 1;
	uint16_t hasDelay : 1;
	uint16_t hasOffset : 1;		// for impact events
	uint16_t dummy : 1;			// reserved for later

	// conditionally sent args
	uint8_t triggerArg;			// additional args for event triggering
	uint16_t delay;				// milliseconds before firing the event
	int16_t offset;				// offset from impact position

	int attackIdx; // attack index which triggered this event (not transferred to clients) (TODO: move to runtime class data)

	// event arguments selected by evtType
	union {
		struct {
			uint16_t sound;		// 9 bits
			uint16_t volume;	// 7 bits
		} idleSound;

		struct {
			uint16_t sound; // 9 bits
			uint16_t channel; // 3 bits
			uint16_t aiVol; // 2 bits - WeaponCustomAiVol
			uint16_t flags; // 2 bits - FL_WC_SOUND_
			uint8_t volume;
			uint8_t attn;
			uint8_t pitchMin;
			uint8_t pitchMax;

			// add more sounds for random selection
			WepEvtArr16 additionalSounds;

			uint8_t distantSound; // not for prediction
		} playSound;

		struct {
			uint8_t newBody;
		} setBody;

		struct {
			uint16_t model;		// 12 bits
			uint16_t sound;		// 2 bits - TE_BOUNCE_*
			uint16_t hasVel;	// 1 bit - true if using a custom velocity
			uint16_t hasRand;	// 1 bit - true if using a custom velocity randomization
			int8_t position[3];	// x, y, z = forward, up, right
			int8_t vel[3];		// x, y, z = forward, up, right
			uint8_t dirRand;	// amount to randomize velocity direction
			uint8_t speedRand;	// amount to randomize velocity speed
		} ejectShell;

		struct {
			uint8_t angleOp;		// 3 bits - WeaponCustomRecoilAngleMode
			uint8_t viewOp;			// 2 bits - WeaponCustomRecoilApplyMode
			uint8_t flags;			// 2 bits - FL_WC_PUNCH_
			uint8_t hasMaxAngles;	// 1 bit
			int16_t angles[3];		// 9.7 fixed point int
			
			uint16_t maxAngleTime;	// time to reach maxAngles amount of recoil for a sustained attack
			int16_t maxAngles[3];	// 9.7 fixed point int. Not used if maxAngleTime=0
		} recoil;

		struct {
			uint8_t angleOp[3];
			uint8_t viewOp[3];

			uint8_t flags;			// FL_WC_PUNCH_
			uint8_t ops[3];			// bitpacked: WeaponCustomRecoilAngleMode + WeaponCustomRecoilApplyMode
			int16_t min[3];			// 9.7 fixed point int
			int16_t max[3];			// 9.7 fixed point int
			uint16_t maxAngleTime;	// time to reach maxAngles amount of recoil for a sustained attack
		} recoilAdv;

		struct {
			uint8_t flags;			// 3 bits - FL_WC_ANIM_*
			uint8_t hasCooldown;	// 1 bit - if set, cooldown attacks and idle animations
			uint8_t hasWeights;		// 1 bit - if set, use weights to influence random selection
			uint8_t akimbo;			// 3 bits - WeaponCustomAnimHand
			WepEvtArr8 anims;

			uint16_t cooldown;		// block attacks for this long.
			WepEvtArr8 weights;		// weights for each anim, for random selection
		} anim;

		struct {
			uint8_t count;			// how many bullets are shot at once
			uint16_t burstDelay;	// milliseconds betwen shots, for burst fire
			uint16_t damage;		// damage per bullet
			uint16_t accuracy[2];	// X/Y accuracy. 0 = perfect, 1 = 180 degrees. 65535 = 1.0f
			uint8_t tracerFreq;		// 4 bits. how often to display a tracer (0 = never, 1 = always, 2 = every other shot)
			uint8_t tracerColor;	// 4 bits. WeaponCustomTracerColor
			uint8_t flashSz;		// 4 bits. WeaponCustomFlashSz
			uint8_t flags;			// 4 bits. FL_WC_BULLETS_*
		} bullets;

		struct {
			uint8_t flags;			// 8 bits. FL_WC_BEAM_*
			uint8_t attachment;		// 3 bits. only 0-4 are valid
			uint8_t hasRicoBeams;	// 1 bit.
			uint16_t sprite;		// 9 bits

			uint8_t ricoBeams;		// max number of ricochet beams
			uint16_t ricoAngle;		// 0 = perfect, 1 = 180 degrees. 65535 = 1.0f

			uint8_t id;				// 4 bits. ID used to update the beam in future events, for constant beams. 0 = always create a new beam
			uint8_t altMode;		// 3 bits. WeaponCustomBeamAnimation
			uint8_t hasImpactSprite;// 1 bit
			uint16_t life;			// how long to keep the beam active (millis). 0 = forever (egon)
			uint16_t accuracy[2];	// X/Y accuracy. 0 = perfect, 1 = 180 degrees. 65535 = 1.0f
			uint16_t damage;		// damage per beam
			uint16_t distance;		// max beam distance
			uint16_t freq;			// how often to apply damage (millis). 0 = once per attack.

			uint8_t width;
			uint8_t noise;
			uint8_t scrollRate;
			RGBA color;

			uint16_t altTime;		// time to transition between styles (millis)
			uint8_t widthAlt;
			uint8_t noiseAlt;
			uint8_t scrollRateAlt;
			RGBA colorAlt;

			uint16_t impactSprite;		// 9 bits
			uint16_t impactSpriteFps;	// 7 bits. 0 - 128
			uint8_t impactSpriteScale;
			RGBA impactSpriteColor;
		} beam;

		struct {
			int16_t pushForce;	// Push force applied to player in opposite direction of aim
			int8_t back;		// force percentage applied to back direction (-100 - 100)
			int8_t right;		// force percentage applied to right direction (-100 - 100)
			int8_t up;			// force percentage applied to up direction (-100 - 100)
			int8_t globalUp;	// force percentage applied to global up direction (view pitch ignored)
		} kickback;

		struct {
			uint16_t millis;
			uint8_t targets; // FL_WC_COOLDOWN_*
		} cooldown;

		struct {
			uint16_t millis; // how long to wait before enabling the laser again
		} laserHide;

		struct {
			uint16_t toggleMode;	// 2 bits - WeaponCustomToggleStateMode
			uint16_t stateBits;		// 14 bits - combination of FL_WC_STATE_*
		} toggleState;

		struct {
			int16_t gravity; // gravity percentage (1000 = 100%, 0 = default)
		} setGravity;

		struct {
			uint8_t radius;
			RGB color;
			uint8_t life;
			uint8_t decayRate;
		} dlight;

		struct {
			uint16_t sprite;
			uint8_t count;
			uint8_t scale;
			uint8_t speed;
			uint8_t speedNoise;
		} spriteTrail;

		struct {
			uint8_t decalIdx;
			uint8_t flags;		// FL_WC_DECAL_*
		} decal;

		struct {
			uint8_t brightness; // WeaponCustomFlashSz
		} muzzleFlash;

		struct {
			uint16_t sprite;	// 9 bits
			uint16_t flags;		// 7 bits - FL_WC_EXPSPRITE_*
			uint8_t scale;
			uint8_t fps;
		} te_explosion;

		struct {
			uint16_t sprite;
			uint8_t life;
			uint8_t scale;
			uint8_t alpha;
		} glow_sprite;

		struct {
			uint8_t dummy; // all events must have at least 1 arg
		} sparks;

		struct {
			uint8_t type;				// 7 bits - WeaponCustomQuakeEffectType
			uint8_t isParticleBurst;	// 1 bit - it true, other args are sent
			uint16_t radius;
			uint8_t color;
			uint8_t life;
		} quake_effect;

		struct {
			uint8_t scale;
		} armor_ricochet;

		struct {
			uint8_t tracers;
			uint8_t radius;
			uint8_t life;
		} implosion;

		struct {
			uint16_t sprite;
			uint8_t count;
			uint8_t speed;
			uint8_t randomness;
		} sprite_spray;

		struct {
			uint8_t count;
			uint8_t color;
			uint16_t speed;
			uint16_t randomness;
		} streak_splash;

		struct {
			uint16_t sprite;	// 9 bits
			uint16_t beamType;	// 4 bits - WeaponCustomBeamCircleType
			uint16_t hasFrame;	// 1 bit - true if a start frame is set
			uint16_t hasHeight;	// 1 bit - true if a height is set (cylinder and torus)
			uint16_t hasNoise;	// 1 bit - true if a noise is set (cylinder)
			int16_t radius;		// max radius
			uint8_t life;		// display time
			RGBA color;			// beam color

			uint8_t frame;		// sprite frame
			uint8_t height;		// height for cylinder/torus types
			uint8_t noise;		// distortion for cylinder type
		} beam_circle;

		//
		// Server-side events
		//

		struct {
			uint16_t radius;
			uint16_t amplitude;
			uint16_t duration;
			uint16_t frequency;
		} shake;

		struct {
			uint16_t radius;
			uint16_t damage;
			uint32_t damageBits;
		} radiusDamage;

		struct {
			int flags; // FL_WC_PROJ_*
			uint16_t accuracy[2]; // X/Y accuracy. 0 = perfect, 1 = 180 degrees. 65535 = 1.0f
			bool hasAvel;

			uint8_t type;
			WeaponCustomProjectileAction world_event;
			WeaponCustomProjectileAction monster_event;
			int speed;				// speed = initial speed of the projectile
			uint16_t life;
			float elasticity;		// percentage of reflected velocity
			float gravity;			// percentage of normal gravity
			float air_friction;
			float water_friction;
			float size;				// hull size (all dimensions)
			float dir[3];			// dir = Direction of the projectile, relative to the aim direction.
			//       Coordinates are given as Right, Up, and Forward units.
			//       Usually you want to set this to straight forward (0 0 1).
			string_t entity_class;	// custom projectile entity
			
			uint16_t model;
			uint8_t renderMode;
			uint8_t renderAmt;
			uint8_t renderFx;
			float scale; // sprites only
			float framerate;

			string_t move_snd;
			uint16_t damage;
			uint32_t damageBits;

			string_t sprite;
			RGBA sprite_color;
			float sprite_scale;

			float angles[3];
			float avel[3];
			float position[3];			// offset from view position given in: right, forward, up
			float player_vel_inf[3];

			uint8_t follow_mode;
			float follow_radius;
			float follow_angle;
			float follow_time[3];

			uint16_t trail_spr;
			uint16_t trail_life;
			uint8_t trail_width;
			RGBA trail_color;
		} proj;

		// user defined server event
		// not networked to the client
		struct {
			uint8_t type;
			int iuser1;
			int iuser2;
			int iuser3;
			int iuser4;
			float fuser1;
			float fuser2;
			float fuser3;
			float fuser4;
			float vuser1[3];
			float vuser2[3];
			float vuser3[3];
			float vuser4[3];
			string_t suser1;
			string_t suser2;
			string_t suser3;
			string_t suser4;
			RGBA cuser1;
			RGBA cuser2;
			RGBA cuser3;
			RGBA cuser4;
			int euser1;
			int euser2;
			int euser3;
			int euser4;
		} server;
	};

	WepEvt() {
		memset(this, 0, sizeof(WepEvt));
	}

	WepEvt(int trigger, int triggerArg = 0, int evtType = 0, int delay = 0) {
		memset(this, 0, sizeof(WepEvt));
		this->trigger = trigger;
		this->triggerArg = triggerArg;
		this->delay = delay;
		this->hasTrigArg = triggerArg != 0;
		this->hasDelay = delay != 0;
		this->evtType = evtType;
	}

	WepEvt clone() {
		WepEvt dup;
		memcpy(&dup, this, sizeof(WepEvt));
		return dup;
	}

	WepEvt Type(int evtType) {
		this->evtType = evtType;
		return *this;
	}

	WepEvt Delay(uint16_t millis) {
		this->delay = millis;
		this->hasDelay = millis != 0;
		return *this;
	}
};

#pragma pack(pop)