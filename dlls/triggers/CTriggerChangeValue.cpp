#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CRuleEntity.h"

//
// CTriggerChangeValue / trigger_changevalue -- changes an entity keyvalue

#define SF_DONT_USE_X 1
#define SF_DONT_USE_Y 2
#define SF_DONT_USE_Z 4
#define SF_INVERT_TARGET_VALUE 32
#define SF_INVERT_SOURCE_VALUE 64 // TODO: this is pointless. Ripent this out of every map that uses it.

// from rehlds
#define MAX_KEY_NAME_LEN 256
#define MAX_KEY_VALUE_LEN 2048

enum changevalue_operations {
	// these values are weird because svencoop.fgd is weird
	CVAL_OP_REPLACE = 0,
	CVAL_OP_ADD = 1,
	CVAL_OP_MUL = 2,
	CVAL_OP_SUB = 3,
	CVAL_OP_DIV = 4,
	CVAL_OP_AND = 5,
	CVAL_OP_OR = 6,
	CVAL_OP_NAND = 7,
	CVAL_OP_NOR = 8,
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

class CTriggerChangeValue : public CPointEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	int OperateInteger(int sourceVal, int targetVal);
	float OperateFloat(float sourceVal, float targetVal);
	Vector OperateVector(CKeyValue& keyvalue);
	const char* OperateString(const char* sourceVal, const char* targetVal);
	const char* Operate(CKeyValue& keyvalue);
	void HandleTarget(CBaseEntity* ent);

	string_t m_iszKeyName;
	string_t m_iszNewValue;
	int m_iAppendSpaces;
	int m_iOperation;

	// temporaries for math
	Vector m_vVal;
	float m_fVal;
	int m_iVal;

	float m_trigIn; // multiplier for trigonemtric function input values (degrees -> degrees/radians)
	float m_trigOut; // multiplier for trigonemtric function output values (radians -> degrees/radians)
};

LINK_ENTITY_TO_CLASS(trigger_changevalue, CTriggerChangeValue);

void CTriggerChangeValue::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszValueName"))
	{
		m_iszKeyName = ALLOC_STRING(pkvd->szValue);
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
	else if (FStrEq(pkvd->szKeyName, "m_iOperation"))
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
	else {
		CPointEntity::KeyValue(pkvd);
	}
}

void CTriggerChangeValue::Spawn(void)
{
	CPointEntity::Spawn();

	if (m_iAppendSpaces > 0 && (m_iOperation == CVAL_OP_APPEND || m_iOperation == CVAL_OP_REPLACE)) {
		static char spacesVal[MAX_KEY_VALUE_LEN];
		const int maxStrLen = MAX_KEY_VALUE_LEN - 2; // exludes the null char

		int oldLen = V_min(maxStrLen, strlen(STRING(m_iszNewValue)));
		int spacesToAppend = V_min(maxStrLen - oldLen, m_iAppendSpaces);

		strncpy(spacesVal, STRING(m_iszNewValue), maxStrLen);
		memset(spacesVal + oldLen, ' ', spacesToAppend);
		spacesVal[oldLen + spacesToAppend] = '\0';

		m_iszNewValue = ALLOC_STRING(spacesVal);
	}

	SetUse(&CTriggerChangeValue::Use);
}

int CTriggerChangeValue::OperateInteger(int sourceVal, int targetVal) {
	if (pev->spawnflags & SF_INVERT_TARGET_VALUE) {
		targetVal = -targetVal;
	}
	if (pev->spawnflags & SF_INVERT_SOURCE_VALUE) {
		sourceVal = -sourceVal;
	}

	switch (m_iOperation) {
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
		ALERT(at_warning, "trigger_changevalue: invalid operation %d on integer value\n", m_iOperation);
		return targetVal;
	}
}

float CTriggerChangeValue::OperateFloat(float sourceVal, float targetVal) {
	if (pev->spawnflags & SF_INVERT_TARGET_VALUE) {
		targetVal = -targetVal;
	}
	if (pev->spawnflags & SF_INVERT_SOURCE_VALUE) {
		sourceVal = -sourceVal;
	}

	switch (m_iOperation) {
	case CVAL_OP_ADD: return targetVal + sourceVal;
	case CVAL_OP_MUL: return targetVal * sourceVal;
	case CVAL_OP_SUB: return targetVal - sourceVal;
	case CVAL_OP_DIV: return targetVal / sourceVal;
	case CVAL_OP_POW: return powf(targetVal, sourceVal);
	case CVAL_OP_SIN: return sinf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_COS: return cosf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_TAN: return tanf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_COT: return (1.0f / tanf(sourceVal * m_trigIn)) * m_trigOut;
	case CVAL_OP_ARCSIN: return asinf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_ARCCOS: return acosf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_ARCTAN: return atanf(sourceVal * m_trigIn) * m_trigOut;
	case CVAL_OP_ARCCOT: return atanf(1.0f / (sourceVal * m_trigIn)) * m_trigOut;
	default:
		ALERT(at_warning, "trigger_changevalue: invalid operation %d on float value\n", m_iOperation);
		return targetVal;
	}
}

Vector CTriggerChangeValue::OperateVector(CKeyValue& keyvalue) {
	Vector vTemp = keyvalue.vVal;

	if (!(pev->spawnflags & SF_DONT_USE_X)) {
		vTemp[0] = OperateFloat(m_vVal[0], keyvalue.vVal[0]);
	}
	if (!(pev->spawnflags & SF_DONT_USE_Y)) {
		vTemp[1] = OperateFloat(m_vVal[1], keyvalue.vVal[1]);
	}
	if (!(pev->spawnflags & SF_DONT_USE_Z)) {
		vTemp[2] = OperateFloat(m_vVal[2], keyvalue.vVal[2]);
	}

	return vTemp;
}

const char* CTriggerChangeValue::Operate(CKeyValue& keyvalue) {
	switch (keyvalue.desc->fieldType) {
	case FIELD_TIME:
	case FIELD_FLOAT:
		return UTIL_VarArgs("%f", OperateFloat(m_fVal, keyvalue.fVal));
	case FIELD_INTEGER:
		return UTIL_VarArgs("%d", OperateFloat(m_iVal, keyvalue.iVal));
	case FIELD_POSITION_VECTOR:
	case FIELD_VECTOR: {
		Vector vTemp = OperateVector(keyvalue);
		return UTIL_VarArgs("%f %f %f", vTemp.x, vTemp.y, vTemp.z);
	}
	case FIELD_MODELNAME:
	case FIELD_SOUNDNAME:
	case FIELD_STRING:
		return OperateString(STRING(m_iszNewValue), keyvalue.sVal ? STRING(keyvalue.sVal) : "");
	default:
		ALERT(at_warning, "trigger_changevalue: operation on keyvalue %s not allowed\n", keyvalue.desc->fieldName);
		return "";
	}
}

const char* CTriggerChangeValue::OperateString(const char* sourceVal, const char* targetVal) {
	if (m_iOperation == CVAL_OP_APPEND) {
		return UTIL_VarArgs("%s%s", targetVal, sourceVal);
	}
	else {
		ALERT(at_warning, "trigger_changevalue: invalid operation %d on string value\n", m_iOperation);
		return targetVal;
	}	
}

void CTriggerChangeValue::HandleTarget(CBaseEntity* pent) {
	if (!pent)
		return;

	KeyValueData dat;
	dat.fHandled = false;
	dat.szKeyName = (char*)STRING(m_iszKeyName);
	dat.szValue = (char*)STRING(m_iszNewValue);
	dat.szClassName = (char*)STRING(pent->pev->classname);

	if (m_iOperation != CVAL_OP_REPLACE) {
		CKeyValue keyvalue = pent->GetKeyValue(dat.szKeyName);

		if (keyvalue.desc) {
			// TODO: operate on entvars keys directly, don't make a new keyvalue
			dat.szValue = Operate(keyvalue);
		}
		else {
			ALERT(at_warning, "trigger_changevalue: keyvalue '%s' in entity '%s' can only be replaced\n",
				dat.szKeyName, STRING(pent->pev->classname));
		}
	}

	DispatchKeyValue(pent->edict(), &dat);
}

void CTriggerChangeValue::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!m_iszKeyName) {
		ALERT(at_warning, "trigger_changevalue: missing key name\n");
		return;
	}
	if (!m_iszNewValue) {
		ALERT(at_warning, "trigger_changevalue: missing source value\n");
		return;
	}

	// setup temporary vars for math
	if (m_iOperation != CVAL_OP_REPLACE) {
		const char* cNewVal = STRING(m_iszNewValue);
		m_fVal = atof(cNewVal);
		m_iVal = atoi(cNewVal);
		UTIL_StringToVector(m_vVal, cNewVal);
	}

	const char* target = STRING(pev->target);

	if (!strcmp(target, "!activator")) {
		HandleTarget(pActivator);
	}
	else if (!strcmp(target, "!caller")) {
		HandleTarget(pCaller);
	}
	else {
		edict_t* ent = NULL;

		while (!FNullEnt(ent = FIND_ENTITY_BY_TARGETNAME(ent, target))) {
			HandleTarget(CBaseEntity::Instance(ent));
		}
	}

	if (pev->message) {
		FireTargets(STRING(pev->message), pActivator, this, USE_TOGGLE, 0.0f);
	}
}
