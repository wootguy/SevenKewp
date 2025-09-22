#include "extdll.h"
#include "util.h"

extern StringMap UTIL_LoadMapSaveKeys(const char* path);

class CTriggerSave : public CPointEntity
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Spawn();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

private:
	string_t m_iszTrigger;
};

LINK_ENTITY_TO_CLASS(trigger_save, CTriggerSave)

void CTriggerSave::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszTrigger"))
	{
		m_iszTrigger = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CTriggerSave::Spawn() {}

void CTriggerSave::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!pev->target || !pev->message || !pev->netname) {
		EALERT(at_warning, "Missing target, target key, or label\n");
		return;
	}

	CBaseEntity* ent = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));

	if (!ent) {
		EALERT(at_warning, "Target entity %s not found\n");
		return;
	}

	const char* keyName = STRING(pev->message);
	CKeyValue srcKey = ent->GetKeyValue(keyName);
	std::string saveDat = "";

	if (srcKey.keyType) {
		switch (srcKey.keyType) {
		case KEY_TYPE_FLOAT:
			saveDat = UTIL_VarArgs("%f", srcKey.fVal);
			break;
		case KEY_TYPE_INT:
			saveDat = UTIL_VarArgs("%d", srcKey.iVal);
			break;
		case KEY_TYPE_VECTOR:
			saveDat = UTIL_VarArgs("%f %f %f", srcKey.vVal[0], srcKey.vVal[1], srcKey.vVal[2]);
			break;
		case KEY_TYPE_STRING:
			saveDat = srcKey.sVal ? STRING(srcKey.sVal) : "";
			break;
		default:
			ALERT(at_warning, "'%s' (%s): operation on keyvalue %s not allowed\n",
				pev->targetname ? STRING(pev->targetname) : "", STRING(pev->classname), srcKey.keyName);
			return;
		}
	}

	std::string fname = "maps/" + toLowerCase(STRING(gpGlobals->mapname)) + ".sav";

	const char* saveLabel = STRING(pev->netname);
	static char savePath[MAX_PATH];
	GET_GAME_DIR(savePath);
	strcat_safe(savePath, UTIL_VarArgs("/%s", fname.c_str()), MAX_PATH);

	StringMap mapKeys = UTIL_LoadMapSaveKeys(fname.c_str());
	mapKeys.put(saveLabel, saveDat.c_str());

	FILE* saveFile = fopen(savePath, "w");

	if (!saveFile) {
		EALERT(at_error, "Failed to write: %s\n", savePath);
		return;
	}

	StringMap::iterator_t iter;
	while (mapKeys.iterate(iter)) {
		const char* saveLine = UTIL_VarArgs("%s:%s\n", iter.key, iter.value);
		fwrite(saveLine, strlen(saveLine), 1, saveFile);
	}

	fclose(saveFile);
	EALERT(at_console, "Wrote %s\n", savePath);

	if (m_iszTrigger) {
		FireTargets(STRING(m_iszTrigger), pActivator, this, USE_TOGGLE);
	}
}

