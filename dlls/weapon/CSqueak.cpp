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
#if !defined( OEM_BUILD ) && !defined( HLDEMO_BUILD )

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monster/monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "CBasePlayer.h"
#include "env/CSoundEnt.h"
#include "gamerules.h"
#include "weapon/CSqueak.h"
#include "weapon/CGrenade.h"

enum squeak_e {
	SQUEAK_IDLE1 = 0,
	SQUEAK_FIDGETFIT,
	SQUEAK_FIDGETNIP,
	SQUEAK_DOWN,
	SQUEAK_UP,
	SQUEAK_THROW
};

LINK_ENTITY_TO_CLASS( weapon_snark, CSqueak );


void CSqueak::Spawn( )
{
	Precache( );
	m_iId = WEAPON_SNARK;
	SET_MODEL(ENT(pev), GetModelW());

	FallInit();//get ready to fall down.

	m_iDefaultAmmo = SNARK_DEFAULT_GIVE;
		
	pev->sequence = 1;
	pev->animtime = gpGlobals->time;
	pev->framerate = 1.0;
}


void CSqueak::Precache( void )
{
	m_defaultModelV = "models/v_squeak.mdl";
	m_defaultModelP = "models/p_squeak.mdl";
	m_defaultModelW = "models/w_sqknest.mdl";
	CBasePlayerWeapon::Precache();

	PRECACHE_SOUND("squeek/sqk_hunt2.wav");
	PRECACHE_SOUND("squeek/sqk_hunt3.wav");
	UTIL_PrecacheOther("monster_snark");

	PrecacheEvents();
}

void CSqueak::PrecacheEvents() {
	m_usSnarkFire = PRECACHE_EVENT(1, "events/snarkfire.sc");
}


int CSqueak::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Snarks";
	p->iMaxAmmo1 = gSkillData.sk_ammo_max_snarks;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_SNARK;
	p->iWeight = SNARK_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}



BOOL CSqueak::Deploy( )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	// play hunt sound
	float flRndSound = RANDOM_FLOAT ( 0 , 1 );

	if ( flRndSound <= 0.5 )
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt2.wav", 1, ATTN_NORM, 0, 100);
	else 
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt3.wav", 1, ATTN_NORM, 0, 100);

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	return DefaultDeploy(GetModelV(), GetModelP(), SQUEAK_UP, "squeak" );
}


void CSqueak::Holster( int skiplocal /* = 0 */ )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	if ( !m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
	{
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_SNARK);
		SetThink( &CSqueak::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
		return;
	}
	
	SendWeaponAnim( SQUEAK_DOWN );
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);
}


void CSqueak::PrimaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if ( m_pPlayer->m_rgAmmo[ m_iPrimaryAmmoType ] )
	{
		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		TraceResult tr;
		Vector trace_origin;

		// HACK HACK:  Ugly hacks to handle change in origin based on new physics code for players
		// Move origin up if crouched and start trace a bit outside of body ( 20 units instead of 16 )
		trace_origin = m_pPlayer->pev->origin;
		if ( m_pPlayer->pev->flags & FL_DUCKING )
		{
			trace_origin = trace_origin - ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
		}

		// find place to toss monster
		UTIL_TraceLine( trace_origin + gpGlobals->v_forward * 20, trace_origin + gpGlobals->v_forward * 64, dont_ignore_monsters, NULL, &tr );

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	    PLAYBACK_EVENT_FULL( flags, m_pPlayer->edict(), m_usSnarkFire, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );

		SendWeaponAnim(SQUEAK_THROW, 0, 0);

		if ( tr.fAllSolid == 0 && tr.fStartSolid == 0 && tr.flFraction > 0.25 )
		{
			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

#ifndef CLIENT_DLL
			CBaseEntity *pSqueak = CBaseEntity::Create( "monster_snark", tr.vecEndPos, m_pPlayer->pev->v_angle, m_pPlayer->edict() );
			pSqueak->pev->velocity = gpGlobals->v_forward * 200 + m_pPlayer->pev->velocity;
#endif

			// play hunt sound
			float flRndSound = RANDOM_FLOAT ( 0 , 1 );

			if ( flRndSound <= 0.5 )
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt2.wav", 1, ATTN_NORM, 0, 105);
			else 
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "squeek/sqk_hunt3.wav", 1, ATTN_NORM, 0, 105);

			m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

			m_fJustThrown = 1;

			m_flNextPrimaryAttack = GetNextAttackDelay(0.3);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
		}
	}
}


void CSqueak::SecondaryAttack( void )
{

}


void CSqueak::WeaponIdle( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if (m_fJustThrown)
	{
		m_fJustThrown = 0;

		if ( !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] )
		{
			RetireWeapon();
			return;
		}

		SendWeaponAnim( SQUEAK_UP );
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
		return;
	}

	int iAnim;
	float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
	if (flRand <= 0.75)
	{
		iAnim = SQUEAK_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 30.0 / 16 * (2);
	}
	else if (flRand <= 0.875)
	{
		iAnim = SQUEAK_FIDGETFIT;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70.0 / 16.0;
	}
	else
	{
		iAnim = SQUEAK_FIDGETNIP;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 80.0 / 16.0;
	}
	SendWeaponAnim( iAnim );
}

#endif
