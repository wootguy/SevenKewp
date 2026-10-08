#pragma once
#include "CBasePlayerWeapon.h"

class CMedkit : public CBasePlayerWeapon
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	int iItemSlot( void ) override { return 1; }
	int GetItemInfo(ItemInfo *p) override;

	void PrimaryAttack( void ) override;
	void SecondaryAttack( void ) override;
	BOOL Deploy( void ) override;
	void Holster( int skiplocal = 0 ) override;
	void WeaponIdle() override;
	virtual void ItemPostFrame(void) override;
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) override;
	virtual int AddToPlayer(CBasePlayer* pPlayer) override;

	virtual int MergedModelBody() override { return MERGE_MDL_W_PMEDKIT; }

	BOOL IsClientWeapon() override { return FALSE; }

	bool CanHealTarget(CBaseEntity* ent);

	void RechargeAmmo();

	void CancelRevive();

	float m_reviveChargedTime; // time when target will be revive charge will complete
	float m_rechargeTime; // time until regenerating ammo
	float m_nextMessageTime; // next time a status message can be sent
	float m_nextSpriteHint;
	EHANDLE h_reviveTarget;
	int m_reviveSpriteIdx;

	virtual BOOL UseDecrement( void ) override
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};
