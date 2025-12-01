#include "extdll.h"
#include "util.h"
#include "water.h"
#include "CBaseEntity.h"
#include "te_effects.h"

struct WaterEntState {
	EHANDLE h_ent;
	int oldContents;
	Vector oldPos;
};

std::vector<WaterEntState> g_waterPhysicsEnts;

void UnloadWaterPhysicsEnts() {
	g_waterPhysicsEnts.clear();
}

void AddWaterPhysicsEnt(CBaseEntity* ent, float waterFriction, float buoyancy) {
	if (ent) {
		ent->m_waterFriction = waterFriction;
		ent->m_buoyancy = buoyancy;

		for (int i = 0; i < g_waterPhysicsEnts.size(); i++) {
			if (g_waterPhysicsEnts[i].h_ent.GetEntity() == ent) {
				return;
			}
		}

		WaterEntState state;
		state.oldContents = UTIL_PointContents(ent->pev->origin);
		state.h_ent = EHANDLE(ent->edict());
		g_waterPhysicsEnts.push_back(state);
	}
}

uint32_t pcg_hash(uint32_t x) {
	x ^= x >> 16;
	x *= 0x7feb352d;
	x ^= x >> 15;
	x *= 0x846ca68b;
	x ^= x >> 16;
	return x;
}

bool isLiquidContents(int contents) {
	switch (contents) {
	case CONTENTS_WATER:
	case CONTENTS_SLIME:
	case CONTENTS_LAVA:
		return true;
	}

	return false;
}

void DoEntWaterPhysics() {
	static float lastPhysics;

	if (gpGlobals->time - lastPhysics < 0.05f || lastPhysics > gpGlobals->time)
		return;

	for (int i = 0; i < g_waterPhysicsEnts.size(); i++) {
		WaterEntState& state = g_waterPhysicsEnts[i];
		CBaseEntity* ent = state.h_ent;

		if (!ent) {
			g_waterPhysicsEnts.erase(g_waterPhysicsEnts.begin() + i);
			i--;
			continue;
		}

		Vector origin = ent->pev->origin;

		if (ent->IsPlayerCorpse() || ent->IsPlayer()) {
			origin.z -= (ent->pev->flags & FL_DUCKING) ? 18 : 36;
		}

		float thickness = V_max(ent->pev->absmax.x - ent->pev->absmin.x,
			ent->pev->absmax.y - ent->pev->absmin.y);

		if (ent->IsNormalMonster() && !ent->IsAlive())
			origin.z += thickness * 0.25f; // sink a bit into the water

		int contents = UTIL_PointContents(origin);
		bool inLiquid = isLiquidContents(contents);

		CBaseMonster* mon = ent->MyMonsterPointer();

		if (inLiquid) {
			int irnd = pcg_hash(ent->entindex());
			float frnd = UTIL_SharedRandomFloat(irnd, 0, 2 * M_PI);
			float t = (gpGlobals->time * 0.1f + frnd);
			float t2 = (gpGlobals->time + frnd);
			float currentX = sinf(t) * 0.01f;
			float currentY = cosf(t) * 0.01f;

			if (fabs(ent->pev->velocity.x + currentX) < 4.0f)
				ent->pev->velocity.x += currentX;
			if (fabs(ent->pev->velocity.y + currentY) < 4.0f)
				ent->pev->velocity.y += currentY;

			ent->pev->avelocity.y = 10.0f * (irnd % 2 ? 1 : -1);

			if (ent->IsPlayerCorpse()) {
				ALERT(at_console, "");
			}

			float waterLevel = UTIL_WaterLevel(origin, origin.z, origin.z + 16);
			bool slowlyFloating = fabs(ent->pev->velocity.z) < 20.0f && ent->m_buoyancy > 0;
			bool nearSurface = fabs(origin.z - waterLevel) < 4;

			if (slowlyFloating && nearSurface) {
				ent->pev->gravity = FLT_MIN;
				ent->pev->velocity.z = 0;
			}
			else {
				if (ent->m_waterFriction != 1.0f)
					ent->pev->velocity = ent->pev->velocity * ent->m_waterFriction;

				if (ent->m_buoyancy != 0)
					ent->pev->gravity = -ent->m_buoyancy;
			}
		}
		else {
			if (ent->m_buoyancy != 0)
				ent->pev->gravity = 0;
		}

		bool wasInLiquid = isLiquidContents(state.oldContents);
		bool solidTransition = state.oldContents == CONTENTS_SOLID || contents == CONTENTS_SOLID;
		bool playerJumpingOut = ent->IsPlayer() && !inLiquid;

		// water transiation
		if (wasInLiquid != inLiquid && !solidTransition && !playerJumpingOut) {
			if (fabs(ent->pev->velocity.Length()) > 100) {
				float scale = V_max(0.3f, 0.3f * (thickness / 16.0f));

				if (ent->IsItem()) {
					scale = 0.3f;
				}

				Vector splashPos = inLiquid ? origin : state.oldPos;

				UTIL_WaterSplash(splashPos, false, true, scale);
			}
		}

		state.oldContents = contents;
		state.oldPos = origin;
	}
}