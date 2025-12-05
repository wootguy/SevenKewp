#pragma once
// shared functions used by logic entities such as trigger_changevalue and trigger_condition
#include "CPointEntity.h"

// generic flags that should be converted to from the child entity's own flags
#define SF_LOGIC_DONT_USE_X 1
#define SF_LOGIC_DONT_USE_Y 2
#define SF_LOGIC_DONT_USE_Z 4

// trigger_copyvalue values
enum float_to_str_conv {
	FLT2STR_6DP = 0, // 6 decimal places when converted to string
	FLT2STR_5DP = 1,
	FLT2STR_4DP = 4,
	FLT2STR_3DP = 7,
	FLT2STR_2DP = 10,
	FLT2STR_1DP = 13,
	FLT2STR_ROUND = 16, // round to nearest whole number when converted to string/int
	FLT2STR_CEIL = 17,
	FLT2STR_FLOOR = 18,
};

class EXPORT CBaseLogic : public CPointEntity
{
public:

	// combination of SF_LOGIC_DONT_USE_* flags for vectors. Child flags are converted here
	virtual int GetVectorDontUseFlags() { return 0; }

	// finds the first entity with the given targetname, or worldspawn if one doesn't exist
	// supports !activator and !caller
	CBaseEntity* FindLogicEntity(string_t targetname);

	// finds all entities with the given targetname, or just the !activator/!caller
	std::vector<CBaseEntity*> FindLogicEntities(const char* targetName);

	// same as FireTargets but also handles !activator and !caller
	void FireLogicTargets(const char* targetName, USE_TYPE useType, float value);

	// returns a single coordinate if 2x don't-use flags are set, else the length of the vector
	float VectorToFloat(Vector v);

	int FloatToInteger(float v, int flt2str_mode);

	std::string FloatToString(float f, int flt2str_mode);

	std::string VectorToString(Vector v, int flt2str_mode);
};