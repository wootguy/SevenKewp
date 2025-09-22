#include "extdll.h"
#include "util.h"
#include <fstream>

class CTriggerLoad : public CPointEntity
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

private:
	string_t m_iszTrigger;
};

LINK_ENTITY_TO_CLASS(trigger_load, CTriggerLoad)

void CTriggerLoad::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszTrigger"))
	{
		m_iszTrigger = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

// load keyvalues saved with trigger_save
StringMap UTIL_LoadMapSaveKeys(const char* path) {
	std::string fpath = getGameFilePath(path);
	std::ifstream infile(fpath);

	StringMap mapKeys;

	if (fpath.empty() || !infile.is_open()) {
		ALERT(at_console, "Failed to load map save file: %s\n", path);
		return mapKeys;
	}

	int lineNum = 0;
	std::string line;
	while (std::getline(infile, line))
	{
		lineNum++;
		std::string paths[2];

		line = trimSpaces(line);
		if (line.empty()) {
			continue;
		}

		int colonIdx = line.find(":");
		if (colonIdx == -1)
			continue;

		std::string keyName = line.substr(0, colonIdx);
		std::string keyVal = line.substr(colonIdx + 1);

		if (keyVal.empty() || keyVal.empty()) {
			continue;
		}

		mapKeys.put(keyName.c_str(), keyVal.c_str());
	}

	return mapKeys;
}


void CTriggerLoad::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
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

	std::string fname = "maps/" + toLowerCase(STRING(gpGlobals->mapname)) + ".sav";
	StringMap mapKeys = UTIL_LoadMapSaveKeys(fname.c_str());

	const char* kval = mapKeys.get(STRING(pev->netname));
	if (!kval) {
		EALERT(at_console, "Key not in save data: %s\n", STRING(pev->netname));
		return;
	}

	KeyValueData dat;
	dat.fHandled = false;
	dat.szClassName = STRING(ent->pev->classname);
	dat.szKeyName = STRING(pev->message);
	dat.szValue = kval;
	DispatchKeyValue(ent->edict(), &dat);

	// in case any changes to solid/origin were made
	UTIL_SetOrigin(ent->pev, ent->pev->origin);

	if (m_iszTrigger) {
		FireTargets(STRING(m_iszTrigger), pActivator, this, USE_TOGGLE);
	}
}
