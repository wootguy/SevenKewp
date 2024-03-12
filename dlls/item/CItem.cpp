#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

#define SF_ITEM_TOUCH_ONLY 128 // Pick this item up only by touching it.
#define SF_ITEM_USE_ONLY 256 // Pick this item up only by using it ('USE' key).
#define SF_ITEM_USE_WITHOUT_LOS 512 // Player can pick up this item even when it's not within his line of sight.

void CItem::Spawn(void)
{
	UTIL_SetOrigin(pev, pev->origin);
	SetSize(Vector(-16, -16, 0), Vector(16, 16, 16));

	if (!(pev->spawnflags & SF_ITEM_USE_ONLY))
		SetTouch(&CItem::ItemTouch);
	
	if (!(pev->spawnflags & SF_ITEM_TOUCH_ONLY))
		SetUse(&CItem::ItemUse);

	if (!pev->movetype) {
		pev->movetype = MOVETYPE_TOSS;
	}
	else if (pev->movetype == -1) {
		pev->movetype = MOVETYPE_NONE;
	}

	if (!pev->solid) {
		pev->solid = SOLID_TRIGGER;
	}
	else if (pev->solid == -1) {
		pev->solid = SOLID_NOT;
	}

	if (pev->movetype == MOVETYPE_TOSS) {
		int dropResult = DROP_TO_FLOOR(ENT(pev));

		if (dropResult == 0)
		{
			ALERT(at_warning, "Item %s fell out of level at %f,%f,%f\n", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
			UTIL_Remove(this);
			return;
		}
		else if (dropResult == -1) {
			ALERT(at_warning, "Item %s spawned inside solid at %f,%f,%f\n", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
		}
	}
	
}

void CItem::KeyValue(KeyValueData* pkvd) {
	if (FStrEq(pkvd->szKeyName, "minhullsize"))
	{
		UTIL_StringToVector(m_minHullSize, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "maxhullsize"))
	{
		UTIL_StringToVector(m_maxHullSize, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue(pkvd);
	}
}

void CItem::SetSize(Vector defaultMins, Vector defaultMaxs) {
	Vector min = m_minHullSize != g_vecZero ? m_minHullSize : defaultMins;
	Vector max = m_maxHullSize != g_vecZero ? m_maxHullSize : defaultMaxs;

	UTIL_SetSize(pev, min, max);
}


extern int gEvilImpulse101;

BOOL CItem::ShouldRespawn() {
	return g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_YES;
}

void CItem::ItemTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
	{
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// ok, a player is touching this item, but can he have it?
	if (!g_pGameRules->CanHaveItem(pPlayer, this))
	{
		// no? Ignore the touch.
		return;
	}

	if (MyTouch(pPlayer))
	{
		SUB_UseTargets(pOther, USE_TOGGLE, 0);
		SetTouch(NULL);

		// player grabbed the item. 
		g_pGameRules->PlayerGotItem(pPlayer, this);
		if (ShouldRespawn())
		{
			Respawn();
		}
		else
		{
			UTIL_Remove(this);
		}
	}
	else if (gEvilImpulse101)
	{
		UTIL_Remove(this);
	}
}

void CItem::ItemUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pCaller && pCaller->IsPlayer()) {

		if (!(pev->spawnflags & SF_ITEM_USE_WITHOUT_LOS)) {
			TraceResult tr;
			TRACE_LINE(pCaller->pev->origin + pCaller->pev->view_ofs, pev->origin, dont_ignore_monsters, pCaller->edict(), &tr);
			
			bool hitItemSurface = tr.pHit && tr.pHit != edict();
			bool enteredItemBox = boxesIntersect(pev->absmin, pev->absmax, tr.vecEndPos, tr.vecEndPos);
			if (!hitItemSurface && !enteredItemBox) {
				ALERT(at_console, "Can't use item not in LOS\n");
				return;
			}
		}

		ItemTouch(pCaller);
	}
}

CBaseEntity* CItem::Respawn(void)
{
	SetTouch(NULL);
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin(pev, g_pGameRules->VecItemRespawnSpot(this));// blip to whereever you should respawn.

	SetThink(&CItem::Materialize);
	pev->nextthink = g_pGameRules->FlItemRespawnTime(this);
	return this;
}

void CItem::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	SetTouch(&CItem::ItemTouch);
}

const char* CItem::GetModel() {
	return pev->model ? STRING(pev->model) : m_defaultModel;
}
