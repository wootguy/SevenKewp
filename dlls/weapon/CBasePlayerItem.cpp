#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "weapons.h"
#include "nodes.h"
#include "env/CSoundEnt.h"
#include "decals.h"
#include "gamerules.h"
#include "CBasePlayerItem.h"

extern int gEvilImpulse101;

TYPEDESCRIPTION	CBasePlayerItem::m_SaveData[] =
{
	DEFINE_FIELD(CBasePlayerItem, m_hPlayer, FIELD_EHANDLE),
	DEFINE_FIELD(CBasePlayerItem, m_pNext, FIELD_EHANDLE),
	//DEFINE_FIELD( CBasePlayerItem, m_fKnown, FIELD_INTEGER ),Reset to zero on load
	DEFINE_FIELD(CBasePlayerItem, m_iId, FIELD_INTEGER),
	// DEFINE_FIELD( CBasePlayerItem, m_iIdPrimary, FIELD_INTEGER ),
	// DEFINE_FIELD( CBasePlayerItem, m_iIdSecondary, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE(CBasePlayerItem, CBaseAnimating)

void CBasePlayerItem::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_flCustomRespawnTime"))
	{
		m_flCustomRespawnTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CBasePlayerItem::SetObjectCollisionBox(void)
{
	pev->absmin = pev->origin + Vector(-24, -24, 0);
	pev->absmax = pev->origin + Vector(24, 24, 16);
}

//=========================================================
// Sets up movetype, size, solidtype for a new weapon. 
//=========================================================
void CBasePlayerItem::FallInit(void)
{
	if (pev->movetype == 0)
		pev->movetype = MOVETYPE_TOSS;
	pev->solid = SOLID_TRIGGER;

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));//pointsize until it lands on the ground.

	if (!(pev->spawnflags & SF_ITEM_USE_ONLY))
		SetTouch(&CBasePlayerItem::DefaultTouch);

	if (!(pev->spawnflags & SF_ITEM_TOUCH_ONLY))
		SetUse(&CBasePlayerItem::DefaultUse);

	pev->nextthink = gpGlobals->time + 0.1;
}

//=========================================================
// FallThink - Items that have just spawned run this think
// to catch them when they hit the ground. Once we're sure
// that the object is grounded, we change its solid type
// to trigger and set it in a large box that helps the
// player get it.
//=========================================================
void CBasePlayerItem::FallThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->flags & FL_ONGROUND)
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if (!FNullEnt(pev->owner))
		{
			int pitch = 95 + RANDOM_LONG(0, 29);
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "items/weapondrop1.wav", 1, ATTN_NORM, 0, pitch);
		}

		// lie flat
		pev->angles.x = 0;
		pev->angles.z = 0;

		Materialize();
	}
	else if (!m_hPlayer)
	{
		SetThink(NULL);
		SetUse(NULL);
	}
}

//=========================================================
// Materialize - make a CBasePlayerItem visible and tangible
//=========================================================
void CBasePlayerItem::Materialize(void)
{
	if (pev->effects & EF_NODRAW)
	{
		// changing from invisible state to visible.
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM, 0, 150);
		pev->effects &= ~EF_NODRAW;
		pev->effects |= EF_MUZZLEFLASH;
	}

	pev->solid = SOLID_TRIGGER;

	UTIL_SetOrigin(pev, pev->origin);// link into world.

	if (!(pev->spawnflags & SF_ITEM_USE_ONLY))
		SetTouch(&CBasePlayerItem::DefaultTouch);

	if (!(pev->spawnflags & SF_ITEM_TOUCH_ONLY))
		SetUse(&CBasePlayerItem::DefaultUse);

}

//=========================================================
// AttemptToMaterialize - the item is trying to rematerialize,
// should it do so now or wait longer?
//=========================================================
void CBasePlayerItem::AttemptToMaterialize(void)
{
	float time = g_pGameRules->FlWeaponTryRespawn(this);

	if (time == 0)
	{
		Materialize();
		return;
	}

	pev->nextthink = gpGlobals->time + time;
}

//=========================================================
// CheckRespawn - a player is taking this weapon, should 
// it respawn?
//=========================================================
void CBasePlayerItem::CheckRespawn(void)
{
	switch (g_pGameRules->WeaponShouldRespawn(this))
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
}

//=========================================================
// Respawn- this item is already in the world, but it is
// invisible and intangible. Make it visible and tangible.
//=========================================================
CBaseEntity* CBasePlayerItem::Respawn(void)
{
	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity* pNewWeapon = CBaseEntity::Create((char*)STRING(pev->classname), g_pGameRules->VecWeaponRespawnSpot(this), pev->angles, pev->owner);

	if (pNewWeapon)
	{
		pNewWeapon->pev->effects |= EF_NODRAW;// invisible for now
		pNewWeapon->SetTouch(NULL);// no touch
		pNewWeapon->SetThink(&CBasePlayerItem::AttemptToMaterialize);

		DROP_TO_FLOOR(ENT(pev));

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->pev->nextthink = g_pGameRules->FlWeaponRespawnTime(this);
	}
	else
	{
		ALERT(at_console, "Respawn failed to create %s!\n", STRING(pev->classname));
	}

	return pNewWeapon;
}

void CBasePlayerItem::DefaultTouch(CBaseEntity* pOther)
{
	// if it's not a player, ignore
	if (!pOther->IsPlayer())
		return;

	if (pev->movetype == MOVETYPE_FOLLOW || m_hPlayer || (pev->effects & EF_NODRAW)) {
		return; // attached to a player
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;

	// can I have this?
	if (!g_pGameRules->CanHavePlayerItem(pPlayer, this))
	{
		if (gEvilImpulse101)
		{
			UTIL_Remove(this);
		}
		return;
	}

	if (!gEvilImpulse101) {
		if (!(iFlags() & ITEM_FLAG_LIMITINWORLD) && !(pev->spawnflags & SF_NORESPAWN)) {
			if (gpGlobals->time < pPlayer->m_nextItemPickups[m_iId]) {
				return;
			}

			pPlayer->m_nextItemPickups[m_iId] = gpGlobals->time + item_repick_time.value;
		}
	}

	if (pOther->AddPlayerItem(this))
	{
		AttachToPlayer(pPlayer);
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);
	}

	SUB_UseTargets(pOther, USE_TOGGLE, 0); // UNDONE: when should this happen?
}


void CBasePlayerItem::DestroyItem(void)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_pPlayer)
	{
		// if attached to a player, remove. 
		m_pPlayer->RemovePlayerItem(this);
	}

	Kill();
}

int CBasePlayerItem::AddToPlayer(CBasePlayer* pPlayer)
{
	m_hPlayer = pPlayer;

	return TRUE;
}

void CBasePlayerItem::Drop(void)
{
	SetTouch(NULL);
	SetUse(NULL);
	SetThink(&CBasePlayerItem::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Kill(void)
{
	SetTouch(NULL);
	SetUse(NULL);
	SetThink(&CBasePlayerItem::SUB_Remove);
	pev->nextthink = gpGlobals->time + .1;
}

void CBasePlayerItem::Holster(int skiplocal /* = 0 */)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_pPlayer->pev->viewmodel = 0;
	m_pPlayer->pev->weaponmodel = 0;
}

void CBasePlayerItem::AttachToPlayer(CBasePlayer* pPlayer)
{
	pev->movetype = MOVETYPE_FOLLOW;
	pev->solid = SOLID_NOT;
	pev->aiment = pPlayer->edict();
	pev->effects = EF_NODRAW; // ??
	pev->modelindex = 0;// server won't send down to clients if modelindex == 0
	pev->model = iStringNull;
	pev->owner = pPlayer->edict();
	pev->nextthink = gpGlobals->time + .1;
	SetTouch(NULL);
	SetUse(NULL);
}

void CBasePlayerItem::DefaultUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pCaller && pCaller->IsPlayer()) {

		if (!(pev->spawnflags & SF_ITEM_USE_WITHOUT_LOS) && !CanReach(pCaller)) {
			return;
		}

		DefaultTouch(pCaller);
	}
}

int CBasePlayerItem::ObjectCaps() {
	if (pev->effects == MOVETYPE_FOLLOW || (pev->effects & EF_NODRAW)) {
		return CBaseEntity::ObjectCaps();
	}
	else {
		return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
	}
}
