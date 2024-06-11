/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#pragma once

enum ShockRifleAnim
{
	SHOCKRIFLE_IDLE1 = 0,
	SHOCKRIFLE_FIRE,
	SHOCKRIFLE_DRAW,
	SHOCKRIFLE_HOLSTER,
	SHOCKRIFLE_IDLE3
};

#define SHOCKRIFLE_DEFAULT_GIVE 10

class CShockRifle : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;

	//void AttachToPlayer(CBasePlayer* pPlayer) override;

	//bool CanDeploy() override;

	BOOL Deploy() override;

	void Holster(int skiplocal = 0) override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void SecondaryAttack() override;

	void Reload() override;

	void ItemPostFrame() override;

	void ToggleChargeBeams(bool enabled);

	int AddToPlayer(CBasePlayer* pPlayer);

	int GetItemInfo(ItemInfo* p) override;

	BOOL UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

	BOOL IsClientWeapon() { return FALSE; }

	void UpdateOnRemove(void) override;

private:
	void RechargeAmmo(bool bLoud);

private:
	int m_iSpriteTexture;

	unsigned short m_usShockRifle;

	float m_flRechargeTime;
	float m_flSoundDelay;
	float m_lastAttack;

	EHANDLE h_beams[3];
};
