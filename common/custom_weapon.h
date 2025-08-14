#pragma once
#include <stdint.h>

#define FLOAT_TO_FP_10_6(val) (clamp((int)(val * 64), INT16_MIN, INT16_MAX))
#define FP_10_6_TO_FLOAT(val) (val / 64.0f)

#define FLOAT_TO_SPREAD(val) (clamp((int)(val * 65535), 0, UINT16_MAX))
#define SPREAD_TO_FLOAT(val) (val / 65535.0f)

#define MAX_WC_EVENTS 64
#define MAX_WC_RANDOM_SELECTION 8

#define FL_WC_WEP_HAS_PRIMARY 1
#define FL_WC_WEP_HAS_SECONDARY 2
#define FL_WC_WEP_HAS_TERTIARY 4
#define FL_WC_WEP_SHOTGUN_RELOAD 8		// start animation + load animation (repeated) + finish animation
#define FL_WC_WEP_UNLINK_COOLDOWNS 16	// primary and secondary attacks cooldown independently
#define FL_WC_WEP_AKIMBO 32				// weapon has an akimbo mode

#define FL_WC_SHOOT_UNDERWATER 1
#define FL_WC_SHOOT_NO_ATTACK 2 // don't run standard weapon attack logic (shoot animations, clicking)
#define FL_WC_SHOOT_NEED_AKIMBO 4 // don't allow attack if not holding the akimbo version of the weapon

#define FL_WC_BULLETS_DYNAMIC_SPREAD 1 // spread widens while moving and tightens while crouching
#define FL_WC_BULLETS_NO_DECAL 2 // don't show gunshot particles and decal at impact point
#define FL_WC_BULLETS_NO_SOUND 4 // don't play texture sound at impact point

#define FL_WC_COOLDOWN_PRIMARY 1
#define FL_WC_COOLDOWN_SECONDARY 2
#define FL_WC_COOLDOWN_IDLE 4

enum WeaponCustomEventTriggerShootArg {
	WC_TRIG_SHOOT_ARG_ALWAYS,		// always fire the shoot event
	WC_TRIG_SHOOT_ARG_AKIMBO,		// only fire the event when in akimbo mode
	WC_TRIG_SHOOT_ARG_NOT_AKIMBO,	// only fire the event when not in akimbo mode
};

enum WeaponCustomEventTriggers {
	WC_TRIG_SHOOT_PRIMARY, // trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_SHOOT_SECONDARY, // trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_SHOOT_TERTIARY,
	WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, // trigger arg is the clip size to trigger on
	WC_TRIG_SHOOT_PRIMARY_ODD, // triggers on odd clip sizes after firing
	WC_TRIG_SHOOT_PRIMARY_EVEN, // triggers on even clip sizes after firing
	WC_TRIG_SHOOT_PRIMARY_NOT_EMPTY, // triggers on non-empty clip size after firing
	WC_TRIG_RELOAD, // triggers when a reload begins. Trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_RELOAD_EMPTY, // triggers when an empty clip reload begins. Trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_RELOAD_NOT_EMPTY, // triggers when a non-empty clip reload begins. Trigger arg: WeaponCustomEventTriggerShootArg
	WC_TRIG_DEPLOY, // Trigger arg : WeaponCustomEventTriggerShootArg
};

enum WeaponCustomEventType {
	WC_EVT_IDLE_SOUND, // simple sound playback for reloads
	WC_EVT_PLAY_SOUND, // advanced sound playback
	WC_EVT_EJECT_SHELL,
	WC_EVT_PUNCH_SET, // set punch angles to the given values
	WC_EVT_PUNCH_RANDOM, // set random punch angle between +/- the given angles
	WC_EVT_SET_BODY,
	WC_EVT_WEP_ANIM,
	WC_EVT_BULLETS,
	WC_EVT_KICKBACK,
	WC_EVT_MUZZLE_FLASH,
	WC_EVT_TOGGLE_ZOOM,
	WC_EVT_COOLDOWN, // adjust cooldowns (by default every action is cooled down after an attack)
	WC_EVT_TOGGLE_AKIMBO, // toggle dual-wield mode
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

struct WepEvt {
	uint32_t evtType : 4;
	uint32_t trigger : 4; // when to trigger the event
	uint32_t triggerArg : 4; // additional args for event triggering
	uint32_t delay : 12; // milliseconds beforing firing the event

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
			uint8_t reserved : 2;
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
			int16_t x; // 10.6 fixed point int
			int16_t y;
			int16_t z;
		} punch;

		struct {
			uint8_t akimbo : 4; // WeaponCustomAnimHand
			uint8_t numAnim : 4;
			uint8_t anims[MAX_WC_RANDOM_SELECTION];
		} anim;

		struct {
			uint8_t count; // how many bullets are shot at once
			uint16_t damage; // damage per bullet
			uint16_t spreadX; // accuracy (0 = perfect, 1 = 180 degrees). 65535 = 1.0f
			uint16_t spreadY; // accuracy (0 = perfect, 1 = 180 degrees). 65535 = 1.0f
			uint8_t tracerFreq; // how often to display a tracer (0 = never, 1 = always, 2 = every other shot)
			uint8_t flashSz : 4; // WeaponCustomFlashSz
			uint8_t flags : 4; // FL_WC_BULLETS_*
		} bullets;

		struct {
			int16_t pushForce; // Push force applied to player in opposite direction of aim
		} kickback;

		struct {
			uint8_t zoomFov;
		} zoomToggle;

		struct {
			uint16_t millis;
			uint8_t targets; // FL_WC_COOLDOWN_*
		} cooldown;
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
		this->trigger = WC_TRIG_SHOOT_PRIMARY;
		return *this;
	}

	WepEvt Secondary() {
		this->trigger = WC_TRIG_SHOOT_SECONDARY;
		return *this;
	}

	WepEvt Tertiary() {
		this->trigger = WC_TRIG_SHOOT_TERTIARY;
		return *this;
	}

	WepEvt PrimaryClip(uint8_t clipSize) {
		this->trigger = WC_TRIG_SHOOT_PRIMARY_CLIPSIZE;
		this->triggerArg = clipSize;
		return *this;
	}

	WepEvt PrimaryOdd() {
		this->trigger = WC_TRIG_SHOOT_PRIMARY_ODD;
		return *this;
	}

	WepEvt PrimaryEven() {
		this->trigger = WC_TRIG_SHOOT_PRIMARY_EVEN;
		return *this;
	}

	WepEvt PrimaryEmpty() {
		this->trigger = WC_TRIG_SHOOT_PRIMARY_CLIPSIZE;
		this->triggerArg = 0;
		return *this;
	}

	WepEvt PrimaryNotEmpty() {
		this->trigger = WC_TRIG_SHOOT_PRIMARY_NOT_EMPTY;
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

	WepEvt Deploy() {
		this->trigger = WC_TRIG_DEPLOY;
		return *this;
	}

	WepEvt Delay(uint16_t millis) {
		this->delay = millis;
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
		evtType = WC_EVT_PUNCH_SET;
		punch.x = FLOAT_TO_FP_10_6(x);
		punch.y = FLOAT_TO_FP_10_6(y);
		punch.z = FLOAT_TO_FP_10_6(z);
		return *this;
	}

	// prefer adding this AFTER a bullets event for prediction accuracy
	WepEvt PunchRandom(float x, float y, float z=0) {
		evtType = WC_EVT_PUNCH_RANDOM;
		punch.x = FLOAT_TO_FP_10_6(x);
		punch.y = FLOAT_TO_FP_10_6(y);
		punch.z = FLOAT_TO_FP_10_6(z);
		return *this;
	}

	WepEvt WepAnim(uint8_t animIdx, uint8_t akimbo=WC_ANIM_BOTH_HANDS) {
		evtType = WC_EVT_WEP_ANIM;
		anim.anims[0] = animIdx;
		anim.numAnim = 1;
		anim.akimbo = akimbo;
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

	WepEvt Bullets(uint8_t count, uint16_t damage, float spreadX, float spreadY,
		uint8_t tracerFreq = 1, uint8_t flasSz=WC_FLASH_NORMAL, uint8_t flags=0) {
		evtType = WC_EVT_BULLETS;
		bullets.count = count;
		bullets.damage = damage;
		bullets.spreadX = FLOAT_TO_SPREAD(spreadX);
		bullets.spreadY = FLOAT_TO_SPREAD(spreadY);
		bullets.tracerFreq = tracerFreq;
		bullets.flags = flags;
		return *this;
	}

	WepEvt Kickback(int16_t pushForce) {
		evtType = WC_EVT_KICKBACK;
		kickback.pushForce = pushForce;
		return *this;
	}

	WepEvt ToggleZoom(uint8_t zoomFov) {
		evtType = WC_EVT_TOGGLE_ZOOM;
		zoomToggle.zoomFov = zoomFov;
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
#endif
};

struct CustomWeaponShootOpts {
	uint8_t flags; // FL_WC_SHOOT_*
	uint8_t ammoCost;
	uint16_t cooldown; // milliseconds
};

struct WeaponCustomReload {
	uint8_t anim;
	uint16_t time; // milliseconds
};

struct WeaponCustomIdle {
	uint8_t anim;
	uint8_t weight; // chance of selection (prefer 0-100 with all idles adding up to 100)
	uint16_t time; // milliseconds before playing another idle animations
};

struct WeaponCustomAkimbo {
	WeaponCustomIdle idles[4];
	WeaponCustomReload reload;
	uint8_t deployAnim; // deploy anim for a single weapon (used for reloading and toggling akimbo mode)
	uint16_t deployTime;
	uint8_t akimboDeployAnim; // deploy anim when selecting the weapon
	uint16_t akimboDeployTime;
	uint8_t holsterAnim; // for reloading a single weapon
	uint16_t holsterTime;
};

struct CustomWeaponParams {
	uint8_t flags; // FL_WC_WEP_*
	uint16_t maxClip;
	uint16_t vmodel;
	uint8_t deployAnim;
	uint16_t deployTime;

	// stage 0 and 1 usage depends on weapon flags:
	// 0 = simple reload animation OR starting animation for shotgun reload mode
	// 1 = simple reload animation (empty clip) OR shotgun reload middle animation (shell insertion)
	// 2 = shotgun reload finish animation (cocking)
	WeaponCustomReload reloadStage[3];

	WeaponCustomIdle idles[4]; // randomly selected idle animations

	CustomWeaponShootOpts shootOpts[3]; // primary, secondary, and tertiary fire

	WeaponCustomAkimbo akimbo;
	
	uint8_t numEvents;
	WepEvt events[MAX_WC_EVENTS];
};
