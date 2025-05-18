#pragma once
#include "CCsPlayerWeapon.h"

const float GLOCK18_MAX_SPEED     = 250.0f;
const float GLOCK18_RANGE_MODIFER = 0.75f;
const float GLOCK18_RELOAD_TIME   = 2.2f;

#define GLOCK18_DEFAULT_GIVE 20

#define MAX_AMMO_9MM_CS16 120
#define GLOCK18_MAX_CLIP 20
#define GLOCK18_WEIGHT 5
#define WPNSTATE_GLOCK18_BURST_MODE (1 << 1)

#define WPNSTATE_SHIELD_DRAWN (1 << 5)

enum glock18_e
{
	GLOCK18_IDLE1,
	GLOCK18_IDLE2,
	GLOCK18_IDLE3,
	GLOCK18_SHOOT,
	GLOCK18_SHOOT2,
	GLOCK18_SHOOT3,
	GLOCK18_SHOOT_EMPTY,
	GLOCK18_RELOAD,
	GLOCK18_DRAW,
	GLOCK18_HOLSTER,
	GLOCK18_ADD_SILENCER,
	GLOCK18_DRAW2,
	GLOCK18_RELOAD2,
};

enum glock18_shield_e
{
	GLOCK18_SHIELD_IDLE1,
	GLOCK18_SHIELD_SHOOT,
	GLOCK18_SHIELD_SHOOT2,
	GLOCK18_SHIELD_SHOOT_EMPTY,
	GLOCK18_SHIELD_RELOAD,
	GLOCK18_SHIELD_DRAW,
	GLOCK18_SHIELD_IDLE_UP,
	GLOCK18_SHIELD_UP,
	GLOCK18_SHIELD_DOWN,
};

class CGLOCK18: public CCsPlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual void PrecacheEvents();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void Reload();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
	#ifdef CLIENT_WEAPONS
		return TRUE;
	#else
		return FALSE;
	#endif
	}
	virtual BOOL IsPistol() { return TRUE; }
	virtual bool IsSecondaryWeapon() { return true; }

public:
	void GLOCK18Fire(float flSpread, float flCycleTime, BOOL bFireBurst);

public:
	bool m_bBurstFire;
};