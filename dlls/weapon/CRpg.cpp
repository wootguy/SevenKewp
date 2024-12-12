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
#if !defined( OEM_BUILD )

#include "extdll.h"
#include "util.h"
#include "skill.h"
#include "weapons.h"
#include "nodes.h"
#include "CBasePlayer.h"
#include "gamerules.h"
#include "weapon/CRpg.h"

enum rpg_e {
	RPG_IDLE = 0,
	RPG_FIDGET,
	RPG_RELOAD,		// to reload
	RPG_FIRE2,		// to empty
	RPG_HOLSTER1,	// loaded
	RPG_DRAW1,		// loaded
	RPG_HOLSTER2,	// unloaded
	RPG_DRAW_UL,	// unloaded
	RPG_IDLE_UL,	// unloaded idle
	RPG_FIDGET_UL,	// unloaded fidget
};

LINK_ENTITY_TO_CLASS( weapon_rpg, CRpg )

int g_laserBeamIdx;

#ifndef CLIENT_DLL

LINK_ENTITY_TO_CLASS( laser_spot, CLaserSpot )

TYPEDESCRIPTION	CRpg::m_SaveData[] =
{
	DEFINE_FIELD(CRpg, m_fSpotActive, FIELD_INTEGER),
	DEFINE_FIELD(CRpg, m_cActiveRockets, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CRpg, CBasePlayerWeapon)

TYPEDESCRIPTION	CRpgRocket::m_SaveData[] =
{
	DEFINE_FIELD(CRpgRocket, m_flIgniteTime, FIELD_TIME),
	DEFINE_FIELD(CRpgRocket, m_hLauncher, FIELD_EHANDLE),
};
IMPLEMENT_SAVERESTORE(CRpgRocket, CGrenade)

//=========================================================
//=========================================================
CLaserSpot *CLaserSpot::CreateSpot(edict_t* owner)
{
	CLaserSpot *pSpot = GetClassPtr( (CLaserSpot *)NULL );
	pSpot->Spawn();

	pSpot->pev->classname = MAKE_STRING("laser_spot");
	pSpot->pev->owner = owner;

	return pSpot;
}

//=========================================================
//=========================================================
void CLaserSpot::Spawn( void )
{
	Precache( );
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;

	pev->rendermode = kRenderGlow;
	pev->renderfx = kRenderFxNoDissipation;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), "sprites/laserdot.spr");
	UTIL_SetOrigin( pev, pev->origin );
}

//=========================================================
// Suspend- make the laser sight invisible. 
//=========================================================
void CLaserSpot::Suspend( float flSuspendTime )
{
	pev->effects |= EF_NODRAW;
	
	SetThink( &CLaserSpot::Revive );
	pev->nextthink = gpGlobals->time + flSuspendTime;
}

//=========================================================
// Revive - bring a suspended laser sight back.
//=========================================================
void CLaserSpot::Revive( void )
{
	pev->effects &= ~EF_NODRAW;

	SetThink( NULL );
}

void CLaserSpot::MonsterAimThink() {
	CBaseEntity* ent = Instance(pev->owner);
	CBaseMonster* owner = ent ? ent->MyMonsterPointer() : NULL;

	if (!owner) {
		return;
	}
	
	Vector vecShootDir = g_vecZero;
	Vector attachOrigin, attachAngles;
	if (owner->GetAttachmentCount() > 0)
		owner->GetAttachment(1, attachOrigin, attachAngles);

	if (!owner->m_hEnemy || owner->HasConditions(bits_COND_ENEMY_OCCLUDED)) {
		vecShootDir = owner->m_vecEnemyLKP - attachOrigin;
	}
	else {
		vecShootDir = owner->m_hEnemy->Center() - attachOrigin;
	}

	TraceResult tr;
	UTIL_TraceLine(attachOrigin, attachOrigin + vecShootDir * 8192, dont_ignore_monsters, edict(), &tr);

	UTIL_SetOrigin(pev, tr.vecEndPos);

	pev->nextthink = gpGlobals->time + 0.01f;
}

void CLaserSpot::ActivateMonsterControl() {
	SetThink(&CLaserSpot::MonsterAimThink);
	MonsterAimThink();
}

void CLaserSpot::Precache( void )
{
	PRECACHE_MODEL("sprites/laserdot.spr");
}

LINK_ENTITY_TO_CLASS( rpg_rocket, CRpgRocket )

//=========================================================
//=========================================================
CRpgRocket *CRpgRocket::CreateRpgRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher )
{
	CRpgRocket *pRocket = GetClassPtr( (CRpgRocket *)NULL );

	UTIL_SetOrigin( pRocket->pev, vecOrigin );
	pRocket->pev->angles = vecAngles;
	pRocket->pev->owner = pOwner->edict();
	pRocket->Spawn();
	pRocket->SetTouch( &CRpgRocket::RocketTouch );
	pRocket->m_hLauncher = pLauncher;// remember what RPG fired me. 

	if (pLauncher)
		pLauncher->m_cActiveRockets++;// register this missile as active for the launcher

	return pRocket;
}

//=========================================================
//=========================================================
void CRpgRocket :: Spawn( void )
{
	Precache( );
	// motor
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	SetGrenadeModel();
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin( pev, pev->origin );

	pev->classname = MAKE_STRING("rpg_rocket");

	SetThink( &CRpgRocket::IgniteThink );
	SetTouch( &CRpgRocket::ExplodeTouch );

	pev->angles.x -= 30;
	UTIL_MakeVectors( pev->angles );
	pev->angles.x = -(pev->angles.x + 30);

	pev->velocity = gpGlobals->v_forward * 250;
	pev->gravity = 0.5;

	pev->nextthink = gpGlobals->time + 0.4;

	float dmg_mult = GetDamageModifier();

	pev->dmg = gSkillData.sk_plr_rpg * dmg_mult;
}

//=========================================================
//=========================================================
void CRpgRocket :: RocketTouch ( CBaseEntity *pOther )
{
	CRpg* m_pLauncher = (CRpg*)m_hLauncher.GetEntity();
	if ( m_pLauncher )
	{
		// my launcher is still around, tell it I'm dead.
		m_pLauncher->m_cActiveRockets--;
		m_hLauncher = NULL;
	}

	STOP_SOUND( edict(), CHAN_VOICE, "weapons/rocket1.wav" );
	ExplodeTouch( pOther );
}

//=========================================================
void CRpgRocket::Explode(TraceResult* pTrace, int bitsDamageType)
{
	//ALERT( at_console, "RpgRocket Explode, m_pLauncher: %u\n", GetLauncher() );
	STOP_SOUND(edict(), CHAN_VOICE, "weapons/rocket1.wav");

	CRpg* m_pLauncher = (CRpg*)m_hLauncher.GetEntity();

	if (m_pLauncher)
	{
		// my launcher is still around, tell it I'm dead.
		m_pLauncher->m_cActiveRockets--;
		m_hLauncher = NULL;
	}

	CGrenade::Explode(pTrace, bitsDamageType);

	// stay visible for another think so the interpolated beam effect has time to catch up
	pev->velocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 1;
}

//=========================================================
//=========================================================
void CRpgRocket :: Precache( void )
{
	m_defaultModel = "models/rpgrocket.mdl";
	PRECACHE_MODEL(GetModel());
	m_iTrail = PRECACHE_MODEL("sprites/smoke.spr");
	PRECACHE_SOUND ("weapons/rocket1.wav");
}


void CRpgRocket :: IgniteThink( void  )
{
	// FLY movetype but with client interpolation
	// not using Parametric interp because rocket trails look weird when following a laser
	pev->movetype = MOVETYPE_BOUNCE;
	pev->gravity = FLT_MIN;
	pev->friction = 1.0f;

	pev->effects |= EF_LIGHT;

	// make rocket sound
	EMIT_SOUND( ENT(pev), CHAN_VOICE, "weapons/rocket1.wav", 1, 0.5 );

	// rocket trail
	UTIL_BeamFollow(entindex(), m_iTrail, 40, 5, RGBA(224, 224, 255, 255), MSG_BROADCAST, NULL);	

	m_flIgniteTime = gpGlobals->time;

	// set to follow laser spot
	SetThink( &CRpgRocket::FollowThink );
	pev->nextthink = gpGlobals->time + 0.1;
}


void CRpgRocket :: FollowThink( void  )
{
	CBaseEntity *pOther = NULL;
	Vector vecTarget;
	Vector vecDir;
	float flDist, flMax, flDot;
	TraceResult tr;

	UTIL_MakeAimVectors( pev->angles );

	vecTarget = gpGlobals->v_forward;
	flMax = 4096;

	float bestDot = -1.0f;
	float bestDist = FLT_MAX;
	
	// Examine all entities within a reasonable radius
	while ((pOther = UTIL_FindEntityByClassname( pOther, "laser_spot" )) != NULL)
	{
		if (pOther->pev->effects & EF_NODRAW) {
			continue; // laser isn't removed when a player reloads
		}

		Vector vSpotLocation = pOther->pev->origin;

		if (UTIL_PointContents(vSpotLocation) == CONTENTS_SKY)
		{
			//ALERT( at_console, "laser spot is in the sky...\n");
		}

		UTIL_TraceLine(pev->origin, vSpotLocation, dont_ignore_monsters, ENT(pev), &tr);
		// ALERT( at_console, "%f\n", tr.flFraction );
		if ((tr.vecEndPos - vSpotLocation).Length() < 16)
		{
			vecDir = pOther->pev->origin - pev->origin;
			flDist = vecDir.Length( );
			vecDir = vecDir.Normalize( );
			flDot = DotProduct( gpGlobals->v_forward, vecDir );

			bool isBetter = true;

			if (mp_rpg_laser_mode.value == 1) {
				// the best target is the brightest and most centered
				bool isMoreCentered = flDot > bestDot;
				bool isBrighter = (flDist * 2 < bestDist && fabs(flDot - bestDot) < 0.05f);
				bool isDimmer = (flDist > bestDist * 2 && fabs(flDot - bestDot) < 0.05f);
				isBetter = (isMoreCentered || isBrighter) && !isDimmer;
			}
			else if (mp_rpg_laser_mode.value == 2) {
				isBetter = pev->owner == pOther->pev->owner;
			}

			if ((flDot > 0) && (flDist * (1 - flDot) < flMax) && isBetter)
			{
				bestDot = flDot;
				bestDist = flDist;
				flMax = flDist * (1 - flDot);
				vecTarget = vecDir;
			}
		}
	}

	pev->angles = UTIL_VecToAngles( vecTarget );

	// this acceleration and turning math is totally wrong, but it seems to respond well so don't change it.
	float flSpeed = pev->velocity.Length();
	if (gpGlobals->time - m_flIgniteTime < 1.0)
	{
		pev->velocity = pev->velocity * 0.2 + vecTarget * (flSpeed * 0.8 + 400);
		if (pev->waterlevel == 3)
		{
			// go slow underwater
			if (pev->velocity.Length() > 300)
			{
				pev->velocity = pev->velocity.Normalize() * 300;
			}
			UTIL_BubbleTrail( pev->origin - pev->velocity * 0.1, pev->origin, 4 );
		} 
		else 
		{
			if (pev->velocity.Length() > 2000)
			{
				pev->velocity = pev->velocity.Normalize() * 2000;
			}
		}
	}
	else
	{
		if (pev->effects & EF_LIGHT)
		{
			pev->effects = 0;
			STOP_SOUND( ENT(pev), CHAN_VOICE, "weapons/rocket1.wav" );
		}
		pev->velocity = pev->velocity * 0.2 + vecTarget * flSpeed * 0.798;
		if (pev->waterlevel == 0 && pev->velocity.Length() < 1500)
		{
			Detonate( );
		}
	}
	
	CRpg* m_pLauncher = (CRpg*)m_hLauncher.GetEntity();
	if (m_pLauncher)
	{
		float flDistance = (pev->origin - m_pLauncher->pev->origin).Length();

		// if we've travelled more than max distance the player can send a spot, stop tracking the original launcher (allow it to reload)		
		if (flDistance > 8192.0f || gpGlobals->time - m_flIgniteTime > 6.0f)
		{
			//ALERT( at_console, "RPG too far (%f)!\n", flDistance );
			m_pLauncher->m_cActiveRockets--;
			m_hLauncher = NULL;
		}

		//ALERT( at_console, "%.0f, m_pLauncher: %u, flDistance: %f\n", flSpeed, GetLauncher(), flDistance );
	}

	if ((UTIL_PointContents(pev->origin) == CONTENTS_SKY))
	{
		//ALERT( at_console, "Rocket is in the sky, detonating...\n");
		Detonate();
	}

	pev->nextthink = gpGlobals->time + 0.1;
}
#endif



void CRpg::Reload( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int iResult = 0;

	if ( m_iClip == 1 )
	{
		// don't bother with any of this if don't need to reload.
		return;
	}

	if ( m_pPlayer->ammo_rockets <= 0 )
		return;

	// because the RPG waits to autoreload when no missiles are active while  the LTD is on, the
	// weapons code is constantly calling into this function, but is often denied because 
	// a) missiles are in flight, but the LTD is on
	// or
	// b) player is totally out of ammo and has nothing to switch to, and should be allowed to
	//    shine the designator around
	//
	// Set the next attack time into the future so that WeaponIdle will get called more often
	// than reload, allowing the RPG LTD to be updated
	
	m_flNextPrimaryAttack = GetNextAttackDelay(0.5);

	if ( m_cActiveRockets && m_fSpotActive )
	{
		// no reloading when there are active missiles tracking the designator.
		// ward off future autoreload attempts by setting next attack time into the future for a bit. 
		return;
	}

#ifndef CLIENT_DLL
	if ( m_hSpot && m_fSpotActive )
	{
		CLaserSpot* m_pSpot = (CLaserSpot*)m_hSpot.GetEntity();
		m_pSpot->Suspend( 2.1 );
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.1;
	}
#endif

	if (m_iClip == 0) {
		iResult = DefaultReload(RPG_MAX_CLIP, RPG_RELOAD, 2);
		m_pPlayer->SetAnimation(PLAYER_RELOAD, 2.0f);
	}
	
	if ( iResult )
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + UTIL_SharedRandomFloat( m_pPlayer->random_seed, 10, 15 );
	
}

void CRpg::Spawn( )
{
	Precache( );
	m_iId = WEAPON_RPG;

	SetWeaponModelW();
	m_fSpotActive = 0;

#ifdef CLIENT_DLL
	if ( bIsMultiplayer() )
#else
	if ( g_pGameRules->IsMultiplayer() )
#endif
	{
		// more default ammo in multiplay. 
		m_iDefaultAmmo = RPG_DEFAULT_GIVE * 2;
	}
	else
	{
		m_iDefaultAmmo = RPG_DEFAULT_GIVE;
	}

	FallInit();// get ready to fall down.
}


void CRpg::Precache( void )
{
	PRECACHE_MODEL("models/w_rpg.mdl");
	PRECACHE_MODEL("models/v_rpg.mdl");
	PRECACHE_MODEL("models/p_rpg.mdl");
	m_defaultModelV = "models/v_rpg.mdl";
	m_defaultModelP = "models/p_rpg.mdl";
	m_defaultModelW = "models/w_rpg.mdl";
	CBasePlayerWeapon::Precache();

	g_laserBeamIdx = PRECACHE_MODEL("sprites/laserbeam.spr");

	PRECACHE_SOUND("items/9mmclip1.wav");

	UTIL_PrecacheOther( "laser_spot" );
	UTIL_PrecacheOther( "rpg_rocket" );
	UTIL_PrecacheOther("ammo_rpgclip");

	PRECACHE_SOUND("weapons/rocketfire1.wav");
	PRECACHE_SOUND("weapons/glauncher.wav"); // alternative fire sound

	PrecacheEvents();
}

void CRpg::PrecacheEvents() {
	m_usRpg = PRECACHE_EVENT(1, "events/rpg.sc");
}


int CRpg::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "rockets";
	p->iMaxAmmo1 = gSkillData.sk_ammo_max_rockets;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = RPG_MAX_CLIP;
	p->iSlot = 3;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_RPG;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHTO;
	p->iWeight = RPG_WEIGHT;

	return 1;
}

BOOL CRpg::Deploy( )
{
	int ret = 0;

	if ( m_iClip == 0 )
	{
		ret = DefaultDeploy(GetModelV(), GetModelP(), RPG_DRAW_UL, "rpg" );
	}

	ret = DefaultDeploy(GetModelV(), GetModelP(), RPG_DRAW1, "rpg" );

#ifndef CLIENT_DLL
	SET_MODEL(edict(), GetModelV());
	m_hasLaserAttachment = GetAttachmentCount() > 0;
	SetWeaponModelW();
#endif

	return ret;
}


BOOL CRpg::CanHolster( void )
{
	if ( m_fSpotActive && m_cActiveRockets )
	{
		// can't put away while guiding a missile.
		return FALSE;
	}

	return TRUE;
}

void CRpg::Holster( int skiplocal /* = 0 */ )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_fInReload = FALSE;// cancel any reload in progress.

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	
	SendWeaponAnim( RPG_HOLSTER1 );

#ifndef CLIENT_DLL
	CLaserSpot* m_pSpot = (CLaserSpot*)m_hSpot.GetEntity();
	if (m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NEVER );
		m_hSpot = NULL;
	}
#endif

}


void CRpg::PrimaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if ( m_iClip )
	{
		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

#ifndef CLIENT_DLL
		// player "shoot" animation
		m_pPlayer->SetAnimation( PLAYER_ATTACK1 );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( ) + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8;
		
		CRpgRocket *pRocket = CRpgRocket::CreateRpgRocket( vecSrc, m_pPlayer->pev->v_angle, m_pPlayer, this );

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );// RpgRocket::Create stomps on globals, so remake.
		pRocket->pev->velocity = pRocket->pev->velocity + gpGlobals->v_forward * DotProduct( m_pPlayer->pev->velocity, gpGlobals->v_forward );
#endif

		// firing RPG no longer turns on the designator. ALT fire is a toggle switch for the LTD.
		// Ken signed up for this as a global change (sjb)

		int flags;
#if defined( CLIENT_WEAPONS )
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

		PLAYBACK_EVENT( flags, m_pPlayer->edict(), m_usRpg );

		m_iClip--; 
				
		m_flNextPrimaryAttack = GetNextAttackDelay(1.5);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
		
		ResetEmptySound();
	}
	else
	{
		PlayEmptySound( );
	}
	UpdateSpot( );
}


void CRpg::SecondaryAttack()
{
	m_fSpotActive = ! m_fSpotActive;

#ifndef CLIENT_DLL
	CLaserSpot* m_pSpot = (CLaserSpot*)m_hSpot.GetEntity();
	if (!m_fSpotActive && m_pSpot)
	{
		m_pSpot->Killed( NULL, GIB_NORMAL );
		m_hSpot = NULL;
	}
#endif

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
}


void CRpg::WeaponIdle( void )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	UpdateSpot( );

	if ( m_flTimeWeaponIdle > UTIL_WeaponTimeBase() )
		return;

	if ( m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		float flRand = UTIL_SharedRandomFloat( m_pPlayer->random_seed, 0, 1 );
		if (flRand <= 0.75 || m_fSpotActive)
		{
			if ( m_iClip == 0 )
				iAnim = RPG_IDLE_UL;
			else
				iAnim = RPG_IDLE;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 90.0 / 15.0;
		}
		else
		{
			if ( m_iClip == 0 )
				iAnim = RPG_FIDGET_UL;
			else
				iAnim = RPG_FIDGET;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}

		ResetEmptySound();
		SendWeaponAnim( iAnim );
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1;
	}
}



void CRpg::UpdateSpot( void )
{

#ifndef CLIENT_DLL
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_fSpotActive)
	{
		if (!m_hSpot) {
			m_hSpot = CLaserSpot::CreateSpot(m_pPlayer->edict());
		}
		CLaserSpot* m_pSpot = (CLaserSpot*)m_hSpot.GetEntity();

		UTIL_MakeVectors( m_pPlayer->pev->v_angle );
		Vector vecSrc = m_pPlayer->GetGunPosition( );
		Vector vecAiming = gpGlobals->v_forward;

		TraceResult tr;
		UTIL_TraceLine ( vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr );
		
		if (UTIL_PointContents(tr.vecEndPos) == CONTENTS_SKY) {
			// back up until out of the sky, or else the client won't render the laser beam
			Vector delta = tr.vecEndPos - vecSrc;
			Vector bestPos = tr.vecEndPos;
			for (float f = 0.01f; f <= 1.0f; f += 0.02f) {
				bestPos = tr.vecEndPos - (delta * f);
				if (UTIL_PointContents(bestPos) != CONTENTS_SKY) {
					break;
				}
			}

			m_pSpot->pev->renderamt = 1; // almost invisible, but still rendered so laser beam works
			UTIL_SetOrigin(m_pSpot->pev, bestPos);
		}
		else {
			m_pSpot->pev->renderamt = 255;
			UTIL_SetOrigin(m_pSpot->pev, tr.vecEndPos);
		}

		if (gpGlobals->time - m_lastBeamUpdate >= 0.95f && !(m_pSpot->pev->effects & EF_NODRAW)) {
			// WARNING: Creating a beam entity that uses attachments has caused client crashes before,
			// but I haven't seen that happen yet with TE_BEAMENTS. If this causes crashes again,
			// then revert to using BeamEntPoint (attached to the player, not spot).
			m_lastBeamUpdate = gpGlobals->time;

			if (m_hasLaserAttachment) {
				UTIL_BeamEnts(m_pSpot->entindex(), 0, m_pPlayer->entindex(), 1, false, g_laserBeamIdx,
					0, 0, 10, 8, 0, RGBA(255, 32, 32, 48), 64, MSG_PVS, m_pPlayer->pev->origin);
			}
			else {
				// show the beam to everyone except the player, unless they're in a third-person view
				for (int i = 1; i < gpGlobals->maxClients; i++) {
					CBasePlayer* plr = UTIL_PlayerByIndex(i);
					if (!plr || (plr == m_pPlayer && plr->m_hViewEntity.GetEntity() == plr)) {
						continue;
					}

					edict_t* ed = plr->edict();
					if (m_pPlayer->isVisibleTo(ed) || m_pSpot->isVisibleTo(ed)) {
						UTIL_BeamEnts(m_pSpot->entindex(), 0, m_pPlayer->entindex(), 1, false, g_laserBeamIdx,
							0, 0, 10, 8, 0, RGBA(255, 32, 32, 48), 64, MSG_ONE_UNRELIABLE, NULL, ed);
					}
				}
			}
		}
	}
	else if (m_lastBeamUpdate) {
		UTIL_KillBeam(m_pPlayer->entindex(), MSG_PVS, m_pPlayer->pev->origin);
		m_lastBeamUpdate = 0;
	}
#endif

}

#endif
