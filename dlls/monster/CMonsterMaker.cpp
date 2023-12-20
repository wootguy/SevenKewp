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
//=========================================================
// Monster Maker - this is an entity that creates monsters
// in the game.
//=========================================================

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "saverestore.h"

// Monstermaker spawnflags
#define	SF_MONSTERMAKER_START_ON	1 // start active ( if has targetname )
#define	SF_MONSTERMAKER_CYCLIC		4 // drop one monster every time fired.
#define SF_MONSTERMAKER_MONSTERCLIP	8 // Children are blocked by monsterclip
#define SF_MONSTERMAKER_PRISONER	16 // Children are prisoners
#define SF_MONSTERMAKER_WAIT_SCRIPT	128 // Children wait for a scripted sequence

#define MAX_XENMAKER_BEAMS 64 // each beam will need a TE message so don't go too crazy...
#define XENMAKER_SOUND1 "debris/beamstart7.wav"
#define XENMAKER_SOUND2 "debris/beamstart2.wav"

enum blocked_spawn_modes {
	SPAWN_BLOCK_LEGACY, // fail the spawn if blocked
	SPAWN_BLOCK_WAIT, // wait for the blockage to clear, then spawn
	SPAWN_BLOCK_IGNORE // spawn even if blocked (xenmaker mode)
};

//=========================================================
// MonsterMaker - this ent creates monsters during the game.
//=========================================================
class CMonsterMaker : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData* pkvd);
	void EXPORT ToggleUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT CyclicUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void EXPORT MakerThink ( void );
	void DeathNotice ( entvars_t *pevChild );// monster maker children use this to tell the monster maker that they have died.
	void MakeMonster( void );
	void XenmakerEffect();

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];
	
	string_t m_iszMonsterClassname;// classname of the monster(s) that will be created.
	
	int	 m_cNumMonsters;// max number of monsters this ent can create

	
	int  m_cLiveChildren;// how many monsters made by this monster maker that are currently alive
	int	 m_iMaxLiveChildren;// max number of monsters that this maker may have out at one time.

	float m_flGround; // z coord of the ground under me, used to make sure no monsters are under the maker when it drops a new child

	BOOL m_fActive;
	BOOL m_fFadeChildren;// should we make the children fadeout?

	int m_blockedSpawnMode;

	// env_xenmaker effect settings
	float m_flBeamRadius;
	int m_iBeamAlpha;
	int m_iBeamCount;
	Vector m_vBeamColor;
	float m_flLightRadius;
	Vector m_vLightColor;
	float m_flStartSpriteFramerate;
	float m_flStartSpriteScale;
	int m_iStartSpriteAlpha;
	int m_xenSpriteIdx;
	int m_xenBeamSpriteIdx;
	float m_nextXenSound;

	string_t m_xenmakerTemplate; // grab xenmaker settings from another entity
};

LINK_ENTITY_TO_CLASS( monstermaker, CMonsterMaker );
LINK_ENTITY_TO_CLASS( squadmaker, CMonsterMaker );
LINK_ENTITY_TO_CLASS( env_xenmaker, CMonsterMaker );

TYPEDESCRIPTION	CMonsterMaker::m_SaveData[] = 
{
	DEFINE_FIELD( CMonsterMaker, m_iszMonsterClassname, FIELD_STRING ),
	DEFINE_FIELD( CMonsterMaker, m_cNumMonsters, FIELD_INTEGER ),
	DEFINE_FIELD( CMonsterMaker, m_cLiveChildren, FIELD_INTEGER ),
	DEFINE_FIELD( CMonsterMaker, m_flGround, FIELD_FLOAT ),
	DEFINE_FIELD( CMonsterMaker, m_iMaxLiveChildren, FIELD_INTEGER ),
	DEFINE_FIELD( CMonsterMaker, m_fActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMonsterMaker, m_fFadeChildren, FIELD_BOOLEAN ),
	DEFINE_FIELD( CMonsterMaker, m_iTriggerCondition, FIELD_INTEGER),
	DEFINE_FIELD( CMonsterMaker, m_iszTriggerTarget, FIELD_STRING),
};


IMPLEMENT_SAVERESTORE( CMonsterMaker, CBaseMonster );

void CMonsterMaker :: KeyValue( KeyValueData *pkvd )
{
	if ( FStrEq(pkvd->szKeyName, "monstercount") )
	{
		m_cNumMonsters = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "m_imaxlivechildren") )
	{
		m_iMaxLiveChildren = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if ( FStrEq(pkvd->szKeyName, "monstertype") )
	{
		m_iszMonsterClassname = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "new_model"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawn_mode"))
	{
		m_blockedSpawnMode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "xenmaker"))
	{
		m_xenmakerTemplate = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	// env_xenmaker keys
	else if (FStrEq(pkvd->szKeyName, "m_flBeamRadius"))
	{
		m_flBeamRadius = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iBeamAlpha"))
	{
		m_iBeamAlpha = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iBeamCount"))
	{
		m_iBeamCount = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_vBeamColor"))
	{
		UTIL_StringToVector(m_vBeamColor, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flLightRadius"))
	{
		m_flLightRadius = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_vLightColor"))
	{
		UTIL_StringToVector(m_vLightColor, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flStartSpriteFramerate"))
	{
		m_flStartSpriteFramerate = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flStartSpriteScale"))
	{
		m_flStartSpriteScale = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iStartSpriteAlpha"))
	{
		// svencoop.fgd implies 255 is fully opaque but it's actually inverted.
		// 255 = barely visible. 1 = max opaqueness. 0 = invisible.
		// most (all?) maps use the default 255 so you may not have ever seen this sprite!
		// This should maybe treat 255 as invisible to save a precache slot.
		m_iStartSpriteAlpha = abs(atoi(pkvd->szValue)) % 256;
		if (m_iStartSpriteAlpha > 0) {
			m_iStartSpriteAlpha = 255 - (m_iStartSpriteAlpha-1);
		}

		pkvd->fHandled = TRUE;
	}

	// the following xenmaker keys exist but have no effect.
	// There is no "EndSprite" and sprite color can't be changed.
	/*
	else if (FStrEq(pkvd->szKeyName, "m_vStartpriteColor"))
	{
		UTIL_StringToVector(m_vStartpriteColor, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flEndSpriteFramerate"))
	{
		m_flEndSpriteFramerate= atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flEndSpriteScale"))
	{
		m_flEndSpriteScale = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iEndSpriteAlpha"))
	{
		m_iEndSpriteAlpha= atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_vEndSpriteColor"))
	{
		UTIL_StringToVector(m_vEndSpriteColor, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	*/

	// TODO: ripent the maps or something. This key logic is duplicated in CBaseMonster
	else if (FStrEq(pkvd->szKeyName, "trigger_target"))
	{
		m_iszTriggerTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "trigger_condition"))
	{
		m_iTriggerCondition = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "respawn_as_playerally"))
	{
		m_IsPlayerAlly = atoi(pkvd->szValue) != 0;
		pkvd->fHandled = TRUE;
	}

	else
		CBaseMonster::KeyValue( pkvd );
}


void CMonsterMaker :: Spawn( )
{
	pev->solid = SOLID_NOT;

	m_cLiveChildren = 0;
	Precache();
	
	if (!strcmp(STRING(pev->classname), "env_xenmaker")) {
		if (pev->spawnflags & 1) { // "Try Once"
			// according to sven manor this should change how blocked spawning works, but it does nothing
			// xenmaker always forces a spawn, even if blocked
		}
		if (pev->spawnflags & 2) { // "No Spawn"
			// hack to prevent anything from spawning
			m_cLiveChildren = m_iMaxLiveChildren = 1;
		}
		
		pev->spawnflags = SF_MONSTERMAKER_CYCLIC;
		m_blockedSpawnMode = SPAWN_BLOCK_IGNORE;
	}

	if ( !FStringNull ( pev->targetname ) )
	{
		if ( pev->spawnflags & SF_MONSTERMAKER_CYCLIC )
		{
			SetUse ( &CMonsterMaker::CyclicUse );// drop one monster each time we fire
		}
		else
		{
			SetUse ( &CMonsterMaker::ToggleUse );// so can be turned on/off
		}

		if ( FBitSet ( pev->spawnflags, SF_MONSTERMAKER_START_ON ) )
		{// start making monsters as soon as monstermaker spawns
			m_fActive = TRUE;
			SetThink ( &CMonsterMaker::MakerThink );
		}
		else
		{// wait to be activated.
			m_fActive = FALSE;
			SetThink ( &CMonsterMaker::SUB_DoNothing );
		}
	}
	else
	{// no targetname, just start.
			pev->nextthink = gpGlobals->time + m_flDelay;
			m_fActive = TRUE;
			SetThink ( &CMonsterMaker::MakerThink );
	}

	if ( m_cNumMonsters == 1 )
	{
		m_fFadeChildren = FALSE;
	}
	else
	{
		m_fFadeChildren = TRUE;
	}

	m_flGround = 0;
}

void CMonsterMaker :: Precache( void )
{
	CBaseMonster::Precache();

	UTIL_PrecacheOther( STRING( m_iszMonsterClassname ) );

	if (pev->model) {
		PRECACHE_MODEL(STRING(pev->model));
	}

	if (!strcmp(STRING(pev->classname), "env_xenmaker")) {
		m_xenSpriteIdx = PRECACHE_MODEL("sprites/fexplo1.spr");
		m_xenBeamSpriteIdx = PRECACHE_MODEL("sprites/laserbeam.spr");
		PRECACHE_SOUND(XENMAKER_SOUND1);
		PRECACHE_SOUND(XENMAKER_SOUND2);
	}
}

//=========================================================
// MakeMonster-  this is the code that drops the monster
//=========================================================
void CMonsterMaker::MakeMonster( void )
{
	edict_t	*pent;
	entvars_t		*pevCreate;

	XenmakerEffect();

	if ( m_iMaxLiveChildren > 0 && m_cLiveChildren >= m_iMaxLiveChildren )
	{// not allowed to make a new one yet. Too many live ones out right now.
		return;
	}

	if ( !m_flGround )
	{
		// set altitude. Now that I'm activated, any breakables, etc should be out from under me. 
		TraceResult tr;

		UTIL_TraceLine ( pev->origin, pev->origin - Vector ( 0, 0, 2048 ), ignore_monsters, ENT(pev), &tr );
		m_flGround = tr.vecEndPos.z;
	}

	Vector mins = pev->origin - Vector( 34, 34, 0 );
	Vector maxs = pev->origin + Vector( 34, 34, 0 );
	maxs.z = pev->origin.z;
	mins.z = m_flGround;

	if (m_blockedSpawnMode != SPAWN_BLOCK_IGNORE) {
		CBaseEntity* pList[2];
		int count = UTIL_EntitiesInBox(pList, 2, mins, maxs, FL_CLIENT | FL_MONSTER, true);
		if (count)
		{
			// don't build a stack of monsters!
			if (m_blockedSpawnMode == SPAWN_BLOCK_WAIT) {
				pev->nextthink = gpGlobals->time + 0.05f; // recheck quickly
			}

			return;
		}
	}

	pent = CREATE_NAMED_ENTITY( m_iszMonsterClassname );

	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, UTIL_VarArgs("NULL Ent '%s' in MonsterMaker!\n", STRING(m_iszMonsterClassname)) );
		return;
	}

	pent->v.model = pev->model;
	if (pev->health) {
		pent->v.health = pev->health;
	}

	// TODO: Repel monsters need to do this setting-copy stuff too.
	// make a generic function that they all use instead of duplicating codes
	pent->v.rendermode = pev->rendermode;
	pent->v.renderamt = pev->renderamt;
	pent->v.renderfx = pev->renderfx;
	pent->v.rendercolor = pev->rendercolor;
	pent->v.weapons = pev->weapons;

	CBaseMonster* mon = ((CBaseEntity*)GET_PRIVATE(pent))->MyMonsterPointer();
	if (mon) {
		mon->m_iszTriggerTarget = m_iszTriggerTarget;
		mon->m_iTriggerCondition = m_iTriggerCondition;
		mon->m_displayName = m_displayName;
		mon->m_Classify = m_Classify;
		mon->m_IsPlayerAlly = m_IsPlayerAlly;
	}

	pevCreate = VARS( pent );
	pevCreate->origin = pev->origin;
	pevCreate->angles = pev->angles;
	SetBits( pevCreate->spawnflags, SF_MONSTER_FALL_TO_GROUND );

	// Children hit monsterclip brushes
	if ( pev->spawnflags & SF_MONSTERMAKER_MONSTERCLIP )
		SetBits( pevCreate->spawnflags, SF_MONSTER_HITMONSTERCLIP );

	// copy prisoner/script flags to child
	pevCreate->spawnflags |= pev->spawnflags & (SF_MONSTERMAKER_PRISONER | SF_MONSTERMAKER_WAIT_SCRIPT);

	DispatchSpawn( ENT( pevCreate ) );
	pevCreate->owner = edict();

	if ( !FStringNull( pev->netname ) )
	{
		// if I have a netname (overloaded), give the child monster that name as a targetname
		pevCreate->targetname = pev->netname;
	}

	m_cLiveChildren++;// count this monster
	m_cNumMonsters--;

	if ( m_cNumMonsters == 0 )
	{
		// Disable this forever.  Don't kill it because it still gets death notices
		SetThink( NULL );
		SetUse( NULL );
	}

	// If I have a target, fire!
	if (!FStringNull(pev->target))
	{
		// delay already overloaded for this entity, so can't call SUB_UseTargets()
		FireTargets(STRING(pev->target), this, this, USE_TOGGLE, 0);
	}
}

//=========================================================
// CyclicUse - drops one monster from the monstermaker
// each time we call this.
//=========================================================
void CMonsterMaker::CyclicUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	MakeMonster();
}

//=========================================================
// ToggleUse - activates/deactivates the monster maker
//=========================================================
void CMonsterMaker :: ToggleUse ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if ( !ShouldToggle( useType, m_fActive ) )
		return;

	if ( m_fActive )
	{
		m_fActive = FALSE;
		SetThink ( NULL );
	}
	else
	{
		m_fActive = TRUE;
		SetThink ( &CMonsterMaker::MakerThink );
	}

	pev->nextthink = gpGlobals->time;
}

void CMonsterMaker :: XenmakerEffect() {
	CMonsterMaker* xen = this;
	
	if (!m_xenSpriteIdx || !m_xenBeamSpriteIdx) {
		//ALERT(at_console, "xenmaker sprites not precached");
		return;
	}

	if (m_xenmakerTemplate) {
		// searching every spawn so that entity order doesn't matter during map init
		edict_t* ent = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_xenmakerTemplate));
		if (!FNullEnt(ent) && !strcmp(STRING(ent->v.classname), "env_xenmaker")) {
			xen = (CMonsterMaker*)GET_PRIVATE(ent);
		}
	}

	Vector position = pev->origin;
	position.z += (pev->absmax.z - pev->absmin.z) * 0.5f;

	const int effectDuration = 10;

	// sven uses a special MSG_TE_CUSTOM message for the effect, which can
	// mostly be recreated with existing temporary entity effects.
	// The difference is that the sprite is killed after a set amount of time,
	// even if the animation hasn't finished, and the framerate can be changed. 
	// No one is going to see the sprite because they're all at 1 opacity by default,
	// so I'll just use TE_SPRITE here.
	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_SPRITE);
	WRITE_COORD(position.x);
	WRITE_COORD(position.y);
	WRITE_COORD(position.z);
	WRITE_SHORT(xen->m_xenSpriteIdx);
	WRITE_BYTE(V_min(255, xen->m_flStartSpriteScale*10));
	WRITE_BYTE(m_iStartSpriteAlpha);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
	WRITE_BYTE(TE_DLIGHT);
	WRITE_COORD(position.x);
	WRITE_COORD(position.y);
	WRITE_COORD(position.z);
	WRITE_BYTE(xen->m_flLightRadius / 10);
	WRITE_BYTE(xen->m_vLightColor.x);
	WRITE_BYTE(xen->m_vLightColor.y);
	WRITE_BYTE(xen->m_vLightColor.z);
	WRITE_BYTE(effectDuration); // life
	WRITE_BYTE(0); // decay rate
	MESSAGE_END();

	int createdBeams = 0;
	TraceResult tr;
	for (int i = 0; i < m_iBeamCount; i++) {
		// TODO: uniform distribution
		Vector randomDir = Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1));
		Vector randomPos = position + randomDir.Normalize() * m_flBeamRadius;

		TRACE_LINE(position, randomPos, ignore_monsters, NULL, &tr);
		if (tr.flFraction < 1.0f) {
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, position);
			WRITE_BYTE(TE_BEAMPOINTS);
			WRITE_COORD(position.x);
			WRITE_COORD(position.y);
			WRITE_COORD(position.z);
			WRITE_COORD(tr.vecEndPos.x);
			WRITE_COORD(tr.vecEndPos.y);
			WRITE_COORD(tr.vecEndPos.z);
			WRITE_SHORT(xen->m_xenBeamSpriteIdx);
			WRITE_BYTE(0); // frame start
			WRITE_BYTE(0); // frame rate
			WRITE_BYTE(effectDuration); // life
			WRITE_BYTE(10); // width
			WRITE_BYTE(50); // noise
			WRITE_BYTE(xen->m_vBeamColor.x);
			WRITE_BYTE(xen->m_vBeamColor.y);
			WRITE_BYTE(xen->m_vBeamColor.z);
			WRITE_BYTE(xen->m_iBeamAlpha);
			WRITE_BYTE(0); // scroll
			MESSAGE_END();

			if (++createdBeams >= MAX_XENMAKER_BEAMS) {
				break;
			}
		}
	}

	EMIT_SOUND(ENT(pev), CHAN_VOICE, XENMAKER_SOUND1, 1.0f, ATTN_NORM);

	m_nextXenSound = gpGlobals->time + 1.0f;

	if (!m_fActive) {
		SetThink(&CMonsterMaker::MakerThink);
		pev->nextthink = gpGlobals->time + 1.0f;
	}
}

//=========================================================
// MakerThink - creates a new monster every so often
//=========================================================
void CMonsterMaker :: MakerThink ( void )
{
	pev->nextthink = gpGlobals->time + m_flDelay;

	if (m_nextXenSound && gpGlobals->time >= m_nextXenSound) {
		m_nextXenSound = 0;
		EMIT_SOUND(ENT(pev), CHAN_VOICE, XENMAKER_SOUND2, 1.0f, ATTN_NORM);
	}

	if (m_fActive) {
		MakeMonster();
	}
	else if (!m_nextXenSound) {
		SetThink(NULL);
	}
}

//=========================================================
//=========================================================
void CMonsterMaker :: DeathNotice ( entvars_t *pevChild )
{
	// ok, we've gotten the deathnotice from our child, now clear out its owner if we don't want it to fade.
	m_cLiveChildren--;

	if ( !m_fFadeChildren )
	{
		pevChild->owner = NULL;
	}
}


