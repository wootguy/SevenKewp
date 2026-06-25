#pragma once
#include <stdint.h>
#include <EHandle.h>
#include "wc_events.h"
#include "string_deltas.h"

#define DEGREES_FROM_CONE(cone_value) (2.0f * asinf(cone_value) * 180.0f / (float)M_PI)
#define DEGREES_FROM_SPREAD(spread) (2.0f * asinf(D100_TO_FLOAT(spread)) * 180.0f / (float)M_PI)

#define MAX_WC_EVENTS 64

#define FL_WC_WEP_HAS_PRIMARY			(1<<0)
#define FL_WC_WEP_HAS_SECONDARY			(1<<1)
#define FL_WC_WEP_HAS_TERTIARY			(1<<2)
#define FL_WC_WEP_HAS_ALT_PRIMARY		(1<<3)	// alternate primary fire toggled by laser or zooming
#define FL_WC_WEP_SHOTGUN_RELOAD		(1<<4)	// start animation + load animation (repeated) + finish animation
#define FL_WC_WEP_SHOTGUN_SMOOTH_CANCEL	(1<<5)	// interrupting a shotgun reload plays the finish animation instead of shooting immediately like the HL shotgun
#define FL_WC_WEP_UNLINK_COOLDOWNS		(1<<6)	// primary and secondary attacks cooldown independently
#define FL_WC_WEP_AKIMBO				(1<<7)	// weapon has an akimbo mode
#define FL_WC_WEP_LINK_CHARGEUPS		(1<<8)	// primary and secondary chargeup state and events are shared (minigun behavior)
#define FL_WC_WEP_PRIMARY_PRIORITY		(1<<10)	// primary fire has priority over secondary when both attack buttons are pressed
#define FL_WC_WEP_EXCLUSIVE_HOLD		(1<<11)	// weapon must be dropped before switching to other weapons
#define FL_WC_WEP_USE_ONLY				(1<<12)	// weapon is collected with the use key, not by touching
#define FL_WC_WEP_HAS_LASER				(1<<13)
#define FL_WC_WEP_DYNAMIC_ACCURACY		(1<<14) // crosshair widens with movement and shrinks when crouched
#define FL_WC_WEP_ZOOM_SPR_STRETCH		(1<<15) // zoom crosshair stretches to fit the screen
#define FL_WC_WEP_ZOOM_SPR_ASPECT		(1<<16) // zoom crosshair keeps its aspect ratio when stretched to fit the screen, and borders are filled black
#define FL_WC_WEP_NO_PREDICTION			(1<<17) // Disable client-side prediction entirely
#define FL_WC_WEP_HIDE_SECONDARY_AMMO	(1<<18) // Hide secondary ammo on HUD
#define FL_WC_WEP_FORCE_ZOOM_SPRITE		(1<<19) // Force use of zoom crosshair sprite when using dynamic crosshairs
#define FL_WC_WEP_HAND_MODELS			(1<<20) // Default model supports alternate hand models (op4/bshift)
#define FL_WC_WEP_ALLOW_HL				(1<<21) // Allow the weapon to be used by vanilla HL clients without prediction
#define FL_WC_WEP_NO_AUTOSWITCHEMPTY	(1<<22) // Don't switch to another weapon when out of ammo
#define FL_WC_WEP_NO_AUTORELOAD			(1<<23) // Don't reload the weapon automatically
#define FL_WC_WEP_SELECTONEMPTY			(1<<24) // allow selecting the weapon when empty
#define FL_WC_WEP_EXHAUSITBLE			(1<<25) // Remove the weapon when out of ammo
#define FL_WC_WEP_IRON_SIGHTS_ZOOM		(1<<26) // zooming uses iron sights
#define FL_WC_WEP_HAS_STATE_SPRITE		(1<<27) // weapon has a HUD sprite indicating weapon state
#define FL_WC_WEP_HAS_ALT_PARAMS		(1<<28) // weapon has an alternate set of weapon params that can be toggled
#define FL_WC_WEP_RELOAD2_IS_DEFAULT	(1<<29) // reload the secondary clip by default
#define FL_WC_WEP_HAS_E_R_TOGGLE		(1<<30) // state is toggled by pressing +use and +reload together

#define FL_WC_SHOOT_UNDERWATER 1
#define FL_WC_SHOOT_NO_ATTACK 2			// don't run standard weapon attack logic (shoot animations, clicking)
#define FL_WC_SHOOT_COOLDOWN_IDLE 4		// cooldown the idle animations even if not attacking
#define FL_WC_SHOOT_NEED_AKIMBO 8		// don't allow attack if not holding the akimbo version of the weapon
#define FL_WC_SHOOT_NEED_FULL_COST 16	// don't allow attack if clip is less than ammo cost
#define FL_WC_SHOOT_NO_AUTOFIRE 32		// one shot per click
#define FL_WC_SHOOT_IS_MELEE 64			// use server-side crowbar attack logic
#define FL_WC_SHOOT_FIRST_SHOT_ACCURACY 128	// use first shot accuracy values for first attack

#define FL_WC_CHARGE_DAMAGE		1		// attack charge progress scales damage events
#define FL_WC_CHARGE_KICKBACK	2		// attack charge progress scales kickback events

enum WeaponCustomAmmoPool
{
	WC_AMMOPOOL_DEFAULT,			// automatically select ammo pool (primary = clip, secondary = secondary)
	WC_AMMOPOOL_PRIMARY_CLIP,		// drain ammo from primary clip
	WC_AMMOPOOL_PRIMARY_RESERVE,	// drain ammo from primary reserve
	WC_AMMOPOOL_SECONDARY_CLIP,		// drain ammo from secondary clip
	WC_AMMOPOOL_SECONDARY_RESERVE,	// drain ammo from secondary reserve
};

enum WeaponCustomChargeupMode {
	WC_CHARGEUP_NONE,			// disable attack charging
	WC_CHARGEUP_CONSTANT,		// fire constantly, optionally while charging up
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

enum WeaponAccuracyMultType {
	// movement mulitpliers (only 1 can be active)
	WC_ACCURACY_MULT_DUCK,	// accuracy multiplier for ducking
	WC_ACCURACY_MULT_CRAWL,	// accuracy multiplier for crawling
	WC_ACCURACY_MULT_WALK,	// accuracy multiplier for walking
	WC_ACCURACY_MULT_RUN,	// accuracy multiplier for running
	WC_ACCURACY_MULT_FLY,	// accuracy multiplier for jumping
	WC_ACCURACY_MULT_FLOAT,	// accuracy multiplier for treading water
	WC_ACCURACY_MULT_SWIM,	// accuracy multiplier for swimming

	// the following types are applied in addition to a movement multiplier
	WC_ACCURACY_MULT_ZOOM,	// accuracy multiplier while zoomed
	WC_ACCURACY_MULT_TYPES,
};

enum WeaponCustomToggleStateMode {
	WC_TOGGLE_STATE_OFF,
	WC_TOGGLE_STATE_ON,
	WC_TOGGLE_STATE_TOGGLE,
};

enum WeaponCustomHasCooldownIndex {
	WC_COOLDOWN_PRIMARY,	// override cooldown for primary fire
	WC_COOLDOWN_SECONDARY,	// override cooldown for secondary fire
	WC_COOLDOWN_TERTIARY,	// override cooldown for tertiary fire
	WC_COOLDOWN_IDLE,		// override cooldown for idle
	WC_COOLDOWN_FAIL,		// cooldown after a failed attack (out of ammo, underwater)
	WC_COOLDOWN_FINISH,		// cooldown after finishing an attack
	WC_COOLDOWN_WATER,		// cooldown while in water (0 = use default cooldown)
	WC_COOLDOWN_TYPES,
};

enum WeaponCustomReloadState {
	WC_RELOAD_STAGE_START,			// start of a simple or shotgun reload
	WC_RELOAD_STAGE_START_EMPTY,	// start of a simple or shotgun reload (empty clip)
	WC_RELOAD_STAGE_SHELL,			// shotgun shell insert
	WC_RELOAD_STAGE_PUMP,			// finish shotgun reload
	WC_RELOAD_STAGE_SECONDARY,		// start of a secondary reload
	WC_RELOAD_STAGE_AKIMBO,			// start of an akimbo reload. Triggers for each hand individually.
	WC_RELOAD_STAGES,				// Note: m_fInSpecialReload is 3 bits in delta.lst
};

enum WeaponCustomAmmoIndex {
	WC_AMMO_PRIMARY,
	WC_AMMO_SECONDARY,
	WC_AMMO_TYPES
};

enum WeaponCustomAttackIndex {
	WC_ATTACK_PRIMARY,
	WC_ATTACK_SECONDARY,
	WC_ATTACK_TERTIARY,
	WC_ATTACK_PRIMARY_ALT,
	WC_ATTACK_TYPES,
};

enum WeaponCustomParamsIndex {
	WC_PARAMS_AUTO,
	WC_PARAMS_DEFAULT,
	WC_PARAMS_ALTERNATE,
	WC_PARAMS_TYPES,
};

#pragma pack(push,1)

struct MeleeOpts {
	int damage;
	int damageBits;
	int range;
	int32_t attackOffset[3];// in forward, up, right units
	uint16_t missCooldown;	// cooldown millis for missing an attack
	uint16_t hitCooldown;	// cooldown millis for hitting something
	uint16_t decalDelay;	// how long to wait after hitting a wall to apply the decal
	uint16_t hitWallSounds[4];
	uint16_t hitFleshSounds[4];
	uint16_t missSounds[4];
};

// options for toggling weapon state
struct WeaponCustomToggle {
	uint8_t hasToggleInfo;	// 1 bit condition for networking
	uint8_t hasZoomInfo;	// 1 bit condition for networking

	uint16_t stateBits;		// FL_WC_STATE_*
	uint8_t mode;			// WeaponCustomToggleStateMode
	uint16_t onDelay;		// time before toggling states on
	uint16_t offDelay;		// time before toggling states off
	uint8_t zoomLevels;		// 2 bits - maximum levels of zoom
	uint8_t zoomFov[3];		// zoom fov for each level
};

// Configure predicted control logic here, not in events. If an effect changes which code paths
// are executed (alters predicted state variables like m_flNextAttack), then the effect belongs here.
struct CustomWeaponShootOpts {
	uint8_t flags;				// FL_WC_SHOOT_*
	uint8_t ammoCost;			// ammo cost of each attack
	uint8_t ammoFreq;			// skip decrementing ammo for this many attacks (for fractional ammo costs)
	uint8_t ammoPool;			// which ammo pool to drain from (WeaponCustomAmmoPool)
	uint16_t emptySound;		// custom empty click sound
	uint16_t cooldown;			// time between attacks (milliseconds)
	uint16_t accuracy[2];		// horizontal+vertical accuracy for crosshair (degrees * 100)
	uint16_t accuracyFirst[2];  // accuracy for the first shot fired
	uint16_t chamberTime;		// if set, chambering is required if the weapon is switched before waiting this long after the attack

	uint8_t hasCooldownOverride[WC_COOLDOWN_TYPES];		// 1 bit conditions for networking
	uint16_t cooldownOverride[WC_COOLDOWN_TYPES];	// cooldown overrides for various actions

	uint8_t chargeMode;			// 4 bits - WeaponCustomChargeupMode
	uint8_t chargeAmmoMode;		// 2 bits - WeaponCustomChargeAmmoMode
	uint8_t overchargeMode;		// 2 bits - WeaponCustomOverchargeMode
	uint8_t chargeFlags;		// FL_WC_CHARGE_*
	uint16_t chargeTime;		// how long the attack button must be held before the attack begins (milliseconds)
	uint16_t chargeDownTime;	// how long before the charge returns to 0 after releasing the attack button
	uint16_t minChargeShootTime;// minimum charge time needed before shooting in constant mode
	uint16_t overchargeTime;	// how long an attack can be charged before triggering an overcharge event and cancelling the chargeup
	uint16_t dischargedCooldown;// cooldown when beginning a charge, increasing to 'cooldown' at full charge
	uint16_t chargeCancelTime;	// minimum time before a charge can be cancelled (milliseconds)
	uint16_t chargeMoveSpeedMult; // movement speed multiplier while charging (0 = don't change)
	
	uint8_t hasAccMult[WC_ACCURACY_MULT_TYPES];		// 1 bit conditions for networking
	uint16_t accuracyMult[WC_ACCURACY_MULT_TYPES];	// accuracy multipliers for player movement

	WeaponCustomToggle toggle;

	// server side settings (not networked)
	MeleeOpts melee;
};

struct WeaponCustomReload {
	uint8_t anim;
	uint16_t time; // milliseconds

	// clip is loaded at this time instead of when the stage is complete.
	// A bullet will be loaded for stages that don't normally load the clip.
	uint16_t loadTime;

	// if set, the gun will require chambering if the reload is aborted after the loadTime
	// and before the chamberTime
	uint16_t chamberTime;
};

struct WeaponCustomAkimbo {
	uint8_t deployAnim;			// deploy anim for a single weapon (used for reloading and toggling akimbo mode)
	uint16_t deployTime;
	uint8_t holsterAnim;		// for reloading a single weapon
	uint16_t holsterTime;
	uint16_t accuracy[2];		// horizontal+vertical accuracy for crosshair (degrees * 100)
};

struct WeaponCustomLaser {
	uint16_t dotSprite;			// sprite used for the end point of the laser
	uint16_t beamSprite;		// sprite used for the beam of the laser
	uint8_t dotSz;				// dot sprite scale*10
	uint8_t beamWidth;
	uint8_t attachment;			// model attachment point for the beam effect (required for beam to display)
};

struct WeaponCustomAmmoInfo {
	string_t config;		// path to an ammo config for loading
	string_t type;			// ammo inventory name
	uint16_t maxClip;		// 0 = no reloading
	uint16_t defaultGive;	// ammo loaded when picked up for the first time. 0 = use maxClip (unless this is secondary ammo, in which case 0 = 0 starting clip)
	string_t dropEnt;		// entity to spawn when dropping ammo
	uint32_t dropAmt;		// amount of ammo to drop
};

// Icon indicating weapon state, shown above ammo (HL25 sprites in the hud .txt file)
struct WeaponCustomStateIcon {
	// server-side names for looking up sprite indexes
	string_t defaultIcon;		// icon to display by default
	string_t semiAutoIcon;		// icon to display for semi auto mode
	string_t primaryAltIcon;	// icon to display for alternate fire

	// 4bit indexes to select into the weapon's custom icon list
	uint8_t defaultIconIdx;
	uint8_t semiAutoIconIdx;
	uint8_t primaryAltIconIdx;
};

// an idea for transferring hud icons as network messages instead of as a .txt file. Not used yet.
struct HudIconItem {
	// delta strings which will be queued first during map load so 256 indexes is enough.
	// Also only need to write these once per group. Can send 32 lines in <180 bytes that way.
	// Still kind of a lot if you spawned with 30+ custom weapons.
	uint8_t name;
	uint8_t spritePath;

	uint8_t x;
	uint8_t y;
	uint8_t w;
	uint8_t h;

	uint8_t hi_coords : 4;
	uint8_t resolution : 4;
};

// weapon state
struct ViewModelSprite {
	int hSprite;
	int loadFailed;
	int frame;
	int x, y;
	int w, h;
	float scale; // sprite scale
	WepEvtArr8 anim;
	WepEvtArr16 animOfsX;
	WepEvtArr16 animOfsY;
	uint16_t fps;
	uint32_t animTime;
	float bobTime; // view bobbing progress
	float moveScale; // bobbing intensity
	float lastUpdate;
	RGBA color;
	int rendermode;
	int renderfx;
	RGB glowShellColor;
	float brighten; // increase light level temporarily
};

struct CustomWeaponParams {
	uint32_t flags; // FL_WC_WEP_*
	uint16_t vmodel;
	uint16_t vmodel_zoom;		// view model displayed while zoomed in
	uint16_t moveSpeedMult;		// move speed multiplier (0 = don't change)
	uint16_t zoomMoveSpeedMult; // movement speed multiplier while zoomed (0 = don't change)
	int jumpPower;				// -1 = disabled, 0 = default velocity (800), 1+ = custom velocity

	WeaponCustomReload reloadStage[WC_RELOAD_STAGES];

	CustomWeaponShootOpts shootOpts[WC_ATTACK_TYPES];
	WeaponCustomAkimbo akimbo;
	WeaponCustomLaser laser;
	WeaponCustomStateIcon stateIcon;

	WeaponCustomToggle erToggle; // toggle action for pressing +use and +reload keys together
	uint16_t erToggleCooldown;

	dstring_t vsprite_path; // path a to a sprite that only the client needs to load
	uint16_t vsprite_base_scale; // base scaling applied to view HUD sprite. HL25 scaling applied in addition
	int8_t vsprite_offset_x;
	int8_t vsprite_offset_y;

	// data for file parsing (not networked)
	WeaponCustomAmmoInfo ammoInfo[2];
	string_t defaultModelV;
	string_t defaultSpriteV;		// for doom style weapon sprites
	string_t defaultModelV_zoom;
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
	string_t killFeedIcon;		// icon displayed in the kill feed (classname of a stock weapon)
	string_t altParams;			// path to an alternate set of parameters
	int8_t slot;				// weapon selection bucket
	int8_t slotPosition;		// position in weapon selection buucket (-1 = auto)
	int32_t weight;				// importance for auto weapon selection
	uint8_t wsprite_frame;		// frame to display when the dropped weapon model is a sprite
	int32_t hull_min[3];		// min hull size for droppd weapon
	int32_t hull_max[3];		// max hull size for droppd weapon

	uint8_t numEvents;
	WepEvt events[MAX_WC_EVENTS];
};

struct CustomAmmoParams {
	string_t classname;
	string_t ammoType;			// what type of ammo this is (default name or a custom one)
	string_t ammoTypeHl;		// ammo type given to HL clients
	string_t model;
	uint16_t modelBody;			// body group to use in model
	string_t pickupSound;
	uint16_t ammoGiven;
	uint16_t maxAmmo; // for custom ammo types
	int32_t hullSizeMin[3];
	int32_t hullSizeMax[3];
};

#pragma pack(pop)
