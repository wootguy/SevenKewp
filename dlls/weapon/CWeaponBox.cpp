#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "weapons.h"
#include "nodes.h"
#include "env/CSoundEnt.h"
#include "decals.h"
#include "gamerules.h"
#include "CWeaponBox.h"
#include "CBasePlayerItem.h"

LINK_ENTITY_TO_CLASS(weaponbox, CWeaponBox)

TYPEDESCRIPTION	CWeaponBox::m_SaveData[] =
{
	DEFINE_ARRAY(CWeaponBox, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_SLOTS),
	DEFINE_ARRAY(CWeaponBox, m_rgiszAmmo, FIELD_STRING, MAX_AMMO_SLOTS),
	DEFINE_ARRAY(CWeaponBox, m_rghPlayerItems, FIELD_EHANDLE, MAX_ITEM_TYPES),
	DEFINE_FIELD(CWeaponBox, m_cAmmoTypes, FIELD_INTEGER),
};
IMPLEMENT_SAVERESTORE(CWeaponBox, CBaseEntity)

//=========================================================
//
//=========================================================
void CWeaponBox::Precache(void)
{
	PRECACHE_REPLACEMENT_MODEL("models/w_weaponbox.mdl");
}

//=========================================================
//=========================================================
void CWeaponBox::KeyValue(KeyValueData* pkvd)
{
	if (m_cAmmoTypes < MAX_AMMO_SLOTS)
	{
		PackAmmo(ALLOC_STRING(pkvd->szKeyName), atoi(pkvd->szValue));
		m_cAmmoTypes++;// count this new ammo type.

		pkvd->fHandled = TRUE;
	}
	else
	{
		ALERT(at_console, "WeaponBox too full! only %d ammotypes allowed\n", MAX_AMMO_SLOTS);
	}
}

//=========================================================
// CWeaponBox - Spawn 
//=========================================================
void CWeaponBox::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_TRIGGER;

	m_spawnTime = gpGlobals->time;

	UTIL_SetSize(pev, g_vecZero, g_vecZero);

	SET_MODEL_MERGED(ENT(pev), "models/w_weaponbox.mdl", MERGE_MDL_W_WEAPONBOX);

	SetTouch(&CWeaponBox::DefaultTouch);

	AddWaterPhysicsEnt(this, 0.95f, 0);
}

//=========================================================
// CWeaponBox - Kill - the think function that removes the
// box from the world.
//=========================================================
void CWeaponBox::Kill(void)
{
	CBasePlayerItem* pWeapon;
	int i;

	// destroy the weapons
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		pWeapon = (CBasePlayerItem*)m_rghPlayerItems[i].GetEntity();

		while (pWeapon)
		{
			pWeapon->SetThink(&CBasePlayerItem::SUB_Remove);
			pWeapon->pev->nextthink = gpGlobals->time + 0.1;
			pWeapon = (CBasePlayerItem*)pWeapon->m_pNext.GetEntity();
		}
	}

	// remove the box
	UTIL_Remove(this);
}

//=========================================================
// CWeaponBox - Touch: try to add my contents to the toucher
// if the toucher is a player.
//=========================================================
void CWeaponBox::DefaultTouch(CBaseEntity* pOther)
{
	ItemBounceTouch(pOther);

	if (!pOther->IsPlayer())
	{
		// only players may touch a weaponbox.
		return;
	}

	if (!pOther->IsAlive())
	{
		// no dead guys.
		return;
	}

	CBasePlayer* pPlayer = (CBasePlayer*)pOther;
	int i;

	if (pev->owner && ENTINDEX(pOther->edict()) == ENTINDEX(pev->owner)) {
		// owner picking the weapon back up
		if (gpGlobals->time - m_spawnTime < 0.5f) {
			return; // don't instantly pick up item again while it's being thrown
		}
	}

	// dole out ammo
	for (i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		if (!FStringNull(m_rgiszAmmo[i]))
		{
			// there's some ammo of this type. 
			pPlayer->GiveAmmo(m_rgAmmo[i], STRING(m_rgiszAmmo[i]), MaxAmmoCarry(m_rgiszAmmo[i]));

			//ALERT ( at_console, "Gave %d rounds of %s\n", m_rgAmmo[i], STRING(m_rgiszAmmo[i]) );

			// now empty the ammo from the weaponbox since we just gave it to the player
			m_rgiszAmmo[i] = iStringNull;
			m_rgAmmo[i] = 0;
		}
	}

	bool hadWeapon = false;

	// go through my weapons and try to give the usable ones to the player. 
	// it's important the the player be given ammo first, so the weapons code doesn't refuse 
	// to deploy a better weapon that the player may pick up because he has no ammo for it.
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rghPlayerItems[i])
		{
			CBasePlayerItem* pItem;

			// have at least one weapon in this slot
			while (m_rghPlayerItems[i])
			{
				//ALERT ( at_console, "trying to give %s\n", STRING( m_rgpPlayerItems[ i ]->pev->classname ) );

				CBaseEntity* ent = m_rghPlayerItems[i].GetEntity();
				pItem = ent ? ent->GetWeaponPtr() : NULL;

				if (pItem) {
					m_rghPlayerItems[i] = pItem->m_pNext.GetEntity();// unlink this weapon from the box
					hadWeapon = true;

					if (pPlayer->AddPlayerItem(pItem))
					{
						pItem->AttachToPlayer(pPlayer);
					}
				}
			}
		}
	}

	EMIT_SOUND(pOther->edict(), CHAN_ITEM, hadWeapon ? "items/gunpickup2.wav" : "items/9mmclip1.wav", 1, ATTN_NORM);
	SetTouch(NULL);
	SetUse(NULL);
	UTIL_Remove(this);
}

void CWeaponBox::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pCaller && pCaller->IsPlayer() && CanReach(pCaller)) {
		DefaultTouch(pCaller);
	}
}

//=========================================================
// CWeaponBox - PackWeapon: Add this weapon to the box
//=========================================================
BOOL CWeaponBox::PackWeapon(CBasePlayerItem* pWeapon)
{
	CBasePlayer* plr = (CBasePlayer*)pWeapon->m_hPlayer.GetEntity();

	// is one of these weapons already packed in this box?
	if (HasWeapon(pWeapon))
	{
		return FALSE;// box can only hold one of each weapon type
	}

	if (plr)
	{
		if (!plr->RemovePlayerItem(pWeapon))
		{
			// failed to unhook the weapon from the player!
			return FALSE;
		}
	}

	int iWeaponSlot = pWeapon->iItemSlot();

	if (m_rghPlayerItems[iWeaponSlot])
	{
		// there's already one weapon in this slot, so link this into the slot's column
		pWeapon->m_pNext = m_rghPlayerItems[iWeaponSlot].GetEntity();
		m_rghPlayerItems[iWeaponSlot] = pWeapon;
	}
	else
	{
		// first weapon we have for this slot
		m_rghPlayerItems[iWeaponSlot] = pWeapon;
		pWeapon->m_pNext = NULL;
	}

	pWeapon->pev->spawnflags |= SF_NORESPAWN;// never respawn
	pWeapon->pev->movetype = MOVETYPE_NONE;
	pWeapon->pev->solid = SOLID_NOT;
	pWeapon->pev->effects = EF_NODRAW;
	pWeapon->pev->modelindex = 0;
	pWeapon->pev->model = iStringNull;
	pWeapon->pev->owner = edict();
	pWeapon->SetThink(NULL);// crowbar may be trying to swing again, etc.
	pWeapon->SetTouch(NULL);
	pWeapon->m_hPlayer = NULL;

	//ALERT ( at_console, "packed %s\n", STRING(pWeapon->pev->classname) );

	return TRUE;
}

//=========================================================
// CWeaponBox - PackAmmo
//=========================================================
BOOL CWeaponBox::PackAmmo(int iszName, int iCount)
{
	int iMaxCarry;

	if (FStringNull(iszName))
	{
		// error here
		ALERT(at_console, "NULL String in PackAmmo!\n");
		return FALSE;
	}

	iMaxCarry = MaxAmmoCarry(iszName);

	if (iMaxCarry != -1 && iCount > 0)
	{
		//ALERT ( at_console, "Packed %d rounds of %s\n", iCount, STRING(iszName) );
		GiveAmmo(iCount, STRING(iszName), iMaxCarry);
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// CWeaponBox - GiveAmmo
//=========================================================
int CWeaponBox::GiveAmmo(int iCount, const char* szName, int iMax, int* pIndex/* = NULL*/)
{
	int i;

	for (i = 1; i < MAX_AMMO_SLOTS && !FStringNull(m_rgiszAmmo[i]); i++)
	{
		if (stricmp(szName, STRING(m_rgiszAmmo[i])) == 0)
		{
			if (pIndex)
				*pIndex = i;

			int iAdd = V_min(iCount, iMax - m_rgAmmo[i]);
			if (iCount == 0 || iAdd > 0)
			{
				m_rgAmmo[i] += iAdd;

				return i;
			}
			return -1;
		}
	}
	if (i < MAX_AMMO_SLOTS)
	{
		if (pIndex)
			*pIndex = i;

		m_rgiszAmmo[i] = MAKE_STRING(szName);
		m_rgAmmo[i] = iCount;

		return i;
	}
	ALERT(at_console, "out of named ammo slots\n");
	return i;
}

//=========================================================
// CWeaponBox::HasWeapon - is a weapon of this type already
// packed in this box?
//=========================================================
BOOL CWeaponBox::HasWeapon(CBasePlayerItem* pCheckItem)
{
	CBasePlayerItem* pItem = (CBasePlayerItem*)m_rghPlayerItems[pCheckItem->iItemSlot()].GetEntity();

	while (pItem)
	{
		if (FClassnameIs(pItem->pev, STRING(pCheckItem->pev->classname)))
		{
			return TRUE;
		}
		pItem = (CBasePlayerItem*)pItem->m_pNext.GetEntity();
	}

	return FALSE;
}

//=========================================================
// CWeaponBox::IsEmpty - is there anything in this box?
//=========================================================
BOOL CWeaponBox::IsEmpty(void)
{
	int i;

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rghPlayerItems[i])
		{
			return FALSE;
		}
	}

	for (i = 0; i < MAX_AMMO_SLOTS; i++)
	{
		if (!FStringNull(m_rgiszAmmo[i]))
		{
			// still have a bit of this type of ammo
			return FALSE;
		}
	}

	return TRUE;
}

//=========================================================
//=========================================================
void CWeaponBox::SetObjectCollisionBox(void)
{
	pev->absmin = pev->origin + Vector(-16, -16, 0);
	pev->absmax = pev->origin + Vector(16, 16, 16);
}

bool CWeaponBox::IsUseOnlyWeapon() {
	bool foundUseOnlyWeapon = false;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rghPlayerItems[i])
		{
			CBasePlayerItem* pItem = m_rghPlayerItems[i]->GetWeaponPtr();

			// have at least one weapon in this slot
			while (pItem)
			{
				CWeaponCustom* cwep = pItem ? pItem->MyWeaponCustomPtr() : NULL;

				if (cwep && (cwep->params.flags & FL_WC_WEP_USE_ONLY)) {
					if (foundUseOnlyWeapon)
						return false;
					foundUseOnlyWeapon = true;
				}
				else {
					return false;
				}

				CBaseEntity* next = pItem->m_pNext.GetEntity();
				pItem = next ? next->GetWeaponPtr() : NULL;
			}
		}
	}

	return foundUseOnlyWeapon;
}
