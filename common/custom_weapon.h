#pragma once
#include <stdint.h>

#define FLOAT_TO_FP_10_6(val) (clamp((int)(val * 64), INT16_MIN, INT16_MAX))
#define FP_10_6_TO_FLOAT(val) (val / 64.0f)

#define FLOAT_TO_SPREAD(val) (clamp((int)(val * 65535), 0, UINT16_MAX))
#define SPREAD_TO_FLOAT(val) (val / 65535.0f)

#define MAX_CUSTOM_WEAPON_EVENTS 64

#define FL_WC_WEP_HAS_PRIMARY 1
#define FL_WC_WEP_HAS_SECONDARY 2
#define FL_WC_WEP_SHOTGUN_RELOAD 4 // start animation + load animation (repeated) + finish animation
#define FL_WC_WEP_UNLINK_COOLDOWNS 8 // primary and secondary attacks cooldown independently

#define FL_WC_SHOOT_UNDERWATER 1
#define FL_WC_SHOOT_NO_ATTACK 2 // don't run standard weapon attack logic (shoot animations, clicking)

#define FL_WC_BULLETS_DYNAMIC_SPREAD 1 // spread widens while moving and tightens while crouching

#define FL_WC_COOLDOWN_PRIMARY 1
#define FL_WC_COOLDOWN_SECONDARY 2
#define FL_WC_COOLDOWN_IDLE 4

enum WeaponCustomEventTriggers {
	WC_TRIG_SHOOT_PRIMARY,
	WC_TRIG_SHOOT_SECONDARY,
	WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, // trigger arg is the clip size to trigger on
	WC_TRIG_SHOOT_PRIMARY_ODD, // triggers on odd clip sizes after firing
	WC_TRIG_SHOOT_PRIMARY_EVEN, // triggers on even clip sizes after firing
	WC_TRIG_SHOOT_PRIMARY_NOT_EMPTY, // triggers on non-empty clip size after firing
	WC_TRIG_RELOAD, // triggers when a reload begins
	WC_TRIG_RELOAD_EMPTY, // triggers when an empty clip reload begins
	WC_TRIG_RELOAD_NOT_EMPTY, // triggers when a non-empty clip reload begins
	WC_TRIG_DEPLOY,
};

enum WeaponCustomEventType {
	WC_EVT_PLAY_SOUND,
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

struct WepEvt {
	uint32_t evtType : 4;
	uint32_t trigger : 4; // when to trigger the event
	uint32_t triggerArg : 10; // additional args for event triggering
	uint32_t delay : 14; // milliseconds beforing firing the event

	// event arguments
	union {
		struct {
			uint16_t sound : 9;
			uint16_t channel : 3;
			uint16_t aiVol : 2; // WeaponCustomAiVol
			uint8_t reserved : 2;
			uint8_t volume;
			uint8_t attn;
			uint8_t pitchMin;
			uint8_t pitchMax;

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
			uint8_t animMin;
			uint8_t animMax;
		} anim;

		struct {
			uint8_t count; // how many bullets are shot at once
			uint16_t damage; // damage per pellet
			uint16_t spreadX; // accuracy (0 = perfect, 1 = 180 degrees). 65535 = 1.0f
			uint16_t spreadY; // accuracy (0 = perfect, 1 = 180 degrees). 65535 = 1.0f
			uint8_t btype; // BULLET_PLAYER_* (affects impact sounds and decals)
			uint8_t tracerFreq; // how often to display a tracer (0 = never, 1 = always, 2 = every other shot)
			uint8_t flashSz : 4; // WeaponCustomFlashSz
			uint8_t flags : 4; // FL_WC_BULLETS_*
		} bullets;

		struct {
			int16_t pushForce; // Push force applied to player in opposite direction of aim
		} kickback;

		struct {
			uint8_t zoomFov; // Push force applied to player in opposite direction of aim
		} zoomToggle;

		struct {
			uint16_t millis;
			uint8_t targets; // FL_WC_COOLDOWN_*
		} cooldown;
	};

#ifndef CLIENT_DLL
	WepEvt() {}

	WepEvt(int trigger, int delay=0, int triggerArg=0) {
		memset(this, 0, sizeof(WepEvt));
		this->trigger = trigger;
		this->triggerArg = triggerArg;
		this->delay = delay;
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

	WepEvt PunchSet(float x, float y, float z = 0) {
		evtType = WC_EVT_PUNCH_SET;
		punch.x = FLOAT_TO_FP_10_6(x);
		punch.y = FLOAT_TO_FP_10_6(y);
		punch.z = FLOAT_TO_FP_10_6(z);
		return *this;
	}

	WepEvt PunchRandom(float x, float y, float z=0) {
		evtType = WC_EVT_PUNCH_RANDOM;
		punch.x = FLOAT_TO_FP_10_6(x);
		punch.y = FLOAT_TO_FP_10_6(y);
		punch.z = FLOAT_TO_FP_10_6(z);
		return *this;
	}

	WepEvt WepAnim(uint8_t animIdx) {
		evtType = WC_EVT_WEP_ANIM;
		anim.animMin = animIdx;
		anim.animMax = animIdx;
		return *this;
	}

	WepEvt WepAnim(uint8_t animMin, uint8_t animMax) {
		evtType = WC_EVT_WEP_ANIM;
		anim.animMin = animMin;
		anim.animMax = animMax;
		return *this;
	}

	WepEvt Bullets(uint8_t count, uint16_t damage, float spreadX, float spreadY, uint8_t bulletType,
		uint8_t tracerFreq = 1, uint8_t flasSz=WC_FLASH_NORMAL, uint8_t flags=0) {
		evtType = WC_EVT_BULLETS;
		bullets.count = count;
		bullets.damage = damage;
		bullets.spreadX = FLOAT_TO_SPREAD(spreadX);
		bullets.spreadY = FLOAT_TO_SPREAD(spreadY);
		bullets.btype = bulletType;
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

struct CustomWeaponParams {
	uint8_t flags; // FL_WC_WEP_*
	uint16_t maxClip;
	uint16_t vmodel;
	uint16_t deployAnim;
	uint16_t deployTime;

	// stage 0 and 1 usage depends on weapon flags:
	// 0 = simple reload animation OR starting animation for shotgun reload mode
	// 1 = simple reload animation (empty clip) OR shotgun reload middle animation (shell insertion)
	// 2 = shotgun reload finish animation (cocking)
	WeaponCustomReload reloadStage[3];

	WeaponCustomIdle idles[4]; // randomly selected idle animations

	CustomWeaponShootOpts shootOpts[2]; // primary and secondary fire
	
	uint8_t numEvents;
	WepEvt events[MAX_CUSTOM_WEAPON_EVENTS];
};
