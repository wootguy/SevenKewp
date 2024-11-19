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
#include "skill.h"
#include "weapons.h"
#include "nodes.h"
#include "CBasePlayer.h"
#include "gamerules.h"
#include "weapon/CSatchel.h"

enum satchel_e {
	SATCHEL_IDLE1 = 0,
	SATCHEL_FIDGET1,
	SATCHEL_DRAW,
	SATCHEL_DROP
};

enum satchel_radio_e {
	SATCHEL_RADIO_IDLE1 = 0,
	SATCHEL_RADIO_FIDGET1,
	SATCHEL_RADIO_DRAW,
	SATCHEL_RADIO_FIRE,
	SATCHEL_RADIO_HOLSTER
};

#ifndef CLIENT_DLL
TYPEDESCRIPTION	CSatchel::m_SaveData[] =
{
	DEFINE_FIELD(CSatchel, m_chargeReady, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CSatchel, CBasePlayerWeapon)
#endif

LINK_ENTITY_TO_CLASS(weapon_satchel, CSatchel)
LINK_ENTITY_TO_CLASS(monster_satchel, CSatchelCharge)

//=========================================================
// Deactivate - do whatever it is we do to an orphaned 
// satchel when we don't want it in the world anymore.
//=========================================================
void CSatchelCharge::Deactivate( void )
{
	pev->solid = SOLID_NOT;
	UTIL_Remove( this );
}


void CSatchelCharge :: Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SetGrenadeModel();
	//UTIL_SetSize(pev, Vector( -16, -16, -4), Vector(16, 16, 32));	// Old box -- size of headcrab monsters/players get blocked by this
	UTIL_SetSize(pev, Vector( -4, -4, -4), Vector(4, 4, 4));	// Uses point-sized, and can be stepped over
	UTIL_SetOrigin( pev, pev->origin );

	SetTouch( &CSatchelCharge::SatchelSlide );
	SetUse( &CSatchelCharge::DetonateUse );
	SetThink( &CSatchelCharge::SatchelThink );
	pev->nextthink = gpGlobals->time + 0.1;

	pev->gravity = 0.5;
	pev->friction = 0.8;

	pev->dmg = gSkillData.sk_plr_satchel;
	// ResetSequenceInfo( );
	pev->sequence = 1;

	int mergedIdle = LookupSequence("satchel_onback");
	if (mergedIdle != -1)
		pev->sequence = mergedIdle;
}


void CSatchelCharge::SatchelSlide( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// pev->avelocity = Vector (300, 300, 300);
	pev->gravity = 1;// normal gravity now

	// HACKHACK - On ground isn't always set, so look for ground underneath
	TraceResult tr;
	UTIL_TraceLine( pev->origin, pev->origin - Vector(0,0,10), ignore_monsters, edict(), &tr );

	if ( tr.flFraction < 1.0 )
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;
		pev->avelocity = pev->avelocity * 0.9;
		// play sliding sound, volume based on velocity
	}
	if ( !(pev->flags & FL_ONGROUND) && pev->velocity.Length2D() > 10 )
	{
		BounceSound();
	}
	StudioFrameAdvance( );
}


void CSatchelCharge :: SatchelThink( void )
{
	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	if (pev->waterlevel == 3)
	{
		pev->movetype = MOVETYPE_FLY;
		pev->velocity = pev->velocity * 0.8;
		pev->avelocity = pev->avelocity * 0.9;
		pev->velocity.z += 8;
	}
	else if (pev->waterlevel == 0)
	{
		pev->movetype = MOVETYPE_BOUNCE;
	}
	else
	{
		pev->velocity.z -= 8;
	}	
}

void CSatchelCharge :: Precache( void )
{
	m_defaultModel = "models/w_satchel.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_SOUND("weapons/g_bounce1.wav");
	PRECACHE_SOUND("weapons/g_bounce2.wav");
	PRECACHE_SOUND("weapons/g_bounce3.wav");
}

void CSatchelCharge :: BounceSound( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce1.wav", 1, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce2.wav", 1, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/g_bounce3.wav", 1, ATTN_NORM);	break;
	}
}


//=========================================================
// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
//=========================================================
int CSatchel::AddDuplicate( CBasePlayerItem *pOriginal )
{
	CSatchel *pSatchel = NULL;

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{
		pSatchel = (CSatchel *)pOriginal;
		CBasePlayer* originalPlayer = pOriginal->GetPlayer();

		if (originalPlayer == NULL)
			return TRUE;

		int nSatchelsInPocket = originalPlayer->m_rgAmmo[pSatchel->PrimaryAmmoIndex()];
		int nNumSatchels = 0;
		CBaseEntity* pLiveSatchel = NULL;

		while ((pLiveSatchel = UTIL_FindEntityInSphere(pLiveSatchel, originalPlayer->pev->origin, 4096)) != NULL)
		{
			if (FClassnameIs(pLiveSatchel->pev, "monster_satchel"))
			{
				if (pLiveSatchel->pev->owner == originalPlayer->edict())
				{
					nNumSatchels++;
				}
			}
		}

		if (pSatchel->m_chargeReady != 0 && (nSatchelsInPocket + nNumSatchels) >= gSkillData.sk_ammo_max_satchels)
		{
			// player has some satchels deployed. Refuse to add more.
			return FALSE;
		}
	}

	return CBasePlayerWeapon::AddDuplicate ( pOriginal );
}

//=========================================================
//=========================================================
int CSatchel::AddToPlayer( CBasePlayer *pPlayer )
{
	int bResult = CBasePlayerWeapon::AddToPlayer( pPlayer );

	pPlayer->pev->weapons |= (1<<m_iId);
	m_chargeReady = 0;// this satchel charge weapon now forgets that any satchels are deployed by it.

	if ( bResult )
	{
		return AddWeapon( );
	}
	return FALSE;
}

void CSatchel::Spawn( )
{
	Precache( );
	m_iId = WEAPON_SATCHEL;
	SetWeaponModelW();

	m_iDefaultAmmo = SATCHEL_DEFAULT_GIVE;
		
	FallInit();// get ready to fall down.

	int mergedIdle = LookupSequence("satchel_idle");
	if (mergedIdle != -1)
		pev->sequence = mergedIdle;
}


void CSatchel::Precache( void )
{
	PRECACHE_MODEL("models/v_satchel_radio.mdl");
	PRECACHE_MODEL("models/p_satchel_radio.mdl");

	m_defaultModelV = "models/v_satchel.mdl";
	m_defaultModelP = "models/p_satchel.mdl";
	m_defaultModelW = "models/w_satchel.mdl";
	CBasePlayerWeapon::Precache();

	UTIL_PrecacheOther( "monster_satchel" );
}


int CSatchel::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "Satchel Charge";
	p->iMaxAmmo1 = gSkillData.sk_ammo_max_satchels;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 1;
	p->iFlags = ITEM_FLAG_SELECTONEMPTY | ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;
	p->iId = m_iId = WEAPON_SATCHEL;
	p->iWeight = SATCHEL_WEIGHT;

	return 1;
}

//=========================================================
//=========================================================
BOOL CSatchel::IsUseable( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	if ( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] > 0 ) 
	{
		// player is carrying some satchels
		return TRUE;
	}

	if ( m_chargeReady != 0 )
	{
		// player isn't carrying any satchels, but has some out
		return TRUE;
	}

	return FALSE;
}

BOOL CSatchel::CanDeploy( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	if ( m_pPlayer->m_rgAmmo[ PrimaryAmmoIndex() ] > 0 ) 
	{
		// player is carrying some satchels
		return TRUE;
	}

	if ( m_chargeReady != 0 )
	{
		// player isn't carrying any satchels, but has some out
		return TRUE;
	}

	return FALSE;
}

BOOL CSatchel::Deploy( )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );

	if ( m_chargeReady )
		return DefaultDeploy(GET_MODEL("models/v_satchel_radio.mdl"), GET_MODEL("models/p_satchel_radio.mdl"), SATCHEL_RADIO_DRAW, "hive" );

	return DefaultDeploy(GetModelV(), GetModelP(), SATCHEL_DRAW, "trip" );
}


void CSatchel::Holster( int skiplocal /* = 0 */ )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	if ( m_chargeReady )
	{
		SendWeaponAnim( SATCHEL_RADIO_HOLSTER );
	}
	else
	{
		SendWeaponAnim( SATCHEL_DROP );
	}
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", 1.0, ATTN_NORM);

	if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && !m_chargeReady )
	{
		m_pPlayer->pev->weapons &= ~(1<<WEAPON_SATCHEL);
		SetThink( &CSatchel::DestroyItem );
		pev->nextthink = gpGlobals->time + 0.1;
	}
}



void CSatchel::PrimaryAttack()
{
	// we're reloading, don't allow fire
	if (m_chargeReady != 2)
	{
		Throw();
	}
}


void CSatchel::SecondaryAttack( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_chargeReady == 1)
	{
		SendWeaponAnim(SATCHEL_RADIO_FIRE);

		edict_t* pPlayer = m_pPlayer->edict();

		CBaseEntity* pSatchel = NULL;

		while ((pSatchel = UTIL_FindEntityInSphere(pSatchel, m_pPlayer->pev->origin, 4096)) != NULL)
		{
			if (FClassnameIs(pSatchel->pev, "monster_satchel"))
			{
				if (pSatchel->pev->owner == pPlayer)
				{
					pSatchel->Use(m_pPlayer, m_pPlayer, USE_ON, 0);
					m_chargeReady = 2;
				}
			}
		}

		m_chargeReady = 2;
		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
}


void CSatchel::Throw( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
	{
		Vector vecSrc = m_pPlayer->pev->origin;

		Vector vecThrow = gpGlobals->v_forward * 274 + m_pPlayer->pev->velocity;

#ifndef CLIENT_DLL
		CBaseEntity *pSatchel = Create( "monster_satchel", vecSrc, Vector( 0, 0, 0), m_pPlayer->edict() );
		SET_MODEL(pSatchel->edict(), GetModelW());
		pSatchel->pev->velocity = vecThrow;
		pSatchel->pev->avelocity.y = 400;

		m_pPlayer->pev->viewmodel = MAKE_STRING(GET_MODEL("models/v_satchel_radio.mdl"));
		m_pPlayer->pev->weaponmodel = MAKE_STRING(GET_MODEL("models/p_satchel_radio.mdl"));
#else
		LoadVModel ( "models/v_satchel_radio.mdl", m_pPlayer );
#endif

		SendWeaponAnim( SATCHEL_RADIO_DRAW );

		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		m_chargeReady = 1;
		
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
	}
}


void CSatchel::WeaponIdle( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	switch( m_chargeReady )
	{
	case 0:
		SendWeaponAnim( SATCHEL_FIDGET1 );
		// use tripmine animations
		strcpy_safe( m_pPlayer->m_szAnimExtention, "trip", 32 );
		break;
	case 1:
		SendWeaponAnim( SATCHEL_RADIO_FIDGET1 );
		// use hivehand animations
		strcpy_safe( m_pPlayer->m_szAnimExtention, "hive", 32 );
		break;
	case 2:
		if ( !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] )
		{
			m_chargeReady = 0;
			RetireWeapon();
			return;
		}

#ifndef CLIENT_DLL
		m_pPlayer->pev->viewmodel = MAKE_STRING(GET_MODEL("models/v_satchel.mdl"));
		m_pPlayer->pev->weaponmodel = MAKE_STRING(GET_MODEL("models/p_satchel.mdl"));
#else
		LoadVModel ( "models/v_satchel.mdl", m_pPlayer );
#endif

		SendWeaponAnim( SATCHEL_DRAW );

		// use tripmine animations
		strcpy_safe( m_pPlayer->m_szAnimExtention, "trip", 32 );

		m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_chargeReady = 0;
		break;
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );// how long till we do this again.
}

//=========================================================
// DeactivateSatchels - removes all satchels owned by
// the provided player. Should only be used upon death.
//
// Made this global on purpose.
//=========================================================
void DeactivateSatchels(CBasePlayer* pOwner)
{
	edict_t* pFind;

	pFind = FIND_ENTITY_BY_CLASSNAME(NULL, "monster_satchel");

	while (!FNullEnt(pFind))
	{
		CBaseEntity* pEnt = CBaseEntity::Instance(pFind);
		CSatchelCharge* pSatchel = (CSatchelCharge*)pEnt;

		if (pSatchel)
		{
			if (pSatchel->pev->owner == pOwner->edict())
			{
				pSatchel->Deactivate();
			}
		}

		pFind = FIND_ENTITY_BY_CLASSNAME(pFind, "monster_satchel");
	}
}

#endif
