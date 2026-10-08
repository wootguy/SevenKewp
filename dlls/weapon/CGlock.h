#pragma once
#include "CBasePlayerWeapon.h"

#define GLOCK_MAX_CLIP			17
#define GLOCK_DEFAULT_GIVE		17

class CGlock : public CBasePlayerWeapon
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void PrecacheEvents() override;
	int iItemSlot( void ) override { return 2; }
	int GetItemInfo(ItemInfo *p) override;

	void PrimaryAttack( void ) override;
	void SecondaryAttack( void ) override;
	void GlockFire( float flSpread, float flCycleTime, BOOL fUseAutoAim );
	BOOL Deploy( void ) override;
	void Reload( void ) override;
	void WeaponIdle( void ) override;
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) override;

	virtual int MergedModelBody() override { return MERGE_MDL_W_9MMHANDGUN; }

	virtual BOOL UseDecrement( void ) override
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	int m_iShell;

	unsigned short m_usFireGlock1;
	unsigned short m_usFireGlock2;
};
