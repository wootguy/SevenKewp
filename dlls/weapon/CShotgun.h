#pragma once
#include "CBasePlayerWeapon.h"

#define SHOTGUN_MAX_CLIP		8
#define SHOTGUN_DEFAULT_GIVE	12

class CShotgun : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save ) override;
	int		Restore( CRestore &restore ) override;
	static	TYPEDESCRIPTION m_SaveData[];
#endif


	void Spawn( void ) override;
	void Precache( void ) override;
	void PrecacheEvents() override;
	int iItemSlot( ) override { return 3; }
	int GetItemInfo(ItemInfo *p) override;

	void PrimaryAttack( void ) override;
	void SecondaryAttack( void ) override;
	BOOL Deploy( ) override;
	void Reload( void ) override;
	void WeaponIdle( void ) override;
	int m_fInReload;
	float m_flNextReload;
	int m_iShell;

	void ItemPostFrame(void) override;

	virtual int MergedModelBody() override { return MERGE_MDL_W_SHOTGUN; }

	virtual BOOL UseDecrement( void ) override
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usDoubleFire;
	unsigned short m_usSingleFire;
};
