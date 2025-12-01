#pragma once
#include "Platform.h"

class CBaseEntity;

EXPORT void AddWaterPhysicsEnt(CBaseEntity* ent, float waterFriction, float buoyancy, float splashSize=0);

EXPORT void DoEntWaterPhysics();

EXPORT void UnloadWaterPhysicsEnts();