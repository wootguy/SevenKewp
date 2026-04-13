#pragma once
#include <stdint.h>
#include <EHandle.h>

#define FLOAT_TO_FP_10_6(val) (clamp((int)(val * 64), INT16_MIN, INT16_MAX))
#define FP_10_6_TO_FLOAT(val) (val / 64.0f)

#define FLOAT_TO_SPREAD(val) (clamp((int)(val * 65535), 0, UINT16_MAX))
#define SPREAD_TO_FLOAT(val) (val / 65535.0f)

#define FLOAT_TO_MOVESPEED_MULT(val) clamp((val * 65535.0f), 1, 65535)
#define MOVESPEED_MULT_TO_FLOAT(val) (val ? val / 65535.0f : 1.0f)

#define MAX_WC_EVENTS 64
#define MAX_WC_RANDOM_SELECTION 8

#define FL_WC_WEP_HAS_PRIMARY		(1<<0)
#define FL_WC_WEP_HAS_SECONDARY		(1<<1)
#define FL_WC_WEP_HAS_TERTIARY		(1<<2)
#define FL_WC_WEP_HAS_ALT_PRIMARY	(1<<3)	// alternate primary fire toggled by laser or zooming
#define FL_WC_WEP_SHOTGUN_RELOAD	(1<<4)	// start animation + load animation (repeated) + finish animation
#define FL_WC_WEP_UNLINK_COOLDOWNS	(1<<5)	// primary and secondary attacks cooldown independently
#define FL_WC_WEP_AKIMBO			(1<<6)	// weapon has an akimbo mode
#define FL_WC_WEP_LINK_CHARGEUPS	(1<<7)	// primary and secondary chargeup state and events are shared (minigun behavior)
#define FL_WC_WEP_PRIMARY_PRIORITY	(1<<8)	// primary fire has priority over secondary when both attack buttons are pressed
#define FL_WC_WEP_EXCLUSIVE_HOLD	(1<<9)	// weapon must be dropped before switching to other weapons
#define FL_WC_WEP_USE_ONLY			(1<<10)	// weapon is collectable with the use key, not by touching
#define FL_WC_WEP_HAS_LASER			(1<<11)
#define FL_WC_WEP_DYNAMIC_ACCURACY	(1<<12) // crosshair widens with movement and shrinks when crouched
#define FL_WC_WEP_ZOOM_SPR_STRETCH	(1<<13) // zoom crosshair stretches to fit the screen
#define FL_WC_WEP_ZOOM_SPR_ASPECT	(1<<14) // zoom crosshair keeps its aspect ratio when stretched to fit the screen, and borders are filled black
#define FL_WC_WEP_EMPTY_IDLES		(1<<15) // The last half of idles are for when the clip is empty
#define FL_WC_WEP_NO_PREDICTION		(1<<16) // Disable client-side prediction entirely

#define FL_WC_SHOOT_UNDERWATER 1
#define FL_WC_SHOOT_NO_ATTACK 2			// don't run standard weapon attack logic (shoot animations, clicking)
#define FL_WC_SHOOT_COOLDOWN_IDLE 4		// cooldown the idle animations even if not attacking
#define FL_WC_SHOOT_NEED_AKIMBO 8		// don't allow attack if not holding the akimbo version of the weapon
#define FL_WC_SHOOT_NEED_FULL_COST 16	// don't allow attack if clip is less than ammo cost
#define FL_WC_SHOOT_NO_AUTOFIRE 32		// one shot per click
#define FL_WC_SHOOT_IS_MELEE 64			// use server-side crowbar attack logic

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

#define FL_WC_PUNCH_SET 1		// don't randomize angles, set to exactly what was given
#define FL_WC_PUNCH_ADD 2		// add to current punch angle
#define FL_WC_PUNCH_NO_RETURN 4	// view does not return to center after the punch
#define FL_WC_PUNCH_DUCK 8		// apply punch angles only when ducked
#define FL_WC_PUNCH_STAND 16	// apply punch angles only while standing

#define FL_WC_BEAM_SPIRAL	1		// render the beam as a spiral (egon effect)
#define FL_WC_BEAM_OPAQUE	2		// render the beam without transparency
#define FL_WC_BEAM_SHADEIN	4		// fade the start of the beam
#define FL_WC_BEAM_SHADEOUT	8		// fade the end of the beam

#define FL_WC_SOUND_CHARGE_PITCH 1	// Sound pitch increases with chargeup progress

#define FL_WC_CHARGE_DAMAGE		1		// attack charge progress scales damage events
#define FL_WC_CHARGE_KICKBACK	2		// attack charge progress scales kickback events

enum WeaponCustomEventTriggerShootArg {
	WC_TRIG_SHOOT_ARG_ALWAYS,		// always fire the shoot event
	WC_TRIG_SHOOT_ARG_AKIMBO,		// only fire the event when in akimbo mode
	WC_TRIG_SHOOT_ARG_NOT_AKIMBO,	// only fire the event when not in akimbo mode
};

// special clip size conditions
enum WeaponCustomEventTriggerClipSpArg {
	WC_TRIG_CLIP_ARG_ODD,		// fire on odd clip sizes
	WC_TRIG_CLIP_ARG_EVEN,		// fire on even clip sizes
	WC_TRIG_CLIP_ARG_NOT_EMPTY,	// fire on non-zero clip sizes
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

enum WeaponCustomEventTriggers {
	WC_TRIG_PRIMARY,			// trigger arg: WeaponCustomEventTriggerShootArg (does not trigger when alternate fire is active)
	WC_TRIG_SECONDARY,			// trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_TERTIARY,
	WC_TRIG_PRIMARY_ALT,		// triggers on alternate primary fire (laser/zoom)
	WC_TRIG_PRIMARY_CLIPSIZE,	// trigger arg is the clip size to trigger on
	WC_TRIG_PRIMARY_CLIP_SP,	// trigger arg: WeaponCustomEventTriggerClipSpArg
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
	WC_TRIG_DEPLOY,				// Trigger arg : WeaponCustomEventTriggerShootArg
	WC_TRIG_BULLET_FIRED,		// triggered when a bullet is fired
	WC_TRIG_LASER_ON,			// triggered when the laser is enabled
	WC_TRIG_LASER_OFF,			// triggered when the laser is disabled
	WC_TRIG_ZOOM_IN,			// triggered when zooming in
	WC_TRIG_ZOOM_OUT,			// triggered when zooming out
	WC_TRIG_IMPACT,				// triggered when an attack trace impacts something. Trigger arg: WeaponCustomEventTriggerImpactArg
};

enum WeaponCustomEventType {
	WC_EVT_IDLE_SOUND,		// simple sound playback for reloads
	WC_EVT_PLAY_SOUND,		// advanced sound playback
	WC_EVT_EJECT_SHELL,
	WC_EVT_PUNCH,			// set punch angles to the given values
	WC_EVT_SET_BODY,
	WC_EVT_WEP_ANIM,
	WC_EVT_BULLETS,
	WC_EVT_BEAM,
	WC_EVT_PROJECTILE,		// for slow-moving projectiles that aren't predicted on the client
	WC_EVT_KICKBACK,
	WC_EVT_MUZZLE_FLASH,
	WC_EVT_TOGGLE_STATE,	// toggle some combination of weapon state bits
	WC_EVT_TOGGLE_ZOOM,
	WC_EVT_HIDE_LASER,		// temporarily hide the laser
	WC_EVT_COOLDOWN,		// adjust cooldowns (by default every action is cooled down after an attack)
	WC_EVT_SET_GRAVITY,		// change player gravity
	WC_EVT_DLIGHT,			// dynamic light
	WC_EVT_SERVER,			// custom server-side logic.
	WC_EVT_MUZZLEFLASH,
	
	// impact events
	WC_EVT_SPRITETRAIL,		// TE_SPRITETRAIL
	WC_EVT_DECAL,			// TE_DECAL
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

enum WeaponCustomAmmoPool
{
	WC_AMMOPOOL_DEFAULT,			// automatically select ammo pool (primary = clip, secondary = secondary)
	WC_AMMOPOOL_PRIMARY_CLIP,		// drain ammo from primary clip
	WC_AMMOPOOL_PRIMARY_RESERVE,	// drain ammo from primary reserve
	WC_AMMOPOOL_SECONDARY_RESERVE,	// drain ammo from secondary reserve
};

enum WeaponCustomToggleStateMode {
	WC_TOGGLE_STATE_OFF,
	WC_TOGGLE_STATE_ON,
	WC_TOGGLE_STATE_TOGGLE,
};

enum WeaponCustomChargeupMode {
	WC_CHARGEUP_NONE,			// disable attack charging
	WC_CHARGEUP_CONSTANT,		// fire constantly after chargeup finishes
	WC_CHARGEUP_SINGLE,			// fire a single shot when charged up, then charge down
	WC_CHARGEUP_SINGLE_HOLD,	// fire a single shot when charged up, then charge down, unless the attack key was released before the charge up finished.
	WC_CHARGEUP_HOLD,			// fire a single shot when the player releases the attack button
};

enum WeaponCustomOverchargeMode {
	WC_OVERCHARGE_CANCEL,		// cancel attack after overcharge
	WC_OVERCHARGE_CONTINUE,		// continue charging after overcharge
};

enum WeaponCustomChargeAmmoMode {
	WC_CHARGE_AMMO_ATTACK,		// spend ammo when the charge up is finished and the attack starts
	WC_CHARGE_AMMO_LOAD,		// spend ammo as the charge up progresses, up to the full cost at 100% charge
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

struct WepEvt {
	uint16_t evtType : 5;
	uint16_t trigger : 5;		// when to trigger the event
	uint16_t triggerArg : 5;	// additional args for event triggering
	uint16_t hasDelay : 1;
	uint16_t delay : 16;		// milliseconds before firing the event

	int attackIdx; // attack index which triggered this event (not transferred to clients)

	// event arguments
	union {
		struct {
			uint16_t sound : 9;
			uint16_t volume : 7;
		} idleSound;

		struct {
			uint16_t sound : 9;
			uint16_t channel : 3;
			uint16_t aiVol : 2; // WeaponCustomAiVol
			uint16_t flags : 2; // FL_WC_SOUND_
			uint8_t volume;
			uint8_t attn;
			uint8_t pitchMin;
			uint8_t pitchMax;

			// add more sounds for random selection
			uint8_t numAdditionalSounds;
			uint16_t additionalSounds[MAX_WC_RANDOM_SELECTION];

			uint8_t distantSound; // not for prediction
		} playSound;

		struct {
			uint8_t newBody;
		} setBody;

		struct {
			uint16_t model : 12;
			uint16_t sound : 2; // TE_BOUNCE_*
			uint16_t hasVel : 1; // true if using a custom velocity
			uint16_t hasRand : 1; // true if using a custom velocity randomization
			int8_t offsetForward;
			int8_t offsetUp;
			int8_t offsetRight;
			int8_t velForward;	// velocity relative to look direction
			int8_t velUp;
			int8_t velRight;
			uint8_t dirRand;	// amount to randomize velocity direction
			uint8_t speedRand;	// amount to randomize velocity speed
		} ejectShell;

		struct {
			uint8_t flags; //  FL_WC_PUNCH_
			int16_t x; // 10.6 fixed point int
			int16_t y;
			int16_t z;
		} punch;

		struct {
			uint8_t flags : 5; // FL_WC_ANIM_*
			uint8_t akimbo : 3; // WeaponCustomAnimHand
			uint8_t numAnim;
			uint8_t anims[MAX_WC_RANDOM_SELECTION];
		} anim;

		struct {
			uint8_t count;			// how many bullets are shot at once
			uint16_t burstDelay;	// milliseconds betwen shots, for burst fire
			uint16_t damage;		// damage per bullet
			uint16_t spreadX;		// accuracy (0 = perfect, 1 = 180 degrees). 65535 = 1.0f
			uint16_t spreadY;		// accuracy (0 = perfect, 1 = 180 degrees). 65535 = 1.0f
			uint8_t tracerFreq : 4;	// how often to display a tracer (0 = never, 1 = always, 2 = every other shot)
			uint8_t tracerColor : 4;// WeaponCustomTracerColor
			uint8_t flashSz : 4;	// WeaponCustomFlashSz
			uint8_t flags : 4;		// FL_WC_BULLETS_*
		} bullets;

		struct {
			uint16_t flags : 4; // FL_WC_BEAM_*
			uint16_t attachment : 3; // only 0-4 are valid
			uint16_t sprite : 9;

			uint8_t id : 4;			// ID used to update the beam in future events, for constant beams. 0 = always create a new beam
			uint8_t altMode : 3;	// WeaponCustomBeamAnimation
			uint8_t hasImpactSprite : 1;
			uint16_t life;			// how long to keep the beam active (millis). 0 = forever (egon)
			uint16_t spreadX;		// accuracy (0 = perfect, 1 = 180 degrees). 65535 = 1.0f
			uint16_t spreadY;		// accuracy (0 = perfect, 1 = 180 degrees). 65535 = 1.0f
			uint16_t damage;		// damage per beam
			uint16_t distance;		// max beam distance
			uint16_t freq;			// how often to apply damage (millis). 0 = once per attack.

			uint8_t width;
			uint8_t noise;
			uint8_t scrollRate;
			RGBA color;

			uint16_t altTime : 12; // time to transition between styles (millis)
			uint8_t widthAlt;
			uint8_t noiseAlt;
			uint8_t scrollRateAlt;
			RGBA colorAlt;

			uint16_t impactSprite : 9;
			uint16_t impactSpriteFps : 7; // 0 - 128
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
			uint8_t zoomFov;
			uint8_t zoomFov2; // 2nd level zoom FOV
		} zoomToggle;

		struct {
			uint16_t millis;
			uint8_t targets; // FL_WC_COOLDOWN_*
		} cooldown;

		struct {
			uint16_t millis; // how long to wait before enabling the laser again
		} laserHide;

		struct {
			uint16_t toggleMode : 2; // WeaponCustomToggleStateMode
			uint16_t stateBits : 14;
		} toggleState;

		struct {
			int16_t gravity; // gravity percentage (1000 = 100%, 0 = default)
		} setGravity;

		struct {
			uint8_t radius;
			uint8_t r, g, b;
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
			uint8_t isGunshot; // create gunshot particle effects
		} decal;

		// not networked to the client. No bit packing necessary
		struct {
			int flags; // FL_WC_PROJ_*
			float spreadX;
			float spreadY;
			bool hasAvel;

			uint8_t type;
			WeaponCustomProjectileAction world_event;
			WeaponCustomProjectileAction monster_event;
			float speed;			// speed = initial speed of the projectile
			float life;
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
			string_t move_snd;
			float damage;
			int damageBits;

			string_t sprite;
			RGBA sprite_color;
			float sprite_scale;

			float angles[3];
			float avel[3];
			float offset[3];	// offset = offset from view position given in: right, forward, up
			float player_vel_inf[3];

			uint8_t follow_mode;
			float follow_radius;
			float follow_angle;
			float follow_time[3];

			string_t trail_spr;
			int trail_life;
			int trail_width;
			RGBA trail_color;
		} proj;

		struct {
			uint8_t brightness; // WeaponCustomFlashSz
		} muzzleFlash;

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
			EHANDLE euser1;
			EHANDLE euser2;
			EHANDLE euser3;
			EHANDLE euser4;
		} server;
	};

	WepEvt() {
		memset(this, 0, sizeof(WepEvt));
	}

#ifndef CLIENT_DLL

	WepEvt(int trigger, int delay=0, int triggerArg=0) {
		memset(this, 0, sizeof(WepEvt));
		this->trigger = trigger;
		this->triggerArg = triggerArg;
		this->delay = delay;
	}

	//
	// Event conditions
	//

	WepEvt Primary() {
		this->trigger = WC_TRIG_PRIMARY;
		return *this;
	}

	WepEvt Secondary() {
		this->trigger = WC_TRIG_SECONDARY;
		return *this;
	}

	WepEvt Tertiary() {
		this->trigger = WC_TRIG_TERTIARY;
		return *this;
	}

	WepEvt PrimaryAlt() {
		this->trigger = WC_TRIG_PRIMARY_ALT;
		return *this;
	}

	WepEvt PrimaryClip(uint8_t clipSize) {
		this->trigger = WC_TRIG_PRIMARY_CLIPSIZE;
		this->triggerArg = clipSize;
		return *this;
	}

	WepEvt PrimaryOdd() {
		this->trigger = WC_TRIG_PRIMARY_CLIP_SP;
		this->triggerArg = WC_TRIG_CLIP_ARG_ODD;
		return *this;
	}

	WepEvt SecondaryOdd() {
		this->trigger = WC_TRIG_SECONDARY_CLIP_SP;
		this->triggerArg = WC_TRIG_CLIP_ARG_ODD;
		return *this;
	}

	WepEvt PrimaryEven() {
		this->trigger = WC_TRIG_PRIMARY_CLIP_SP;
		this->triggerArg = WC_TRIG_CLIP_ARG_EVEN;
		return *this;
	}

	WepEvt SecondaryEven() {
		this->trigger = WC_TRIG_SECONDARY_CLIP_SP;
		this->triggerArg = WC_TRIG_CLIP_ARG_EVEN;
		return *this;
	}

	WepEvt PrimaryEmpty() {
		this->trigger = WC_TRIG_PRIMARY_CLIPSIZE;
		this->triggerArg = 0;
		return *this;
	}

	WepEvt SecondaryEmpty() {
		this->trigger = WC_TRIG_SECONDARY_CLIPSIZE;
		this->triggerArg = 0;
		return *this;
	}

	WepEvt PrimaryNotEmpty() {
		this->trigger = WC_TRIG_PRIMARY_CLIP_SP;
		this->triggerArg = WC_TRIG_CLIP_ARG_NOT_EMPTY;
		return *this;
	}

	WepEvt SecondaryNotEmpty() {
		this->trigger = WC_TRIG_SECONDARY_CLIP_SP;
		this->triggerArg = WC_TRIG_CLIP_ARG_NOT_EMPTY;
		return *this;
	}

	WepEvt PrimaryCharge() {
		this->trigger = WC_TRIG_PRIMARY_CHARGE;
		return *this;
	}

	WepEvt SecondaryCharge() {
		this->trigger = WC_TRIG_SECONDARY_CHARGE;
		return *this;
	}

	WepEvt PrimaryOvercharge() {
		this->trigger = WC_TRIG_PRIMARY_OVERCHARGE;
		return *this;
	}

	WepEvt SecondaryOvercharge() {
		this->trigger = WC_TRIG_SECONDARY_OVERCHARGE;
		return *this;
	}

	WepEvt PrimaryStop() {
		this->trigger = WC_TRIG_PRIMARY_STOP;
		return *this;
	}

	WepEvt SecondaryStop() {
		this->trigger = WC_TRIG_SECONDARY_STOP;
		return *this;
	}

	WepEvt PrimaryStart() {
		this->trigger = WC_TRIG_PRIMARY_START;
		return *this;
	}

	WepEvt SecondaryStart() {
		this->trigger = WC_TRIG_SECONDARY_START;
		return *this;
	}

	WepEvt PrimaryFail() {
		this->trigger = WC_TRIG_PRIMARY_FAIL;
		return *this;
	}

	WepEvt SecondaryFail() {
		this->trigger = WC_TRIG_SECONDARY_FAIL;
		return *this;
	}

	WepEvt Reload() {
		this->trigger = WC_TRIG_RELOAD;
		return *this;
	}

	WepEvt ReloadEmpty() {
		this->trigger = WC_TRIG_RELOAD_EMPTY;
		return *this;
	}

	WepEvt ReloadNotEmpty() {
		this->trigger = WC_TRIG_RELOAD_NOT_EMPTY;
		return *this;
	}

	WepEvt ReloadFinish() {
		this->trigger = WC_TRIG_RELOAD_FINISH;
		return *this;
	}

	WepEvt Deploy() {
		this->trigger = WC_TRIG_DEPLOY;
		return *this;
	}

	WepEvt Delay(uint16_t millis) {
		this->delay = millis;
		this->hasDelay = millis != 0;
		return *this;
	}

	WepEvt NotAkimbo() {
		this->triggerArg = WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
		return *this;
	}

	WepEvt AkimboOnly() {
		this->triggerArg = WC_TRIG_SHOOT_ARG_AKIMBO;
		return *this;
	}

	WepEvt Impact(int type) {
		this->trigger = WC_TRIG_IMPACT;
		this->triggerArg = type;
		return *this;
	}

	//
	// Event actions
	//

	// play a weapon fidgeting sound (pumping, reloading, etc.)
	// prefer using this instead of animation events so that other players can hear the sound
	WepEvt IdleSound(int sound, float volume=1.0f) {
		evtType = WC_EVT_IDLE_SOUND;
		idleSound.sound = sound;
		idleSound.volume = (int)(volume * 127.5f);
		return *this;
	}

	WepEvt PlaySound(int sound, uint8_t channel, float volume, float attn, int pitch) {
		evtType = WC_EVT_PLAY_SOUND;
		playSound.sound = sound;
		playSound.channel = channel;
		playSound.volume = (int)(volume * 255.5f);
		playSound.attn = clampf(attn * 64, 0, 255.0f);
		playSound.pitchMin = pitch;
		playSound.pitchMax = pitch;
		return *this;
	}

	WepEvt PlaySound(int sound, uint8_t channel, float volume, float attn, int pitchMin, int pitchMax,
		uint8_t distantSound, uint8_t aiVol, int flags) {
		evtType = WC_EVT_PLAY_SOUND;
		playSound.sound = sound;
		playSound.channel = channel;
		playSound.aiVol = aiVol;
		playSound.volume = (int)(volume * 255.5f);
		playSound.attn = clampf(attn * 64, 0, 255.0f);
		playSound.pitchMin = pitchMin;
		playSound.pitchMax = pitchMax;
		playSound.distantSound = distantSound;
		playSound.flags = flags;
		return *this;
	}

	// add an additional sound to an existing PlaySound event, for random selection
	WepEvt AddSound(int sound) {
		if (evtType == WC_EVT_PLAY_SOUND) {
			if (playSound.numAdditionalSounds < MAX_WC_RANDOM_SELECTION) {
				playSound.additionalSounds[playSound.numAdditionalSounds++] = sound;
			}
			else {
				ALERT(at_error, "AddSound exceeded max random sounds\n");
			}
		}
		else {
			ALERT(at_error, "AddSound can be called on PlaySound events only\n");
		}
		return *this;
	}

	WepEvt SetBody(int newBody) {
		evtType = WC_EVT_SET_BODY;
		setBody.newBody = newBody;
		return *this;
	}

	WepEvt EjectShell(uint16_t model, int sound, float offsetForward, float offsetUp, float offsetRight,
		float velForward=0, float velUp=0, float velRight=0, float dirRand=0, float speedRand=0) {
		evtType = WC_EVT_EJECT_SHELL;
		ejectShell.model = model;
		ejectShell.sound = sound;
		ejectShell.hasVel = velForward || velUp || velRight;
		ejectShell.hasRand = dirRand || speedRand;
		ejectShell.offsetForward = offsetForward;
		ejectShell.offsetUp = offsetUp;
		ejectShell.offsetRight = offsetRight;
		ejectShell.velForward = velForward;
		ejectShell.velUp = velUp;
		ejectShell.velRight = velRight;
		ejectShell.dirRand = dirRand;
		ejectShell.speedRand = speedRand;
		return *this;
	}

	// prefer adding this AFTER a bullets event for prediction accuracy
	WepEvt PunchSet(float x, float y, float z = 0) {
		evtType = WC_EVT_PUNCH;
		punch.flags = FL_WC_PUNCH_SET;
		punch.x = FLOAT_TO_FP_10_6(x);
		punch.y = FLOAT_TO_FP_10_6(y);
		punch.z = FLOAT_TO_FP_10_6(z);
		return *this;
	}

	WepEvt PunchAdd(float x, float y, float z = 0) {
		evtType = WC_EVT_PUNCH;
		punch.flags = FL_WC_PUNCH_ADD;
		punch.x = FLOAT_TO_FP_10_6(x);
		punch.y = FLOAT_TO_FP_10_6(y);
		punch.z = FLOAT_TO_FP_10_6(z);
		return *this;
	}

	// prefer adding this AFTER a bullets event for prediction accuracy
	WepEvt PunchRandom(float x, float y, float z=0) {
		evtType = WC_EVT_PUNCH;
		punch.x = FLOAT_TO_FP_10_6(x);
		punch.y = FLOAT_TO_FP_10_6(y);
		punch.z = FLOAT_TO_FP_10_6(z);
		return *this;
	}

	// prefer adding this AFTER a bullets event for prediction accuracy
	WepEvt RotateView(float x, float y, float z = 0) {
		evtType = WC_EVT_PUNCH;
		punch.flags |= FL_WC_PUNCH_NO_RETURN;
		punch.x = FLOAT_TO_FP_10_6(x);
		punch.y = FLOAT_TO_FP_10_6(y);
		punch.z = FLOAT_TO_FP_10_6(z);
		return *this;
	}

	WepEvt DuckOnly() {
		if (evtType == WC_EVT_PUNCH) {
			punch.flags |= FL_WC_PUNCH_DUCK;
			punch.flags &= ~FL_WC_PUNCH_STAND;
		}
		else {
			ALERT(at_error, "DuckOnly can be called on Punch events only\n");
		}
		return *this;
	}

	WepEvt StandOnly() {
		if (evtType == WC_EVT_PUNCH) {
			punch.flags |= FL_WC_PUNCH_STAND;
			punch.flags &= ~FL_WC_PUNCH_DUCK;
		}
		else {
			ALERT(at_error, "StandOnly can be called on Punch events only\n");
		}
		return *this;
	}

	WepEvt WepAnim(uint8_t animIdx, uint8_t akimbo=WC_ANIM_BOTH_HANDS, uint8_t flags=0) {
		evtType = WC_EVT_WEP_ANIM;
		anim.anims[0] = animIdx;
		anim.numAnim = 1;
		anim.akimbo = akimbo;
		anim.flags = flags;
		return *this;
	}

	WepEvt AddAnim(uint8_t animIdx) {
		if (evtType == WC_EVT_WEP_ANIM) {
			if (playSound.numAdditionalSounds < MAX_WC_RANDOM_SELECTION) {
				anim.anims[anim.numAnim++] = animIdx;
			}
			else {
				ALERT(at_error, "AddAnim exceeded max random animations\n");
			}
		}
		else {
			ALERT(at_error, "AddAnim can be called on WepAnim events only\n");
		}
		return *this;
	}

	WepEvt Bullets(uint8_t count, uint16_t burstDelay, uint16_t damage, float spreadX, float spreadY,
		uint8_t tracerFreq = 1, uint8_t flashSz=WC_FLASH_NORMAL, uint8_t flags=0) {
		evtType = WC_EVT_BULLETS;
		bullets.count = count;
		bullets.burstDelay = burstDelay;
		bullets.damage = damage;
		bullets.spreadX = FLOAT_TO_SPREAD(spreadX);
		bullets.spreadY = FLOAT_TO_SPREAD(spreadY);
		bullets.tracerFreq = tracerFreq;
		bullets.tracerColor = WC_TRACER_COLOR_DEFAULT;
		bullets.flashSz = flashSz;
		bullets.flags = flags;
		return *this;
	}

	WepEvt BulletColor(uint8_t color) {
		bullets.tracerColor = color;
		return *this;
	}

	// Not necessary for bullet events which include a muzzleflash argument
	// brightness = WeaponCustomFlashSz
	WepEvt MuzzleFlash(uint8_t brightness) {
		evtType = WC_EVT_MUZZLEFLASH;
		muzzleFlash.brightness = brightness;
		return *this;
	}

	// id = used to update this beam in future events. 0 = always create a new beam
	// life = duration in millis. 0 = constant mode (egon)
	// flags = FL_WC_BEAM_*
	WepEvt Beam(uint8_t id, uint16_t life, uint16_t distance = 8192) {
		evtType = WC_EVT_BEAM;
		beam.id = id;
		beam.life = life;
		beam.distance = distance;
		beam.color = RGBA(255, 255, 255, 255);
		beam.width = 16;
		beam.attachment = 1;
		return *this;
	}

	// configure visual appearance of a Beam() event
	WepEvt BeamStyle(uint16_t sprite, RGBA color=RGBA(255, 255, 255, 255), uint8_t width = 16,
		uint8_t noise = 0, uint8_t scrollRate = 0, uint8_t attachment=1, uint8_t flags = 0) {
		beam.sprite = sprite;
		beam.color = color;
		beam.width = width;
		beam.noise = noise;
		beam.scrollRate = scrollRate;
		beam.attachment = attachment;
		beam.flags = flags;
		return *this;
	}

	// configure style animation
	// mode = WeaponCustomBeamAnimation
	WepEvt BeamStyleAlt(int mode, uint16_t animTime, RGBA color = RGBA(255, 255, 255, 255),
		uint8_t width = 16, uint8_t noise = 0, uint8_t scrollRate = 0) {
		beam.altMode = mode;
		beam.altTime = animTime;
		beam.colorAlt = color;
		beam.widthAlt = width;
		beam.noiseAlt = noise;
		beam.scrollRateAlt = scrollRate;
		return *this;
	}

	// configure beam accuracy and damage of a Beam() event
	WepEvt BeamDamage(uint16_t damage, float spreadX, float spreadY, uint16_t freq=0) {
		beam.damage = damage;
		beam.spreadX = FLOAT_TO_SPREAD(spreadX);
		beam.spreadY = FLOAT_TO_SPREAD(spreadY);
		beam.freq = freq;
		return *this;
	}

	WepEvt BeamImpactSprite(uint16_t sprite, uint8_t fps, uint8_t scale, RGBA color) {
		beam.hasImpactSprite = 1;
		beam.impactSprite = sprite;
		beam.impactSpriteFps = V_min(127, fps);
		beam.impactSpriteScale = scale;
		beam.impactSpriteColor = color;
		return *this;
	}

	WepEvt BulletFired() {
		trigger = WC_TRIG_BULLET_FIRED;
		return *this;
	}

	// Fill in WepEvt.proj separately to configure
	WepEvt Projectile(WeaponCustomProjectile type) {
		evtType = WC_EVT_PROJECTILE;
		proj.type = type;
		proj.speed = 400;
		(Vector)proj.dir = Vector(0,0,1);
		proj.world_event = WC_PROJ_ACT_IMPACT;
		proj.monster_event = WC_PROJ_ACT_IMPACT;
		proj.elasticity = 0.8f;
		proj.size = 0.001f;
		proj.dir[2] = 1.0f;
		return *this;
	}

	WepEvt LaserOn() {
		trigger = WC_TRIG_LASER_ON;
		return *this;
	}

	WepEvt LaserOff() {
		trigger = WC_TRIG_LASER_OFF;
		return *this;
	}

	WepEvt HideLaser(uint16_t millis) {
		evtType = WC_EVT_HIDE_LASER;
		laserHide.millis = millis;
		return *this;
	}

	WepEvt ZoomIn() {
		trigger = WC_TRIG_ZOOM_IN;
		return *this;
	}

	WepEvt ZoomOut() {
		trigger = WC_TRIG_ZOOM_OUT;
		return *this;
	}

	WepEvt Kickback(int16_t pushForce, int8_t back=100, int8_t right=0, int8_t up=0, int8_t globalUp=0) {
		evtType = WC_EVT_KICKBACK;
		kickback.pushForce = pushForce;
		kickback.back = clamp(back, -100, 100);
		kickback.right = clamp(right, -100, 100);
		kickback.up = clamp(up, -100, 100);
		kickback.globalUp = clamp(globalUp, -100, 100);
		return *this;
	}

	// stateBits = combination of FL_WC_STATE_*
	WepEvt EnableState(uint8_t stateBits) {
		evtType = WC_EVT_TOGGLE_STATE;
		toggleState.toggleMode = WC_TOGGLE_STATE_ON;
		toggleState.stateBits = stateBits;
		return *this;
	}

	// stateBits = combination of FL_WC_STATE_*
	WepEvt DisableState(uint8_t stateBits) {
		evtType = WC_EVT_TOGGLE_STATE;
		toggleState.toggleMode = WC_TOGGLE_STATE_OFF;
		toggleState.stateBits = stateBits;
		return *this;
	}

	// stateBits = combination of FL_WC_STATE_*
	WepEvt ToggleState(uint8_t stateBits) {
		evtType = WC_EVT_TOGGLE_STATE;
		toggleState.toggleMode = WC_TOGGLE_STATE_TOGGLE;
		toggleState.stateBits = stateBits;
		return *this;
	}

	WepEvt ToggleZoom(uint8_t zoomFov, uint8_t zoomFov2=0) {
		evtType = WC_EVT_TOGGLE_ZOOM;
		zoomToggle.zoomFov = zoomFov;
		zoomToggle.zoomFov2 = zoomFov2;
		return *this;
	}

	// targets = combination of FL_WC_COOLDOWN_
	WepEvt Cooldown(uint16_t millis, uint8_t targets) {
		evtType = WC_EVT_COOLDOWN;
		cooldown.millis = millis;
		cooldown.targets = targets;
		return *this;
	}

	WepEvt SetGravity(float gravity) {
		evtType = WC_EVT_SET_GRAVITY;
		setGravity.gravity = clampf(gravity * 1000, -32727, 32767);
		return *this;
	}

	WepEvt DLight(uint8_t radius=20, RGB c=RGB(255,255,255), uint8_t life = 0.1f, uint8_t decayRate = 0) {
		evtType = WC_EVT_DLIGHT;
		dlight.radius = radius;
		dlight.r = c.r;
		dlight.g = c.g;
		dlight.b = c.b;
		dlight.life = life;
		dlight.decayRate = decayRate;
		return *this;
	}

	WepEvt SpriteTrail(uint16_t sprite, uint8_t count, uint8_t scale, uint8_t speed, uint8_t speedNoise) {
		evtType = WC_EVT_SPRITETRAIL;
		spriteTrail.sprite = sprite;
		spriteTrail.count = count;
		spriteTrail.scale = scale;
		spriteTrail.speed = speed;
		spriteTrail.speedNoise = speedNoise;
		return *this;
	}

	WepEvt Decal(uint8_t decalIdx, bool gunshotEffects) {
		evtType = WC_EVT_DECAL;
		decal.isGunshot = gunshotEffects;
		decal.decalIdx = decalIdx;
		return *this;
	}

	// type = user defined
	// fill in WepEvt.server fields separately and create a CustomServerEvent() override
	WepEvt CustomServerLogic(uint8_t type) {
		evtType = WC_EVT_SERVER;
		server.type = type;
		return *this;
	}
#endif
};

struct MeleeOpts {
	float damage;
	int damageBits;
	float range;
	Vector attackOffset;	// in forward, up, right units
	uint16_t missCooldown;	// cooldown millis for missing an attack
	uint16_t hitCooldown;	// cooldown millis for hitting something
	uint16_t decalDelay;	// how long to wait after hitting a wall to apply the decal
	uint16_t hitWallSounds[4];
	uint16_t hitFleshSounds[4];
	uint16_t missSounds[4];
};

struct CustomWeaponShootOpts {
	uint8_t flags;				// FL_WC_SHOOT_*
	uint8_t ammoCost;			// ammo cost of each attack
	uint8_t ammoFreq;			// skip decrementing ammo for this many attacks (for fractional ammo costs)
	uint8_t ammoPool;			// which ammo pool to drain from (WeaponCustomAmmoPool)
	uint16_t cooldown;			// time between attacks (milliseconds)
	uint16_t cooldownFail;		// cooldown after a failed attack (out of ammo, underwater) (milliseconds)
	uint8_t chargeMode : 3;		// WeaponCustomChargeupMode
	uint8_t chargeAmmoMode : 2; // WeaponCustomChargeAmmoMode
	uint8_t overchargeMode : 2;	// WeaponCustomOverchargeMode
	uint8_t chargeFlags;		// FL_WC_CHARGE_*
	uint16_t chargeTime;		// how long the attack button must be held before the attack begins (milliseconds)
	uint16_t overchargeTime;	// how long an attack can be charged before triggering an overcharge event and cancelling the chargeup
	uint16_t chargeCancelTime;	// minimum time before a charge can be cancelled (milliseconds)
	uint16_t chargeMoveSpeedMult; // movement speed multiplier while charging (1-65535) (65535 = 100%) (0 = don't change)
	uint16_t accuracyX;			// horizontal accuracy for crosshair (degrees * 100)
	uint16_t accuracyY;			// vertical accuracy for crosshair (degrees * 100)
	uint16_t emptySound;		// custom empty click sound

	// server side settings (not networked)
	MeleeOpts melee;
};

struct WeaponCustomReload {
	uint8_t anim;
	uint16_t time; // milliseconds
};

struct WeaponCustomIdle {
	uint8_t anim;
	uint8_t weight;	// chance of selection (prefer 0-100 with all idles adding up to 100)
	uint16_t time;	// milliseconds before playing another idle animation
};

struct WeaponCustomAkimbo {
	WeaponCustomIdle idles[4];
	WeaponCustomReload reload;
	uint8_t deployAnim;			// deploy anim for a single weapon (used for reloading and toggling akimbo mode)
	uint16_t deployTime;
	uint8_t akimboDeployAnim;	// deploy anim when selecting the weapon
	uint16_t akimboDeployTime;  // time before you can attack
	uint16_t akimboDeployAnimTime;  // time before the weapon idles
	uint8_t holsterAnim;		// for reloading a single weapon
	uint16_t holsterTime;
	uint16_t accuracyX;			// horizontal accuracy for crosshair (degrees * 100)
	uint16_t accuracyY;			// vertical accuracy for crosshair (degrees * 100)
};

struct WeaponCustomLaser {
	WeaponCustomIdle idles[4];	// alternate set of idle animations that don't twist the laser
	uint16_t dotSprite;			// sprite used for the end point of the laser
	uint16_t beamSprite;		// sprite used for the beam of the laser
	uint8_t dotSz;				// dot sprite scale*10
	uint8_t beamWidth;
	uint8_t attachment;			// model attachment point for the beam effect (required for beam to display)
};

struct CustomWeaponParams {
	uint32_t flags; // FL_WC_WEP_*
	uint16_t maxClip;
	uint16_t vmodel;
	uint8_t deployAnim;
	uint16_t deployTime; // time before you can attack
	uint16_t deployAnimTime; // time before the weapon idles (length of the deployment animation)
	uint16_t moveSpeedMult; // move speed multiplier (1-65535) (65535 = 100%) (0 = don't change)
	int jumpPower;			// -1 = disabled, 0 = default velocity (800), 1+ = custom velocity

	// stage 0 and 1 usage depends on weapon flags:
	// 0 = simple reload animation OR starting animation for shotgun reload mode
	// 1 = simple reload animation (empty clip) OR shotgun reload middle animation (shell insertion)
	// 2 = shotgun reload finish animation (cocking)
	WeaponCustomReload reloadStage[3];

	WeaponCustomIdle idles[4]; // randomly selected idle animations
	CustomWeaponShootOpts shootOpts[4]; // primary, secondary, tertiary, and alt primary fire
	WeaponCustomAkimbo akimbo;
	WeaponCustomLaser laser;
	
	uint8_t numEvents;
	WepEvt events[MAX_WC_EVENTS];
};
