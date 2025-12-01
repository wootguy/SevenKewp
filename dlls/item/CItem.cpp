#include "extdll.h"
#include "util.h"
#include "skill.h"
#include "CItem.h"
#include "gamerules.h"

void CItem::Spawn(void)
{
	UTIL_SetOrigin(pev, pev->origin);
	SetSize(Vector(-16, -16, 0), Vector(16, 16, 16));

	if (!(pev->spawnflags & SF_ITEM_USE_ONLY))
		SetTouch(&CItem::ItemTouch);
	
	if (!(pev->spawnflags & SF_ITEM_TOUCH_ONLY))
		SetUse(&CItem::ItemUse);

	// both "Touch only" and "Use only" set so it can't be collected. Assume "Use only" is what they wanted.
	if ((pev->spawnflags & (SF_ITEM_USE_ONLY|SF_ITEM_TOUCH_ONLY)) == (SF_ITEM_USE_ONLY | SF_ITEM_TOUCH_ONLY))
		SetUse(&CItem::ItemUse);

	if (!pev->movetype) {
		pev->movetype = MOVETYPE_TOSS;
	}
	else if (pev->movetype == -1 || pev->movetype == MOVETYPE_FLY) {
		pev->movetype = MOVETYPE_NOCLIP;
		pev->velocity.z = -FLT_MIN; // needed for interpolation if given avelocity
	}

	if (!pev->solid) {
		pev->solid = SOLID_TRIGGER;
	}
	else if (pev->solid == -1) {
		pev->solid = SOLID_NOT;
	}

	if (pev->movetype == MOVETYPE_TOSS) {
		// wait for solid entities to spawn
		SetThink(&CItem::DropThink);
		pev->nextthink = gpGlobals->time;
	}
	
	if (m_sequence_name && !pev->sequence) {
		pev->sequence = LookupSequence(STRING(m_sequence_name));
	}
	ResetSequenceInfo();

	AddWaterPhysicsEnt(this, 0.95f, 0);
}

void CItem::DropThink() {
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

	SetThink(NULL);
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
	else if (FStrEq(pkvd->szKeyName, "sequencename"))
	{
		m_sequence_name = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_flCustomRespawnTime"))
	{
		m_flCustomRespawnTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseAnimating::KeyValue(pkvd);
	}
}

void CItem::SetSize(Vector defaultMins, Vector defaultMaxs) {
	Vector min = m_minHullSize != g_vecZero ? m_minHullSize : defaultMins;
	Vector max = m_maxHullSize != g_vecZero ? m_maxHullSize : defaultMaxs;

	UTIL_SetSize(pev, min, max);
}

void CItem::SetItemModel() {
	if (pev->model || MergedModelBody() == -1) {
		SET_MODEL(ENT(pev), GetModel());
	}
	else {
		SET_MODEL_MERGED(ENT(pev), GetModel(), MergedModelBody());
	}
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

	if (pev->effects & EF_NODRAW) {
		return; // waiting to respawn
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

		if (!(pev->spawnflags & SF_ITEM_USE_WITHOUT_LOS) && !CanReach(pCaller)) {
			return;
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
	if (pev->model) {
		return STRING(pev->model);
	}

	return mp_mergemodels.value && MergedModelBody() != -1 ? MERGED_ITEMS_MODEL : m_defaultModel;
}

int	CItem::ObjectCaps(void) {
	if (pev->effects & EF_NODRAW) {
		return CBaseEntity::ObjectCaps();
	}
	else {
		return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
	}
}