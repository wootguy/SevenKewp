#include "extdll.h"
#include "util.h"
#include "CBaseDMStart.h"
#include "CBasePlayer.h"

#define FL_LANDMARK_SPAWN_POINT 1
#define FL_LANDMARK_USE_ANGLES 2

class EXPORT CInfoLandmark : public CPointEntity
{
public:
	void Spawn(void);
	void CreateSpawnPoints(void);
private:
};

LINK_ENTITY_TO_CLASS(info_landmark, CInfoLandmark)

void CInfoLandmark::Spawn() {
	CPointEntity::Spawn();

	if (pev->spawnflags & FL_LANDMARK_SPAWN_POINT) {
		SetThink(&CInfoLandmark::CreateSpawnPoints);
		pev->nextthink = gpGlobals->time;
	}
}

void CInfoLandmark::CreateSpawnPoints() {
	Vector vecStart = pev->origin;

	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time;

	TraceResult tr;
	UTIL_TraceLine(vecStart, vecStart - Vector(0, 0, 4096), ignore_monsters, NULL, &tr);

	if (tr.fAllSolid) {
		ALERT(at_warning, "Landmark '%s' is inside a solid. Spawnpoints not created.\n",
			STRING(pev->targetname));
		return;
	}

	Vector bottom = tr.vecEndPos;
	UTIL_TraceLine(bottom, bottom + Vector(0, 0, 4096), ignore_monsters, NULL, &tr);
	Vector top = tr.vecEndPos;
	Vector mid = bottom + (top - bottom) * 0.5f;

	const int minVertSpace = VEC_HUMAN_HULL_DUCK.z + 1;
	if (top.z - bottom.z < minVertSpace) {
		EALERT(at_warning, "Less than %d units of vertical space. Spawnpoints not created.\n", minVertSpace);
		return;
	}

	UTIL_TraceHull(mid, mid - Vector(0, 0, 4096), ignore_monsters, head_hull, NULL, &tr);
	Vector hullBottom = tr.vecEndPos;

	if (tr.fAllSolid) {
		EALERT(at_warning, "Too close to a wall. Spawnpoints not created.\n");
		return;
	}

	// find the nearest level trigger, and aim away from that
	CBaseEntity* pEnt = NULL;
	float bestDist = FLT_MAX;
	CBaseEntity* bestLevelChange = NULL;
	while ((pEnt = UTIL_FindEntityByClassname(pEnt, "trigger_changelevel")) != NULL) {
		float dist = (pEnt->Center() - pev->origin).Length();
		if (dist < bestDist) {
			bestDist = dist;
			bestLevelChange = pEnt;
		}
	}

	Vector angles = pev->angles; // fall back to landmark angles
	if (bestLevelChange && !(pev->spawnflags & FL_LANDMARK_USE_ANGLES)) {
		Vector dir = (pev->origin - bestLevelChange->Center());
		dir.z = 0; // no vertical angles

		Vector ori = bestLevelChange->Center();

		angles = UTIL_VecToAngles(dir.Normalize());

		// snap to 90 degree angles
		angles.y = (((int)angles.y + 45) / 90) * 90;
	}

	CBaseEntity::Create("info_player_deathmatch", hullBottom, angles, true);
	EALERT(at_console, "Created spawn point\n");
}