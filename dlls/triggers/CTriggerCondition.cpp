#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CRuleEntity.h"

//
// CTriggerCondition / trigger_condition -- compare entity keyvalues

#define SF_START_OFF 1
#define SF_DONT_USE_X 2
#define SF_DONT_USE_Y 4
#define SF_DONT_USE_Z 8
#define SF_DONT_USE_W 16 // TODO: which keys have a W/A value?
#define SF_CYCLIC 32
#define SF_KEEP_ACTIVATOR 64
#define SF_IGNORE_INITIAL_RESULT 128

TYPEDESCRIPTION g_staticCompareKeyTypeString = {
	FIELD_STRING, "m_iszCheckValue", -1, 4, 0
};
TYPEDESCRIPTION g_staticCompareKeyTypeInt = {
	FIELD_INTEGER, "m_iszCheckValue", -1, 4, 0
};

enum compare_types {
	COMPARE_EQUAL,
	COMPARE_NEQUAL,
	COMPARE_LESS,
	COMPARE_GREATER,
	COMPARE_LEQUAL,
	COMPARE_GEQUAL,
	COMPARE_BITSET,
	
	COMPARE_TYPES,
};

enum constant_mode_check_behaviors {
	MODE_WAIT_BOTH, // only fire target when result changes
	MODE_WAIT_AFTER_FALSE, // always fire true, only fire false when last result was true
	MODE_WAIT_AFTER_TRUE, // always fire false, only fire true when last result was false
	MODE_WAIT_NEVER, // always fire true/false

	CONSTANT_MODES
};

class CTriggerCondition : public CPointEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT TimedThink();

	CKeyValue LoadKey(string_t entName, string_t keyName, const char* errorDesc);
	void Evaluate();
	bool Compare(const CKeyValue& monitorKey, const CKeyValue& compareKey);

	int getValueAsInt(const CKeyValue& key);
	float getValueAsFloat(const CKeyValue& key);
	float getValueAsFloat(Vector v);
	const char* getValueAsString(const CKeyValue& key);

	bool CompareFloats(const CKeyValue& monitorKey, const CKeyValue& compareKey);
	bool CompareInts(const CKeyValue& monitorKey, const CKeyValue& compareKey);
	bool CompareVectors(const CKeyValue& monitorKey, const CKeyValue& compareKey);
	bool CompareStrings(const CKeyValue& monitorKey, const CKeyValue& compareKey);

	string_t m_monitoredKey;
	string_t m_compareEntity;
	string_t m_compareKey;
	string_t m_compareValue; // static value to use if no compare entity is set
	int m_checkType;
	int m_iCheckBehavior; // for constant mode
	float m_checkInterval;

	bool isActive;
	bool m_checkedFirstResult;
	int m_lastResult;

	EHANDLE h_activator;
	EHANDLE h_caller;
};

LINK_ENTITY_TO_CLASS(trigger_condition, CTriggerCondition);

void CTriggerCondition::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszValueName"))
	{
		m_monitoredKey = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszSourceName"))
	{
		m_compareEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszSourceKey"))
	{
		m_compareKey = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszCheckValue"))
	{
		m_compareValue = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iCheckType"))
	{
		m_checkType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iCheckBehaviour"))
	{
		m_iCheckBehavior = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fCheckInterval"))
	{
		m_checkInterval = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CPointEntity::KeyValue(pkvd);
	}
}

void CTriggerCondition::Spawn(void)
{
	CPointEntity::Spawn();

	if (!(pev->spawnflags & (SF_START_OFF | SF_CYCLIC))) {
		SetThink(&CTriggerCondition::TimedThink);
		pev->nextthink = gpGlobals->time;
	}
}

void CTriggerCondition::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	isActive = useType == USE_TOGGLE ? !isActive : useType == USE_ON;
	h_activator = pActivator;
	h_caller = pCaller;

	m_checkedFirstResult = false;
	m_lastResult = -1;

	if (pev->spawnflags & SF_CYCLIC) {
		Evaluate();
	}
	else {
		if (isActive) {
			Evaluate();
			SetThink(&CTriggerCondition::TimedThink);
			pev->nextthink = gpGlobals->time + m_checkInterval;
		}
		else {
			SetThink(NULL);
			pev->nextthink = 0;
		}
	}
}

void CTriggerCondition::TimedThink() {
	Evaluate();
	pev->nextthink = gpGlobals->time + m_checkInterval;
}

CKeyValue CTriggerCondition::LoadKey(string_t entName, string_t keyName, const char* errorDesc) {
	CKeyValue key;
	memset(&key, 0, sizeof(CKeyValue));

	edict_t* ent = NULL;
	if (!strcmp(STRING(entName), "!activator")) {
		ent = h_activator.GetEdict();
	}
	else if (!strcmp(STRING(entName), "!caller")) {
		ent = h_caller.GetEdict();
	}
	else {
		ent = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(entName));
	}

	CBaseEntity* pent = CBaseEntity::Instance(ent);

	if (!pent) {
		ALERT(at_console, "'%s' (trigger_condition): %s ent %s not found\n",
			pev->targetname ? STRING(pev->targetname) : "", errorDesc, STRING(entName));
		return key;
	}

	key = pent->GetKeyValue(STRING(keyName));
	if (!key.desc) {
		ALERT(at_console, "'%s' (trigger_condition): %s ent key %s is invalid\n",
			pev->targetname ? STRING(pev->targetname) : "", errorDesc, STRING(keyName));
	}

	return key;
}

void CTriggerCondition::Evaluate() {
	if (!pev->target || !m_monitoredKey) {
		ALERT(at_console, "'%s' (trigger_condition): missing monitored entity/key\n",
			pev->targetname ? STRING(pev->targetname) : "");
		return;
	}
	if (m_checkType < 0 || m_checkType >= COMPARE_TYPES) {
		ALERT(at_console, "'%s' (trigger_condition): invalid check type %d\n",
			pev->targetname ? STRING(pev->targetname) : "", m_checkType);
		return;
	}
	if (m_iCheckBehavior < 0 || m_iCheckBehavior >= CONSTANT_MODES) {
		ALERT(at_console, "'%s' (trigger_condition): invalid constant mode behavior %d\n",
			pev->targetname ? STRING(pev->targetname) : "", m_checkType);
		return;
	}

	CKeyValue monitorKey = LoadKey(pev->target, m_monitoredKey, "monitored");
	if (!monitorKey.desc) {
		return;
	}

	CKeyValue compareKey;

	if (m_compareEntity && m_compareKey) {
		compareKey = LoadKey(m_compareEntity, m_compareKey, "compare");
		if (!compareKey.desc) {
			return;
		}
	}
	else {
		// static value
		if (m_compareValue) {
			compareKey.desc = &g_staticCompareKeyTypeString;
			compareKey.keyType = KEY_TYPE_STRING;
			compareKey.sVal = m_compareValue;
		}
		else {
			compareKey.desc = &g_staticCompareKeyTypeInt;
			compareKey.keyType = KEY_TYPE_INT;
			compareKey.iVal = 0;
		}
	}

	bool result = Compare(monitorKey, compareKey);
	bool isCyclic = pev->spawnflags & SF_CYCLIC;
	bool ignoreFirstResult = pev->spawnflags & SF_IGNORE_INITIAL_RESULT;
	bool shouldFireResultTarget = !ignoreFirstResult || m_checkedFirstResult || isCyclic;

	if (!isCyclic && shouldFireResultTarget) {
		switch (m_iCheckBehavior) {
		case MODE_WAIT_BOTH:
			shouldFireResultTarget = m_lastResult != result;
			break;
		case MODE_WAIT_AFTER_FALSE:
			shouldFireResultTarget = m_lastResult != result || result == true;
			break;
		case MODE_WAIT_AFTER_TRUE:
			shouldFireResultTarget = m_lastResult != result || result == false;
			break;
		case MODE_WAIT_NEVER:
		default:
			break;
		}
	}

	if (shouldFireResultTarget) {
		CBaseEntity* activator = (pev->spawnflags & SF_KEEP_ACTIVATOR) ? h_activator.GetEntity() : this;

		if (result && pev->netname) {
			ALERT(at_console, "'%s' (%s): Firing TRUE target %s\n",
				pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), STRING(pev->netname));
			FireTargets(STRING(pev->netname), activator, this, USE_TOGGLE, 0.0f);
		}
		else if (!result && pev->message) {
			ALERT(at_console, "'%s' (%s): Firing FALSE target %s\n",
				pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), STRING(pev->netname));
			FireTargets(STRING(pev->message), activator, this, USE_TOGGLE, 0.0f);
		}
	}

	m_checkedFirstResult = true;
	m_lastResult = result;
}

bool CTriggerCondition::Compare(const CKeyValue& monitorKey, const CKeyValue& compareKey) {
	// TODO: does this ent follow c promotion rules? Would an int compared to a float mean
	// the int is promoted to float, regardless of which side the float was on?

	switch (monitorKey.keyType) {
	case KEY_TYPE_FLOAT:
		return CompareFloats(monitorKey, compareKey);
	case KEY_TYPE_INT:
		return CompareInts(monitorKey, compareKey);
	case KEY_TYPE_VECTOR:
		return CompareVectors(monitorKey, compareKey);
	case KEY_TYPE_STRING:
		return CompareStrings(monitorKey, compareKey);
	default:
		ALERT(at_console, "'%s' (%s): cannot compare key %s (invalid type)\n",
			pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), monitorKey.desc->fieldName);
		return false;
	}
}

bool CTriggerCondition::CompareFloats(const CKeyValue& monitorKey, const CKeyValue& compareKey) {
	switch (m_checkType) {
	case COMPARE_EQUAL: return monitorKey.fVal == getValueAsFloat(compareKey);
	case COMPARE_NEQUAL: return monitorKey.fVal != getValueAsFloat(compareKey);
	case COMPARE_LESS: return monitorKey.fVal < getValueAsFloat(compareKey);
	case COMPARE_GREATER: return monitorKey.fVal > getValueAsFloat(compareKey);
	case COMPARE_LEQUAL: return monitorKey.fVal <= getValueAsFloat(compareKey);
	case COMPARE_GEQUAL: return monitorKey.fVal >= getValueAsFloat(compareKey);
	case COMPARE_BITSET:
	default:
		ALERT(at_console, "'%s' (%s): invalid compare type %d used on float key %s\n",
			pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), monitorKey.desc->fieldName);
		return false;
	}
}

bool CTriggerCondition::CompareInts(const CKeyValue& monitorKey, const CKeyValue& compareKey) {
	switch (m_checkType) {
	case COMPARE_EQUAL: return monitorKey.iVal == getValueAsInt(compareKey);
	case COMPARE_NEQUAL: return monitorKey.iVal != getValueAsInt(compareKey);
	case COMPARE_LESS: return monitorKey.iVal < getValueAsInt(compareKey);
	case COMPARE_GREATER: return monitorKey.iVal > getValueAsInt(compareKey);
	case COMPARE_LEQUAL: return monitorKey.iVal <= getValueAsInt(compareKey);
	case COMPARE_GEQUAL: return monitorKey.iVal >= getValueAsInt(compareKey);
	case COMPARE_BITSET: return monitorKey.iVal & getValueAsInt(compareKey);
	default:
		ALERT(at_console, "'%s' (%s): invalid compare type %d used on int key %s\n",
			pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), monitorKey.desc->fieldName);
		return false;
	}
}

bool CTriggerCondition::CompareVectors(const CKeyValue& monitorKey, const CKeyValue& compareKey) {
	Vector monitorVector = monitorKey.vVal;
	Vector compareVector = compareKey.vVal;
	bool compareKeyIsVector = compareKey.keyType == KEY_TYPE_VECTOR;
	
	if (!compareKeyIsVector && compareKey.keyType == KEY_TYPE_STRING && UTIL_StringIsVector(STRING(compareKey.sVal))) {
		compareKeyIsVector = true;
		UTIL_StringToVector(compareVector, STRING(compareKey.sVal));
	}

	if (pev->spawnflags & SF_DONT_USE_X) {
		monitorVector.x = 0;
		compareVector.x = 0;
	}
	if (pev->spawnflags & SF_DONT_USE_Y) {
		monitorVector.y = 0;
		compareVector.y = 0;
	}
	if (pev->spawnflags & SF_DONT_USE_Z) {
		monitorVector.z = 0;
		compareVector.z = 0;
	}

	// lengths or single components are compared if the compare value is not a vector,
	// or if check type is anything but ==/!=
	float monitorFloat = getValueAsFloat(monitorVector);
	float compareFloat = getValueAsFloat(compareKey);

	switch (m_checkType) {
	case COMPARE_EQUAL:
		if (compareKeyIsVector) {
			return monitorVector == compareVector;
		}
		else {
			return monitorFloat == compareFloat;
		}
	case COMPARE_NEQUAL:
		if (compareKeyIsVector) {
			return monitorVector != compareVector;
		}
		else {
			return monitorFloat != compareFloat;
		}
	case COMPARE_LESS: return monitorFloat < compareFloat;
	case COMPARE_GREATER: return monitorFloat > compareFloat;
	case COMPARE_LEQUAL: return monitorFloat <= compareFloat;
	case COMPARE_GEQUAL: return monitorFloat >= compareFloat;
	case COMPARE_BITSET:
	default:
		ALERT(at_console, "'%s' (%s): invalid compare type %d used on vector key %s\n",
			pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), monitorKey.desc->fieldName);
		return false;
	}
}

bool CTriggerCondition::CompareStrings(const CKeyValue& monitorKey, const CKeyValue& compareKey) {
	switch (m_checkType) {
	case COMPARE_EQUAL: return !strcmp(STRING(monitorKey.sVal), getValueAsString(compareKey));
	case COMPARE_NEQUAL: return strcmp(STRING(monitorKey.sVal), getValueAsString(compareKey));
	case COMPARE_LESS:
	case COMPARE_GREATER:
	case COMPARE_LEQUAL:
	case COMPARE_GEQUAL:
	case COMPARE_BITSET:
	default:
		ALERT(at_console, "'%s' (%s): invalid compare type %d used on string key %s\n",
			pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), monitorKey.desc->fieldName);
		return false;
	}
}

float CTriggerCondition::getValueAsFloat(const CKeyValue& key) {
	switch (key.keyType) {
	case KEY_TYPE_FLOAT:
		return key.fVal;
	case KEY_TYPE_INT:
		return key.iVal;
	case KEY_TYPE_VECTOR:
		return getValueAsFloat(key.vVal);
	case KEY_TYPE_STRING: {
		if (UTIL_StringIsVector(STRING(key.sVal))) {
			Vector vTemp;
			UTIL_StringToVector(vTemp, STRING(key.sVal));
			return getValueAsFloat(vTemp);
		}
		else {
			return atof(STRING(key.sVal));
		}
	}
	default:
		ALERT(at_console, "'%s' (%s): cannot convert key %s to float (invalid type)\n",
			pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), key.desc->fieldName);
		return 0;
	}
}

float CTriggerCondition::getValueAsFloat(Vector v) {
	int dont_use_flags = SF_DONT_USE_X | SF_DONT_USE_Y | SF_DONT_USE_Z;

	if ((pev->spawnflags & dont_use_flags) == (SF_DONT_USE_Y | SF_DONT_USE_Z)) {
		return v.x;
	}
	if ((pev->spawnflags & dont_use_flags) == (SF_DONT_USE_X | SF_DONT_USE_Z)) {
		return v.y;
	}
	if ((pev->spawnflags & dont_use_flags) == (SF_DONT_USE_X | SF_DONT_USE_Y)) {
		return v.z;
	}

	if (pev->spawnflags & SF_DONT_USE_X) {
		v.x = 0;
	}
	if (pev->spawnflags & SF_DONT_USE_Y) {
		v.y = 0;
	}
	if (pev->spawnflags & SF_DONT_USE_Z) {
		v.z = 0;
	}

	return v.Length();
}

int CTriggerCondition::getValueAsInt(const CKeyValue& key) {
	switch (key.keyType) {
	case KEY_TYPE_FLOAT:
	case KEY_TYPE_INT:
	case KEY_TYPE_VECTOR:
		return getValueAsFloat(key);
	case KEY_TYPE_STRING: {
		if (UTIL_StringIsVector(STRING(key.sVal))) {
			return getValueAsFloat(key);
		}
		else {
			return atoi(STRING(key.sVal)); // atoi - the only reason getValueAsInt exists
		}
	}
	default:
		ALERT(at_console, "'%s' (%s): cannot convert key %s to int (invalid type)\n",
			pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), key.desc->fieldName);
		return 0;
	}
}

const char* CTriggerCondition::getValueAsString(const CKeyValue& key) {
	switch (key.keyType) {
	case KEY_TYPE_FLOAT:
		return UTIL_VarArgs("%f", key.fVal);
	case KEY_TYPE_INT:
		return UTIL_VarArgs("%d", key.iVal);
	case KEY_TYPE_VECTOR: {
		// TODO: code duplicated in trigger_changevalue
		std::string s;

		if (!(pev->spawnflags & SF_DONT_USE_X)) {
			s += UTIL_VarArgs("%f", key.vVal[0]);
		}
		if (!(pev->spawnflags & SF_DONT_USE_Y)) {
			if (s.length()) {
				s += " ";
			}
			s += UTIL_VarArgs("%f", key.vVal[1]);
		}
		if (!(pev->spawnflags & SF_DONT_USE_Z)) {
			if (s.length()) {
				s += " ";
			}
			s += UTIL_VarArgs("%f", key.vVal[2]);
		}

		return UTIL_VarArgs("%2", s.c_str());
	}
	case KEY_TYPE_STRING:
		return STRING(key.sVal);
	default:
		ALERT(at_console, "'%s' (%s): cannot convert key %s to string (invalid type)\n",
			pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), key.desc->fieldName);
		return "";
	}
}
