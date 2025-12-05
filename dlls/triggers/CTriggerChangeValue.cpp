#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "CRuleEntity.h"
#include "CBaseLogic.h"
#include "hlds_hooks.h"

//
// CTriggerChangeValue / trigger_changevalue -- changes an entity keyvalue

// TODO: key values like MOVETYPE_SOLID and bitwise negation (~) work according to the docs

#define SF_TCVAL_DONT_USE_X 1
#define SF_TCVAL_DONT_USE_Y 2
#define SF_TCVAL_DONT_USE_Z 4
#define SF_TCVAL_CONSTANT 8
#define SF_TCVAL_START_ON 16
#define SF_TCVAL_INVERT_TARGET_VALUE 32
#define SF_TCVAL_INVERT_SOURCE_VALUE 64 // TODO: this is pointless. Ripent this out of every map that uses it.
#define SF_TCVAL_MULTIPLE_DESTINATIONS 128 // TODO: When you would ever want to target only the first entity found? As a mapper, you have no idea what that will be. It just seems like a bug.

// from rehlds
#define MAX_KEY_NAME_LEN 256
#define MAX_KEY_VALUE_LEN 2048

enum changevalue_operations {
	CVAL_OP_REPLACE = 0,
	CVAL_OP_ADD = 1,
	CVAL_OP_MUL = 2,
	CVAL_OP_SUB = 3,
	CVAL_OP_DIV = 4,
	CVAL_OP_AND = 5,
	CVAL_OP_OR = 6,
	CVAL_OP_NAND = 7,
	CVAL_OP_NOR = 8,
	CVAL_OP_DIR2ANG = 9,
	CVAL_OP_ANG2DIR = 10,
	CVAL_OP_APPEND = 11, 
	CVAL_OP_MOD = 12,
	CVAL_OP_XOR = 13,
	CVAL_OP_NXOR = 14,
	CVAL_OP_POW = 16,
	CVAL_OP_SIN = 17,
	CVAL_OP_COS = 18,
	CVAL_OP_TAN = 19,
	CVAL_OP_ARCSIN = 20,
	CVAL_OP_ARCCOS = 21,
	CVAL_OP_ARCTAN = 22,
	CVAL_OP_COT = 23,
	CVAL_OP_ARCCOT = 24,
};

enum changevalue_trig_behavior {
	CVAL_TRIG_DEGREES,
	CVAL_TRIG_RADIANS
};

enum copyvalue_float_conv {
	CVAL_STR_6DP = 0, // 6 decimal places when converted to string
	CVAL_STR_5DP = 1,
	CVAL_STR_4DP = 4,
	CVAL_STR_3DP = 7,
	CVAL_STR_2DP = 10,
	CVAL_STR_1DP = 13,
	CVAL_STR_ROUND = 16, // round to nearest whole number when converted to string/int
	CVAL_STR_CEIL = 17,
	CVAL_STR_FLOOR = 18,
};

class CTriggerChangeValue : public CBaseLogic
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT TimedThink();

	int OperateInteger(int sourceVal, int targetVal);
	float OperateFloat(float sourceVal, float targetVal);
	Vector OperateVector(CKeyValue& keyvalue);
	const char* OperateString(const char* sourceVal, const char* targetVal);
	const char* Operate(CKeyValue& keyvalue);
	void HandleTarget(CBaseEntity* ent);
	void LoadSourceValues();
	bool LoadSpecialStaticValue(const char* val, int& specialVal);
	void ChangeValues();

	int GetVectorDontUseFlags() { return pev->spawnflags & (SF_TCVAL_DONT_USE_X | SF_TCVAL_DONT_USE_Y | SF_TCVAL_DONT_USE_Z); }

	string_t m_iszKeyName;
	string_t m_iszNewValue;
	int m_iAppendSpaces;
	int m_iOperation;

	string_t m_iszDstKeyName;
	string_t m_iszSrcKeyName;
	int m_iFloatConversion;

	// source values loaded from source entity or static keyvalue
	Vector m_vSrc;
	float m_fSrc;
	int m_iSrc;
	const char* m_sSrc;

	float m_trigIn; // multiplier for trigonemtric function input values (degrees -> degrees/radians)
	float m_trigOut; // multiplier for trigonemtric function output values (radians -> degrees/radians)

	bool isCopyValue; // this entitiy is a trigger_copyvalue not a trigger_changevalue
	bool isActive;
};

LINK_ENTITY_TO_CLASS(trigger_changevalue, CTriggerChangeValue)
LINK_ENTITY_TO_CLASS(trigger_copyvalue, CTriggerChangeValue)

void CTriggerChangeValue::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszValueName"))
	{
		m_iszKeyName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszDstValueName"))
	{
		m_iszDstKeyName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszNewValue"))
	{
		m_iszNewValue = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iAppendSpaces"))
	{
		m_iAppendSpaces = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszValueType"))
	{
		m_iOperation = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_trigonometricBehaviour"))
	{
		int behavior = atoi(pkvd->szValue);

		if (behavior == CVAL_TRIG_DEGREES) {
			// trig functions work with radians, so input/output values need conversion
			m_trigIn = M_PI / 180.0f;
			m_trigOut = 180.0f / M_PI;
		}
		else if (behavior == CVAL_TRIG_RADIANS) {
			m_trigIn = 1.0f;
			m_trigOut = 1.0f;
		}
		else {
			ALERT(at_warning, "trigger_changevalue: invalid trig behavior %d\n", behavior);
		}
		
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszSrcValueName"))
	{
		m_iszSrcKeyName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iFloatConversion"))
	{
		m_iFloatConversion = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CPointEntity::KeyValue(pkvd);
	}
}

void CTriggerChangeValue::Spawn(void)
{
	CPointEntity::Spawn();

	isCopyValue = !strcmp(STRING(pev->classname), "trigger_copyvalue");

	if (!isCopyValue) {
		// disable copyvalue-specific settings
		pev->spawnflags &= ~(SF_TCVAL_CONSTANT | SF_TCVAL_MULTIPLE_DESTINATIONS | SF_TCVAL_START_ON);
		pev->netname = 0;
		pev->dmg = 0;
		m_iszSrcKeyName = 0;
		m_iFloatConversion = 0;
	}
	else {
		// disable changevalue-specific settings
		m_iszKeyName = m_iszDstKeyName;
		m_iszNewValue = 0;
	}

	if ((pev->spawnflags & SF_TCVAL_START_ON) && (pev->spawnflags & SF_TCVAL_CONSTANT)) {
		isActive = true;
		SetThink(&CTriggerChangeValue::TimedThink);
		pev->nextthink = gpGlobals->time + pev->dmg;
	}

	SetUse(&CTriggerChangeValue::Use);
}

int CTriggerChangeValue::OperateInteger(int sourceVal, int targetVal) {
	if (pev->spawnflags & SF_TCVAL_INVERT_TARGET_VALUE) {
		targetVal = -targetVal;
	}
	if (pev->spawnflags & SF_TCVAL_INVERT_SOURCE_VALUE) {
		sourceVal = -sourceVal;
	}

	switch (m_iOperation) {
	case CVAL_OP_REPLACE: return sourceVal;
	case CVAL_OP_ADD: return targetVal + sourceVal;
	case CVAL_OP_MUL: return targetVal * sourceVal;
	case CVAL_OP_SUB: return targetVal - sourceVal;
	case CVAL_OP_DIV: return targetVal / sourceVal;
	case CVAL_OP_POW: return powf(targetVal, sourceVal);
	case CVAL_OP_MOD: return targetVal % sourceVal;
	case CVAL_OP_AND: return targetVal & sourceVal;
	case CVAL_OP_OR: return targetVal | sourceVal;
	case CVAL_OP_XOR: return targetVal ^ sourceVal;
	case CVAL_OP_NAND: return ~(targetVal & sourceVal);
	case CVAL_OP_NOR: return ~(targetVal | sourceVal);
	case CVAL_OP_NXOR: return ~(targetVal ^ sourceVal);
	case CVAL_OP_SIN: return sinf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_COS: return cosf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_TAN: return tanf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_COT: return (1.0f / tanf(sourceVal * m_trigIn)) * m_trigOut;
	case CVAL_OP_ARCSIN: return asinf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_ARCCOS: return acosf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_ARCTAN: return atanf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_ARCCOT: return atanf(1.0f / (sourceVal * m_trigIn)) * m_trigOut;
	default:
		EALERT(at_warning, "invalid operation %d on integer value\n", m_iOperation);
		return targetVal;
	}
}

float CTriggerChangeValue::OperateFloat(float sourceVal, float targetVal) {
	if (pev->spawnflags & SF_TCVAL_INVERT_TARGET_VALUE) {
		targetVal = -targetVal;
	}
	if (pev->spawnflags & SF_TCVAL_INVERT_SOURCE_VALUE) {
		sourceVal = -sourceVal;
	}

	switch (m_iOperation) {
	case CVAL_OP_REPLACE: return sourceVal;
	case CVAL_OP_ADD: return targetVal + sourceVal;
	case CVAL_OP_MUL: return targetVal * sourceVal;
	case CVAL_OP_SUB: return targetVal - sourceVal;
	case CVAL_OP_DIV: return targetVal / sourceVal;
	case CVAL_OP_POW: return powf(targetVal, sourceVal);
	case CVAL_OP_MOD: return (int)targetVal % (int)sourceVal;
	case CVAL_OP_AND: return (int)targetVal & (int)sourceVal;
	case CVAL_OP_OR: return (int)targetVal | (int)sourceVal;
	case CVAL_OP_XOR: return (int)targetVal ^ (int)sourceVal;
	case CVAL_OP_NAND: return ~((int)targetVal & (int)sourceVal);
	case CVAL_OP_NOR: return ~((int)targetVal | (int)sourceVal);
	case CVAL_OP_NXOR: return ~((int)targetVal ^ (int)sourceVal);
	case CVAL_OP_SIN: return sinf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_COS: return cosf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_TAN: return tanf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_COT: return (1.0f / tanf(sourceVal * m_trigIn)) * m_trigOut;
	case CVAL_OP_ARCSIN: return asinf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_ARCCOS: return acosf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_ARCTAN: return atanf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_ARCCOT: return atanf(1.0f / (sourceVal * m_trigIn)) * m_trigOut;
	default:
		EALERT(at_warning, "invalid operation %d on float value\n", m_iOperation);
		return targetVal;
	}
}

Vector CTriggerChangeValue::OperateVector(CKeyValue& keyvalue) {
	Vector vTemp = keyvalue.vVal;

	switch (m_iOperation) {
	case CVAL_OP_DIR2ANG:
		return UTIL_VecToAngles(vTemp.Normalize());
	case CVAL_OP_ANG2DIR:
		UTIL_MakeVectors(vTemp);
		return gpGlobals->v_forward;
	default:
		if (!(pev->spawnflags & SF_TCVAL_DONT_USE_X)) {
			vTemp[0] = OperateFloat(m_vSrc[0], keyvalue.vVal[0]);
		}
		if (!(pev->spawnflags & SF_TCVAL_DONT_USE_Y)) {
			vTemp[1] = OperateFloat(m_vSrc[1], keyvalue.vVal[1]);
		}
		if (!(pev->spawnflags & SF_TCVAL_DONT_USE_Z)) {
			vTemp[2] = OperateFloat(m_vSrc[2], keyvalue.vVal[2]);
		}
		return vTemp;
	}
}

const char* CTriggerChangeValue::Operate(CKeyValue& keyvalue) {
	switch (keyvalue.keyType) {
	case KEY_TYPE_FLOAT:
		return UTIL_VarArgs("%f", OperateFloat(m_fSrc, keyvalue.fVal));
	case KEY_TYPE_INT:
		return UTIL_VarArgs("%d", OperateInteger(m_iSrc, keyvalue.iVal));
	case KEY_TYPE_VECTOR: {
		Vector vTemp = OperateVector(keyvalue);
		return UTIL_VarArgs("%f %f %f", vTemp.x, vTemp.y, vTemp.z);
	}
	case KEY_TYPE_STRING:
		return OperateString(m_sSrc, keyvalue.sVal ? STRING(keyvalue.sVal) : "");
	default:
		EALERT(at_warning, "operation on keyvalue %s not allowed\n", keyvalue.keyName);
		return "";
	}
}

const char* CTriggerChangeValue::OperateString(const char* sourceVal, const char* targetVal) {
	static char spaces[MAX_KEY_VALUE_LEN];

	if (m_iAppendSpaces > 0) {
		int appendCnt = V_min(MAX_KEY_VALUE_LEN - 1, m_iAppendSpaces);
		memset(spaces, ' ', appendCnt);
		spaces[appendCnt] = 0;
	}
	else {
		spaces[0] = 0;
	}

	switch (m_iOperation) {
	case CVAL_OP_REPLACE:
		return UTIL_VarArgs("%s%s", sourceVal, spaces);
	case CVAL_OP_APPEND:
		return UTIL_VarArgs("%s%s%s", targetVal, sourceVal, spaces);
	default:
		EALERT(at_warning, "invalid operation %d on string value\n", m_iOperation);
		return targetVal;
	}
}

void CTriggerChangeValue::HandleTarget(CBaseEntity* pent) {
	if (!pent)
		return;

	KeyValueData dat;
	dat.fHandled = false;
	dat.szKeyName = (char*)STRING(m_iszKeyName);
	dat.szValue = (char*)m_sSrc;
	dat.szClassName = (char*)STRING(pent->pev->classname);

	CKeyValue keyvalue = pent->GetKeyValue(dat.szKeyName);

	if (keyvalue.keyType) {
		// TODO: operate on entvars data directly, don't make a new keyvalue
		dat.szValue = Operate(keyvalue);
	}
	else if (m_iOperation != CVAL_OP_REPLACE) {
		EALERT(at_warning, "keyvalue '%s' in entity '%s' can only be replaced\n",
			 dat.szKeyName, STRING(pent->pev->classname));
	}
	
	/*
	ALERT(at_console, "'%s' (%s): send value '%s' to key '%s' in entity '%s'\n",
		pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname),
		dat.szValue, dat.szKeyName, STRING(pent->pev->targetname));
	*/

	DispatchKeyValue(pent->edict(), &dat); // using this for private fields
	UTIL_SetOrigin(pent->pev, pent->pev->origin); // in case any changes to solid/origin were made
}

void CTriggerChangeValue::LoadSourceValues() {
	// defined here to prevent destruction before keyvalues are sent,
	// and this exists at all because UTIL_VarArgs uses a single static buffer
	static std::string temp;

	// set up source values
	m_iSrc = m_fSrc = 0;
	m_sSrc = "";
	m_vSrc = g_vecZero;

	if (isCopyValue) {
		CBaseEntity* pent = FindLogicEntity(pev->netname);
		if (pent) {
			CKeyValue srcKey = pent->GetKeyValue(STRING(m_iszSrcKeyName));
			if (srcKey.keyType) {
				switch (srcKey.keyType) {
				case KEY_TYPE_FLOAT:
					m_fSrc = srcKey.fVal;
					m_iSrc = FloatToInteger(m_fSrc, m_iFloatConversion);
					m_vSrc = Vector(srcKey.fVal, srcKey.fVal, srcKey.fVal);
					temp = FloatToString(srcKey.fVal, m_iFloatConversion);
					m_sSrc = temp.c_str();
					break;
				case KEY_TYPE_INT:
					m_fSrc = srcKey.iVal;
					m_iSrc = FloatToInteger(m_fSrc, m_iFloatConversion);
					m_vSrc = Vector(srcKey.iVal, srcKey.iVal, srcKey.iVal);
					temp = UTIL_VarArgs("%d", srcKey.iVal);
					m_sSrc = temp.c_str();
					break;
				case KEY_TYPE_VECTOR: {
					m_fSrc = VectorToFloat(srcKey.vVal);
					m_iSrc = FloatToInteger(m_fSrc, m_iFloatConversion);
					m_vSrc = srcKey.vVal;
					temp = VectorToString(srcKey.vVal, m_iFloatConversion);
					m_sSrc = temp.c_str();
					break;
				}
				case KEY_TYPE_STRING:
					m_fSrc = m_iSrc = 0;
					m_vSrc = g_vecZero;
					m_sSrc = srcKey.sVal ? STRING(srcKey.sVal) : "";
					break;
				default:
					ALERT(at_warning, "'%s' (%s): operation on keyvalue %s not allowed\n",
						pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), srcKey.keyName);
					return;
				}
			}
		}
	}
	else if (m_iszNewValue) {
		// trigger_changevalue static value	
		const char* cNewVal = STRING(m_iszNewValue);
		UTIL_StringToVector(m_vSrc, cNewVal);

		if (m_vSrc.y != 0 || m_vSrc.z != 0) {
			// length of vector used when assigning a vector to a float/int
			m_iSrc = m_fSrc = VectorToFloat(m_vSrc);
		}
		else {
			// single value used
			if (cNewVal[0] == '~') {
				cNewVal = cNewVal + 1;
				
				int specialVal = 0;
				if (LoadSpecialStaticValue(cNewVal, specialVal)) {
					m_fSrc = ~specialVal;
					m_iSrc = ~specialVal;
				}
				else {
					m_fSrc = ~((int)atof(cNewVal));
					m_iSrc = ~atoi(cNewVal);
				}
			}
			else {
				int specialVal = 0;
				if (LoadSpecialStaticValue(cNewVal, specialVal)) {
					m_fSrc = specialVal;
					m_iSrc = specialVal;
				}
				else {
					m_fSrc = atof(cNewVal);
					m_iSrc = atoi(cNewVal);
				}
			}
				
			m_vSrc = Vector(m_fSrc, m_fSrc, m_fSrc);
		}

		m_sSrc = STRING(m_iszNewValue);
	}
}

bool CTriggerChangeValue::LoadSpecialStaticValue(const char* val, int& specialVal) {
	static HashMap<int> special_vals = {
		{"FL_FLY", FL_FLY},
		{"FL_SWIM", FL_SWIM},
		{"FL_CONVEYOR", FL_CONVEYOR},
		{"FL_CLIENT", FL_CLIENT},
		{"FL_INWATER", FL_INWATER},
		{"FL_MONSTER", FL_MONSTER},
		{"FL_GODMODE", FL_GODMODE},
		{"FL_NOTARGET", FL_NOTARGET},
		{"FL_SKIPLOCALHOST", FL_SKIPLOCALHOST},
		{"FL_ONGROUND", FL_ONGROUND},
		{"FL_PARTIALGROUND", FL_PARTIALGROUND},
		{"FL_WATERJUMP", FL_WATERJUMP},
		{"FL_FROZEN", FL_FROZEN},
		{"FL_FAKECLIENT", FL_FAKECLIENT},
		{"FL_DUCKING", FL_DUCKING},
		{"FL_FLOAT", FL_FLOAT},

		{"IN_ATTACK", IN_ATTACK},
		{"IN_JUMP", IN_JUMP},
		{"IN_DUCK", IN_DUCK},
		{"IN_FORWARD", IN_FORWARD},
		{"IN_BACK", IN_BACK},
		{"IN_USE", IN_USE},
		{"IN_CANCEL", IN_CANCEL},
		{"IN_LEFT", IN_LEFT},
		{"IN_RIGHT", IN_RIGHT},
		{"IN_MOVELEFT", IN_MOVELEFT},
		{"IN_MOVERIGHT", IN_MOVERIGHT},
		{"IN_ATTACK2", IN_ATTACK2},
		{"IN_RUN", IN_RUN},
		{"IN_RELOAD", IN_RELOAD},
		{"IN_ALT1", IN_ALT1},
		{"IN_SCORE", IN_SCORE},

		{"DEAD_NO", DEAD_NO},
		{"DEAD_DYING", DEAD_DYING},
		{"DEAD_DEAD", DEAD_DEAD},
		{"DEAD_RESPAWNABLE", DEAD_RESPAWNABLE},
		{"DEAD_DISCARDBODY", DEAD_DISCARDBODY},

		{"DAMAGE_NO", DAMAGE_NO},
		{"DAMAGE_YES", DAMAGE_YES},
		{"DAMAGE_AIM", DAMAGE_AIM},

		{"SOLID_NOT", SOLID_NOT},
		{"SOLID_TRIGGER", SOLID_TRIGGER},
		{"SOLID_BBOX", SOLID_BBOX},
		{"SOLID_SLIDEBOX", SOLID_SLIDEBOX},
		{"SOLID_BSP", SOLID_BSP},

		{"WATERLEVEL_DRY", WATERLEVEL_DRY},
		{"WATERLEVEL_FEET", WATERLEVEL_FEET},
		{"WATERLEVEL_WAIST", WATERLEVEL_WAIST},
		{"WATERLEVEL_HEAD", WATERLEVEL_HEAD},

		{"MOVETYPE_NONE", MOVETYPE_NONE},
		{"MOVETYPE_WALK", MOVETYPE_WALK},
		{"MOVETYPE_STEP", MOVETYPE_STEP},
		{"MOVETYPE_FLY", MOVETYPE_FLY},
		{"MOVETYPE_TOSS", MOVETYPE_TOSS},
		{"MOVETYPE_PUSH", MOVETYPE_PUSH},
		{"MOVETYPE_NOCLIP", MOVETYPE_NOCLIP},
		{"MOVETYPE_FLYMISSILE", MOVETYPE_FLYMISSILE},
		{"MOVETYPE_BOUNCE", MOVETYPE_BOUNCE},
		{"MOVETYPE_BOUNCEMISSILE", MOVETYPE_BOUNCEMISSILE},
		{"MOVETYPE_FOLLOW", MOVETYPE_FOLLOW},
		{"MOVETYPE_PUSHSTEP", MOVETYPE_PUSHSTEP},

		{"EF_BRIGHTFIELD", EF_BRIGHTFIELD},
		{"EF_MUZZLEFLASH", EF_MUZZLEFLASH},
		{"EF_BRIGHTLIGHT", EF_BRIGHTLIGHT},
		{"EF_DIMLIGHT", EF_DIMLIGHT},
		{"EF_INVLIGHT", EF_INVLIGHT},
		{"EF_NOINTERP", EF_NOINTERP},
		{"EF_LIGHT", EF_LIGHT},
		{"EF_NODRAW", EF_NODRAW},
		//{"EF_NOANIMTEXTURES", EF_NOANIMTEXTURES},
		//{"EF_FRAMEANIMTEXTURES", EF_FRAMEANIMTEXTURES},
		//{"EF_SPRITE_CUSTOM_VP", EF_SPRITE_CUSTOM_VP},
		//{"EF_NODECALS", EF_NODECALS},
		
		{"KRENDERFXNONE", kRenderFxNone},
		{"KRENDERFXPULSESLOW", kRenderFxPulseSlow},
		{"KRENDERFXPULSEFAST", kRenderFxPulseFast},
		{"KRENDERFXPULSESLOWWIDE", kRenderFxPulseSlowWide},
		{"KRENDERFXPULSEFASTWIDE", kRenderFxPulseFastWide},
		{"KRENDERFXFADESLOW", kRenderFxFadeSlow},
		{"KRENDERFXFADEFAST", kRenderFxFadeFast},
		{"KRENDERFXSOLIDSLOW", kRenderFxSolidSlow},
		{"KRENDERFXSOLIDFAST", kRenderFxSolidFast},
		{"KRENDERFXSTROBESLOW", kRenderFxStrobeSlow},
		{"KRENDERFXSTROBEFAST", kRenderFxStrobeFast},
		{"KRENDERFXSTROBEFASTER", kRenderFxStrobeFaster},
		{"KRENDERFXFLICKERFAST", kRenderFxFlickerFast},
		{"KRENDERFXNODISSIPATION", kRenderFxNoDissipation},
		{"KRENDERFXDISTORT", kRenderFxDistort},
		{"KRENDERFXHOLOGRAM", kRenderFxHologram},
		{"KRENDERFXDEADPLAYER", kRenderFxDeadPlayer},
		{"KRENDERFXEXPLODE", kRenderFxExplode},
		{"KRENDERFXGLOWSHELL", kRenderFxGlowShell},
		{"KRENDERFXCLAMPMINSCALE", kRenderFxClampMinScale },

		{"KRENDERNORMAL", kRenderNormal },
		{"KRENDERTRANSCOLOR", kRenderTransColor },
		{"KRENDERTRANSTEXTURE", kRenderTransTexture },
		{"KRENDERGLOW", kRenderGlow },
		{"KRENDERTRANSALPHA", kRenderTransAlpha },
		{"KRENDERTRANSADD", kRenderTransAdd },

		//{"CLASS_FORCE_NONE", CLASS_FORCE_NONE },
		{"CLASS_NONE", CLASS_NONE },
		{"CLASS_MACHINE", CLASS_MACHINE },
		{"CLASS_PLAYER", CLASS_PLAYER },
		{"CLASS_HUMAN_PASSIVE", CLASS_HUMAN_PASSIVE },
		{"CLASS_HUMAN_MILITARY", CLASS_HUMAN_MILITARY },
		{"CLASS_ALIEN_MILITARY", CLASS_ALIEN_MILITARY },
		{"CLASS_ALIEN_PASSIVE", CLASS_ALIEN_PASSIVE },
		{"CLASS_ALIEN_MONSTER", CLASS_ALIEN_MONSTER },
		{"CLASS_ALIEN_PREY", CLASS_ALIEN_PREY },
		{"CLASS_ALIEN_PREDATOR", CLASS_ALIEN_PREDATOR },
		{"CLASS_INSECT", CLASS_INSECT },
		{"CLASS_PLAYER_ALLY", CLASS_PLAYER_ALLY },
		{"CLASS_PLAYER_BIOWEAPON", CLASS_PLAYER_BIOWEAPON },
		{"CLASS_ALIEN_BIOWEAPON", CLASS_ALIEN_BIOWEAPON },
		{"CLASS_XRACE_PITDRONE", CLASS_ALIEN_RACE_X_PITDRONE },
		{"CLASS_ALIEN_RACE_X_PITDRONE", CLASS_ALIEN_RACE_X_PITDRONE },
		{"CLASS_XRACE_SHOCK", CLASS_ALIEN_RACE_X },
		{"CLASS_ALIEN_RACE_X", CLASS_ALIEN_RACE_X },
		//{"CLASS_TEAM1", CLASS_TEAM1 },
		//{"CLASS_TEAM2", CLASS_TEAM2 },
		//{"CLASS_TEAM3", CLASS_TEAM3 },
		//{"CLASS_TEAM4", CLASS_TEAM4 },
		{"CLASS_BARNACLE", CLASS_BARNACLE },
	};

	std::string upperVal = toUpperCase(val);

	int* special = special_vals.get(upperVal.c_str());
	if (special) {
		specialVal = *special;
		return true;
	}

	return false;
}

void CTriggerChangeValue::ChangeValues() {
	if (isCopyValue) {
		if (!pev->netname) {
			ALERT(at_warning, "'%s' (%s): missing source entity\n",
				pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname));
			return;
		}
		if (!m_iszSrcKeyName) {
			ALERT(at_warning, "'%s' (%s): missing source key\n",
				pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname));
			return;
		}
	}
	else {
		if (!m_iszKeyName) {
			ALERT(at_warning, "'%s' (%s): missing key name\n",
				pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname));
			return;
		}
	}	

	LoadSourceValues();

	const char* target = STRING(pev->target);

	if (!strcmp(target, "!activator")) {
		HandleTarget(m_hActivator);
	}
	else if (!strcmp(target, "!caller")) {
		HandleTarget(m_hCaller);
	}
	else {
		CBaseEntity* ent = NULL;

		while ((ent = UTIL_FindEntityByTargetname(ent, target))) {
			HandleTarget(ent);

			if (isCopyValue && !(pev->spawnflags & SF_TCVAL_MULTIPLE_DESTINATIONS)) {
				break;
			}
		}
	}

	if (pev->message) {
		FireLogicTargets(STRING(pev->message), USE_TOGGLE, 0.0f);
	}
}

void CTriggerChangeValue::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	ChangeValues();

	if (pev->spawnflags & SF_TCVAL_CONSTANT) {
		isActive = useType == USE_TOGGLE ? !isActive : (bool)useType;

		if (isActive) {
			SetThink(&CTriggerChangeValue::TimedThink);
			pev->nextthink = gpGlobals->time + pev->dmg;
		}
		else {
			SetThink(NULL);
			pev->nextthink = 0;
		}
	}
}

void CTriggerChangeValue::TimedThink() {
	ChangeValues();
	pev->nextthink = gpGlobals->time + pev->dmg;
}
