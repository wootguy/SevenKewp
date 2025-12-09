#pragma once
#include <stdint.h>

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

enum WeaponCustomEventTriggerShootArg {
	WC_TRIG_SHOOT_ARG_ALWAYS,		// always fire the shoot event
	WC_TRIG_SHOOT_ARG_AKIMBO,		// only fire the event when in akimbo mode
	WC_TRIG_SHOOT_ARG_NOT_AKIMBO,	// only fire the event when not in akimbo mode
};

enum WeaponCustomEventTriggers {
	WC_TRIG_PRIMARY,			// trigger arg: WeaponCustomEventTriggerShootArg (does not trigger when alternate fire is active)
	WC_TRIG_SECONDARY,			// trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_TERTIARY,
	WC_TRIG_PRIMARY_ALT,		// triggers on alternate primary fire (laser/zoom)
	WC_TRIG_PRIMARY_CLIPSIZE,	// trigger arg is the clip size to trigger on
	WC_TRIG_PRIMARY_ODD,		// triggers on odd clip sizes after firing
	WC_TRIG_PRIMARY_EVEN,		// triggers on even clip sizes after firing
	WC_TRIG_PRIMARY_NOT_EMPTY,	// triggers on non-empty clip size after firing
	WC_TRIG_PRIMARY_CHARGE,		// triggers when primary fire begins charging
	WC_TRIG_PRIMARY_STOP,		// triggers when primary fire key is released
	WC_TRIG_PRIMARY_FAIL,		// triggers when primary fire fails (no ammo, underwater, ...)
	WC_TRIG_SECONDARY_CHARGE,	// triggers when secondary fire begins charging
	WC_TRIG_SECONDARY_STOP,		// triggers when secondary fire key is released
	WC_TRIG_SECONDARY_FAIL,		// triggers when secondar fire fails (no ammo, underwater, ...)
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
};

enum WeaponCustomEventType {
	WC_EVT_IDLE_SOUND,		// simple sound playback for reloads
	WC_EVT_PLAY_SOUND,		// advanced sound playback
	WC_EVT_EJECT_SHELL,
	WC_EVT_PUNCH,			// set punch angles to the given values
	WC_EVT_SET_BODY,
	WC_EVT_WEP_ANIM,
	WC_EVT_BULLETS,
	WC_EVT_PROJECTILE,		// for slow-moving projectiles that aren't predicted on the client
	WC_EVT_KICKBACK,
	WC_EVT_MUZZLE_FLASH,
	WC_EVT_TOGGLE_ZOOM,
	WC_EVT_TOGGLE_LASER,
	WC_EVT_HIDE_LASER,		// temporarily hide the laser
	WC_EVT_COOLDOWN,		// adjust cooldowns (by default every action is cooled down after an attack)
	WC_EVT_TOGGLE_AKIMBO,	// toggle dual-wield mode
	WC_EVT_SET_GRAVITY,		// change player gravity
	WC_EVT_DLIGHT,			// dynamic light
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

struct WepEvt {
	uint16_t evtType : 5;
	uint16_t trigger : 5;		// when to trigger the event
	uint16_t triggerArg : 5;	// additional args for event triggering
	uint16_t hasDelay : 1;
	uint16_t delay : 16;		// milliseconds before firing the event

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
			uint16_t reserved : 2;
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
			uint16_t model;
			int16_t offsetForward; // 10.6 fixed point int
			int16_t offsetUp;
			int16_t offsetRight;
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
			uint8_t tracerFreq;		// how often to display a tracer (0 = never, 1 = always, 2 = every other shot)
			uint8_t flashSz : 4;	// WeaponCustomFlashSz
			uint8_t flags : 4;		// FL_WC_BULLETS_*
		} bullets;

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
			int16_t gravity; // gravity percentage (1000 = 100%, 0 = default)
		} setGravity;

		struct {
			uint8_t radius;
			uint8_t r, g, b;
			uint8_t life;
			uint8_t decayRate;
		} dlight;

		// not networked to the client. No bit packing necessary
		struct {
			float spreadX;
			float spreadY;
			bool hasAvel;

			uint8_t type;
			int world_event; // WeaponCustomProjectileAction
			int monster_event; // WeaponCustomProjectileAction
			float speed;
			float life;
			float elasticity; // percentage of reflected velocity
			float gravity; // percentage of normal gravity
			float air_friction;
			float water_friction;
			float size;		  // hull size (all dimensions)
			float dir[3];
			string_t entity_class; // custom projectile entity
			uint16_t model;
			uint16_t move_snd;

			uint16_t sprite;
			uint8_t sprite_color[4];
			float sprite_scale;

			float angles[3];
			float avel[3];
			float offset[3];
			float player_vel_inf[3];

			uint8_t follow_mode;
			float follow_radius;
			float follow_angle;
			float follow_time[3];

			uint16_t trail_spr;
			int trail_life;
			int trail_width;
			uint8_t trail_color[4];
			float trail_effect_freq;
			float bounce_effect_delay;
		} proj;
	};

#ifndef CLIENT_DLL
	WepEvt() {
		memset(this, 0, sizeof(WepEvt));
	}

	WepEvt(int trigger, int delay=0, int triggerArg=0) {
		memset(this, 0, sizeof(WepEvt));
		this->trigger = trigger;
		this->triggerArg = triggerArg;
		this->delay = delay;
	}

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
		this->trigger = WC_TRIG_PRIMARY_ODD;
		return *this;
	}

	WepEvt PrimaryEven() {
		this->trigger = WC_TRIG_PRIMARY_EVEN;
		return *this;
	}

	WepEvt PrimaryEmpty() {
		this->trigger = WC_TRIG_PRIMARY_CLIPSIZE;
		this->triggerArg = 0;
		return *this;
	}

	WepEvt PrimaryNotEmpty() {
		this->trigger = WC_TRIG_PRIMARY_NOT_EMPTY;
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

	WepEvt PrimaryStop() {
		this->trigger = WC_TRIG_PRIMARY_STOP;
		return *this;
	}

	WepEvt SecondaryStop() {
		this->trigger = WC_TRIG_SECONDARY_STOP;
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
		uint8_t distantSound, uint8_t aiVol) {
		evtType = WC_EVT_PLAY_SOUND;
		playSound.sound = sound;
		playSound.channel = channel;
		playSound.aiVol = aiVol;
		playSound.volume = (int)(volume * 255.5f);
		playSound.attn = clampf(attn * 64, 0, 255.0f);
		playSound.pitchMin = pitchMin;
		playSound.pitchMax = pitchMax;
		playSound.distantSound = distantSound;
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

	WepEvt EjectShell(uint16_t model, float offsetForward, float offsetUp, float offsetRight) {
		evtType = WC_EVT_EJECT_SHELL;
		ejectShell.model = model;
		ejectShell.offsetForward = FLOAT_TO_FP_10_6(offsetForward);
		ejectShell.offsetUp = FLOAT_TO_FP_10_6(offsetUp);
		ejectShell.offsetRight = FLOAT_TO_FP_10_6(offsetRight);
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

	// cost = unused
	WepEvt Bullets(uint8_t count, uint16_t burstDelay, uint16_t damage, float spreadX, float spreadY,
		uint8_t tracerFreq = 1, uint8_t flashSz=WC_FLASH_NORMAL, uint8_t flags=0) {
		evtType = WC_EVT_BULLETS;
		bullets.count = count;
		bullets.burstDelay = burstDelay;
		bullets.damage = damage;
		bullets.spreadX = FLOAT_TO_SPREAD(spreadX);
		bullets.spreadY = FLOAT_TO_SPREAD(spreadY);
		bullets.tracerFreq = tracerFreq;
		bullets.flashSz = flashSz;
		bullets.flags = flags;
		return *this;
	}

	WepEvt BulletFired() {
		trigger = WC_TRIG_BULLET_FIRED;
		return *this;
	}

	// offset = offset from view position given in: right, forward, up
	// speed = initial speed of the projectile
	// dir = Direction of the projectile, relative to the aim direction.
	//       Coordinates are given as Right, Up, and Forward units.
	//       Usually you want to set this to straight forward (0 0 1).
	WepEvt Projectile(WeaponCustomProjectile type, float speed = 800, float spreadX=0, float spreadY=0, Vector offset=Vector(0,16,0), Vector dir = Vector(0, 0, 1)) {
		evtType = WC_EVT_PROJECTILE;
		proj.type = type;
		*(Vector*)proj.offset = offset;
		proj.speed = speed;
		proj.spreadX = spreadX;
		proj.spreadY = spreadY;
		*(Vector*)proj.dir = dir;
		proj.world_event = WC_PROJ_ACT_IMPACT;
		proj.monster_event = WC_PROJ_ACT_IMPACT;
		proj.elasticity = 0.8f;
		proj.size = 0.001f;
		proj.dir[2] = 1.0f;
		return *this;
	}

	WepEvt ProjClass(string_t clazz) {
		proj.entity_class = clazz;
		return* this;
	}

	WepEvt ProjPhysics(float gravity, float elasticity=0.8f, float air_friction=0, float water_friction=0) {
		proj.gravity = gravity;
		proj.elasticity = elasticity;
		proj.air_friction = air_friction;
		proj.water_friction = water_friction;
		return *this;
	}

	WepEvt ProjAvel(Vector avel) {
		*(Vector*)proj.avel = avel;
		proj.hasAvel = true;
		return *this;
	}

	WepEvt ProjModel(uint16_t modelIdx) {
		proj.model = modelIdx;
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

	WepEvt ToggleZoom(uint8_t zoomFov, uint8_t zoomFov2=0) {
		evtType = WC_EVT_TOGGLE_ZOOM;
		zoomToggle.zoomFov = zoomFov;
		zoomToggle.zoomFov2 = zoomFov2;
		return *this;
	}

	WepEvt ToggleLaser() {
		evtType = WC_EVT_TOGGLE_LASER;
		return *this;
	}

	WepEvt Cooldown(uint16_t millis, uint8_t targets) {
		evtType = WC_EVT_COOLDOWN;
		cooldown.millis = millis;
		cooldown.targets = targets;
		return *this;
	}

	WepEvt ToggleAkimbo() {
		evtType = WC_EVT_TOGGLE_AKIMBO;
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
#endif
};

struct CustomWeaponShootOpts {
	uint8_t flags;				// FL_WC_SHOOT_*
	uint8_t ammoCost;			// ammo cost of each attack
	uint8_t ammoFreq;			// skip decrementing ammo for this many attacks (for fractional ammo costs)
	uint8_t ammoPool;			// which ammo pool to drain from (WeaponCustomAmmoPool)
	uint16_t cooldown;			// time between attacks (milliseconds)
	uint16_t cooldownFail;		// cooldown after a failed attack (out of ammo, underwater) (milliseconds)
	uint16_t chargeTime;		// how long the attack button must be held before the attack begins (milliseconds)
	uint16_t chargeCancelTime;	// minimum time before a charge can be cancelled (milliseconds)
	uint16_t chargeMoveSpeedMult; // movement speed multiplier while charging (1-65535) (65535 = 100%) (0 = don't change)
	uint16_t accuracyX;			// horizontal accuracy for crosshair (degrees * 100)
	uint16_t accuracyY;			// vertical accuracy for crosshair (degrees * 100)
};

struct WeaponCustomReload {
	uint8_t anim;
	uint16_t time; // milliseconds
};

struct WeaponCustomIdle {
	uint8_t anim;
	uint8_t weight;	// chance of selection (prefer 0-100 with all idles adding up to 100)
	uint16_t time;	// milliseconds before playing another idle animations
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
