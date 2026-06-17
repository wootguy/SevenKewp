#pragma once
#include "StringPool.h"
#include "HashMap.h"
#include "wc_params.h"

enum WC_PARAM_TYPE {
	WC_PARAM_UINT8,
	WC_PARAM_UINT8_D100,	// percentage stored as a uint8_t
	WC_PARAM_UINT8_FLAGS,	// flags stored in an 8-bit int, written to config on a single line
	WC_PARAM_UINT8_ENUM,	// enum value stored in an 8-bit int
	WC_PARAM_UINT8_ARRAY_8,	// array of up to 8 uint8_t
	WC_PARAM_UINT16,
	WC_PARAM_UINT16_FLAGS,
	WC_PARAM_UINT16_D100,	// unsigned 100ths of an int (+/-327 accurate to 0.01)
	WC_PARAM_UINT16_D1000,	// unsigned 100ths of an int (+/-32 accurate to 0.001)
	WC_PARAM_UINT32,
	WC_PARAM_UINT32_FLAGS,
	WC_PARAM_INT8,
	WC_PARAM_INT16,
	WC_PARAM_INT32,
	WC_PARAM_INT32_SD1000,	// signed 1000ths of an int (+/-2.1mil accurate to 0.001) 
	WC_PARAM_RGB,
	WC_PARAM_RGBA,
	WC_PARAM_INT32_VEC3_SD1000,			// vector as an array of signed 1000ths. (+/-2,147,483 accurate to 0.001) 
	WC_PARAM_VECTOR_INT8,			// vector as an array of int8_t. No decimals.
	WC_PARAM_VEC2_SD100,			// vector as an array of signed 100ths. (+/-327 accurate to 0.01)
	WC_PARAM_VEC3_SD100,			// vector as an array of signed 1000ths. (+/-32 accurate to 0.001)
	WC_PARAM_VEC3_SD1000,			// vector as an array of signed 1000ths. (+/-32 accurate to 0.001)
	WC_PARAM_SOUND_INDEX,			// sound file path stored as an index
	WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2,// array of up to 8 sound indexes, one sound per config line, with key naming starting at index 2
	WC_PARAM_MODEL_INDEX,			// model file path stored as an index
	WC_PARAM_DECAL_INDEX,			// texture name stored as a decal index
	WC_PARAM_STRING_DELTA,			// string stored as a delta index for the string delta networking system
	WC_PARAM_TIME,					// time value stored as uint16_t
	WC_PARAM_STRING,
};

enum event_category {
	WC_EVT_CATEGORY_PRIMARY,
	WC_EVT_CATEGORY_PRIMARY_ALT,
	WC_EVT_CATEGORY_SECONDARY,
	WC_EVT_CATEGORY_TERTIARY,
	WC_EVT_CATEGORY_RELOAD,
	WC_EVT_CATEGORY_DEPLOY,
	WC_EVT_CATEGORY_IDLE,
	WC_EVT_CATEGORY_REACTION,
	WC_EVT_CATEGORY_STATE_CHANGE,
	WC_EVT_CATEGORY_UNKNOWN,
	WC_EVT_CATEGORY_TOTAL,
};

#define FL_FIELD_NO_NETWORK 1		// field is not networked to clients
#define FL_FIELD_NO_CFG 2			// field is calculated automatically and not saved to a config
#define FL_FIELD_ALWAYS_WRITE_CFG 4	// always write this field to the CFG, even if using the default value

struct field_desc_t {
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

struct struct_desc_t {
	const char* name;			// for config file
	field_desc_t* fields;
	int numFields;
	StringSet validFields;
};

extern struct_desc_t g_wc_desc_general;
extern struct_desc_t g_wc_desc_er_toggle;
extern struct_desc_t g_wc_desc_ammo;
extern struct_desc_t g_wc_desc_reload;
extern struct_desc_t g_wc_desc_akimbo;
extern struct_desc_t g_wc_desc_shoot_opts;
extern struct_desc_t g_wc_desc_laser;
extern struct_desc_t g_wc_desc_state_sprite;
extern struct_desc_t g_wc_desc_evt[WC_EVT_TOTAL];

extern struct_desc_t g_wc_desc_custom_ammo;

extern const char* g_wc_evt_trigger_names[128];
extern const char* g_wc_evt_trigger_arg_primary_names[32];
extern const char* g_wc_evt_trigger_clip_sp_names[32];
extern const char* g_wc_evt_trigger_impact_names[32];
extern const char* g_wc_evt_charge_names[32];
extern const char* g_wc_evt_category_names[32];

#define WC_MAX_TRIGGER_VALUES (64 * 256) // 64 trigger and 256 arg possibilities
extern const char* g_wc_evt_type_names[128];
extern mod_string_t g_wc_trigger_to_name[WC_MAX_TRIGGER_VALUES];

extern HashMap<uint16_t> g_wc_name_to_trigger; // maps a group name to an event trigger + argument value
extern HashMap<uint8_t> g_wc_name_to_action; // maps an action key value to its event number
extern StringPool g_wc_trigger_string_pool;

void init_weapon_custom_config_parser();

struct_desc_t* get_evt_desc(int idx);
EXPORT const char* describe_event(WepEvt& evt);
void set_valid_fields(struct_desc_t& desc);

int wc_get_field_bytes(field_desc_t& field);
std::string wc_get_field_str(field_desc_t& field, uint8_t* dat);

EXPORT bool is_server_side_event(int evtId);
void wc_post_parse_event(WepEvt& evt);
void wc_post_parse_struct(void* dat, struct_desc_t& desc);
int wc_get_event_category(int evt);