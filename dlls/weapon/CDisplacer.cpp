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
#include "CDisplacerBall.h"
#endif

#include "CDisplacer.h"

/*
BEGIN_DATAMAP(CDisplacer)
DEFINE_FUNCTION(SpinupThink),
DEFINE_FUNCTION(AltSpinupThink),
DEFINE_FUNCTION(FireThink),
DEFINE_FUNCTION(AltFireThink),
END_DATAMAP();
*/

LINK_ENTITY_TO_CLASS(weapon_displacer, CDisplacer);

void CDisplacer::Spawn()
{
	Precache();
	m_iId = WEAPON_DISPLACER;
	SET_MODEL(ENT(pev), GetModelW());
	m_iClip = -1;
	m_iDefaultAmmo = DISPLACER_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

void CDisplacer::Precache()
{
	m_defaultModelV = "models/v_displacer.mdl";
	m_defaultModelP = "models/p_displacer.mdl";
	m_defaultModelW = "models/w_displacer.mdl";
	CBasePlayerWeapon::Precache();

	PRECACHE_SOUND("weapons/displacer_fire.wav");
	PRECACHE_SOUND("weapons/displacer_self.wav");
	PRECACHE_SOUND("weapons/displacer_spin.wav");
	PRECACHE_SOUND("weapons/displacer_spin2.wav");
	PRECACHE_SOUND("buttons/button11.wav");

	m_iSpriteTexture = PRECACHE_MODEL("sprites/shockwave.spr");

	UTIL_PrecacheOther("displacer_ball");

	m_usFireDisplacer = PRECACHE_EVENT(1, "events/displacer.sc");

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_displacer.txt");
}

BOOL CDisplacer::Deploy()
{
	return DefaultDeploy("models/v_displacer.mdl", "models/p_displacer.mdl", DISPLACER_DRAW, "egon");
}

void CDisplacer::Holster(int skiplocal)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_fInReload = false;

	STOP_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/displacer_spin.wav");

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 5.0);

	SendWeaponAnim(DISPLACER_HOLSTER1);

	if (m_pfnThink == &CDisplacer::SpinupThink)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 20;
		SetThink(nullptr);
	}
	else if (m_pfnThink == &CDisplacer::AltSpinupThink)
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 60;
		SetThink(nullptr);
	}
}

void CDisplacer::WeaponIdle()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flSoundDelay != 0 && gpGlobals->time >= m_flSoundDelay)
		m_flSoundDelay = 0;

	if (m_flTimeWeaponIdle <= UTIL_WeaponTimeBase())
	{
		const float flNextIdle = RANDOM_FLOAT(0, 1);

		int iAnim;

		if (flNextIdle <= 0.5)
		{
			iAnim = DISPLACER_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}
		else
		{
			iAnim = DISPLACER_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}

		SendWeaponAnim(iAnim);
	}
}

void CDisplacer::PrimaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 20)
	{
		SetThink(&CDisplacer::SpinupThink);

		pev->nextthink = gpGlobals->time;

		m_Mode = DisplacerMode::STARTED;

		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/displacer_spin.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);

		m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.5;
	}
	else
	{
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "buttons/button11.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CDisplacer::SecondaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= 60)
	{
		SetThink(&CDisplacer::AltSpinupThink);

		pev->nextthink = gpGlobals->time;

		m_Mode = DisplacerMode::STARTED;

		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/displacer_spin2.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);

		m_flTimeWeaponIdle = m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5;
	}
	else
	{
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "buttons/button11.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CDisplacer::Reload()
{
	// Nothing
}

void CDisplacer::SpinupThink()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_Mode == DisplacerMode::STARTED)
	{
		SendWeaponAnim(DISPLACER_SPINUP);

		m_Mode = DisplacerMode::SPINNING_UP;

		int flags;

		// #if defined( CLIENT_WEAPONS )
		//		flags = FEV_NOTHOST;
		// #else
		flags = 0;
		// #endif

		//PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireDisplacer, 0, g_vecZero, g_vecZero,
		//	0, 0, static_cast<int>(m_Mode), 0, 0, 0);

		m_flStartTime = gpGlobals->time;
		m_iSoundState = 0;
	}

	if (m_Mode <= DisplacerMode::SPINNING_UP)
	{
		if (gpGlobals->time > m_flStartTime + 0.9)
		{
			m_Mode = DisplacerMode::SPINNING;

			SetThink(&CDisplacer::FireThink);

			pev->nextthink = gpGlobals->time + 0.1;
		}

		m_iImplodeCounter = static_cast<int>((gpGlobals->time - m_flStartTime) * 100.0 + 50.0);
	}

	if (m_iImplodeCounter > 250)
		m_iImplodeCounter = 250;

	m_iSoundState = 128;

	pev->nextthink = gpGlobals->time + 0.1;
}

void CDisplacer::AltSpinupThink()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_Mode == DisplacerMode::STARTED)
	{
		SendWeaponAnim(DISPLACER_SPINUP);

		m_Mode = DisplacerMode::SPINNING_UP;

		int flags;

		// #if defined( CLIENT_WEAPONS )
		//		flags = FEV_NOTHOST;
		// #else
		flags = 0;
		// #endif

		//PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireDisplacer, 0, g_vecZero, g_vecZero,
		//	0, 0, static_cast<int>(m_Mode), 0, 0, 0);

		m_flStartTime = gpGlobals->time;
		m_iSoundState = 0;
	}

	if (m_Mode <= DisplacerMode::SPINNING_UP)
	{
		if (gpGlobals->time > m_flStartTime + 0.9)
		{
			m_Mode = DisplacerMode::SPINNING;

			SetThink(&CDisplacer::AltFireThink);

			pev->nextthink = gpGlobals->time + 0.1;
		}

		m_iImplodeCounter = static_cast<int>((gpGlobals->time - m_flStartTime) * 100.0 + 50.0);
	}

	if (m_iImplodeCounter > 250)
		m_iImplodeCounter = 250;

	m_iSoundState = 128;

	pev->nextthink = gpGlobals->time + 0.1;
}

void CDisplacer::FireThink()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 20;

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	SendWeaponAnim(DISPLACER_FIRE);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/displacer_fire.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);

	int flags;

#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	/*
	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usFireDisplacer, 0,
		g_vecZero, g_vecZero, 0, 0, static_cast<int>( DisplacerMode::FIRED ), 0, 0, 0 );
		*/

#ifndef CLIENT_DLL
	const Vector vecAnglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

	UTIL_MakeVectors(vecAnglesAim);

	Vector vecSrc = m_pPlayer->GetGunPosition();

	// Update auto-aim
	//m_pPlayer->GetAutoaimVectorFromPoint(vecSrc, AUTOAIM_10DEGREES);
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	CDisplacerBall::CreateDisplacerBall(vecSrc, vecAnglesAim, m_pPlayer);

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0)
	{
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", TRUE, SUIT_REPEAT_OK);
	}
#endif

	SetThink(nullptr);
}

void CDisplacer::AltFireThink()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	/*
#ifndef CLIENT_DLL
	if (m_pPlayer->IsOnRope())
	{
		m_pPlayer->pev->movetype = MOVETYPE_WALK;
		m_pPlayer->pev->solid = SOLID_SLIDEBOX;
		m_pPlayer->SetOnRopeState(false);
		m_pPlayer->GetRope()->DetachObject();
		m_pPlayer->SetRope(nullptr);
	}
#endif
*/

	STOP_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/displacer_spin.wav");

	m_pPlayer->m_DisplacerReturn = m_pPlayer->pev->origin;
	m_pPlayer->m_DisplacerSndRoomtype = m_pPlayer->m_flSndRoomtype;

/*
#ifndef CLIENT_DLL
	if (g_pGameRules->IsCTF() && m_pPlayer->m_pFlag)
	{
		CTFGoalFlag* pFlag = m_pPlayer->m_pFlag;

		pFlag->DropFlag(m_pPlayer);
	}
#endif
*/

#ifndef CLIENT_DLL
	CBaseEntity* pDestination = NULL;

	if (!g_pGameRules->IsMultiplayer() || g_pGameRules->IsCoOp())
	{
		pDestination = UTIL_FindEntityByClassname(nullptr, "info_displacer_xen_target");
	}
	else
	{
		pDestination = CBaseEntity::Instance(EntSelectSpawnPoint(m_pPlayer));

		//if (!pDestination)
		//	pDestination = g_pLastSpawn;

		if (pDestination) {
			Vector vecEnd = pDestination->pev->origin;
			vecEnd.z -= 100;

			TraceResult tr;
			UTIL_TraceLine(pDestination->pev->origin, vecEnd, ignore_monsters, edict(), &tr);
			UTIL_SetOrigin(m_pPlayer->pev, tr.vecEndPos + Vector(0, 0, 37));
		}
	}

	if (!FNullEnt(pDestination))
	{
		m_pPlayer->pev->flags &= ~FL_SKIPLOCALHOST;

		Vector vecNewOrigin = pDestination->pev->origin;

		if (!g_pGameRules->IsMultiplayer() || g_pGameRules->IsCoOp())
		{
			vecNewOrigin.z += 37;
		}

		UTIL_SetOrigin(m_pPlayer->pev, vecNewOrigin);

		m_pPlayer->pev->angles = pDestination->pev->angles;
		m_pPlayer->pev->v_angle = pDestination->pev->angles;
		m_pPlayer->pev->fixangle = TRUE;

		m_pPlayer->pev->basevelocity = g_vecZero;
		m_pPlayer->pev->velocity = g_vecZero;
#endif

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2;

		SetThink(nullptr);

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// Must always be handled on the server side in order to play the right sounds and effects.
		//int flags = 0;

		//PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireDisplacer, 0, g_vecZero, g_vecZero,
		//	0, 0, static_cast<int>(DisplacerMode::FIRED), 0, 1, 0);

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 60;

		CDisplacerBall::CreateDisplacerBall(m_pPlayer->m_DisplacerReturn, Vector(90, 0, 0), m_pPlayer);

		//if (0 == GetMagazine1())
		{
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
				m_pPlayer->SetSuitUpdate("!HEV_AMO0", TRUE, SUIT_REPEAT_OK);
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase();

		if (!g_pGameRules->IsMultiplayer())
			m_pPlayer->pev->gravity = 0.6;
	}
	else
	{
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "buttons/button11.wav", RANDOM_FLOAT(0.8, 0.9), ATTN_NORM);
	}
#endif
}

int CDisplacer::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "uranium";
	p->iMaxAmmo1 = URANIUM_MAX_CARRY;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 5;
	p->iPosition = 1;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_DISPLACER;
	p->iWeight = DISPLACER_WEIGHT;

	return 1;
}
