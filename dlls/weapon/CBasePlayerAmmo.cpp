#include "extdll.h"
#include "util.h"
#include "nodes.h"
#include "env/CSoundEnt.h"
#include "decals.h"
#include "gamerules.h"
#include "CBasePlayerAmmo.h"
#include "weapons.h"
#include "CBasePlayer.h"

extern int gEvilImpulse101;

void CBasePlayerAmmo::Spawn(void)
{
	pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	UTIL_SetOrigin(pev, pev->origin);

	if (!(pev->spawnflags & SF_ITEM_USE_ONLY))
		SetTouch(&CBasePlayerAmmo::DefaultTouch);

	if (!(pev->spawnflags & SF_ITEM_TOUCH_ONLY))
		SetUse(&CBasePlayerAmmo::DefaultUse);	

	UTIL_RegisterEquipmentEntity(STRING(pev->classname));
}

void CBasePlayerAmmo::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_flCustomRespawnTime"))
	{
		m_flCustomRespawnTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

CBaseEntity* CBasePlayerAmmo::Respawn(void)
{
	pev->effects |= EF_NODRAW;
	SetTouch(NULL);

	UTIL_SetOrigin(pev, g_pGameRules->VecAmmoRespawnSpot(this));// move to wherever I'm supposed to repawn.

	if (mp_one_pickup_per_player.value) {
		Materialize();
	}
	else {
		SetThink(&CBasePlayerAmmo::Materialize);
		pev->nextthink = g_pGameRules->FlAmmoRespawnTime(this);
	}

	return this;
}

void CBasePlayerAmmo::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		if (!mp_one_pickup_per_player.value) {
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
			pev->effects |= EF_MUZZLEFLASH;
		}
		pev->effects &= ~EF_NODRAW;
	}

	if (!(pev->spawnflags & SF_ITEM_USE_ONLY))
		SetTouch(&CBasePlayerAmmo::DefaultTouch);

	if (!(pev->spawnflags & SF_ITEM_TOUCH_ONLY))
		SetUse(&CBasePlayerAmmo::DefaultUse);
}

void CBasePlayerAmmo::DefaultTouch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{
		return;
	}

	if (pev->effects & EF_NODRAW) {
		return; // waiting to respawn
	}

	if (mp_one_pickup_per_player.value && (m_pickupPlayers & PLRBIT(pOther->edict()))) {
		return;
	}

	if (AddAmmo(pOther))
	{
		m_pickupPlayers |= PLRBIT(pOther->edict());

		if (g_pGameRules->AmmoShouldRespawn(this) == GR_AMMO_RESPAWN_YES)
		{
			Respawn();
		}
		else
		{
			SetTouch(NULL);
			SetThink(&CBasePlayerAmmo::SUB_Remove);
			pev->nextthink = gpGlobals->time + .1;
		}
	}
	else if (gEvilImpulse101)
	{
		// evil impulse 101 hack, kill always
		SetTouch(NULL);
		SetThink(&CBasePlayerAmmo::SUB_Remove);
		pev->nextthink = gpGlobals->time + .1;
	}
}

void CBasePlayerAmmo::DefaultUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pCaller && pCaller->IsPlayer()) {

		if (!(pev->spawnflags & SF_ITEM_USE_WITHOUT_LOS) && !CanReach(pCaller)) {
			return;
		}

		if (pActivator->IsPlayer() && mp_one_pickup_per_player.value && (m_pickupPlayers & PLRBIT(pActivator->edict()))) {
			UTIL_ClientPrint(pActivator, print_center, "Can't collect again until you respawn\n");
		}

		DefaultTouch(pCaller);
	}
}

int	CBasePlayerAmmo::ObjectCaps(void) {
	if (pev->effects & EF_NODRAW) {
		return CBaseEntity::ObjectCaps();
	}
	else {
		return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
	}
}

int CBasePlayerAmmo::AddToFullPack(struct entity_state_s* state, CBasePlayer* player) {
	if (mp_one_pickup_per_player.value && (m_pickupPlayers & PLRBIT(player->edict()))) {
		state->rendermode = kRenderTransAlpha;
		state->renderamt = 40;
	}

	return 1;
}