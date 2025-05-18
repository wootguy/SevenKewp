#pragma once
#include "CBasePlayerWeapon.h"

const float GLOCK18_DAMAGE = 25.0f;

class CCsPlayerWeapon : public CBasePlayerWeapon {
public:
	float m_flAccuracy;
	int m_iShotsFired;
	float m_flLastFire;
	bool m_bDelayFire;
	float m_flDecreaseShotsFired;
	int m_iShell;
	float m_flNextReload;
	int m_iSwing;

	int m_iGlock18ShotsFired = 0;
	float m_flGlock18Shoot = 0;

	float m_flFamasShoot;
	int m_iFamasShotsFired;

	virtual CCsPlayerWeapon* GetWeaponPtrCs16(void) { return this; };
	virtual void ItemPostFrame() override;
	void FireRemaining(int& shotsFired, float& shootTime, BOOL bIsGlock);
	void EjectBrassLate();

	virtual bool IsSecondaryWeapon() { return false; }
	virtual bool HasSecondaryAttack();
};

extern int m_usFireGlock18;