#pragma once
#include "extdll.h"
#include "CBaseTurret.h"

#define TURRET_GLOW_SPRITE "sprites/flare3.spr"

class EXPORT CTurret : public CBaseTurret
{
public:
	void Spawn(void);
	void Precache(void);
	const char* DisplayName();
	// Think functions
	void SpinUpCall(void);
	void SpinDownCall(void);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy);

private:
	int m_iStartSpin;

};
