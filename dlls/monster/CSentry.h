#pragma once
#include "extdll.h"
#include "CBaseTurret.h"

//=========================================================
// Sentry gun - smallest turret, placed near grunt entrenchments
//=========================================================
class EXPORT CSentry : public CBaseTurret
{
public:
	void Spawn();
	void Precache(void);
	const char* DisplayName();
	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void SentryTouch(CBaseEntity* pOther);
	void SentryDeath(void);
	void DropInit(void);
	void Deploy(void);

};

