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
#include "CM249.h"

//LINK_ENTITY_TO_CLASS(weapon_m249, CM249)
//LINK_ENTITY_TO_CLASS(weapon_saw, CM249)

void CM249::Spawn()
{
	pev->classname = MAKE_STRING("weapon_m249"); // hack to allow for old names
	Precache();
	SetWeaponModelW();
	m_iId = WEAPON_M249;
	m_iDefaultAmmo = M249_DEFAULT_GIVE;
	FallInit();
}

void CM249::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_saw.mdl";
	m_defaultModelP = "models/p_saw.mdl";
	m_defaultModelW = "models/w_saw.mdl";
	CBasePlayerWeapon::Precache();

	m_iShell = PRECACHE_MODEL("models/saw_shell.mdl");
	m_iLink = PRECACHE_MODEL("models/saw_link.mdl");
	m_iSmoke = PRECACHE_MODEL("sprites/wep_smoke_01.spr");
	m_iFire = PRECACHE_MODEL("sprites/xfire.spr");

	PRECACHE_SOUND("weapons/saw_reload.wav");
	PRECACHE_SOUND("weapons/saw_reload2.wav");
	PRECACHE_SOUND("weapons/saw_fire1.wav");

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_m249.txt");
}

BOOL CM249::Deploy()
{
	return DefaultDeploy("models/v_saw.mdl", "models/p_saw.mdl", M249_DRAW, "mp5");
}

void CM249::Holster(int skiplocal)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	SetThink(nullptr);

	SendWeaponAnim(M249_HOLSTER);

	m_bReloading = false;

	m_fInReload = false;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 10.0, 15.0);

	CBasePlayerWeapon::Holster();
}

void CM249::WeaponIdle()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	ResetEmptySound();

	// Update auto-aim
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_bReloading && gpGlobals->time >= m_flReloadStart + 1.33)
	{
		m_bReloading = false;

		pev->body = 0;

		SendWeaponAnim(M249_RELOAD_END);
	}

	if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
	{
		const float flNextIdle = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

		int iAnim;

		if (flNextIdle <= 0.95)
		{
			iAnim = M249_SLOWIDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.0;
		}
		else
		{
			iAnim = M249_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 6.16;
		}

		SendWeaponAnim(iAnim);
	}
}

#include "event_args.h"

#define	DEFAULT_VIEWHEIGHT	28

void EV_GetDefaultShellInfo(CBasePlayer* plr, event_args_t* args, float* origin, float* velocity,
	float* ShellVelocity, float* ShellOrigin, float* forward, float* right, float* up,
	float forwardScale, float upScale, float rightScale)
{
	int i;
	Vector view_ofs;
	float fR, fU;

	int idx;

	idx = args->entindex;

	/*
	view_ofs[2] = DEFAULT_VIEWHEIGHT;

	if (EV_IsPlayer(idx))
	{
		if (EV_IsLocal(idx))
		{
			gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);
		}
		else if (args->ducking == 1)
		{
			view_ofs[2] = VEC_DUCK_VIEW;
		}
	}
	*/
	view_ofs = plr->pev->view_ofs;

	fR = RANDOM_FLOAT(50, 70);
	fU = RANDOM_FLOAT(100, 150);

	for (i = 0; i < 3; i++)
	{
		ShellVelocity[i] = velocity[i] + right[i] * fR + up[i] * fU + forward[i] * 25;
		ShellOrigin[i] = origin[i] + view_ofs[i] + up[i] * upScale + forward[i] * forwardScale + right[i] * rightScale;
	}
}

void CM249::EV_FireM249(event_args_t* args) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int iBody = args->iparam1;

	const bool bAlternatingEject = args->bparam1 != 0;

	Vector up, right, forward;

	UTIL_MakeVectorsPrivate(args->angles, forward, right, up);

	int iShell = bAlternatingEject ? m_iLink : m_iShell;

	SetBodygroup(0, iBody);
	//EV_MuzzleFlash();
	SendWeaponAnim(RANDOM_LONG(0, 2) + M249_SHOOT1, 1, iBody);
	m_pPlayer->pev->punchangle.x = RANDOM_FLOAT(-2, 2);
	m_pPlayer->pev->punchangle.y = RANDOM_FLOAT(-1, 1);

	Vector ShellVelocity;
	Vector ShellOrigin;

	EV_GetDefaultShellInfo(
		m_pPlayer,
		args,
		args->origin, args->velocity,
		ShellVelocity,
		ShellOrigin,
		forward, right, up,
		14.0, -12.0, 4.0);

	EjectBrass(ShellOrigin, ShellVelocity, args->angles[1], iShell, TE_BOUNCE_SHELL);

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/saw_fire1.wav",
		VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));

	/*
	Vector vecSrc;

	m_pPlayer->GetGunPosition(args, vecSrc, args->origin);

	Vector vecAiming = forward;

	EV_HLDM_FireBullets(
		args->entindex,
		forward, right, up,
		1,
		vecSrc, vecAiming,
		8192.0,
		BULLET_PLAYER_556,
		0, nullptr,
		args->fparam1, args->fparam2);
	*/
}

void CM249::PrimaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD)
	{
		PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		if (!m_fInReload)
		{
			PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		}

		return;
	}

	m_iClip -= 1;

	pev->body = RecalculateBody(m_iClip);

	m_bAlternatingEject = !m_bAlternatingEject;

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	m_flNextAnimTime = UTIL_WeaponTimeBase() + 0.2;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();

	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	Vector vecSpread;

	bool wideSpread = true;
	//if (g_Skill.GetValue("m249_wide_spread") != 0)
	if (wideSpread)
	{
		if ((m_pPlayer->pev->button & IN_DUCK) != 0)
		{
			vecSpread = VECTOR_CONE_3DEGREES;
		}
		else if ((m_pPlayer->pev->button & (IN_MOVERIGHT | IN_MOVELEFT | IN_FORWARD | IN_BACK)) != 0)
		{
			vecSpread = VECTOR_CONE_15DEGREES;
		}
		else
		{
			vecSpread = VECTOR_CONE_6DEGREES;
		}
	}
	else
	{
		if ((m_pPlayer->pev->button & IN_DUCK) != 0)
		{
			vecSpread = VECTOR_CONE_2DEGREES;
		}
		else if ((m_pPlayer->pev->button & (IN_MOVERIGHT | IN_MOVELEFT | IN_FORWARD | IN_BACK)) != 0)
		{
			vecSpread = VECTOR_CONE_10DEGREES;
		}
		else
		{
			vecSpread = VECTOR_CONE_4DEGREES;
		}
	}

	Vector vecDir = m_pPlayer->FireBulletsPlayer(
		1,
		vecSrc, vecAiming, vecSpread,
		8192.0, BULLET_PLAYER_556, 2, 0,
		m_pPlayer->pev, m_pPlayer->random_seed);

	int flags;
#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	/*
	PLAYBACK_EVENT_FULL(
		flags, m_pPlayer->edict(), m_usFireM249, 0,
		g_vecZero, g_vecZero,
		vecDir.x, vecDir.y,
		pev->body, 0,
		m_bAlternatingEject ? 1 : 0, 0);
	*/
	event_args_t args;
	memset(&args, 0, sizeof(event_args_t));
	args.flags = flags;
	args.entindex = m_pPlayer->entindex();
	args.origin[0] = m_pPlayer->pev->origin.x;
	args.origin[1] = m_pPlayer->pev->origin.y;
	args.origin[2] = m_pPlayer->pev->origin.z;
	args.angles[0] = m_pPlayer->pev->angles.x;
	args.angles[1] = m_pPlayer->pev->angles.y;
	args.angles[2] = m_pPlayer->pev->angles.z;
	args.fparam1 = vecDir.x;
	args.fparam2 = vecDir.y;
	args.iparam1 = pev->body;
	args.bparam1 = m_bAlternatingEject ? 1 : 0;

	EV_FireM249(&args);

	if (m_iClip == 0)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", SUIT_REPEAT_OK, 0);
		}
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.067;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2;

#ifndef CLIENT_DLL
	m_pPlayer->pev->punchangle.x = RANDOM_FLOAT(-2, 2);

	m_pPlayer->pev->punchangle.y = RANDOM_FLOAT(-1, 1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	const Vector& vecVelocity = m_pPlayer->pev->velocity;

	const float flZVel = m_pPlayer->pev->velocity.z;

	Vector vecInvPushDir = gpGlobals->v_forward * 35.0;

	float flNewZVel = CVAR_GET_FLOAT("sv_maxspeed");

	if (vecInvPushDir.z >= 10.0)
		flNewZVel = vecInvPushDir.z;

	if (!g_pGameRules->IsMultiplayer())
	{
		m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - vecInvPushDir;

		// Restore Z velocity to make deathmatch easier.
		m_pPlayer->pev->velocity.z = flZVel;
	}
	else
	{
		const float flZTreshold = -(flNewZVel + 100.0);

		if (vecVelocity.x > flZTreshold)
		{
			m_pPlayer->pev->velocity.x -= vecInvPushDir.x;
		}

		if (vecVelocity.y > flZTreshold)
		{
			m_pPlayer->pev->velocity.y -= vecInvPushDir.y;
		}

		m_pPlayer->pev->velocity.z -= vecInvPushDir.z;
	}
#endif
}

void CM249::Reload()
{
	if (DefaultReload(M249_MAX_CLIP, M249_RELOAD_START, 1.0))
	{
		m_bReloading = true;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.78;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.78;

		m_flReloadStart = gpGlobals->time;
	}
}

int CM249::RecalculateBody(int iClip)
{
	if (iClip == 0)
	{
		return 8;
	}
	else if (iClip >= 0 && iClip <= 7)
	{
		return 9 - iClip;
	}
	else
	{
		return 0;
	}
}

int CM249::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_m249";

	p->pszAmmo1 = "556";
	p->iMaxClip = M249_MAX_CLIP;
	p->iMaxAmmo1 = 200;
	p->iSlot = 2;
	p->iPosition = 4;
	p->iId = WEAPON_M249;
	p->iWeight = M249_WEIGHT;

	return true;
}
