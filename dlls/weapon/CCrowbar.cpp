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
#include "gamerules.h"
#include "weapon/CCrowbar.h"

#define	CROWBAR_BODYHIT_VOLUME 128
#define	CROWBAR_WALLHIT_VOLUME 512

LINK_ENTITY_TO_CLASS( weapon_crowbar, CCrowbar )


enum crowbar_e {
	CROWBAR_IDLE = 0,
	CROWBAR_DRAW,
	CROWBAR_HOLSTER,
	CROWBAR_ATTACK1HIT,
	CROWBAR_ATTACK1MISS,
	CROWBAR_ATTACK2MISS,
	CROWBAR_ATTACK2HIT,
	CROWBAR_ATTACK3MISS,
	CROWBAR_ATTACK3HIT
};


void CCrowbar::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_crowbar"); // hack to allow for alternate names

	Precache( );
	m_iId = WEAPON_CROWBAR;
	SetWeaponModelW();
	m_iClip = -1;

	FallInit();// get ready to fall down.
}


void CCrowbar::Precache( void )
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_crowbar.mdl";
	m_defaultModelP = "models/p_crowbar.mdl";
	m_defaultModelW = "models/w_crowbar.mdl";
	CBasePlayerWeapon::Precache();

	PRECACHE_SOUND("weapons/cbar_hit1.wav");
	PRECACHE_SOUND("weapons/cbar_hit2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
	PRECACHE_SOUND("weapons/cbar_hitbod3.wav");
	PRECACHE_SOUND("weapons/cbar_miss1.wav");

	PrecacheEvents();
}

void CCrowbar::PrecacheEvents() {
	m_usCrowbar = PRECACHE_EVENT(1, "events/crowbar.sc");
}

int CCrowbar::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = WEAPON_CROWBAR;
	p->iWeight = CROWBAR_WEIGHT;
	return 1;
}



BOOL CCrowbar::Deploy( )
{
	return DefaultDeploy(GetModelV(), GetModelP(), CROWBAR_DRAW, "crowbar" );
}

void CCrowbar::Holster( int skiplocal /* = 0 */ )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim( CROWBAR_HOLSTER );
}


void CCrowbar::PrimaryAttack()
{
	if (! Swing( 1 ))
	{
		SetThink( &CCrowbar::SwingAgain );
		pev->nextthink = gpGlobals->time + 0.1;
	}
}


void CCrowbar::Smack( )
{
	DecalGunshot( &m_trHit, BULLET_PLAYER_CROWBAR );
}


void CCrowbar::SwingAgain( void )
{
	Swing( 0 );
}


int CCrowbar::Swing( int fFirst )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	int fDidHit = FALSE;

	TraceResult tr;

	UTIL_MakeVectors (m_pPlayer->pev->v_angle);
	Vector vecSrc	= m_pPlayer->GetGunPosition( );
	Vector vecEnd	= vecSrc + gpGlobals->v_forward * 32;

#ifndef CLIENT_DLL
	SolidifyNearbyCorpses(false);

	lagcomp_begin(m_pPlayer);
#endif

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
	if ( tr.flFraction >= 1.0 )
	{
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT( m_pPlayer->pev ), &tr );
		if ( tr.flFraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
			if ( !pHit || pHit->IsBSPModel() )
				FindHullIntersection( vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict() );
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}

	lagcomp_end();

	SolidifyNearbyCorpses(true);
#endif

	// disabled client prediction to stop doubling sounds/animations
	if (fFirst) {
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usCrowbar,
			0.0, (float*)&g_vecZero, (float*)&g_vecZero, 0, 0, 0,
			0.0, 0, 0.0);
	}

	if ( tr.flFraction >= 1.0 )
	{
		if (fFirst)
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
			
			// player "shoot" animation
			m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		}
	}
	else
	{
		switch( ((m_iSwing++) % 2) + 1 )
		{
		case 0:
			SendWeaponAnim( CROWBAR_ATTACK1HIT ); break;
		case 1:
			SendWeaponAnim( CROWBAR_ATTACK2HIT ); break;
		case 2:
			SendWeaponAnim( CROWBAR_ATTACK3HIT ); break;
		}

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );
		
#ifndef CLIENT_DLL

		// hit
		fDidHit = TRUE;
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		ClearMultiDamage( );

		// JoshA: Changed from < -> <= to fix the full swing logic since client weapon prediction.
		// -1.0f + 1.0f = 0.0f. UTIL_WeaponTimeBase is always 0 with client weapon prediction (0 time base vs curtime base)
		if ( (m_flNextPrimaryAttack + 1 <= UTIL_WeaponTimeBase() ) || g_pGameRules->IsMultiplayer() )
		{
			// first swing does full damage
			pEntity->TraceAttack(m_pPlayer->pev, GetDamage(gSkillData.sk_plr_crowbar), gpGlobals->v_forward, &tr, DMG_CLUB );
		}
		else
		{
			// subsequent swings do half
			pEntity->TraceAttack(m_pPlayer->pev, GetDamage(gSkillData.sk_plr_crowbar) / 2, gpGlobals->v_forward, &tr, DMG_CLUB );
		}	
		ApplyMultiDamage( m_pPlayer->pev, m_pPlayer->pev );

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		int fHitWorld = TRUE;

		if (pEntity)
		{
			bool suitableClass = pEntity->Classify() != CLASS_NONE || pEntity->IsPlayerCorpse();
			if (suitableClass && !pEntity->IsMachine() && !pEntity->IsBSPModel() )
			{
				// play thwack or smack sound
				switch( RANDOM_LONG(0,2) )
				{
				case 0:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM); break;
				case 1:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM); break;
				case 2:
					EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM); break;
				}
				m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
				if ( !pEntity->IsAlive() )
					  return TRUE; // rapid crowbar bug :D
				else
					  flVol = 0.1;

				fHitWorld = FALSE;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (fHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd-vecSrc)*2, BULLET_PLAYER_CROWBAR);

			if ( g_pGameRules->IsMultiplayer() )
			{
				// override the volume here, cause we don't play texture sounds in multiplayer, 
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play crowbar strike
			switch( RANDOM_LONG(0,1) )
			{
			case 0:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			case 1:
				EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0,3)); 
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
#endif
		m_flNextPrimaryAttack = GetNextAttackDelay(0.25);
		
		SetThink( &CCrowbar::Smack );
		pev->nextthink = gpGlobals->time + 0.2;

		
	}
	return fDidHit;
}
