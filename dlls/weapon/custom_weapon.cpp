#include "custom_weapon.h"
#include "CWeaponCustom.h"
#include "StringPool.h"

using namespace std;

enum WC_SUBSTRUCT_TYPE {
	WC_SUBSTRUCT_NONE,
	WC_SUBSTRUCT_RELOAD,
	WC_SUBSTRUCT_IDLE,
	WC_SUBSTRUCT_SHOOT_OPTS,
	WC_SUBSTRUCT_SHOOT_OPTS_MELEE,
	WC_SUBSTRUCT_AKIMBO,
	WC_SUBSTRUCT_AKIMBO_IDLE,
	WC_SUBSTRUCT_AKIMBO_RELOAD,
	WC_SUBSTRUCT_LASER,
	WC_SUBSTRUCT_LASER_IDLE,
};

enum WC_PARAM_TYPE {
	WC_PARAM_UINT8,
	WC_PARAM_UINT16,
	WC_PARAM_UINT16_PERCENT, // percentage stored as a uint16_t
	WC_PARAM_UINT32,
	WC_PARAM_INT8,
	WC_PARAM_INT16,
	WC_PARAM_INT32,
	WC_PARAM_RGBA,
	WC_PARAM_FLOAT,
	WC_PARAM_VECTOR,
	WC_PARAM_SOUND_INDEX, // sound file path stored as an index
	WC_PARAM_MODEL_INDEX, // model file path stored as an index
	WC_PARAM_TIME, // time value stored as uint16_t
	WC_PARAM_ACCURACY_UINT16, // degrees of accuracy (0-1 = 0-180) scaled to uint16_t
	WC_PARAM_ACCURACY_100, // degrees of accuracy scaled by 100
	WC_PARAM_ANIM, // animation index (write even if 0)
	WC_PARAM_STRING,
};

struct wep_param_t {
	const char* name;
	int substruct;
	int offset;
	int type;
};

struct SettingsGroup {
	string name;
	StringMap keys;
	int lineno;
};

#define DECL_WC_PARAM(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_NONE, \
	offsetof(CustomWeaponParams, struct_name), \
	type \
}

#define DECL_WC_PARAM_RELOAD(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_RELOAD, \
	offsetof(WeaponCustomReload, struct_name), \
	type \
}

#define DECL_WC_PARAM_IDLE(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_IDLE, \
	offsetof(WeaponCustomIdle, struct_name), \
	type \
}

#define DECL_WC_PARAM_AKIMBO_IDLE(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_AKIMBO_IDLE, \
	offsetof(WeaponCustomIdle, struct_name), \
	type \
}

#define DECL_WC_PARAM_LASER_IDLE(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_LASER_IDLE, \
	offsetof(WeaponCustomIdle, struct_name), \
	type \
}

#define DECL_WC_PARAM_AKIMBO_RELOAD(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_AKIMBO_RELOAD, \
	offsetof(WeaponCustomReload, struct_name), \
	type \
}

#define DECL_WC_PARAM_SHOOT_OPTS(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_SHOOT_OPTS, \
	offsetof(CustomWeaponShootOpts, struct_name), \
	type \
}

#define DECL_WC_PARAM_SHOOT_OPTS_MELEE(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_SHOOT_OPTS_MELEE, \
	offsetof(MeleeOpts, struct_name), \
	type \
}

#define DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND(cfg_name, struct_name, offset, type) { \
	cfg_name, \
	WC_SUBSTRUCT_SHOOT_OPTS_MELEE, \
	offsetof(MeleeOpts, struct_name) + sizeof(uint16_t)*offset, \
	type \
}

#define DECL_WC_PARAM_AKIMBO(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_AKIMBO, \
	offsetof(WeaponCustomAkimbo, struct_name), \
	type \
}

#define DECL_WC_PARAM_LASER(cfg_name, struct_name, type) { \
	cfg_name, \
	WC_SUBSTRUCT_LASER, \
	offsetof(WeaponCustomLaser, struct_name), \
	type \
}

const char* g_wc_flag_names[32];
const char* g_wc_shoot_flag_names[32];
const char* g_distant_sound_names[8];
const char* g_shell_sound_names[8];
const char* g_wc_evt_playsound_flag_names[32];
const char* g_wc_evt_trigger_names[32];
const char* g_wc_evt_trigger_arg_primary_names[32];
const char* g_wc_evt_trigger_clip_sp_names[32];
const char* g_wc_evt_trigger_impact_names[32];
const char* g_wc_evt_type_names[32];
const char* g_wc_evt_channel_names[8];
const char* g_wc_evt_aivol_names[8];
const char* g_wc_evt_anim_flag_names[8];
const char* g_wc_evt_anim_hand_names[8];
const char* g_wc_evt_punch_flag_names[8];
const char* g_wc_evt_bullet_flag_names[8];
const char* g_wc_evt_bullet_color_names[32];
const char* g_wc_evt_bullet_flash_size_names[32];
const char* g_wc_evt_beam_flag_names[32];
const char* g_wc_evt_beam_anim_names[32];
const char* g_wc_evt_proj_flag_names[32];
const char* g_wc_evt_proj_type_names[32];
const char* g_wc_evt_proj_action_names[32];
const char* g_wc_evt_togglestate_mode_names[32];
const char* g_wc_evt_togglestate_flag_names[32];
const char* g_wc_evt_cooldown_flag_names[32];

HashMap<uint16_t> g_wc_name_to_trigger; // maps a group name to an event trigger + argument value
HashMap<uint8_t> g_wc_name_to_action; // maps an action key value to its event number
mod_string_t g_wc_trigger_to_name[32*32]; // 32 trigger/arg possibilities
StringPool g_wc_trigger_string_pool;

wep_param_t g_wc_params[] = {
	//DECL_WC_PARAM("flags", flags, WC_PARAM_UINT32),
	DECL_WC_PARAM("classname", classname, WC_PARAM_STRING),
	DECL_WC_PARAM("hl_client_classname", wrongClientWeapon, WC_PARAM_STRING),
	DECL_WC_PARAM("clip_size", maxClip, WC_PARAM_UINT16),
	DECL_WC_PARAM("default_ammo", defaultAmmo, WC_PARAM_UINT16),
	DECL_WC_PARAM("v_model", vmodel, WC_PARAM_MODEL_INDEX),
	DECL_WC_PARAM("p_model", pmodel, WC_PARAM_MODEL_INDEX),
	DECL_WC_PARAM("p_model_akimbo", pmodelAkimbo, WC_PARAM_MODEL_INDEX),
	DECL_WC_PARAM("w_model", wmodel, WC_PARAM_MODEL_INDEX),
	DECL_WC_PARAM("w_model_akimbo", wmodelAkimbo, WC_PARAM_MODEL_INDEX),
	DECL_WC_PARAM("thirdperson_anims", animExt, WC_PARAM_STRING),
	DECL_WC_PARAM("thirdperson_anims_zoom", animExtZoom, WC_PARAM_STRING),
	DECL_WC_PARAM("thirdperson_anims_akimbo", animExtAkimbo, WC_PARAM_STRING),
	DECL_WC_PARAM("deploy_anim", deployAnim, WC_PARAM_ANIM),
	DECL_WC_PARAM("deploy_time", deployTime, WC_PARAM_UINT16),
	DECL_WC_PARAM("deploy_anim_time", deployAnimTime, WC_PARAM_UINT16),
	DECL_WC_PARAM("move_speed", moveSpeedMult, WC_PARAM_UINT16_PERCENT),
	DECL_WC_PARAM("jump_power", jumpPower, WC_PARAM_INT32),

	DECL_WC_PARAM_RELOAD("anim", anim, WC_PARAM_ANIM),
	DECL_WC_PARAM_RELOAD("time", time, WC_PARAM_TIME),

	DECL_WC_PARAM_AKIMBO_RELOAD("anim", anim, WC_PARAM_ANIM),
	DECL_WC_PARAM_AKIMBO_RELOAD("time", time, WC_PARAM_TIME),

	DECL_WC_PARAM_IDLE("anim", anim, WC_PARAM_ANIM),
	DECL_WC_PARAM_IDLE("weight", weight, WC_PARAM_UINT8),
	DECL_WC_PARAM_IDLE("time", time, WC_PARAM_TIME),

	DECL_WC_PARAM_AKIMBO_IDLE("anim", anim, WC_PARAM_ANIM),
	DECL_WC_PARAM_AKIMBO_IDLE("weight", weight, WC_PARAM_UINT8),
	DECL_WC_PARAM_AKIMBO_IDLE("time", time, WC_PARAM_TIME),

	DECL_WC_PARAM_LASER_IDLE("anim", anim, WC_PARAM_ANIM),
	DECL_WC_PARAM_LASER_IDLE("weight", weight, WC_PARAM_UINT8),
	DECL_WC_PARAM_LASER_IDLE("time", time, WC_PARAM_TIME),

	//DECL_WC_PARAM_SHOOT_OPTS("flags", flags, WC_PARAM_UINT8),
	DECL_WC_PARAM_SHOOT_OPTS("ammo_cost", ammoCost, WC_PARAM_UINT8),
	DECL_WC_PARAM_SHOOT_OPTS("ammo_freq", ammoFreq, WC_PARAM_UINT8),
	DECL_WC_PARAM_SHOOT_OPTS("ammo_pool", ammoPool, WC_PARAM_UINT8),
	DECL_WC_PARAM_SHOOT_OPTS("cooldown", cooldown, WC_PARAM_TIME),
	DECL_WC_PARAM_SHOOT_OPTS("cooldown_fail", cooldownFail, WC_PARAM_TIME),
	DECL_WC_PARAM_SHOOT_OPTS("charge_mode", chargeMode, WC_PARAM_UINT8),
	DECL_WC_PARAM_SHOOT_OPTS("charge_ammo_mode", chargeAmmoMode, WC_PARAM_UINT8),
	DECL_WC_PARAM_SHOOT_OPTS("overcharge_mode", overchargeMode, WC_PARAM_UINT8),
	DECL_WC_PARAM_SHOOT_OPTS("charge_flags", chargeFlags, WC_PARAM_UINT8),
	DECL_WC_PARAM_SHOOT_OPTS("charge_time", chargeTime, WC_PARAM_TIME),
	DECL_WC_PARAM_SHOOT_OPTS("overcharge_time", chargeTime, WC_PARAM_TIME),
	DECL_WC_PARAM_SHOOT_OPTS("charge_cancel_time", chargeCancelTime, WC_PARAM_TIME),
	DECL_WC_PARAM_SHOOT_OPTS("charge_move_speed", chargeMoveSpeedMult, WC_PARAM_UINT16_PERCENT),
	//DECL_WC_PARAM_SHOOT_OPTS("accuracy_x", accuracyX, WC_PARAM_ACCURACY_100),
	//DECL_WC_PARAM_SHOOT_OPTS("accuracy_y", accuracyY, WC_PARAM_ACCURACY_100),
	DECL_WC_PARAM_SHOOT_OPTS("empty_sound", emptySound, WC_PARAM_SOUND_INDEX),

	DECL_WC_PARAM_SHOOT_OPTS_MELEE("melee_damage", damage, WC_PARAM_FLOAT),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE("melee_damage_type", damageBits, WC_PARAM_INT32),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE("melee_range", range, WC_PARAM_FLOAT),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE("melee_attack_offset", attackOffset, WC_PARAM_VECTOR),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE("melee_miss_cooldown", missCooldown, WC_PARAM_TIME),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE("melee_hit_cooldown", hitCooldown, WC_PARAM_TIME),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE("melee_decal_delay", decalDelay, WC_PARAM_TIME),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_hit_wall_sound", hitWallSounds, 0, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_hit_wall_sound2", hitWallSounds, 1, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_hit_wall_sound3", hitWallSounds, 2, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_hit_wall_sound4", hitWallSounds, 3, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_hit_flesh_sound", hitFleshSounds, 0, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_hit_flesh_sound2", hitFleshSounds, 1, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_hit_flesh_sound3", hitFleshSounds, 2, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_hit_flesh_sound4", hitFleshSounds, 3, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_miss_sound", missSounds, 0, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_miss_sound2", missSounds, 1, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_miss_sound3", missSounds, 2, WC_PARAM_SOUND_INDEX),
	DECL_WC_PARAM_SHOOT_OPTS_MELEE_SND("melee_miss_sound4", missSounds, 3, WC_PARAM_SOUND_INDEX),

	DECL_WC_PARAM_AKIMBO("deploy_anim", deployAnim, WC_PARAM_ANIM),
	DECL_WC_PARAM_AKIMBO("deploy_time", deployTime, WC_PARAM_TIME),
	DECL_WC_PARAM_AKIMBO("akimbo_deploy_anim", akimboDeployAnim, WC_PARAM_ANIM),
	DECL_WC_PARAM_AKIMBO("akimbo_deploy_time", akimboDeployTime, WC_PARAM_TIME),
	DECL_WC_PARAM_AKIMBO("akimbo_deploy_anim_time", akimboDeployAnimTime, WC_PARAM_TIME),
	DECL_WC_PARAM_AKIMBO("holster_anim", holsterAnim, WC_PARAM_ANIM),
	DECL_WC_PARAM_AKIMBO("holster_time", holsterTime, WC_PARAM_TIME),
	//DECL_WC_PARAM_AKIMBO("accuracy_x", accuracyX, WC_PARAM_ACCURACY_100),
	//DECL_WC_PARAM_AKIMBO("accuracy_y", accuracyY, WC_PARAM_ACCURACY_100),

	DECL_WC_PARAM_LASER("dot_sprite", dotSprite, WC_PARAM_MODEL_INDEX),
	DECL_WC_PARAM_LASER("beam_sprite", beamSprite, WC_PARAM_MODEL_INDEX),
	DECL_WC_PARAM_LASER("dot_size", dotSz, WC_PARAM_UINT8),
	DECL_WC_PARAM_LASER("beam_width", beamWidth, WC_PARAM_UINT8),
	DECL_WC_PARAM_LASER("attachment", attachment, WC_PARAM_UINT8),
};

int g_wc_params_count = sizeof(g_wc_params) / sizeof(wep_param_t);

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

void init_weapon_custom_config_parser() {
	g_wc_name_to_trigger.clear();
	g_wc_name_to_action.clear();

	//g_wc_flag_names[BitToIndex(FL_WC_WEP_HAS_PRIMARY)] = "has_primary";
	//g_wc_flag_names[BitToIndex(FL_WC_WEP_HAS_SECONDARY)] = "has_secondary";
	//g_wc_flag_names[BitToIndex(FL_WC_WEP_HAS_TERTIARY)] = "has_tertiary";
	//g_wc_flag_names[BitToIndex(FL_WC_WEP_HAS_ALT_PRIMARY)] = "has_alt_primary";
	//g_wc_flag_names[BitToIndex(FL_WC_WEP_SHOTGUN_RELOAD)] = "shotgun_reload";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_UNLINK_COOLDOWNS)] = "unlink_cooldowns";
	//g_wc_flag_names[BitToIndex(FL_WC_WEP_AKIMBO)] = "akimbo";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_LINK_CHARGEUPS)] = "link_chargeups";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_PRIMARY_PRIORITY)] = "primary_priority";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_EXCLUSIVE_HOLD)] = "exclusive_hold";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_USE_ONLY)] = "use_only";
	//g_wc_flag_names[BitToIndex(FL_WC_WEP_HAS_LASER)] = "has_laser";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_DYNAMIC_ACCURACY)] = "dynamic_accuracy";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_ZOOM_SPR_STRETCH)] = "strech_zoom_sprite";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_ZOOM_SPR_ASPECT)] = "keep_zoom_sprite_aspect";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_EMPTY_IDLES)] = "empty_idles";
	g_wc_flag_names[BitToIndex(FL_WC_WEP_NO_PREDICTION)] = "no_prediction";

	g_wc_shoot_flag_names[BitToIndex(FL_WC_SHOOT_UNDERWATER)] = "works_underwater";
	g_wc_shoot_flag_names[BitToIndex(FL_WC_SHOOT_NO_ATTACK)] = "not_an_attack";
	g_wc_shoot_flag_names[BitToIndex(FL_WC_SHOOT_COOLDOWN_IDLE)] = "always_cooldown_idle";
	g_wc_shoot_flag_names[BitToIndex(FL_WC_SHOOT_NEED_AKIMBO)] = "akimbo_only";
	g_wc_shoot_flag_names[BitToIndex(FL_WC_SHOOT_NEED_FULL_COST)] = "need_full_cost";
	g_wc_shoot_flag_names[BitToIndex(FL_WC_SHOOT_NO_AUTOFIRE)] = "no_autofire";
	//g_wc_shoot_flag_names[BitToIndex(FL_WC_SHOOT_IS_MELEE)] = "is_melee";

	g_wc_evt_trigger_names[WC_TRIG_PRIMARY] = "event.primary";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY] = "event.secondary";
	g_wc_evt_trigger_names[WC_TRIG_TERTIARY] = "event.tertiary";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_ALT] = "event.primary_alt";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_CLIPSIZE] = "event.primary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_CLIP_SP] = "event.primary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_CHARGE] = "event.primary_charge";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_OVERCHARGE] = "event.primary_overcharge";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_START] = "event.primary_start";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_STOP] = "event.primary_stop";
	g_wc_evt_trigger_names[WC_TRIG_PRIMARY_FAIL] = "event.primary_fail";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_CLIPSIZE] = "event.secondary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_CLIP_SP] = "event.secondary_clip_is";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_CHARGE] = "event.secondary_charge";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_OVERCHARGE] = "event.secondary_overcharge";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_START] = "event.secondary_start";
	g_wc_evt_trigger_names[WC_TRIG_SECONDARY_STOP] = "event.secondary_stop";
	g_wc_evt_trigger_names[WC_TRIG_RELOAD] = "event.reload";
	g_wc_evt_trigger_names[WC_TRIG_RELOAD_EMPTY] = "event.reload_empty";
	g_wc_evt_trigger_names[WC_TRIG_RELOAD_NOT_EMPTY] = "event.reload_not_empty";
	g_wc_evt_trigger_names[WC_TRIG_RELOAD_FINISH] = "event.reload_finish";
	g_wc_evt_trigger_names[WC_TRIG_DEPLOY] = "event.deploy";
	g_wc_evt_trigger_names[WC_TRIG_BULLET_FIRED] = "event.bullet_fired";
	g_wc_evt_trigger_names[WC_TRIG_LASER_ON] = "event.laser_on";
	g_wc_evt_trigger_names[WC_TRIG_LASER_OFF] = "event.laser_off";
	g_wc_evt_trigger_names[WC_TRIG_ZOOM_IN] = "event.zoom_in";
	g_wc_evt_trigger_names[WC_TRIG_ZOOM_OUT] = "event.zoom_out";
	g_wc_evt_trigger_names[WC_TRIG_IMPACT] = "event.impact";

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

	// impact events
	g_wc_evt_type_names[WC_EVT_MUZZLEFLASH] = "idle_sound";
	g_wc_evt_type_names[WC_EVT_SPRITETRAIL] = "sprite_trail";
	g_wc_evt_type_names[WC_EVT_DECAL] = "decal";

	for (int i = 0; i < ARRAY_SZ(g_wc_evt_type_names); i++) {
		if (!g_wc_evt_type_names[i])
			continue;

		g_wc_name_to_action.put(g_wc_evt_type_names[i], i);
	}

	g_wc_evt_channel_names[CHAN_AUTO] = "auto";
	g_wc_evt_channel_names[CHAN_WEAPON] = "weapon";
	g_wc_evt_channel_names[CHAN_VOICE] = "voice";
	g_wc_evt_channel_names[CHAN_ITEM] = "item";
	g_wc_evt_channel_names[CHAN_BODY] = "body";
	g_wc_evt_channel_names[CHAN_STATIC] = "static";

	g_wc_evt_aivol_names[WC_AIVOL_SILENT] = "silent";
	g_wc_evt_aivol_names[WC_AIVOL_QUIET] = "quiet";
	g_wc_evt_aivol_names[WC_AIVOL_NORMAL] = "normal";
	g_wc_evt_aivol_names[WC_AIVOL_LOUD] = "loud";

	g_distant_sound_names[DISTANT_NONE] = "none";
	g_distant_sound_names[DISTANT_9MM] = "distant_9mm";
	g_distant_sound_names[DISTANT_357] = "distant_357";
	g_distant_sound_names[DISTANT_556] = "distant_556";
	g_distant_sound_names[DISTANT_BOOM] = "distant_boom";

	g_shell_sound_names[TE_BOUNCE_NULL] = "none";
	g_shell_sound_names[TE_BOUNCE_SHELL] = "shell";
	g_shell_sound_names[TE_BOUNCE_SHOTSHELL] = "shotshell";

	g_wc_evt_anim_hand_names[WC_ANIM_BOTH_HANDS] = "both";
	g_wc_evt_anim_hand_names[WC_ANIM_LEFT_HAND] = "left";
	g_wc_evt_anim_hand_names[WC_ANIM_RIGHT_HAND] = "right";
	g_wc_evt_anim_hand_names[WC_ANIM_TRIG_HAND] = "trigger";

	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_WHITE] = "white";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_RED] = "red";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_GREEN] = "green";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_BLUE] = "blue";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_DEFAULT] = "default";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_YELLOW] = "yellow";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_ORANGE2] = "orange2";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_BLUE2] = "blue2";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_ORANGE3] = "orange3";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_ORANGE4] = "orange4";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_TAN] = "tan";
	g_wc_evt_bullet_color_names[WC_TRACER_COLOR_ORANGE] = "orange";

	g_wc_evt_bullet_flash_size_names[WC_FLASH_NONE] = "none";
	g_wc_evt_bullet_flash_size_names[WC_FLASH_DIM] = "dim";
	g_wc_evt_bullet_flash_size_names[WC_FLASH_NORMAL] = "normal";
	g_wc_evt_bullet_flash_size_names[WC_FLASH_BRIGHT] = "bright";

	g_wc_evt_beam_anim_names[WC_BEAM_ANIM_DISABLED] = "disabled";
	g_wc_evt_beam_anim_names[WC_BEAM_ANIM_TOGGLE] = "toggle";
	g_wc_evt_beam_anim_names[WC_BEAM_ANIM_LINEAR] = "linear";
	g_wc_evt_beam_anim_names[WC_BEAM_ANIM_LINEAR_TOGGLE] = "linear_toggle";
	g_wc_evt_beam_anim_names[WC_BEAM_ANIM_EASE_IN_OUT] = "ease_in_out";

	g_wc_evt_proj_type_names[WC_PROJECTILE_ARGRENADE] = "ar_grenade";
	g_wc_evt_proj_type_names[WC_PROJECTILE_BANANA] = "banana_bomb";
	g_wc_evt_proj_type_names[WC_PROJECTILE_BOLT] = "crossbow_bolt";
	g_wc_evt_proj_type_names[WC_PROJECTILE_DISPLACER] = "displacer_portal";
	g_wc_evt_proj_type_names[WC_PROJECTILE_GRENADE] = "hand_grenade";
	g_wc_evt_proj_type_names[WC_PROJECTILE_HORNET] = "hornet";
	g_wc_evt_proj_type_names[WC_PROJECTILE_HVR] = "hvr";
	g_wc_evt_proj_type_names[WC_PROJECTILE_MORTAR] = "mortar";
	g_wc_evt_proj_type_names[WC_PROJECTILE_RPG] = "rpg";
	g_wc_evt_proj_type_names[WC_PROJECTILE_SHOCK] = "shock_beam";
	g_wc_evt_proj_type_names[WC_PROJECTILE_WEAPON] = "weapon";
	g_wc_evt_proj_type_names[WC_PROJECTILE_TRIPMINE] = "tripmine";
	g_wc_evt_proj_type_names[WC_PROJECTILE_CUSTOM] = "custom";
	g_wc_evt_proj_type_names[WC_PROJECTILE_OTHER] = "other";

	g_wc_evt_proj_action_names[WC_PROJ_ACT_IMPACT] = "impact";
	g_wc_evt_proj_action_names[WC_PROJ_ACT_BOUNCE] = "bounce";
	g_wc_evt_proj_action_names[WC_PROJ_ACT_ATTACH] = "attach";

	g_wc_evt_togglestate_mode_names[WC_TOGGLE_STATE_OFF] = "off";
	g_wc_evt_togglestate_mode_names[WC_TOGGLE_STATE_ON] = "on";
	g_wc_evt_togglestate_mode_names[WC_TOGGLE_STATE_TOGGLE] = "toggle";

	g_wc_evt_playsound_flag_names[BitToIndex(FL_WC_SOUND_CHARGE_PITCH)] = "chargeup_pitch";

	g_wc_evt_punch_flag_names[BitToIndex(FL_WC_PUNCH_SET)] = "set_angles";
	g_wc_evt_punch_flag_names[BitToIndex(FL_WC_PUNCH_ADD)] = "add_angles";
	g_wc_evt_punch_flag_names[BitToIndex(FL_WC_PUNCH_NO_RETURN)] = "no_recovery";
	g_wc_evt_punch_flag_names[BitToIndex(FL_WC_PUNCH_DUCK)] = "only_when_ducking";
	g_wc_evt_punch_flag_names[BitToIndex(FL_WC_PUNCH_STAND)] = "only_when_standing";

	g_wc_evt_anim_flag_names[BitToIndex(FL_WC_ANIM_NO_RESET)] = "no_reset";
	g_wc_evt_anim_flag_names[BitToIndex(FL_WC_ANIM_PMODEL)] = "thirdperson_model";
	g_wc_evt_anim_flag_names[BitToIndex(FL_WC_ANIM_ORDERED)] = "play_in_order";

	g_wc_evt_bullet_flag_names[BitToIndex(FL_WC_BULLETS_DYNAMIC_SPREAD)] = "dynamic_spread";
	g_wc_evt_bullet_flag_names[BitToIndex(FL_WC_BULLETS_NO_DECAL)] = "no_decal";
	g_wc_evt_bullet_flag_names[BitToIndex(FL_WC_BULLETS_NO_SOUND)] = "no_sound";

	g_wc_evt_beam_flag_names[BitToIndex(FL_WC_BEAM_SPIRAL)] = "spiral";
	g_wc_evt_beam_flag_names[BitToIndex(FL_WC_BEAM_OPAQUE)] = "opaque";
	g_wc_evt_beam_flag_names[BitToIndex(FL_WC_BEAM_SHADEIN)] = "fade_in";
	g_wc_evt_beam_flag_names[BitToIndex(FL_WC_BEAM_SHADEOUT)] = "fade_out";

	g_wc_evt_proj_flag_names[BitToIndex(FL_WC_PROJ_NO_BUBBLES)] = "no_bubbles";
	g_wc_evt_proj_flag_names[BitToIndex(FL_WC_PROJ_NO_ORIENT)] = "static_angles";
	g_wc_evt_proj_flag_names[BitToIndex(FL_WC_PROJ_IS_HOOK)] = "is_hook";

	g_wc_evt_togglestate_flag_names[BitToIndex(FL_WC_STATE_PRIMARY_ALT)] = "primary_alt";
	g_wc_evt_togglestate_flag_names[BitToIndex(FL_WC_STATE_LASER)] = "laser";
	g_wc_evt_togglestate_flag_names[BitToIndex(FL_WC_STATE_IS_AKIMBO)] = "akimbo";
	g_wc_evt_togglestate_flag_names[BitToIndex(FL_WC_STATE_CAN_AKIMBO)] = "can_akimbo";

	g_wc_evt_cooldown_flag_names[BitToIndex(FL_WC_COOLDOWN_PRIMARY)] = "primary";
	g_wc_evt_cooldown_flag_names[BitToIndex(FL_WC_COOLDOWN_SECONDARY)] = "secondary";
	g_wc_evt_cooldown_flag_names[BitToIndex(FL_WC_COOLDOWN_TERTIARY)] = "tertiary";
	g_wc_evt_cooldown_flag_names[BitToIndex(FL_WC_COOLDOWN_IDLE)] = "idle";
}


uint16_t wc_get_sound_index(StringMap& kv, const char* key) {
	const char* val = kv.get(key);
	return val ? SOUND_INDEX(val) : 0;
}

uint16_t wc_get_model_index(StringMap& kv, const char* key) {
	const char* val = kv.get(key);
	return val ? MODEL_INDEX(val) : 0;
}

int wc_get_decal_idx(StringMap& kv, const char* key) {
	const char* val = kv.get(key);
	return val ? DECAL_INDEX(val) : 0;
}

Vector wc_get_vector(StringMap& kv, const char* key) {
	const char* val = kv.get(key);
	return val ? UTIL_ParseVector(val) : Vector();
}

RGBA wc_get_rgba(StringMap& kv, const char* key) {
	const char* val = kv.get(key);
	return val ? UTIL_ParseRGBA(val) : RGBA();
}

float wc_get_float(StringMap& kv, const char* key, float defaultValue = 0) {
	const char* val = kv.get(key);
	return val ? atof(val) : defaultValue;
}

int wc_get_int(StringMap& kv, const char* key, int defaultValue = 0) {
	const char* val = kv.get(key);
	return val ? atoi(val) : defaultValue;
}

bool wc_get_bool(StringMap& kv, const char* key) {
	const char* val = kv.get(key);
	return val ? toLowerCase(val) == "true" : false;
}

uint16_t wc_get_time(const char* fname, SettingsGroup& group, const char* key) {
	const char* val = group.keys.get(key);

	if (!val)
		return 0;

	string sval = val;
	int msSuffix = sval.find("ms");
	if (msSuffix != -1) {
		return atoi(sval.substr(0, msSuffix).c_str());
	}

	int sSuffix = sval.find("s");
	if (sSuffix) {
		return atof(sval.substr(0, msSuffix).c_str()) * 1000;
	}

	ALERT(at_error, "%s (line %d) key '%s' is missing time unit suffix in group '%s'\n",
		fname, group.lineno, key, group.name.c_str());

	return 0;
}

string_t wc_get_string(StringMap& kv, const char* key) {
	const char* val = kv.get(key);
	return val ? ALLOC_STRING(val) : 0;
}

vector<string> wc_get_strings(StringMap& kv, const char* key) {
	const char* val = kv.get(key);
	return val ? splitString(val, " ") : vector<string>();
}

int wc_get_named_value_idx(StringMap& kv, const char* key, const char** names, int namesLen, int defaultValue = 0) {
	const char* val = kv.get(key);
	if (!val)
		return defaultValue;

	for (int i = 0; i < namesLen; i++) {
		if (names[i] && !strcmp(val, names[i])) {
			return i;
		}
	}

	return defaultValue;
}

#define wc_get_named_value(kv, key, names, ...) wc_get_named_value_idx(kv, key, names, ARRAY_SZ(names), ##__VA_ARGS__)

uint32_t wc_read_flags(const char* fname, SettingsGroup& group, const char** flag_names) {
	uint32_t flags = 0;

	for (int i = 0; i < 32; i++) {
		if (!flag_names[i] || !flag_names[i][0])
			continue;

		const char* val = group.keys.get(flag_names[i]);
		if (!val)
			continue;

		string sval = toLowerCase(val);

		if (sval == "true") {
			flags |= 1 << i;
		}
		else if (sval != "false") {
			ALERT(at_error, "%s (line %d): Invalid value '%s' for key '%s' in group '%s'\n",
				fname, group.lineno, val, flag_names[i], group.name.c_str());
		}
	}

	return flags;
}

bool wc_read_accuracy(const char* fname, SettingsGroup& group, const char* key, float& accuracyX, float& accuracyY) {
	accuracyX = 0;
	accuracyY = 0;

	const char* val = group.keys.get(key);
	if (!val)
		return false;

	vector<string> parts = splitString(val, " ");
	if (parts.size() > 2) {
		ALERT(at_error, "%s (line %d): Invalid number of values for key '%s' in group '%s'\n",
			fname, group.lineno, key, group.name.c_str());
		return false;
	}
	else if (parts.size() == 2) {
		accuracyX = atof(parts[0].c_str());
		accuracyY = atof(parts[1].c_str());
	}
	else if (parts.size() == 1) {
		accuracyX = accuracyY = atof(parts[0].c_str());
	}

	return true;
}

vector<SettingsGroup> parse_settings_groups(const char* path) {
	string fname = string("/weapons/") + path;

	vector<SettingsGroup> groups;

	FILE* cfg = UTIL_OpenFile(fname.c_str(), "r");

	if (!cfg)
		return groups;

	string group_name;
	StringMap group_keys;

	int group_lineno = 0;
	int lineno = 0;
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), cfg)) {
		string line = buffer;
		lineno++;

		int comment = line.find("//");
		if (comment != -1) {
			line = line.substr(0, comment);
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

void wc_read_param(const char* fname, SettingsGroup& group, void* dat, const char* name, int ptype) {
	const char* val = group.keys.get(name);
	if (!val)
		return;

	switch (ptype) {
	case WC_PARAM_UINT8:	*(uint8_t*)dat = atoi(val); break;
	case WC_PARAM_ANIM:		*(uint8_t*)dat = atoi(val); break;
	case WC_PARAM_UINT16:	*(uint16_t*)dat = atoi(val); break;
	case WC_PARAM_UINT32:	*(uint32_t*)dat = atoi(val); break;
	case WC_PARAM_INT8:		*(int8_t*)dat = atoi(val); break;
	case WC_PARAM_INT16:	*(int16_t*)dat = atoi(val); break;
	case WC_PARAM_INT32:	*(int32_t*)dat = atoi(val); break;
	case WC_PARAM_FLOAT:	*(float*)dat = atof(val); break;
	case WC_PARAM_RGBA: *(RGBA*)dat = UTIL_ParseRGBA(val); break;
	case WC_PARAM_VECTOR: *(Vector*)dat = UTIL_ParseVector(val); break;
	case WC_PARAM_SOUND_INDEX:		*(uint16_t*)dat = SOUND_INDEX(val); break;
	case WC_PARAM_MODEL_INDEX:		*(uint16_t*)dat = MODEL_INDEX(val); break;
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
	case WC_PARAM_ACCURACY_UINT16:	*(uint16_t*)dat = FLOAT_TO_SPREAD(UTIL_ConeFromDegrees(atof(val)).x); break;
	case WC_PARAM_ACCURACY_100:		*(uint16_t*)dat = atof(val) * 100; break;
	case WC_PARAM_UINT16_PERCENT:	*(uint16_t*)dat = clamp(0, atof(val) * 65535, 65535); break;
	case WC_PARAM_STRING:			*(string_t*)dat = ALLOC_STRING(val); break;
	default:
		ALERT(at_error, "%s (line %d): Unknown param type %d for key '%s' in group '%s'\n",
			fname, group.lineno, ptype, name, group.name.c_str());
	}
}

void wc_read_param(const char* fname, SettingsGroup& group, void* dat, wep_param_t& param) {
	wc_read_param(fname, group, dat, param.name, param.type);
}

void wc_read_params(const char* fname, SettingsGroup& group, void* dat, int substruct) {
	for (int i = 0; i < g_wc_params_count; i++) {
		wep_param_t& param = g_wc_params[i];

		if (param.substruct == substruct) {
			wc_read_param(fname, group, (uint8_t*)dat + param.offset, param);
		}
	}
}

void wc_read_shoot_opts(const char* path, SettingsGroup& group, CustomWeaponParams& params, int idx) {
	wc_read_params(path, group, &params.shootOpts[idx], WC_SUBSTRUCT_SHOOT_OPTS);
	wc_read_params(path, group, &params.shootOpts[idx].melee, WC_SUBSTRUCT_SHOOT_OPTS_MELEE);
	params.shootOpts[idx].flags = wc_read_flags(path, group, g_wc_shoot_flag_names);

	static MeleeOpts emptyMelee;
	if (memcmp(&params.shootOpts[idx].melee, &emptyMelee, sizeof(MeleeOpts))) {
		params.shootOpts[idx].flags |= FL_WC_SHOOT_IS_MELEE;
	}

	float accuracyX, accuracyY;
	if (wc_read_accuracy(path, group, "accuracy", accuracyX, accuracyY)) {
		params.shootOpts[idx].accuracyX = accuracyX * 100 + 0.5f;
		params.shootOpts[idx].accuracyY = accuracyY * 100 + 0.5f;
	}
}

#define WC_COMPARE_FIELD(field, fmt) if (a.field != b.field) { \
	ALERT(at_error, "Mismatch on field '" #field "' " fmt " != " fmt "\n", a.field, b.field); }
#define WC_COMPARE_FIELD_STR(field) if (strcmp(STRING(a.field), STRING(b.field))) { \
	ALERT(at_error, "Mismatch on field '" #field "' '%s' != '%s'\n", STRING(a.field), STRING(b.field)); }
#define WC_COMPARE_FIELD_IDX(field, fmt, idx) if (a.field != b.field) { \
	ALERT(at_error, "Mismatch on field '" #field "' " fmt " != " fmt " (i=%d)\n", a.field, b.field, idx); }
#define WC_COMPARE_FIELD_IDX2(field, fmt, idx, idx2) if (a.field != b.field) { \
	ALERT(at_error, "Mismatch on field '" #field "' " fmt " != " fmt " (i=%d, k=%d)\n", a.field, b.field, idx, idx2); }
#define WC_COMPARE_FIELD_EV(field, fmt, idx) if (e1.field != e2.field) { \
	ALERT(at_error, "Mismatch on event field '" #field "' " fmt " != " fmt " (idx %d)\n", e1.field, e2.field, idx); }
#define WC_COMPARE_FIELD_EV_STR(field, idx) if (strcmp(STRING(e1.field), STRING(e2.field))) { \
	ALERT(at_error, "Mismatch on event field '" #field "' '%s' != '%s' (idx %d)\n", STRING(e1.field), STRING(e2.field), idx); }

// for testing
void compare_params(CustomWeaponParams& a, CustomWeaponParams& b) {
	WC_COMPARE_FIELD(flags, "%d")
	WC_COMPARE_FIELD(maxClip, "%d")
	WC_COMPARE_FIELD(defaultAmmo, "%d")
	WC_COMPARE_FIELD(vmodel, "%d")
	WC_COMPARE_FIELD(deployAnim, "%d")
	WC_COMPARE_FIELD(deployTime, "%d")
	WC_COMPARE_FIELD(deployAnimTime, "%d")
	WC_COMPARE_FIELD(moveSpeedMult, "%d")
	WC_COMPARE_FIELD(jumpPower, "%d")

	for (int i = 0; i < ARRAY_SZ(a.reloadStage); i++) {
		WC_COMPARE_FIELD_IDX(reloadStage[i].anim, "%d", i)
		WC_COMPARE_FIELD_IDX(reloadStage[i].time, "%d", i)
	}

	for (int i = 0; i < ARRAY_SZ(a.idles); i++) {
		WC_COMPARE_FIELD_IDX(idles[i].anim, "%d", i)
		WC_COMPARE_FIELD_IDX(idles[i].weight, "%d", i)
		WC_COMPARE_FIELD_IDX(idles[i].time, "%d", i)
	}

	for (int i = 0; i < ARRAY_SZ(a.shootOpts); i++) {
		WC_COMPARE_FIELD_IDX(shootOpts[i].flags, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].ammoCost, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].ammoFreq, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].ammoPool, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].cooldown, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].cooldownFail, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].chargeMode, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].chargeAmmoMode, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].chargeFlags, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].chargeTime, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].overchargeTime, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].chargeCancelTime, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].chargeMoveSpeedMult, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].accuracyX, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].accuracyY, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].emptySound, "%d", i)

		WC_COMPARE_FIELD_IDX(shootOpts[i].melee.damage, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].melee.damageBits, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].melee.range, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].melee.attackOffset, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].melee.missCooldown, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].melee.hitCooldown, "%d", i)
		WC_COMPARE_FIELD_IDX(shootOpts[i].melee.decalDelay, "%d", i)

		for (int k = 0; k < ARRAY_SZ(a.shootOpts[0].melee.hitWallSounds); k++) {
			WC_COMPARE_FIELD_IDX2(shootOpts[i].melee.hitWallSounds[k], "%d", i, k)
			WC_COMPARE_FIELD_IDX2(shootOpts[i].melee.hitFleshSounds[k], "%d", i, k)
			WC_COMPARE_FIELD_IDX2(shootOpts[i].melee.missSounds[k], "%d", i, k)
		}
	}

	for (int i = 0; i < ARRAY_SZ(a.akimbo.idles); i++) {
		WC_COMPARE_FIELD_IDX(akimbo.idles[i].anim, "%d", i)
		WC_COMPARE_FIELD_IDX(akimbo.idles[i].weight, "%d", i)
		WC_COMPARE_FIELD_IDX(akimbo.idles[i].time, "%d", i)
	}
	
	WC_COMPARE_FIELD(akimbo.reload.anim, "%d")
	WC_COMPARE_FIELD(akimbo.reload.time, "%d")
	WC_COMPARE_FIELD(akimbo.deployAnim, "%d")
	WC_COMPARE_FIELD(akimbo.deployTime, "%d")
	WC_COMPARE_FIELD(akimbo.akimboDeployAnim, "%d")
	WC_COMPARE_FIELD(akimbo.akimboDeployAnimTime, "%d")
	WC_COMPARE_FIELD(akimbo.holsterAnim, "%d")
	WC_COMPARE_FIELD(akimbo.holsterTime, "%d")
	WC_COMPARE_FIELD(akimbo.accuracyX, "%d")
	WC_COMPARE_FIELD(akimbo.accuracyY, "%d")

	for (int i = 0; i < ARRAY_SZ(a.laser.idles); i++) {
		WC_COMPARE_FIELD_IDX(laser.idles[i].anim, "%d", i)
		WC_COMPARE_FIELD_IDX(laser.idles[i].weight, "%d", i)
		WC_COMPARE_FIELD_IDX(laser.idles[i].time, "%d", i)
	}

	WC_COMPARE_FIELD(laser.dotSprite, "%d")
	WC_COMPARE_FIELD(laser.beamSprite, "%d")
	WC_COMPARE_FIELD(laser.dotSz, "%d")
	WC_COMPARE_FIELD(laser.beamWidth, "%d")
	WC_COMPARE_FIELD(laser.attachment, "%d")

	WC_COMPARE_FIELD(pmodel, "%d")
	WC_COMPARE_FIELD(wmodel, "%d")
	WC_COMPARE_FIELD(pmodelAkimbo, "%d")
	WC_COMPARE_FIELD(wmodelAkimbo, "%d")
	WC_COMPARE_FIELD_STR(classname)
	WC_COMPARE_FIELD_STR(wrongClientWeapon)
	WC_COMPARE_FIELD_STR(animExt)
	WC_COMPARE_FIELD_STR(animExtZoom)
	WC_COMPARE_FIELD_STR(animExtAkimbo)

	WC_COMPARE_FIELD(numEvents, "%d")

	for (int i = 0; i < a.numEvents && i < b.numEvents; i++) {
		WepEvt& e1 = a.events[i];
		WepEvt& e2 = b.events[i];

		WC_COMPARE_FIELD_EV(evtType, "%d", i)
		WC_COMPARE_FIELD_EV(trigger, "%d", i)
		WC_COMPARE_FIELD_EV(triggerArg, "%d", i)
		WC_COMPARE_FIELD_EV(hasDelay, "%d", i)
		WC_COMPARE_FIELD_EV(delay, "%d", i)

		// this field doesn't matter
		e2.attackIdx = e1.attackIdx;

		if (memcmp(&e1, &e2, sizeof(WepEvt))) {
			ALERT(at_error, "Mismatch data on event %d (%s)\n", i, g_wc_evt_type_names[e1.evtType]);

			if (e1.evtType == WC_EVT_PUNCH && memcmp(&e1.punch, &e2.punch, sizeof(WepEvt::punch))) {
				ALERT(at_error, "Mismatch punch data too\n");
			}

			switch (e1.evtType) {
			case WC_EVT_IDLE_SOUND:
				WC_COMPARE_FIELD_EV(idleSound.sound, "%d", i)
				WC_COMPARE_FIELD_EV(idleSound.volume, "%d", i)
				break;
			case WC_EVT_PLAY_SOUND: {
				WC_COMPARE_FIELD_EV(playSound.sound, "%d", i)
				WC_COMPARE_FIELD_EV(playSound.numAdditionalSounds, "%d", i)
				for (int k = 0; k < MAX_WC_RANDOM_SELECTION; k++) {
					WC_COMPARE_FIELD_EV(playSound.additionalSounds[k], "%d", i)
				}
				WC_COMPARE_FIELD_EV(playSound.channel, "%d", i)
				WC_COMPARE_FIELD_EV(playSound.volume, "%d", i)
				WC_COMPARE_FIELD_EV(playSound.aiVol, "%d", i)
				WC_COMPARE_FIELD_EV(playSound.distantSound, "%d", i)
				WC_COMPARE_FIELD_EV(playSound.attn, "%d", i)
				WC_COMPARE_FIELD_EV(playSound.pitchMin, "%d", i)
				WC_COMPARE_FIELD_EV(playSound.pitchMax, "%d", i)
				WC_COMPARE_FIELD_EV(playSound.flags, "%d", i)
				break;
			}
			case WC_EVT_EJECT_SHELL:
				WC_COMPARE_FIELD_EV(ejectShell.model, "%d", i)
				WC_COMPARE_FIELD_EV(ejectShell.sound, "%d", i)
				WC_COMPARE_FIELD_EV(ejectShell.offsetForward, "%d", i)
				WC_COMPARE_FIELD_EV(ejectShell.offsetUp, "%d", i)
				WC_COMPARE_FIELD_EV(ejectShell.offsetRight, "%d", i)
				WC_COMPARE_FIELD_EV(ejectShell.velForward, "%d", i)
				WC_COMPARE_FIELD_EV(ejectShell.velUp, "%d", i)
				WC_COMPARE_FIELD_EV(ejectShell.velRight, "%d", i)
				WC_COMPARE_FIELD_EV(ejectShell.dirRand, "%d", i)
				WC_COMPARE_FIELD_EV(ejectShell.speedRand, "%d", i)
				break;
			case WC_EVT_PUNCH:
				WC_COMPARE_FIELD_EV(punch.x, "%d", i)
				WC_COMPARE_FIELD_EV(punch.y, "%d", i)
				WC_COMPARE_FIELD_EV(punch.z, "%d", i)
				WC_COMPARE_FIELD_EV(punch.flags, "%d", i)
				break;
			case WC_EVT_SET_BODY:
				WC_COMPARE_FIELD_EV(setBody.newBody, "%d", i)
				break;
			case WC_EVT_WEP_ANIM:
				WC_COMPARE_FIELD_EV(anim.flags, "%d", i)
				WC_COMPARE_FIELD_EV(anim.akimbo, "%d", i)
				WC_COMPARE_FIELD_EV(anim.numAnim, "%d", i)
				for (int k = 0; k < e1.anim.numAnim; k++) {
					WC_COMPARE_FIELD_EV(anim.anims[k], "%d", i)
				}
				break;
			case WC_EVT_BULLETS:
				WC_COMPARE_FIELD_EV(bullets.count, "%d", i)
				WC_COMPARE_FIELD_EV(bullets.burstDelay, "%d", i)
				WC_COMPARE_FIELD_EV(bullets.damage, "%d", i)
				WC_COMPARE_FIELD_EV(bullets.spreadX, "%d", i)
				WC_COMPARE_FIELD_EV(bullets.spreadY, "%d", i)
				WC_COMPARE_FIELD_EV(bullets.tracerColor, "%d", i)
				WC_COMPARE_FIELD_EV(bullets.tracerFreq, "%d", i)
				WC_COMPARE_FIELD_EV(bullets.flashSz, "%d", i)
				WC_COMPARE_FIELD_EV(bullets.flags, "%d", i)
				break;
			case WC_EVT_BEAM:
				WC_COMPARE_FIELD_EV(beam.flags, "%d", i)
				WC_COMPARE_FIELD_EV(beam.attachment, "%d", i)
				WC_COMPARE_FIELD_EV(beam.sprite, "%d", i)
				WC_COMPARE_FIELD_EV(beam.id, "%d", i)
				WC_COMPARE_FIELD_EV(beam.altMode, "%d", i)
				WC_COMPARE_FIELD_EV(beam.altTime, "%d", i)
				WC_COMPARE_FIELD_EV(beam.life, "%d", i)
				WC_COMPARE_FIELD_EV(beam.spreadX, "%d", i)
				WC_COMPARE_FIELD_EV(beam.spreadY, "%d", i)
				WC_COMPARE_FIELD_EV(beam.damage, "%d", i)
				WC_COMPARE_FIELD_EV(beam.distance, "%d", i)
				WC_COMPARE_FIELD_EV(beam.freq, "%d", i)
				WC_COMPARE_FIELD_EV(beam.width, "%d", i)
				WC_COMPARE_FIELD_EV(beam.widthAlt, "%d", i)
				WC_COMPARE_FIELD_EV(beam.noise, "%d", i)
				WC_COMPARE_FIELD_EV(beam.noiseAlt, "%d", i)
				WC_COMPARE_FIELD_EV(beam.scrollRate, "%d", i)
				WC_COMPARE_FIELD_EV(beam.scrollRateAlt, "%d", i)
				WC_COMPARE_FIELD_EV(beam.widthAlt, "%d", i)
				WC_COMPARE_FIELD_EV(beam.color.r, "%d", i)
				WC_COMPARE_FIELD_EV(beam.color.g, "%d", i)
				WC_COMPARE_FIELD_EV(beam.color.b, "%d", i)
				WC_COMPARE_FIELD_EV(beam.color.a, "%d", i)
				WC_COMPARE_FIELD_EV(beam.colorAlt.r, "%d", i)
				WC_COMPARE_FIELD_EV(beam.colorAlt.g, "%d", i)
				WC_COMPARE_FIELD_EV(beam.colorAlt.b, "%d", i)
				WC_COMPARE_FIELD_EV(beam.colorAlt.a, "%d", i)
				WC_COMPARE_FIELD_EV(beam.hasImpactSprite, "%d", i)
				WC_COMPARE_FIELD_EV(beam.impactSprite, "%d", i)
				WC_COMPARE_FIELD_EV(beam.impactSpriteFps, "%d", i)
				WC_COMPARE_FIELD_EV(beam.impactSpriteScale, "%d", i)
				WC_COMPARE_FIELD_EV(beam.impactSpriteColor.r, "%d", i)
				WC_COMPARE_FIELD_EV(beam.impactSpriteColor.g, "%d", i)
				WC_COMPARE_FIELD_EV(beam.impactSpriteColor.b, "%d", i)
				WC_COMPARE_FIELD_EV(beam.impactSpriteColor.a, "%d", i)
				break;
			case WC_EVT_PROJECTILE:
				WC_COMPARE_FIELD_EV(proj.type, "%d", i)
				WC_COMPARE_FIELD_EV(proj.entity_class, "%d", i)
				WC_COMPARE_FIELD_EV(proj.flags, "%d", i)
				WC_COMPARE_FIELD_EV(proj.spreadX, "%d", i)
				WC_COMPARE_FIELD_EV(proj.spreadY, "%d", i)
				WC_COMPARE_FIELD_EV(proj.world_event, "%d", i)
				WC_COMPARE_FIELD_EV(proj.monster_event, "%d", i)
				WC_COMPARE_FIELD_EV(proj.speed, "%f", i)
				WC_COMPARE_FIELD_EV(proj.life, "%f", i)
				WC_COMPARE_FIELD_EV(proj.elasticity, "%f", i)
				WC_COMPARE_FIELD_EV(proj.gravity, "%f", i)
				WC_COMPARE_FIELD_EV(proj.air_friction, "%f", i)
				WC_COMPARE_FIELD_EV(proj.water_friction, "%f", i)
				WC_COMPARE_FIELD_EV(proj.size, "%f", i)
				WC_COMPARE_FIELD_EV(proj.dir[0], "%f", i)
				WC_COMPARE_FIELD_EV(proj.dir[1], "%f", i)
				WC_COMPARE_FIELD_EV(proj.dir[2], "%f", i)
				WC_COMPARE_FIELD_EV(proj.model, "%d", i)
				WC_COMPARE_FIELD_EV(proj.move_snd, "%d", i)
				WC_COMPARE_FIELD_EV(proj.damage, "%f", i)
				WC_COMPARE_FIELD_EV(proj.damageBits, "%d", i)
				WC_COMPARE_FIELD_EV_STR(proj.sprite, i)
				WC_COMPARE_FIELD_EV(proj.sprite_color.r, "%d", i)
				WC_COMPARE_FIELD_EV(proj.sprite_color.g, "%d", i)
				WC_COMPARE_FIELD_EV(proj.sprite_color.b, "%d", i)
				WC_COMPARE_FIELD_EV(proj.sprite_color.a, "%d", i)
				WC_COMPARE_FIELD_EV(proj.angles[0], "%f", i)
				WC_COMPARE_FIELD_EV(proj.angles[1], "%f", i)
				WC_COMPARE_FIELD_EV(proj.angles[2], "%f", i)
				WC_COMPARE_FIELD_EV(proj.avel[0], "%f", i)
				WC_COMPARE_FIELD_EV(proj.avel[1], "%f", i)
				WC_COMPARE_FIELD_EV(proj.avel[2], "%f", i)
				WC_COMPARE_FIELD_EV(proj.offset[0], "%f", i)
				WC_COMPARE_FIELD_EV(proj.offset[1], "%f", i)
				WC_COMPARE_FIELD_EV(proj.offset[2], "%f", i)
				WC_COMPARE_FIELD_EV(proj.player_vel_inf[0], "%f", i)
				WC_COMPARE_FIELD_EV(proj.player_vel_inf[1], "%f", i)
				WC_COMPARE_FIELD_EV(proj.player_vel_inf[2], "%f", i)
				WC_COMPARE_FIELD_EV(proj.follow_mode, "%d", i)
				WC_COMPARE_FIELD_EV(proj.follow_radius, "%f", i)
				WC_COMPARE_FIELD_EV(proj.follow_angle, "%f", i)
				WC_COMPARE_FIELD_EV(proj.follow_time[0], "%f", i)
				WC_COMPARE_FIELD_EV(proj.follow_time[1], "%f", i)
				WC_COMPARE_FIELD_EV(proj.follow_time[2], "%f", i)
				WC_COMPARE_FIELD_EV_STR(proj.trail_spr, i)
				WC_COMPARE_FIELD_EV(proj.trail_life, "%d", i)
				WC_COMPARE_FIELD_EV(proj.trail_width, "%d", i)
				WC_COMPARE_FIELD_EV(proj.trail_color.r, "%d", i)
				WC_COMPARE_FIELD_EV(proj.trail_color.g, "%d", i)
				WC_COMPARE_FIELD_EV(proj.trail_color.b, "%d", i)
				WC_COMPARE_FIELD_EV(proj.trail_color.a, "%d", i)
				break;
			case WC_EVT_KICKBACK:
				WC_COMPARE_FIELD_EV(kickback.pushForce, "%d", i)
				WC_COMPARE_FIELD_EV(kickback.back, "%d", i)
				WC_COMPARE_FIELD_EV(kickback.right, "%d", i)
				WC_COMPARE_FIELD_EV(kickback.up, "%d", i)
				WC_COMPARE_FIELD_EV(kickback.globalUp, "%d", i)
				break;
			case WC_EVT_TOGGLE_STATE: {
				WC_COMPARE_FIELD_EV(toggleState.toggleMode, "%d", i)
				WC_COMPARE_FIELD_EV(toggleState.stateBits, "%d", i)
				break;
			}
			case WC_EVT_TOGGLE_ZOOM:
				WC_COMPARE_FIELD_EV(zoomToggle.zoomFov, "%d", i)
				WC_COMPARE_FIELD_EV(zoomToggle.zoomFov2, "%d", i)
				break;
			case WC_EVT_COOLDOWN:
				WC_COMPARE_FIELD_EV(cooldown.millis, "%d", i)
				WC_COMPARE_FIELD_EV(cooldown.targets, "%d", i)
				break;
			case WC_EVT_SET_GRAVITY:
				WC_COMPARE_FIELD_EV(setGravity.gravity, "%d", i)
				break;
			case WC_EVT_DLIGHT:
				WC_COMPARE_FIELD_EV(dlight.radius, "%d", i)
				WC_COMPARE_FIELD_EV(dlight.r, "%d", i)
				WC_COMPARE_FIELD_EV(dlight.g, "%d", i)
				WC_COMPARE_FIELD_EV(dlight.b, "%d", i)
				WC_COMPARE_FIELD_EV(dlight.life, "%d", i)
				WC_COMPARE_FIELD_EV(dlight.decayRate, "%d", i)
				break;
			case WC_EVT_SERVER:
				WC_COMPARE_FIELD_EV(server.type, "%d", i)
				WC_COMPARE_FIELD_EV(server.iuser1, "%d", i)
				WC_COMPARE_FIELD_EV(server.iuser2, "%d", i)
				WC_COMPARE_FIELD_EV(server.iuser3, "%d", i)
				WC_COMPARE_FIELD_EV(server.iuser4, "%d", i)
				WC_COMPARE_FIELD_EV(server.fuser1, "%f", i)
				WC_COMPARE_FIELD_EV(server.fuser2, "%f", i)
				WC_COMPARE_FIELD_EV(server.fuser3, "%f", i)
				WC_COMPARE_FIELD_EV(server.fuser4, "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser1[0], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser1[1], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser1[2], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser1[3], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser2[0], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser2[1], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser2[2], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser2[3], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser3[0], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser3[1], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser3[2], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser3[3], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser4[0], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser4[1], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser4[2], "%f", i)
				WC_COMPARE_FIELD_EV(server.vuser4[3], "%f", i)
				WC_COMPARE_FIELD_EV(server.suser1, "%d", i)
				WC_COMPARE_FIELD_EV(server.suser2, "%d", i)
				WC_COMPARE_FIELD_EV(server.suser3, "%d", i)
				WC_COMPARE_FIELD_EV(server.suser4, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser1.r, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser1.g, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser1.b, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser1.a, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser2.r, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser2.g, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser2.b, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser2.a, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser3.r, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser3.g, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser3.b, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser3.a, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser4.r, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser4.g, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser4.b, "%d", i)
				WC_COMPARE_FIELD_EV(server.cuser4.a, "%d", i)
				break;
			case WC_EVT_MUZZLEFLASH:
				WC_COMPARE_FIELD_EV(muzzleFlash.brightness, "%d", i)
				break;
			case WC_EVT_HIDE_LASER:
				WC_COMPARE_FIELD_EV(laserHide.millis, "%d", i)
				break;
			case WC_EVT_SPRITETRAIL:
				WC_COMPARE_FIELD_EV(spriteTrail.sprite, "%d", i)
				WC_COMPARE_FIELD_EV(spriteTrail.count, "%d", i)
				WC_COMPARE_FIELD_EV(spriteTrail.scale, "%d", i)
				WC_COMPARE_FIELD_EV(spriteTrail.speed, "%d", i)
				WC_COMPARE_FIELD_EV(spriteTrail.speedNoise, "%d", i)
				break;
			case WC_EVT_DECAL:
				WC_COMPARE_FIELD_EV(decal.decalIdx, "%d", i)
				WC_COMPARE_FIELD_EV(decal.isGunshot, "%d", i)
				break;
			}
		}
	}
}


void wc_fwrite_param(FILE* f, void* dat, const char* name, int ptype) {
	// don't write default values
	switch (ptype) {
	case WC_PARAM_UINT8:	if (*(uint8_t*)dat == 0) return; else break;
	case WC_PARAM_ANIM:		break;
	case WC_PARAM_UINT16:	if (*(uint16_t*)dat == 0) return; else break;
	case WC_PARAM_UINT32:	if (*(uint32_t*)dat == 0) return; else break;
	case WC_PARAM_INT8:		if (*(int8_t*)dat == 0) return; else break;
	case WC_PARAM_INT16:	if (*(int16_t*)dat == 0) return; else break;
	case WC_PARAM_INT32:	if (*(int32_t*)dat == 0) return; else break;
	case WC_PARAM_FLOAT:	if (*(float*)dat == 0) return; else break;
	case WC_PARAM_RGBA: {
		RGBA& c = *(RGBA*)dat;
		if (!c.r && !c.g && !c.b && !c.a) return;
		break;
	}
	case WC_PARAM_VECTOR: {
		Vector& v = *(Vector*)dat;
		if (!v.x && !v.y && !v.z) return;
		break;
	}
	case WC_PARAM_SOUND_INDEX:	if (*(uint16_t*)dat == 0) return; else break;
	case WC_PARAM_MODEL_INDEX:	if (*(uint16_t*)dat == 0) return; else break;
	case WC_PARAM_TIME:			if (*(uint16_t*)dat == 0) return; else break;
	case WC_PARAM_ACCURACY_100:		if (*(uint16_t*)dat == 0) return; else break;
	case WC_PARAM_ACCURACY_UINT16:	if (*(uint16_t*)dat == 0) return; else break;
	case WC_PARAM_UINT16_PERCENT:	if (*(uint16_t*)dat == 0) return; else break;
	case WC_PARAM_STRING:			if (*(uint16_t*)dat == 0) return; else break;
	}

	fprintf(f, "%-24s= ", name);

	switch (ptype) {
	case WC_PARAM_UINT8:	fprintf(f, "%u\n", (uint32_t)(*(uint8_t*)dat)); break;
	case WC_PARAM_ANIM:		fprintf(f, "%u\n", (uint32_t)(*(uint8_t*)dat)); break;
	case WC_PARAM_UINT16:	fprintf(f, "%u\n", (uint32_t)(*(uint16_t*)dat)); break;
	case WC_PARAM_UINT32:	fprintf(f, "%u\n", (uint32_t)(*(uint32_t*)dat)); break;
	case WC_PARAM_INT8:		fprintf(f, "%d\n", (int32_t)(*(int8_t*)dat)); break;
	case WC_PARAM_INT16:	fprintf(f, "%d\n", (int32_t)(*(int16_t*)dat)); break;
	case WC_PARAM_INT32:	fprintf(f, "%d\n", (int32_t)(*(int32_t*)dat)); break;
	case WC_PARAM_FLOAT:	fprintf(f, "%f\n", *(float*)dat); break;
	case WC_PARAM_RGBA: {
		RGBA& c = *(RGBA*)dat;
		fprintf(f, "%u %u %u %u\n", (uint32_t)c.r, (uint32_t)c.g, (uint32_t)c.b, (uint32_t)c.a);
		break;
	}
	case WC_PARAM_VECTOR: {
		Vector& v = *(Vector*)dat;
		fprintf(f, "%f %f %f\n", v.x, v.y, v.z);
		break;
	}
	case WC_PARAM_SOUND_INDEX:		fprintf(f, "%s\n", INDEX_SOUND(*(uint16_t*)dat)); break;
	case WC_PARAM_MODEL_INDEX:		fprintf(f, "%s\n", INDEX_MODEL(*(uint16_t*)dat)); break;
	case WC_PARAM_TIME:				fprintf(f, "%ums\n", (uint32_t)(*(uint16_t*)dat)); break;
	case WC_PARAM_ACCURACY_UINT16:	fprintf(f, "%.2f\n", DEGREES_FROM_SPREAD(*(uint16_t*)dat)); break;
	case WC_PARAM_ACCURACY_100:		fprintf(f, "%.2f\n", (*(uint16_t*)dat) / 100.0f); break;
	case WC_PARAM_UINT16_PERCENT:	fprintf(f, "%.3f\n", (*(uint16_t*)dat) / 655.35f); break;
	case WC_PARAM_STRING:			fprintf(f, "%s\n", STRING(*(string_t*)dat)); break;
	default:
		ALERT(at_error, "Unknown param type %d\n", ptype);
	}
}

void wc_fwrite_param(FILE* f, void* dat, wep_param_t& param) {
	wc_fwrite_param(f, dat, param.name, param.type);
}

void wc_fwrite_params(FILE* f, const char* section_name, void* dat, int substruct) {
	if (section_name)
		fprintf(f, "\n[%s]\n", section_name);

	for (int i = 0; i < g_wc_params_count; i++) {
		wep_param_t& param = g_wc_params[i];

		if (param.substruct == substruct) {
			wc_fwrite_param(f, (uint8_t*)dat + param.offset, param);
		}
	}
}

void wc_fwrite_flags(FILE* f, const char** flagNames, int flags) {
	for (int i = 0; i < 32; i++) {
		if ((flags & (1 << i)) && flagNames[i] && flagNames[i][0]) {
			fprintf(f, "%-24s= true\n", flagNames[i]);
		}
	}
}

void wc_fwrite_weapon_settings(FILE* cfg, CustomWeaponParams& params) {
	fprintf(cfg, "[general]\n");

	wc_fwrite_params(cfg, NULL, &params, WC_SUBSTRUCT_NONE);
	wc_fwrite_flags(cfg, g_wc_flag_names, params.flags);

	const char* reloadNames[3] = { "reload", "reload_empty", "reload_pump" };

	for (int k = 0; k < ARRAY_SZ(params.reloadStage); k++) {
		const char* section = reloadNames[k];

		if ((params.flags & FL_WC_WEP_SHOTGUN_RELOAD) && k == 1) {
			section = "reload_shell";
		}

		if (params.reloadStage[k].time == 0)
			continue;

		wc_fwrite_params(cfg, section, &params.reloadStage[k], WC_SUBSTRUCT_RELOAD);
	}

	for (int k = 0; k < ARRAY_SZ(params.idles); k++) {
		if (params.idles[k].time == 0)
			continue;

		wc_fwrite_params(cfg, "idle", &params.idles[k], WC_SUBSTRUCT_IDLE);
	}

	if (params.flags & FL_WC_WEP_AKIMBO) {
		wc_fwrite_params(cfg, "akimbo", &params.akimbo, WC_SUBSTRUCT_AKIMBO);
		if (params.akimbo.accuracyX || params.akimbo.accuracyY) {
			if (params.akimbo.accuracyX == params.akimbo.accuracyY) {
				fprintf(cfg, "%-24s= %.2f\n", "accuracy", params.akimbo.accuracyX / 100.0f);
			}
			else {
				fprintf(cfg, "%-24s= %.2f %.2f\n", "accuracy", params.akimbo.accuracyX / 100.0f, params.akimbo.accuracyY / 100.0f);
			}
		}

		wc_fwrite_params(cfg, "reload_akimbo", &params.akimbo.reload, WC_SUBSTRUCT_AKIMBO_RELOAD);

		for (int k = 0; k < ARRAY_SZ(params.akimbo.idles); k++) {
			if (params.akimbo.idles[k].time == 0)
				continue;

			wc_fwrite_params(cfg, "idle_akimbo", &params.akimbo.idles[k], WC_SUBSTRUCT_AKIMBO_IDLE);
		}
	}

	if (params.flags & FL_WC_WEP_HAS_LASER) {
		wc_fwrite_params(cfg, "laser", &params.laser, WC_SUBSTRUCT_LASER);

		for (int k = 0; k < ARRAY_SZ(params.laser.idles); k++) {
			if (params.laser.idles[k].time == 0)
				continue;

			wc_fwrite_params(cfg, "idle_laser", &params.laser.idles[k], WC_SUBSTRUCT_LASER_IDLE);
		}
	}

	static int optBits[4] = { FL_WC_WEP_HAS_PRIMARY, FL_WC_WEP_HAS_SECONDARY, FL_WC_WEP_HAS_TERTIARY, FL_WC_WEP_HAS_ALT_PRIMARY };
	static const char* optNames[4] = { "primary", "secondary", "tertiary", "primary_alt" };

	for (int k = 0; k < ARRAY_SZ(params.shootOpts); k++) {
		if (!(params.flags & optBits[k]))
			continue;

		CustomWeaponShootOpts& opts = params.shootOpts[k];

		fprintf(cfg, "\n[%s]\n", optNames[k]);
		wc_fwrite_params(cfg, NULL, &opts, WC_SUBSTRUCT_SHOOT_OPTS);
		wc_fwrite_flags(cfg, g_wc_shoot_flag_names, opts.flags);

		if (opts.accuracyX || opts.accuracyY) {
			if (opts.accuracyX == opts.accuracyY) {
				fprintf(cfg, "%-24s= %.2f\n", "accuracy", opts.accuracyX / 100.0f);
			}
			else {
				fprintf(cfg, "%-24s= %.2f %.2f\n", "accuracy", opts.accuracyX / 100.0f, opts.accuracyY / 100.0f);
			}
		}

		if (opts.flags & FL_WC_SHOOT_IS_MELEE)
			wc_fwrite_params(cfg, NULL, &opts.melee, WC_SUBSTRUCT_SHOOT_OPTS_MELEE);
	}
}


void wc_fwrite_events(FILE* f, CustomWeaponParams& params) {
	for (int i = 0; i < params.numEvents; i++) {
		WepEvt& evt = params.events[i];

		uint16_t key = (evt.triggerArg << 5) | evt.trigger;
		if (key > ARRAY_SZ(g_wc_trigger_to_name) || g_wc_trigger_to_name[key].pool == NULL) {
			ALERT(at_error, "Invalid trigger key value %d\n", key);
			continue;
		}

		fprintf(f, "\n[%s]\n", g_wc_trigger_to_name[key].str());
		fprintf(f, "%-24s= %s\n", "action", g_wc_evt_type_names[evt.evtType]);

		if (evt.delay)
			fprintf(f, "%-24s= %ums\n", "delay", (uint32_t)evt.delay);

		switch (evt.evtType) {
		case WC_EVT_IDLE_SOUND:
			fprintf(f, "%-24s= %s\n", "sound", INDEX_SOUND(evt.idleSound.sound));
			if (evt.idleSound.volume != 127) fprintf(f, "%-24s= %.2f\n", "volume", evt.idleSound.volume / 127.0f);
			break;
		case WC_EVT_PLAY_SOUND: {
			fprintf(f, "%-24s= %s\n", "sound", INDEX_SOUND(evt.playSound.sound));
			for (int k = 0; k < evt.playSound.numAdditionalSounds; k++)
				fprintf(f, "%-24s= %s\n", ("sound" + to_string(k+2)).c_str(), INDEX_SOUND(evt.playSound.additionalSounds[k]));
			if (evt.playSound.channel != CHAN_STATIC) fprintf(f, "%-24s= %s\n", "channel", g_wc_evt_channel_names[evt.playSound.channel]);
			if (evt.playSound.volume != 255) fprintf(f, "%-24s= %.2f\n", "volume", evt.playSound.volume / 255.0f);
			if (evt.playSound.aiVol) fprintf(f, "%-24s= %s\n", "volume_for_ai", g_wc_evt_aivol_names[evt.playSound.aiVol]);
			if (evt.playSound.distantSound) fprintf(f, "%-24s= %s\n", "distant_sound", g_distant_sound_names[evt.playSound.distantSound]);
			if (evt.playSound.attn != (int)(ATTN_IDLE*64)) fprintf(f, "%-24s= %.2f\n", "attenuation", (evt.playSound.attn / 64.0f) + (0.5f / 64.0f));
			if (evt.playSound.pitchMin != 100 || evt.playSound.pitchMax != 100) {
				fprintf(f, "%-24s= %u\n", "pitch_min", (uint32_t)evt.playSound.pitchMin);
				fprintf(f, "%-24s= %u\n", "pitch_max", (uint32_t)evt.playSound.pitchMax);
			}
			wc_fwrite_flags(f, g_wc_evt_playsound_flag_names, evt.playSound.flags);
			break;
		}
		case WC_EVT_EJECT_SHELL:
			fprintf(f, "%-24s= %s\n", "model", INDEX_MODEL(evt.ejectShell.model));
			fprintf(f, "%-24s= %s\n", "sound", g_shell_sound_names[evt.ejectShell.sound]);
			fprintf(f, "%-24s= %d %d %d\n", "offset", (int)evt.ejectShell.offsetForward, (int)evt.ejectShell.offsetUp, (int)evt.ejectShell.offsetRight);
			if (evt.ejectShell.velForward || evt.ejectShell.velUp || evt.ejectShell.velRight) fprintf(f, "%-24s= %d %d %d\n", "velocity", (int)evt.ejectShell.velForward, (int)evt.ejectShell.velUp, (int)evt.ejectShell.velRight);
			if (evt.ejectShell.dirRand) fprintf(f, "%-24s= %u\n", "direction_randomness", (uint32_t)evt.ejectShell.dirRand);
			if (evt.ejectShell.speedRand) fprintf(f, "%-24s= %u\n", "speed_randomness", (uint32_t)evt.ejectShell.speedRand);
			break;
		case WC_EVT_PUNCH:
			fprintf(f, "%-24s= %.2f %.2f %.2f\n", "angles", FP_10_6_TO_FLOAT(evt.punch.x), FP_10_6_TO_FLOAT(evt.punch.y), FP_10_6_TO_FLOAT(evt.punch.z));
			wc_fwrite_flags(f, g_wc_evt_punch_flag_names, evt.punch.flags);
			break;
		case WC_EVT_SET_BODY:
			fprintf(f, "%-24s= %u\n", "new_body", (uint32_t)evt.setBody.newBody);
			break;
		case WC_EVT_WEP_ANIM:
			wc_fwrite_flags(f, g_wc_evt_anim_flag_names, evt.anim.flags);
			if (evt.anim.akimbo)
				fprintf(f, "%-24s= %s\n", "hand", g_wc_evt_anim_hand_names[evt.anim.akimbo]);

			fprintf(f, "%-24s=", "anims");
			for (int k = 0; k < evt.anim.numAnim; k++) {
				fprintf(f, " %u", evt.anim.anims[k]);
			}
			fprintf(f, "\n");
			break;
		case WC_EVT_BULLETS:
			fprintf(f, "%-24s= %u\n", "count", (uint32_t)evt.bullets.count);
			if (evt.bullets.burstDelay) fprintf(f, "%-24s= %u\n", "burst_delay", (uint32_t)evt.bullets.burstDelay);
			fprintf(f, "%-24s= %u\n", "damage", (uint32_t)evt.bullets.damage);
			if (evt.bullets.spreadX == evt.bullets.spreadY) {
				fprintf(f, "%-24s= %.2f\n", "accuracy", DEGREES_FROM_SPREAD(evt.bullets.spreadX));
			}
			else {
				fprintf(f, "%-24s= %.2f %.2f\n", "accuracy", DEGREES_FROM_SPREAD(evt.bullets.spreadX), DEGREES_FROM_SPREAD(evt.bullets.spreadY));
			}
			if (evt.bullets.tracerColor != WC_TRACER_COLOR_DEFAULT) fprintf(f, "%-24s= %s\n", "tracer_color", g_wc_evt_bullet_color_names[evt.bullets.tracerColor]);
			if (evt.bullets.tracerFreq != 1) fprintf(f, "%-24s= %u\n", "tracer_frequency", (uint32_t)evt.bullets.tracerFreq);
			if (evt.bullets.flashSz != WC_FLASH_NORMAL) fprintf(f, "%-24s= %s\n", "flash_size", g_wc_evt_bullet_flash_size_names[evt.bullets.flashSz]);
			wc_fwrite_flags(f, g_wc_evt_bullet_flag_names, evt.bullets.flags);
			break;
		case WC_EVT_BEAM:
			wc_fwrite_flags(f, g_wc_evt_beam_flag_names, evt.beam.flags);
			fprintf(f, "%-24s= %u\n", "attachment", (uint32_t)evt.beam.attachment);
			fprintf(f, "%-24s= %s\n", "sprite", INDEX_MODEL(evt.beam.sprite));
			fprintf(f, "%-24s= %u\n", "id", (uint32_t)evt.beam.id);
			if (evt.beam.altMode) fprintf(f, "%-24s= %s\n", "animate", g_wc_evt_beam_anim_names[evt.beam.altMode]);
			if (evt.beam.altMode) fprintf(f, "%-24s= %ums\n", "animate_time", evt.beam.altTime);
			fprintf(f, "%-24s= %ums\n", "life", (uint32_t)evt.beam.life);

			if (evt.beam.spreadX == evt.beam.spreadY) {
				fprintf(f, "%-24s= %.2f\n", "accuracy", DEGREES_FROM_SPREAD(evt.beam.spreadX));
			}
			else {
				fprintf(f, "%-24s= %.2f %.2f\n", "accuracy", DEGREES_FROM_SPREAD(evt.beam.spreadX), DEGREES_FROM_SPREAD(evt.bullets.spreadY));
			}

			fprintf(f, "%-24s= %u\n", "damage", (uint32_t)evt.beam.damage);
			fprintf(f, "%-24s= %u\n", "distance", (uint32_t)evt.beam.distance);
			fprintf(f, "%-24s= %ums\n", "frequency", (uint32_t)evt.beam.freq);
			fprintf(f, "%-24s= %u\n", "width", (uint32_t)evt.beam.width);
			if (evt.beam.altMode) fprintf(f, "%-24s= %u\n", "width_alt", (uint32_t)evt.beam.widthAlt);
			fprintf(f, "%-24s= %u\n", "noise", (uint32_t)evt.beam.noise);
			if (evt.beam.altMode) fprintf(f, "%-24s= %u\n", "noise_alt", (uint32_t)evt.beam.noiseAlt);
			fprintf(f, "%-24s= %u\n", "scroll_rate", (uint32_t)evt.beam.scrollRate);
			if (evt.beam.altMode) fprintf(f, "%-24s= %u\n", "scroll_rate_alt", (uint32_t)evt.beam.scrollRateAlt);
			fprintf(f, "%-24s= %u %u %u %u\n", "color", (uint32_t)evt.beam.color.r, (uint32_t)evt.beam.color.g, (uint32_t)evt.beam.color.b, (uint32_t)evt.beam.color.a);
			if (evt.beam.altMode) fprintf(f, "%-24s= %u %u %u %u\n", "color_alt", (uint32_t)evt.beam.colorAlt.r, (uint32_t)evt.beam.colorAlt.g, (uint32_t)evt.beam.colorAlt.b, (uint32_t)evt.beam.colorAlt.a);
			if (evt.beam.hasImpactSprite) {
				fprintf(f, "%-24s= %s\n", "impact_sprite", INDEX_MODEL(evt.beam.impactSprite));
				fprintf(f, "%-24s= %u\n", "impact_sprite_fps", (uint32_t)evt.beam.impactSpriteFps);
				fprintf(f, "%-24s= %u\n", "impact_sprite_scale", (uint32_t)evt.beam.impactSpriteScale);
				fprintf(f, "%-24s= %u %u %u %u\n", "impact_sprite_color", (uint32_t)evt.beam.impactSpriteColor.r, (uint32_t)evt.beam.impactSpriteColor.g, (uint32_t)evt.beam.impactSpriteColor.b, (uint32_t)evt.beam.impactSpriteColor.a);
			}
			break;
		case WC_EVT_PROJECTILE:
			fprintf(f, "%-24s= %s\n", "type", g_wc_evt_proj_type_names[evt.proj.type]);
			if (evt.proj.type == WC_PROJECTILE_CUSTOM || evt.proj.type == WC_PROJECTILE_OTHER)
				fprintf(f, "%-24s= %s\n", "entity_class", STRING(evt.proj.entity_class));
			wc_fwrite_flags(f, g_wc_evt_proj_flag_names, evt.proj.flags);
			
			if (evt.proj.spreadX == evt.proj.spreadY) {
				if (evt.proj.spreadX)
					fprintf(f, "%-24s= %.2f\n", "accuracy", DEGREES_FROM_SPREAD(evt.proj.spreadX));
			}
			else {
				fprintf(f, "%-24s= %.2f %.2f\n", "accuracy", DEGREES_FROM_SPREAD(evt.proj.spreadX), DEGREES_FROM_SPREAD(evt.bullets.spreadY));
			}

			if (evt.proj.world_event != WC_PROJ_ACT_IMPACT) fprintf(f, "%-24s= %s\n", "hit_world_action", g_wc_evt_proj_action_names[evt.proj.world_event]);
			if (evt.proj.monster_event != WC_PROJ_ACT_IMPACT) fprintf(f, "%-24s= %s\n", "hit_monster_action", g_wc_evt_proj_action_names[evt.proj.monster_event]);
			if (evt.proj.speed) fprintf(f, "%-24s= %u\n", "speed", (uint32_t)evt.proj.speed);
			if (evt.proj.life) fprintf(f, "%-24s= %ums\n", "life", (uint32_t)(evt.proj.life*1000));
			if (evt.proj.elasticity) fprintf(f, "%-24s= %.2f\n", "elasticity", evt.proj.elasticity);
			if (evt.proj.gravity) fprintf(f, "%-24s= %.2f\n", "gravity", evt.proj.gravity);
			if (evt.proj.air_friction) fprintf(f, "%-24s= %.2f\n", "air_friction", evt.proj.air_friction);
			if (evt.proj.water_friction) fprintf(f, "%-24s= %.2f\n", "water_friction", evt.proj.water_friction);
			if (evt.proj.size) fprintf(f, "%-24s= %.2f\n", "hull_size", evt.proj.size);
			if (*(Vector*)evt.proj.dir != g_vecZero) fprintf(f, "%-24s= %.2f %.2f %.2f\n", "direction", evt.proj.dir[0], evt.proj.dir[1], evt.proj.dir[2]);
			if (evt.proj.model) fprintf(f, "%-24s= %s\n", "model", INDEX_MODEL(evt.proj.model));
			if (evt.proj.move_snd) fprintf(f, "%-24s= %s\n", "move_sound", STRING(evt.proj.move_snd));
			if (evt.proj.damage) fprintf(f, "%-24s= %d\n", "damage", (int)(evt.proj.damage));
			if (evt.proj.damageBits) fprintf(f, "%-24s= %d\n", "damage_type", (int)(evt.proj.damageBits));
			if (evt.proj.sprite) {
				fprintf(f, "%-24s= %s\n", "sprite", STRING(evt.proj.sprite));
				fprintf(f, "%-24s= %u %u %u %u\n", "sprite_color", (uint32_t)evt.proj.sprite_color.r, (uint32_t)evt.proj.sprite_color.g, (uint32_t)evt.proj.sprite_color.b, (uint32_t)evt.proj.sprite_color.a);
			}
			if (*(Vector*)evt.proj.angles != g_vecZero) fprintf(f, "%-24s= %d %d %d\n", "angles", (int)evt.proj.angles[0], (int)evt.proj.angles[1], (int)evt.proj.angles[2]);
			if (*(Vector*)evt.proj.avel != g_vecZero) fprintf(f, "%-24s= %d %d %d\n", "angular_velocity", (int)evt.proj.avel[0], (int)evt.proj.avel[1], (int)evt.proj.avel[2]);
			if (*(Vector*)evt.proj.offset != g_vecZero) fprintf(f, "%-24s= %d %d %d\n", "offset", (int)evt.proj.offset[0], (int)evt.proj.offset[1], (int)evt.proj.offset[2]);
			if (*(Vector*)evt.proj.player_vel_inf != g_vecZero) fprintf(f, "%-24s= %.2f %.2f %.2f\n", "player_speed_influence", evt.proj.player_vel_inf[0], evt.proj.player_vel_inf[1], evt.proj.player_vel_inf[2]);
			if (evt.proj.follow_mode) {
				fprintf(f, "%-24s= %u\n", "follow_mode", (uint32_t)evt.proj.follow_mode);
				if (evt.proj.follow_radius) fprintf(f, "%-24s= %.2f\n", "follow_radius", evt.proj.follow_radius);
				if (evt.proj.follow_angle) fprintf(f, "%-24s= %.2f\n", "follow_angle", evt.proj.follow_angle);
				if (*(Vector*)evt.proj.dir != g_vecZero)fprintf(f, "%-24s= %.2f %.2f %.2f\n", "follow_time", evt.proj.follow_time[0], evt.proj.follow_time[1], evt.proj.follow_time[2]);
			}
			if (evt.proj.trail_spr) {
				fprintf(f, "%-24s= %s\n", "trail_sprite", STRING(evt.proj.trail_spr));
				if (evt.proj.trail_life) fprintf(f, "%-24s= %d\n", "trail_life", (uint32_t)evt.proj.trail_life);
				fprintf(f, "%-24s= %d\n", "trail_width", (uint32_t)evt.proj.trail_width);
				if (!evt.proj.trail_color.isEmpty()) fprintf(f, "%-24s= %u %u %u %u\n", "trail_color", (uint32_t)evt.proj.trail_color.r, (uint32_t)evt.proj.trail_color.g, (uint32_t)evt.proj.trail_color.b, (uint32_t)evt.proj.trail_color.a);
			}
			break;
		case WC_EVT_KICKBACK:
			fprintf(f, "%-24s= %d\n", "push_force", (int)evt.kickback.pushForce);
			if (evt.kickback.back) fprintf(f, "%-24s= %d\n", "percent_back", (int)evt.kickback.back);
			if (evt.kickback.right) fprintf(f, "%-24s= %d\n", "percent_right", (int)evt.kickback.right);
			if (evt.kickback.up) fprintf(f, "%-24s= %d\n", "percent_up", (int)evt.kickback.up);
			if (evt.kickback.globalUp) fprintf(f, "%-24s= %d\n", "percent_global_up", (int)evt.kickback.globalUp);
			break;
		case WC_EVT_TOGGLE_STATE: {
			fprintf(f, "%-24s= %s\n", "toggle_mode", g_wc_evt_togglestate_mode_names[evt.toggleState.toggleMode]);

			fprintf(f, "%-24s=", "toggled_states");
			for (int k = 0; k < 32; k++) {
				if (evt.toggleState.stateBits & (1 << k)) {
					fprintf(f, " %s", g_wc_evt_togglestate_flag_names[k]);
				}
			}
			fprintf(f, "\n");

			break;
		}
		case WC_EVT_TOGGLE_ZOOM:
			fprintf(f, "%-24s= %u\n", "zoom_fov", (uint32_t)evt.zoomToggle.zoomFov);
			if (evt.zoomToggle.zoomFov2) fprintf(f, "%-24s= %u\n", "zoom_fov2", (uint32_t)evt.zoomToggle.zoomFov2);
			break;
		case WC_EVT_COOLDOWN:
			fprintf(f, "%-24s= %ums\n", "time", (uint32_t)evt.cooldown.millis);
			wc_fwrite_flags(f, g_wc_evt_cooldown_flag_names, evt.cooldown.targets);
			break;
		case WC_EVT_SET_GRAVITY:
			fprintf(f, "%-24s= %d\n", "gravity", (int)evt.setGravity.gravity);
			break;
		case WC_EVT_DLIGHT:
			fprintf(f, "%-24s= %u\n", "radius", (uint32_t)evt.dlight.radius);
			fprintf(f, "%-24s= %u %u %u\n", "color", (uint32_t)evt.dlight.r, (uint32_t)evt.dlight.g, (uint32_t)evt.dlight.b);
			fprintf(f, "%-24s= %u\n", "life", (uint32_t)evt.dlight.life);
			fprintf(f, "%-24s= %u\n", "decay_rate", (uint32_t)evt.dlight.decayRate);
			break;
		case WC_EVT_SERVER:
			fprintf(f, "%-24s= %u\n", "type", (uint32_t)evt.server.type);
			if (evt.server.iuser1) fprintf(f, "%-24s= %d\n", "iuser1", evt.server.iuser1);
			if (evt.server.iuser2) fprintf(f, "%-24s= %d\n", "iuser2", evt.server.iuser2);
			if (evt.server.iuser3) fprintf(f, "%-24s= %d\n", "iuser3", evt.server.iuser3);
			if (evt.server.iuser4) fprintf(f, "%-24s= %d\n", "iuser4", evt.server.iuser4);
			if (evt.server.fuser1) fprintf(f, "%-24s= %f\n", "fuser1", evt.server.fuser1);
			if (evt.server.fuser1) fprintf(f, "%-24s= %f\n", "fuser2", evt.server.fuser2);
			if (evt.server.fuser1) fprintf(f, "%-24s= %f\n", "fuser3", evt.server.fuser3);
			if (evt.server.fuser1) fprintf(f, "%-24s= %f\n", "fuser4", evt.server.fuser4);
			if (evt.server.vuser1 != g_vecZero) fprintf(f, "%-24s= %f %f %f\n", "vuser1", evt.server.vuser1[0], evt.server.vuser1[1], evt.server.vuser1[2]);
			if (evt.server.vuser2 != g_vecZero) fprintf(f, "%-24s= %f %f %f\n", "vuser2", evt.server.vuser2[0], evt.server.vuser2[1], evt.server.vuser2[2]);
			if (evt.server.vuser3 != g_vecZero) fprintf(f, "%-24s= %f %f %f\n", "vuser3", evt.server.vuser3[0], evt.server.vuser3[1], evt.server.vuser3[2]);
			if (evt.server.vuser4 != g_vecZero) fprintf(f, "%-24s= %f %f %f\n", "vuser4", evt.server.vuser4[0], evt.server.vuser4[1], evt.server.vuser4[2]);
			if (evt.server.suser1) fprintf(f, "%-24s= %s\n", "suser1", STRING(evt.server.suser1));
			if (evt.server.suser2) fprintf(f, "%-24s= %s\n", "suser2", STRING(evt.server.suser2));
			if (evt.server.suser3) fprintf(f, "%-24s= %s\n", "suser3", STRING(evt.server.suser3));
			if (evt.server.suser4) fprintf(f, "%-24s= %s\n", "suser4", STRING(evt.server.suser4));
			if (!evt.server.cuser1.isEmpty()) fprintf(f, "%-24s= %u %u %u %u\n", "cuser1", (uint32_t)evt.server.cuser1.r, (uint32_t)evt.server.cuser1.g, (uint32_t)evt.server.cuser1.b, (uint32_t)evt.server.cuser1.a);
			if (!evt.server.cuser2.isEmpty()) fprintf(f, "%-24s= %u %u %u %u\n", "cuser2", (uint32_t)evt.server.cuser2.r, (uint32_t)evt.server.cuser2.g, (uint32_t)evt.server.cuser2.b, (uint32_t)evt.server.cuser2.a);
			if (!evt.server.cuser3.isEmpty()) fprintf(f, "%-24s= %u %u %u %u\n", "cuser3", (uint32_t)evt.server.cuser3.r, (uint32_t)evt.server.cuser3.g, (uint32_t)evt.server.cuser3.b, (uint32_t)evt.server.cuser3.a);
			if (!evt.server.cuser4.isEmpty()) fprintf(f, "%-24s= %u %u %u %u\n", "cuser4", (uint32_t)evt.server.cuser4.r, (uint32_t)evt.server.cuser4.g, (uint32_t)evt.server.cuser4.b, (uint32_t)evt.server.cuser4.a);
			if (evt.server.euser1) fprintf(f, "%-24s= %s\n", "euser1", "unsupported");
			if (evt.server.euser2) fprintf(f, "%-24s= %s\n", "euser2", "unsupported");
			if (evt.server.euser3) fprintf(f, "%-24s= %s\n", "euser3", "unsupported");
			if (evt.server.euser4) fprintf(f, "%-24s= %s\n", "euser4", "unsupported");
			break;
		case WC_EVT_MUZZLEFLASH:
			fprintf(f, "%-24s= %s\n", "flash_size", g_wc_evt_bullet_flash_size_names[evt.muzzleFlash.brightness]);
			break;
		case WC_EVT_HIDE_LASER:
			fprintf(f, "%-24s= %ums\n", "time", (uint32_t)evt.laserHide.millis);
			break;
		case WC_EVT_SPRITETRAIL:
			fprintf(f, "%-24s= %s\n", "sprite", INDEX_MODEL(evt.spriteTrail.sprite));
			fprintf(f, "%-24s= %u\n", "count", (uint32_t)evt.spriteTrail.count);
			fprintf(f, "%-24s= %u\n", "scale", (uint32_t)evt.spriteTrail.scale);
			fprintf(f, "%-24s= %u\n", "speed", (uint32_t)evt.spriteTrail.speed);
			fprintf(f, "%-24s= %u\n", "speed_randomness", (uint32_t)evt.spriteTrail.speedNoise);
			break;
		case WC_EVT_DECAL:
			fprintf(f, "%-24s= %s\n", "texture", get_decal_name(evt.decal.decalIdx));
			fprintf(f, "%-24s= %s\n", "particles", evt.decal.isGunshot ? "true" : "false");
			break;
		}
	}
}

void wc_parse_event(const char* path, CustomWeaponParams& params, SettingsGroup& group) {
	uint16_t* val = g_wc_name_to_trigger.get(group.name.c_str());
	const char* action = group.keys.get("action");
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

	switch (evt.evtType) {
	case WC_EVT_IDLE_SOUND:
		ALERT(at_error, "this shouldn't happen\n");
		break;
	case WC_EVT_PLAY_SOUND: {
		evt.playSound.sound = wc_get_sound_index(kv, "sound");
		
		for (int k = 0; k < MAX_WC_RANDOM_SELECTION; k++) {
			uint16_t idx = wc_get_sound_index(kv, UTIL_VarArgs("sound%d", k));
			if (idx) {
				evt.playSound.additionalSounds[evt.playSound.numAdditionalSounds++] = idx;
			}
		}
		evt.playSound.channel = wc_get_named_value(kv, "channel", g_wc_evt_channel_names, CHAN_STATIC);
		evt.playSound.volume = wc_get_float(kv, "volume", 1.0f) * 255 + 0.5f;
		evt.playSound.aiVol = wc_get_named_value(kv, "volume_for_ai", g_wc_evt_aivol_names);
		evt.playSound.distantSound = wc_get_named_value(kv, "distant_sound", g_distant_sound_names);
		evt.playSound.attn = wc_get_float(kv, "attenuation", 2.0f) * 64;
		evt.playSound.pitchMin = wc_get_int(kv, "pitch_min", 100);
		evt.playSound.pitchMax = wc_get_int(kv, "pitch_max", 100);
		evt.playSound.flags = wc_read_flags(path, group, g_wc_evt_playsound_flag_names);

		// use smaller event if most params are default
		if (evt.playSound.channel == CHAN_STATIC && evt.playSound.aiVol == 0 && evt.playSound.distantSound == 0
			&& evt.playSound.attn == ATTN_IDLE * 64 && evt.playSound.pitchMin == 100 && evt.playSound.pitchMax == 100
			&& evt.playSound.flags == 0 && evt.playSound.numAdditionalSounds == 0)
		{
			uint16_t soundIdx = evt.playSound.sound;
			memset(&evt.playSound, 0, sizeof(evt.playSound));
			evt.idleSound.sound = soundIdx;
			evt.idleSound.volume = wc_get_float(kv, "volume", 1.0f) * 127 + 0.5;
			evt.evtType = WC_EVT_IDLE_SOUND;
		}
		break;
	}
	case WC_EVT_EJECT_SHELL: {
		evt.ejectShell.model = wc_get_model_index(kv, "model");
		evt.ejectShell.sound = wc_get_named_value(kv, "sound", g_shell_sound_names);

		Vector offset = wc_get_vector(kv, "offset");
		evt.ejectShell.offsetForward = offset.x;
		evt.ejectShell.offsetUp = offset.y;
		evt.ejectShell.offsetRight = offset.z;

		Vector velocity = wc_get_vector(kv, "velocity");
		evt.ejectShell.velForward = velocity.x;
		evt.ejectShell.velRight = velocity.y;
		evt.ejectShell.velUp = velocity.z;

		evt.ejectShell.dirRand = wc_get_int(kv, "direction_randomness");
		evt.ejectShell.speedRand = wc_get_int(kv, "speed_randomness");

		evt.ejectShell.hasVel = evt.ejectShell.velForward || evt.ejectShell.velRight || evt.ejectShell.velUp;
		evt.ejectShell.hasRand = evt.ejectShell.dirRand || evt.ejectShell.speedRand;
		break;
	}
	case WC_EVT_PUNCH: {
		Vector angles = wc_get_vector(kv, "angles");
		evt.punch.x = FLOAT_TO_FP_10_6(angles.x);
		evt.punch.y = FLOAT_TO_FP_10_6(angles.y);
		evt.punch.z = FLOAT_TO_FP_10_6(angles.z);
		evt.punch.flags = wc_read_flags(path, group, g_wc_evt_punch_flag_names);
		break;
	}
	case WC_EVT_SET_BODY:
		evt.setBody.newBody = wc_get_int(kv, "new_body");
		break;
	case WC_EVT_WEP_ANIM: {
		evt.anim.flags = wc_read_flags(path, group, g_wc_evt_anim_flag_names);
		evt.anim.akimbo = wc_get_named_value(kv, "hand", g_wc_evt_anim_hand_names);

		vector<string> anims = wc_get_strings(kv, "anims");
		for (int i = 0; i < anims.size() && i < MAX_WC_RANDOM_SELECTION; i++) {
			evt.anim.anims[i] = atoi(anims[i].c_str());
			evt.anim.numAnim++;
		}

		if (anims.size() > MAX_WC_RANDOM_SELECTION) {
			ALERT(at_error, "%s (line %d): Too many animations in key '%s' for group '%s'.\n",
				path, group.lineno, "anims", group.name.c_str());
		}
		break;
	}
	case WC_EVT_BULLETS: {
		evt.bullets.count = wc_get_int(kv, "count");
		evt.bullets.burstDelay = wc_get_int(kv, "burst_delay");
		evt.bullets.damage = wc_get_int(kv, "damage");
		evt.bullets.tracerColor = wc_get_named_value(kv, "tracer_color", g_wc_evt_bullet_color_names, WC_TRACER_COLOR_DEFAULT);
		evt.bullets.tracerFreq = wc_get_float(kv, "tracer_frequency", 1);
		evt.bullets.flashSz = wc_get_named_value(kv, "flash_size", g_wc_evt_bullet_flash_size_names, WC_FLASH_NORMAL);
		evt.bullets.flags = wc_read_flags(path, group, g_wc_evt_bullet_flag_names);

		float accuracyX, accuracyY;
		if (wc_read_accuracy(path, group, "accuracy", accuracyX, accuracyY)) {
			evt.bullets.spreadX = FLOAT_TO_SPREAD(UTIL_ConeFromDegrees(accuracyX).x);
			evt.bullets.spreadY = FLOAT_TO_SPREAD(UTIL_ConeFromDegrees(accuracyY).x);
		}
		break;
	}
	case WC_EVT_BEAM: {
		evt.beam.flags = wc_read_flags(path, group, g_wc_evt_beam_flag_names);
		evt.beam.attachment = wc_get_int(kv, "attachment");
		evt.beam.sprite = wc_get_model_index(kv, "sprite");
		evt.beam.id = wc_get_int(kv, "id");
		evt.beam.altMode = wc_get_named_value(kv, "animate", g_wc_evt_beam_anim_names);
		evt.beam.altTime = wc_get_time(path, group, "animate_time");
		evt.beam.life = wc_get_time(path, group, "life");
		evt.beam.damage = wc_get_int(kv, "damage");
		evt.beam.distance = wc_get_int(kv, "distance");
		evt.beam.freq = wc_get_time(path, group, "frequency");
		evt.beam.width = wc_get_int(kv, "width");
		evt.beam.noise = wc_get_int(kv, "noise");
		evt.beam.noiseAlt = wc_get_int(kv, "noise_alt");
		evt.beam.scrollRate = wc_get_int(kv, "scroll_rate");
		evt.beam.scrollRateAlt = wc_get_int(kv, "scroll_rate_alt");
		evt.beam.color = wc_get_rgba(kv, "color");
		evt.beam.colorAlt = wc_get_rgba(kv, "color_alt");
		evt.beam.impactSprite = wc_get_model_index(kv, "impact_sprite");
		evt.beam.impactSpriteFps = wc_get_int(kv, "impact_sprite_fps");
		evt.beam.impactSpriteScale = wc_get_int(kv, "impact_sprite_scale");
		evt.beam.impactSpriteColor = wc_get_rgba(kv, "impact_sprite_color");

		float accuracyX, accuracyY;
		if (wc_read_accuracy(path, group, "accuracy", accuracyX, accuracyY)) {
			evt.bullets.spreadX = FLOAT_TO_SPREAD(UTIL_ConeFromDegrees(accuracyX).x + (0.5f / 65535.0f));
			evt.bullets.spreadY = FLOAT_TO_SPREAD(UTIL_ConeFromDegrees(accuracyY).x + (0.5f / 65535.0f));
		}
		break;
	}
	case WC_EVT_PROJECTILE: {
		evt.proj.type = wc_get_named_value(kv, "type", g_wc_evt_proj_type_names);
		evt.proj.entity_class = wc_get_string(kv, "entity_class");
		evt.proj.flags = wc_read_flags(path, group, g_wc_evt_proj_flag_names);
		evt.proj.world_event = (WeaponCustomProjectileAction)wc_get_named_value(kv, "hit_world_action", g_wc_evt_proj_action_names, WC_PROJ_ACT_IMPACT);
		evt.proj.monster_event = (WeaponCustomProjectileAction)wc_get_named_value(kv, "hit_monster_action", g_wc_evt_proj_action_names, WC_PROJ_ACT_IMPACT);
		evt.proj.speed = wc_get_int(kv, "speed");
		evt.proj.life = wc_get_time(path, group, "life") / 1000.0f;
		evt.proj.elasticity = wc_get_float(kv, "elasticity");
		evt.proj.gravity = wc_get_float(kv, "gravity");
		evt.proj.air_friction = wc_get_float(kv, "air_friction");
		evt.proj.water_friction = wc_get_float(kv, "water_friction");
		evt.proj.size = wc_get_float(kv, "hull_size");
		*(Vector*)evt.proj.dir = wc_get_vector(kv, "direction");
		evt.proj.model = wc_get_model_index(kv, "model");
		evt.proj.move_snd = wc_get_string(kv, "move_sound");
		evt.proj.damage = wc_get_int(kv, "damage");
		evt.proj.damageBits = wc_get_int(kv, "damage_type");
		evt.proj.damageBits = wc_get_int(kv, "damage_type");
		evt.proj.sprite = wc_get_string(kv, "sprite");
		evt.proj.sprite_color = wc_get_rgba(kv, "sprite_color");
		*(Vector*)evt.proj.angles = wc_get_vector(kv, "angles");
		*(Vector*)evt.proj.avel = wc_get_vector(kv, "angular_velocity");
		*(Vector*)evt.proj.offset = wc_get_vector(kv, "offset");
		*(Vector*)evt.proj.player_vel_inf = wc_get_vector(kv, "player_speed_influence");
		*(Vector*)evt.proj.player_vel_inf = wc_get_vector(kv, "player_speed_influence");
		evt.proj.follow_mode = wc_get_int(kv, "follow_mode");
		evt.proj.follow_radius = wc_get_float(kv, "follow_radius");
		evt.proj.follow_angle = wc_get_float(kv, "follow_angle");
		*(Vector*)evt.proj.follow_time = wc_get_vector(kv, "follow_time");
		evt.proj.trail_spr = wc_get_string(kv, "trail_sprite");
		evt.proj.trail_life = wc_get_int(kv, "trail_life");
		evt.proj.trail_width = wc_get_int(kv, "trail_width");
		evt.proj.trail_color = wc_get_rgba(kv, "trail_color");

		float accuracyX, accuracyY;
		if (wc_read_accuracy(path, group, "accuracy", accuracyX, accuracyY)) {
			evt.proj.spreadX = FLOAT_TO_SPREAD(UTIL_ConeFromDegrees(accuracyX).x + (0.5f / 65535.0f));
			evt.proj.spreadY = FLOAT_TO_SPREAD(UTIL_ConeFromDegrees(accuracyY).x + (0.5f / 65535.0f));
		}
		break;
	}
	case WC_EVT_KICKBACK:
		evt.kickback.pushForce = wc_get_int(kv, "push_force");
		evt.kickback.back = wc_get_int(kv, "percent_back");
		evt.kickback.right = wc_get_int(kv, "percent_right");
		evt.kickback.up = wc_get_int(kv, "percent_up");
		evt.kickback.globalUp = wc_get_int(kv, "percent_global_up");
		break;
	case WC_EVT_TOGGLE_STATE: {
		evt.toggleState.toggleMode = wc_get_named_value(kv, "toggle_mode", g_wc_evt_togglestate_mode_names);

		vector<string> states = wc_get_strings(kv, "toggled_states");
		for (int k = 0; k < states.size(); k++) {
			for (int j = 0; j < 32; j++) {
				if (g_wc_evt_togglestate_flag_names[j] && g_wc_evt_togglestate_flag_names[j] == states[k]) {
					evt.toggleState.stateBits |= 1 << j;
					break;
				}
			}
		}
		break;
	}
	case WC_EVT_TOGGLE_ZOOM:
		evt.zoomToggle.zoomFov = wc_get_int(kv, "zoom_fov");
		evt.zoomToggle.zoomFov2 = wc_get_int(kv, "zoom_fov2");
		break;
	case WC_EVT_COOLDOWN:
		evt.cooldown.millis = wc_get_time(path, group, "time");
		evt.cooldown.targets = wc_read_flags(path, group, g_wc_evt_cooldown_flag_names);
		break;
	case WC_EVT_SET_GRAVITY:
		evt.setGravity.gravity = wc_get_int(kv, "gravity");
		break;
	case WC_EVT_DLIGHT: {
		evt.dlight.radius = wc_get_int(kv, "radius");
		evt.dlight.life = wc_get_int(kv, "life");
		evt.dlight.decayRate = wc_get_int(kv, "decay_rate");

		RGBA c = wc_get_rgba(kv, "color");
		evt.dlight.r = c.r;
		evt.dlight.g = c.g;
		evt.dlight.b = c.b;
		break;
	}
	case WC_EVT_SERVER:
		evt.server.type = wc_get_int(kv, "type");
		evt.server.iuser1 = wc_get_int(kv, "iuser1");
		evt.server.iuser2 = wc_get_int(kv, "iuser2");
		evt.server.iuser3 = wc_get_int(kv, "iuser3");
		evt.server.iuser4 = wc_get_int(kv, "iuser4");
		evt.server.fuser1 = wc_get_float(kv, "fuser1");
		evt.server.fuser2 = wc_get_float(kv, "fuser2");
		evt.server.fuser3 = wc_get_float(kv, "fuser3");
		evt.server.fuser4 = wc_get_float(kv, "fuser4");
		*(Vector*)evt.server.vuser1 = wc_get_vector(kv, "vuser1");
		*(Vector*)evt.server.vuser2 = wc_get_vector(kv, "vuser2");
		*(Vector*)evt.server.vuser3 = wc_get_vector(kv, "vuser3");
		*(Vector*)evt.server.vuser4 = wc_get_vector(kv, "vuser4");
		evt.server.suser1 = wc_get_string(kv, "suser1");
		evt.server.suser2 = wc_get_string(kv, "suser2");
		evt.server.suser3 = wc_get_string(kv, "suser3");
		evt.server.suser4 = wc_get_string(kv, "suser4");
		evt.server.cuser1 = wc_get_rgba(kv, "cuser1");
		evt.server.cuser2 = wc_get_rgba(kv, "cuser2");
		evt.server.cuser3 = wc_get_rgba(kv, "cuser3");
		evt.server.cuser4 = wc_get_rgba(kv, "cuser4");
		// euser can't be defined in a config
		break;
	case WC_EVT_MUZZLEFLASH:
		evt.muzzleFlash.brightness = wc_get_named_value(kv, "toggle_mode", g_wc_evt_bullet_flash_size_names);
		break;
	case WC_EVT_HIDE_LASER:
		evt.laserHide.millis = wc_get_time(path, group, "time");
		break;
	case WC_EVT_SPRITETRAIL:
		evt.spriteTrail.sprite = wc_get_model_index(kv, "sprite");
		evt.spriteTrail.count = wc_get_int(kv, "count");
		evt.spriteTrail.scale = wc_get_int(kv, "scale");
		evt.spriteTrail.speed = wc_get_int(kv, "speed");
		evt.spriteTrail.speedNoise = wc_get_int(kv, "speed_randomness");
		break;
	case WC_EVT_DECAL:
		evt.decal.decalIdx = wc_get_decal_idx(kv, "texture");
		evt.decal.isGunshot = wc_get_bool(kv, "particles");
		break;
	}

	params.events[params.numEvents++] = evt;
}


void UTIL_DumpCustomWeaponConfig(const char* path, CustomWeaponParams& params) {
	string fname = string("/weapons/") + path;
	
	FILE* cfg = UTIL_OpenFile(fname.c_str(), "w");

	if (!cfg)
		return;

	wc_fwrite_weapon_settings(cfg, params);

	fprintf(cfg, "\n\n\n//\n// Events\n//\n");
	wc_fwrite_events(cfg, params);

	fclose(cfg);
}

bool UTIL_ParseCustomWeaponConfig(const char* path, CustomWeaponParams& params) {
	vector<SettingsGroup> groups = parse_settings_groups(path);

	memset(&params, 0, sizeof(CustomWeaponParams));

	if (!groups.size())
		return false;

	int idleCount = 0;
	int akimboIdleCount = 0;
	int laserIdleCount = 0;

	for (SettingsGroup& group : groups) {
		if (group.name == "general") {
			wc_read_params(path, group, &params, WC_SUBSTRUCT_NONE);
			params.flags |= wc_read_flags(path, group, g_wc_flag_names);
		}
		else if (group.name == "reload") {
			wc_read_params(path, group, &params.reloadStage[0], WC_SUBSTRUCT_RELOAD);
		}
		else if (group.name == "reload_empty") {
			wc_read_params(path, group, &params.reloadStage[1], WC_SUBSTRUCT_RELOAD);
		}
		else if (group.name == "reload_shell") {
			params.flags |= FL_WC_WEP_SHOTGUN_RELOAD;
			wc_read_params(path, group, &params.reloadStage[1], WC_SUBSTRUCT_RELOAD);
		}
		else if (group.name == "reload_pump") {
			params.flags |= FL_WC_WEP_SHOTGUN_RELOAD;
			wc_read_params(path, group, &params.reloadStage[2], WC_SUBSTRUCT_RELOAD);
		}
		else if (group.name == "idle") {
			int maxIdles = ARRAY_SZ(params.idles);
			if (idleCount >= maxIdles) {
				ALERT(at_error, "%s (line %d): Too many [idle] sections (max is %d).\n",
					path, group.lineno, maxIdles);
				continue;
			}
			wc_read_params(path, group, &params.idles[idleCount], WC_SUBSTRUCT_IDLE);
			idleCount++;
		}
		else if (group.name == "primary") {
			params.flags |= FL_WC_WEP_HAS_PRIMARY;
			wc_read_shoot_opts(path, group, params, 0);
		}
		else if (group.name == "secondary") {
			params.flags |= FL_WC_WEP_HAS_SECONDARY;
			wc_read_shoot_opts(path, group, params, 1);
		}
		else if (group.name == "tertiary") {
			params.flags |= FL_WC_WEP_HAS_TERTIARY;
			wc_read_shoot_opts(path, group, params, 2);
		}
		else if (group.name == "primary_alt") {
			params.flags |= FL_WC_WEP_HAS_ALT_PRIMARY;
			wc_read_shoot_opts(path, group, params, 3);
		}
		else if (group.name == "akimbo") {
			params.flags |= FL_WC_WEP_AKIMBO;
			wc_read_params(path, group, &params.akimbo, WC_SUBSTRUCT_AKIMBO);

			float accuracyX, accuracyY;
			if (wc_read_accuracy(path, group, "accuracy", accuracyX, accuracyY)) {
				params.akimbo.accuracyX = accuracyX * 100 + 0.5f;
				params.akimbo.accuracyY = accuracyY * 100 + 0.5f;
			}
		}
		else if (group.name == "idle_akimbo") {
			int maxIdles = ARRAY_SZ(params.akimbo.idles);
			if (akimboIdleCount >= maxIdles) {
				ALERT(at_error, "%s (line %d): Too many [idle_akimbo] sections (max is %d).\n",
					path, group.lineno, maxIdles);
				continue;
			}
			wc_read_params(path, group, &params.akimbo.idles[akimboIdleCount], WC_SUBSTRUCT_AKIMBO_IDLE);
			akimboIdleCount++;
		}
		else if (group.name == "reload_akimbo") {
			wc_read_params(path, group, &params.akimbo.reload, WC_SUBSTRUCT_AKIMBO_RELOAD);
		}
		else if (group.name == "laser") {
			params.flags |= FL_WC_WEP_HAS_LASER;
			wc_read_params(path, group, &params.laser, WC_SUBSTRUCT_LASER);
		}
		else if (group.name == "idle_laser") {
			int maxIdles = ARRAY_SZ(params.laser.idles);
			if (laserIdleCount >= maxIdles) {
				ALERT(at_error, "%s (line %d): Too many [idle_laser] sections (max is %d).\n",
					path, group.lineno, maxIdles);
				continue;
			}
			wc_read_params(path, group, &params.laser.idles[laserIdleCount], WC_SUBSTRUCT_IDLE);
			laserIdleCount++;
		}
		else if (group.name.find("event.") == 0) {
			wc_parse_event(path, params, group);
		}
	}

	return true;
}

void UTIL_TestConfig(CWeaponCustom* wep) {
	CustomWeaponParams& wepParams = wep->params;
	UTIL_DumpCustomWeaponConfig(UTIL_VarArgs("%s.txt", STRING(wep->pev->classname)), wepParams);

	CustomWeaponParams cfgParams;
	UTIL_ParseCustomWeaponConfig(UTIL_VarArgs("%s.txt", STRING(wep->pev->classname)), cfgParams);

	compare_params(wepParams, cfgParams);
}