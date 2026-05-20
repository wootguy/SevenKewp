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
	virtual void ItemPostFrame(void) override;
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount);
	virtual int AddToPlayer(CBasePlayer* pPlayer);

	virtual int MergedModelBody() { return MERGE_MDL_W_PMEDKIT; }

	BOOL IsClientWeapon() { return FALSE; }

	bool CanHealTarget(CBaseEntity* ent);

	void RechargeAmmo();

	void CancelRevive();

	float m_reviveChargedTime; // time when target will be revive charge will complete
	float m_rechargeTime; // time until regenerating ammo
	float m_nextMessageTime; // next time a status message can be sent
	float m_nextSpriteHint;
	EHANDLE h_reviveTarget;
	int m_reviveSpriteIdx;

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};
