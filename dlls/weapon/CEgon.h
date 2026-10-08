#pragma once
#include "CBasePlayerWeapon.h"

#define EGON_DEFAULT_GIVE			20

class CEgon : public CBasePlayerWeapon
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
	int iItemSlot( void ) override { return 4; }
	int GetItemInfo(ItemInfo *p) override;

	BOOL Deploy( void ) override;
	void Holster( int skiplocal = 0 ) override;

	void UpdateEffect( const Vector &startPoint, const Vector &endPoint, float timeBlend );

	void CreateEffect ( void );
	void DestroyEffect ( void );

	void EndAttack( void );
	void Attack( void );
	void PrimaryAttack( void ) override;
	BOOL ShouldWeaponIdle() override { return TRUE; }
	void WeaponIdle( void ) override;

	float m_flAmmoUseTime;// since we use < 1 point of ammo per update, we subtract ammo on a timer.

	float GetPulseInterval( void );
	float GetDischargeInterval( void );

	void Fire( const Vector &vecOrigSrc, const Vector &vecDir );

	BOOL HasAmmo( void );

	void UseAmmo( int count );
	
	enum EGON_FIREMODE { FIRE_NARROW, FIRE_WIDE};

	EHANDLE m_hBeam;
	EHANDLE m_hNoise;
	EHANDLE m_hSprite;
	float m_lastBubble;

	virtual int MergedModelBody() override { return MERGE_MDL_W_EGON; }

	virtual BOOL UseDecrement( void ) override
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

	unsigned short m_usEgonStop;

private:
	EGON_FIREMODE		m_fireMode;
	float				m_shakeTime;
	BOOL				m_deployed;

	unsigned short m_usEgonFire;
};