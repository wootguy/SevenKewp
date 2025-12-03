#include "CProjectileCustom.h"

LINK_ENTITY_TO_CLASS(custom_projectile, CProjectileCustom)

void CProjectileCustom::Spawn()
{
	Precache();
	pev->solid = SOLID_BBOX;
	pev->movetype = MOVETYPE_BOUNCE;
	pev->gravity = FLT_MIN;
	SET_MODEL(ENT(pev), STRING(pev->model));

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	SetTouch(&CProjectileCustom::DefaultTouch);
	SetThink(&CProjectileCustom::DefaultThink);
	pev->nextthink = gpGlobals->time + 0.1;

	//ParametricInterpolation(0.1f);
	AddWaterPhysicsEnt(this, 1, 0, 0.3f);
}


void CProjectileCustom::Precache() {}

int	CProjectileCustom::Classify(void)
{
	return CLASS_NONE;
}

void CProjectileCustom::DefaultTouch(CBaseEntity* pOther)
{
	if (CustomTouch(pOther)) {
		return;
	}

	SetTouch(NULL);
	SetThink(NULL);
	UTIL_Remove(this);
}

void CProjectileCustom::DefaultThink(void)
{
	if (CustomThink()) {
		return;
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

