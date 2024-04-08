#pragma once

edict_t* EntSelectSpawnPoint(CBaseEntity* pPlayer);

BOOL IsSpawnPointClear(CBaseEntity* pPlayer, CBaseEntity* pSpot);

inline int FNullEnt(CBaseEntity* ent) { return (ent == NULL) || FNullEnt(ent->edict()); }
