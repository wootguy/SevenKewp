#include "extdll.h"
#include "util.h"
#include "wc_config.h"
#include "skill.h"
#include "decals.h"
#include "Scheduler.h"
#include <fstream>

HashMap<WeaponConfigCache> g_customWeaponCache;
HashMap<AmmoConfigCache> g_customAmmoCache;
unordered_map<string, vector<HudIconDef>> g_weaponHudConfigCache;

bool g_autoConfigReload = false;

unordered_map<string_t, string> g_migrateSoundMap;
unordered_map<string_t, string> g_migrateModelMap;
unordered_map<string_t, string> g_migrateStringMap;
unordered_map<string_t, string> g_migrateCvarMap;
bool g_migrationDumpMode = false; // if true, use the mappings when writing sounds/models/strings
bool g_dumpingAmmoConfig = false; // adjusts config padding if true
bool g_parsingAltConfig = false;

void clear_weapon_custom_cache() {
	g_customWeaponCache.clear();
	g_customAmmoCache.clear();
	g_weaponHudConfigCache.clear();

	g_migrateSoundMap.clear();
	g_migrateModelMap.clear();
	g_migrateStringMap.clear();
	g_migrateCvarMap.clear();
}

//
// Functions for reading/writing each type of field
// Update this when adding new field types
//

int wc_get_int(const char* val);
float wc_get_float(const char* val);
void fprintf_float(FILE* f, int decimals, int val);
void fprintf_vec2(FILE* f, int decimals, int v1, int v2);
void fprintf_vec3(FILE* f, int decimals, int v1, int v2, int v3);

struct ivec3 {
	int v[3];
};

int wc_parse_dx_int(string str, int decimals) {
	int dot = str.find_first_of(".");
	if (dot != -1) {
		string whole = str.substr(0, dot);
		string decimal = str.substr(dot + 1);

		switch (decimals) {
		case 1000:	while (decimal.size() < 3) { decimal += "0"; } break;
		case 100:	while (decimal.size() < 2) { decimal += "0"; } break;
		default:
			ALERT(at_error, "Unhandled decimals for parser: %d\n", decimals);
			break;
		}

		int iwhole = atoi(whole.c_str());
		int idecimal = atoi(decimal.c_str());

		if (str.find_first_of("-") != -1) {
			return -(-iwhole * decimals + idecimal);
		}
		else {
			return iwhole * decimals + idecimal;
		}
	}
	else {
		return atoi(str.c_str()) * decimals;
	}
}

ivec3 UTIL_ParseVectorD(const char* pString, int decimals)
{
	ivec3 ivec;
	memset(&ivec, 0, sizeof(ivec3));

	vector<string> parts = splitString(pString, " \t");

	for (int i = 0; i < 3 && i < parts.size(); i++) {
		ivec.v[i] = wc_parse_dx_int(parts[i], decimals);
	}

	return ivec;
}



void wc_read_field(const char* fname, SettingsGroup& group, void* dat, const char* name, const char* val,
	int ptype, field_desc_t* field) {
	switch (ptype) {
	case WC_PARAM_UINT8:			*(uint8_t*)dat = wc_get_int(val); break;
	case WC_PARAM_UINT8_D100:		*(uint8_t*)dat = atof(val) * 100; break;
	case WC_PARAM_UINT16:			*(uint16_t*)dat = wc_get_int(val); break;
	case WC_PARAM_UINT32:			*(uint32_t*)dat = wc_get_int(val); break;
	case WC_PARAM_INT8:				*(int8_t*)dat = wc_get_int(val); break;
	case WC_PARAM_INT16:			*(int16_t*)dat = wc_get_int(val); break;
	case WC_PARAM_INT32:			*(int32_t*)dat = wc_get_int(val); break;
	case WC_PARAM_INT32_SD1000:		*(int32_t*)dat = FLOAT_TO_SD1000_32BIT(wc_get_float(val)); break;
	case WC_PARAM_RGBA:				*(RGBA*)dat = UTIL_ParseRGBA(val); break;
	case WC_PARAM_RGB:				*(RGB*)dat = UTIL_ParseRGB(val); break;
	case WC_PARAM_VECTOR_INT8: {
		Vector v = UTIL_ParseVector(val);
		int8_t* cur = (int8_t*)dat;
		cur[0] = v.x;
		cur[1] = v.y;
		cur[2] = v.z;
		break;
	}
	case WC_PARAM_VEC3_SD100: {
		ivec3 v = UTIL_ParseVectorD(val, 100);
		int16_t* cur = (int16_t*)dat;
		cur[0] = v.v[0];
		cur[1] = v.v[1];
		cur[2] = v.v[2];
		break;
	}
	case WC_PARAM_VEC3_SD1000: {
		ivec3 v = UTIL_ParseVectorD(val, 1000);
		int16_t* cur = (int16_t*)dat;
		cur[0] = v.v[0];
		cur[1] = v.v[1];
		cur[2] = v.v[2];
		break;
	}
	case WC_PARAM_INT32_VEC3_SD1000: {
		ivec3 v = UTIL_ParseVectorD(val, 1000);
		int32_t* cur = (int32_t*)dat;
		cur[0] = v.v[0];
		cur[1] = v.v[1];
		cur[2] = v.v[2];
		break;
	}
	case WC_PARAM_UINT16_D1000: *(uint16_t*)dat = FLOAT_TO_D1000(wc_get_float(val)); break;
	case WC_PARAM_UINT16_D100: *(uint16_t*)dat = FLOAT_TO_D100(wc_get_float(val)); break;
	case WC_PARAM_INT8_ARRAY_32:
	case WC_PARAM_UINT8_ARRAY_32: {
		WepEvtArr8* arr = (WepEvtArr8*)dat;
		vector<string> parts = splitString(val, " ");
		memset(arr->arr, 0, sizeof(arr->arr));
		arr->arrSz = 0;
		for (int i = 0; i < (int)parts.size(); i++) {
			if (parts[i].empty())
				continue;

			if (i >= MAX_WC_ARR_LEN) {
				ALERT(at_error, "%s (line %d): Too many values in key '%s' for group '%s'.\n",
					fname, group.lineno, name, group.name.c_str());
				break;
			}

			arr->arr[arr->arrSz++] = atoi(parts[i].c_str());
		}
		break;
	}
	case WC_PARAM_INT16_ARRAY_32:
	case WC_PARAM_UINT16_D100_ARRAY_32: {
		WepEvtArr16* arr = (WepEvtArr16*)dat;
		vector<string> parts = splitString(val, " ");
		memset(arr->arr, 0, sizeof(arr->arr));
		arr->arrSz = 0;
		for (int i = 0; i < (int)parts.size(); i++) {
			if (parts[i].empty())
				continue;

			if (i >= MAX_WC_ARR_LEN) {
				ALERT(at_error, "%s (line %d): Too many values in key '%s' for group '%s'.\n",
					fname, group.lineno, name, group.name.c_str());
				break;
			}

			if (ptype == WC_PARAM_INT16_ARRAY_32) 
				arr->arr[arr->arrSz++] = wc_get_int(parts[i].c_str());
			else
				arr->arr[arr->arrSz++] = wc_parse_dx_int(parts[i], 100);
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
	case WC_PARAM_UINT16_FLAGS:
	case WC_PARAM_UINT8_FLAGS: {
		vector<string> words = splitString(val, "+");
		uint32_t flags = 0;
		for (int k = 0; k < (int)words.size(); k++) {
			bool foundName = false;
			words[k] = trimSpaces(words[k]);

			for (int j = 0; j < field->valNamesSz; j++) {
				if (field->valNames[j] && field->valNames[j] == words[k]) {
					flags |= 1 << j;
					foundName = true;
					break;
				}
			}

			if (!foundName && words[k] != "0") {
				ALERT(at_error, "%s (line %d): Unknown value '%s' for '%s' in group '%s'.\n",
					fname, group.lineno, words[k].c_str(), name, group.name.c_str());
			}
		}

		if (ptype == WC_PARAM_UINT32_FLAGS)		*(uint32_t*)dat = flags;
		else if (ptype == WC_PARAM_UINT16_FLAGS)*(uint16_t*)dat = flags;
		else if (ptype == WC_PARAM_UINT8_FLAGS)	*(uint8_t*)dat = flags;

		break;
	}
	case WC_PARAM_UINT8_ENUM: {
		*(uint8_t*)dat = 0;

		bool foundName = false;

		for (int i = 0; i < field->valNamesSz; i++) {
			if (field->valNames[i] && !strcmp(val, field->valNames[i])) {
				*(uint8_t*)dat = i;
				foundName = true;
				break;
			}
		}

		if (!foundName && strcmp(val, "0")) {
			ALERT(at_error, "%s (line %d): Unknown value '%s' for '%s' in group '%s'.\n",
				fname, group.lineno, val, name, group.name.c_str());
		}

		break;
	}
	case WC_PARAM_SOUND_INDEX:	*(uint16_t*)dat = val ? PRECACHE_SOUND_NULLENT(val) : 0; break;
	case WC_PARAM_MODEL_INDEX:	*(uint16_t*)dat = val ? PRECACHE_MODEL_NULLENT(val) : 0; break;
	case WC_PARAM_DECAL_INDEX:	*(uint8_t*)dat = val ? DECAL_INDEX(val) : 0; break;
	case WC_PARAM_STRING_DELTA:	*(dstring_t*)dat = val ? QueueDeltaString(ALLOC_STRING(val)) : 0; break;
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
	case WC_PARAM_VEC2_SD100: {
		vector<string> parts = splitString(val, " ");
		if (parts.size() > 0)
			((uint16_t*)dat)[0] = wc_get_float(parts[0].c_str()) * 100;

		if (parts.size() > 1)
			((uint16_t*)dat)[1] = wc_get_float(parts[1].c_str()) * 100;
		else
			((uint16_t*)dat)[1] = ((uint16_t*)dat)[0];
		break;
	}
	case WC_PARAM_STRING:			*(string_t*)dat = val && val[0] ? ALLOC_STRING(val) : 0; break;
	default:
		ALERT(at_error, "%s (line %d): Unknown param type %d for key '%s' in group '%s'\n",
			fname, group.lineno, ptype, name, group.name.c_str());
	}
}

void wc_fwrite_field(FILE* f, void* dat, const char* name, int ptype, field_desc_t* field) {
	if (ptype != WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2)
		fprintf(f, g_dumpingAmmoConfig ? "%-13s= " : "%-24s= ", name);

	switch (ptype) {
	case WC_PARAM_UINT8:	fprintf(f, "%u\n", (uint32_t)(*(uint8_t*)dat)); break;
	case WC_PARAM_UINT8_D100:
		fprintf_float(f, 100, *(uint8_t*)dat);
		fprintf(f, "\n");
		break;
	case WC_PARAM_UINT16: {
		uint16_t val = *(uint16_t*)dat;
		if (g_migrationDumpMode && g_migrateCvarMap.find(val) != g_migrateCvarMap.end()) {
			fprintf(f, "%s\n", g_migrateCvarMap[val].c_str());
		}
		else {
			fprintf(f, "%u\n", (uint32_t)val);
		}
		break;
	}
	case WC_PARAM_UINT32:	fprintf(f, "%u\n", (uint32_t)(*(uint32_t*)dat)); break;
	case WC_PARAM_INT8:		fprintf(f, "%d\n", (int32_t)(*(int8_t*)dat)); break;
	case WC_PARAM_INT16:	fprintf(f, "%d\n", (int32_t)(*(int16_t*)dat)); break;
	case WC_PARAM_INT32:	fprintf(f, "%d\n", (int32_t)(*(int32_t*)dat)); break;
	case WC_PARAM_UINT32_FLAGS:
	case WC_PARAM_UINT16_FLAGS:
	case WC_PARAM_UINT8_FLAGS: {
		uint32_t flags = 0;
		if (ptype == WC_PARAM_UINT32_FLAGS)			flags = *(uint32_t*)dat;
		else if (ptype == WC_PARAM_UINT16_FLAGS)	flags = *(uint16_t*)dat;
		else if (ptype == WC_PARAM_UINT8_FLAGS)		flags = *(uint8_t*)dat;

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
	case WC_PARAM_INT32_SD1000: {
		int32_t v = *(int32_t*)dat;
		if (v < 0) {
			fprintf(f, "-");
			v = -v;
		}
		if (v % 10 == 0) {
			fprintf(f, "%u.%02u\n", v / 1000, (v / 10) % 100); // remove trailing 1000th zero
		}
		else {
			fprintf(f, "%u.%03u\n", v / 1000, v % 1000);
		}
		break;
	}
	case WC_PARAM_INT32_VEC3_SD1000: {
		int32_t* arr = (int32_t*)dat;
		for (int i = 0; i < 3; i++) {
			int32_t v = arr[i];
			if (i != 0)
				fprintf(f, " ");
			if (v < 0) {
				fprintf(f, "-");
				v = -v;
			}
			if (v % 10 == 0) {
				fprintf(f, "%u.%02u", v / 1000, (v / 10) % 100); // remove trailing 1000th zero
			}
			else {
				fprintf(f, "%u.%03u", v / 1000, v % 1000);
			}
		}
		fprintf(f, "\n");
		break;
	}
	case WC_PARAM_VECTOR_INT8: {
		int8_t* v = (int8_t*)dat;
		fprintf(f, "%d %d %d\n", (int)v[0], (int)v[1], (int)v[2]);
		break;
	}
	case WC_PARAM_VEC3_SD100: {
		int16_t* v = (int16_t*)dat;
		fprintf_vec3(f, 100, v[0], v[1], v[2]);
		fprintf(f, "\n");
		break;
	}
	case WC_PARAM_VEC3_SD1000: {
		int16_t* v = (int16_t*)dat;
		fprintf_vec3(f, 1000, v[0], v[1], v[2]);
		fprintf(f, "\n");
		break;
	}
	case WC_PARAM_UINT16_D1000: {
		fprintf_float(f, 1000, *(int16_t*)dat);
		fprintf(f, "\n");
		break;
	}
	case WC_PARAM_UINT16_D100: {
		fprintf_float(f, 100, *(int16_t*)dat);
		fprintf(f, "\n");
		break;
	}
	case WC_PARAM_INT8_ARRAY_32:
	case WC_PARAM_UINT8_ARRAY_32: {
		WepEvtArr8* arr = (WepEvtArr8*)dat;
		for (int i = 0; i < MAX_WC_ARR_LEN && i < arr->arrSz; i++) {
			if (i != 0)
				fprintf(f, " ");
			if (ptype == WC_PARAM_INT8_ARRAY_32)
				fprintf(f, "%d", (int8_t)arr->arr[i]);
			else
				fprintf(f, "%u", (uint8_t)arr->arr[i]);
		}
		fprintf(f, "\n");
		break;
	}
	case WC_PARAM_INT16_ARRAY_32:
	case WC_PARAM_UINT16_D100_ARRAY_32: {
		WepEvtArr16* arr = (WepEvtArr16*)dat;
		for (int i = 0; i < MAX_WC_ARR_LEN && i < arr->arrSz; i++) {
			if (i != 0)
				fprintf(f, " ");
			if (ptype == WC_PARAM_INT16_ARRAY_32)
				fprintf(f, "%d", (int16_t)arr->arr[i]);
			else
				fprintf_float(f, 100, arr->arr[i]);
		}
		fprintf(f, "\n");
		break;
	}
	case WC_PARAM_SOUND_INDEX_ARRAY_8_IDX2: {
		WepEvtArr16* arr = (WepEvtArr16*)dat;
		for (int i = 0; i < MAX_WC_RANDOM_SELECTION && i < arr->arrSz; i++) {
			string keyName = name + to_string(i + 2);
			const char* fmt = g_dumpingAmmoConfig ? "%-13s= %s\n" : "%-24s= %s\n";

			if (g_migrationDumpMode)
				fprintf(f, fmt, keyName.c_str(), g_migrateSoundMap[arr->arr[i]].c_str());
			else
				fprintf(f, fmt, keyName.c_str(), INDEX_SOUND(arr->arr[i]));
		}
		break;
	}
	case WC_PARAM_SOUND_INDEX:
		if (g_migrationDumpMode) {
			fprintf(f, "%s\n", g_migrateSoundMap[*(uint16_t*)dat].c_str());
		}
		else {
			fprintf(f, "%s\n", INDEX_SOUND(*(uint16_t*)dat));
		}
		break;
	case WC_PARAM_MODEL_INDEX:
		if (g_migrationDumpMode) {
			fprintf(f, "%s\n", g_migrateModelMap[*(uint16_t*)dat].c_str());
		}
		else {
			fprintf(f, "%s\n", INDEX_MODEL(*(uint16_t*)dat));
		}
		break;
	case WC_PARAM_DECAL_INDEX:		fprintf(f, "%s\n", get_decal_name(*(uint8_t*)dat)); break;
	case WC_PARAM_STRING_DELTA:		fprintf(f, "%s\n", GetDeltaString(*(dstring_t*)dat)); break;
	case WC_PARAM_TIME:				fprintf(f, "%ums\n", (uint32_t)(*(uint16_t*)dat)); break;
	case WC_PARAM_VEC2_SD100: {
		uint16_t* acc = (uint16_t*)dat;
		if (acc[0] == acc[1]) {
			fprintf_float(f, 100, acc[0]);
			fprintf(f, "\n");
		}
		else {
			fprintf_vec2(f, 100, acc[0], acc[1]);
			fprintf(f, "\n");
		}
		break;
	}
	case WC_PARAM_STRING:
		if (g_migrationDumpMode) {
			fprintf(f, "%s\n", g_migrateStringMap[*(string_t*)dat].c_str());
		}
		else {
			fprintf(f, "%s\n", STRING(*(string_t*)dat));
		}
		break;
	default:
		ALERT(at_error, "%s: Unknown param type %d\n", __func__, ptype);
	}
}


//
// Config parser
//

float wc_get_float(const char* val) {
#ifndef CLIENT_DLL
	skill_cvar_t** skillVal = g_skillCvars.get(val);
	if (skillVal) {
		return (*skillVal)->cvar.value;
	}
#endif
	float ret = atof(val);

	return ret;
}

void fprintf_float(FILE* f, int decimals, int val) {	
	const char* fmt = "";
	switch (decimals) {
	case 100: fmt = "%u.%02u"; break;
	case 1000: fmt = "%u.%02u"; break;
	default:
		ALERT(at_error, "Unimplemented decimal count for floats: %d\n", decimals);
		return;
	}
	
	if (val < 0) {
		fprintf(f, "-");
		val = -val;
	}

	fprintf(f, fmt, val / decimals, val % decimals);
}

void fprintf_vec2(FILE* f, int decimals, int v1, int v2) {
	fprintf_float(f, decimals, v1);
	fprintf(f, " ");
	fprintf_float(f, decimals, v2);
}

void fprintf_vec3(FILE* f, int decimals, int v1, int v2, int v3) {
	fprintf_float(f, decimals, v1);
	fprintf(f, " ");
	fprintf_float(f, decimals, v2);
	fprintf(f, " ");
	fprintf_float(f, decimals, v3);
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
		groups.push_back({ group_name, group_keys, group_lineno });
	}

	fclose(cfg);

	return groups;
}

bool wc_check_default_dat(field_desc_t& field, uint8_t* dat) {
	if (field.flags & FL_FIELD_ALWAYS_WRITE_CFG)
		return false; // always written

	static char defaultDat[128];
	memset(&defaultDat, 0, sizeof(defaultDat));

	static SettingsGroup dummyGroup;

	wc_read_field("null", dummyGroup, defaultDat, field.name, field.defaultValue, field.type, &field);

	if (field.type == WC_PARAM_STRING) {
		if (g_migrationDumpMode) {
			string_t ogDat = *(string_t*)dat;
			string remappedString = g_migrateStringMap[ogDat];

			return !strcmp(remappedString.c_str(), STRING(*(string_t*)defaultDat));
		}
		else {
			return !strcmp(STRING(*(string_t*)dat), STRING(*(string_t*)defaultDat));
		}
	}

	int bytes = wc_get_field_bytes(field);
	return !memcmp(dat, defaultDat, bytes);
}

void wc_fwrite_struct_fields(FILE* f, void* dat, struct_desc_t& desc) {
	for (int i = 0; i < desc.numFields; i++) {
		field_desc_t& field = desc.fields[i];
		uint8_t* fieldDat = ((uint8_t*)dat) + field.offset;

		if (field.flags & FL_FIELD_NO_CFG)
			continue;

		if (wc_check_default_dat(field, fieldDat))
			continue;

		if (field.type == WC_PARAM_UINT8_FLAGS || field.type == WC_PARAM_UINT16_FLAGS || field.type == WC_PARAM_UINT32_FLAGS) {
			bool anyFlagsToWrite = false;
			uint32_t flags = 0;

			if (field.type == WC_PARAM_UINT32_FLAGS)		flags = *(uint32_t*)fieldDat;
			else if (field.type == WC_PARAM_UINT16_FLAGS)	flags = *(uint16_t*)fieldDat;
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

void wc_read_struct(const char* fname, SettingsGroup& group, void* dat, struct_desc_t& desc) {
	for (int i = 0; i < desc.numFields; i++) {
		field_desc_t& field = desc.fields[i];
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

	wc_post_parse_struct(dat, desc);

	StringMap::iterator_t iter;
	while (group.keys.iterate(iter)) {
		if (!desc.validFields.hasKey(iter.key)) {
			ALERT(at_error, "%s (line %d): Invalid field '%s' in group '%s'.\n",
				fname, group.lineno, iter.key, group.name.c_str());
		}
	}
}

void wc_fwrite_events(FILE* f, CustomWeaponParams& params, int category) {
	for (int i = 0; i < params.numEvents; i++) {
		WepEvt& evt = params.events[i];

		if (category != -1 && category != wc_get_event_category(evt.trigger)) {
			continue;
		}

		uint16_t key = (evt.triggerArg << EVT_TRIGGER_BITS) | evt.trigger;
		if (key > ARRAY_SZ(g_wc_trigger_to_name) || g_wc_trigger_to_name[key].pool == NULL) {
			ALERT(at_error, "Invalid trigger key value %d\n", key);
			continue;
		}

		fprintf(f, "\n[event.%s.%s]\n", g_wc_trigger_to_name[key].str(), g_wc_evt_type_names[evt.evtType]);

		if (evt.delay)
			fprintf(f, "%-24s= %ums\n", "delay", (uint32_t)evt.delay);
		if (evt.offset)
			fprintf(f, "%-24s= %d\n", "offset", (int32_t)evt.offset);

		struct_desc_t* desc = get_evt_desc(evt.evtType);

		if (desc) {
			wc_fwrite_struct_fields(f, &evt, *desc);
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
	const char* offset = group.keys.get("offset");

	if (!val) {
		ALERT(at_error, "%s (line %d): Invalid event trigger '%s'.\n",
			path, group.lineno, header_parts[1].c_str());
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
	evt.trigger = *val & ((1 << EVT_TYPE_BITS) - 1);
	evt.triggerArg = *val >> EVT_TYPE_BITS;
	evt.delay = delay ? atoi(delay) : 0;
	evt.offset = offset ? atoi(offset) : 0;
	evt.hasTrigArg = evt.triggerArg != 0;
	evt.hasDelay = evt.delay != 0;
	evt.hasOffset = evt.offset != 0;

	struct_desc_t* desc = get_evt_desc(evt.evtType);

	if (!desc) {
		return;
	}

	wc_read_struct(path, group, &evt, *desc);
	wc_post_parse_event(evt);

	params.events[params.numEvents++] = evt;
}

void wc_read_shoot_opts(const char* path, SettingsGroup& group, CustomWeaponParams& params, int idx) {
	uint8_t* dat = ((uint8_t*)&params) + sizeof(CustomWeaponShootOpts) * idx;
	wc_read_struct(path, group, dat, g_wc_desc_shoot_opts);

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
	wc_fwrite_struct_fields(cfg, &params, g_wc_desc_general);

	static const char* ammoNames[4] = { "primary_ammo", "secondary_ammo" };
	for (int k = 0; k < ARRAY_SZ(params.ammoInfo); k++) {
		if (!params.ammoInfo[k].type)
			continue;

		fprintf(cfg, "\n[%s]\n", ammoNames[k]);
		wc_fwrite_struct_fields(cfg, dat + sizeof(WeaponCustomAmmoInfo) * k, g_wc_desc_ammo);
	}


	if (params.flags & FL_WC_WEP_AKIMBO) {
		fprintf(cfg, "\n[%s]\n", g_wc_desc_akimbo.name);
		wc_fwrite_struct_fields(cfg, &params, g_wc_desc_akimbo);
	}

	if (params.flags & FL_WC_WEP_HAS_LASER) {
		fprintf(cfg, "\n[%s]\n", g_wc_desc_laser.name);
		wc_fwrite_struct_fields(cfg, &params, g_wc_desc_laser);
	}

	if (params.flags & FL_WC_WEP_HAS_STATE_SPRITE) {
		fprintf(cfg, "\n[%s]\n", g_wc_desc_state_sprite.name);
		wc_fwrite_struct_fields(cfg, &params, g_wc_desc_state_sprite);
	}

	if (params.flags & FL_WC_WEP_HAS_E_R_TOGGLE) {
		fprintf(cfg, "\n[%s]\n", g_wc_desc_er_toggle.name);
		wc_fwrite_struct_fields(cfg, &params, g_wc_desc_er_toggle);
	}

	static int attackOrder[4] = { 0, 3, 1, 2 };
	static int optBits[4] = { FL_WC_WEP_HAS_PRIMARY, FL_WC_WEP_HAS_SECONDARY, FL_WC_WEP_HAS_TERTIARY, FL_WC_WEP_HAS_ALT_PRIMARY };
	static const char* optNames[4] = { "primary_attack", "secondary_attack", "tertiary_attack", "primary_alt_attack" };
	static int optEventCats[4] = { WC_EVT_CATEGORY_PRIMARY, WC_EVT_CATEGORY_SECONDARY, WC_EVT_CATEGORY_TERTIARY, WC_EVT_CATEGORY_PRIMARY_ALT };

	for (int k = 0; k < ARRAY_SZ(params.shootOpts); k++) {
		int idx = attackOrder[k];

		if (!(params.flags & optBits[idx]))
			continue;

		if (prettyPrint)
			write_section_header(cfg, g_wc_evt_category_names[optEventCats[idx]]);

		fprintf(cfg, "\n[%s]\n", optNames[idx]);
		wc_fwrite_struct_fields(cfg, dat + sizeof(CustomWeaponShootOpts) * idx, g_wc_desc_shoot_opts);

		if (prettyPrint)
			wc_fwrite_events(cfg, params, optEventCats[idx]);
	}

	bool hasReload = false;
	for (int k = 0; k < WC_RELOAD_STAGES; k++) {
		if (params.reloadStage[k].time != 0) {
			hasReload = true;
			break;
		}
	}

	if (hasReload) {
		if (prettyPrint)
			write_section_header(cfg, "Reload");
		
		const char* reloadNames[WC_RELOAD_STAGES] = { 
			"reload", "reload_empty", "reload_shell", "reload_pump", "reload_secondary", "reload_akimbo"
		};
		for (int k = 0; k < WC_RELOAD_STAGES; k++) {
			const char* section = reloadNames[k];

			if (params.reloadStage[k].time == 0)
				continue;

			fprintf(cfg, "\n[%s]\n", section);
			wc_fwrite_struct_fields(cfg, dat + sizeof(WeaponCustomReload) * k, g_wc_desc_reload);
		}

		if (prettyPrint)
			wc_fwrite_events(cfg, params, WC_EVT_CATEGORY_RELOAD);
	}
}

bool UTIL_ParseCustomWeaponConfig(const char* path, CustomWeaponParams& params, bool altParams) {
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

	uint8_t* dat = (uint8_t*)&params;

	for (SettingsGroup& group : groups) {
		if (group.name == "general") {
			wc_read_struct(path, group, &params, g_wc_desc_general);
		}
		else if (group.name == "primary_ammo") {
			wc_read_struct(path, group, dat + sizeof(WeaponCustomAmmoInfo) * 0, g_wc_desc_ammo);
		}
		else if (group.name == "secondary_ammo") {
			wc_read_struct(path, group, dat + sizeof(WeaponCustomAmmoInfo) * 1, g_wc_desc_ammo);
		}
		else if (group.name == "reload") {
			wc_read_struct(path, group, dat + sizeof(WeaponCustomReload) * WC_RELOAD_STAGE_START, g_wc_desc_reload);
		}
		else if (group.name == "reload_empty") {
			wc_read_struct(path, group, dat + sizeof(WeaponCustomReload) * WC_RELOAD_STAGE_START_EMPTY, g_wc_desc_reload);
		}
		else if (group.name == "reload_shell") {
			params.flags |= FL_WC_WEP_SHOTGUN_RELOAD;
			wc_read_struct(path, group, dat + sizeof(WeaponCustomReload) * WC_RELOAD_STAGE_SHELL, g_wc_desc_reload);
		}
		else if (group.name == "reload_pump") {
			params.flags |= FL_WC_WEP_SHOTGUN_RELOAD;
			wc_read_struct(path, group, dat + sizeof(WeaponCustomReload) * WC_RELOAD_STAGE_PUMP, g_wc_desc_reload);
		}
		else if (group.name == "reload_secondary") {
			wc_read_struct(path, group, dat + sizeof(WeaponCustomReload) * WC_RELOAD_STAGE_SECONDARY, g_wc_desc_reload);
		}
		else if (group.name == "reload_akimbo") {
			wc_read_struct(path, group, dat + sizeof(WeaponCustomReload) * WC_RELOAD_STAGE_AKIMBO, g_wc_desc_reload);
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
			wc_read_struct(path, group, &params, g_wc_desc_akimbo);
		}
		else if (group.name == "laser") {
			params.flags |= FL_WC_WEP_HAS_LASER;
			wc_read_struct(path, group, &params, g_wc_desc_laser);
		}
		else if (group.name == "weapon_state_icon") {
			params.flags |= FL_WC_WEP_HAS_STATE_SPRITE;
			wc_read_struct(path, group, &params, g_wc_desc_state_sprite);
		}
		else if (group.name == "e_r_toggle") {
			params.flags |= FL_WC_WEP_HAS_E_R_TOGGLE;
			wc_read_struct(path, group, &params, g_wc_desc_er_toggle);
		}
		else if (group.name.find("event.") == 0) {
			wc_parse_event(path, params, group);
		}
	}

	if (params.altParams) {
		params.flags |= FL_WC_WEP_HAS_ALT_PARAMS;
	}

	if (params.defaultSpriteV) {
		std::string shortened = STRING(params.defaultSpriteV);
		shortened = shortened.substr(8, shortened.size() - (8 + 4));
		params.vsprite_path = QueueDeltaString(ALLOC_STRING(shortened.c_str()));
	}

	if (params.erToggle.stateBits) {
		params.erToggle.hasToggleInfo = params.erToggle.stateBits != 0;
		params.erToggle.hasZoomInfo = params.erToggle.zoomLevels != 0;
	}

	if (!params.displayName) {
		ALERT(at_error, "Display name missing in config: %s\n", path);
	}

	if (params.flags & FL_WC_WEP_HAS_STATE_SPRITE) {
		WeaponCustomStateIcon& icon = params.stateIcon;
		const char* hud_path = UTIL_VarArgs("sprites/%s/%s.txt",
			STRING(params.hudFolder), STRING(params.classname));
		vector<HudIconDef> defs = UTIL_ParseWeaponHudConfig(hud_path);

		int customIdx = -1;
		StringSet uniqueNames;

		for (HudIconDef& def : defs) {
			const char* defName = def.name.c_str();
			bool isCustom = !g_default_weapon_hud_icon_names.hasKey(defName);

			if (isCustom) {
				if (!uniqueNames.hasKey(defName)) {
					uniqueNames.put(defName);
					customIdx++;
				}

				if (icon.defaultIcon && !strcmp(defName, STRING(icon.defaultIcon))) {
					icon.defaultIconIdx = customIdx;
				}
				if (icon.primaryAltIcon && !strcmp(defName, STRING(icon.primaryAltIcon))) {
					icon.primaryAltIconIdx = customIdx;
				}
				if (icon.semiAutoIcon && !strcmp(defName, STRING(icon.semiAutoIcon))) {
					icon.semiAutoIconIdx = customIdx;
				}
			}
		}
	}

	WeaponConfigCache newCache;
	newCache.fileModifiedTime = lastModified;
	newCache.params = params;
	g_customWeaponCache.put(relPath, newCache);

	// cache alternate params but don't link them yet
	if (!altParams && params.altParams) {
		CustomWeaponParams altParams;
		std::string path = UTIL_VarArgs("%s.txt", STRING(params.altParams));
		UTIL_ParseCustomWeaponConfig(path.c_str(), altParams, true);
	}
	

	return true;
}

bool UTIL_IsWeaponConfigUpdated(const char* path) {
	const char* relPath = path;
	string searchPath = string("weapons/") + path;
	string fpath = getGameFilePath(searchPath.c_str(), false);

	if (!fileExists(fpath.c_str())) {
		ALERT(at_error, "Weapon config not found: '%s'\n", searchPath.c_str());
		return false;
	}

	uint64_t lastModified = getFileModifiedTime(fpath.c_str());

	WeaponConfigCache* cache = g_customWeaponCache.get(relPath);
	return cache && lastModified != cache->fileModifiedTime;
}

bool UTIL_ParseCustomAmmoConfig(const char* path, CustomAmmoParams& params) {
	const char* relPath = path;
	string searchPath = string("weapons/") + path;
	string fpath = getGameFilePath(searchPath.c_str(), false);

	memset(&params, 0, sizeof(CustomAmmoParams));

	if (!fileExists(fpath.c_str())) {
		ALERT(at_error, "Ammo config not found: '%s'\n", searchPath.c_str());
		return false;
	}

	uint64_t lastModified = getFileModifiedTime(fpath.c_str());

	AmmoConfigCache* cache = g_customAmmoCache.get(relPath);
	if (cache) {
		if (lastModified == cache->fileModifiedTime) {
			params = cache->params;
			return true;
		}
		else {
			ALERT(at_logged, "Reloading modified ammo config: %s\n", relPath);
			g_customWeaponCache.del(relPath);
		}
	}

	path = fpath.c_str();

	ALERT(at_console, "Parsing ammo config: %s\n", path);

	vector<SettingsGroup> groups = parse_settings_groups(path);

	if (!groups.size())
		return false;

	for (SettingsGroup& group : groups) {
		if (group.name == "general") {
			wc_read_struct(path, group, &params, g_wc_desc_custom_ammo);
		}
	}

	AmmoConfigCache newCache;
	newCache.fileModifiedTime = lastModified;
	newCache.params = params;
	g_customAmmoCache.put(relPath, newCache);

	return true;
}

void UTIL_DumpCustomWeaponConfig(const char* path, CustomWeaponParams& params, bool prettyPrint) {
	g_dumpingAmmoConfig = false;
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
			case WC_EVT_CATEGORY_IDLE:
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

	ALERT(at_console, "Wrote weapon config: %s\n", fname.c_str());
}

void UTIL_DumpCustomAmmoConfig(const char* path, CustomAmmoParams& params, bool prettyPrint) {
	g_dumpingAmmoConfig = true;
	string fname = string("/weapons/") + path;

	FILE* cfg = UTIL_OpenFile(fname.c_str(), "w");

	if (!cfg)
		return;

	fprintf(cfg, "[%s]\n", g_wc_desc_custom_ammo.name);
	wc_fwrite_struct_fields(cfg, &params, g_wc_desc_custom_ammo);

	fclose(cfg);

	ALERT(at_console, "Wrote ammo config: %s\n", fname.c_str());
}

std::vector<HudIconDef> UTIL_ParseWeaponHudConfig(const char* path) {
	std::string fpath = getGameFilePath(path);
	std::ifstream infile(fpath);

	vector<HudIconDef> defs;

	if (fpath.empty() || !infile.is_open()) {
		ALERT(at_error, "Failed to load weapon hud config: %s\n", path);
		return defs;
	}

	auto cache = g_weaponHudConfigCache.find(fpath);
	if (cache != g_weaponHudConfigCache.end()) {
		ALERT(at_console, "Use cached hud config\n");
		return cache->second;
	}

	int lineNum = 0;
	std::string line;
	while (std::getline(infile, line))
	{
		lineNum++;
		vector<string> parts = splitString(line, " \t");
		if (parts.size() <= 1) { // empty line or the number of sprites
			continue;
		}
		if (parts.size() < 7) {
			ALERT(at_error, "Malformed HUD icon definition at %s (line %d)\n", fpath.c_str(), lineNum);
			continue;
		}

		HudIconDef def;
		def.name = parts[0];
		def.sprite = parts[1];
		def.resolution = atoi(parts[2].c_str());
		def.x = atoi(parts[3].c_str());
		def.y = atoi(parts[4].c_str());
		def.w = atoi(parts[5].c_str());
		def.h = atoi(parts[6].c_str());

		defs.push_back(def);
	}

	g_weaponHudConfigCache[fpath] = defs;

	return defs;
}

// 
// Config live reloading
//

void GivePlayerItemDelayed(EHANDLE h_plr, const char* cname) {
	CBasePlayer* plr = h_plr ? h_plr->MyPlayerPointer() : NULL;
	if (!plr)
		return;

	plr->GiveNamedItem(cname);
}

void SelectPlayerItemDelayed(EHANDLE h_plr, const char* cname) {
	CBasePlayer* plr = h_plr ? h_plr->MyPlayerPointer() : NULL;
	if (!plr)
		return;

	plr->SelectItem(cname);
}

void AutoReloadConfigs() {
	StringMap reloadConfigs;
	StringMap::iterator_t iter;
	while (g_customWeaponConfigs.iterate(iter)) {
		int* id = g_weaponClassIds.get(iter.key);

		if (id) {
			if (UTIL_IsWeaponConfigUpdated(iter.value)) {
				UTIL_ReloadWeaponConfigs();
				return;
			}
		}
	}

	StringSet::iterator_t iter2;
	while (g_customWeaponConfigsAlt.iterate(iter2)) {
		if (UTIL_IsWeaponConfigUpdated(iter2.key)) {
			UTIL_ReloadWeaponConfigs();
			return;
		}
	}
}

void UTIL_ReloadWeaponConfigs() {
	StringMap reloadConfigs;
	StringMap::iterator_t iter;
	while (g_customWeaponConfigs.iterate(iter)) {
		int* id = g_weaponClassIds.get(iter.key);

		if (id) {
			reloadConfigs.put(iter.key, iter.value);
			g_weaponClassIds.del(iter.key);
			g_weaponNames.del(iter.key);

			for (int i = 0; i < MAX_WEAPONS; i++) {
				ItemInfo& info = CBasePlayerItem::ItemInfoArray[i];
				if (*id == info.iId) {
					g_filledWeaponSlots[info.iSlot][info.iPosition] = NULL;
					memset(&info, 0, sizeof(ItemInfo));
				}
			}
		}
	}
	g_customWeaponConfigsAlt.clear();

	iter.reset();
	while (reloadConfigs.iterate(iter)) {
		bool firstFound = true;
		ALERT(at_console, "Reload weapon %s\n", iter.key);

		CBaseEntity* ent = NULL;
		while ((ent = UTIL_FindEntityByClassname(ent, iter.key)) != NULL) {
			CWeaponCustom* wep = ent->MyWeaponCustomPtr();
			if (!wep)
				continue;

			if (firstFound) {
				firstFound = false;
				ItemInfo info;
				wep->GetItemInfo(&info);
				g_filledWeaponSlots[info.iSlot][info.iPosition] = NULL;
				UTIL_RegisterWeapon(iter.key, iter.value);
			}

			wep->PrecacheEvents();
		}
	}

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);
		if (!plr)
			continue;

		EHANDLE h_plr = EHANDLE(plr->edict());

		CBaseEntity* pPendingItem = NULL;
		for (int k = 0; k < MAX_ITEM_TYPES; k++) {
			CBaseEntity* ent = plr->m_rgpPlayerItems[k].GetEntity();
			while (ent) {
				CBasePlayerItem* item = ent ? ent->GetWeaponPtr() : NULL;
				pPendingItem = item ? item->m_pNext.GetEntity() : NULL;
				if (item)
					g_Scheduler.SetTimeout(GivePlayerItemDelayed, 0.1f, h_plr, STRING(item->pev->classname));
				ent = pPendingItem;
			}
		}

		const char* activeItem = plr->m_pActiveItem ? STRING(plr->m_pActiveItem->pev->classname) : NULL;
		if (activeItem)
			g_Scheduler.SetTimeout(SelectPlayerItemDelayed, 0.2f, h_plr, activeItem);

		int	oldRgAmmo[MAX_AMMO_SLOTS];

		memcpy(oldRgAmmo, plr->m_rgAmmo, sizeof(plr->m_rgAmmo));

		plr->ReloadHUD();
		plr->RemoveAllItems(false);

		memcpy(plr->m_rgAmmo, oldRgAmmo, sizeof(plr->m_rgAmmo));
	}

	memset(g_wcPredDataSent, 0, sizeof(g_wcPredDataSent));
}

void UTIL_AutoReloadWeaponConfigs(bool enabled) {
	if (g_autoConfigReload == enabled)
		return;

	g_autoConfigReload = enabled;
	UTIL_ClientPrintAll(print_chat, UTIL_VarArgs("Automatic weapon config reloading is %s\n",
		g_autoConfigReload ? "ENABLED" : "DISABLED"));

	static ScheduledFunction func;

	if (g_autoConfigReload) {
		func = g_Scheduler.SetInterval(AutoReloadConfigs, 0.5f, -1);
	}
	else {
		g_Scheduler.RemoveTimer(func);
	}
}


//
// Config testing
//

void wc_compare_struct_fields(struct_desc_t& desc, void* struct1, void* struct2, int idx) {
	for (int i = 0; i < desc.numFields; i++) {
		field_desc_t& field = desc.fields[i];
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

void wc_compare_params(CustomWeaponParams& a, CustomWeaponParams& b) {
	uint8_t* dat1 = (uint8_t*)&a;
	uint8_t* dat2 = (uint8_t*)&b;

	wc_compare_struct_fields(g_wc_desc_general, &a, &b, 0);
	wc_compare_struct_fields(g_wc_desc_akimbo, dat1, dat2, 0);
	wc_compare_struct_fields(g_wc_desc_laser, dat1, dat2, 0);
	wc_compare_struct_fields(g_wc_desc_state_sprite, dat1, dat2, 0);
	wc_compare_struct_fields(g_wc_desc_er_toggle, dat1, dat2, 0);

	for (int i = 0; i < WC_RELOAD_STAGES; i++) {
		int offset = sizeof(WeaponCustomReload) * i;
		wc_compare_struct_fields(g_wc_desc_reload, dat1 + offset, dat2 + offset, i);
	}

	for (int i = 0; i < ARRAY_SZ(a.shootOpts); i++) {
		wc_compare_struct_fields(g_wc_desc_shoot_opts, &a, &b, i);
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

			struct_desc_t* desc = get_evt_desc(e1.evtType);

			if (desc)
				wc_compare_struct_fields(*desc, &e1, &e2, i);
		}
	}
}

void UTIL_TestConfig(CWeaponCustom* wep) {
	CustomWeaponParams& wepParams = wep->defaultParams;
	UTIL_DumpCustomWeaponConfig(UTIL_VarArgs("test/%s.txt", STRING(wep->pev->classname)), wepParams, false);

	CustomWeaponParams cfgParams;
	UTIL_ParseCustomWeaponConfig(UTIL_VarArgs("test/%s.txt", STRING(wep->pev->classname)), cfgParams);

	wc_compare_params(wepParams, cfgParams);
}


//
// Config migration
//

string migratePath = "valve/weapons/_migrate/raw";

unordered_map<string_t, string> readIndexMap(string fpath, StringMap remap) {
	std::ifstream infile(fpath);

	unordered_map<string_t, string> table;

	if (fpath.empty() || !infile.is_open()) {
		ALERT(at_error, "Failed to load index map file: %s\n", fpath.c_str());
		return table;
	}

	StringMap reverseMap;
	StringMap::iterator_t iter;
	while (remap.iterate(iter)) {
		reverseMap.put(iter.value, iter.key);
	}

	std::string line;
	while (std::getline(infile, line)) {
		vector<string> parts = splitString(line, "=");
		if (parts.size() == 2) {
			if (parts[1] == "sprites/hlcoop/laserdot.spr")
				ALERT(at_console, "");

			string_t idx = (unsigned int)strtoul(parts[0].c_str(), nullptr, 10);
			const char* unreplace = reverseMap.get(parts[1].c_str());
			table[idx] = unreplace ? unreplace : parts[1].c_str();
		}
	}

	return table;
}

void MigrateWeaponsBegin() {
#ifndef CLIENT_DLL
	std::vector<std::string> files, folders;
	UTIL_FindFilesRecursive("valve/weapons", files, folders);

	clear_weapon_custom_cache();

	if (UTIL_FolderExists(migratePath)) {
		if (!UTIL_DeleteFolderRecursive(migratePath)) {
			ALERT(at_error, "Failed to delete migration folder: %s\n", migratePath.c_str());
			return;
		}
	}

	if (!UTIL_CreateFolder(migratePath)) {
		ALERT(at_error, "Failed to create migration folder: %s\n", migratePath.c_str());
		return;
	}

	g_migrationDumpMode = true;

	// set unique values in cvars so they can be looked up when dumping later
	string cvarsListPath = migratePath + "/_cvars.txt";
	FILE* cvarList = fopen(cvarsListPath.c_str(), "w");
	if (cvarList) {
		int uniqueValue = 34500;
		HashMap<skill_cvar_t*>::iterator_t iter;
		while (g_skillCvars.iterate(iter)) {
			CVAR_SET_FLOAT((*iter.value)->cvar.name, uniqueValue);
			(*iter.value)->cvar.value = uniqueValue;
			fprintf(cvarList, "%d=%s\n", uniqueValue, (*iter.value)->cvar.name);
			uniqueValue++;
		}
		fclose(cvarList);
	}
	else {
		ALERT(at_error, "Failed to open cvar list file: %s\n", cvarsListPath.c_str());
	}

	for (std::string file : files) {
		string src = replaceString(file, "valve/weapons/", "");

		int lastSlash = src.find_last_of("/");
		string fname = lastSlash != -1 ? src.substr(lastSlash + 1) : src;

		bool isWeaponConfig = fname.find("weapon_") == 0;
		bool isAmmoConfig = fname.find("ammo_") == 0;

		if (!(isWeaponConfig || isAmmoConfig)) {
			continue;
		}
		if (fname.find(".txt") != fname.size() - 4) {
			continue;
		}
		if (file.find("valve/weapons/dump/") == 0
			|| file.find("valve/weapons/test/") == 0
			|| file.find("valve/weapons/_migrate/") == 0) {
			continue;
		}

		string dst = migratePath + "/" + replaceString(src, ".txt", ".dat");

		string folderPath = lastSlash != -1 ? src.substr(0, lastSlash) : "";
		folderPath = migratePath + "/" + folderPath;

		if (folderPath.size() && !UTIL_FolderExists(folderPath)) {
			if (!UTIL_CreateFolder(folderPath)) {
				ALERT(at_error, "Failed to create migration folder: %s\n", folderPath.c_str());
				continue;
			}
		}

		if (isWeaponConfig) {
			CustomWeaponParams params;
			if (UTIL_ParseCustomWeaponConfig(src.c_str(), params)) {
				FILE* dat = fopen(dst.c_str(), "wb");

				if (dat) {
					fwrite(&params, sizeof(CustomWeaponParams), 1, dat);
					fclose(dat);
					//ALERT(at_console, "Wrote %s\n", dst.c_str());
				}
				else {
					ALERT(at_error, "Failed to write weapon dat file: %s\n", dst.c_str());
				}
			}
			else {
				ALERT(at_error, "Failed to convert weapon config: %s\n", src.c_str());
			}
		}
		else {
			CustomAmmoParams params;
			if (UTIL_ParseCustomAmmoConfig(src.c_str(), params)) {
				FILE* dat = fopen(dst.c_str(), "wb");

				if (dat) {
					fwrite(&params, sizeof(CustomAmmoParams), 1, dat);
					fclose(dat);
					//ALERT(at_console, "Wrote %s\n", dst.c_str());
				}
				else {
					ALERT(at_error, "Failed to write ammo dat file: %s\n", dst.c_str());
				}
			}
			else {
				ALERT(at_error, "Failed to convert ammo config: %s\n", src.c_str());
			}
		}
	}

	string soundListPath = migratePath + "/_sounds.txt";
	FILE* soundList = fopen(soundListPath.c_str(), "w");
	if (soundList) {
		for (int i = 0; i < MAX_PRECACHE; i++) {
			if (g_indexSounds[i]) {
				fprintf(soundList, "%d=%s\n", i, STRING(g_indexSounds[i]));
			}
		}
		fclose(soundList);
	}
	else {
		ALERT(at_error, "Failed to open sound list file: %s\n", soundListPath.c_str());
	}

	string modelListPath = migratePath + "/_models.txt";
	FILE* modelList = fopen(modelListPath.c_str(), "w");
	if (modelList) {
		for (int i = 0; i < MAX_PRECACHE; i++) {
			if (g_indexModels[i]) {
				fprintf(modelList, "%d=%s\n", i, STRING(g_indexModels[i]));
			}
		}
		fclose(modelList);
	}
	else {
		ALERT(at_error, "Failed to open model list file: %s\n", modelListPath.c_str());
	}

	string stringsListPath = migratePath + "/_strings.txt";
	FILE* stringList = fopen(stringsListPath.c_str(), "w");
	if (stringList) {
		HashMap<string_t>::iterator_t iter;
		while (g_allocedStrings.iterate(iter)) {
			fprintf(stringList, "%u=%s\n", *iter.value, iter.key);
		}
		fclose(stringList);
	}
	else {
		ALERT(at_error, "Failed to open model list file: %s\n", stringsListPath.c_str());
	}

	g_migrationDumpMode = false;
#endif
}

// make any adjustments to the data before dumping the config
void MigratePreDump(CustomWeaponParams& params) {

	if (false) {
		for (int i = 0; i < params.numEvents; i++) {
			WepEvt& evt = params.events[i];
		}
	}
}

void MigrateWeaponsEnd() {
	std::vector<std::string> files, folders;
	UTIL_FindFilesRecursive("valve/weapons/_migrate/raw", files, folders);

	string cfgPath = "valve/weapons/_migrate/cfg";

	if (UTIL_FolderExists(cfgPath)) {
		if (!UTIL_DeleteFolderRecursive(cfgPath)) {
			ALERT(at_error, "Failed to delete migration folder: %s\n", cfgPath.c_str());
			return;
		}
	}

	if (!UTIL_CreateFolder(cfgPath)) {
		ALERT(at_error, "Failed to create migration folder: %s\n", cfgPath.c_str());
		return;
	}

	g_migrateSoundMap = readIndexMap(migratePath + "/_sounds.txt", g_soundReplacementsMod);
	g_migrateModelMap = readIndexMap(migratePath + "/_models.txt", g_modelReplacementsMod);
	g_migrateStringMap = readIndexMap(migratePath + "/_strings.txt", StringMap());
	g_migrateCvarMap = readIndexMap(migratePath + "/_cvars.txt", StringMap());
	g_migrateStringMap[0] = "";
	g_migrationDumpMode = true;

	for (std::string file : files) {
		string src = replaceString(file, "valve/weapons/_migrate/raw/", "");

		int lastSlash = src.find_last_of("/");
		string fname = lastSlash != -1 ? src.substr(lastSlash + 1) : src;

		bool isWeaponConfig = fname.find("weapon_") == 0;
		bool isAmmoConfig = fname.find("ammo_") == 0;

		if (!(isWeaponConfig || isAmmoConfig)) {
			continue;
		}
		if (fname.find(".dat") != fname.size() - 4) {
			continue;
		}

		string dst = "_migrate/cfg/" + replaceString(src, ".dat", ".txt");

		string folderPath = lastSlash != -1 ? src.substr(0, lastSlash) : "";
		folderPath = cfgPath + "/" + folderPath;

		if (folderPath.size() && !UTIL_FolderExists(folderPath)) {
			if (!UTIL_CreateFolder(folderPath)) {
				ALERT(at_error, "Failed to create migration folder: %s\n", folderPath.c_str());
				continue;
			}
		}

		if (isWeaponConfig) {
			int sz;
			uint8_t* dat = UTIL_LoadFileRoot(file.c_str(), &sz);
			if (dat) {
				if (sizeof(CustomWeaponParams) == sz) {
					MigratePreDump(*(CustomWeaponParams*)dat);
					UTIL_DumpCustomWeaponConfig(dst.c_str(), *(CustomWeaponParams*)dat, true);
				}
				else {
					ALERT(at_error, "Invalid dat size (%d != %d) for file: %s\n", sz, sizeof(CustomWeaponParams), file.c_str());
				}
			}
			else {
				ALERT(at_error, "Failed to load dat file: %s\n", file.c_str());
			}

			delete[] dat;
		}
		else {
			int sz;
			uint8_t* dat = UTIL_LoadFileRoot(file.c_str(), &sz);
			if (dat) {
				if (sizeof(CustomAmmoParams) == sz) {
					UTIL_DumpCustomAmmoConfig(dst.c_str(), *(CustomAmmoParams*)dat, true);
				}
				else {
					ALERT(at_error, "Invalid dat size (%d != %d) for file: %s\n", sz, sizeof(CustomAmmoParams), file.c_str());
				}
			}
			else {
				ALERT(at_error, "Failed to load dat file: %s\n", file.c_str());
			}

			delete[] dat;
		}
	}

	g_migrationDumpMode = false;
}

void UTIL_MigrateWeaponConfigs(int migrateMode) {
	if (migrateMode == 1) {
		MigrateWeaponsBegin();
	}
	else if (migrateMode == 2) {
		MigrateWeaponsEnd();
	}
	else if (migrateMode == 3) {
		MigrateWeaponsBegin();
		MigrateWeaponsEnd();
	}
}