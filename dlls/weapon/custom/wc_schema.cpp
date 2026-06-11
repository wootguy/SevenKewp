#include "extdll.h"
#include "util.h"
#include "wc_params.h"
#include "wc_schema.h"
#include "StringPool.h"
#include "CWeaponCustom.h"
#include "CProjectileCustom.h"

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

#define AMMO_FIELD(name, default_val, struct_field, bits, field_type, ...) \
	{ name, default_val, offsetof(CustomAmmoParams, struct_field), bits, field_type, ##__VA_ARGS__}
#define AMMO__ENUM(name, default_val, struct_field, bits, enum_arr, ...) \
	{ name, default_val, offsetof(CustomAmmoParams, struct_field), bits, WC_PARAM_UINT8_ENUM, enum_arr, ARRAY_SZ(enum_arr), ##__VA_ARGS__}

#define EVT_DESC(id, cfgName, ...) do { \
		g_wc_evt_type_names[id] = cfgName; \
        static field_desc_t fields[] = { __VA_ARGS__ }; \
        g_wc_desc_evt[id] = { cfgName, fields, ARRAY_SZ(fields) }; \
    } while (0)

#define WEP_STRUCT_DESC(var, cfgName, ...) do { \
        static field_desc_t fields[] = { __VA_ARGS__ }; \
        var = { cfgName, fields, ARRAY_SZ(fields) }; \
    } while (0)

struct_desc_t g_wc_desc_general;
struct_desc_t g_wc_desc_ammo;
struct_desc_t g_wc_desc_reload;
struct_desc_t g_wc_desc_akimbo_reload;
struct_desc_t g_wc_desc_akimbo;
struct_desc_t g_wc_desc_shoot_opts;
struct_desc_t g_wc_desc_laser;
struct_desc_t g_wc_desc_evt[WC_EVT_TOTAL];

struct_desc_t g_wc_desc_custom_ammo;

const char* g_wc_evt_trigger_names[128];
const char* g_wc_evt_trigger_arg_primary_names[32];
const char* g_wc_evt_trigger_clip_sp_names[32];
const char* g_wc_evt_trigger_impact_names[32];
const char* g_wc_evt_trigger_arg_idle_names[32];
const char* g_wc_evt_trigger_arg_deploy_names[32];
const char* g_wc_evt_charge_names[32];
const char* g_wc_evt_category_names[32];
static const char* g_wc_dmgFlags[32];

const char* g_wc_evt_type_names[128];
mod_string_t g_wc_trigger_to_name[WC_MAX_TRIGGER_VALUES];

HashMap<uint16_t> g_wc_name_to_trigger; // maps a group name to an event trigger + argument value
HashMap<uint8_t> g_wc_name_to_action; // maps an action key value to its event number
StringPool g_wc_trigger_string_pool;

#ifdef CLIENT_DLL
#include "../cl_dll/hud.h"
#endif

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

//
// Field definitions for all custom weapon structures
// Update this whenever fields/events/enums/flags are changed
//

void init_weapon_struct_fields() {
	{
		static const char* wep_flags[32];
		//wep_flags[BitIndex(FL_WC_WEP_HAS_PRIMARY)] = "has_primary";
		//wep_flags[BitIndex(FL_WC_WEP_HAS_SECONDARY)] = "has_secondary";
		//wep_flags[BitIndex(FL_WC_WEP_HAS_TERTIARY)] = "has_tertiary";
		//wep_flags[BitIndex(FL_WC_WEP_HAS_ALT_PRIMARY)] = "has_alt_primary";
		//wep_flags[BitIndex(FL_WC_WEP_SHOTGUN_RELOAD)] = "shotgun_reload";
		wep_flags[BitIndex(FL_WC_WEP_UNLINK_COOLDOWNS)] = "unlink_cooldowns";
		//wep_flags[BitIndex(FL_WC_WEP_AKIMBO)] = "akimbo";
		wep_flags[BitIndex(FL_WC_WEP_LINK_CHARGEUPS)] = "link_chargeups";
		wep_flags[BitIndex(FL_WC_WEP_PRIMARY_PRIORITY)] = "primary_priority";
		wep_flags[BitIndex(FL_WC_WEP_EXCLUSIVE_HOLD)] = "exclusive_hold";
		wep_flags[BitIndex(FL_WC_WEP_USE_ONLY)] = "use_only";
		//wep_flags[BitIndex(FL_WC_WEP_HAS_LASER)] = "has_laser";
		wep_flags[BitIndex(FL_WC_WEP_DYNAMIC_ACCURACY)] = "dynamic_accuracy";
		wep_flags[BitIndex(FL_WC_WEP_ZOOM_SPR_STRETCH)] = "strech_zoom_sprite";
		wep_flags[BitIndex(FL_WC_WEP_ZOOM_SPR_ASPECT)] = "keep_zoom_sprite_aspect";
		wep_flags[BitIndex(FL_WC_WEP_NO_PREDICTION)] = "no_prediction";
		wep_flags[BitIndex(FL_WC_WEP_HIDE_SECONDARY_AMMO)] = "hide_secondary_ammo";
		wep_flags[BitIndex(FL_WC_WEP_FORCE_ZOOM_SPRITE)] = "force_zoom_sprite";
		wep_flags[BitIndex(FL_WC_WEP_HAND_MODELS)] = "has_hand_models";
		wep_flags[BitIndex(FL_WC_WEP_ALLOW_HL)] = "hl_client_can_use";
		wep_flags[BitIndex(FL_WC_WEP_NO_AUTOSWITCHEMPTY)] = "no_autoswitch_empty";
		wep_flags[BitIndex(FL_WC_WEP_NO_AUTORELOAD)] = "no_autoreload";
		wep_flags[BitIndex(FL_WC_WEP_SELECTONEMPTY)] = "select_on_empty";
		wep_flags[BitIndex(FL_WC_WEP_EXHAUSITBLE)] = "exhaustible";
		wep_flags[BitIndex(FL_WC_WEP_IRON_SIGHTS_ZOOM)] = "iron_sights";

		WEP_STRUCT_DESC(g_wc_desc_general, "general",
			WEP_FIELD("classname", "", classname, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("hl_client_classname", "", wrongClientWeapon, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("kill_feed_icon", "", killFeedIcon, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("display_name", "", displayName, 0, WC_PARAM_STRING, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FLAGS32("flags", "0", flags, 0, wep_flags),
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

	{
		static const char* flags[32];
		flags[BitIndex(FL_WC_SHOOT_UNDERWATER)] = "works_underwater";
		flags[BitIndex(FL_WC_SHOOT_NO_ATTACK)] = "not_an_attack";
		flags[BitIndex(FL_WC_SHOOT_COOLDOWN_IDLE)] = "always_cooldown_idle";
		flags[BitIndex(FL_WC_SHOOT_NEED_AKIMBO)] = "akimbo_only";
		flags[BitIndex(FL_WC_SHOOT_NEED_FULL_COST)] = "need_full_cost";
		flags[BitIndex(FL_WC_SHOOT_NO_AUTOFIRE)] = "no_autofire";
		//flags[BitIndex(FL_WC_SHOOT_IS_MELEE)] = "is_melee";

		static const char* ammoPools[32];
		ammoPools[WC_AMMOPOOL_DEFAULT] = "default";
		ammoPools[WC_AMMOPOOL_PRIMARY_CLIP] = "primary_clip";
		ammoPools[WC_AMMOPOOL_PRIMARY_RESERVE] = "primary_reserve";
		ammoPools[WC_AMMOPOOL_SECONDARY_RESERVE] = "secondary_reserve";

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
		chargeFlags[BitIndex(FL_WC_CHARGE_DAMAGE)] = "scale_damage";
		chargeFlags[BitIndex(FL_WC_CHARGE_KICKBACK)] = "scale_kickback";

		WEP_STRUCT_DESC(g_wc_desc_shoot_opts, "unnamed_attack",
			WEP_FLAGS("flags", "0", shootOpts[0].flags, 0, flags),
			WEP_FIELD("ammo_cost", "0", shootOpts[0].ammoCost, 0, WC_PARAM_UINT8),
			WEP_FIELD("ammo_freq", "0", shootOpts[0].ammoFreq, 0, WC_PARAM_UINT8),
			WEP__ENUM("ammo_pool", "0", shootOpts[0].ammoPool, 0, ammoPools),
			WEP_FIELD("cooldown", "0", shootOpts[0].cooldown, 0, WC_PARAM_TIME),
			WEP_FIELD("cooldown_fail", "0", shootOpts[0].cooldownFail, 0, WC_PARAM_TIME),
			WEP_FIELD("cooldown_water", "0", shootOpts[0].cooldownWater, 0, WC_PARAM_TIME),
			WEP_FIELD("accuracy", "0", shootOpts[0].accuracy, 0, WC_PARAM_ACCURACY_100_2X),
			WEP_FIELD("empty_sound", NULL, shootOpts[0].emptySound, 0, WC_PARAM_SOUND_INDEX),

			WEP__ENUM("charge_mode", "0", shootOpts[0].chargeMode, 4, chargeModes),
			WEP__ENUM("charge_ammo_mode", "0", shootOpts[0].chargeAmmoMode, 2, chargeAmmoModes),
			WEP__ENUM("overcharge_mode", "0", shootOpts[0].overchargeMode, 2, overchargeModes),

			WEP_FLAGS("charge_flags", "0", shootOpts[0].chargeFlags, 0, chargeFlags, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			WEP_FIELD("charge_time", "0", shootOpts[0].chargeTime, 0, WC_PARAM_TIME, NULL, 0, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			WEP_FIELD("charge_down_time", "0", shootOpts[0].chargeDownTime, 0, WC_PARAM_TIME, NULL, 0, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			WEP_FIELD("charge_shoot_time", "0", shootOpts[0].minChargeShootTime, 0, WC_PARAM_TIME, NULL, 0, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			WEP_FIELD("overcharge_time", "0", shootOpts[0].overchargeTime, 0, WC_PARAM_TIME, NULL, 0, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			WEP_FIELD("charge_cancel_time", "0", shootOpts[0].chargeCancelTime, 0, WC_PARAM_TIME, NULL, 0, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),
			WEP_FIELD("discharged_cooldown", "0", shootOpts[0].dischargedCooldown, 0, WC_PARAM_TIME, NULL, 0, 0, WEP_COND_BYTE(shootOpts[0].chargeMode)),

			WEP_FIELD("charge_move_speed", "0", shootOpts[0].chargeMoveSpeedMult, 0, WC_PARAM_UINT16_PERCENT, NULL, 0, FL_FIELD_NO_NETWORK),
			
			WEP_FIELD("accuracy_mult_fly", "3.0", shootOpts[0].accuracyMult[WC_ACCURACY_MULT_FLY], 0, WC_PARAM_UINT16_FP_4_12),
			WEP_FIELD("accuracy_mult_swim", "3.0", shootOpts[0].accuracyMult[WC_ACCURACY_MULT_SWIM], 0, WC_PARAM_UINT16_FP_4_12),
			WEP_FIELD("accuracy_mult_float", "2.0", shootOpts[0].accuracyMult[WC_ACCURACY_MULT_FLOAT], 0, WC_PARAM_UINT16_FP_4_12),
			WEP_FIELD("accuracy_mult_run", "2.0", shootOpts[0].accuracyMult[WC_ACCURACY_MULT_RUN], 0, WC_PARAM_UINT16_FP_4_12),
			WEP_FIELD("accuracy_mult_walk", "1.0", shootOpts[0].accuracyMult[WC_ACCURACY_MULT_WALK], 0, WC_PARAM_UINT16_FP_4_12),
			WEP_FIELD("accuracy_mult_crawl", "0.5", shootOpts[0].accuracyMult[WC_ACCURACY_MULT_CRAWL], 0, WC_PARAM_UINT16_FP_4_12),
			WEP_FIELD("accuracy_mult_duck", "0.5", shootOpts[0].accuracyMult[WC_ACCURACY_MULT_DUCK], 0, WC_PARAM_UINT16_FP_4_12),
			WEP_FIELD("accuracy_mult_zoom", "1.0", shootOpts[0].accuracyMult[WC_ACCURACY_MULT_ZOOM], 0, WC_PARAM_UINT16_FP_4_12),

			WEP_FIELD("melee_damage", "0", shootOpts[0].melee.damage, 0, WC_PARAM_INT32, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FLAGS32("melee_damage_type", "0", shootOpts[0].melee.damageBits, 0, g_wc_dmgFlags, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_range", "0", shootOpts[0].melee.range, 0, WC_PARAM_INT32, NULL, 0, FL_FIELD_NO_NETWORK),
			WEP_FIELD("melee_attack_offset", "0 0 0", shootOpts[0].melee.attackOffset, 0, WC_PARAM_VECTOR, NULL, 0, FL_FIELD_NO_NETWORK),
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

	EVT_DESC(WC_EVT_IDLE_SOUND, "idle_sound",
		EVT_FIELD("volume", "1.0", idleSound.volume, 7, WC_PARAM_7BIT_PERCENT),
		EVT_FIELD("sound", NULL, idleSound.sound, 9, WC_PARAM_SOUND_INDEX),
	);
	g_wc_evt_type_names[WC_EVT_IDLE_SOUND] = "sound"; // written to config as "sound"

	{
		static const char* distant_sound_names[8];
		distant_sound_names[DISTANT_NONE] = "none";
		distant_sound_names[DISTANT_9MM] = "distant_9mm";
		distant_sound_names[DISTANT_357] = "distant_357";
		distant_sound_names[DISTANT_556] = "distant_556";
		distant_sound_names[DISTANT_BOOM] = "distant_boom";

		static const char* flags[32];
		flags[BitIndex(FL_WC_SOUND_CHARGE_PITCH)] = "chargeup_pitch";

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
			EVT__ENUM("sound", "shell", ejectShell.sound, 2, shell_sound_names),
			EVT_FIELD("model", NULL, ejectShell.model, 12, WC_PARAM_MODEL_INDEX),
			EVT_FIELD("position", "0 0 0", ejectShell.position, 0, WC_PARAM_VECTOR_INT8),
			EVT_FIELD("velocity", "0 0 0", ejectShell.vel, 0, WC_PARAM_VECTOR_INT8, NULL, 0, 0, EVT_COND_BYTE(ejectShell.hasVel)),
			EVT_FIELD("direction_randomness", "0", ejectShell.dirRand, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(ejectShell.hasRand)),
			EVT_FIELD("speed_randomness", "0", ejectShell.speedRand, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(ejectShell.hasRand)),
		);
	}

	{
		static const char* angleOps[8];
		angleOps[WC_RECOIL_ANGLES_COPY] = "copy";
		angleOps[WC_RECOIL_ANGLES_RANDOM] = "random";
		angleOps[WC_RECOIL_ANGLES_RANDOM_SIMPLE] = "random_simple";
		angleOps[WC_RECOIL_ANGLES_RANDOM_RANGE] = "random_range";
		angleOps[WC_RECOIL_ANGLES_RANDOM_RANGE_SIMPLE] = "random_range_simple";
		angleOps[WC_RECOIL_ANGLES_LINEAR_RAMP] = "random_linear_ramp";

		static const char* viewOps[8];
		viewOps[WC_RECOIL_APPLY_PUNCH_SET] = "punch";
		viewOps[WC_RECOIL_APPLY_PUNCH_ADD] = "punch_stack";
		viewOps[WC_RECOIL_APPLY_ROTATE] = "rotate";

		static const char* flags[8];
		flags[BitIndex(FL_WC_RECOIL_DUCK)] = "only_when_ducking";
		flags[BitIndex(FL_WC_RECOIL_STAND)] = "only_when_standing";

		EVT_DESC(WC_EVT_RECOIL, "recoil",
			EVT__ENUM("angle_mode", "copy", recoil.angleOp, 3, angleOps),
			EVT__ENUM("view_mode", "punch", recoil.viewOp, 2, viewOps),
			EVT_FLAGS("flags", "0", recoil.flags, 2, flags),
			EVT_FIELD("has_max_angles", "0", recoil.hasMaxAngles, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("angles", "0 0 0", recoil.angles, 0, WC_PARAM_VECTOR_SFP_9_7),
			EVT_FIELD("max_angles_time", "0", recoil.maxAngleTime, 0, WC_PARAM_TIME),
			EVT_FIELD("max_angles", "0 0 0", recoil.maxAngles, 0, WC_PARAM_VECTOR_SFP_9_7, NULL, 0, 0, EVT_COND_BYTE(recoil.hasMaxAngles)),
		);

		EVT_DESC(WC_EVT_RECOIL_ADV, "recoil_advanced",
			EVT__ENUM("angle_mode_x", "copy", recoilAdv.angleOp[0], 0, angleOps, FL_FIELD_NO_NETWORK),
			EVT__ENUM("angle_mode_y", "copy", recoilAdv.angleOp[1], 0, angleOps, FL_FIELD_NO_NETWORK),
			EVT__ENUM("angle_mode_z", "copy", recoilAdv.angleOp[2], 0, angleOps, FL_FIELD_NO_NETWORK),
			EVT__ENUM("view_mode_x", "punch", recoilAdv.viewOp[0], 0, viewOps, FL_FIELD_NO_NETWORK),
			EVT__ENUM("view_mode_y", "punch", recoilAdv.viewOp[1], 0, viewOps, FL_FIELD_NO_NETWORK),
			EVT__ENUM("view_mode_z", "punch", recoilAdv.viewOp[2], 0, viewOps, FL_FIELD_NO_NETWORK),
			
			EVT_FIELD("bitpacked_x", "0", recoilAdv.ops[0], 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("bitpacked_y", "0", recoilAdv.ops[1], 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("bitpacked_z", "0", recoilAdv.ops[2], 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FLAGS("flags", "0", recoilAdv.flags, 0, flags),
			EVT_FIELD("min", "0 0 0", recoilAdv.min, 0, WC_PARAM_VECTOR_SFP_9_7),
			EVT_FIELD("max", "0 0 0", recoilAdv.max, 0, WC_PARAM_VECTOR_SFP_9_7),
			EVT_FIELD("max_angles_time", "0", recoilAdv.maxAngleTime, 0, WC_PARAM_TIME),
		);
	}

	EVT_DESC(WC_EVT_SET_BODY, "set_weapon_body",
		EVT_FIELD("new_body", "0", setBody.newBody, 0, WC_PARAM_UINT8),
	);

	{
		static const char* flags[8];
		flags[BitIndex(FL_WC_ANIM_NO_RESET)] = "no_reset";
		flags[BitIndex(FL_WC_ANIM_PMODEL)] = "thirdperson_model";
		flags[BitIndex(FL_WC_ANIM_ORDERED)] = "play_in_order";

		static const char* hand_names[8];
		hand_names[WC_ANIM_BOTH_HANDS] = "both";
		hand_names[WC_ANIM_LEFT_HAND] = "left";
		hand_names[WC_ANIM_RIGHT_HAND] = "right";
		hand_names[WC_ANIM_TRIG_HAND] = "trigger";

		EVT_DESC(WC_EVT_WEP_ANIM, "weapon_anim",
			EVT_FLAGS("flags", "0", anim.flags, 3, flags),
			EVT_FIELD("has_cooldown", "0", anim.hasCooldown, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("has_weights", "0", anim.hasWeights, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT__ENUM("hand", "both", anim.akimbo, 3, hand_names),
			EVT_FIELD("anims", "0", anim.anims, 0, WC_PARAM_UINT8_ARRAY_8, NULL, 0, FL_FIELD_ALWAYS_WRITE_CFG),
			EVT_FIELD("weights", "0", anim.weights, 0, WC_PARAM_UINT8_ARRAY_8, NULL, 0, 0, EVT_COND_BYTE(anim.hasWeights)),
			EVT_FIELD("cooldown", "0", anim.cooldown, 0, WC_PARAM_TIME, NULL, 0, 0, EVT_COND_BYTE(anim.hasCooldown)),
		);
	}

	{
		static const char* flags[8];
		flags[BitIndex(FL_WC_BULLETS_DYNAMIC_SPREAD)] = "dynamic_spread";
		flags[BitIndex(FL_WC_BULLETS_NO_DECAL)] = "no_decal";
		flags[BitIndex(FL_WC_BULLETS_NO_SOUND)] = "no_sound";

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
		flags[BitIndex(FL_WC_BEAM_SPIRAL)] = "spiral";
		flags[BitIndex(FL_WC_BEAM_OPAQUE)] = "opaque";
		flags[BitIndex(FL_WC_BEAM_SHADEIN)] = "fade_in";
		flags[BitIndex(FL_WC_BEAM_SHADEOUT)] = "fade_out";

		static const char* anim_names[32];
		anim_names[WC_BEAM_ANIM_DISABLED] = "disabled";
		anim_names[WC_BEAM_ANIM_TOGGLE] = "toggle";
		anim_names[WC_BEAM_ANIM_LINEAR] = "linear";
		anim_names[WC_BEAM_ANIM_LINEAR_TOGGLE] = "linear_toggle";
		anim_names[WC_BEAM_ANIM_EASE_IN_OUT] = "ease_in_out";

		EVT_DESC(WC_EVT_BEAM, "beam",
			EVT_FLAGS("flags", "0", beam.flags, 0, flags),
			EVT_FIELD("attachment", "0", beam.attachment, 3, WC_PARAM_UINT8),
			EVT_FIELD("has_rico_beams", "0", beam.hasRicoBeams, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("sprite", NULL, beam.sprite, 12, WC_PARAM_MODEL_INDEX),

			EVT_FIELD("richochet_limit", "0", beam.ricoBeams, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(beam.hasRicoBeams)),
			EVT_FIELD("richochet_angle", "0", beam.ricoAngle, 0, WC_PARAM_ACCURACY_UINT16, NULL, 0, 0, EVT_COND_BYTE(beam.hasRicoBeams)),

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
		flags[BitIndex(FL_WC_PROJ_NO_BUBBLES)] = "no_bubbles";
		flags[BitIndex(FL_WC_PROJ_NO_ORIENT)] = "static_angles";
		flags[BitIndex(FL_WC_PROJ_IS_HOOK)] = "is_hook";

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

		static const char* render_modes[32];
		render_modes[kRenderNormal] = "normal";
		render_modes[kRenderTransColor] = "color";
		render_modes[kRenderTransTexture] = "texture";
		render_modes[kRenderGlow] = "glow";
		render_modes[kRenderTransAlpha] = "alpha";
		render_modes[kRenderTransAdd] = "additive";

		static const char* render_fx[32];
		render_fx[kRenderFxNone] = "none";
		render_fx[kRenderFxPulseSlow] = "pulse_slow";
		render_fx[kRenderFxPulseFast] = "pulse_fast";
		render_fx[kRenderFxPulseSlowWide] = "pulse_slow_wide";
		render_fx[kRenderFxPulseFastWide] = "pulse_fast_wide";
		render_fx[kRenderFxFadeSlow] = "fade_slow";
		render_fx[kRenderFxFadeFast] = "fade_fast";
		render_fx[kRenderFxSolidSlow] = "strobe_slow";
		render_fx[kRenderFxStrobeFast] = "strobe_fast";
		render_fx[kRenderFxStrobeFaster] = "strobe_faster";
		render_fx[kRenderFxFlickerSlow] = "flicker_slow";
		render_fx[kRenderFxFlickerFast] = "flicker_fast";
		render_fx[kRenderFxNoDissipation] = "no_dissipation";
		render_fx[kRenderFxDistort] = "distort";
		render_fx[kRenderFxHologram] = "hologram";
		render_fx[kRenderFxDeadPlayer] = "dead_player";
		render_fx[kRenderFxExplode] = "explode";
		render_fx[kRenderFxGlowShell] = "glow_shell";
		render_fx[kRenderFxClampMinScale] = "clamp_min_scale";
		render_fx[kRenderFxLightMultiplier] = "light_multiplier";

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
			EVT_FIELD("direction", "0 0 1", proj.dir, 0, WC_PARAM_VECTOR),
			EVT_FIELD("model", NULL, proj.model, 0, WC_PARAM_MODEL_INDEX),
			EVT__ENUM("rendermode", "normal", proj.renderMode, 0, render_modes),
			EVT_FIELD("renderamt", "0", proj.renderAmt, 0, WC_PARAM_UINT8),
			EVT__ENUM("renderfx", "none", proj.renderFx, 0, render_fx),
			EVT_FIELD("scale", "0", proj.scale, 0, WC_PARAM_FLOAT),
			EVT_FIELD("framerate", "0", proj.framerate, 0, WC_PARAM_FLOAT),
			EVT_FIELD("move_sound", "", proj.move_snd, 0, WC_PARAM_STRING),
			EVT_FIELD("damage", "0", proj.damage, 0, WC_PARAM_UINT16),
			EVT_FLAGS("damage_type", "0", proj.damageBits, 0, g_wc_dmgFlags),
			EVT_FIELD("sprite", "", proj.sprite, 0, WC_PARAM_STRING),
			EVT_FIELD("sprite_color", "0 0 0 0", proj.sprite_color, 0, WC_PARAM_RGBA),
			EVT_FIELD("angles", "0 0 0", proj.angles, 0, WC_PARAM_VECTOR),
			EVT_FIELD("angular_velocity", "0 0 0", proj.avel, 0, WC_PARAM_VECTOR),
			EVT_FIELD("position", "0 0 0", proj.position, 0, WC_PARAM_VECTOR),
			EVT_FIELD("player_vel_inf", "0 0 0", proj.player_vel_inf, 0, WC_PARAM_VECTOR),
			EVT_FIELD("follow_mode", "0", proj.follow_mode, 0, WC_PARAM_UINT8),
			EVT_FIELD("follow_radius", "0", proj.follow_radius, 0, WC_PARAM_FLOAT),
			EVT_FIELD("follow_time", "0 0 0", proj.follow_time, 0, WC_PARAM_VECTOR),
			EVT_FIELD("trail_sprite", NULL, proj.trail_spr, 0, WC_PARAM_MODEL_INDEX),
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
		state_names[BitIndex(FL_WC_STATE_PRIMARY_ALT)] = "primary_alt";
		state_names[BitIndex(FL_WC_STATE_LASER)] = "laser";
		state_names[BitIndex(FL_WC_STATE_IS_AKIMBO)] = "akimbo";
		state_names[BitIndex(FL_WC_STATE_CAN_AKIMBO)] = "can_akimbo";

		EVT_DESC(WC_EVT_TOGGLE_STATE, "toggle_state",
			EVT__ENUM("toggle_mode", "toggle", toggleState.toggleMode, 2, mode_names),
			EVT_FLAGS("toggled_states", "0", toggleState.stateBits, 14, state_names),
		);
	}

	EVT_DESC(WC_EVT_TOGGLE_ZOOM, "toggle_zoom",
		EVT_FIELD("zoom_fov", "0", zoomToggle.zoomFov, 0, WC_PARAM_UINT8),
		EVT_FIELD("zoom_fov2", "0", zoomToggle.zoomFov2, 0, WC_PARAM_UINT8),
	);

	EVT_DESC(WC_EVT_HIDE_LASER, "hide_laser",
		EVT_FIELD("time", "0", laserHide.millis, 0, WC_PARAM_TIME),
	);

	{
		static const char* targets[32];
		targets[BitIndex(FL_WC_COOLDOWN_PRIMARY)] = "primary";
		targets[BitIndex(FL_WC_COOLDOWN_SECONDARY)] = "secondary";
		targets[BitIndex(FL_WC_COOLDOWN_TERTIARY)] = "tertiary";
		targets[BitIndex(FL_WC_COOLDOWN_IDLE)] = "idle";

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

	EVT_DESC(WC_EVT_SPRITETRAIL, "sprite_trail",
		EVT_FIELD("sprite", NULL, spriteTrail.sprite, 0, WC_PARAM_MODEL_INDEX),
		EVT_FIELD("count", "0", spriteTrail.count, 0, WC_PARAM_UINT8),
		EVT_FIELD("scale", "0", spriteTrail.scale, 0, WC_PARAM_UINT8),
		EVT_FIELD("speed", "0", spriteTrail.speed, 0, WC_PARAM_UINT8),
		EVT_FIELD("speed_randomness", "0", spriteTrail.speedNoise, 0, WC_PARAM_UINT8),
	);

	{
		static const char* flags[32];
		flags[BitIndex(1)] = "particles";

		EVT_DESC(WC_EVT_DECAL, "decal",
			EVT_FIELD("texture", "0", decal.decalIdx, 0, WC_PARAM_DECAL_INDEX),
			EVT_FLAGS("flags", "0", decal.flags, 0, flags),
		);
	}

	EVT_DESC(WC_EVT_RADIUS_DAMAGE, "radius_damage",
		EVT_FIELD("radius", "0", radiusDamage.radius, 0, WC_PARAM_UINT16),
		EVT_FIELD("damage", "0", radiusDamage.damage, 0, WC_PARAM_UINT16),
		EVT_FLAGS("damage_type", "0", radiusDamage.damageBits, 0, g_wc_dmgFlags),
	);

	{
		static const char* teExpFlags[32];
		teExpFlags[BitIndex(FL_WC_TE_EXPLOSION_OPAQUE)] = "opaque";
		teExpFlags[BitIndex(FL_WC_TE_EXPLOSION_NO_DLIGHT)] = "no_dlight";
		teExpFlags[BitIndex(FL_WC_TE_EXPLOSION_NO_SOUND)] = "no_sound";
		teExpFlags[BitIndex(FL_WC_TE_EXPLOSION_NO_PARTICLES)] = "no_particles";

		EVT_DESC(WC_EVT_EXPLOSION, "te_explosion",
			EVT_FIELD("sprite", NULL, te_explosion.sprite, 9, WC_PARAM_MODEL_INDEX),
			EVT_FLAGS("flags", "0", te_explosion.flags, 7, teExpFlags),
			EVT_FIELD("scale", "0", te_explosion.scale, 0, WC_PARAM_UINT8),
			EVT_FIELD("fps", "0", te_explosion.fps, 0, WC_PARAM_UINT8),
		);
	}

	{
		static const char* circleTypes[32];
		circleTypes[WC_BEAM_CIRCLE_TYPE_CYLINDER] = "cylinder";
		circleTypes[WC_BEAM_CIRCLE_TYPE_TORUS] = "torus";
		circleTypes[WC_BEAM_CIRCLE_TYPE_DISK] = "disk";

		EVT_DESC(WC_EVT_BEAM_CIRCLE, "beam_circle",
			EVT_FIELD("sprite", NULL, beam_circle.sprite, 9, WC_PARAM_MODEL_INDEX),
			EVT__ENUM("type", "cylinder", beam_circle.beamType, 4, circleTypes),
			EVT_FIELD("has_frame", "0", beam_circle.hasFrame, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("has_height", "0", beam_circle.hasHeight, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("has_noise", "0", beam_circle.hasNoise, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("radius", "0", beam_circle.radius, 0, WC_PARAM_INT16),
			EVT_FIELD("life", "0", beam_circle.life, 0, WC_PARAM_UINT8),
			EVT_FIELD("color", "0 0 0 0", beam_circle.color, 0, WC_PARAM_RGBA),
			EVT_FIELD("frame", "0", beam_circle.frame, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(beam_circle.hasFrame)),
			EVT_FIELD("height", "0", beam_circle.height, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(beam_circle.hasHeight)),
			EVT_FIELD("noise", "0", beam_circle.noise, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(beam_circle.hasNoise)),
		);
	}

	EVT_DESC(WC_EVT_GLOW_SPRITE, "glow_sprite",
		EVT_FIELD("sprite", NULL, glow_sprite.sprite, 0, WC_PARAM_MODEL_INDEX),
		EVT_FIELD("life", "0", glow_sprite.life, 0, WC_PARAM_UINT8),
		EVT_FIELD("scale", "0", glow_sprite.scale, 0, WC_PARAM_UINT8),
		EVT_FIELD("alpha", "0", glow_sprite.alpha, 0, WC_PARAM_UINT8),
	);

	EVT_DESC(WC_EVT_SPARKS, "sparks",
		EVT_FIELD("dummy", "0", sparks.dummy, 0, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG | FL_FIELD_NO_NETWORK),
	);

	EVT_DESC(WC_EVT_ARMOR_RICOCHET, "armor_ricochet",
		EVT_FIELD("scale", "0", armor_ricochet.scale, 0, WC_PARAM_UINT8),
	);

	{
		static const char* quakeEffects[32];
		quakeEffects[WC_QUAKE_EFFECT_GUNSHOT] = "gunshot";
		quakeEffects[WC_QUAKE_EFFECT_EXPLOSION] = "explosion";
		quakeEffects[WC_QUAKE_EFFECT_EXPLOSION2] = "explosion2";
		quakeEffects[WC_QUAKE_EFFECT_LAVASPLASH] = "lavasplash";
		quakeEffects[WC_QUAKE_EFFECT_TELEPORT] = "teleport";
		quakeEffects[WC_QUAKE_EFFECT_PARTICLE_BURST] = "particle_burst";

		EVT_DESC(WC_EVT_QUAKE_EFFECT, "quake_effect",
			EVT__ENUM("type", "gunshot", quake_effect.type, 7, quakeEffects),
			EVT_FIELD("is_particle_burst", "0", quake_effect.isParticleBurst, 1, WC_PARAM_UINT8, NULL, 0, FL_FIELD_NO_CFG),
			EVT_FIELD("radius", "0", quake_effect.radius, 0, WC_PARAM_UINT16, NULL, 0, 0, EVT_COND_BYTE(quake_effect.isParticleBurst)),
			EVT_FIELD("color", "0", quake_effect.color, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(quake_effect.isParticleBurst)),
			EVT_FIELD("life", "0", quake_effect.life, 0, WC_PARAM_UINT8, NULL, 0, 0, EVT_COND_BYTE(quake_effect.isParticleBurst)),
		);
	}

	EVT_DESC(WC_EVT_IMPLOSION, "implosion",
		EVT_FIELD("tracers", "0", implosion.tracers, 0, WC_PARAM_UINT8),
		EVT_FIELD("radius", "0", implosion.radius, 0, WC_PARAM_UINT8),
		EVT_FIELD("life", "0", implosion.life, 0, WC_PARAM_UINT8),
	);

	EVT_DESC(WC_EVT_SPRITE_SPRAY, "sprite_spray",
		EVT_FIELD("sprite", NULL, glow_sprite.sprite, 0, WC_PARAM_MODEL_INDEX),
		EVT_FIELD("count", "0", sprite_spray.count, 0, WC_PARAM_UINT8),
		EVT_FIELD("speed", "0", sprite_spray.speed, 0, WC_PARAM_UINT8),
		EVT_FIELD("randomness", "0", sprite_spray.randomness, 0, WC_PARAM_UINT8),
	);

	EVT_DESC(WC_EVT_STREAK_SPLASH, "streak_splash",
		EVT_FIELD("count", "0", streak_splash.count, 0, WC_PARAM_UINT8),
		EVT_FIELD("color", "0", streak_splash.color, 0, WC_PARAM_UINT8),
		EVT_FIELD("speed", "0", streak_splash.speed, 0, WC_PARAM_UINT16),
		EVT_FIELD("randomness", "0", streak_splash.randomness, 0, WC_PARAM_UINT16),
	);

	EVT_DESC(WC_EVT_SHAKE, "shake",
		EVT_FIELD("radius", "0", shake.radius, 0, WC_PARAM_UINT16),
		EVT_FIELD("amplitude", "0", shake.amplitude, 0, WC_PARAM_UINT16_FP_4_12),
		EVT_FIELD("duration", "0", shake.duration, 0, WC_PARAM_TIME),
		EVT_FIELD("frequency", "0", shake.amplitude, 0, WC_PARAM_UINT16_FP_8_8),
	);
}

void init_custom_ammo_fields() {
	WEP_STRUCT_DESC(g_wc_desc_custom_ammo, "general",
		AMMO_FIELD("classname", "", classname, 0, WC_PARAM_STRING),
		AMMO_FIELD("model", "", model, 0, WC_PARAM_STRING),
		AMMO_FIELD("hull_min", "0 0 0", hullSizeMin, 0, WC_PARAM_VECTOR),
		AMMO_FIELD("hull_max", "0 0 0", hullSizeMax, 0, WC_PARAM_VECTOR),
		AMMO_FIELD("pickup_sound", "items/9mmclip1.wav", pickupSound, 0, WC_PARAM_STRING),
		AMMO_FIELD("ammo_type", "", ammoType, 0, WC_PARAM_STRING),
		AMMO_FIELD("ammo_type_hl", "", ammoTypeHl, 0, WC_PARAM_STRING),
		AMMO_FIELD("ammo_given", "0", ammoGiven, 0, WC_PARAM_UINT16),
		AMMO_FIELD("max_ammo", "0", maxAmmo, 0, WC_PARAM_UINT16),
	);
}

void init_weapon_custom_config_parser() {
	g_wc_name_to_trigger.clear();
	g_wc_name_to_action.clear();

	g_wc_dmgFlags[BitIndex(DMG_CRUSH)] = "crush";
	g_wc_dmgFlags[BitIndex(DMG_BULLET)] = "bullet";
	g_wc_dmgFlags[BitIndex(DMG_SLASH)] = "slash";
	g_wc_dmgFlags[BitIndex(DMG_BURN)] = "burn";
	g_wc_dmgFlags[BitIndex(DMG_FREEZE)] = "freeze";
	g_wc_dmgFlags[BitIndex(DMG_BLAST)] = "blast";
	g_wc_dmgFlags[BitIndex(DMG_CLUB)] = "club";
	g_wc_dmgFlags[BitIndex(DMG_SHOCK)] = "shock";
	g_wc_dmgFlags[BitIndex(DMG_SONIC)] = "sonic";
	g_wc_dmgFlags[BitIndex(DMG_ENERGYBEAM)] = "energybeam";
	g_wc_dmgFlags[BitIndex(DMG_NEVERGIB)] = "nevergib";
	g_wc_dmgFlags[BitIndex(DMG_ALWAYSGIB)] = "alwaysgib";
	g_wc_dmgFlags[BitIndex(DMG_DROWN)] = "drown";
	g_wc_dmgFlags[BitIndex(DMG_PARALYZE)] = "paralyze";
	g_wc_dmgFlags[BitIndex(DMG_NERVEGAS)] = "nervegas";
	g_wc_dmgFlags[BitIndex(DMG_POISON)] = "poison";
	g_wc_dmgFlags[BitIndex(DMG_RADIATION)] = "radiation";
	g_wc_dmgFlags[BitIndex(DMG_DROWNRECOVER)] = "drownrecover";
	g_wc_dmgFlags[BitIndex(DMG_SLOWBURN)] = "slowburn";
	g_wc_dmgFlags[BitIndex(DMG_SLOWFREEZE)] = "slowfreeze";
	g_wc_dmgFlags[BitIndex(DMG_MORTAR)] = "mortar";
	g_wc_dmgFlags[BitIndex(DMG_SNIPER)] = "sniper";
	g_wc_dmgFlags[BitIndex(DMG_MEDKITHEAL)] = "medkitheal";
	g_wc_dmgFlags[BitIndex(DMG_LAUNCH)] = "launch";
	g_wc_dmgFlags[BitIndex(DMG_SHOCK_GLOW)] = "shock_glow";

	init_weapon_struct_fields();
	init_event_fields();
	init_custom_ammo_fields();

	g_wc_evt_trigger_names[WC_TRIG_PRIMARY] = "primary_attack";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY] = "secondary_attack";
	g_wc_evt_trigger_names[WC_TRIG_TERTIARY] = "tertiary_attack";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_ALT] = "primary_alt_attack";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_CLIPSIZE] = "primary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_CLIP_SP] = "primary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_ALT_CLIPSIZE] = "primary_alt_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_ALT_CLIP_SP] = "primary_alt_clip_is";
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
	g_wc_evt_trigger_names[WC_TRIG_IDLE] = "idle";
	g_wc_evt_trigger_names[WC_TRIG_BULLET_FIRED] = "bullet_fired";
	g_wc_evt_trigger_names[WC_TRIG_LASER_ON] = "laser_on";
	g_wc_evt_trigger_names[WC_TRIG_LASER_OFF] = "laser_off";
	g_wc_evt_trigger_names[WC_TRIG_ZOOM_IN] = "zoom_in";
	g_wc_evt_trigger_names[WC_TRIG_ZOOM_OUT] = "zoom_out";
	g_wc_evt_trigger_names[WC_TRIG_IMPACT] = "impact";
	g_wc_evt_trigger_names[WC_TRIG_RICOCHET] = "ricochet";

	g_wc_evt_trigger_arg_primary_names[WC_TRIG_SHOOT_ARG_ALWAYS] = "";
	g_wc_evt_trigger_arg_primary_names[WC_TRIG_SHOOT_ARG_AKIMBO] = "_akimbo";
	g_wc_evt_trigger_arg_primary_names[WC_TRIG_SHOOT_ARG_NOT_AKIMBO] = "_not_akimbo";

	g_wc_evt_trigger_clip_sp_names[WC_TRIG_CLIP_ARG_ODD] = "odd";
	g_wc_evt_trigger_clip_sp_names[WC_TRIG_CLIP_ARG_EVEN] = "even";
	g_wc_evt_trigger_clip_sp_names[WC_TRIG_CLIP_ARG_EMPTY] = "empty";
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

	g_wc_evt_trigger_arg_idle_names[WC_TRIG_IDLE_ARG_DEFAULT] = "";
	g_wc_evt_trigger_arg_idle_names[WC_TRIG_IDLE_ARG_EMPTY] = "_empty";
	g_wc_evt_trigger_arg_idle_names[WC_TRIG_IDLE_ARG_LASER] = "_laser";
	g_wc_evt_trigger_arg_idle_names[WC_TRIG_IDLE_ARG_AKIMBO] = "_akimbo";
	g_wc_evt_trigger_arg_idle_names[WC_TRIG_IDLE_ARG_ZOOM] = "_zoom";

	g_wc_evt_trigger_arg_deploy_names[WC_TRIG_DEPLOY_ARG_DEFAULT] = "";
	g_wc_evt_trigger_arg_deploy_names[WC_TRIG_DEPLOY_ARG_LASER] = "_laser";
	g_wc_evt_trigger_arg_deploy_names[WC_TRIG_DEPLOY_ARG_AKIMBO] = "_akimbo";
	g_wc_evt_trigger_arg_deploy_names[WC_TRIG_DEPLOY_ARG_EMPTY] = "_empty";
	g_wc_evt_trigger_arg_deploy_names[WC_TRIG_DEPLOY_ARG_FIRST] = "_first";

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
		case WC_TRIG_RELOAD_FINISH: {
			for (int k = 0; k < ARRAY_SZ(g_wc_evt_trigger_arg_primary_names); k++) {
				const char* key = UTIL_VarArgs("%s%s", tname, g_wc_evt_trigger_arg_primary_names[k]);
				uint16_t val = (k << EVT_TYPE_BITS) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		}
		case WC_TRIG_IDLE:
			for (int k = 0; k < ARRAY_SZ(g_wc_evt_trigger_arg_idle_names); k++) {
				const char* key = UTIL_VarArgs("%s%s", tname, g_wc_evt_trigger_arg_idle_names[k]);
				uint16_t val = (k << EVT_TYPE_BITS) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		case WC_TRIG_DEPLOY:
			for (int k = 0; k < ARRAY_SZ(g_wc_evt_trigger_arg_deploy_names); k++) {
				const char* key = UTIL_VarArgs("%s%s", tname, g_wc_evt_trigger_arg_deploy_names[k]);
				uint16_t val = (k << EVT_TYPE_BITS) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		case WC_TRIG_PRIMARY_CLIPSIZE:
		case WC_TRIG_PRIMARY_ALT_CLIPSIZE:
			for (int k = 0; k < 32; k++) {
				const char* key = UTIL_VarArgs("%s_%u", tname, k);
				uint16_t val = (k << EVT_TYPE_BITS) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		case WC_TRIG_PRIMARY_CLIP_SP:
		case WC_TRIG_PRIMARY_ALT_CLIP_SP:
		case WC_TRIG_SECONDARY_CLIP_SP:
			for (int k = 0; k < ARRAY_SZ(g_wc_evt_trigger_clip_sp_names); k++) {
				const char* key = UTIL_VarArgs("%s_%s", tname, g_wc_evt_trigger_clip_sp_names[k]);
				uint16_t val = (k << EVT_TYPE_BITS) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		case WC_TRIG_IMPACT:
		case WC_TRIG_RICOCHET:
			for (int k = 0; k < ARRAY_SZ(g_wc_evt_trigger_impact_names); k++) {
				const char* key = UTIL_VarArgs("%s_%s%%", tname, g_wc_evt_trigger_impact_names[k]);
				uint16_t val = (k << EVT_TYPE_BITS) | i;
				g_wc_name_to_trigger.put(key, val);
				g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
			}
			break;
		case WC_TRIG_PRIMARY_CHARGE:
		case WC_TRIG_SECONDARY_CHARGE:
			g_wc_name_to_trigger.put(tname, i);
			g_wc_trigger_to_name[i] = g_wc_trigger_string_pool.alloc(tname);

			for (int k = 0; k <= 10; k++) {
				{
					const char* key = UTIL_VarArgs("%s_above_%d", tname, k * 10);
					uint16_t val = ((k + 1) << EVT_TYPE_BITS) | i;
					g_wc_name_to_trigger.put(key, val);
					g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
				}
				{
					const char* key = UTIL_VarArgs("%s_below_%d", tname, k * 10);
					uint16_t val = ((k + 12) << EVT_TYPE_BITS) | i;
					g_wc_name_to_trigger.put(key, val);
					g_wc_trigger_to_name[val] = g_wc_trigger_string_pool.alloc(key);
				}
			}
			break;
		default:
			g_wc_name_to_trigger.put(tname, i);
			g_wc_trigger_to_name[i] = g_wc_trigger_string_pool.alloc(tname);
			break;
		}
	}

	g_wc_evt_category_names[WC_EVT_CATEGORY_PRIMARY] = "Primary attack";
	g_wc_evt_category_names[WC_EVT_CATEGORY_PRIMARY_ALT] = "Alternate primary attack";
	g_wc_evt_category_names[WC_EVT_CATEGORY_SECONDARY] = "Secondary attack";
	g_wc_evt_category_names[WC_EVT_CATEGORY_TERTIARY] = "Tertiary attack";
	g_wc_evt_category_names[WC_EVT_CATEGORY_RELOAD] = "Reload";
	g_wc_evt_category_names[WC_EVT_CATEGORY_DEPLOY] = "Deploy events";
	g_wc_evt_category_names[WC_EVT_CATEGORY_IDLE] = "Idle events";
	g_wc_evt_category_names[WC_EVT_CATEGORY_REACTION] = "Reactionary events";
	g_wc_evt_category_names[WC_EVT_CATEGORY_STATE_CHANGE] = "State change events";
	g_wc_evt_category_names[WC_EVT_CATEGORY_UNKNOWN] = "Uncategorized events";

	for (int i = 0; i < ARRAY_SZ(g_wc_evt_type_names); i++) {
		if (!g_wc_evt_type_names[i])
			continue;

		g_wc_name_to_action.put(g_wc_evt_type_names[i], i);
	}
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

struct_desc_t* get_evt_desc(int idx) {
	if (idx >= WC_EVT_TOTAL) {
		ALERT(at_error, "Invalid event type %d\n", idx);
		return NULL;
	}

	struct_desc_t* desc = &g_wc_desc_evt[idx];

	if (!desc->name) {
		ALERT(at_error, "unimplemented event writer for %d\n", idx);
		return NULL;
	}

	return desc;
}

const char* describe_event(WepEvt& evt) {
	static char temp[256];
	temp[0] = 0;

	const char* trig = g_wc_trigger_to_name[(evt.triggerArg << EVT_TYPE_BITS) | evt.trigger].str();

	strcat_safe(temp, trig, sizeof(temp));
	strcat_safe(temp, " -> ", sizeof(temp));
	strcat_safe(temp, g_wc_evt_type_names[evt.evtType], sizeof(temp));

	return temp;
}


// Paramter type info funcs

int wc_get_field_bytes(field_desc_t& field) {
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
	case WC_PARAM_ACCURACY_UINT16:
	case WC_PARAM_UINT16_FP_4_12:
	case WC_PARAM_UINT16_FP_8_8:
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
	case WC_PARAM_STRING:
		return 4;
	case WC_PARAM_VECTOR_SFP_10_6:
	case WC_PARAM_VECTOR_SFP_6_10:
	case WC_PARAM_VECTOR_SFP_9_7:
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

std::string wc_get_field_str(field_desc_t& field, uint8_t* dat) {
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
		return UTIL_VarArgs("%.2f", SFP_10_6_TO_FLOAT(*dat));
	case WC_PARAM_UINT16:
	case WC_PARAM_INT16:
	case WC_PARAM_SOUND_INDEX:
	case WC_PARAM_MODEL_INDEX:
	case WC_PARAM_ACCURACY_UINT16:
	case WC_PARAM_TIME:
	case WC_PARAM_UINT16_PERCENT:
	case WC_PARAM_UINT16_FP_4_12:
	case WC_PARAM_UINT16_FP_8_8:
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
	case WC_PARAM_VECTOR_SFP_6_10: {
		int16_t* v = (int16_t*)dat;
		return UTIL_VarArgs("(%.3f %.3f %.3f)",
			SFP_10_6_TO_FLOAT(v[0]), SFP_10_6_TO_FLOAT(v[1]), SFP_10_6_TO_FLOAT(v[2]));
	}
	case WC_PARAM_VECTOR_SFP_10_6: {
		int16_t* v = (int16_t*)dat;
		return UTIL_VarArgs("(%.2f %.2f %.2f)",
			SFP_6_10_TO_FLOAT(v[0]), SFP_6_10_TO_FLOAT(v[1]), SFP_6_10_TO_FLOAT(v[2]));
	}
	case WC_PARAM_VECTOR_SFP_9_7: {
		int16_t* v = (int16_t*)dat;
		return UTIL_VarArgs("(%.2f %.2f %.2f)",
			SFP_9_7_TO_FLOAT(v[0]), SFP_9_7_TO_FLOAT(v[1]), SFP_9_7_TO_FLOAT(v[2]));
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


// event type info funcs

bool is_server_side_event(int evtId) {
	switch (evtId) {
	case WC_EVT_PROJECTILE:
	case WC_EVT_RADIUS_DAMAGE:
	case WC_EVT_SHAKE:
	case WC_EVT_SERVER:
		return true;
	}

	return false;
}

void wc_post_parse_event(WepEvt& evt) {
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
	case WC_EVT_WEP_ANIM:
		evt.anim.hasCooldown = evt.anim.cooldown != 0;
		evt.anim.hasWeights = evt.anim.weights.arrSz != 0;
		break;
	case WC_EVT_EJECT_SHELL:
		evt.ejectShell.hasVel = evt.ejectShell.vel[0] || evt.ejectShell.vel[1] || evt.ejectShell.vel[2];
		evt.ejectShell.hasRand = evt.ejectShell.dirRand || evt.ejectShell.speedRand;
		break;
	case WC_EVT_BEAM:
		evt.beam.hasImpactSprite = evt.beam.impactSprite != 0;
		evt.beam.hasRicoBeams = evt.beam.ricoBeams != 0;
		break;
	case WC_EVT_RECOIL:
		evt.recoil.hasMaxAngles = evt.recoil.maxAngleTime != 0
			|| evt.recoil.maxAngles[0] || evt.recoil.maxAngles[1] || evt.recoil.maxAngles[2];
		break;
	case WC_EVT_RECOIL_ADV: {
		for (int i = 0; i < 3; i++)
			evt.recoilAdv.ops[i] = ((evt.recoilAdv.angleOp[i] & 0xf) << 4) | (evt.recoilAdv.viewOp[i] & 0xf);
		break;
	}
	case WC_EVT_BEAM_CIRCLE:
		evt.beam_circle.hasFrame = evt.beam_circle.frame != 0;
		evt.beam_circle.hasHeight = evt.beam_circle.height != 0;
		evt.beam_circle.hasNoise = evt.beam_circle.noise != 0;
		break;
	case WC_EVT_QUAKE_EFFECT:
		evt.quake_effect.isParticleBurst = evt.quake_effect.type == WC_QUAKE_EFFECT_PARTICLE_BURST;
		break;
	}
}

int wc_get_event_category(int evt) {
	switch (evt) {
	default:
		return WC_EVT_CATEGORY_UNKNOWN;

	case WC_TRIG_TERTIARY:
		return WC_EVT_CATEGORY_TERTIARY;

	case WC_TRIG_PRIMARY_ALT:
	case WC_TRIG_PRIMARY_ALT_CLIP_SP:
	case WC_TRIG_PRIMARY_ALT_CLIPSIZE:
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

	case WC_TRIG_IDLE:
		return WC_EVT_CATEGORY_IDLE;

	case WC_TRIG_LASER_ON:
	case WC_TRIG_LASER_OFF:
	case WC_TRIG_ZOOM_IN:
	case WC_TRIG_ZOOM_OUT:
		return WC_EVT_CATEGORY_STATE_CHANGE;

	case WC_TRIG_BULLET_FIRED:
	case WC_TRIG_IMPACT:
	case WC_TRIG_RICOCHET:
		return WC_EVT_CATEGORY_REACTION;
	}
}