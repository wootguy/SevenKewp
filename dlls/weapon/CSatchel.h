#pragma once
#include "CBasePlayerWeapon.h"
#include "CGrenade.h"

#define SATCHEL_DEFAULT_GIVE		1

class CSatchel : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	int iItemSlot( void ) { return 5; }
	int GetItemInfo(ItemInfo *p);
	int AddToPlayer( CBasePlayer *pPlayer );
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int AddDuplicate( CBasePlayerItem *pOriginal );
	BOOL CanDeploy( void );
	BOOL Deploy( void );
	BOOL IsUseable( void );
	
	void Holster( int skiplocal = 0 );
	void WeaponIdle( void );
	void Throw( void );
	
	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};

class CSatchelCharge : public CGrenade
{
	void Spawn(void);
	void Precache(void);
	void BounceSound(void);

	void EXPORT SatchelSlide(CBaseEntity* pOther);
	void EXPORT SatchelThink(void);

public:
	void Deactivate(void);
};

void DeactivateSatchels(CBasePlayer* pOwner);