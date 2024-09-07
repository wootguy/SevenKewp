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
#include "cbase.h"
#include "weapons.h"
#include "customentity.h"
#include "skill.h"
#include "decals.h"
#include "gamerules.h"
#include "effects.h"

#include "CShockBeam.h"

#ifndef CLIENT_DLL
TYPEDESCRIPTION CShockBeam::m_SaveData[] =
{
	DEFINE_FIELD(CShockBeam, m_hBeam1, FIELD_EHANDLE),
	DEFINE_FIELD(CShockBeam, m_hBeam2, FIELD_EHANDLE),
	DEFINE_FIELD(CShockBeam, m_hSprite, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CShockBeam, CShockBeam::BaseClass)
#endif

LINK_ENTITY_TO_CLASS(shock_beam, CShockBeam)

void CShockBeam::Precache()
{
	PRECACHE_MODEL( "sprites/flare3.spr" );
	PRECACHE_MODEL( "sprites/lgtning.spr" );
	PRECACHE_MODEL( "sprites/glow01.spr" );
	PRECACHE_MODEL("models/shock_effect.mdl" );
	PRECACHE_SOUND("weapons/shock_impact.wav" );

	m_waterExplodeSpr = PRECACHE_MODEL("sprites/xspark2.spr");

}

void CShockBeam::Spawn()
{
	Precache();

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_TRIGGER;

	SET_MODEL( edict(), "models/shock_effect.mdl" );

	UTIL_SetOrigin( pev, pev->origin );

	UTIL_SetSize( pev, Vector( -4, -4, -4 ), Vector( 4, 4, 4 ) );

	SetTouch( &CShockBeam::BallTouch );
	SetThink( &CShockBeam::FlyThink );

	CSprite* m_pSprite = CSprite::SpriteCreate( "sprites/flare3.spr", pev->origin, false );
	m_hSprite = m_pSprite;

	m_pSprite->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxDistort );

	m_pSprite->SetScale( 0.35 );

	m_pSprite->SetAttachment( edict(), 0 );

	CBeam* m_pBeam1 = CBeam::BeamCreate( "sprites/lgtning.spr", 60 );
	m_hBeam1 = m_pBeam1;

	m_lastPos = pev->origin;

	if( m_pBeam1 )
	{
		UTIL_SetOrigin( m_pBeam1->pev, pev->origin );

		m_pBeam1->EntsInit( entindex(), entindex() );

		m_pBeam1->SetStartAttachment( 1 );
		m_pBeam1->SetEndAttachment( 2 );

		m_pBeam1->SetColor( 0, 253, 253 );

		m_pBeam1->SetFlags( BEAM_FSHADEOUT );
		m_pBeam1->SetBrightness( 180 );
		m_pBeam1->SetNoise( 0 );

		m_pBeam1->SetScrollRate( 10 );

		if( g_pGameRules->IsMultiplayer() )
		{
			pev->nextthink = gpGlobals->time + 0.01;
			return;
		}

		CBeam* m_pBeam2 = CBeam::BeamCreate( "sprites/lgtning.spr", 20 );
		m_hBeam2 = m_pBeam2;

		if( m_pBeam2 )
		{
			UTIL_SetOrigin( m_pBeam2->pev, pev->origin );

			m_pBeam2->EntsInit( entindex(), entindex() );

			m_pBeam2->SetStartAttachment( 1 );
			m_pBeam2->SetEndAttachment( 2 );

			m_pBeam2->SetColor( 255, 255, 157 );

			m_pBeam2->SetFlags( BEAM_FSHADEOUT );
			m_pBeam2->SetBrightness( 180 );
			m_pBeam2->SetNoise( 30 );

			m_pBeam2->SetScrollRate( 30 );

			pev->nextthink = gpGlobals->time + 0.01;
		}
	}
}

void CShockBeam::FlyThink()
{
	if (pev->waterlevel == 3)
	{
		SetThink(&CShockBeam::WaterExplodeThink);
	}

	/*
	// SOLID_TRIGGER entities don't touch stationary monsters
	// so a trace is needed to check for collisions
	TraceResult tr;
	TRACE_MONSTER_HULL(edict(), m_lastPos, pev->origin, 0, pev->owner, &tr);
	if (!FNullEnt(tr.pHit) && tr.pHit != pev->owner) {
		BallTouch(CBaseEntity::Instance(tr.pHit));
	}
	m_lastPos = pev->origin;

	pev->nextthink = gpGlobals->time + 0.01;
	*/
	// removed in favor of sv_retouch in rehlds. This wasn't working with gargs.

	pev->nextthink = gpGlobals->time + 0.05;
}

void CShockBeam::ExplodeThink()
{
	Explode();
	UTIL_Remove( this );
}

void CShockBeam::WaterExplodeThink()
{
	auto pOwner = VARS( pev->owner );

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_EXPLOSION);
	WRITE_COORD(pev->origin.x);
	WRITE_COORD(pev->origin.y);
	WRITE_COORD(pev->origin.z);
	WRITE_SHORT(m_waterExplodeSpr);
	WRITE_BYTE(30); // scale * 10
	WRITE_BYTE(50); // framerate
	WRITE_BYTE(2 | 4 | 8); // no light, sound, nor particles
	MESSAGE_END();

	Explode();

	::RadiusDamage( pev->origin, pev, pOwner, 100.0, 150.0, CLASS_NONE, DMG_ALWAYSGIB | DMG_BLAST );
	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, QUIET_GUN_VOLUME, 0.1);

	UTIL_Remove( this );
}

void CShockBeam::BallTouch( CBaseEntity* pOther )
{
	if (pOther->edict() == pev->owner || pOther->pev->solid == SOLID_TRIGGER) {
		return;
	}

	SetTouch( nullptr );
	SetThink( nullptr );

	if( pOther->pev->takedamage != DAMAGE_NO )
	{
		TraceResult tr = UTIL_GetGlobalTrace();
		tr.vecEndPos = pev->origin;

		ClearMultiDamage();

		const auto damage = gSkillData.sk_plr_shockrifle;

		auto bitsDamageTypes = DMG_SHOCK;

		auto pMonster = pOther->MyMonsterPointer();

		if( pMonster )
		{
			bitsDamageTypes |= DMG_BLAST;

			if( pMonster->m_flShockDuration > 0 )
			{
				bitsDamageTypes |= DMG_ALWAYSGIB;
			}
		}

		pOther->TraceAttack( VARS( pev->owner ), damage, pev->velocity.Normalize(), &tr, bitsDamageTypes );

		if( pMonster )
		{
			pMonster->AddShockEffect( 63.0, 152.0, 208.0, 16.0, 0.5 );
		}

		ApplyMultiDamage( pev, VARS( pev->owner ) );

		pev->velocity = g_vecZero;
	}

	SetThink( &CShockBeam::ExplodeThink );
	pev->nextthink = gpGlobals->time + 0.01;

	if( pOther->pev->takedamage == DAMAGE_NO )
	{
		TraceResult tr;
		UTIL_TraceLine( pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, edict(), &tr );

		UTIL_DecalTrace( &tr, DECAL_OFSCORCH1 + RANDOM_LONG( 0, 2 ) );

		MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		g_engfuncs.pfnWriteByte( TE_SPARKS );
		g_engfuncs.pfnWriteCoord( pev->origin.x );
		g_engfuncs.pfnWriteCoord( pev->origin.y );
		g_engfuncs.pfnWriteCoord( pev->origin.z );
		MESSAGE_END();
	}
}

void CShockBeam::Explode()
{
	if( m_hSprite )
	{
		UTIL_Remove(m_hSprite);
		m_hSprite = nullptr;
	}

	if( m_hBeam1 )
	{
		UTIL_Remove(m_hBeam1);
		m_hBeam1 = nullptr;
	}

	if( m_hBeam2 )
	{
		UTIL_Remove(m_hBeam2);
		m_hBeam2 = nullptr;
	}

	pev->dmg = 40;

	MESSAGE_BEGIN( MSG_PVS, SVC_TEMPENTITY, pev->origin );
	WRITE_BYTE( TE_DLIGHT );
	WRITE_COORD( pev->origin.x );
	WRITE_COORD( pev->origin.y );
	WRITE_COORD( pev->origin.z );
	WRITE_BYTE( 8 );
	WRITE_BYTE( 0 );
	WRITE_BYTE( 253 );
	WRITE_BYTE( 253 );
	WRITE_BYTE( 5 );
	WRITE_BYTE( 10 );
	MESSAGE_END();

	// gib corpses
	edict_t* ent = NULL;
	while (!FNullEnt(ent = FIND_ENTITY_IN_SPHERE(ent, pev->origin, 16))) {
		if ((ent->v.flags & (FL_MONSTER | FL_CLIENT)) && ent->v.deadflag >= DEAD_DEAD && ent->v.solid == SOLID_NOT && !(ent->v.effects & EF_NODRAW)) {
			CBaseEntity* baseent = CBaseEntity::Instance(ent);
			if (baseent && pev->owner) {
				baseent->Killed(&pev->owner->v, GIB_ALWAYS);
			}
		}
	}

	pev->owner = nullptr;

	EMIT_SOUND( edict(), CHAN_WEAPON, "weapons/shock_impact.wav", RANDOM_FLOAT( 0.8, 0.9 ), ATTN_NORM );
	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, QUIET_GUN_VOLUME, 0.1);
}

CShockBeam* CShockBeam::CreateShockBeam( const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner )
{
	auto pBeam = GetClassPtr<CShockBeam>( nullptr );

	pBeam->pev->origin = vecOrigin;
	pBeam->pev->angles = vecAngles;
	pBeam->pev->angles.x = -pBeam->pev->angles.x;

	UTIL_MakeVectors( pBeam->pev->angles );

	pBeam->pev->velocity = gpGlobals->v_forward * 2000.0;
	pBeam->pev->velocity.z = -pBeam->pev->velocity.z;

	pBeam->pev->classname = MAKE_STRING( "shock_beam" );

	pBeam->Spawn();

	pBeam->pev->owner = pOwner->edict();

	return pBeam;
}
