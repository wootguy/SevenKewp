#include "custom_weapon.h"
#include "CWeaponCustom.h"
#include "StringPool.h"

#ifdef CLIENT_DLL
#include "../cl_dll/hud.h"
#define INDEX_SOUND(...) NULL
#define PRECACHE_SOUND_NULLENT(...) 0
#define PRECACHE_MODEL_NULLENT(...) 0
#define INDEX_MODEL(...) NULL
#define get_decal_name(...) NULL
RGB UTIL_ParseRGB(const char* pString) { return RGB(); }
RGBA UTIL_ParseRGBA(const char* pString) { return RGBA(); }
Vector UTIL_ParseVector(const char* pString) { return Vector(); }
#include "parsemsg.h"
void InitCustomWeapon(int id);
CustomWeaponParams* GetCustomWeaponParams(int id);
#endif

#define WEP_FIELD(name, default_val, struct_field, bits, field_type, ...) \
	{ name, default_val, offsetof(CustomWeaponParams, struct_field), bits, field_type, ##__VA_ARGS__}
#define WEP__ENUM(name, default_val, struct_field, bits, enum_arr, ...) \
	{ name, default_val, offsetof(CustomWeaponParams, struct_field), bits, WC_PARAM_UINT8_ENUM, enum_arr, ARRAY_SZ(enum_arr), ##__VA_ARGS__}
#define WEP_FLAGS(name, default_val, struct_field, bits, flags_arr, ...) \
	{ name, default_val, offsetof(CustomWeaponParams, struct_field), bits, WC_PARAM_UINT8_FLAGS, flags_arr, ARRAY_SZ(flags_arr), ##__VA_ARGS__}
#define WEP_FLAGS32(name, default_val, struct_field, bits, flags_arr, ...) \
	{ name, default_val, offsetof(CustomWeaponParams, struct_field), bits, WC_PARAM_UINT32_FLAGS, flags_arr, ARRAY_SZ(flags_arr), ##__VA_ARGS__}
#define WEP_COND_BYTE(field) offsetof(CustomWeaponParams, field)

#define EVT_FIELD(name, default_val, struct_field, bits, field_type, ...) \
	{ name, default_val, offsetof(WepEvt, struct_field), bits, field_type, ##__VA_ARGS__}
#define EVT__ENUM(name, default_val, struct_field, bits, enum_arr, ...) \
	{ name, default_val, offsetof(WepEvt, struct_field), bits, WC_PARAM_UINT8_ENUM, enum_arr, ARRAY_SZ(enum_arr), ##__VA_ARGS__}
#define EVT_FLAGS(name, default_val, struct_field, bits, flags_arr, ...) \
	{ name, default_val, offsetof(WepEvt, struct_field), bits, WC_PARAM_UINT8_FLAGS, flags_arr, ARRAY_SZ(flags_arr), ##__VA_ARGS__}
#define EVT_COND_BYTE(field) offsetof(WepEvt, field)

#define EVT_DESC(id, cfgName, ...) do { \
        static wep_field_desc_t fields[] = { __VA_ARGS__ }; \
        g_wc_desc_evt[id] = { cfgName, fields, ARRAY_SZ(fields) }; \
    } while (0)

#define WEP_STRUCT_DESC(var, cfgName, ...) do { \
        static wep_field_desc_t fields[] = { __VA_ARGS__ }; \
        var = { cfgName, fields, ARRAY_SZ(fields) }; \
    } while (0)

#define MAX_NET_MESSAGE_SIZE 192

using namespace std;

enum WC_PARAM_TYPE {
	WC_PARAM_UINT8,
	WC_PARAM_UINT8_PERCENT, // percentage stored as a uint8_t
	WC_PARAM_7BIT_PERCENT,	// percentage stored as a uint8_t, using only 7 bits
	WC_PARAM_UINT8_FP_2_6,	// float value stored as a fixed point integer (6 bits for decimal)
	WC_PARAM_UINT8_FLAGS,	// flags stored in an 8-bit int, written to config on a single line
	WC_PARAM_UINT8_ENUM,	// enum value stored in an 8-bit int
	WC_PARAM_UINT8_ARRAY_8,	// array of up to 8 uint8_t
	WC_PARAM_UINT16,
	WC_PARAM_UINT16_PERCENT, // percentage stored as a uint16_t
	WC_PARAM_UINT32,
	WC_PARAM_UINT32_FLAGS,
	WC_PARAM_INT8,
	WC_PARAM_INT16,
	WC_PARAM_INT32,
	WC_PARAM_RGB,
	WC_PARAM_RGBA,
	WC_PARAM_FLOAT,
	WC_PARAM_VECTOR,
	WC_PARAM_VECTOR_INT8,			// vector as an array of int8_t. No decimals.
	WC_PARAM_VECTOR_FP_10_6,		// vector as an array of 10.6 fixed point ints.
	WC_PARAM_SOUND_INDEX,			// sound file path stored as an index
	WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2,// array of up to 8 sound indexes, one sound per config line, with key naming starting at index 2
	WC_PARAM_MODEL_INDEX,			// model file path stored as an index
	WC_PARAM_DECAL_INDEX,			// texture name stored as a decal index
	WC_PARAM_TIME,					// time value stored as uint16_t
	WC_PARAM_ACCURACY_UINT16_2X,	// degrees of accuracy (0-1 = 0-180) scaled to uint16_t (X + Y)
	WC_PARAM_ACCURACY_100_2X,		// degrees of accuracy (0-1 = 0-180) scaled to uint16_t (X + Y)
	WC_PARAM_STRING,
};

enum event_category {
	WC_EVT_CATEGORY_PRIMARY,
	WC_EVT_CATEGORY_PRIMARY_ALT,
	WC_EVT_CATEGORY_SECONDARY,
	WC_EVT_CATEGORY_TERTIARY,
	WC_EVT_CATEGORY_RELOAD,
	WC_EVT_CATEGORY_DEPLOY,
	WC_EVT_CATEGORY_REACTION,
	WC_EVT_CATEGORY_STATE_CHANGE,
	WC_EVT_CATEGORY_UNKNOWN,
	WC_EVT_CATEGORY_TOTAL,
};

#define FL_FIELD_NO_NETWORK 1		// field is not networked to clients
#define FL_FIELD_NO_CFG 2			// field is calculated automatically and not saved to a config
#define FL_FIELD_ALWAYS_WRITE_CFG 4	// always write this field to the CFG, even if using the default value

struct WeaponConfigCache {
	CustomWeaponParams params;
	uint64_t fileModifiedTime;
};

HashMap<WeaponConfigCache> g_customWeaponCache;

void clear_weapon_custom_cache() {
	g_customWeaponCache.clear();
}

struct SettingsGroup {
	string name;
	StringMap keys;
	int lineno;
};

struct wep_field_desc_t {
	const char* name;			// name for config file
	const char* defaultValue;	// initialized value
	uint16_t offset;			// offset in event struct
	uint16_t bits;				// 0 = default for data type
	WC_PARAM_TYPE type;			// how to parse this field
	const char** valNames;		// for flag data type: list of flag names for each bit
								// for enum data type: list of enum value names
	int valNamesSz;
	uint8_t flags;				// FL_FIELD_*
	uint16_t conditionByteOfs;	// field is not networked if the given byte is 0 (given as offset in struct)
};

struct wep_struct_desc_t {
	const char* name;			// for config file
	wep_field_desc_t* fields;
	int numFields;
};

uint32_t g_wcPredDataSent[MAX_WEAPONS];

const char* g_wc_evt_trigger_names[32];
const char* g_wc_evt_trigger_arg_primary_names[32];
const char* g_wc_evt_trigger_clip_sp_names[32];
const char* g_wc_evt_trigger_impact_names[32];
const char* g_wc_evt_type_names[32];
const char* g_wc_evt_category_names[32];

HashMap<uint16_t> g_wc_name_to_trigger; // maps a group name to an event trigger + argument value
HashMap<uint8_t> g_wc_name_to_action; // maps an action key value to its event number
mod_string_t g_wc_trigger_to_name[32 * 32]; // 32 trigger/arg possibilities
StringPool g_wc_trigger_string_pool;

wep_struct_desc_t g_wc_desc_general;
wep_struct_desc_t g_wc_desc_ammo;
wep_struct_desc_t g_wc_desc_reload;
wep_struct_desc_t g_wc_desc_idle;
wep_struct_desc_t g_wc_desc_akimbo_reload;
wep_struct_desc_t g_wc_desc_akimbo_idle;
wep_struct_desc_t g_wc_desc_akimbo;
wep_struct_desc_t g_wc_desc_laser_idle;
wep_struct_desc_t g_wc_desc_shoot_opts;
wep_struct_desc_t g_wc_desc_laser;
wep_struct_desc_t g_wc_desc_evt[WC_EVT_TOTAL];

int wc_get_int(const char* val);
float wc_get_float(const char* val);

int BitToIndex(uint32_t mask) {
	if (mask == 0) return -1; // or handle error

#if defined(_MSC_VER)
	unsigned long index;
	_BitScanForward(&index, mask);
	return (int)index;
#else
	return __builtin_ctz(mask);
#endif
}

//
// Field definitions for all custom weapon structures
// Update this whenever fields/events/enums/flags are changed
//

void init_weapon_struct_fields() {
	{
		static const char* wep_flags[32];
		//wep_flags[BitToIndex(FL_WC_WEP_HAS_PRIMARY)] = "has_primary";
		//wep_flags[BitToIndex(FL_WC_WEP_HAS_SECONDARY)] = "has_secondary";
		//wep_flags[BitToIndex(FL_WC_WEP_HAS_TERTIARY)] = "has_tertiary";
		//wep_flags[BitToIndex(FL_WC_WEP_HAS_ALT_PRIMARY)] = "has_alt_primary";
		//wep_flags[BitToIndex(FL_WC_WEP_SHOTGUN_RELOAD)] = "shotgun_reload";
		wep_flags[BitToIndex(FL_WC_WEP_UNLINK_COOLDOWNS)] = "unlink_cooldowns";
		//wep_flags[BitToIndex(FL_WC_WEP_AKIMBO)] = "akimbo";
		wep_flags[BitToIndex(FL_WC_WEP_LINK_CHARGEUPS)] = "link_chargeups";
		wep_flags[BitToIndex(FL_WC_WEP_PRIMARY_PRIORITY)] = "primary_priority";
		wep_flags[BitToIndex(FL_WC_WEP_EXCLUSIVE_HOLD)] = "exclusive_hold";
		wep_flags[BitToIndex(FL_WC_WEP_USE_ONLY)] = "use_only";
		//wep_flags[BitToIndex(FL_WC_WEP_HAS_LASER)] = "has_laser";
		wep_flags[BitToIndex(FL_WC_WEP_DYNAMIC_ACCURACY)] = "dynamic_accuracy";
		wep_flags[BitToIndex(FL_WC_WEP_ZOOM_SPR_STRETCH)] = "strech_zoom_sprite";
		wep_flags[BitToIndex(FL_WC_WEP_ZOOM_SPR_ASPECT)] = "keep_zoom_sprite_aspect";
		wep_flags[BitToIndex(FL_WC_WEP_EMPTY_IDLES)] = "empty_idles";
		wep_flags[BitToIndex(FL_WC_WEP_NO_PREDICTION)] = "no_prediction";
		wep_flags[BitToIndex(FL_WC_WEP_HIDE_SECONDARY_AMMO)] = "hide_secondary_ammo";
		wep_flags[BitToIndex(FL_WC_WEP_FORCE_ZOOM_SPRITE)] = "force_zoom_sprite";
		wep_flags[BitToIndex(FL_WC_WEP_HAND_MODELS)] = "has_hand_models";
		wep_flags[BitToIndex(FL_WC_WEP_ALLOW_HL)] = "hl_client_can_use";

		WEP_STRUCT_DESC(g_wc_desc_general, "general",
			WEP_FIELD("classname", "", classname, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("hl_client_classname", "", wrongClientWeapon, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("kill_feed_icon", "", killFeedIcon, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("display_name", "", displayName, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FLAGS32("flags", "0", flags, 0, wep_flags),
			WEP_FIELD("clip_size", "0", maxClip, 0, WC_PARAM_UINT16), // TODO: Remove me
			WEP_FIELD("vmodel", NULL, vmodel, 0, WC_PARAM_MODEL_INDEX, NULL, 0, FL_FIELD_NO_CFG),
			
			WEP_FIELD("v_model", NULL, defaultModelV, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("p_model", NULL, defaultModelP, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("p_model_akimbo", NULL, pmodelAkimbo, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("w_model", NULL, defaultModelW, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("w_model_akimbo", NULL, wmodelAkimbo, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("hud_folder", "", hudFolder, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("slot", "0", slot, 0, WC_PARAM_INT8, NULL, 0, FL_FIELD_NO_NETWORK | FL_FIELD_ALWAYS_WRITE_CFG),
			WEP_FIELD("slot_position", "-1", slotPosition, 0, WC_PARAM_INT8, NULL, 0, FL_FIELD_NO_NETWORK | FL_FIELD_ALWAYS_WRITE_CFG),
			WEP_FIELD("weight", "0", weight, 0, WC_PARAM_INT32, NULL, 0, FL_FIELD_NO_NETWORK),

			WEP_FIELD("deploy_anim", "0", deployAnim, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
			WEP_FIELD("deploy_time", "0", deployTime, 0, WC_PARAM_TIME),
			WEP_FIELD("deploy_anim_time", "0", deployAnimTime, 0, WC_PARAM_TIME),

			WEP_FIELD("thirdperson_anims", "", animExt, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("thirdperson_anims_zoom", "", animExtZoom, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("thirdperson_anims_akimbo", "", animExtAkimbo, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("move_speed", "0", moveSpeedMult, 0, WC_PARAM_UINT16_PERCENT, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("jump_power", "0", jumpPower, 0, WC_PARAM_INT32, NULL, 0, FL_FIELD_NO_NETWORK),
		);
	}
	
	WEP_STRUCT_DESC(g_wc_desc_ammo, "ammo_unnamed",
		WEP_FIELD("type", "", ammoInfo[0].type, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
		WEP_FIELD("clip_size", "0", ammoInfo[0].maxClip, 0, WC_PARAM_UINT16),
		WEP_FIELD("default_give", "0", ammoInfo[0].defaultGive, 0, WC_PARAM_UINT16, NULL, 0, FL_FIELD_NO_NETWORK),
		WEP_FIELD("drop_entity", "", ammoInfo[0].dropEnt, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
		WEP_FIELD("drop_amount", "0", ammoInfo[0].dropAmt, 0, WC_PARAM_UINT32, NULL, 0, FL_FIELD_NO_NETWORK),
	);

	WEP_STRUCT_DESC(g_wc_desc_reload, "reload_unnamed",
		WEP_FIELD("anim", "0", reloadStage[0].anim, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
		WEP_FIELD("time", "0", reloadStage[0].time, 0, WC_PARAM_TIME),
	);

	WEP_STRUCT_DESC(g_wc_desc_akimbo_reload, "reload_akimbo",
		WEP_FIELD("anim", "0", akimbo.reload.anim, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
		WEP_FIELD("time", "0", akimbo.reload.time, 0, WC_PARAM_TIME),
	);

	WEP_STRUCT_DESC(g_wc_desc_idle, "idle",
		WEP_FIELD("anim", "0", idles[0].anim, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
		WEP_FIELD("weight", "0", idles[0].weight, 0, WC_PARAM_UINT8),
		WEP_FIELD("time", "0", idles[0].time, 0, WC_PARAM_TIME),
	);

	WEP_STRUCT_DESC(g_wc_desc_akimbo_idle, "idle_akimbo",
		WEP_FIELD("anim", "0", akimbo.idles[0].anim, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
		WEP_FIELD("weight", "0", akimbo.idles[0].weight, 0, WC_PARAM_UINT8),
		WEP_FIELD("time", "0", akimbo.idles[0].time, 0, WC_PARAM_TIME),
	);

	WEP_STRUCT_DESC(g_wc_desc_laser_idle, "idle_laser",
		WEP_FIELD("anim", "0", laser.idles[0].anim, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
		WEP_FIELD("weight", "0", laser.idles[0].weight, 0, WC_PARAM_UINT8),
		WEP_FIELD("time", "0", laser.idles[0].time, 0, WC_PARAM_TIME),
	);

	{
		static const char* flags[32];
		flags[BitToIndex(FL_WC_SHOOT_UNDERWATER)] = "works_underwater";
		flags[BitToIndex(FL_WC_SHOOT_NO_ATTACK)] = "not_an_attack";
		flags[BitToIndex(FL_WC_SHOOT_COOLDOWN_IDLE)] = "always_cooldown_idle";
		flags[BitToIndex(FL_WC_SHOOT_NEED_AKIMBO)] = "akimbo_only";
		flags[BitToIndex(FL_WC_SHOOT_NEED_FULL_COST)] = "need_full_cost";
		flags[BitToIndex(FL_WC_SHOOT_NO_AUTOFIRE)] = "no_autofire";
		//flags[BitToIndex(FL_WC_SHOOT_IS_MELEE)] = "is_melee";

		static const char* chargeModes[32];
		chargeModes[WC_CHARGEUP_NONE] = "none";
		chargeModes[WC_CHARGEUP_CONSTANT] = "constant_fire";
		chargeModes[WC_CHARGEUP_SINGLE] = "single_fire";
		chargeModes[WC_CHARGEUP_SINGLE_HOLD] = "single_fire_cancellable";
		chargeModes[WC_CHARGEUP_HOLD] = "single_fire_on_release";

		static const char* overchargeModes[32];
		overchargeModes[WC_OVERCHARGE_CANCEL] = "cancel";
		overchargeModes[WC_OVERCHARGE_CONTINUE] = "continue";

		static const char* chargeAmmoModes[32];
		chargeAmmoModes[WC_CHARGE_AMMO_ATTACK] = "spend_on_attack";
		chargeAmmoModes[WC_CHARGE_AMMO_LOAD] = "spend_during_chargeup";

		static const char* chargeFlags[32];
		chargeFlags[BitToIndex(FL_WC_CHARGE_DAMAGE)] = "scale_damage";
		chargeFlags[BitToIndex(FL_WC_CHARGE_KICKBACK)] = "scale_kickback";

		static const char* dmgFlags[32];
		dmgFlags[BitToIndex(DMG_CRUSH)] = "crush";
		dmgFlags[BitToIndex(DMG_BULLET)] = "bullet";
		dmgFlags[BitToIndex(DMG_SLASH)] = "slash";
		dmgFlags[BitToIndex(DMG_BURN)] = "burn";
		dmgFlags[BitToIndex(DMG_FREEZE)] = "freeze";
		dmgFlags[BitToIndex(DMG_BLAST)] = "blast";
		dmgFlags[BitToIndex(DMG_CLUB)] = "club";
		dmgFlags[BitToIndex(DMG_SHOCK)] = "shock";
		dmgFlags[BitToIndex(DMG_SONIC)] = "sonic";
		dmgFlags[BitToIndex(DMG_ENERGYBEAM)] = "energybeam";
		dmgFlags[BitToIndex(DMG_NEVERGIB)] = "nevergib";
		dmgFlags[BitToIndex(DMG_ALWAYSGIB)] = "alwaysgib";
		dmgFlags[BitToIndex(DMG_DROWN)] = "drown";
		dmgFlags[BitToIndex(DMG_PARALYZE)] = "paralyze";
		dmgFlags[BitToIndex(DMG_NERVEGAS)] = "nervegas";
		dmgFlags[BitToIndex(DMG_POISON)] = "poison";
		dmgFlags[BitToIndex(DMG_RADIATION)] = "radiation";
		dmgFlags[BitToIndex(DMG_DROWNRECOVER)] = "drownrecover";
		dmgFlags[BitToIndex(DMG_SLOWBURN)] = "slowburn";
		dmgFlags[BitToIndex(DMG_SLOWFREEZE)] = "slowfreeze";
		dmgFlags[BitToIndex(DMG_MORTAR)] = "mortar";
		dmgFlags[BitToIndex(DMG_SNIPER)] = "sniper";
		dmgFlags[BitToIndex(DMG_MEDKITHEAL)] = "medkitheal";
		dmgFlags[BitToIndex(DMG_LAUNCH)] = "launch";
		dmgFlags[BitToIndex(DMG_SHOCK_GLOW)] = "shock_glow";
		
		WEP_STRUCT_DESC(g_wc_desc_shoot_opts, "unnamed_attack",
			WEP_FLAGS("flags", "0", shootOpts[0].flags, 0, flags),
			WEP_FIELD("ammo_cost", "0", shootOpts[0].ammoCost, 0, WC_PARAM_UINT8),
			WEP_FIELD("ammo_freq", "0", shootOpts[0].ammoFreq, 0, WC_PARAM_UINT8),
			WEP_FIELD("ammo_pool", "0", shootOpts[0].ammoPool, 0, WC_PARAM_UINT8),
			WEP_FIELD("cooldown", "0", shootOpts[0].cooldown, 0, WC_PARAM_TIME),
			WEP_FIELD("cooldown_fail", "0", shootOpts[0].cooldownFail, 0, WC_PARAM_TIME),
			WEP_FIELD("accuracy", "0", shootOpts[0].accuracy, 0, WC_PARAM_ACCURACY_100_2X),
			WEP_FIELD("empty_sound", NULL, shootOpts[0].emptySound, 0, WC_PARAM_SOUND_INDEX),
			WEP__ENUM("overcharge_mode", "0", shootOpts[0].overchargeMode, 2, overchargeModes),
			WEP__ENUM("charge_ammo_mode", "0", shootOpts[0].chargeAmmoMode, 2, chargeAmmoModes),
			WEP__ENUM("charge_mode", "0", shootOpts[0].chargeMode, 4, chargeModes),

			WEP_FLAGS("charge_flags", "0", shootOpts[0].chargeFlags, 0, chargeFlags, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			WEP_FIELD("charge_time", "0", shootOpts[0].chargeTime, 0, WC_PARAM_TIME, NULL, 0, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			WEP_FIELD("overcharge_time", "0", shootOpts[0].overchargeTime, 0, WC_PARAM_TIME, NULL, 0, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			WEP_FIELD("charge_cancel_time", "0", shootOpts[0].chargeCancelTime, 0, WC_PARAM_TIME, NULL, 0, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			
			WEP_FIELD("charge_move_speed", "0", shootOpts[0].chargeMoveSpeedMult, 0, WC_PARAM_UINT16_PERCENT, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_damage", "0", shootOpts[0].melee.damage, 0, WC_PARAM_INT32, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FLAGS32("melee_damage_type", "0", shootOpts[0].melee.damageBits, 0, dmgFlags, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_range", "0", shootOpts[0].melee.range, 0, WC_PARAM_INT32, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_attack_offset", "0", shootOpts[0].melee.attackOffset, 0, WC_PARAM_VECTOR, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_miss_cooldown", "0", shootOpts[0].melee.missCooldown, 0, WC_PARAM_TIME, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_hit_cooldown", "0", shootOpts[0].melee.hitCooldown, 0, WC_PARAM_TIME, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_decal_delay", "0", shootOpts[0].melee.decalDelay, 0, WC_PARAM_TIME, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_hit_wall_sound", NULL, shootOpts[0].melee.hitWallSounds[0], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_hit_wall_sound2", NULL, shootOpts[0].melee.hitWallSounds[1], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_hit_wall_sound3", NULL, shootOpts[0].melee.hitWallSounds[2], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_hit_wall_sound4", NULL, shootOpts[0].melee.hitWallSounds[3], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_hit_flesh_sound", NULL, shootOpts[0].melee.hitFleshSounds[0], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_hit_flesh_sound2", NULL, shootOpts[0].melee.hitFleshSounds[1], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_hit_flesh_sound3", NULL, shootOpts[0].melee.hitFleshSounds[2], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_hit_flesh_sound4", NULL, shootOpts[0].melee.hitFleshSounds[3], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_miss_sound", NULL, shootOpts[0].melee.missSounds[0], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_miss_sound2", NULL, shootOpts[0].melee.missSounds[1], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_miss_sound3", NULL, shootOpts[0].melee.missSounds[2], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_miss_sound4", NULL, shootOpts[0].melee.missSounds[3], 0, WC_PARAM_SOUND_INDEX, NULL, 0, FL_FIELD_NO_NETWORK),
		);
	}

	WEP_STRUCT_DESC(g_wc_desc_akimbo, "akimbo",
		WEP_FIELD("deploy_anim", "0", akimbo.deployAnim, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
		WEP_FIELD("deploy_time", "0", akimbo.deployTime, 0, WC_PARAM_TIME),
		WEP_FIELD("akimbo_deploy_anim", "0", akimbo.akimboDeployAnim, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
		WEP_FIELD("akimbo_deploy_time", "0", akimbo.akimboDeployTime, 0, WC_PARAM_TIME),
		WEP_FIELD("akimbo_deploy_anim_time", "0", akimbo.akimboDeployAnimTime, 0, WC_PARAM_TIME),
		WEP_FIELD("holster_anim", "0", akimbo.holsterAnim, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
		WEP_FIELD("holster_time", "0", akimbo.holsterTime, 0, WC_PARAM_TIME),
		WEP_FIELD("accuracy", "0", akimbo.accuracy, 0, WC_PARAM_ACCURACY_100_2X),
	);

	WEP_STRUCT_DESC(g_wc_desc_laser, "laser",
		WEP_FIELD("dot_sprite", NULL, laser.dotSprite, 0, WC_PARAM_MODEL_INDEX),
		WEP_FIELD("beam_sprite", NULL, laser.beamSprite, 0, WC_PARAM_MODEL_INDEX),
		WEP_FIELD("dot_size", "0", laser.dotSz, 0, WC_PARAM_UINT8),
		WEP_FIELD("beam_width", "0", laser.beamWidth, 0, WC_PARAM_UINT8),
		WEP_FIELD("attachment", "0", laser.attachment, 0, WC_PARAM_UINT8),
	);
}

void init_event_fields() {
	static const char* flash_size_names[32];
	flash_size_names[WC_FLASH_NONE] = "none";
	flash_size_names[WC_FLASH_DIM] = "dim";
	flash_size_names[WC_FLASH_NORMAL] = "normal";
	flash_size_names[WC_FLASH_BRIGHT] = "bright";

	EVT_DESC(WC_EVT_IDLE_SOUND, "idle_sound", // written to config as "sound"
		EVT_FIELD("volume", "1.0", idleSound.volume, 7, WC_PARAM_7BIT_PERCENT),
		EVT_FIELD("sound", NULL, idleSound.sound, 9, WC_PARAM_SOUND_INDEX),
	);

	{
		static const char* distant_sound_names[8];
		distant_sound_names[DISTANT_NONE] = "none";
		distant_sound_names[DISTANT_9MM] = "distant_9mm";
		distant_sound_names[DISTANT_357] = "distant_357";
		distant_sound_names[DISTANT_556] = "distant_556";
		distant_sound_names[DISTANT_BOOM] = "distant_boom";

		static const char* flags[32];
		flags[BitToIndex(FL_WC_SOUND_CHARGE_PITCH)] = "chargeup_pitch";

		static const char* channel_names[8];
		channel_names[CHAN_AUTO] = "auto";
		channel_names[CHAN_WEAPON] = "weapon";
		channel_names[CHAN_VOICE] = "voice";
		channel_names[CHAN_ITEM] = "item";
		channel_names[CHAN_BODY] = "body";
		channel_names[CHAN_STATIC] = "static";

		static const char* aivol_names[8];
		aivol_names[WC_AIVOL_SILENT] = "silent";
		aivol_names[WC_AIVOL_QUIET] = "quiet";
		aivol_names[WC_AIVOL_NORMAL] = "normal";
		aivol_names[WC_AIVOL_LOUD] = "loud";

		EVT_DESC(WC_EVT_PLAY_SOUND, "sound",
			EVT_FLAGS("flags", "0", playSound.flags, 2, flags),
			EVT__ENUM("volume_for_ai", "silent", playSound.aiVol, 2, aivol_names),
			EVT__ENUM("channel", "static", playSound.channel, 3, channel_names),
			EVT_FIELD("sound", NULL, playSound.sound, 9, WC_PARAM_SOUND_INDEX),
			EVT_FIELD("volume", "1.0", playSound.volume, 0, WC_PARAM_UINT8_PERCENT),
			EVT_FIELD("attenuation", "2.0", playSound.attn, 0, WC_PARAM_UINT8_FP_2_6), // default = ATTN_IDLE
			EVT_FIELD("pitch_min", "100", playSound.pitchMin, 0, WC_PARAM_UINT8),
			EVT_FIELD("pitch_max", "100", playSound.pitchMax, 0, WC_PARAM_UINT8),
			EVT_FIELD("sound", NULL, playSound.additionalSounds, 0, WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2),
			EVT__ENUM("distant_sound", "none", playSound.distantSound, 0, distant_sound_names, FL_FIELD_NO_NETWORK),
		);
	}

	{
		static const char* shell_sound_names[8];
		shell_sound_names[TE_BOUNCE_NULL] = "none";
		shell_sound_names[TE_BOUNCE_SHELL] = "shell";
		shell_sound_names[TE_BOUNCE_SHOTSHELL] = "shotshell";

		EVT_DESC(WC_EVT_EJECT_SHELL, "eject_shell",
			EVT_FIELD("has_rand", "0", ejectShell.hasRand, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("has_vel", "0", ejectShell.hasVel, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT__ENUM("sound", "0", ejectShell.sound, 2, shell_sound_names),
			EVT_FIELD("model", NULL, ejectShell.model, 12, WC_PARAM_MODEL_INDEX),
			EVT_FIELD("offset", "0 0 0", ejectShell.offset, 0, WC_PARAM_VECTOR_INT8),
			EVT_FIELD("vel", "0 0 0", ejectShell.vel, 0, WC_PARAM_VECTOR_INT8, NULL, 0, 0, EVT_COND_BYTE(ejectShell.hasVel)),
			EVT_FIELD("direction_randomness", "0", ejectShell.dirRand, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(ejectShell.hasRand)),
			EVT_FIELD("speed_randomness", "0", ejectShell.speedRand, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(ejectShell.hasRand)),
		);
	}

	{
		static const char* flags[8];
		flags[BitToIndex(FL_WC_PUNCH_SET)] = "set_angles";
		flags[BitToIndex(FL_WC_PUNCH_ADD)] = "add_angles";
		flags[BitToIndex(FL_WC_PUNCH_NO_RETURN)] = "no_recovery";
		flags[BitToIndex(FL_WC_PUNCH_DUCK)] = "only_when_ducking";
		flags[BitToIndex(FL_WC_PUNCH_STAND)] = "only_when_standing";

		EVT_DESC(WC_EVT_PUNCH, "recoil",
			EVT_FLAGS("flags", "0", punch.flags, 8, flags),
			EVT_FIELD("angles", "0 0 0", punch.angles, 0, WC_PARAM_VECTOR_FP_10_6),
		);
	}

	EVT_DESC(WC_EVT_SET_BODY, "new_body",
		EVT_FIELD("new_body", "0", setBody.newBody, 0, WC_PARAM_UINT8),
	);

	{
		static const char* flags[8];
		flags[BitToIndex(FL_WC_ANIM_NO_RESET)] = "no_reset";
		flags[BitToIndex(FL_WC_ANIM_PMODEL)] = "thirdperson_model";
		flags[BitToIndex(FL_WC_ANIM_ORDERED)] = "play_in_order";

		static const char* hand_names[8];
		hand_names[WC_ANIM_BOTH_HANDS] = "both";
		hand_names[WC_ANIM_LEFT_HAND] = "left";
		hand_names[WC_ANIM_RIGHT_HAND] = "right";
		hand_names[WC_ANIM_TRIG_HAND] = "trigger";

		EVT_DESC(WC_EVT_WEP_ANIM, "weapon_anim",
			EVT__ENUM("hand", "both", anim.akimbo, 3, hand_names),
			EVT_FLAGS("flags", "0", anim.flags, 5, flags),
			EVT_FIELD("anims", "0", anim.anims, 0, WC_PARAM_UINT8_ARRAY_8),
		);
	}

	{
		static const char* flags[8];
		flags[BitToIndex(FL_WC_BULLETS_DYNAMIC_SPREAD)] = "dynamic_spread";
		flags[BitToIndex(FL_WC_BULLETS_NO_DECAL)] = "no_decal";
		flags[BitToIndex(FL_WC_BULLETS_NO_SOUND)] = "no_sound";

		static const char* bullet_color_names[32];
		bullet_color_names[WC_TRACER_COLOR_WHITE] = "white";
		bullet_color_names[WC_TRACER_COLOR_RED] = "red";
		bullet_color_names[WC_TRACER_COLOR_GREEN] = "green";
		bullet_color_names[WC_TRACER_COLOR_BLUE] = "blue";
		bullet_color_names[WC_TRACER_COLOR_DEFAULT] = "default";
		bullet_color_names[WC_TRACER_COLOR_YELLOW] = "yellow";
		bullet_color_names[WC_TRACER_COLOR_ORANGE2] = "orange2";
		bullet_color_names[WC_TRACER_COLOR_BLUE2] = "blue2";
		bullet_color_names[WC_TRACER_COLOR_ORANGE3] = "orange3";
		bullet_color_names[WC_TRACER_COLOR_ORANGE4] = "orange4";
		bullet_color_names[WC_TRACER_COLOR_TAN] = "tan";
		bullet_color_names[WC_TRACER_COLOR_ORANGE] = "orange";

		EVT_DESC(WC_EVT_BULLETS, "bullets",
			EVT_FIELD("count", "0", bullets.count, 0, WC_PARAM_UINT8),
			EVT_FIELD("burst_delay", "0", bullets.burstDelay, 0, WC_PARAM_TIME),
			EVT_FIELD("damage", "0", bullets.damage, 0, WC_PARAM_UINT16),
			EVT_FIELD("accuracy", "0", bullets.accuracy, 0, WC_PARAM_ACCURACY_UINT16_2X),
			EVT__ENUM("tracer_color", "default", bullets.tracerColor, 4, bullet_color_names),
			EVT_FIELD("tracer_frequency", "0", bullets.tracerFreq, 4, WC_PARAM_UINT8),
			EVT__ENUM("flash_size", "normal", bullets.flashSz, 4, flash_size_names),
			EVT_FLAGS("flags", "0", bullets.flags, 4, flags),
		);
	}

	{
		static const char* flags[32];
		flags[BitToIndex(FL_WC_BEAM_SPIRAL)] = "spiral";
		flags[BitToIndex(FL_WC_BEAM_OPAQUE)] = "opaque";
		flags[BitToIndex(FL_WC_BEAM_SHADEIN)] = "fade_in";
		flags[BitToIndex(FL_WC_BEAM_SHADEOUT)] = "fade_out";

		static const char* anim_names[32];
		anim_names[WC_BEAM_ANIM_DISABLED] = "disabled";
		anim_names[WC_BEAM_ANIM_TOGGLE] = "toggle";
		anim_names[WC_BEAM_ANIM_LINEAR] = "linear";
		anim_names[WC_BEAM_ANIM_LINEAR_TOGGLE] = "linear_toggle";
		anim_names[WC_BEAM_ANIM_EASE_IN_OUT] = "ease_in_out";

		EVT_DESC(WC_EVT_BEAM, "beam",
			EVT_FLAGS("flags", "0", beam.flags, 4, flags),
			EVT_FIELD("attachment", "0", beam.attachment, 3, WC_PARAM_UINT8),
			EVT_FIELD("sprite", NULL, beam.sprite, 9, WC_PARAM_MODEL_INDEX),

			EVT_FIELD("life", "0", beam.life, 0, WC_PARAM_TIME),
			EVT_FIELD("accuracy", "0", beam.accuracy, 0, WC_PARAM_ACCURACY_UINT16_2X),
			EVT_FIELD("damage", "0", beam.damage, 0, WC_PARAM_UINT16),
			EVT_FIELD("distance", "0", beam.distance, 0, WC_PARAM_UINT16),
			EVT_FIELD("frequency", "0", beam.freq, 0, WC_PARAM_TIME),

			EVT_FIELD("has_impact_sprite", "0", beam.hasImpactSprite, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT__ENUM("animate", "disabled", beam.altMode, 3, anim_names),
			EVT_FIELD("id", "0", beam.id, 4, WC_PARAM_UINT8),			

			EVT_FIELD("width", "0", beam.width, 0, WC_PARAM_UINT8),
			EVT_FIELD("noise", "0", beam.noise, 0, WC_PARAM_UINT8),
			EVT_FIELD("scroll_rate", "0", beam.scrollRate, 0, WC_PARAM_UINT8),
			EVT_FIELD("color", "0 0 0 0", beam.color, 0, WC_PARAM_RGBA),

			EVT_FIELD("animate_time", "0", beam.altTime, 0, WC_PARAM_TIME, NULL, 0, 0, EVT_COND_BYTE(beam.altMode)),
			EVT_FIELD("width_alt", "0", beam.widthAlt, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(beam.altMode)),
			EVT_FIELD("noise_alt", "0", beam.noiseAlt, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(beam.altMode)),
			EVT_FIELD("scroll_rate_alt", "0", beam.scrollRateAlt, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(beam.altMode)),
			EVT_FIELD("color_alt", "0 0 0 0", beam.colorAlt, 0, WC_PARAM_RGBA, NULL, 0, 0, EVT_COND_BYTE(beam.altMode)),

			EVT_FIELD("impact_sprite_fps", "0", beam.impactSpriteFps, 7, WC_PARAM_UINT16, NULL, 0, 0, EVT_COND_BYTE(beam.hasImpactSprite)),
			EVT_FIELD("impact_sprite", NULL, beam.impactSprite, 9, WC_PARAM_MODEL_INDEX, NULL, 0, 0, EVT_COND_BYTE(beam.hasImpactSprite)),
			EVT_FIELD("impact_sprite_scale", "0", beam.impactSpriteScale, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(beam.hasImpactSprite)),
			EVT_FIELD("impact_sprite_color", "0", beam.impactSpriteColor, 0, WC_PARAM_RGBA, NULL, 0, 0, EVT_COND_BYTE(beam.hasImpactSprite)),
		);
	}

	{
		static const char* flags[32];
		flags[BitToIndex(FL_WC_PROJ_NO_BUBBLES)] = "no_bubbles";
		flags[BitToIndex(FL_WC_PROJ_NO_ORIENT)] = "static_angles";
		flags[BitToIndex(FL_WC_PROJ_IS_HOOK)] = "is_hook";

		static const char* proj_type_names[32];
		proj_type_names[WC_PROJECTILE_ARGRENADE] = "ar_grenade";
		proj_type_names[WC_PROJECTILE_BANANA] = "banana_bomb";
		proj_type_names[WC_PROJECTILE_BOLT] = "crossbow_bolt";
		proj_type_names[WC_PROJECTILE_DISPLACER] = "displacer_portal";
		proj_type_names[WC_PROJECTILE_GRENADE] = "hand_grenade";
		proj_type_names[WC_PROJECTILE_HORNET] = "hornet";
		proj_type_names[WC_PROJECTILE_HVR] = "hvr";
		proj_type_names[WC_PROJECTILE_MORTAR] = "mortar";
		proj_type_names[WC_PROJECTILE_RPG] = "rpg";
		proj_type_names[WC_PROJECTILE_SHOCK] = "shock_beam";
		proj_type_names[WC_PROJECTILE_WEAPON] = "weapon";
		proj_type_names[WC_PROJECTILE_TRIPMINE] = "tripmine";
		proj_type_names[WC_PROJECTILE_CUSTOM] = "custom";
		proj_type_names[WC_PROJECTILE_OTHER] = "other";

		static const char* proj_action_names[32];
		proj_action_names[WC_PROJ_ACT_IMPACT] = "impact";
		proj_action_names[WC_PROJ_ACT_BOUNCE] = "bounce";
		proj_action_names[WC_PROJ_ACT_ATTACH] = "attach";

		EVT_DESC(WC_EVT_PROJECTILE, "projectile",
			EVT__ENUM("type", "0", proj.type, 0, proj_type_names),
			EVT_FIELD("entity_class", "", proj.entity_class, 0, WC_PARAM_STRING),
			EVT_FLAGS("flags", "0", proj.flags, 0, flags),
			EVT_FIELD("accuracy", "0", proj.accuracy, 0, WC_PARAM_ACCURACY_UINT16_2X),
			EVT__ENUM("hit_world_action", "impact", proj.world_event, 0, proj_action_names),
			EVT__ENUM("hit_monster_action", "impact", proj.monster_event, 0, proj_action_names),
			EVT_FIELD("speed", "0", proj.speed, 0, WC_PARAM_INT32),
			EVT_FIELD("life", "0", proj.life, 0, WC_PARAM_TIME),
			EVT_FIELD("elasticity", "0", proj.elasticity, 0, WC_PARAM_FLOAT),
			EVT_FIELD("gravity", "0", proj.gravity, 0, WC_PARAM_FLOAT),
			EVT_FIELD("air_friction", "0", proj.air_friction, 0, WC_PARAM_FLOAT),
			EVT_FIELD("water_friction", "0", proj.water_friction, 0, WC_PARAM_FLOAT),
			EVT_FIELD("hull_size", "0", proj.size, 0, WC_PARAM_FLOAT),
			EVT_FIELD("direction", "0 0 0", proj.dir, 0, WC_PARAM_VECTOR),
			EVT_FIELD("model", NULL, proj.model, 0, WC_PARAM_MODEL_INDEX),
			EVT_FIELD("move_snd", "", proj.move_snd, 0, WC_PARAM_STRING),
			EVT_FIELD("damage", "0", proj.damage, 0, WC_PARAM_UINT16),
			EVT_FIELD("damageBits", "0", proj.damageBits, 0, WC_PARAM_UINT32),
			EVT_FIELD("sprite", "", proj.sprite, 0, WC_PARAM_STRING),
			EVT_FIELD("sprite_color", "0 0 0 0", proj.sprite_color, 0, WC_PARAM_RGBA),
			EVT_FIELD("angles", "0 0 0", proj.angles, 0, WC_PARAM_VECTOR),
			EVT_FIELD("angular_velocity", "0 0 0", proj.avel, 0, WC_PARAM_VECTOR),
			EVT_FIELD("offset", "0 0 0", proj.offset, 0, WC_PARAM_VECTOR),
			EVT_FIELD("player_vel_inf", "0 0 0", proj.player_vel_inf, 0, WC_PARAM_VECTOR),
			EVT_FIELD("follow_mode", "0", proj.follow_mode, 0, WC_PARAM_UINT8),
			EVT_FIELD("follow_radius", "0", proj.follow_radius, 0, WC_PARAM_FLOAT),
			EVT_FIELD("follow_time", "0 0 0", proj.follow_time, 0, WC_PARAM_VECTOR),
			EVT_FIELD("trail_sprite", "", proj.trail_spr, 0, WC_PARAM_STRING),
			EVT_FIELD("trail_life", "0", proj.trail_life, 0, WC_PARAM_TIME),
			EVT_FIELD("trail_width", "0", proj.trail_width, 0, WC_PARAM_UINT8),
			EVT_FIELD("trail_color", "0 0 0 0", proj.trail_color, 0, WC_PARAM_RGBA),
		);
	}

	EVT_DESC(WC_EVT_KICKBACK, "kickback",
		EVT_FIELD("push_force", "0", kickback.pushForce, 0, WC_PARAM_UINT16),
		EVT_FIELD("percent_back", "0", kickback.back, 0, WC_PARAM_INT8),
		EVT_FIELD("percent_right", "0", kickback.right, 0, WC_PARAM_INT8),
		EVT_FIELD("percent_up", "0", kickback.up, 0, WC_PARAM_INT8),
		EVT_FIELD("percent_global_up", "0", kickback.globalUp, 0, WC_PARAM_INT8),
	);

	{
		static const char* mode_names[32];
		mode_names[WC_TOGGLE_STATE_OFF] = "off";
		mode_names[WC_TOGGLE_STATE_ON] = "on";
		mode_names[WC_TOGGLE_STATE_TOGGLE] = "toggle";

		static const char* state_names[32];
		state_names[BitToIndex(FL_WC_STATE_PRIMARY_ALT)] = "primary_alt";
		state_names[BitToIndex(FL_WC_STATE_LASER)] = "laser";
		state_names[BitToIndex(FL_WC_STATE_IS_AKIMBO)] = "akimbo";
		state_names[BitToIndex(FL_WC_STATE_CAN_AKIMBO)] = "can_akimbo";

		EVT_DESC(WC_EVT_TOGGLE_STATE, "toggle_state",
			EVT__ENUM("toggle_mode", "toggle", toggleState.toggleMode, 2, mode_names),
			EVT_FLAGS("toggled_states", "0", toggleState.stateBits, 14, state_names),
		);
	}

	EVT_DESC(WC_EVT_TOGGLE_ZOOM, "toggle_zoom",
		EVT_FIELD("zoom_fov", "0", zoomToggle.zoomFov, 0, WC_PARAM_UINT8),
		EVT_FIELD("zoom_fov2", "0", zoomToggle.zoomFov2, 0, WC_PARAM_UINT8),
	);

	{
		static const char* targets[32];
		targets[BitToIndex(FL_WC_COOLDOWN_PRIMARY)] = "primary";
		targets[BitToIndex(FL_WC_COOLDOWN_SECONDARY)] = "secondary";
		targets[BitToIndex(FL_WC_COOLDOWN_TERTIARY)] = "tertiary";
		targets[BitToIndex(FL_WC_COOLDOWN_IDLE)] = "idle";

		EVT_DESC(WC_EVT_COOLDOWN, "cooldown",
			EVT_FIELD("time", "0", cooldown.millis, 0, WC_PARAM_TIME),
			EVT_FLAGS("actions", "0", cooldown.targets, 0, targets),
		);
	}

	EVT_DESC(WC_EVT_SET_GRAVITY, "set_gravity",
		EVT_FIELD("gravity", "0", setGravity.gravity, 0, WC_PARAM_INT16),
	);

	EVT_DESC(WC_EVT_DLIGHT, "dynamic_light",
		EVT_FIELD("color", "0 0 0", dlight.color, 0, WC_PARAM_RGB),
		EVT_FIELD("radius", "0", dlight.radius, 0, WC_PARAM_UINT8),
		EVT_FIELD("life", "0", dlight.life, 0, WC_PARAM_UINT8),
		EVT_FIELD("decay_rate", "0", dlight.decayRate, 0, WC_PARAM_UINT8),
	);

	EVT_DESC(WC_EVT_SERVER, "user_defined",
		EVT_FIELD("type", "0", server.type, 0, WC_PARAM_UINT8),
		EVT_FIELD("iuser1", "0", server.iuser1, 0, WC_PARAM_INT32),
		EVT_FIELD("iuser2", "0", server.iuser2, 0, WC_PARAM_INT32),
		EVT_FIELD("iuser3", "0", server.iuser3, 0, WC_PARAM_INT32),
		EVT_FIELD("iuser4", "0", server.iuser4, 0, WC_PARAM_INT32),
		EVT_FIELD("fuser1", "0", server.fuser1, 0, WC_PARAM_FLOAT),
		EVT_FIELD("fuser2", "0", server.fuser2, 0, WC_PARAM_FLOAT),
		EVT_FIELD("fuser3", "0", server.fuser3, 0, WC_PARAM_FLOAT),
		EVT_FIELD("fuser4", "0", server.fuser4, 0, WC_PARAM_FLOAT),
		EVT_FIELD("vuser1", "0 0 0", server.vuser1, 0, WC_PARAM_VECTOR),
		EVT_FIELD("vuser2", "0 0 0", server.vuser2, 0, WC_PARAM_VECTOR),
		EVT_FIELD("vuser3", "0 0 0", server.vuser3, 0, WC_PARAM_VECTOR),
		EVT_FIELD("vuser4", "0 0 0", server.vuser4, 0, WC_PARAM_VECTOR),
		EVT_FIELD("suser1", "", server.suser1, 0, WC_PARAM_STRING),
		EVT_FIELD("suser2", "", server.suser2, 0, WC_PARAM_STRING),
		EVT_FIELD("suser3", "", server.suser3, 0, WC_PARAM_STRING),
		EVT_FIELD("suser4", "", server.suser4, 0, WC_PARAM_STRING),
		EVT_FIELD("cuser1", "0 0 0 0", server.cuser1, 0, WC_PARAM_RGBA),
		EVT_FIELD("cuser2", "0 0 0 0", server.cuser2, 0, WC_PARAM_RGBA),
		EVT_FIELD("cuser3", "0 0 0 0", server.cuser3, 0, WC_PARAM_RGBA),
		EVT_FIELD("cuser4", "0 0 0 0", server.cuser4, 0, WC_PARAM_RGBA),
		EVT_FIELD("euser1", "0", server.euser1, 0, WC_PARAM_UINT32),
		EVT_FIELD("euser2", "0", server.euser2, 0, WC_PARAM_UINT32),
		EVT_FIELD("euser3", "0", server.euser3, 0, WC_PARAM_UINT32),
		EVT_FIELD("euser4", "0", server.euser4, 0, WC_PARAM_UINT32),
	);

	EVT_DESC(WC_EVT_MUZZLEFLASH, "muzzle_flash",
		EVT__ENUM("flash_size", "0", muzzleFlash.brightness, 0, flash_size_names),
	);

	EVT_DESC(WC_EVT_HIDE_LASER, "hide_laser",
		EVT_FIELD("time", "0", laserHide.millis, 0, WC_PARAM_TIME),
	);

	EVT_DESC(WC_EVT_SPRITETRAIL, "sprite_trail",
		EVT_FIELD("sprite", NULL, spriteTrail.sprite, 0, WC_PARAM_MODEL_INDEX),
		EVT_FIELD("count", "0", spriteTrail.count, 0, WC_PARAM_UINT8),
		EVT_FIELD("scale", "0", spriteTrail.scale, 0, WC_PARAM_UINT8),
		EVT_FIELD("speed", "0", spriteTrail.speed, 0, WC_PARAM_UINT8),
		EVT_FIELD("speed_randomness", "0", spriteTrail.speedNoise, 0, WC_PARAM_UINT8),
	);

	{
		static const char* flags[32];
		flags[BitToIndex(1)] = "particles";

		EVT_DESC(WC_EVT_DECAL, "decal",
			EVT_FIELD("texture", "0", decal.decalIdx, 0, WC_PARAM_DECAL_INDEX),
			EVT_FLAGS("flags", "0", decal.flags, 0, flags),
		);
	}
}

void init_weapon_custom_config_parser() {
	g_wc_name_to_trigger.clear();
	g_wc_name_to_action.clear();

	init_weapon_struct_fields();
	init_event_fields();

	g_wc_evt_trigger_names[WC_TRIG_PRIMARY] = "primary_attack";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY] = "secondary_attack";
	g_wc_evt_trigger_names[WC_TRIG_TERTIARY] = "tertiary_attack";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_ALT] = "primary_alt_attack";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_CLIPSIZE] = "primary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_CLIP_SP] = "primary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_CHARGE] = "primary_attack_charge";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_OVERCHARGE] = "primary_attack_overcharge";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_START] = "primary_attack_start";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_STOP] = "primary_attack_stop";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_FAIL] = "primary_attack_fail";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_CLIPSIZE] = "secondary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_CLIP_SP] = "secondary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_CHARGE] = "secondary_attack_charge";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_OVERCHARGE] = "secondary_attack_overcharge";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_START] = "secondary_attack_start";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_STOP] = "secondary_attack_stop";
	g_wc_evt_trigger_names[WC_TRIG_RELOAD] = "reload";
	g_wc_evt_trigger_names[WC_TRIG_RELOAD_EMPTY] = "reload_empty";
	g_wc_evt_trigger_names[WC_TRIG_RELOAD_NOT_EMPTY] = "reload_not_empty";
	g_wc_evt_trigger_names[WC_TRIG_RELOAD_FINISH] = "reload_finish";
	g_wc_evt_trigger_names[WC_TRIG_DEPLOY] = "deploy";
	g_wc_evt_trigger_names[WC_TRIG_BULLET_FIRED] = "bullet_fired";
	g_wc_evt_trigger_names[WC_TRIG_LASER_ON] = "laser_on";
	g_wc_evt_trigger_names[WC_TRIG_LASER_OFF] = "laser_off";
	g_wc_evt_trigger_names[WC_TRIG_ZOOM_IN] = "zoom_in";
	g_wc_evt_trigger_names[WC_TRIG_ZOOM_OUT] = "zoom_out";
	g_wc_evt_trigger_names[WC_TRIG_IMPACT] = "impact";

	g_wc_evt_trigger_arg_primary_names[WC_TRIG_SHOOT_ARG_ALWAYS] = "";
	g_wc_evt_trigger_arg_primary_names[WC_TRIG_SHOOT_ARG_AKIMBO] = "_akimbo";
	g_wc_evt_trigger_arg_primary_names[WC_TRIG_SHOOT_ARG_NOT_AKIMBO] = "_not_akimbo";

	g_wc_evt_trigger_clip_sp_names[WC_TRIG_CLIP_ARG_ODD] = "odd";
	g_wc_evt_trigger_clip_sp_names[WC_TRIG_CLIP_ARG_EVEN] = "even";
	g_wc_evt_trigger_clip_sp_names[WC_TRIG_CLIP_ARG_NOT_EMPTY] = "not_empty";

	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_PRIMARY_ANY] = "primary_any";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_PRIMARY_WORLD] = "primary_world";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_PRIMARY_MONSTER] = "primary_monster";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_SECONDARY_ANY] = "secondary_any";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_SECONDARY_WORLD] = "secondary_world";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_SECONDARY_MONSTER] = "secondary_monster";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_TERTIARY_ANY] = "tertiary_any";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_TERTIARY_WORLD] = "tertiary_world";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_TERTIARY_MONSTER] = "tertiary_monster";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_PRIMARY_ALT_ANY] = "primary_alt_any";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_PRIMARY_ALT_WORLD] = "primary_alt_world";
	g_wc_evt_trigger_impact_names[WC_TRIG_IMPACT_PRIMARY_ALT_MONSTER] = "primary_alt_monster";

	for (int i = 0; i < ARRAY_SZ(g_wc_evt_trigger_names); i++) {
		if (!g_wc_evt_trigger_names[i])
			continue;

		const char* tname = g_wc_evt_trigger_names[i];

		switch (i) {
		case WC_TRIG_PRIMARY:
		case WC_TRIG_SECONDARY:
		case WC_TRIG_TERTIARY:
		case WC_TRIG_RELOAD:
		case WC_TRIG_RELOAD_EMPTY:
		case WC_TRIG_RELOAD_NOT_EMPTY:
		case WC_TRIG_RELOAD_FINISH:
		case WC_TRIG_DEPLOY: {
			for (int k = 0; k < ARRAY_SZ(g_wc_evt_trigger_arg_primary_names); k++) {
				const char* key = UTIL_VarArgs("%s%s", tname, g_wc_evt_trigger_arg_primary_names[k]);
				uint16_t val = (k << 5) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		}
		case WC_TRIG_PRIMARY_CLIPSIZE:
			for (int k = 0; k < 32; k++) {
				const char* key = UTIL_VarArgs("%s_%u", tname, k);
				uint16_t val = (k << 5) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		case WC_TRIG_PRIMARY_CLIP_SP:
		case WC_TRIG_SECONDARY_CLIP_SP:
			for (int k = 0; k < ARRAY_SZ(g_wc_evt_trigger_clip_sp_names); k++) {
				const char* key = UTIL_VarArgs("%s_%s", tname, g_wc_evt_trigger_clip_sp_names[k]);
				uint16_t val = (k << 5) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		case WC_TRIG_IMPACT:
			for (int k = 0; k < ARRAY_SZ(g_wc_evt_trigger_impact_names); k++) {
				const char* key = UTIL_VarArgs("%s_%s", tname, g_wc_evt_trigger_impact_names[k]);
				uint16_t val = (k << 5) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		default:
			g_wc_name_to_trigger.put(tname, i);
			g_wc_trigger_to_name[i] = g_wc_trigger_string_pool.alloc(tname);
			break;
		}
	}

	g_wc_evt_type_names[WC_EVT_IDLE_SOUND] = "sound";
	g_wc_evt_type_names[WC_EVT_PLAY_SOUND] = "sound";
	g_wc_evt_type_names[WC_EVT_EJECT_SHELL] = "eject_shell";
	g_wc_evt_type_names[WC_EVT_PUNCH] = "recoil";
	g_wc_evt_type_names[WC_EVT_SET_BODY] = "set_weapon_body";
	g_wc_evt_type_names[WC_EVT_WEP_ANIM] = "weapon_anim";
	g_wc_evt_type_names[WC_EVT_BULLETS] = "bullets";
	g_wc_evt_type_names[WC_EVT_BEAM] = "beam";
	g_wc_evt_type_names[WC_EVT_PROJECTILE] = "projectile";
	g_wc_evt_type_names[WC_EVT_KICKBACK] = "kickback";
	g_wc_evt_type_names[WC_EVT_TOGGLE_STATE] = "toggle_state";
	g_wc_evt_type_names[WC_EVT_TOGGLE_ZOOM] = "toggle_zoom";
	g_wc_evt_type_names[WC_EVT_HIDE_LASER] = "hide_laser";
	g_wc_evt_type_names[WC_EVT_COOLDOWN] = "cooldown";
	g_wc_evt_type_names[WC_EVT_SET_GRAVITY] = "set_gravity";
	g_wc_evt_type_names[WC_EVT_DLIGHT] = "dynamic_light";
	g_wc_evt_type_names[WC_EVT_SERVER] = "user_defined";
	g_wc_evt_type_names[WC_EVT_MUZZLEFLASH] = "muzzle_flash";

	g_wc_evt_category_names[WC_EVT_CATEGORY_PRIMARY] = "Primary attack";
	g_wc_evt_category_names[WC_EVT_CATEGORY_PRIMARY_ALT] = "Alternate primary attack";
	g_wc_evt_category_names[WC_EVT_CATEGORY_SECONDARY] = "Secondary attack";
	g_wc_evt_category_names[WC_EVT_CATEGORY_TERTIARY] = "Tertiary attack";
	g_wc_evt_category_names[WC_EVT_CATEGORY_RELOAD] = "Reload";
	g_wc_evt_category_names[WC_EVT_CATEGORY_DEPLOY] = "Deploy events";
	g_wc_evt_category_names[WC_EVT_CATEGORY_REACTION] = "Reactionary events";
	g_wc_evt_category_names[WC_EVT_CATEGORY_STATE_CHANGE] = "State change events";
	g_wc_evt_category_names[WC_EVT_CATEGORY_UNKNOWN] = "Uncategorized events";

	// impact events
	g_wc_evt_type_names[WC_EVT_SPRITETRAIL] = "sprite_trail";
	g_wc_evt_type_names[WC_EVT_DECAL] = "decal";

	for (int i = 0; i < ARRAY_SZ(g_wc_evt_type_names); i++) {
		if (!g_wc_evt_type_names[i])
			continue;

		g_wc_name_to_action.put(g_wc_evt_type_names[i], i);
	}
}


//
// Functions for reading/writing each type of field
// Update this when adding new field types
//

void wc_read_field(const char* fname, SettingsGroup& group, void* dat, const char* name, const char* val,
	int ptype, wep_field_desc_t* field) {
	switch (ptype) {
	case WC_PARAM_UINT8:			*(uint8_t*)dat = wc_get_int(val); break;
	case WC_PARAM_UINT8_PERCENT:	*(uint8_t*)dat = atof(val) * 255 + 0.5f; break;
	case WC_PARAM_7BIT_PERCENT:		*(uint8_t*)dat = atof(val) * 127 + 0.5f; break;
	case WC_PARAM_UINT8_FP_2_6:		*(uint8_t*)dat = atof(val) * 64; break;
	case WC_PARAM_UINT16:			*(uint16_t*)dat = wc_get_int(val); break;
	case WC_PARAM_UINT32:			*(uint32_t*)dat = wc_get_int(val); break;
	case WC_PARAM_INT8:				*(int8_t*)dat = wc_get_int(val); break;
	case WC_PARAM_INT16:			*(int16_t*)dat = wc_get_int(val); break;
	case WC_PARAM_INT32:			*(int32_t*)dat = wc_get_int(val); break;
	case WC_PARAM_FLOAT:			*(float*)dat = wc_get_float(val); break;
	case WC_PARAM_RGBA:				*(RGBA*)dat = UTIL_ParseRGBA(val); break;
	case WC_PARAM_RGB:				*(RGB*)dat = UTIL_ParseRGB(val); break;
	case WC_PARAM_VECTOR:			*(Vector*)dat = UTIL_ParseVector(val); break;
	case WC_PARAM_VECTOR_INT8: {
		Vector v = UTIL_ParseVector(val);
		int8_t* cur = (int8_t*)dat;
		cur[0] = v.x;
		cur[1] = v.y;
		cur[2] = v.z;
		break;
	}
	case WC_PARAM_VECTOR_FP_10_6: {
		Vector v = UTIL_ParseVector(val);
		int16_t* cur = (int16_t*)dat;
		cur[0] = FLOAT_TO_FP_10_6(v.x);
		cur[1] = FLOAT_TO_FP_10_6(v.y);
		cur[2] = FLOAT_TO_FP_10_6(v.z);
		break;
	}
	case WC_PARAM_UINT8_ARRAY_8: {
		WepEvtArr8* arr = (WepEvtArr8*)dat;
		vector<string> parts = splitString(val, " ");
		memset(arr->arr, 0, sizeof(arr->arr));
		arr->arrSz = 0;
		for (int i = 0; i < parts.size(); i++) {
			if (parts[i].empty())
				continue;

			if (i >= MAX_WC_RANDOM_SELECTION) {
				ALERT(at_error, "%s (line %d): Too many animations in key '%s' for group '%s'.\n",
					fname, group.lineno, name, group.name.c_str());
				break;
			}

			arr->arr[arr->arrSz++] = atoi(parts[i].c_str());
		}
		break;
	}
	case WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2: {
		if (val) {
			WepEvtArr16* arr = (WepEvtArr16*)dat;

			if (arr->arrSz >= MAX_WC_RANDOM_SELECTION) {
				ALERT(at_error, "%s (line %d): Too many sounds for '%s' for group '%s'.\n",
					fname, group.lineno, name, group.name.c_str());
				break;
			}

			arr->arr[arr->arrSz++] = PRECACHE_SOUND_NULLENT(val);
		}
		break;
	}
	case WC_PARAM_UINT32_FLAGS:
	case WC_PARAM_UINT8_FLAGS: {
		vector<string> words = splitString(val, "+");
		uint32_t flags = 0;
		for (int k = 0; k < words.size(); k++) {
			for (int j = 0; j < field->valNamesSz; j++) {
				if (field->valNames[j] && field->valNames[j] == trimSpaces(words[k])) {
					flags |= 1 << j;
					break;
				}
			}
		}

		if (ptype == WC_PARAM_UINT32_FLAGS)		*(uint32_t*)dat = flags;
		else if (ptype == WC_PARAM_UINT8_FLAGS)	*(uint8_t*)dat = flags;

		break;
	}
	case WC_PARAM_UINT8_ENUM: {
		*(uint8_t*)dat = 0;

		for (int i = 0; i < field->valNamesSz; i++) {
			if (field->valNames[i] && !strcmp(val, field->valNames[i])) {
				*(uint8_t*)dat = i;
				break;
			}
		}

		break;
	}
	case WC_PARAM_SOUND_INDEX:	*(uint16_t*)dat = val ? PRECACHE_SOUND_NULLENT(val) : 0; break;
	case WC_PARAM_MODEL_INDEX:	*(uint16_t*)dat = val ? PRECACHE_MODEL_NULLENT(val) : 0; break;
	case WC_PARAM_DECAL_INDEX:	*(uint8_t*)dat = val ? DECAL_INDEX(val) : 0; break;
	case WC_PARAM_TIME: {
		string sval = val;
		int msSuffix = sval.find("ms");
		int sSuffix = sval.find("s");
		if (msSuffix != -1) {
			*(uint16_t*)dat = atoi(sval.substr(0, msSuffix).c_str()); break;
		}
		else if (sSuffix) {
			*(uint16_t*)dat = atof(sval.substr(0, msSuffix).c_str()) * 1000; break;
		}
		else {
			ALERT(at_error, "%s (line %d) group key '%s' is missing time unit suffix\n",
				fname, group.lineno, name, group.name.c_str());
		}
		break;
	}
	case WC_PARAM_ACCURACY_UINT16_2X: {
		vector<string> parts = splitString(val, " ");
		if (parts.size() > 0)
			((uint16_t*)dat)[0] = FLOAT_TO_SPREAD(UTIL_ConeFromDegrees(atof(parts[0].c_str())).x);

		if (parts.size() > 1)
			((uint16_t*)dat)[1] = FLOAT_TO_SPREAD(UTIL_ConeFromDegrees(atof(parts[1].c_str())).x);
		else
			((uint16_t*)dat)[1] = ((uint16_t*)dat)[0];
		break;
	}
	case WC_PARAM_ACCURACY_100_2X: {
		vector<string> parts = splitString(val, " ");
		if (parts.size() > 0)
			((uint16_t*)dat)[0] = atof(parts[0].c_str()) * 100;

		if (parts.size() > 1)
			((uint16_t*)dat)[1] = atof(parts[1].c_str()) * 100;
		else
			((uint16_t*)dat)[1] = ((uint16_t*)dat)[0];
		break;
	}
	case WC_PARAM_UINT16_PERCENT:	*(uint16_t*)dat = atof(val) ? FLOAT_TO_MOVESPEED_MULT(atof(val)) : 0; break;
	case WC_PARAM_STRING:			*(string_t*)dat = val && val[0] ? ALLOC_STRING(val) : 0; break;
	default:
		ALERT(at_error, "%s (line %d): Unknown param type %d for key '%s' in group '%s'\n",
			fname, group.lineno, ptype, name, group.name.c_str());
	}
}

void wc_fwrite_field(FILE* f, void* dat, const char* name, int ptype, wep_field_desc_t* field) {
	if (ptype != WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2)
		fprintf(f, "%-24s= ", name);

	switch (ptype) {
	case WC_PARAM_UINT8:	fprintf(f, "%u\n", (uint32_t)(*(uint8_t*)dat)); break;
	case WC_PARAM_UINT8_PERCENT:	fprintf(f, "%.2f\n", (*(uint8_t*)dat) / 255.0f); break;
	case WC_PARAM_7BIT_PERCENT:	fprintf(f, "%.2f\n", (*(uint8_t*)dat) / 127.0f); break;
	case WC_PARAM_UINT8_FP_2_6:	fprintf(f, "%.2f\n", ((*(uint8_t*)dat) / 64.0f) + (0.5f / 64.0f)); break;
	case WC_PARAM_UINT16:	fprintf(f, "%u\n", (uint32_t)(*(uint16_t*)dat)); break;
	case WC_PARAM_UINT32:	fprintf(f, "%u\n", (uint32_t)(*(uint32_t*)dat)); break;
	case WC_PARAM_INT8:		fprintf(f, "%d\n", (int32_t)(*(int8_t*)dat)); break;
	case WC_PARAM_INT16:	fprintf(f, "%d\n", (int32_t)(*(int16_t*)dat)); break;
	case WC_PARAM_INT32:	fprintf(f, "%d\n", (int32_t)(*(int32_t*)dat)); break;
	case WC_PARAM_FLOAT:	fprintf(f, "%.2f\n", *(float*)dat); break;
	case WC_PARAM_UINT32_FLAGS:
	case WC_PARAM_UINT8_FLAGS: {
		uint32_t flags = 0;
		if (ptype == WC_PARAM_UINT32_FLAGS)			flags = *(uint32_t*)dat;
		else if (ptype == WC_PARAM_UINT8_FLAGS)	flags = *(uint8_t*)dat;

		int count = 0;
		for (int i = 0; i < field->valNamesSz; i++) {
			if ((flags & (1 << i)) && field->valNames[i] && field->valNames[i][0]) {
				if (count++ != 0)
					fprintf(f, " + ");
				fprintf(f, "%s", field->valNames[i]);
			}
		}
		fprintf(f, "\n");
		break;
	}
	case WC_PARAM_UINT8_ENUM:
		fprintf(f, "%s\n", field->valNames[*(uint8_t*)dat]);
		break;
	case WC_PARAM_RGB: {
		RGB& c = *(RGB*)dat;
		fprintf(f, "%u %u %u\n", (uint32_t)c.r, (uint32_t)c.g, (uint32_t)c.b);
		break;
	}
	case WC_PARAM_RGBA: {
		RGBA& c = *(RGBA*)dat;
		fprintf(f, "%u %u %u %u\n", (uint32_t)c.r, (uint32_t)c.g, (uint32_t)c.b, (uint32_t)c.a);
		break;
	}
	case WC_PARAM_VECTOR: {
		Vector& v = *(Vector*)dat;
		fprintf(f, "%.2f %.2f %.2f\n", v.x, v.y, v.z);
		break;
	}
	case WC_PARAM_VECTOR_INT8: {
		int8_t* v = (int8_t*)dat;
		fprintf(f, "%d %d %d\n", (int)v[0], (int)v[1], (int)v[2]);
		break;
	}
	case WC_PARAM_VECTOR_FP_10_6: {
		int16_t* v = (int16_t*)dat;
		fprintf(f, "%.2f %.2f %.2f\n", FP_10_6_TO_FLOAT(v[0]), FP_10_6_TO_FLOAT(v[1]), FP_10_6_TO_FLOAT(v[2]));
		break;
	}
	case WC_PARAM_UINT8_ARRAY_8: {
		WepEvtArr8* arr = (WepEvtArr8*)dat;
		for (int i = 0; i < MAX_WC_RANDOM_SELECTION && i < arr->arrSz; i++) {
			if (i != 0)
				fprintf(f, " ");
			fprintf(f, "%d", arr->arr[i]);
		}
		fprintf(f, "\n");
		break;
	}
	case WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2: {
		WepEvtArr16* arr = (WepEvtArr16*)dat;
		for (int i = 0; i < MAX_WC_RANDOM_SELECTION && i < arr->arrSz; i++) {
			string keyName = name + to_string(i + 2);
			fprintf(f, "%-24s= %s\n", keyName.c_str(), INDEX_SOUND(arr->arr[i]));
		}
		break;
	}
	case WC_PARAM_SOUND_INDEX:		fprintf(f, "%s\n", INDEX_SOUND(*(uint16_t*)dat)); break;
	case WC_PARAM_MODEL_INDEX:		fprintf(f, "%s\n", INDEX_MODEL(*(uint16_t*)dat)); break;
	case WC_PARAM_DECAL_INDEX:		fprintf(f, "%s\n", get_decal_name(*(uint8_t*)dat)); break;
	case WC_PARAM_TIME:				fprintf(f, "%ums\n", (uint32_t)(*(uint16_t*)dat)); break;
	case WC_PARAM_ACCURACY_UINT16_2X: {
		uint16_t* acc = (uint16_t*)dat;
		if (acc[0] == acc[1])
			fprintf(f, "%.2f\n", DEGREES_FROM_SPREAD(acc[0]));
		else
			fprintf(f, "%.2f %.2f\n", DEGREES_FROM_SPREAD(acc[0]), DEGREES_FROM_SPREAD(acc[1]));
		break;
	}
	case WC_PARAM_ACCURACY_100_2X: {
		uint16_t* acc = (uint16_t*)dat;
		if (acc[0] == acc[1])
			fprintf(f, "%.2f\n", acc[0] / 100.0f);
		else
			fprintf(f, "%.2f %.2f\n", acc[0] / 100.0f, acc[1] / 100.0f);
		break;
	}
	case WC_PARAM_UINT16_PERCENT:	fprintf(f, "%.2f\n", MOVESPEED_MULT_TO_FLOAT(*(uint16_t*)dat)); break;
	case WC_PARAM_STRING:			fprintf(f, "%s\n", STRING(*(string_t*)dat)); break;
	default:
		ALERT(at_error, "%s: Unknown param type %d\n", __func__, ptype);
	}
}

int wc_get_field_bytes(wep_field_desc_t& field) {
	switch (field.type) {
	case WC_PARAM_UINT8:
	case WC_PARAM_UINT8_PERCENT:
	case WC_PARAM_7BIT_PERCENT:
	case WC_PARAM_UINT8_FP_2_6:
	case WC_PARAM_INT8:
	case WC_PARAM_UINT8_FLAGS:
	case WC_PARAM_UINT8_ENUM:
	case WC_PARAM_DECAL_INDEX:
		return 1;
	case WC_PARAM_UINT16:
	case WC_PARAM_INT16:
	case WC_PARAM_SOUND_INDEX:
	case WC_PARAM_MODEL_INDEX:
	case WC_PARAM_TIME:
	case WC_PARAM_UINT16_PERCENT:
	case WC_PARAM_STRING:
		return 2;
	case WC_PARAM_VECTOR_INT8:
	case WC_PARAM_RGB:
		return 3;
	case WC_PARAM_UINT32:
	case WC_PARAM_UINT32_FLAGS:
	case WC_PARAM_INT32:
	case WC_PARAM_FLOAT:
	case WC_PARAM_RGBA:
	case WC_PARAM_ACCURACY_UINT16_2X:
	case WC_PARAM_ACCURACY_100_2X:
		return 4;
	case WC_PARAM_VECTOR_FP_10_6:
		return 6;
	case WC_PARAM_UINT8_ARRAY_8:
		return 9;
	case WC_PARAM_VECTOR:
		return 12;
	case WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2:
		return 17;
	default:
		ALERT(at_error, "Unknown field type size for %d\n", field.type);
		return 0;
	}
}

std::string wc_get_field_str(wep_field_desc_t& field, uint8_t* dat) {
	switch (field.type) {
	case WC_PARAM_UINT8:
	case WC_PARAM_UINT8_PERCENT:
	case WC_PARAM_7BIT_PERCENT:
	case WC_PARAM_INT8:
	case WC_PARAM_UINT8_FLAGS:
	case WC_PARAM_UINT8_ENUM:
	case WC_PARAM_DECAL_INDEX:
		return UTIL_VarArgs("%d", (int)(*dat));
	case WC_PARAM_UINT8_FP_2_6:
		return UTIL_VarArgs("%.2f", FP_10_6_TO_FLOAT(*dat));
	case WC_PARAM_UINT16:
	case WC_PARAM_INT16:
	case WC_PARAM_SOUND_INDEX:
	case WC_PARAM_MODEL_INDEX:
	case WC_PARAM_TIME:
	case WC_PARAM_UINT16_PERCENT:
		return UTIL_VarArgs("%d", (int)(*(uint16_t*)dat));
	case WC_PARAM_VECTOR_INT8:
		return UTIL_VarArgs("(%d %d %d)", (int)((int8_t*)dat)[0], (int)((int8_t*)dat)[1], (int)((int8_t*)dat)[2]);
	case WC_PARAM_RGB:
		return UTIL_VarArgs("(%d %d %d)", (int)dat[0], (int)dat[1], (int)dat[2]);
	case WC_PARAM_UINT32:
	case WC_PARAM_UINT32_FLAGS:
		return UTIL_VarArgs("%u", *(uint32_t*)dat);
	case WC_PARAM_INT32:
		return UTIL_VarArgs("%d", *(int*)dat);
	case WC_PARAM_FLOAT:
		return UTIL_VarArgs("%f", *(float*)dat);
	case WC_PARAM_RGBA:
		return UTIL_VarArgs("(%d %d %d %d)", (int)dat[0], (int)dat[1], (int)dat[2], (int)dat[3]);
	case WC_PARAM_ACCURACY_UINT16_2X:
	case WC_PARAM_ACCURACY_100_2X:
		return UTIL_VarArgs("(%d %d)", (int)((uint16_t*)dat)[0], (int)((uint16_t*)dat)[1]);
	case WC_PARAM_VECTOR_FP_10_6: {
		int16_t* v = (int16_t*)dat;
		return UTIL_VarArgs("(%.2f %.2f %.2f)",
			FP_10_6_TO_FLOAT(v[0]), FP_10_6_TO_FLOAT(v[1]), FP_10_6_TO_FLOAT(v[2]));
	}
	case WC_PARAM_VECTOR: {
		float* v1 = (float*)dat;
		return UTIL_VarArgs("(%f %f %f)", v1[0], v1[1], v1[2]);
	}
	case WC_PARAM_UINT8_ARRAY_8: {
		WepEvtArr8* arr = (WepEvtArr8*)dat;
		return UTIL_VarArgs("(%d sz: %d %d %d %d %d %d %d %d)",
			(int)arr->arrSz,
			(int)arr->arr[0], (int)arr->arr[1], (int)arr->arr[2], (int)arr->arr[3],
			(int)arr->arr[0], (int)arr->arr[1], (int)arr->arr[2], (int)arr->arr[3]);
	}
	case WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2: {
		WepEvtArr16* arr = (WepEvtArr16*)dat;
		return UTIL_VarArgs("(%d sz: %d %d %d %d %d %d %d %d)",
			(int)arr->arrSz,
			(int)arr->arr[0], (int)arr->arr[1], (int)arr->arr[2], (int)arr->arr[3],
			(int)arr->arr[0], (int)arr->arr[1], (int)arr->arr[2], (int)arr->arr[3]);
	}
	case WC_PARAM_STRING:
		return UTIL_VarArgs("'%s'", STRING(*(string_t*)dat));
	default:
		return "???";
	}
}


//
// General parsing utils
//

float wc_get_float(const char* val) {
#ifndef CLIENT_DLL
	skill_cvar_t** skillVal = g_skillCvars.get(val);
	if (skillVal) {
		return (*skillVal)->cvar.value;
	}	
#endif
	return atof(val);
}

int wc_get_int(const char* val) {
#ifndef CLIENT_DLL
	skill_cvar_t** skillVal = g_skillCvars.get(val);
	if (skillVal) {
		return (*skillVal)->cvar.value;
	}
#endif
	return atoi(val);
}

void wc_compare_struct_fields(wep_struct_desc_t& desc, void* struct1, void* struct2, int idx) {
	for (int i = 0; i < desc.numFields; i++) {
		wep_field_desc_t& field = desc.fields[i];
		uint8_t* dat1 = ((uint8_t*)struct1) + field.offset;
		uint8_t* dat2 = ((uint8_t*)struct2) + field.offset;

		bool matches = false;

		switch (field.type) {
		case WC_PARAM_STRING:
			matches = !strcmp(STRING(*(string_t*)dat1), STRING(*(string_t*)dat2));
			break;
		default: {
			int bytes = wc_get_field_bytes(field);
			matches = !memcmp(dat1, dat2, bytes);
			break;
		}
		}

		if (matches)
			continue;

		std::string info = wc_get_field_str(field, dat1) + " != " + wc_get_field_str(field, dat2);

		ALERT(at_error, "Mismatch on field '%s' %s (idx %d)\n",
			field.name, info.c_str(), idx);
	}
}

vector<SettingsGroup> parse_settings_groups(const char* path) {
	vector<SettingsGroup> groups;

	FILE* cfg = fopen(path, "r");

	if (!cfg)
		return groups;

	string group_name;
	StringMap group_keys;

	static const char* commentChars[] = { "//", ";", "#" };

	int group_lineno = 0;
	int lineno = 0;
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), cfg)) {
		string line = buffer;
		lineno++;

		for (int k = 0; k < ARRAY_SZ(commentChars); k++) {
			int comment = line.find(commentChars[k]);
			if (comment != -1) {
				line = line.substr(0, comment);
			}
		}

		line = trimSpaces(line);

		if (line.empty())
			continue;

		if (line[0] == '[') {
			if (group_name.size()) {
				groups.push_back({ group_name, group_keys, group_lineno });
			}

			group_lineno = lineno;
			group_name = line.substr(1, line.size() - 2);
			group_keys.clear();
			continue;
		}

		vector<string> parts = splitString(line, "=");

		if (parts.size() == 1) {
			ALERT(at_error, "%s (line %d): key is missing value.\n", path, lineno);
			continue;
		}
		else if (parts.size() != 2) {
			ALERT(at_error, "%s (line %d): config line has more than one '=' symbol.\n", path, lineno);
			continue;
		}

		string key = trimSpaces(parts[0]);
		string val = trimSpaces(parts[1]);
		group_keys.put(key.c_str(), val.c_str());
	}

	if (group_name.size()) {
		groups.push_back({ group_name, group_keys });
	}

	fclose(cfg);

	return groups;
}

wep_struct_desc_t* get_evt_desc(int idx) {
	if (idx >= WC_EVT_TOTAL) {
		ALERT(at_error, "Invalid event type %d\n", idx);
		return NULL;
	}

	wep_struct_desc_t* desc = &g_wc_desc_evt[idx];

	if (!desc->name) {
		ALERT(at_error, "unimplemented event writer for %d\n", idx);
		return NULL;
	}

	return desc;
}

bool wc_check_default_dat(wep_field_desc_t& field, uint8_t* dat) {
	if (field.flags & FL_FIELD_ALWAYS_WRITE_CFG)
		return false; // always written

	static char defaultDat[128];
	memset(&defaultDat, 0, sizeof(defaultDat));

	static SettingsGroup dummyGroup;

	wc_read_field("null", dummyGroup, defaultDat, field.name, field.defaultValue, field.type, &field);

	if (field.type == WC_PARAM_STRING)
		return !strcmp(STRING(*(string_t*)dat), STRING(*(string_t*)defaultDat));

	int bytes = wc_get_field_bytes(field);
	return !memcmp(dat, defaultDat, bytes);
}

void wc_fwrite_wep_struct_fields(FILE* f, void* dat, wep_struct_desc_t& desc) {
	for (int i = 0; i < desc.numFields; i++) {
		wep_field_desc_t& field = desc.fields[i];
		uint8_t* fieldDat = ((uint8_t*)dat) + field.offset;

		if (field.flags & FL_FIELD_NO_CFG)
			continue;

		if (wc_check_default_dat(field, fieldDat))
			continue;

		if (field.type == WC_PARAM_UINT8_FLAGS || field.type == WC_PARAM_UINT32_FLAGS) {
			bool anyFlagsToWrite = false;
			uint32_t flags = 0;
			
			if (field.type == WC_PARAM_UINT32_FLAGS)		flags = *(uint32_t*)fieldDat;
			else if (field.type == WC_PARAM_UINT8_FLAGS)	flags = *fieldDat;

			for (int i = 0; i < field.valNamesSz; i++) {
				if ((flags & (1 << i)) && field.valNames[i] && field.valNames[i][0]) {
					anyFlagsToWrite = true;
					break;
				}
			}

			if (!anyFlagsToWrite)
				continue;
		}

		wc_fwrite_field(f, fieldDat, field.name, field.type, &field);
	}
}

void wc_read_wep_struct(const char* fname, SettingsGroup& group, void* dat, wep_struct_desc_t& desc) {
	for (int i = 0; i < desc.numFields; i++) {
		wep_field_desc_t& field = desc.fields[i];
		uint8_t* fieldDat = ((uint8_t*)dat) + field.offset;

		if (field.defaultValue)
			wc_read_field(fname, group, fieldDat, field.name, field.defaultValue, field.type, &field);

		if (field.type == WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2) {
			for (int k = 0; k < MAX_WC_RANDOM_SELECTION; k++) {
				string key_name = field.name + to_string(k + 2);
				const char* val = group.keys.get(key_name.c_str());
				if (val)
					wc_read_field(fname, group, fieldDat, key_name.c_str(), val, field.type, &field);
			}
		}
		else {
			const char* val = group.keys.get(field.name);
			if (val)
				wc_read_field(fname, group, fieldDat, field.name, val, field.type, &field);
		}
	}
}

int wc_get_event_category(int evt) {	
	switch (evt) {
	default:
		return WC_EVT_CATEGORY_UNKNOWN;

	case WC_TRIG_TERTIARY:
		return WC_EVT_CATEGORY_TERTIARY;

	case WC_TRIG_PRIMARY_ALT:
		return WC_EVT_CATEGORY_PRIMARY_ALT;

	case WC_TRIG_PRIMARY:
	case WC_TRIG_PRIMARY_CLIPSIZE:
	case WC_TRIG_PRIMARY_CLIP_SP:
	case WC_TRIG_PRIMARY_CHARGE:
	case WC_TRIG_PRIMARY_OVERCHARGE:
	case WC_TRIG_PRIMARY_START:
	case WC_TRIG_PRIMARY_STOP:
	case WC_TRIG_PRIMARY_FAIL:
	
		return WC_EVT_CATEGORY_PRIMARY;

	case WC_TRIG_SECONDARY:
	case WC_TRIG_SECONDARY_CLIPSIZE:
	case WC_TRIG_SECONDARY_CLIP_SP:
	case WC_TRIG_SECONDARY_CHARGE:
	case WC_TRIG_SECONDARY_OVERCHARGE:
	case WC_TRIG_SECONDARY_START:
	case WC_TRIG_SECONDARY_STOP:
	case WC_TRIG_SECONDARY_FAIL:
		return WC_EVT_CATEGORY_SECONDARY;

	case WC_TRIG_RELOAD:
	case WC_TRIG_RELOAD_EMPTY:
	case WC_TRIG_RELOAD_NOT_EMPTY:
	case WC_TRIG_RELOAD_FINISH:
		return WC_EVT_CATEGORY_RELOAD;

	case WC_TRIG_DEPLOY:
		return WC_EVT_CATEGORY_DEPLOY;

	case WC_TRIG_LASER_ON:
	case WC_TRIG_LASER_OFF:
	case WC_TRIG_ZOOM_IN:
	case WC_TRIG_ZOOM_OUT:
		return WC_EVT_CATEGORY_STATE_CHANGE;

	case WC_TRIG_BULLET_FIRED:
	case WC_TRIG_IMPACT:
		return WC_EVT_CATEGORY_REACTION;
	}
}

void wc_fwrite_events(FILE* f, CustomWeaponParams& params, int category) {	
	for (int i = 0; i < params.numEvents; i++) {
		WepEvt& evt = params.events[i];

		if (category != -1 && category != wc_get_event_category(evt.trigger)) {
			continue;
		}

		uint16_t key = (evt.triggerArg << 5) | evt.trigger;
		if (key > ARRAY_SZ(g_wc_trigger_to_name) || g_wc_trigger_to_name[key].pool == NULL) {
			ALERT(at_error, "Invalid trigger key value %d\n", key);
			continue;
		}

		fprintf(f, "\n[event.%s.%s]\n", g_wc_trigger_to_name[key].str(), g_wc_evt_type_names[evt.evtType]);

		if (evt.delay)
			fprintf(f, "%-24s= %ums\n", "delay", (uint32_t)evt.delay);

		wep_struct_desc_t* desc = get_evt_desc(evt.evtType);

		if (desc) {
			wc_fwrite_wep_struct_fields(f, &evt, *desc);
		}
	}
}

void wc_parse_event(const char* path, CustomWeaponParams& params, SettingsGroup& group) {
	vector<string> header_parts = splitString(group.name, ".");
	if (header_parts.size() != 3) {
		ALERT(at_error, "%s (line %d): Malformed event header '%s'.\n",
			path, group.lineno, group.name.c_str());
		return;
	}

	uint16_t* val = g_wc_name_to_trigger.get(header_parts[1].c_str());
	const char* action = header_parts[2].c_str();
	const char* delay = group.keys.get("delay");

	if (!val) {
		ALERT(at_error, "%s (line %d): Invalid event type '%s'.\n",
			path, group.lineno, group.name.c_str());
		return;
	}
	if (!action) {
		ALERT(at_error, "%s (line %d): '%s' missing 'action' key.\n",
			path, group.lineno, group.name.c_str());
		return;
	}

	uint8_t* actionId = g_wc_name_to_action.get(action);
	if (!actionId) {
		ALERT(at_error, "%s (line %d): invalid event action value for group '%s'.\n",
			path, group.lineno, group.name.c_str());
		return;
	}

	WepEvt evt;
	memset(&evt, 0, sizeof(WepEvt));

	evt.evtType = *actionId;
	evt.trigger = *val & 0x1f;
	evt.triggerArg = *val >> 5;
	evt.delay = delay ? atoi(delay) : 0;
	evt.hasDelay = evt.delay != 0;

	StringMap& kv = group.keys;

	wep_struct_desc_t* desc = get_evt_desc(evt.evtType);

	if (!desc) {
		return;
	}

	wc_read_wep_struct(path, group, &evt, *desc);

	switch (evt.evtType) {
	case WC_EVT_PLAY_SOUND: {
		// use smaller event if most params are default
		if (evt.playSound.channel == CHAN_STATIC && evt.playSound.aiVol == 0 && evt.playSound.distantSound == 0
			&& evt.playSound.attn == ATTN_IDLE * 64 && evt.playSound.pitchMin == 100 && evt.playSound.pitchMax == 100
			&& evt.playSound.flags == 0 && evt.playSound.additionalSounds.arrSz == 0)
		{
			uint16_t soundIdx = evt.playSound.sound;
			uint8_t newVol = evt.playSound.volume / 2;
			memset(&evt.playSound, 0, sizeof(evt.playSound));
			evt.idleSound.sound = soundIdx;
			evt.idleSound.volume = newVol;
			evt.evtType = WC_EVT_IDLE_SOUND;
		}

		break;
	}
	case WC_EVT_EJECT_SHELL:
		evt.ejectShell.hasVel = evt.ejectShell.vel[0] || evt.ejectShell.vel[1] || evt.ejectShell.vel[2];
		evt.ejectShell.hasRand = evt.ejectShell.dirRand || evt.ejectShell.speedRand;
		break;
	case WC_EVT_BEAM: {
		evt.beam.hasImpactSprite = evt.beam.impactSprite != 0;
		break;
	}
	}

	params.events[params.numEvents++] = evt;
}

void wc_compare_params(CustomWeaponParams& a, CustomWeaponParams& b) {
	uint8_t* dat1 = (uint8_t*)&a;
	uint8_t* dat2 = (uint8_t*)&b;

	wc_compare_struct_fields(g_wc_desc_general, &a, &b, 0);
	wc_compare_struct_fields(g_wc_desc_akimbo, dat1, dat2, 0);
	wc_compare_struct_fields(g_wc_desc_laser, dat1, dat2, 0);

	for (int i = 0; i < ARRAY_SZ(a.reloadStage); i++) {
		int offset = sizeof(WeaponCustomReload) * i;
		wc_compare_struct_fields(g_wc_desc_reload, dat1 + offset, dat2 + offset, i);
	}

	for (int i = 0; i < ARRAY_SZ(a.idles); i++) {
		int offset = sizeof(WeaponCustomIdle) * i;
		wc_compare_struct_fields(g_wc_desc_idle, dat1 + offset, dat2 + offset, i);
	}

	for (int i = 0; i < ARRAY_SZ(a.shootOpts); i++) {
		wc_compare_struct_fields(g_wc_desc_shoot_opts, &a, &b, i);
	}

	for (int i = 0; i < ARRAY_SZ(a.akimbo.idles); i++) {
		int offset = sizeof(WeaponCustomIdle) * i;
		wc_compare_struct_fields(g_wc_desc_akimbo_idle, dat1 + offset, dat2 + offset, i);
	}

	for (int i = 0; i < ARRAY_SZ(a.laser.idles); i++) {
		int offset = sizeof(WeaponCustomIdle) * i;
		wc_compare_struct_fields(g_wc_desc_laser_idle, dat1 + offset, dat2 + offset, i);
	}

	if (a.numEvents != b.numEvents)
		ALERT(at_error, "Mismatch event count %d != %d\n", a.numEvents, b.numEvents);

	for (int i = 0; i < a.numEvents && i < b.numEvents; i++) {
		WepEvt& e1 = a.events[i];
		WepEvt& e2 = b.events[i];

		if (e1.evtType != e2.evtType) {
			ALERT(at_error, "Mismatch event type %d != %d (idx %d)\n", e1.evtType, e2.evtType, i);
		}
		if (e1.trigger != e2.trigger || e1.triggerArg != e2.triggerArg) {
			ALERT(at_error, "Mismatch event trigger (%d %d) != (%d %d) (idx %d)\n",
				e1.trigger, e2.trigger, e1.triggerArg, e2.triggerArg, i);
		}
		if (e1.delay != e2.delay || e1.hasDelay != e2.hasDelay) {
			ALERT(at_error, "Mismatch event delay (%d %d) != (%d %d) (idx %d)\n",
				e1.delay, e2.delay, e1.hasDelay, e2.hasDelay, i);
		}

		// this field doesn't matter
		e2.attackIdx = e1.attackIdx;

		if (memcmp(&e1, &e2, sizeof(WepEvt))) {
			ALERT(at_error, "Mismatch data on event %d (%s)\n", i, g_wc_evt_type_names[e1.evtType]);

			wep_struct_desc_t* desc = get_evt_desc(e1.evtType);

			if (desc)
				wc_compare_struct_fields(*desc, &e1, &e2, i);
		}
	}
}

void wc_read_shoot_opts(const char* path, SettingsGroup& group, CustomWeaponParams& params, int idx) {
	uint8_t* dat = ((uint8_t*)&params) + sizeof(CustomWeaponShootOpts) * idx;
	wc_read_wep_struct(path, group, dat, g_wc_desc_shoot_opts);

	static MeleeOpts emptyMelee;
	if (memcmp(&params.shootOpts[idx].melee, &emptyMelee, sizeof(MeleeOpts))) {
		params.shootOpts[idx].flags |= FL_WC_SHOOT_IS_MELEE;
	}
}

void write_section_header(FILE* f, const char* name) {
	fprintf(f, "\n\n\n; --------------------------------\n; %s\n; --------------------------------\n", name);
}

void wc_fwrite_weapon_settings(FILE* cfg, CustomWeaponParams& params, bool prettyPrint) {
	uint8_t* dat = (uint8_t*)&params;

	fprintf(cfg, "[%s]\n", g_wc_desc_general.name);
	wc_fwrite_wep_struct_fields(cfg, &params, g_wc_desc_general);

	static const char* ammoNames[4] = { "primary_ammo", "secondary_ammo" };
	for (int k = 0; k < ARRAY_SZ(params.ammoInfo); k++) {
		if (!params.ammoInfo[k].type)
			continue;

		fprintf(cfg, "\n[%s]\n", ammoNames[k]);
		wc_fwrite_wep_struct_fields(cfg, dat + sizeof(WeaponCustomAmmoInfo) * k, g_wc_desc_ammo);
	}
	

	if (params.flags & FL_WC_WEP_AKIMBO) {
		fprintf(cfg, "\n[%s]\n", g_wc_desc_akimbo.name);
		wc_fwrite_wep_struct_fields(cfg, &params, g_wc_desc_akimbo);
	}

	if (params.flags & FL_WC_WEP_HAS_LASER) {
		fprintf(cfg, "\n[%s]\n", g_wc_desc_laser.name);
		wc_fwrite_wep_struct_fields(cfg, &params, g_wc_desc_laser);
	}

	for (int k = 0; k < ARRAY_SZ(params.idles); k++) {
		if (params.idles[k].time == 0)
			continue;

		fprintf(cfg, "\n[%s]\n", g_wc_desc_idle.name);
		wc_fwrite_wep_struct_fields(cfg, dat + sizeof(WeaponCustomIdle) * k, g_wc_desc_idle);
	}

	if (params.flags & FL_WC_WEP_AKIMBO) {
		for (int k = 0; k < ARRAY_SZ(params.akimbo.idles); k++) {
			if (params.akimbo.idles[k].time == 0)
				continue;

			fprintf(cfg, "\n[%s]\n", g_wc_desc_akimbo_idle.name);
			wc_fwrite_wep_struct_fields(cfg, dat + sizeof(WeaponCustomIdle) * k, g_wc_desc_akimbo_idle);
		}
	}

	if (params.flags & FL_WC_WEP_HAS_LASER) {
		for (int k = 0; k < ARRAY_SZ(params.laser.idles); k++) {
			if (params.laser.idles[k].time == 0)
				continue;

			fprintf(cfg, "\n[%s]\n", g_wc_desc_laser_idle.name);
			wc_fwrite_wep_struct_fields(cfg, dat + sizeof(WeaponCustomIdle) * k, g_wc_desc_laser_idle);
		}
	}

	static int attackOrder[4] = { 0, 3, 1, 2 };
	static int optBits[4] = { FL_WC_WEP_HAS_PRIMARY, FL_WC_WEP_HAS_SECONDARY, FL_WC_WEP_HAS_TERTIARY, FL_WC_WEP_HAS_ALT_PRIMARY };
	static const char* optNames[4] = { "primary_attack", "secondary_attack", "tertiary_attack", "primary_alt_attack" };
	static int optEventCats[4] = { WC_EVT_CATEGORY_PRIMARY, WC_EVT_CATEGORY_SECONDARY, WC_EVT_CATEGORY_TERTIARY, WC_EVT_CATEGORY_PRIMARY_ALT };

	for (int k = 0; k < ARRAY_SZ(params.shootOpts); k++) {
		int idx = attackOrder[k];

		if (!(params.flags & optBits[idx]))
			continue;

		CustomWeaponShootOpts& opts = params.shootOpts[idx];

		if (prettyPrint)
			write_section_header(cfg, g_wc_evt_category_names[optEventCats[idx]]);

		fprintf(cfg, "\n[%s]\n", optNames[idx]);
		wc_fwrite_wep_struct_fields(cfg, dat + sizeof(CustomWeaponShootOpts) * idx, g_wc_desc_shoot_opts);

		if (prettyPrint)
			wc_fwrite_events(cfg, params, optEventCats[idx]);
	}

	bool hasReload = false;
	for (int k = 0; k < ARRAY_SZ(params.reloadStage); k++) {
		if (params.reloadStage[k].time != 0) {
			hasReload = true;
			break;
		}
	}

	if (hasReload) {
		if (prettyPrint)
			write_section_header(cfg, "Reload");
		const char* reloadNames[3] = { "reload", "reload_empty", "reload_pump" };
		for (int k = 0; k < ARRAY_SZ(params.reloadStage); k++) {
			const char* section = reloadNames[k];

			if ((params.flags & FL_WC_WEP_SHOTGUN_RELOAD) && k == 1) {
				section = "reload_shell";
			}

			if (params.reloadStage[k].time == 0)
				continue;

			fprintf(cfg, "\n[%s]\n", section);
			wc_fwrite_wep_struct_fields(cfg, dat + sizeof(WeaponCustomReload) * k, g_wc_desc_reload);
		}

		if (params.flags & FL_WC_WEP_AKIMBO) {
			fprintf(cfg, "\n[%s]\n", g_wc_desc_akimbo_reload.name);
			wc_fwrite_wep_struct_fields(cfg, &params, g_wc_desc_akimbo_reload);
		}

		if (prettyPrint)
			wc_fwrite_events(cfg, params, WC_EVT_CATEGORY_RELOAD);
	}
}

bool UTIL_ParseCustomWeaponConfig(const char* path, CustomWeaponParams& params) {
	const char* relPath = path;
	string searchPath = string("weapons/") + path;
	string fpath = getGameFilePath(searchPath.c_str(), false);

	memset(&params, 0, sizeof(CustomWeaponParams));

	if (!fileExists(fpath.c_str())) {
		ALERT(at_error, "Weapon config not found: '%s'\n", searchPath.c_str());
		return false;
	}

	uint64_t lastModified = getFileModifiedTime(fpath.c_str());

	WeaponConfigCache* cache = g_customWeaponCache.get(relPath);
	if (cache) {
		if (lastModified == cache->fileModifiedTime) {
			params = cache->params;
			return true;
		}
		else {
			ALERT(at_logged, "Reloading modified weapon config: %s\n", relPath);
			g_customWeaponCache.del(relPath);
		}		
	}

	path = fpath.c_str();

	ALERT(at_console, "Parsing weapon config: %s\n", path);

	vector<SettingsGroup> groups = parse_settings_groups(path);

	if (!groups.size())
		return false;

	int idleCount = 0;
	int akimboIdleCount = 0;
	int laserIdleCount = 0;

	uint8_t* dat = (uint8_t*)&params;

	for (SettingsGroup& group : groups) {
		if (group.name == "general") {
			wc_read_wep_struct(path, group, &params, g_wc_desc_general);
		}
		else if (group.name == "primary_ammo") {
			wc_read_wep_struct(path, group, dat + sizeof(WeaponCustomAmmoInfo) * 0, g_wc_desc_ammo);
		}
		else if (group.name == "secondary_ammo") {
			wc_read_wep_struct(path, group, dat + sizeof(WeaponCustomAmmoInfo) * 1, g_wc_desc_ammo);
		}
		else if (group.name == "reload") {
			wc_read_wep_struct(path, group, dat + sizeof(WeaponCustomReload) * 0, g_wc_desc_reload);
		}
		else if (group.name == "reload_empty") {
			wc_read_wep_struct(path, group, dat + sizeof(WeaponCustomReload) * 1, g_wc_desc_reload);
		}
		else if (group.name == "reload_shell") {
			params.flags |= FL_WC_WEP_SHOTGUN_RELOAD;
			wc_read_wep_struct(path, group, dat + sizeof(WeaponCustomReload) * 1, g_wc_desc_reload);
		}
		else if (group.name == "reload_pump") {
			params.flags |= FL_WC_WEP_SHOTGUN_RELOAD;
			wc_read_wep_struct(path, group, dat + sizeof(WeaponCustomReload) * 2, g_wc_desc_reload);
		}
		else if (group.name == "idle") {
			int maxIdles = ARRAY_SZ(params.idles);
			if (idleCount >= maxIdles) {
				ALERT(at_error, "%s (line %d): Too many [idle] sections (max is %d).\n",
					path, group.lineno, maxIdles);
				continue;
			}
			wc_read_wep_struct(path, group, dat + sizeof(WeaponCustomIdle) * idleCount, g_wc_desc_idle);
			idleCount++;
		}
		else if (group.name == "primary_attack") {
			params.flags |= FL_WC_WEP_HAS_PRIMARY;
			wc_read_shoot_opts(path, group, params, 0);
		}
		else if (group.name == "secondary_attack") {
			params.flags |= FL_WC_WEP_HAS_SECONDARY;
			wc_read_shoot_opts(path, group, params, 1);
		}
		else if (group.name == "tertiary_attack") {
			params.flags |= FL_WC_WEP_HAS_TERTIARY;
			wc_read_shoot_opts(path, group, params, 2);
		}
		else if (group.name == "primary_alt_attack") {
			params.flags |= FL_WC_WEP_HAS_ALT_PRIMARY;
			wc_read_shoot_opts(path, group, params, 3);
		}
		else if (group.name == "akimbo") {
			params.flags |= FL_WC_WEP_AKIMBO;
			wc_read_wep_struct(path, group, &params, g_wc_desc_akimbo);
		}
		else if (group.name == "idle_akimbo") {
			int maxIdles = ARRAY_SZ(params.akimbo.idles);
			if (akimboIdleCount >= maxIdles) {
				ALERT(at_error, "%s (line %d): Too many [idle_akimbo] sections (max is %d).\n",
					path, group.lineno, maxIdles);
				continue;
			}
			wc_read_wep_struct(path, group, dat + sizeof(WeaponCustomIdle) * akimboIdleCount, g_wc_desc_akimbo_idle);
			akimboIdleCount++;
		}
		else if (group.name == "reload_akimbo") {
			wc_read_wep_struct(path, group, &params, g_wc_desc_akimbo_reload);
		}
		else if (group.name == "laser") {
			params.flags |= FL_WC_WEP_HAS_LASER;
			wc_read_wep_struct(path, group, &params, g_wc_desc_laser);
		}
		else if (group.name == "idle_laser") {
			int maxIdles = ARRAY_SZ(params.laser.idles);
			if (laserIdleCount >= maxIdles) {
				ALERT(at_error, "%s (line %d): Too many [idle_laser] sections (max is %d).\n",
					path, group.lineno, maxIdles);
				continue;
			}
			wc_read_wep_struct(path, group, dat + sizeof(WeaponCustomIdle) * laserIdleCount, g_wc_desc_laser_idle);
			laserIdleCount++;
		}
		else if (group.name.find("event.") == 0) {
			wc_parse_event(path, params, group);
		}
	}

	WeaponConfigCache newCache;
	newCache.fileModifiedTime = lastModified;
	newCache.params = params;
	g_customWeaponCache.put(relPath, newCache);

	return true;
}

void UTIL_DumpCustomWeaponConfig(const char* path, CustomWeaponParams& params, bool prettyPrint) {
	string fname = string("/weapons/") + path;

	FILE* cfg = UTIL_OpenFile(fname.c_str(), "w");

	if (!cfg)
		return;

	wc_fwrite_weapon_settings(cfg, params, prettyPrint);

	if (prettyPrint) {
		for (int k = 0; k < WC_EVT_CATEGORY_TOTAL; k++) {
			bool alreadyWritten = true;
			switch (k) {
			case WC_EVT_CATEGORY_DEPLOY:
			case WC_EVT_CATEGORY_REACTION:
			case WC_EVT_CATEGORY_STATE_CHANGE:
			case WC_EVT_CATEGORY_UNKNOWN:
				alreadyWritten = false;
				break;
			}

			if (alreadyWritten)
				continue;

			bool anyEvents = false;
			for (int i = 0; i < params.numEvents; i++) {
				WepEvt& evt = params.events[i];
				if (k == wc_get_event_category(evt.trigger)) {
					anyEvents = true;
					break;
				}
			}

			if (!anyEvents)
				continue;

			write_section_header(cfg, g_wc_evt_category_names[k]);
			wc_fwrite_events(cfg, params, k);
		}
	}
	else {
		// write events in the exact order
		write_section_header(cfg, "Events");
		wc_fwrite_events(cfg, params, -1);
	}

	fclose(cfg);
}

void UTIL_TestConfig(CWeaponCustom* wep) {
	CustomWeaponParams& wepParams = wep->params;
	UTIL_DumpCustomWeaponConfig(UTIL_VarArgs("test/%s.txt", STRING(wep->pev->classname)), wepParams, false);

	CustomWeaponParams cfgParams;
	UTIL_ParseCustomWeaponConfig(UTIL_VarArgs("test/%s.txt", STRING(wep->pev->classname)), cfgParams);

	wc_compare_params(wepParams, cfgParams);
}


//
// Functions for reading/writing prediction network messages
//

uint32_t wc_get_bits(void* dat, int bits) {
	uint32_t mask = ((1 << bits) - 1);

	if (bits > 16) {
		return *((uint32_t*)dat) & mask;
	}
	else if (bits > 8) {
		return *((uint16_t*)dat) & mask;
	}
	return *((uint8_t*)dat) & mask;
}

int wc_send_netmsg_struct(wep_struct_desc_t& desc, void* dat) {
	int byteCount = 0;

#ifndef CLIENT_DLL
	uint32_t packedFields = 0;
	int packedBitCount = 0;
	ALERT(at_aiconsole, "Send net struct '%s' (%d fields)\n", desc.name, desc.numFields);

	for (int i = 0; i < desc.numFields; i++) {
		wep_field_desc_t& field = desc.fields[i];
		uint8_t* fieldDat = ((uint8_t*)dat) + field.offset;

		if (field.flags & FL_FIELD_NO_NETWORK) {
			ALERT(at_aiconsole, "    %-20s = (skipped server-side field)\n", field.name);
			continue;
		}

		if (field.conditionByteOfs) {
			uint8_t* condDat = ((uint8_t*)dat) + field.conditionByteOfs;
			if (*condDat == 0) {
				ALERT(at_aiconsole, "    %-20s = (skipped by condition byte %d)\n", field.name, (int)field.conditionByteOfs);
				continue;
			}
		}

		int bytes = wc_get_field_bytes(field);
		bool isBitPacked = field.bits != 0 && field.bits != bytes * 8;

		if (g_developer->value) {
			string fieldStr = wc_get_field_str(field, fieldDat);
			if (isBitPacked) {
				ALERT(at_aiconsole, "    %-20s = %-24s (%d bits)\n", field.name, fieldStr.c_str(), field.bits);
			}
			else {
				ALERT(at_aiconsole, "    %-20s = %-24s (%d bytes)\n", field.name, fieldStr.c_str(), bytes);
			}
		}

		if (isBitPacked) {
			uint32_t appendVal = wc_get_bits(fieldDat, field.bits);
			packedFields |= appendVal << packedBitCount;
			packedBitCount += field.bits;

			if (packedBitCount % 8 == 0) {
				bytes = packedBitCount / 8;
				WRITE_BYTES((uint8_t*)&packedFields, bytes);
				byteCount += bytes;
				packedFields = 0;
				packedBitCount = 0;
			}
			continue;
		}
		
		if (packedBitCount != 0) {
			ALERT(at_error, "bit packed fields not byte aligned for %s at %s\n", desc.name, field.name);
			return byteCount;
		}
		switch (field.type) {
		case WC_PARAM_STRING:
			WRITE_STRING(STRING(*(string_t*)dat));
			byteCount += 1 + strlen(STRING(*(string_t*)dat));
			break;
		case WC_PARAM_UINT8_ARRAY_8: {
			WepEvtArr8* arr = (WepEvtArr8*)fieldDat;
			WRITE_BYTE(arr->arrSz);
			WRITE_BYTES(arr->arr, arr->arrSz);
			byteCount += 1 + arr->arrSz;
			break;
		}
		case WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2: {
			WepEvtArr16* arr = (WepEvtArr16*)fieldDat;
			WRITE_BYTE(arr->arrSz);
			WRITE_BYTES((uint8_t*)arr->arr, arr->arrSz * sizeof(uint16_t));
			byteCount += 1 + arr->arrSz * sizeof(uint16_t);
			break;
		}
		default:
			WRITE_BYTES(fieldDat, bytes);
			byteCount += bytes;
			break;
		}
	}

	if (packedBitCount) {
		ALERT(at_error, "bit packed fields not byte aligned for %s\n", desc.name);
	}

	ALERT(at_aiconsole, "Wrote %d bytes\n\n", byteCount);

#endif
	return byteCount;
}

void READ_BYTES(uint8_t* bytes, int count) {
#ifdef CLIENT_DLL
	int longCount = count / 4;
	int byteCount = count % 4;

	int32_t* longPtr = (int32_t*)bytes;
	for (int i = 0; i < longCount; i++) {
		*longPtr = READ_LONG();
		longPtr++;
	}

	uint8_t* bytePtr = bytes + longCount * 4;
	for (int i = 0; i < byteCount; i++) {
		*bytePtr = READ_BYTE();
		bytePtr++;
	}
#endif
}

void wc_read_netmsg_struct(wep_struct_desc_t& desc, void* dat) {
#ifdef CLIENT_DLL
	int packedBitCount = 0;
	int bitPackFieldStartIdx = -1;
	int byteCount = 0;
	bool verbose = gHUD.m_Debug.m_HUD_debug->value;
	if (verbose) PRINTD("\nParse net struct '%s' (%d fields)\n", desc.name, desc.numFields);

	for (int i = 0; i < desc.numFields; i++) {
		wep_field_desc_t& field = desc.fields[i];
		uint8_t* fieldDat = ((uint8_t*)dat) + field.offset;

		if (field.flags & FL_FIELD_NO_NETWORK) {
			if (verbose) PRINTD("    %-20s = (skipped server-side field)\n", field.name);
			continue;
		}

		if (field.conditionByteOfs) {
			uint8_t* condDat = ((uint8_t*)dat) + field.conditionByteOfs;
			if (*condDat == 0) {
				if (verbose) PRINTD("    %-20s = (skipped by condition byte %d)\n", field.name, (int)field.conditionByteOfs);
				continue;
			}
		}

		int bytes = wc_get_field_bytes(field);
		bool isBitPacked = field.bits != 0 && field.bits != bytes * 8;

		// read bit-packed fields into a buffer until a regular field is read
		if (isBitPacked) {
			if (bitPackFieldStartIdx < 0)
				bitPackFieldStartIdx = i;

			packedBitCount += field.bits;

			if (packedBitCount % 8 != 0) {
				continue; // keep reading packed fields until byte alignment
			}
		}
		
		if (packedBitCount) {
			if (packedBitCount % 8 == 0) {
				uint32_t packedFields = 0;
				if (packedBitCount == 32) {
					packedFields = (uint32_t)READ_LONG();
					byteCount += 4;
				}
				else if (packedBitCount == 16) {
					packedFields = (uint32_t)READ_SHORT();
					byteCount += 2;
				}
				else if (packedBitCount == 8) {
					packedFields = (uint32_t)READ_BYTE();
					byteCount += 1;
				}
				else {
					PRINTF("Invalid bit byte alignment %d\n", packedBitCount);
					return;
				}

				for (int k = bitPackFieldStartIdx; k <= i; k++) {
					wep_field_desc_t& packedField = desc.fields[k];
					uint8_t* packedFieldDat = ((uint8_t*)dat) + packedField.offset;

					// read bit packed data out of the buffer
					uint64_t val = packedFields & ((1ULL << packedField.bits) - 1ULL);

					if (field.bits > 16)
						*(uint32_t*)packedFieldDat = (uint32_t)val;
					else if (field.bits > 8)
						*(uint16_t*)packedFieldDat = (uint16_t)val;
					else
						*(uint8_t*)packedFieldDat = (uint8_t)val;

					if (verbose) {
						string fieldStr = wc_get_field_str(field, fieldDat);
						PRINTD("    %-20s = %-24s (%d bits)\n", field.name, fieldStr.c_str(), field.bits);
					}

					packedFields >>= packedField.bits;
					packedBitCount -= packedField.bits;
				}

				packedFields = 0;
				packedBitCount = 0;
				bitPackFieldStartIdx = -1;
				continue;
			}
			else {
				PRINTF("bit packed fields not byte aligned for %s\n", desc.name);
				return;
			}
		}

		switch (field.type) {
		case WC_PARAM_STRING: {
			const char* temp = READ_STRING();
			byteCount += strlen(temp) + 1;
			PRINTF("String parsing not implemented\n");
			break;
		}
		case WC_PARAM_UINT8_ARRAY_8: {
			uint8_t sz = READ_BYTE();
			*fieldDat = sz;
			READ_BYTES(fieldDat + 1, sz);
			byteCount += 1 + sz;
			break;
		}
		case WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2: {
			uint8_t sz = READ_BYTE();
			*fieldDat = sz;
			READ_BYTES(fieldDat + 1, sz*sizeof(uint16_t));
			byteCount += 1 + sz * sizeof(uint16_t);
			break;
		}
		default:
			READ_BYTES(fieldDat, bytes);
			byteCount += bytes;
			break;
		}

		if (verbose) {
			string fieldStr = wc_get_field_str(field, fieldDat);
			PRINTD("    %-20s = %-24s (%d bytes)\n", field.name, fieldStr.c_str(), bytes);
		}
	}

	if (verbose)
		PRINTD("Read %d bytes\n", byteCount);
#endif
}

bool should_send_event(int evtId) {
	switch (evtId) {
	case WC_EVT_PROJECTILE:
	case WC_EVT_SERVER:
		return false;
	}

	return true;
}

void UTIL_SendCustomWeaponPredictionData(edict_t* target, CWeaponCustom* wep, PredictionDataSendMode sendMode) {
#ifndef CLIENT_DLL
	CustomWeaponParams& params = wep->params;
	uint8_t* dat = (uint8_t*)&params;

	if (params.flags & FL_WC_WEP_NO_PREDICTION) {
		return;
	}

	if (UTIL_HasCustomWeaponPredictionData(target, wep) && sendMode == WC_PRED_SEND_INIT) {
		//ALERT(at_console, "PLayer already has the prediction data\n");
		return;
	}

	params.maxClip = params.ammoInfo[0].maxClip;

	if (sendMode != WC_PRED_SEND_EVT) {
		MESSAGE_BEGIN(MSG_ONE, gmsgCustomWeapon, NULL, target);
		WRITE_BYTE(wep->m_iId);

		wc_send_netmsg_struct(g_wc_desc_general, dat);

		// TODO: For next client update
		//for (int k = 0; k < 2; k++) {
		//	wc_send_netmsg_struct(g_wc_desc_ammo, dat + sizeof(WeaponCustomAmmoInfo) * k);
		//}

		for (int k = 0; k < 3; k++) {
			wc_send_netmsg_struct(g_wc_desc_reload, dat + sizeof(WeaponCustomReload) * k);

			if (k == 2 && !(params.flags & FL_WC_WEP_SHOTGUN_RELOAD))
				break;
		}

		for (int k = 0; k < 4; k++) {
			wc_send_netmsg_struct(g_wc_desc_idle, dat + sizeof(WeaponCustomIdle) * k);
		}

		if (params.flags & FL_WC_WEP_AKIMBO) {
			for (int k = 0; k < 4; k++) {
				wc_send_netmsg_struct(g_wc_desc_akimbo_idle, dat + sizeof(WeaponCustomIdle) * k);
			}

			wc_send_netmsg_struct(g_wc_desc_akimbo_reload, dat);
			wc_send_netmsg_struct(g_wc_desc_akimbo, dat);
		}

		if (params.flags & FL_WC_WEP_HAS_LASER) {
			for (int k = 0; k < 4; k++) {
				wc_send_netmsg_struct(g_wc_desc_laser_idle, dat + sizeof(WeaponCustomIdle) * k);
			}

			wc_send_netmsg_struct(g_wc_desc_laser, dat);
		}

		for (int k = 0; k < 4; k++) {
			if (!(params.flags & FL_WC_WEP_HAS_PRIMARY) && k == 0)
				continue;
			if (!(params.flags & FL_WC_WEP_HAS_SECONDARY) && k == 1)
				continue;
			if (!(params.flags & FL_WC_WEP_HAS_TERTIARY) && k == 2)
				continue;
			if (!(params.flags & FL_WC_WEP_HAS_ALT_PRIMARY) && k == 3)
				continue;

			wc_send_netmsg_struct(g_wc_desc_shoot_opts, dat + sizeof(CustomWeaponShootOpts) * k);
		}
		MESSAGE_END();
	}

	int mainBytes = LastMsgSize();

	if (sendMode != WC_PRED_SEND_WEP) {
		MESSAGE_BEGIN(MSG_ONE, gmsgCustomWeaponEvents, NULL, target);
		WRITE_BYTE(wep->m_iId);

		int sendEvents = 0;
		for (int k = 0; k < params.numEvents; k++) {
			WepEvt& evt = params.events[k];

			wep_struct_desc_t* desc = get_evt_desc(evt.evtType);
			if (desc && should_send_event(evt.evtType))
				sendEvents++;
		}

		WRITE_BYTE(sendEvents);

		for (int k = 0; k < params.numEvents; k++) {
			WepEvt& evt = params.events[k];

			if (!should_send_event(evt.evtType))
				continue;

			wep_struct_desc_t* desc = get_evt_desc(evt.evtType);
			if (!desc)
				continue;
			
			uint16_t packedHeader = (evt.hasDelay << 15) | (evt.triggerArg << 10) | (evt.trigger << 5) | evt.evtType;
			WRITE_SHORT(packedHeader);
			if (evt.hasDelay) {
				WRITE_SHORT(evt.delay);
			}

			const char* evtName = evt.evtType < WC_EVT_TOTAL ? g_wc_evt_type_names[evt.evtType] : "???";
			ALERT(at_aiconsole, "Write event %s (%d header bytes)\n",
				evtName, evt.hasDelay ? 4 : 2);

			wc_send_netmsg_struct(*desc, &evt);
		}
		MESSAGE_END();
	}

	int evBytes = LastMsgSize();
	ALERT(at_console, "Sent %d prediction bytes for %s (%d + %d evt)\n",
		evBytes + mainBytes, STRING(wep->pev->classname), mainBytes, evBytes);

	g_wcPredDataSent[wep->m_iId] |= PLRBIT(target);
#endif
}

int UTIL_ReadCustomWeaponPredictionData(const char* pszName, int iSize, void* pbuf) {
#ifdef CLIENT_DLL
	BEGIN_READ(pbuf, iSize);

	int weaponId = READ_BYTE();

	if (weaponId < 0 || weaponId >= MAX_WEAPONS)
		return 0;


	InitCustomWeapon(weaponId);
	CustomWeaponParams& parms = *GetCustomWeaponParams(weaponId);
	memset(&parms, 0, sizeof(CustomWeaponParams));
	uint8_t* dat = (uint8_t*)&parms;

	wc_read_netmsg_struct(g_wc_desc_general, dat);

	// TODO: For next client update
	//for (int k = 0; k < 2; k++) {
	//	wc_read_netmsg_struct(g_wc_desc_ammo, dat + sizeof(WeaponCustomAmmoInfo) * k);
	//}
	parms.ammoInfo[0].maxClip = parms.maxClip;

	for (int k = 0; k < 3; k++) {
		wc_read_netmsg_struct(g_wc_desc_reload, dat + sizeof(WeaponCustomReload)*k);

		if (k == 2 && !(parms.flags & FL_WC_WEP_SHOTGUN_RELOAD))
			break;
	}

	for (int k = 0; k < 4; k++) {
		wc_read_netmsg_struct(g_wc_desc_idle, dat + sizeof(WeaponCustomIdle) * k);
	}

	if (parms.flags & FL_WC_WEP_AKIMBO) {
		for (int k = 0; k < 4; k++) {
			wc_read_netmsg_struct(g_wc_desc_akimbo_idle, dat + sizeof(WeaponCustomIdle) * k);
		}

		wc_read_netmsg_struct(g_wc_desc_akimbo_reload, dat);
		wc_read_netmsg_struct(g_wc_desc_akimbo, dat);
	}

	if (parms.flags & FL_WC_WEP_HAS_LASER) {
		for (int k = 0; k < 4; k++) {
			wc_read_netmsg_struct(g_wc_desc_laser_idle, dat + sizeof(WeaponCustomIdle) * k);
		}

		wc_read_netmsg_struct(g_wc_desc_laser, dat);
	}

	for (int i = 0; i < 4; i++) {
		if (!(parms.flags & FL_WC_WEP_HAS_PRIMARY) && i == 0)
			continue;
		if (!(parms.flags & FL_WC_WEP_HAS_SECONDARY) && i == 1)
			continue;
		if (!(parms.flags & FL_WC_WEP_HAS_TERTIARY) && i == 2)
			continue;
		if (!(parms.flags & FL_WC_WEP_HAS_ALT_PRIMARY) && i == 3)
			continue;

		wc_read_netmsg_struct(g_wc_desc_shoot_opts, dat + sizeof(CustomWeaponShootOpts) * i);
	}

#endif
	return 1;
}

int UTIL_ReadCustomWeaponPredictionEventData(const char* pszName, int iSize, void* pbuf) {
#ifdef CLIENT_DLL
	BEGIN_READ(pbuf, iSize);

	int weaponId = READ_BYTE();

	if (weaponId < 0 || weaponId >= MAX_WEAPONS)
		return 0;

	CustomWeaponParams& parms = *GetCustomWeaponParams(weaponId);

	parms.numEvents = READ_BYTE();
	if (parms.numEvents >= MAX_WC_EVENTS) {
		return 0;
	}

	for (int i = 0; i < parms.numEvents; i++) {
		uint16_t packedHeader = READ_SHORT();
		WepEvt& evt = parms.events[i];
		memset(&evt, 0, sizeof(WepEvt));

		evt.evtType = packedHeader & 0x1F;
		evt.trigger = (packedHeader >> 5) & 0x1F;
		evt.triggerArg = (packedHeader >> 10) & 0x1F;
		evt.hasDelay = packedHeader >> 15;

		if (evt.hasDelay)
			evt.delay = READ_SHORT();

		wep_struct_desc_t* desc = get_evt_desc(evt.evtType);
		if (!desc) {
			PRINTF("Bad custom weapon event type %d\n", (int)evt.evtType);
			continue;
		}

		wc_read_netmsg_struct(*desc, &evt);
	}
#endif
	return 1;
}

bool UTIL_HasCustomWeaponPredictionData(edict_t* target, CWeaponCustom* wep) {
	return g_wcPredDataSent[wep->m_iId] & PLRBIT(target);
}
