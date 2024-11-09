#pragma once

class CBaseDMStart : public CPointEntity
{
public:
	void		Spawn(void);
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	BOOL		IsTriggered(CBaseEntity* pEntity);
	void		KeyValue(KeyValueData* pkvd);

	void		SpawnPlayer(CBasePlayer* plr);

private:
	bool isActive;
	int triggerState;
};

edict_t* EntSelectSpawnPoint(CBaseEntity* pPlayer, bool includeDisabledSpawns=false);

BOOL IsSpawnPointClear(CBaseEntity* pPlayer, CBaseEntity* pSpot);

inline int FNullEnt(CBaseEntity* ent) { return (ent == NULL) || FNullEnt(ent->edict()); }
