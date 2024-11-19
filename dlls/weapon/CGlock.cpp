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

#include "extdll.h"
#include "util.h"
#include "weapon/weapons.h"
#include "nodes.h"
#include "CBasePlayer.h"
#include "weapon/CGlock.h"
#include "skill.h"

enum glock_e {
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

LINK_ENTITY_TO_CLASS( weapon_glock, CGlock )
LINK_ENTITY_TO_CLASS( weapon_9mmhandgun, CGlock )


void CGlock::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_9mmhandgun"); // hack to allow for old names
	Precache( );
	m_iId = WEAPON_GLOCK;
	SetWeaponModelW();

	m_iDefaultAmmo = GLOCK_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CGlock::Precache( void )
{
	m_defaultModelV = "models/v_9mmhandgun.mdl";
	m_defaultModelP = "models/p_9mmhandgun.mdl";
	m_defaultModelW = "models/w_9mmhandgun.mdl";
	CBasePlayerWeapon::Precache();

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shell

	PRECACHE_SOUND("items/9mmclip1.wav");
	PRECACHE_SOUND("items/9mmclip2.wav");
	PRECACHE_SOUND("weapons/glock_reload.wav");

	PRECACHE_SOUND ("weapons/pl_gun1.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pl_gun2.wav");//silenced handgun
	PRECACHE_SOUND ("weapons/pl_gun3.wav");//handgun

	UTIL_PrecacheOther("ammo_9mm");

	PrecacheEvents();
}

void CGlock::PrecacheEvents() {
	m_usFireGlock1 = PRECACHE_EVENT(1, "events/glock1.sc");
	m_usFireGlock2 = PRECACHE_EVENT(1, "events/glock2.sc");
}

int CGlock::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = gSkillData.sk_ammo_max_9mm;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GLOCK_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_GLOCK;
	p->iWeight = GLOCK_WEIGHT;

	return 1;
}

BOOL CGlock::Deploy( )
{
	// pev->body = 1;
	return DefaultDeploy(GetModelV(), GetModelP(), GLOCK_DRAW, "onehanded", /*UseDecrement() ? 1 : 0*/ 0 );
}

void CGlock::SecondaryAttack( void )
{
	GlockFire( 0.1, 0.2, FALSE );
}

void CGlock::PrimaryAttack( void )
{
	GlockFire( 0.01, 0.3, TRUE );
}

void CGlock::GlockFire( float flSpread , float flCycleTime, BOOL fUseAutoAim )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = GetNextAttackDelay(0.2);
		}

		return;
	}

	m_iClip--;

	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	int flags;

#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	// silenced
	if (pev->body == 1)
	{
		m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
	}
	else
	{
		// non-silenced
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	}

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming;
	
	if ( fUseAutoAim )
	{
		vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );
	}
	else
	{
		vecAiming = gpGlobals->v_forward;
	}

	lagcomp_begin(m_pPlayer);
	Vector vecDir;
	vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, Vector( flSpread, flSpread, flSpread ), 8192, BULLET_PLAYER_9MM, 0, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	lagcomp_end();

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), fUseAutoAim ? m_usFireGlock1 : m_usFireGlock2, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, ( m_iClip == 0 ) ? 1 : 0, 0 );

	PLAY_DISTANT_SOUND(m_pPlayer->edict(), DISTANT_9MM);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}


void CGlock::Reload( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if ( m_pPlayer->ammo_9mm <= 0 )
		 return;


	if (DefaultReload(17, m_iClip ? GLOCK_RELOAD_NOT_EMPTY : GLOCK_RELOAD, 1.5)) {
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD, 1.5f);

		// send reload sound to everyone except the reloader if they're in first-person mode,
		// because the reloading client will play sounds via model events
		uint32_t messageTargets = 0xffffffff;
		if (m_pPlayer->IsFirstPerson()) {
			messageTargets &= ~PLRBIT(m_pPlayer->edict());
		}
		StartSound(m_pPlayer->edict(), CHAN_ITEM, "weapons/glock_reload.wav", 0.8f,
			ATTN_NORM, SND_FL_PREDICTED, 93 + RANDOM_LONG(0, 15), m_pPlayer->pev->origin, messageTargets);

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
#endif
	}
}



void CGlock::WeaponIdle( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_10DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	// only idle if the slid isn't back
	if (m_iClip != 0)
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0.0, 1.0 );

		if (flRand <= 0.3 + 0 * 0.75)
		{
			iAnim = GLOCK_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 49.0 / 16;
		}
		else if (flRand <= 0.6 + 0 * 0.875)
		{
			iAnim = GLOCK_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0 / 16.0;
		}
		else
		{
			iAnim = GLOCK_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 40.0 / 16.0;
		}
		SendWeaponAnim( iAnim, 1 );
	}
}

void CGlock::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	ammoEntName = "ammo_9mm";
	dropAmount = 17;
}