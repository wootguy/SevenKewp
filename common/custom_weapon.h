#pragma once
#include <stdint.h>
#include <EHandle.h>
#include "custom_weapon_events.h"

#define DEGREES_FROM_CONE(cone_value) (2.0f * asinf(cone_value) * 180.0f / (float)M_PI)
#define DEGREES_FROM_SPREAD(spread) (2.0f * asinf(SPREAD_TO_FLOAT(spread)) * 180.0f / (float)M_PI)

#define FLOAT_TO_MOVESPEED_MULT(val) clamp(((val) * 65535.0f), 1, 65535)
#define MOVESPEED_MULT_TO_FLOAT(val) ((val) ? (val) / 65535.0f : 1.0f)

#define MAX_WC_EVENTS 64

#define FL_WC_WEP_HAS_PRIMARY			(1<<0)
#define FL_WC_WEP_HAS_SECONDARY			(1<<1)
#define FL_WC_WEP_HAS_TERTIARY			(1<<2)
#define FL_WC_WEP_HAS_ALT_PRIMARY		(1<<3)	// alternate primary fire toggled by laser or zooming
#define FL_WC_WEP_SHOTGUN_RELOAD		(1<<4)	// start animation + load animation (repeated) + finish animation
#define FL_WC_WEP_UNLINK_COOLDOWNS		(1<<5)	// primary and secondary attacks cooldown independently
#define FL_WC_WEP_AKIMBO				(1<<6)	// weapon has an akimbo mode
#define FL_WC_WEP_LINK_CHARGEUPS		(1<<7)	// primary and secondary chargeup state and events are shared (minigun behavior)
#define FL_WC_WEP_PRIMARY_PRIORITY		(1<<8)	// primary fire has priority over secondary when both attack buttons are pressed
#define FL_WC_WEP_EXCLUSIVE_HOLD		(1<<9)	// weapon must be dropped before switching to other weapons
#define FL_WC_WEP_USE_ONLY				(1<<10)	// weapon is collectable with the use key, not by touching
#define FL_WC_WEP_HAS_LASER				(1<<11)
#define FL_WC_WEP_DYNAMIC_ACCURACY		(1<<12) // crosshair widens with movement and shrinks when crouched
#define FL_WC_WEP_ZOOM_SPR_STRETCH		(1<<13) // zoom crosshair stretches to fit the screen
#define FL_WC_WEP_ZOOM_SPR_ASPECT		(1<<14) // zoom crosshair keeps its aspect ratio when stretched to fit the screen, and borders are filled black
#define FL_WC_WEP_EMPTY_IDLES			(1<<15) // The last half of idles are for when the clip is empty
#define FL_WC_WEP_NO_PREDICTION			(1<<16) // Disable client-side prediction entirely
#define FL_WC_WEP_HIDE_SECONDARY_AMMO	(1<<17) // Hide secondary ammo on HUD
#define FL_WC_WEP_FORCE_ZOOM_SPRITE		(1<<18) // Force use of zoom crosshair sprite when using dynamic crosshairs
#define FL_WC_WEP_HAND_MODELS			(1<<19) // Default model supports alternate hand models (op4/bshift)
#define FL_WC_WEP_ALLOW_HL				(1<<20) // Allow the weapon to be used by vanilla HL clients without prediction

#define FL_WC_SHOOT_UNDERWATER 1
#define FL_WC_SHOOT_NO_ATTACK 2			// don't run standard weapon attack logic (shoot animations, clicking)
#define FL_WC_SHOOT_COOLDOWN_IDLE 4		// cooldown the idle animations even if not attacking
#define FL_WC_SHOOT_NEED_AKIMBO 8		// don't allow attack if not holding the akimbo version of the weapon
#define FL_WC_SHOOT_NEED_FULL_COST 16	// don't allow attack if clip is less than ammo cost
#define FL_WC_SHOOT_NO_AUTOFIRE 32		// one shot per click
#define FL_WC_SHOOT_IS_MELEE 64			// use server-side crowbar attack logic

#define FL_WC_CHARGE_DAMAGE		1		// attack charge progress scales damage events
#define FL_WC_CHARGE_KICKBACK	2		// attack charge progress scales kickback events

enum WeaponCustomAmmoPool
{
	WC_AMMOPOOL_DEFAULT,			// automatically select ammo pool (primary = clip, secondary = secondary)
	WC_AMMOPOOL_PRIMARY_CLIP,		// drain ammo from primary clip
	WC_AMMOPOOL_PRIMARY_RESERVE,	// drain ammo from primary reserve
	WC_AMMOPOOL_SECONDARY_RESERVE,	// drain ammo from secondary reserve
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

struct MeleeOpts {
	int damage;
	int damageBits;
	int range;
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
	uint8_t chargeMode;			// WeaponCustomChargeupMode (4 bits)
	uint8_t chargeAmmoMode;		// WeaponCustomChargeAmmoMode (2 bits)
	uint8_t overchargeMode;		// WeaponCustomOverchargeMode (2 bits)
	uint8_t chargeFlags;		// FL_WC_CHARGE_*
	uint16_t chargeTime;		// how long the attack button must be held before the attack begins (milliseconds)
	uint16_t overchargeTime;	// how long an attack can be charged before triggering an overcharge event and cancelling the chargeup
	uint16_t chargeCancelTime;	// minimum time before a charge can be cancelled (milliseconds)
	uint16_t chargeMoveSpeedMult; // movement speed multiplier while charging (1-65535) (65535 = 100%) (0 = don't change)
	uint16_t accuracy[2];		// horizontal+vertical accuracy for crosshair (degrees * 100)
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
	uint16_t accuracy[2];		// horizontal+vertical accuracy for crosshair (degrees * 100)
};

struct WeaponCustomLaser {
	WeaponCustomIdle idles[4];	// alternate set of idle animations that don't twist the laser
	uint16_t dotSprite;			// sprite used for the end point of the laser
	uint16_t beamSprite;		// sprite used for the beam of the laser
	uint8_t dotSz;				// dot sprite scale*10
	uint8_t beamWidth;
	uint8_t attachment;			// model attachment point for the beam effect (required for beam to display)
};

struct WeaponCustomAmmoInfo {
	string_t type;			// ammo inventory name
	uint16_t maxClip;		// 0 = no reloading
	uint16_t defaultGive;	// ammo loaded given when picked up for the first time. 0 = use maxClip
	string_t dropEnt;		// entity to spawn when dropping ammo
	uint32_t dropAmt;		// amount of ammo to drop
};

struct CustomWeaponParams {
	uint32_t flags; // FL_WC_WEP_*
	uint16_t maxClip; // TODO: Remove in next client update (redundant)
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

	// data for file parsing (not networked)
	WeaponCustomAmmoInfo ammoInfo[2];
	string_t defaultModelV;
	string_t defaultModelP;
	string_t defaultModelW;
	string_t pmodelAkimbo;		// thirdperson model used in akimbo mode
	string_t wmodelAkimbo;		// world model used in akimbo mode
	string_t classname;
	string_t wrongClientWeapon;	// weapon given to players who don't have the correct client to use this one. Do not use aliases here.
	string_t animExt;			// third person player animation set
	string_t animExtZoom;		// animation set used while zoomed
	string_t animExtAkimbo;		// animation set used while in akimbo mode
	string_t hudFolder;			// path to the folder containing the HUD config
	string_t displayName;		// name displayed in text messages describing this weapon
	string_t killFeedIcon;		// icon displayed in the kill feed
	int8_t slot;				// weapon selection bucket
	int8_t slotPosition;		// position in weapon selection buucket (-1 = auto)
	int32_t weight;				// importance for auto weapon selection

	uint8_t numEvents;
	WepEvt events[MAX_WC_EVENTS];
};

struct CustomAmmoParams {
	string_t classname;
	string_t ammoType;			// what type of ammo this is (default name or a custom one)
	string_t ammoTypeHl;		// ammo type given to HL clients
	string_t model;
	string_t pickupSound;
	uint16_t ammoGiven;
	uint16_t maxAmmo; // for custom ammo types
	float hullSizeMin[3];
	float hullSizeMax[3];
};

class CWeaponCustom;

enum PredictionDataSendMode {
	WC_PRED_SEND_INIT,	// initialize prediction data for first pickup
	WC_PRED_SEND_WEP,	// only send weapon prediction data
	WC_PRED_SEND_EVT,	// only send event prediction data
	WC_PRED_SEND_BOTH	// send weapon and event data
};

extern uint32_t g_wcPredDataSent[MAX_WEAPONS]; // bitfields indicating which players received prediction data

// call once when dll loaded
void init_weapon_custom_config_parser();

// call on map change
void clear_weapon_custom_cache();

EXPORT bool UTIL_ParseCustomWeaponConfig(const char* path, CustomWeaponParams& params);

EXPORT bool UTIL_ParseCustomAmmoConfig(const char* path, CustomAmmoParams& params);

// prettyPrint = if true, organizes configurations and event data into groups.
//               This changes event ordering which could affect weapon behaviors.
EXPORT void UTIL_DumpCustomWeaponConfig(const char* path, CustomWeaponParams& params, bool prettyPrint);

EXPORT void UTIL_SendCustomWeaponPredictionData(edict_t* target, CWeaponCustom* wep, PredictionDataSendMode sendMode);

EXPORT bool UTIL_HasCustomWeaponPredictionData(edict_t* target, CWeaponCustom* wep);

// client utils
int UTIL_ReadCustomWeaponPredictionData(const char* pszName, int iSize, void* pbuf);
int UTIL_ReadCustomWeaponPredictionEventData(const char* pszName, int iSize, void* pbuf);

void UTIL_TestConfig(CWeaponCustom* wep); // validate that config file dumper is accurate