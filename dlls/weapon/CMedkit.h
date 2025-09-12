#pragma once
#include "CBasePlayerWeapon.h"

class CMedkit : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 1; }
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	void WeaponIdle();
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount);
	virtual int AddToPlayer(CBasePlayer* pPlayer);

	virtual int MergedModelBody() { return MERGE_MDL_W_PMEDKIT; }

	BOOL IsClientWeapon() { return FALSE; }

	bool CanHealTarget(CBaseEntity* ent);

	void RechargeAmmo();

	float m_reviveChargedTime; // time when target will be revive charge will complete
	float m_rechargeTime; // time until regenerating ammo

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};
