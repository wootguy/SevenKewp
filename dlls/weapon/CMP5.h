#pragma once
#include "CBasePlayerWeapon.h"

#define MP5_MAX_CLIP			50
#define MP5_DEFAULT_GIVE		50
#define MP5_DEFAULT_AMMO		50
#define MP5_M203_DEFAULT_GIVE	0

class CMP5 : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	void PrecacheEvents();
	int iItemSlot( void ) { return 3; }
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	int SecondaryAmmoIndex( void );
	BOOL Deploy( void );
	void Reload( void );
	void WeaponIdle( void );
	float m_flNextAnimTime;
	int m_iShell;
	const char* DisplayName() override { return "MP5"; }

	virtual int MergedModelBody() { return MERGE_MDL_W_9MMAR; }

	virtual BOOL UseDecrement( void )
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