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
#include "skill.h"
#include "weapons.h"
#include "nodes.h"
#include "CBasePlayer.h"
#include "env/CSoundEnt.h"
#include "gamerules.h"
#include "weapon/CMP5.h"
#include "weapon/CGrenade.h"

enum mp5_e
{
	MP5_LONGIDLE = 0,
	MP5_IDLE1,
	MP5_LAUNCH,
	MP5_RELOAD,
	MP5_DEPLOY,
	MP5_FIRE1,
	MP5_FIRE2,
	MP5_FIRE3,
};


LINK_ENTITY_TO_CLASS( weapon_mp5, CMP5 )
LINK_ENTITY_TO_CLASS( weapon_9mmAR, CMP5 )
LINK_ENTITY_TO_CLASS( weapon_9mmar, CMP5 )

// TODO: implement these weapons instead of remapping them
LINK_ENTITY_TO_CLASS(weapon_uzi, CMP5)
LINK_ENTITY_TO_CLASS(weapon_uziakimbo, CMP5)
LINK_ENTITY_TO_CLASS(weapon_m16, CMP5)
LINK_ENTITY_TO_CLASS(weapon_m249, CMP5)
LINK_ENTITY_TO_CLASS(weapon_saw, CMP5)
LINK_ENTITY_TO_CLASS(weapon_minigun, CMP5)

//=========================================================
//=========================================================
int CMP5::SecondaryAmmoIndex( void )
{
	return m_iSecondaryAmmoType;
}

void CMP5::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_9mmAR"); // hack to allow for old names
	Precache( );
	SetWeaponModelW();
	m_iId = WEAPON_MP5;

	m_iDefaultAmmo = MP5_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}


void CMP5::Precache( void )
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_9mmAR.mdl";
	m_defaultModelP = "models/p_9mmAR.mdl";
	m_defaultModelW = "models/w_9mmAR.mdl";
	CBasePlayerWeapon::Precache();

	m_iShell = PRECACHE_MODEL ("models/shell.mdl");// brass shellTE_MODEL

	PRECACHE_MODEL("models/grenade.mdl");	// grenade
	PRECACHE_SOUND("items/9mmclip1.wav");              

	PRECACHE_SOUND("items/clipinsert1.wav");
	PRECACHE_SOUND("items/cliprelease1.wav");
	PRECACHE_SOUND("weapons/9mmar_reload.wav"); // combined reload sound for less network usage and code

	PRECACHE_SOUND ("weapons/hks1.wav");// H to the K
	PRECACHE_SOUND ("weapons/hks2.wav");// H to the K
	PRECACHE_SOUND ("weapons/hks3.wav");// H to the K

	PRECACHE_SOUND( "weapons/glauncher.wav" );
	PRECACHE_SOUND( "weapons/glauncher2.wav" );

	PRECACHE_SOUND ("weapons/357_cock1.wav");

	UTIL_PrecacheOther("ammo_9mmAR");
	UTIL_PrecacheOther("ammo_ARgrenades");

	PrecacheEvents();
}

void CMP5::PrecacheEvents() {
	m_usMP5 = PRECACHE_EVENT(1, "events/mp5.sc");
	m_usMP52 = PRECACHE_EVENT(1, "events/mp52.sc");
}

int CMP5::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = gSkillData.sk_ammo_max_9mm;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = gSkillData.sk_ammo_max_argrenades;
	p->iMaxClip = MP5_MAX_CLIP;
	p->iSlot = 2;
	p->iPosition = 0;
	p->iFlags = 0;
	p->iId = m_iId = WEAPON_MP5;
	p->iWeight = MP5_WEIGHT;

	return 1;
}

BOOL CMP5::Deploy( )
{
	return DefaultDeploy(GetModelV(), GetModelP(), MP5_DEPLOY, "mp5" );
}


void CMP5::PrimaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_iClip <= 0)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_iClip--;


	m_pPlayer->pev->effects = (int)(m_pPlayer->pev->effects) | EF_MUZZLEFLASH;

	// player "shoot" animation
	m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

	Vector vecSrc	 = m_pPlayer->GetGunPosition( );
	Vector vecAiming = m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );
	Vector vecDir;

	lagcomp_begin(m_pPlayer);

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{
		// optimized multiplayer. Widened to make it easier to hit a moving player
		vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_6DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}
	else
	{
		// single player spread
		vecDir = m_pPlayer->FireBulletsPlayer( 1, vecSrc, vecAiming, VECTOR_CONE_3DEGREES, 8192, BULLET_PLAYER_MP5, 2, 0, m_pPlayer->pev, m_pPlayer->random_seed );
	}

	lagcomp_end();

  int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usMP5, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, 0, 0, 0, 0 );

	PLAY_DISTANT_SOUND(m_pPlayer->edict(), DISTANT_9MM);

	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);

	m_flNextPrimaryAttack = GetNextAttackDelay(0.1);

	if ( m_flNextPrimaryAttack < UTIL_WeaponTimeBase() )
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
}



void CMP5::SecondaryAttack( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	// don't fire underwater
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound( );
		m_flNextPrimaryAttack = 0.15;
		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
	{
		PlayEmptySound( );
		return;
	}

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;
	m_pPlayer->m_flStopExtraSoundTime = UTIL_WeaponTimeBase() + 0.2;
			
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	strcpy_safe(m_pPlayer->m_szAnimExtention, "gauss", 32); // not enough recoil in the mp5 animation
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
	strcpy_safe(m_pPlayer->m_szAnimExtention, "mp5", 32);

 	UTIL_MakeVectors( m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle );

	// we don't add in player velocity anymore.
	CGrenade::ShootContact( m_pPlayer->pev, 
							m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16, 
							gpGlobals->v_forward * 800 );

	int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usMP52 );
	
	m_flNextPrimaryAttack = GetNextAttackDelay(1);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5;// idle pretty soon after shooting.

	if (!m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		// HEV suit - indicate out of ammo condition
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
}

void CMP5::Reload( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if ( m_pPlayer->ammo_9mm <= 0 )
		return;

	if (DefaultReload(MP5_MAX_CLIP, MP5_RELOAD, 1.5)) {
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD, 1.5f);
			
		// send reload sound to everyone except the reloader if they're in first-person mode,
		// because the reloading client will play sounds via model events
		uint32_t messageTargets = 0xffffffff;
		if (m_pPlayer->IsFirstPerson()) {
			messageTargets &= ~PLRBIT(m_pPlayer->edict());
		}
		StartSound(m_pPlayer->edict(), CHAN_ITEM, "weapons/9mmar_reload.wav", 0.8f,
			ATTN_NORM, SND_FL_PREDICTED, 93 + RANDOM_LONG(0, 15), m_pPlayer->pev->origin, messageTargets);
#endif
	}
}


void CMP5::WeaponIdle( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	ResetEmptySound( );

	m_pPlayer->GetAutoaimVector( AUTOAIM_5DEGREES );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	int iAnim;
	switch ( RANDOM_LONG( 0, 1 ) )
	{
	case 0:	
		iAnim = MP5_LONGIDLE;	
		break;
	
	default:
	case 1:
		iAnim = MP5_IDLE1;
		break;
	}

	SendWeaponAnim( iAnim );

	m_flTimeWeaponIdle = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 ); // how long till we do this again.
}
