#pragma once
#include "CBasePlayerWeapon.h"

#define MP5_MAX_CLIP			50
#define MP5_DEFAULT_GIVE		50
#define MP5_DEFAULT_AMMO		50
#define MP5_M203_DEFAULT_GIVE	0

class CMP5 : public CBasePlayerWeapon
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void PrecacheEvents() override;
	int iItemSlot( void ) override { return 3; }
	int GetItemInfo(ItemInfo *p) override;

	void PrimaryAttack( void ) override;
	void SecondaryAttack( void ) override;
	int SecondaryAmmoIndex( void ) override;
	BOOL Deploy( void ) override;
	void Reload( void ) override;
	void WeaponIdle( void ) override;
	float m_flNextAnimTime;
	int m_iShell;
	const char* DisplayName() override { return "MP5"; }

	virtual int MergedModelBody() override { return MERGE_MDL_W_9MMAR; }

	virtual BOOL UseDecrement( void ) override
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usMP5;
	unsigned short m_usMP52;
};