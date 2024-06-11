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
#include "CSpore.h"
#endif

#include "CSporeLauncher.h"

/*
BEGIN_DATAMAP(CSporeLauncher)
DEFINE_FIELD(m_ReloadState, FIELD_INTEGER),
	END_DATAMAP();
*/

LINK_ENTITY_TO_CLASS(weapon_sporelauncher, CSporeLauncher);

void CSporeLauncher::Spawn()
{
	Precache();
	m_iId = WEAPON_SPORELAUNCHER;
	SET_MODEL(ENT(pev), GetModelW());
	m_iDefaultAmmo = SPORELAUNCHER_DEFAULT_GIVE;

	FallInit();// get ready to fall down.

	pev->sequence = 0;
	pev->animtime = gpGlobals->time;
	pev->framerate = 1;
}

void CSporeLauncher::Precache()
{
	m_defaultModelV = "models/v_spore_launcher.mdl";
	m_defaultModelP = "models/p_spore_launcher.mdl";
	m_defaultModelW = "models/w_spore_launcher.mdl";
	CBasePlayerWeapon::Precache();

	PRECACHE_SOUND("weapons/splauncher_fire.wav");
	PRECACHE_SOUND("weapons/splauncher_altfire.wav");
	PRECACHE_SOUND("weapons/splauncher_bounce.wav");
	PRECACHE_SOUND("weapons/splauncher_reload.wav");
	PRECACHE_SOUND("weapons/splauncher_pet.wav");

	UTIL_PrecacheOther("spore");

	//m_usFireSpore = PRECACHE_EVENT(1, "events/spore.sc");

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_sporelauncher.txt");
}

BOOL CSporeLauncher::Deploy()
{
	return DefaultDeploy("models/v_spore_launcher.mdl", "models/p_spore_launcher.mdl", SPLAUNCHER_DRAW1, "rpg");
}

void CSporeLauncher::Holster(int skiplocal)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_ReloadState = ReloadState::NOT_RELOADING;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	SendWeaponAnim(SPLAUNCHER_HOLSTER1);
}

void CSporeLauncher::WeaponIdle()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 &&
			m_ReloadState == ReloadState::NOT_RELOADING &&
			0 != m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_ReloadState != ReloadState::NOT_RELOADING)
		{
			int maxClip = SPORELAUNCHER_MAX_CLIP;

			/*
			if ((m_pPlayer->m_iItems & CTFItem::Backpack) != 0)
			{
				maxClip *= 2;
			}
			*/

			if (m_iClip != maxClip && 0 != m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else
			{
				// reload debounce has timed out
				SendWeaponAnim(SPLAUNCHER_AIM);

				m_ReloadState = ReloadState::NOT_RELOADING;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.83;
			}
		}
		else
		{
			int iAnim;
			float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);
			if (flRand <= 0.75)
			{
				iAnim = SPLAUNCHER_IDLE;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2;
			}
			else if (flRand <= 0.95)
			{
				iAnim = SPLAUNCHER_IDLE2;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4;
			}
			else
			{
				iAnim = SPLAUNCHER_FIDGET;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 4;

				EMIT_SOUND(m_pPlayer->edict(),CHAN_ITEM, "weapons/splauncher_pet.wav", 0.7, ATTN_NORM);
			}

			SendWeaponAnim(iAnim);
		}
	}
}

void CSporeLauncher::PrimaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (0 != m_iClip)
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		SendWeaponAnim(SPLAUNCHER_FIRE);
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/splauncher_fire.wav", VOL_NORM, ATTN_NORM);

		Vector vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		UTIL_MakeVectors(vecAngles);

		const Vector vecSrc =
			m_pPlayer->GetGunPosition() +
			gpGlobals->v_forward * 16 +
			gpGlobals->v_right * 8 +
			gpGlobals->v_up * -8;

		//vecAngles = vecAngles + m_pPlayer->GetAutoaimVectorFromPoint(vecSrc, AUTOAIM_10DEGREES);
		vecAngles = vecAngles + m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		CSpore* pSpore = CSpore::CreateSpore(
			vecSrc, vecAngles,
			m_pPlayer,
			CSpore::SporeType::ROCKET, false, false);

		UTIL_MakeVectors(vecAngles);

		pSpore->pev->velocity = pSpore->pev->velocity + DotProduct(pSpore->pev->velocity, gpGlobals->v_forward) * gpGlobals->v_forward;
#endif

		int flags;

#if defined(CLIENT_WEAPONS)
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usFireSpore);

		m_iClip -= 1;
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;

	m_ReloadState = ReloadState::NOT_RELOADING;
}

void CSporeLauncher::SecondaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (0 != m_iClip)
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		SendWeaponAnim(SPLAUNCHER_FIRE);
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/splauncher_fire.wav", VOL_NORM, ATTN_NORM);

		Vector vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		UTIL_MakeVectors(vecAngles);

		const Vector vecSrc =
			m_pPlayer->GetGunPosition() +
			gpGlobals->v_forward * 16 +
			gpGlobals->v_right * 8 +
			gpGlobals->v_up * -8;

		//vecAngles = vecAngles + m_pPlayer->GetAutoaimVectorFromPoint(vecSrc, AUTOAIM_10DEGREES);
		vecAngles = vecAngles + m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		CSpore* pSpore = CSpore::CreateSpore(
			vecSrc, vecAngles,
			m_pPlayer,
			CSpore::SporeType::GRENADE, false, false);

		UTIL_MakeVectors(vecAngles);

		pSpore->pev->velocity = m_pPlayer->pev->velocity + 800 * gpGlobals->v_forward;
#endif

		int flags;

#if defined(CLIENT_WEAPONS)
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usFireSpore);

		m_iClip -= 1;
	}

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;

	m_ReloadState = ReloadState::NOT_RELOADING;
}

void CSporeLauncher::Reload()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int maxClip = SPORELAUNCHER_MAX_CLIP;
	
	/*
	if ((m_pPlayer->m_iItems & CTFItem::Backpack) != 0)
	{
		maxClip *= 2;
	}
	*/

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == maxClip)
		return;

	// don't reload until recoil is done
	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	// check to see if we're ready to reload
	if (m_ReloadState == ReloadState::NOT_RELOADING)
	{
		SendWeaponAnim(SPLAUNCHER_RELOAD_REACH);
		m_ReloadState = ReloadState::DO_RELOAD_EFFECTS;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.66;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.66;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.66;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.66;
		return;
	}
	else if (m_ReloadState == ReloadState::DO_RELOAD_EFFECTS)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;
		// was waiting for gun to move to side
		m_ReloadState = ReloadState::RELOAD_ONE;

		EMIT_SOUND(m_pPlayer->edict(),CHAN_ITEM, "weapons/splauncher_reload.wav", 0.7, ATTN_NORM);

		SendWeaponAnim(SPLAUNCHER_RELOAD);

		m_flNextReload = UTIL_WeaponTimeBase() + 1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
	else
	{
		// Add them to the clip
		m_iClip += 1;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;
		m_ReloadState = ReloadState::DO_RELOAD_EFFECTS;
	}
}

int CSporeLauncher::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_sporelauncher";

	p->pszAmmo1 = "spores";
	p->iMaxAmmo1 = SPORE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SPORELAUNCHER_MAX_CLIP;
	p->iSlot = 4; // should be 6 but that slot  doesn't exist in vanilla HL
	p->iPosition = 4;
	p->iId = WEAPON_SPORELAUNCHER;
	p->iWeight = SHOCKRIFLE_WEIGHT;
	return 1;
}
