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
#include "cbase.h"
#include "user_messages.h"

#ifndef CLIENT_DLL
#include "CShockBeam.h"
#endif

#include "CShockRifle.h"

/*
BEGIN_DATAMAP(CShockRifle)
// This isn't restored in the original
DEFINE_FIELD(m_flRechargeTime, FIELD_TIME),
	END_DATAMAP();
*/

LINK_ENTITY_TO_CLASS(weapon_shockrifle, CShockRifle);

void CShockRifle::Spawn()
{
	Precache();
	m_iId = WEAPON_SHOCKRIFLE;
	SET_MODEL(ENT(pev), GetModelW());
	m_iClip = WEAPON_NOCLIP;
	m_iDefaultAmmo = SHOCKRIFLE_DEFAULT_GIVE;

	FallInit();// get ready to fall down.

	pev->sequence = 0;
	pev->animtime = gpGlobals->time;
	pev->framerate = 1;
}

void CShockRifle::Precache()
{
	m_defaultModelV = "models/v_shock.mdl";
	m_defaultModelP = "models/p_shock.mdl";
	m_defaultModelW = "models/w_shock.mdl";
	CBasePlayerWeapon::Precache();

	m_iSpriteTexture = PRECACHE_MODEL("sprites/shockwave.spr");
	PRECACHE_MODEL("sprites/lgtning.spr");

	PRECACHE_SOUND("weapons/shock_fire.wav");
	PRECACHE_SOUND("weapons/shock_draw.wav");
	PRECACHE_SOUND("weapons/shock_recharge.wav");
	PRECACHE_SOUND("weapons/shock_discharge.wav");

	//m_usShockRifle = PRECACHE_EVENT(1, "events/shock.sc");

	UTIL_PrecacheOther("shock_beam");

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_pipewrench.txt");
}

/*
void CShockRifle::AttachToPlayer(CBasePlayer* pPlayer)
{
	if (0 == m_iDefaultAmmo)
		m_iDefaultAmmo = 1;

	BaseClass::AttachToPlayer(pPlayer);
}

bool CShockRifle::CanDeploy()
{
	return true;
}

*/

BOOL CShockRifle::Deploy()
{
	/*
	if (g_Skill.GetValue("shockrifle_fast") != 0)
	{
		m_flRechargeTime = gpGlobals->time + 0.25;
	}
	else
	{
		m_flRechargeTime = gpGlobals->time + 0.5;
	}
	*/

	m_flRechargeTime = gpGlobals->time + 0.5;

	return DefaultDeploy("models/v_shock.mdl", "models/p_shock.mdl", SHOCKRIFLE_DRAW, "bow");
}

void CShockRifle::Holster(int skiplocal)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_fInReload = false;

	SetThink(nullptr);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;

	SendWeaponAnim(SHOCKRIFLE_HOLSTER);

	if (0 == m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 1;
	}
}

void CShockRifle::WeaponIdle()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	Reload();

	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flSoundDelay != 0 && gpGlobals->time >= m_flSoundDelay)
	{
		m_flSoundDelay = 0;
	}

	// This used to be completely broken. It used the current game time instead of the weapon time base, which froze the idle animation.
	// It also never handled IDLE3, so it only ever played IDLE1, and then only animated it when you held down secondary fire.
	// This is now fixed.
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;

	const float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);

	if (flRand <= 0.75)
	{
		iAnim = SHOCKRIFLE_IDLE3;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 51.0 / 15.0;
	}
	else
	{
		iAnim = SHOCKRIFLE_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 101.0 / 30.0;
	}

	SendWeaponAnim(iAnim);
}

void CShockRifle::PrimaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD)
	{
		// Water goes zap.
		const float flVolume = RANDOM_FLOAT(0.8, 0.9);

		EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/shock_discharge.wav", flVolume, ATTN_NONE);

		const int ammoCount = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

		RadiusDamage(pev->origin, m_pPlayer->pev, m_pPlayer->pev, ammoCount * 100, ammoCount * 150, CLASS_NONE, DMG_ALWAYSGIB | DMG_BLAST);

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;

		return;
	}

	Reload();

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;

	m_flRechargeTime = gpGlobals->time + 1.0;

	int flags;

#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usShockRifle);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

#ifndef CLIENT_DLL
	const Vector vecAnglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

	UTIL_MakeVectors(vecAnglesAim);

	const auto vecSrc =
		m_pPlayer->GetGunPosition() +
		gpGlobals->v_forward * 16 +
		gpGlobals->v_right * 9 +
		gpGlobals->v_up * -7;

	// Update auto-aim
	//m_pPlayer->GetAutoaimVectorFromPoint(vecSrc, AUTOAIM_10DEGREES);
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	auto pBeam = CShockBeam::CreateShockBeam(vecSrc, vecAnglesAim, m_pPlayer);

	UTIL_SetOrigin(pBeam->m_hBeam1->pev, pBeam->pev->origin);

	if (!g_pGameRules->IsMultiplayer())
	{
		UTIL_SetOrigin(pBeam->m_hBeam2->pev, pBeam->pev->origin);
	}
#endif

	/*
	if (g_Skill.GetValue("shockrifle_fast") != 0)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	}
	else
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
	}
	*/

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33;
}

void CShockRifle::SecondaryAttack()
{
	// Nothing
}

void CShockRifle::Reload()
{
	RechargeAmmo(true);
}

void CShockRifle::ItemPostFrame()
{
	CBasePlayerWeapon::ItemPostFrame();

	Reload();
}

void CShockRifle::RechargeAmmo(bool bLoud)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int ammoCount = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

	while (ammoCount < SHOCKRIFLE_DEFAULT_GIVE && m_flRechargeTime < gpGlobals->time)
	{
		++ammoCount;

		if (bLoud)
		{
			EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/shock_recharge.wav", VOL_NORM, ATTN_NORM);
		}

		/*
		if (g_Skill.GetValue("shockrifle_fast") != 0)
		{
			m_flRechargeTime += 0.25;
		}
		else
		{
			m_flRechargeTime += 0.5;
		}
		*/

		m_flRechargeTime += 0.5;
	}

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = ammoCount;
}

int CShockRifle::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "shock";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 5; // should be 6 but that slot isn't bound by default in vanilla HL
	p->iPosition = 1;
	p->iId = WEAPON_SHOCKRIFLE;
	p->iWeight = SHOCKRIFLE_WEIGHT;
	p->iFlags = ITEM_FLAG_NOAUTORELOAD | ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	return 1;
}