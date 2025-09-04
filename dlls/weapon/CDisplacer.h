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

#define DISPLACER_DEFAULT_GIVE 40

enum DisplacerAnim
{
	DISPLACER_IDLE1 = 0,
	DISPLACER_IDLE2,
	DISPLACER_SPINUP,
	DISPLACER_SPIN,
	DISPLACER_FIRE,
	DISPLACER_DRAW,
	DISPLACER_HOLSTER1
};

enum class DisplacerMode
{
	STARTED = 0,
	SPINNING_UP,
	SPINNING,
	FIRED
};

static const size_t DISPLACER_NUM_BEAMS = 4;

class CDisplacer : public CBasePlayerWeapon
{
public:
	void Spawn() override;

	void Precache() override;

	void KeyValue(KeyValueData* pkvd) override;
	CBaseEntity* Respawn() override;

	BOOL Deploy(void) override;

	void Holster(int skiplocal = 0) override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void SecondaryAttack() override;

	void Reload() override;

	void SpinupThink();

	void AltSpinupThink();

	void FireThink();

	void AltFireThink();

	void ToggleChargeBeams(bool enabled);

	void ItemPostFrame() override;

	int GetItemInfo(ItemInfo* p) override;

	virtual int MergedModelBody() { return MERGE_MDL_W_DISPLACER; }

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
	int m_iSpriteTexture;

	float m_flStartTime;
	float m_flSoundDelay;

	DisplacerMode m_Mode;

	int m_iImplodeCounter;
	int m_iSoundState;

	unsigned short m_usFireDisplacer;

	float m_lastAttack;
	EHANDLE h_beams[2];

	string_t m_iszTeleportDestination;
};
