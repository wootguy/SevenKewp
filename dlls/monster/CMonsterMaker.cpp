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
#include "monsters.h"
#include "saverestore.h"
#include "weapons.h"
#include "CBasePlayerWeapon.h"
#include "CMonsterMaker.h"

LINK_ENTITY_TO_CLASS( monstermaker, CMonsterMaker )
LINK_ENTITY_TO_CLASS( squadmaker, CMonsterMaker )
LINK_ENTITY_TO_CLASS( env_xenmaker, CMonsterMaker )

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


IMPLEMENT_SAVERESTORE( CMonsterMaker, CBaseMonster )

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
	else if (FStrEq(pkvd->szKeyName, "wpn_v_model"))
	{
		m_weaponModelV = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wpn_p_model"))
	{
		m_weaponModelP = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wpn_w_model"))
	{
		m_weaponModelW = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "change_rendermode"))
	{
		m_changeRenderMode = atoi(pkvd->szValue);
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
	else if (FStrEq(pkvd->szKeyName, "TriggerTarget"))
	{
		// prevent base class key overriding the monstermaker key
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "TriggerCondition"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "is_player_ally"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "new_body"))
	{
		pev->body = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	else
		CBaseMonster::KeyValue( pkvd );
}

int CMonsterMaker::Classify(void) {
	return CBaseMonster::Classify(DefaultClassify(STRING(m_iszMonsterClassname)));
}

BOOL CMonsterMaker::HasTarget(string_t targetname) {
	return (CBaseMonster::HasTarget(targetname) || CBaseEntity::HasTarget(targetname)) ? TRUE : FALSE;
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
		m_cNumMonsters = -1;
		m_blockedSpawnMode = SPAWN_BLOCK_IGNORE;
	}

	if ((m_iszTriggerTarget || pev->target) && m_blockedSpawnMode == SPAWN_BLOCK_LEGACY) {
		// prevent softlocks from blocked spawns
		m_blockedSpawnMode = SPAWN_BLOCK_IGNORE;
	}

	pev->classname = MAKE_STRING("monstermaker");

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
			pev->nextthink = gpGlobals->time + m_flDelay;
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

	StringMap keys;
	if (m_soundReplacementKey)
		keys.put("soundlist", STRING(m_soundReplacementKey));
	if (m_IsPlayerAlly)
		keys.put("is_player_ally", "1");
	if (pev->weapons)
		keys.put("weapons", UTIL_VarArgs("%d", pev->weapons));

	UTIL_PrecacheOther( STRING( m_iszMonsterClassname ), keys );

	if (pev->model) {
		PRECACHE_MODEL(STRING(pev->model));
	}

	if (m_weaponModelV) {
		PRECACHE_MODEL(STRING(m_weaponModelV));
	}
	if (m_weaponModelP) {
		PRECACHE_MODEL(STRING(m_weaponModelP));
	}
	if (m_weaponModelW) {
		PRECACHE_MODEL(STRING(m_weaponModelW));
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
		int count = UTIL_EntitiesInBox(pList, 2, mins, maxs, FL_CLIENT | FL_MONSTER, true, true);
		if (count)
		{
			// don't build a stack of monsters!
			if (m_blockedSpawnMode == SPAWN_BLOCK_WAIT) {
				pev->nextthink = gpGlobals->time + 0.05f; // recheck quickly

				if (pev->spawnflags & SF_MONSTERMAKER_CYCLIC) {
					// this entity doesn't normally think in cyclic mode
					SetThink(&CMonsterMaker::BlockedCyclicThink);
				}
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
	pent->v.weapons = pev->weapons;

	// TODO: Repel monsters need to do this setting-copy stuff too.
	// make a generic function that they all use instead of duplicating codes
	if (m_changeRenderMode) {
		pent->v.rendermode = pev->rendermode;
		pent->v.renderamt = pev->renderamt;
		pent->v.renderfx = pev->renderfx;
		pent->v.rendercolor = pev->rendercolor;
	}

	CBaseMonster* mon = ((CBaseEntity*)GET_PRIVATE(pent))->MyMonsterPointer();
	if (mon) {
		mon->m_iszTriggerTarget = m_iszTriggerTarget;
		mon->m_iTriggerCondition = m_iTriggerCondition;
		mon->m_displayName = m_displayName;
		mon->m_Classify = m_Classify;
		mon->m_IsPlayerAlly = m_IsPlayerAlly;
		mon->m_soundReplacementKey = m_soundReplacementKey;
		mon->m_minHullSize = m_minHullSize;
		mon->m_maxHullSize = m_maxHullSize;
	}

	CBasePlayerWeapon* wep = ((CBaseEntity*)GET_PRIVATE(pent))->GetWeaponPtr();
	if (wep) {
		wep->m_customModelV = m_weaponModelV;
		wep->m_customModelP = m_weaponModelP;
		wep->m_customModelW = m_weaponModelW;
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

	if (pev->body > 0) {
		pent->v.body = pev->body;
	}

	if ( !FStringNull( pev->netname ) )
	{
		// if I have a netname (overloaded), give the child monster that name as a targetname
		pevCreate->targetname = pev->netname;
	}

	// unstuck monsters if spawned inside the floor/ceiling
	// (unless it's a barnacle/turret or smth that doesn't move)
	if (pent->v.movetype == MOVETYPE_STEP) {
		TraceResult tr;
		TRACE_MONSTER_HULL(pent, pent->v.origin, pent->v.origin + Vector(0, 0, maxs.z), ignore_monsters, pent, &tr);

		if (tr.fAllSolid) {
			Vector upPos = tr.vecEndPos;
			TRACE_MONSTER_HULL(pent, upPos, upPos - Vector(0, 0, 4096), ignore_monsters, pent, &tr);
			
			if (!tr.fAllSolid)
				UTIL_SetOrigin(&pent->v, tr.vecEndPos);

			// not using this because it will send the monster through solid entities if the monster is spawning
			// inside of another monster (startSolid)
			//DROP_TO_FLOOR(pent);
		}
		else if (tr.fStartSolid) {
			// was stuck in the floor, drop from the upward trace position
			TRACE_MONSTER_HULL(pent, tr.vecEndPos, tr.vecEndPos - Vector(0, 0, maxs.z), ignore_monsters, pent, &tr);
			if (!tr.fAllSolid)
				UTIL_SetOrigin(&pent->v, tr.vecEndPos);
		}
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
	UTIL_Sprite(position, m_xenSpriteIdx, V_min(255, xen->m_flStartSpriteScale * 10), m_iStartSpriteAlpha);

	UTIL_DLight(position, xen->m_flLightRadius / 10, xen->m_vLightColor, effectDuration, 0);

	int createdBeams = 0;
	TraceResult tr;
	for (int i = 0; i < m_iBeamCount; i++) {
		// TODO: uniform distribution
		Vector randomDir = Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1));
		Vector randomPos = position + randomDir.Normalize() * m_flBeamRadius;

		TRACE_LINE(position, randomPos, ignore_monsters, NULL, &tr);
		if (tr.flFraction < 1.0f) {
			UTIL_BeamPoints(position, tr.vecEndPos, xen->m_xenBeamSpriteIdx, 0, 0, effectDuration, 10, 50,
				RGBA(xen->m_vBeamColor, xen->m_iBeamAlpha), 0, MSG_PVS, position);

			if (++createdBeams >= MAX_XENMAKER_BEAMS) {
				break;
			}
		}
	}

	UTIL_TempSound(pev->origin, XENMAKER_SOUND1);

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
		UTIL_TempSound(pev->origin, XENMAKER_SOUND2);
	}

	if (m_fActive) {
		MakeMonster();
	}
	else if (!m_nextXenSound) {
		SetThink(NULL);
	}
}

void CMonsterMaker::BlockedCyclicThink() {
	SetThink(NULL); // cyclic makers shouldn't think unless MakeMonster is blocked
	MakeMonster();
}

//=========================================================
//=========================================================
void CMonsterMaker :: DeathNotice ( entvars_t *pevChild )
{
	// ok, we've gotten the deathnotice from our child, now clear out its owner if we don't want it to fade.
	m_cLiveChildren--;
	pevChild->owner = NULL;
}

// return number of triggers that would be removed if all monstermakers were nerfed
int CountMonsterTriggerNerfs(string_t targetname) {
	edict_t* edicts = ENT(0);
	int triggerCount = 0;
	int nerfCount = 0;

	int maxNerfedSpawnCount = mp_maxmonsterrespawns.value + 1.5f;

	for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++)
	{
		edict_t* ent = &edicts[i];
		if (ent->free)
			continue;

		const char* cname = STRING(ent->v.classname);
		CBaseEntity* pent = (CBaseEntity*)GET_PRIVATE(ent);
		if (!pent)
			continue;

		CBaseMonster* mon = pent->MyMonsterPointer();
		if (!mon) {
			continue;
		}

		if (!mon->HasTarget(targetname)) {
			continue;
		}

		if (!strcmp(cname, "monstermaker")) {
			CMonsterMaker* maker = (CMonsterMaker*)pent;

			triggerCount += maker->m_cNumMonsters;
			if (maker->m_cNumMonsters < 0) {
				return -1;
			}
			else if (maker->m_cNumMonsters > maxNerfedSpawnCount) {
				nerfCount += maker->m_cNumMonsters - maxNerfedSpawnCount;
			}
		}
		else if (strstr(cname, "monster_") == cname) {
			triggerCount++;
		}
		else {
			ALERT(at_console, "Unknown monster trigger classname: %s\n", STRING(ent->v.classname));
			return -1;
		}
	}

	return nerfCount;
}

bool CMonsterMaker::NerfMonsterCounters(string_t target) {
	bool shouldNerf = true;
	int maxNerfedSpawnCount = mp_maxmonsterrespawns.value + 1.5f;

	if (m_cNumMonsters <= maxNerfedSpawnCount) {
		return false;
	}

	edict_t* ent = NULL;
	while (!FNullEnt(ent = FIND_ENTITY_BY_TARGETNAME(ent, STRING(target)))) {
		if (strcmp(STRING(ent->v.classname), "game_counter") && strcmp(STRING(ent->v.classname), "trigger_counter")) {
			ALERT(at_aiconsole, "Not nerfing %d count %s maker (triggers '%s')\n",
				m_cNumMonsters, STRING(m_iszMonsterClassname), STRING(target));
			return false;
		}
	}

	if (shouldNerf) {
		ent = NULL;
		int reducedCount = m_cNumMonsters - maxNerfedSpawnCount;
		while (!FNullEnt(ent = FIND_ENTITY_BY_TARGETNAME(ent, STRING(target)))) {
			if (!strcmp(STRING(ent->v.classname), "game_counter")) {
				int trigCount = CountMonsterTriggerNerfs(target);
				if (trigCount == -1 || trigCount >= (int)ent->v.health) {
					ALERT(at_aiconsole, "Not nerfing %d count %s maker because game_counter '%s' would be nerfed into a negative count (%d > %d)\n",
						m_cNumMonsters, STRING(m_iszMonsterClassname), STRING(target), trigCount, (int)ent->v.health);
					return false;
				}

				ent->v.health -= reducedCount;
				ALERT(at_aiconsole, "Reduced game_counter %s limit by %d (%d total)\n",
					STRING(target), reducedCount, (int)ent->v.health);
			}
			else if (!strcmp(STRING(ent->v.classname), "trigger_counter")) {
				CBaseToggle* trig = (CBaseToggle*)GET_PRIVATE(ent);
				if (trig) {
					int trigCount = CountMonsterTriggerNerfs(target);
					if (trigCount == -1 || trigCount >= trig->m_cTriggersLeft) {
						ALERT(at_aiconsole, "Not nerfing %d count %s maker because trigger_counter '%s' would be nerfed into a negative count (%d > %d)\n",
							m_cNumMonsters, STRING(m_iszMonsterClassname), STRING(m_iszTriggerTarget), trigCount, trig->m_cTriggersLeft);
						return false;
					}

					trig->m_cTriggersLeft -= reducedCount;
					ALERT(at_aiconsole, "Reduced trigger_counter %s limit by %d (%d total)\n",
						STRING(target), reducedCount, (int)trig->m_cTriggersLeft);
				}
				else {
					return false;
				}
			}
		}
	}

	return true;
}

void CMonsterMaker::Nerf() {
	const char* spawnCname = STRING(m_iszMonsterClassname);

	if (strstr(spawnCname, "monster_") != spawnCname) {
		return; // doesn't spawn monsters
	}

	int clazz = Classify();

	if (CBaseEntity::IRelationship(CLASS_PLAYER, clazz) <= R_NO || !strcmp(spawnCname, "monster_snark")) {
		return; // don't care about nerfing friendlies/insects/snarks
	}

	float defaultHealth = GetDefaultHealth(spawnCname, false);
	float preNerfHealth = pev->health ? pev->health : defaultHealth;

	CBaseMonster::Nerf(); // reduce health

	int maxNerfedSpawnCount = mp_maxmonsterrespawns.value + 1.5f;
	bool spawnsTooMany = (m_cNumMonsters < 0 || m_cNumMonsters > maxNerfedSpawnCount);

	if (mp_maxmonsterrespawns.value >= 0 && spawnsTooMany) {
		bool shouldNerf = true;
		if (m_iszTriggerTarget && pev->target) {
			// not actually a problem, just lazy.
			// Need to be able to undo counter changes if one target fails to update
			ALERT(at_aiconsole, "Not nerfing %d count %s maker (complicated triggers)\n",
				m_cNumMonsters, STRING(m_iszMonsterClassname));
			shouldNerf = false;
		}
		else if (m_iszTriggerTarget) {
			shouldNerf = NerfMonsterCounters(m_iszTriggerTarget);
		}
		else if (pev->target) {
			shouldNerf = NerfMonsterCounters(pev->target);
		}
		else if (pev->spawnflags & (SF_MONSTERMAKER_PRISONER | SF_MONSTERMAKER_WAIT_SCRIPT)) {
			ALERT(at_aiconsole, "Not nerfing %d count %s maker (prisoner/scripted)\n",
				m_cNumMonsters, STRING(m_iszMonsterClassname));
			shouldNerf = false;
		}
		/*
		else if (pev->targetname) {
			// if the spawner can be turned off, then allow infinite spawns (svencooprpg)
			int neededTriggers = (pev->spawnflags & SF_MONSTERMAKER_START_ON) ? 1 : 2;
			int foundTriggers = 0;
			const char* foundRepeatTrigger = NULL;

			edict_t* edicts = ENT(0);
			for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++)
			{
				edict_t* ent = &edicts[i];
				if (ent->free)
					continue;

				const char* cname = STRING(ent->v.classname);
				CBaseEntity* pent = (CBaseEntity*)GET_PRIVATE(ent);
				if (!pent)
					continue;

				if (!pent->HasTarget(pev->targetname)) {
					continue;
				}
				
				if (!strcmp(cname, "trigger_once") || !strcmp(cname, "func_breakable")) {
					foundTriggers++;
				}
				else if (!strcmp(cname, "func_button")) {
					CBaseToggle* toggle = (CBaseToggle*)GET_PRIVATE(ent);
					if (toggle && toggle->m_flWait < 0) {
						foundTriggers++;
					}
					else {
						foundRepeatTrigger = cname;
						break;
					}
				}
				else {
					foundRepeatTrigger = cname;
					break;
				}
			}

			if (foundRepeatTrigger) {
				ALERT(at_aiconsole, "Not nerfing %d count %s maker (can be triggered on/off by '%s')\n",
					m_cNumMonsters, STRING(m_iszMonsterClassname), foundRepeatTrigger);
				shouldNerf = false;
			}
			else if (foundTriggers >= neededTriggers) {
				ALERT(at_aiconsole, "Not nerfing %d count %s maker (can be triggered %d times as '%s')\n",
					m_cNumMonsters, STRING(m_iszMonsterClassname), foundTriggers, STRING(pev->targetname));
				shouldNerf = false;
			}
		}
		*/

		if (shouldNerf) {
			ALERT(at_aiconsole, "Nerf %s maker '%s' count: %d -> %d\n", STRING(m_iszMonsterClassname),
				STRING(pev->targetname), m_cNumMonsters, maxNerfedSpawnCount);
			if (m_cNumMonsters < 0) {
				g_nerfStats.nerfedMonsterInfiniSpawns++;
			}
			else {
				g_nerfStats.nerfedMonsterSpawns += m_cNumMonsters - maxNerfedSpawnCount;
			}
			
			m_cNumMonsters = maxNerfedSpawnCount;
		}
		else {
			if (m_cNumMonsters > 0) {
				g_nerfStats.skippedMonsterSpawns += m_cNumMonsters - maxNerfedSpawnCount;
			}
		}
	}

	if (m_cNumMonsters > 0) {
		g_nerfStats.totalMonsters += m_cNumMonsters - 1; // CBaseMonster::Nerf increments this too
		g_nerfStats.totalMonsterHealth += (m_cNumMonsters - 1) * pev->health;

		if (preNerfHealth > pev->health)
			g_nerfStats.nerfedMonsterHealth += (m_cNumMonsters - 1) * (preNerfHealth - pev->health);

		if (pev->health > defaultHealth)
			g_nerfStats.skippedMonsterHealth += (m_cNumMonsters - 1) * (pev->health - defaultHealth);
	}
	else {
		g_nerfStats.skippedMonsterInfiniSpawns++;
	}
}
