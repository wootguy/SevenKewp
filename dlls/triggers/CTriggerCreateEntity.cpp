#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CRuleEntity.h"

//
// CTriggerCreateEntity / trigger_createentity -- spawns any entity

#define MAX_CREATE_KEYS 64

enum create_key_value_types {
	CREATE_KEY_STATIC, // value is used as is
	CREATE_KEY_COPY // value is the name of an entity to copy from
};

struct create_key_t {
	string_t key_name;
	string_t key_value;
	int valueType;
};

class CTriggerCreateEntity : public CPointEntity
{
public:
	void Precache(void);
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	string_t m_childClass;
	string_t m_triggerAfterSpawn;
	create_key_t m_keys[MAX_CREATE_KEYS];
	int m_iKeys;
};

LINK_ENTITY_TO_CLASS(trigger_createentity, CTriggerCreateEntity);

void CTriggerCreateEntity::KeyValue(KeyValueData* pkvd)
{
	if (pkvd->szKeyName[0] == '-' || pkvd->szKeyName[0] == '+')
	{
		if (m_iKeys >= MAX_CREATE_KEYS) {
			ALERT(at_console, "trigger_createentity: exceeded 64 key limit\n");
		}
		else if (strlen(pkvd->szKeyName) > 1) {
			create_key_t key;

			key.key_name = ALLOC_STRING(pkvd->szKeyName + 1);
			key.key_value = ALLOC_STRING(pkvd->szValue);
			key.valueType = pkvd->szKeyName[0] == '-' ? CREATE_KEY_STATIC : CREATE_KEY_COPY;

			m_keys[m_iKeys++] = key;
		}

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszCrtEntChildName"))
	{
		if (m_iKeys >= MAX_CREATE_KEYS) {
			ALERT(at_console, "trigger_createentity: exceeded 64 key limit\n");
		}
		else {
			create_key_t key;

			key.key_name = ALLOC_STRING("targetname");
			key.key_value = ALLOC_STRING(pkvd->szValue);
			key.valueType = CREATE_KEY_STATIC;

			m_keys[m_iKeys++] = key;
		}

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszCrtEntChildClass"))
	{
		m_childClass = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iszTriggerAfter"))
	{
		m_triggerAfterSpawn = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CPointEntity::KeyValue(pkvd);
	}
}

void CTriggerCreateEntity::Precache() {
	std::map<std::string, std::string> keys;
	
	for (int i = 0; i < m_iKeys; i++) {
		if (m_keys[i].valueType == CREATE_KEY_COPY) {
			continue; // don't care about copied keys. The copied ent should precache itself.
		}

		keys[STRING(m_keys[i].key_name)] = STRING(m_keys[i].key_value);
	}

	if (m_childClass)
		UTIL_PrecacheOther(STRING(m_childClass), keys);
}

void CTriggerCreateEntity::Spawn(void)
{
	Precache();
	CPointEntity::Spawn();
}

void CTriggerCreateEntity::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!m_childClass) {
		ALERT(at_console, "%s (%s): Spawn aborted. Missing child class.\n",
			pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname));
		return;
	}

	std::map<std::string, std::string> keys;

	for (int i = 0; i < m_iKeys; i++) {
		const char* keyName = STRING(m_keys[i].key_name);
		const char* keyValue = STRING(m_keys[i].key_value);

		if (m_keys[i].valueType == CREATE_KEY_STATIC) {
			keys[keyName] = keyValue;
			continue;
		}

		edict_t* ent = FIND_ENTITY_BY_TARGETNAME(NULL, keyValue);
		CBaseEntity* pent = CBaseEntity::Instance(ent);

		if (!pent) {
			if (!strcmp(STRING(m_keys[i].key_name), "model")) {
				ALERT(at_console, "%s (%s): Spawn aborted. Couldn't find entity '%s' to copy model from.\n",
					pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), keyValue);
				return;
			}
		}

		CKeyValue srcKey = pent->GetKeyValue(keyName);
		if (srcKey.keyType) {
			switch (srcKey.keyType) {
			case KEY_TYPE_FLOAT:
				keys[keyName] = UTIL_VarArgs("%f", srcKey.fVal);
				break;
			case KEY_TYPE_INT:
				keys[keyName] = UTIL_VarArgs("%d", srcKey.iVal);
				break;
			case KEY_TYPE_VECTOR: {
				keys[keyName] = UTIL_VarArgs("%f %f %f", srcKey.vVal[0], srcKey.vVal[1], srcKey.vVal[2]);
				break;
			}
			case KEY_TYPE_STRING:
				keys[keyName] = STRING(srcKey.sVal);
				break;
			default:
				ALERT(at_console, "'%s' (%s): failed to copy keyvalue '%s' (invalid type)\n",
					pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), srcKey.keyName);
				break;
			}
		}
		else {
			ALERT(at_console, "'%s' (%s): failed to copy keyvalue '%s' (mistyped or private)\n",
				pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), srcKey.keyName);
		}
	}

	CBaseEntity::Create(STRING(m_childClass), pev->origin, pev->angles, NULL, keys);

	if (m_triggerAfterSpawn) {
		FireTargets(STRING(m_triggerAfterSpawn), pActivator, this, USE_TOGGLE, 0);
	}
}
