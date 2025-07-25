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
/*

===== generic grenade.cpp ========================================================

*/

#include "extdll.h"
#include "util.h"
#include "skill.h"
#include "weapons.h"
#include "nodes.h"
#include "env/CSoundEnt.h"
#include "decals.h"
#include "weapon/CGrenade.h"
#include "te_effects.h"

//===================grenade


LINK_ENTITY_TO_CLASS( grenade, CGrenade )

// Grenades flagged with this will be triggered when the owner calls detonateSatchelCharges
#define SF_GRENADE_DETONATE		0x0001

//
// Grenade Explode
//
void CGrenade::Explode( Vector vecSrc, Vector vecAim )
{
	TraceResult tr;
	UTIL_TraceLine ( pev->origin, pev->origin + Vector ( 0, 0, -32 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}

// UNDONE: temporary scorching for PreAlpha - find a less sleazy permenant solution.
void CGrenade::Explode( TraceResult *pTrace, int bitsDamageType )
{
	pev->model = iStringNull;//invisible
	pev->solid = SOLID_NOT;// intangible

	pev->takedamage = DAMAGE_NO;

	m_effectOrigin = pev->origin;

	// Pull out of the wall a bit
	if (pTrace->flFraction != 1.0)
	{
		float dist = V_max(16, (pev->dmg - 24)) * 0.6;
		m_effectOrigin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * dist);

		if (mp_explosionbug.value == 0) {
			// Move damage origin closer to sprite origin so explosions don't detonate inside
			// tiny puddles and do no damage. TODO: duplicated in CEnvExplosion.
			Vector damageOrigin = pTrace->vecEndPos + (pTrace->vecPlaneNormal * V_min(32, dist));
			
			TraceResult tr;
			TRACE_LINE(pev->origin, damageOrigin, ignore_monsters, NULL, &tr);
			if (!tr.fStartSolid)
				pev->origin = tr.vecEndPos;
		}
	}

	int iContents = UTIL_PointContents ( pev->origin );
	
	int spr = iContents != CONTENTS_WATER ? g_sModelIndexFireball : g_sModelIndexWExplosion;
	UTIL_Explosion(m_effectOrigin, spr, (pev->dmg - 50) * .60, 15, TE_EXPLFLAG_NONE);

	CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 3.0 );
	entvars_t *pevOwner;
	if ( pev->owner )
		pevOwner = VARS( pev->owner );
	else
		pevOwner = NULL;

	pev->owner = NULL; // can't traceline attack owner if this is set

	if (mp_explosionbug.value) {
		RadiusDamage(m_effectOrigin, pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType);
	}
	else {
		RadiusDamage(pev, pevOwner, pev->dmg, CLASS_NONE, bitsDamageType);
	}

	if ( RANDOM_FLOAT( 0 , 1 ) < 0.5 )
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH1 );
	}
	else
	{
		UTIL_DecalTrace( pTrace, DECAL_SCORCH2 );
	}

	switch ( RANDOM_LONG( 0, 2 ) )
	{
		case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM);	break;
		case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM);	break;
		case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM);	break;
	}

	PLAY_DISTANT_SOUND(edict(), DISTANT_BOOM);

	// not using EF_NODRAW so sounds work out of bounds
	pev->renderamt = 0;
	pev->rendermode = kRenderTransTexture;

	SetThink( &CGrenade::Smoke );
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.3;

	if (iContents != CONTENTS_WATER)
	{
		int maxShowers = UTIL_IsValidTempEntOrigin(m_effectOrigin) ? 3 : 2;
		int sparkCount = RANDOM_LONG(0, maxShowers);
		for ( int i = 0; i < sparkCount; i++ )
			Create( "spark_shower", m_effectOrigin, pTrace->vecPlaneNormal );
	}
}


void CGrenade::Smoke( void )
{
	if (UTIL_PointContents ( pev->origin ) == CONTENTS_WATER)
	{
		UTIL_Bubbles( pev->origin - Vector( 64, 64, 64 ), pev->origin + Vector( 64, 64, 64 ), 100 );
	}
	else
	{
		UTIL_Smoke(m_effectOrigin, g_sModelIndexSmoke, (pev->dmg - 50) * 0.80, 12);
	}
	UTIL_Remove( this );
}

void CGrenade::Killed( entvars_t *pevAttacker, int iGib )
{
	Detonate( );
}

const char* CGrenade::GetModel() {

	if (pev->model) {
		return STRING(pev->model);
	}

	return mp_mergemodels.value && MergedModelBody() != -1 ? MERGED_ITEMS_MODEL : m_defaultModel;
}

void CGrenade::SetGrenadeModel() {
	if (pev->model || MergedModelBody() == -1) {
		SET_MODEL(ENT(pev), GetModel());
	}
	else {
		SET_MODEL_MERGED(ENT(pev), GetModel(), MergedModelBody());
	}
}

// Timed grenade, this think is called when time runs out.
void CGrenade::DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	SetThink( &CGrenade::Detonate );
	pev->nextthink = gpGlobals->time;
}

void CGrenade::PreDetonate( void )
{
	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, 400, 0.3 );

	SetThink( &CGrenade::Detonate );
	pev->nextthink = gpGlobals->time + 1;
}


void CGrenade::Detonate( void )
{
	FCheckAITrigger();
	// tell owner ( if any ) that we're dead.This is mostly for MonsterMaker functionality.
	CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
	if (pOwner && !m_deathNoticeSent) {
		pOwner->DeathNotice(pev);
		m_deathNoticeSent = true;
	}

	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	vecSpot = pev->origin + Vector ( 0 , 0 , 8 );
	UTIL_TraceLine ( vecSpot, vecSpot + Vector ( 0, 0, -40 ),  ignore_monsters, ENT(pev), & tr);

	Explode( &tr, DMG_BLAST );
}


//
// Contact grenade, explode when it touches something
// 
void CGrenade::ExplodeTouch( CBaseEntity *pOther )
{
	TraceResult tr;
	Vector		vecSpot;// trace starts here!

	pev->enemy = pOther->edict();

	vecSpot = pev->origin - pev->velocity.Normalize() * 32;
	UTIL_TraceLine( vecSpot, vecSpot + pev->velocity.Normalize() * 64, ignore_monsters, ENT(pev), &tr );

	Explode( &tr, DMG_BLAST );
}


void CGrenade::DangerSoundThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * 0.5, pev->velocity.Length( ), 0.2 );
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
	}
}


void CGrenade::BounceTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// only do damage if we're moving fairly fast
	if (m_flNextAttack < gpGlobals->time && pev->velocity.Length() > 100)
	{
		entvars_t *pevOwner = VARS( pev->owner );
		if (pevOwner)
		{
			TraceResult tr = UTIL_GetGlobalTrace( );
			ClearMultiDamage( );
			pOther->TraceAttack(pevOwner, 1, gpGlobals->v_forward, &tr, DMG_CLUB ); 
			ApplyMultiDamage( pev, pevOwner);
		}
		m_flNextAttack = gpGlobals->time + 1.0; // debounce
	}

	Vector vecTestVelocity;
	// pev->avelocity = Vector (300, 300, 300);

	// this is my heuristic for modulating the grenade velocity because grenades dropped purely vertical
	// or thrown very far tend to slow down too quickly for me to always catch just by testing velocity. 
	// trimming the Z velocity a bit seems to help quite a bit.
	vecTestVelocity = pev->velocity; 
	vecTestVelocity.z *= 0.45;

	if ( !m_fRegisteredSound && vecTestVelocity.Length() <= 60 )
	{
		//ALERT( at_console, "Grenade Registered!: %f\n", vecTestVelocity.Length() );

		// grenade is moving really slow. It's probably very close to where it will ultimately stop moving. 
		// go ahead and emit the danger sound.
		
		// register a radius louder than the explosion, so we make sure everyone gets out of the way
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin, pev->dmg / 0.4, 0.3 );
		m_fRegisteredSound = TRUE;
	}

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.985;

		if (pev->sequence != 1) {
			pev->sequence = 1;
			ResetSequenceInfo();
		}
	}
	else
	{
		// play bounce sound
		BounceSound();
	}
}



void CGrenade::SlideTouch( CBaseEntity *pOther )
{
	// don't hit the guy that launched this grenade
	if ( pOther->edict() == pev->owner )
		return;

	// pev->avelocity = Vector (300, 300, 300);

	if (pev->flags & FL_ONGROUND)
	{
		// add a bit of static friction
		pev->velocity = pev->velocity * 0.95;

		if (pev->velocity.x != 0 || pev->velocity.y != 0)
		{
			// maintain sliding sound
		}
	}
	else
	{
		BounceSound();
	}
}

void CGrenade :: BounceSound( void )
{
	switch ( RANDOM_LONG( 0, 2 ) )
	{
	case 0:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit1.wav", 0.25, ATTN_NORM);	break;
	case 1:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit2.wav", 0.25, ATTN_NORM);	break;
	case 2:	EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/grenade_hit3.wav", 0.25, ATTN_NORM);	break;
	}
}

void CGrenade :: TumbleThink( void )
{
	if (!IsInWorld())
	{
		UTIL_Remove( this );
		return;
	}

	StudioFrameAdvance( );
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->dmgtime - 1 < gpGlobals->time)
	{
		CSoundEnt::InsertSound ( bits_SOUND_DANGER, pev->origin + pev->velocity * (pev->dmgtime - gpGlobals->time), 400, 0.1 );
	}

	if (pev->dmgtime <= gpGlobals->time)
	{
		SetThink( &CGrenade::Detonate );
	}
	if (pev->waterlevel != 0)
	{
		pev->velocity = pev->velocity * 0.5;
		pev->framerate = 0.2;
	}

	if (pev->sequence == 1) {
		pev->framerate = pev->velocity.Length() / 40.0;
		if (pev->framerate > 8.0)
			pev->framerate = 8;
		else if (pev->velocity.Length() < 50) {
			pev->framerate = FLT_MIN;
		}

		if (pev->flags & FL_ONGROUND && pev->velocity.Length() < 100) {
			pev->velocity = pev->velocity * 0.6f;
		}
	}
}


void CGrenade:: Spawn( void )
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;

	m_defaultModel = "models/w_grenade.mdl";
	SET_MODEL(ENT(pev), GetModel());
	UTIL_SetSize(pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	pev->dmg = GetDamage(100);
	m_fRegisteredSound = FALSE;

	if (FClassnameIs(pev, "monster_handgrenade")) {
		SetTouch(&CGrenade::BounceTouch);	// Bounce if touched

		float time = 3.5f;
		pev->dmgtime = gpGlobals->time + time;
		SetThink(&CGrenade::TumbleThink);
		pev->nextthink = gpGlobals->time + 0.1;

		pev->sequence = RANDOM_LONG(3, 6);
		pev->framerate = 1.0;

		pev->gravity = 0.5;
		pev->friction = 0.8;
	}
	else {
		pev->classname = MAKE_STRING("grenade");
	}
}


CGrenade *CGrenade::ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );

	pGrenade->pev->model = MAKE_STRING("models/grenade.mdl");
	pGrenade->Spawn();

	// contact grenades arc lower
	pGrenade->pev->gravity = 0.5;// lower gravity since grenade is aerodynamic and engine doesn't know it.
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles (pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	// make monsters afaid of it while in the air
	pGrenade->SetThink( &CGrenade::DangerSoundThink );
	pGrenade->pev->nextthink = gpGlobals->time;
	
	// Tumble in air
	pGrenade->pev->avelocity.x = RANDOM_FLOAT ( -100, -500 );
	
	// Explode on contact
	pGrenade->SetTouch( &CGrenade::ExplodeTouch );

	pGrenade->pev->dmg = pGrenade->GetDamage(gSkillData.sk_plr_9mmAR_grenade);

	return pGrenade;
}


CGrenade * CGrenade:: ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, const char* model )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->Spawn();
	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = UTIL_VecToAngles(pGrenade->pev->velocity);
	pGrenade->pev->owner = ENT(pevOwner);
	
	pGrenade->SetTouch( &CGrenade::BounceTouch );	// Bounce if touched
	
	// Take one second off of the desired detonation time and set the think to PreDetonate. PreDetonate
	// will insert a DANGER sound into the world sound list and delay detonation for one second so that 
	// the grenade explodes after the exact amount of time specified in the call to ShootTimed(). 

	pGrenade->pev->dmgtime = gpGlobals->time + time;
	pGrenade->SetThink( &CGrenade::TumbleThink );
	pGrenade->pev->nextthink = gpGlobals->time + 0.1;
	if (time < 0.1)
	{
		pGrenade->pev->nextthink = gpGlobals->time;
		pGrenade->pev->velocity = Vector( 0, 0, 0 );
	}
		
	pGrenade->pev->sequence = RANDOM_LONG( 3, 6 );
	pGrenade->pev->framerate = 1.0;

	// Tumble through the air
	// pGrenade->pev->avelocity.x = -400;

	pGrenade->pev->gravity = 0.5;
	pGrenade->pev->friction = 0.8;

	pGrenade->pev->dmg = pGrenade->GetDamage(100);

	SET_MODEL(ENT(pGrenade->pev), model ? model : pGrenade->GetModel());

	return pGrenade;
}


CGrenade * CGrenade :: ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity )
{
	CGrenade *pGrenade = GetClassPtr( (CGrenade *)NULL );
	pGrenade->pev->movetype = MOVETYPE_BOUNCE;
	pGrenade->pev->classname = MAKE_STRING( "grenade" );
	
	pGrenade->pev->solid = SOLID_BBOX;

	pGrenade->m_defaultModel = "models/w_satchel.mdl";
	SET_MODEL(ENT(pGrenade->pev), pGrenade->GetModel());	// Change this to satchel charge model

	UTIL_SetSize(pGrenade->pev, Vector( 0, 0, 0), Vector(0, 0, 0));

	UTIL_SetOrigin( pGrenade->pev, vecStart );
	pGrenade->pev->velocity = vecVelocity;
	pGrenade->pev->angles = g_vecZero;
	pGrenade->pev->owner = ENT(pevOwner);

	pGrenade->pev->dmg = pGrenade->GetDamage(200);
	
	// Detonate in "time" seconds
	pGrenade->SetThink( &CGrenade::SUB_DoNothing );
	pGrenade->SetUse( &CGrenade::DetonateUse );
	pGrenade->SetTouch( &CGrenade::SlideTouch );
	pGrenade->pev->spawnflags = SF_GRENADE_DETONATE;

	pGrenade->pev->friction = 0.9;

	return pGrenade;
}



void CGrenade :: UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code )
{
	CBaseEntity *pEnt;
	edict_t *pentOwner;

	if ( !pevOwner )
		return;

	CBaseEntity	*pOwner = CBaseEntity::Instance( pevOwner );

	pentOwner = pOwner->edict();

	pEnt = UTIL_FindEntityByClassname( NULL, "grenade" );
	while (pEnt)
	{
		if ( FBitSet( pEnt->pev->spawnflags, SF_GRENADE_DETONATE ) && pEnt->pev->owner == pentOwner )
		{
			if ( code == SATCHEL_DETONATE )
				pEnt->Use( pOwner, pOwner, USE_ON, 0 );
			else	// SATCHEL_RELEASE
				pEnt->pev->owner = NULL;
		}
		pEnt = UTIL_FindEntityByClassname(pEnt, "grenade" );
	}
}

LINK_ENTITY_TO_CLASS(monster_handgrenade, CGrenade)

//======================end grenade

