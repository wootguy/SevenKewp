#pragma once
#include "extdll.h"
#include <cstdint>
enum keyvalue_types {
	KEY_TYPE_NONE, // indicates value failed to load
	KEY_TYPE_INT,
	KEY_TYPE_FLOAT,
	KEY_TYPE_VECTOR,
	KEY_TYPE_STRING,
	KEY_TYPE_EHANDLE
};

struct CKeyValue {
	uint16_t keyType; // simplified type from keyvalue_types. Key is invalid if this is 0.
	uint16_t keyOffset;
	const char* keyName;

	union {
		int32_t iVal;
		float fVal;
		string_t sVal;
		float vVal[3];
	};
};