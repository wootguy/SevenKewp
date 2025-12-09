#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "CBreakable.h"
#include "decals.h"
#include "explode.h"
#include "CBaseMonster.h"
#include "monsters.h"

// =================== FUNC_Breakable ==============================================

// Just add more items to the bottom of this array and they will automagically be supported
// This is done instead of just a classname in the FGD so we can control which entities can
// be spawned, and still remain fairly flexible
const char* CBreakable::pSpawnObjects[] =
{
	NULL,				// 0
	"item_battery",		// 1
	"item_healthkit",	// 2
	"weapon_9mmhandgun",// 3
	"ammo_9mmclip",		// 4
	"weapon_9mmAR",		// 5
	"ammo_9mmAR",		// 6
	"ammo_ARgrenades",	// 7
	"weapon_shotgun",	// 8
	"ammo_buckshot",	// 9
	"weapon_crossbow",	// 10
	"ammo_crossbow",	// 11
	"weapon_357",		// 12
	"ammo_357",			// 13
	"weapon_rpg",		// 14
	"ammo_rpgclip",		// 15
	"ammo_gaussclip",	// 16
	"weapon_handgrenade",// 17
	"weapon_tripmine",	// 18
	"weapon_satchel",	// 19
	"weapon_snark",		// 20
	"weapon_hornetgun",	// 21
	"weapon_crowbar",	// 22
	"weapon_pipewrench",// 23
	"weapon_sniperrifle",// 24
	"ammo_762",			// 25
	"weapon_m16",		// 26
	"weapon_saw",		// 27
	"weapon_minigun",	// 28
	"ammo_556",			// 29
	"weapon_sporelauncher",	// 30
	"ammo_sporeclip",	// 31
	"ammo_9mmbox",		// 32
	"weapon_uzi",		// 33
	"weapon_uziakimbo",	// 34
	"weapon_eagle",		// 35
	"weapon_grapple",	// 36
	"weapon_medkit",	// 37
	"item_suit",		// 38
	"item_antidote",	// 39
};

string_t g_breakableSpawnRemap[MAX_BREAKABLE_OBJECT_SPAWN_TYPES];

void CBreakable::KeyValue(KeyValueData* pkvd)
{
	// UNDONE_WC: explicitly ignoring these fields, but they shouldn't be in the map file!
	if (FStrEq(pkvd->szKeyName, "deadmodel"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shards"))
	{
		//			m_iShards = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnobject"))
	{
		int object = atoi(pkvd->szValue);
		if (object > 0 && object < (int)ARRAYSIZE(pSpawnObjects)) {
			if (g_breakableSpawnRemap[object]) {
				m_iszSpawnObject = g_breakableSpawnRemap[object];
			}
			else {
				m_iszSpawnObject = MAKE_STRING(pSpawnObjects[object]);
			}
		}

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lip")) {
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}


//
// func_breakable - bmodel that breaks into pieces after taking damage
//
LINK_ENTITY_TO_CLASS(func_breakable, CBreakable)
TYPEDESCRIPTION CBreakable::m_SaveData[] =
{
	DEFINE_FIELD(CBreakable, m_breakMaterial, FIELD_INTEGER),

	// Don't need to save/restore these because we precache after restore
	//	DEFINE_FIELD( CBreakable, m_idShard, FIELD_INTEGER ),

		DEFINE_FIELD(CBreakable, m_angle, FIELD_FLOAT),
		DEFINE_FIELD(CBreakable, m_iszSpawnObject, FIELD_STRING),

		// Explosion magnitude is stored in pev->impulse
};

IMPLEMENT_SAVERESTORE(CBreakable, CBaseEntity)

void CBreakable::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	// translate func_breakable flags into common breakable flags
	m_breakFlags |= FL_BREAK_IS_BREAKABLE | FL_BREAK_CAN_TRIGGER;
	if (pev->spawnflags & SF_BREAK_TRIGGER_ONLY)
		m_breakFlags |= FL_BREAK_TRIGGER_ONLY;
	if (pev->spawnflags & SF_BREAK_INSTANT)
		m_breakFlags |= FL_BREAK_INSTANT;
	if (pev->spawnflags & SF_BREAK_EXPLOSIVES_ONLY)
		m_breakFlags |= FL_BREAK_EXPLOSIVES_ONLY;
	if (pev->spawnflags & SF_BREAK_REPAIRABLE)
		m_breakFlags |= FL_BREAK_REPAIRABLE;

	CBaseEntity::Spawn();
	Precache();

	if (FBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))
		pev->takedamage = DAMAGE_NO;
	else
		pev->takedamage = DAMAGE_YES;

	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	m_angle = pev->angles.y;
	pev->angles.y = 0;
	pev->max_health = pev->health;

	// HACK:  matGlass can receive decals, we need the client to know about this
	//  so use class to store the material flag
	if (m_breakMaterial == matGlass)
	{
		pev->playerclass = 1;
	}

	SET_MODEL(ENT(pev), STRING(pev->model));//set size and link into world.

	SetTouch(&CBreakable::BreakTouch);
	if (FBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))		// Only break on trigger
		SetTouch(NULL);
	else {
		pev->flags |= FL_POSSIBLE_TARGET;
	}

	// Flag unbreakable glass as "worldbrush" so it will block ALL tracelines
	if (!IsBreakable() && pev->rendermode != kRenderNormal)
		pev->flags |= FL_WORLDBRUSH;
}

void CBreakable::Precache(void)
{
	// Precache the spawn item's data
	if (m_iszSpawnObject)
		UTIL_PrecacheOther((char*)STRING(m_iszSpawnObject));
}

// play shard sound when func_breakable takes damage.
// the more damage, the louder the shard sound.

void CBreakable::BreakTouch(CBaseEntity* pOther)
{
	float flDamage;
	entvars_t* pevToucher = pOther->pev;

	// only players can break these right now
	if (!pOther->IsPlayer() || !IsBreakable())
	{
		return;
	}

	if (FBitSet(pev->spawnflags, SF_BREAK_TOUCH))
	{// can be broken when run into 
		flDamage = pevToucher->velocity.Length() * 0.01;

		if (flDamage >= pev->health)
		{
			SetTouch(NULL);
			TakeDamage(pevToucher, pevToucher, flDamage, DMG_CRUSH);

			// do a little damage to player if we broke glass or computer
			pOther->TakeDamage(pev, pev, flDamage / 4, DMG_SLASH);
		}
	}

	if (FBitSet(pev->spawnflags, SF_BREAK_PRESSURE) && pevToucher->absmin.z >= pev->maxs.z - 2)
	{// can be broken when stood upon

		// play creaking sound here.
		BreakableDamageSound();

		SetTouch(NULL);
		BreakableDie(pOther);
	}
}

// Break when triggered
void CBreakable::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	BreakableUse(pActivator, pCaller, useType, value);
}


void CBreakable::BreakableDie(CBaseEntity* pActivator)
{
	CBaseEntity::BreakableDie(pActivator);

	// Fire targets on break (CBaseEntity doesn't handle trigger delay, so a separate target field is used)
	SUB_UseTargets(pActivator, USE_TOGGLE, 0);
	
	if (m_iszSpawnObject)
		CBaseEntity::Create((char*)STRING(m_iszSpawnObject), VecBModelOrigin(pev), pev->angles, true, edict());
}
