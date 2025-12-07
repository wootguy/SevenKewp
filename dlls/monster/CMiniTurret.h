#pragma once
#include "extdll.h"
#include "CBaseTurret.h"

class EXPORT CMiniTurret : public CBaseTurret
{
public:
	void Spawn();
	void Precache(void);
	const char* DisplayName() { return m_displayName ? CBaseMonster::DisplayName() : "Mini Turret"; }
	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy);
};
