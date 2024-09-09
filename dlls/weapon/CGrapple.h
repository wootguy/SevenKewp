/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   This source code contains proprietary and confidential information of
 *   Valve LLC and its suppliers.  Access to this code is restricted to
 *   persons who have executed a written SDK license with Valve.  Any access,
 *   use or distribution of this code by or to any unlicensed person is illegal.
 *
 ****/

#pragma once

#include "vector.h"
#include "CBasePlayerWeapon.h"

class CGrappleTip;

enum BarnacleGrappleAnim
{
	BGRAPPLE_BREATHE = 0,
	BGRAPPLE_LONGIDLE,
	BGRAPPLE_SHORTIDLE,
	BGRAPPLE_COUGH,
	BGRAPPLE_DOWN,
	BGRAPPLE_UP,
	BGRAPPLE_FIRE,
	BGRAPPLE_FIREWAITING,
	BGRAPPLE_FIREREACHED,
	BGRAPPLE_FIRETRAVEL,
	BGRAPPLE_FIRERELEASE
};

class CGrapple : public CBasePlayerWeapon
{
private:
	enum class FireState
	{
		OFF = 0,
		CHARGE = 1
	};

public:
	void Spawn();
	void Precache(void);
	BOOL Deploy();
	void Holster(int skiplocal = 0);
	void WeaponIdle(void);
	void PrimaryAttack();
	void SecondaryAttack();
	int GetItemInfo(ItemInfo* p);
	BOOL IsClientWeapon() { return FALSE; }

	virtual int MergedModelBody() { return MERGE_MDL_W_BGRAP; }

	BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void Fire(const Vector& vecOrigin, const Vector& vecDir);
	void EndAttack();

	void CreateEffect();
	void UpdateEffect();
	void DestroyEffect();

private:
	EHANDLE m_hTip;
	EHANDLE m_hBeam;

	float m_flShootTime;
	float m_flDamageTime;

	FireState m_FireState;

	bool m_bGrappling = false;

	bool m_bMissed;

	bool m_bMomentaryStuck;
};
