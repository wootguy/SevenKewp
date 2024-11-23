#include "CWeaponInventory.h"
#include "CItemInventory.h"

LINK_ENTITY_TO_CLASS(weapon_inventory, CWeaponInventory)

void CWeaponInventory::Spawn()
{
	Precache();
	m_iId = WEAPON_INVENTORY;
	SetWeaponModelW();
	m_iClip = -1;

	FallInit();// get ready to fall down.
}

void CWeaponInventory::Precache()
{
	m_defaultModelV = NOT_PRECACHED_MODEL;
	m_defaultModelP = NOT_PRECACHED_MODEL;
	m_defaultModelW = NOT_PRECACHED_MODEL;
	CBasePlayerWeapon::Precache();

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_inventory.txt");
}

BOOL CWeaponInventory::Deploy()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return FALSE;

	if (DefaultDeploy(GetModelV(), GetModelP(), 0, "trip")) {
		m_pPlayer->pev->weaponmodel = 0; // inventory item model is rendered separately
		m_pPlayer->pev->viewmodel = 0;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.0;
		return TRUE;
	}

	return FALSE;
}

void CWeaponInventory::Holster(int skiplocal)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	// clear the HUD
	UTIL_ClientPrint(m_pPlayer, print_center, "");

	CItemInventory* item = m_pPlayer->m_inventory ? m_pPlayer->m_inventory.GetEntity()->MyInventoryPointer() : NULL;

	while (item) {
		item->m_is_viewing = false;
		item = item->m_pNext ? item->m_pNext.GetEntity()->MyInventoryPointer() : NULL;
	}
}

void CWeaponInventory::WeaponIdle()
{
	UpdateInventoryHud();
}

void CWeaponInventory::PrimaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer || !m_pPlayer->m_inventory || m_nextAction > gpGlobals->time)
		return;

	CItemInventory* item = m_pPlayer->m_inventory.GetEntity()->MyInventoryPointer();
	CItemInventory* selectedItem = item;
	int idx = 0;

	while (item) {
		if (m_itemIdx == idx) {
			selectedItem = item;
			break;
		}
		idx++;
		item = item->m_pNext ? item->m_pNext.GetEntity()->MyInventoryPointer() : NULL;
	}

	int ret = selectedItem->ActivateItem();
	if (ret) {
		SetError(ret);
	}

	m_nextAction = gpGlobals->time + 0.5;
}

void CWeaponInventory::SecondaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer || !m_pPlayer->m_inventory || m_nextAction > gpGlobals->time)
		return;

	m_nextAction = gpGlobals->time + 0.5;

	CItemInventory* item = m_pPlayer->m_inventory.GetEntity()->MyInventoryPointer();
	int idx = 0;

	while (item) {
		if (m_itemIdx == idx) {
			if (item->m_holder_can_drop) {
				item->Detach(true);
				m_pPlayer->SetAnimation(PLAYER_DROP_ITEM);
			}
			else {
				SetError(INV_ERROR_CANT_DROP);
				item->FireInvTargets(m_pPlayer, item->m_target_cant_drop);
			}
			
			break;
		}

		idx++;
		item = item->m_pNext ? item->m_pNext.GetEntity()->MyInventoryPointer() : NULL;
	}

	if (m_pPlayer->CountInventoryItems() == 0) {
		// removed last item, so get rid of the inventory browser
		m_pPlayer->RemovePlayerItem(this);
		g_pGameRules->GetNextBestWeapon(m_pPlayer, this);
	}
	else {
		m_itemIdx = m_itemIdx % m_pPlayer->CountInventoryItems();
	}
}

void CWeaponInventory::Reload(void)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_pPlayer->m_afButtonLast & IN_RELOAD) {
		return;
	}

	m_itemIdx = (m_itemIdx + 1) % m_pPlayer->CountInventoryItems();

	UpdateInventoryHud();
}

void CWeaponInventory::SetError(int errorCode) {
	m_errorCode = errorCode;
	m_errorText = 0;
	m_errorTime = g_engfuncs.pfnTime();
}

void CWeaponInventory::SetError(const char* errorText) {
	m_errorText = ALLOC_STRING((std::string("\n\n") + errorText).c_str());
	m_errorTime = g_engfuncs.pfnTime();
}

void CWeaponInventory::UpdateInventoryHud() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer || !m_pPlayer->m_inventory)
		return;

	CItemInventory* item = m_pPlayer->m_inventory.GetEntity()->MyInventoryPointer();
	CItemInventory* selectedItem = item;
	int idx = 0;

	while (item) {
		item->m_is_viewing = false;

		if (m_itemIdx == idx) {
			selectedItem = item;
		}
		idx++;
		item = item->m_pNext ? item->m_pNext.GetEntity()->MyInventoryPointer() : NULL;
	}

	// only put items in your face if you have the option to switch to a different weapon
	if (!m_pPlayer->m_weaponsDisabled)
		selectedItem->m_is_viewing = true;

	bool canActivate = selectedItem->m_holder_can_activate && !selectedItem->m_is_active;
	const char* desc = selectedItem->m_description ? STRING(selectedItem->m_description) : "";
	const char* help = canActivate ? "\n[PRIMARY] = Activate" : "";
	const char* help2 = selectedItem->m_holder_can_drop ? "\n[SECONDARY] = Drop" : "";
	const char* help3 = idx > 1 ? "\n[RELOAD] = Next Item" : "";

	const char* error = "";

	if (g_engfuncs.pfnTime() - m_errorTime < 3.0f) {
		switch (m_errorCode) {
		case INV_ERROR_CANT_ACTIVATE_EVER:
			error = "\n\nThis item can't be activated";
			break;
		case INV_ERROR_CANT_ACTIVATE_ACTIVE:
			error = "\n\nThis item is already active";
			break;
		case INV_ERROR_CANT_ACTIVATE_COOLDOWN:
			error = "\n\nThis item was activated too recently";
			break;
		case INV_ERROR_CANT_ACTIVATE_LIMIT:
			error = "\n\nItem has been activated too many times";
			break;
		case INV_ERROR_CANT_DROP:
			error = "\n\nThis item can't be dropped";
			break;
		case INV_ERROR_ACTIVATED:
			error = "\n\nActivated!";
			break;
		default:
			break;
		}
		if (m_errorText) {
			error = STRING(m_errorText);
		}
	}
	std::string position = idx > 1 ? UTIL_VarArgs(" (%d / %d)", m_itemIdx + 1, idx) : "";
	const char* hudText = UTIL_VarArgs("Your Inventory%s:\n\n%s\n%s\n%s%s%s%s",
		position.c_str(), selectedItem->DisplayName(), desc, help, help2, help3, error);

	if (!strcmp(hudText, m_previousHudText) && g_engfuncs.pfnTime() - m_lastHudUpdate < 1.0f) {
		return;
	}

	strcpy_safe(m_previousHudText, hudText, 512);
	m_lastHudUpdate = g_engfuncs.pfnTime();

	UTIL_ClientPrint(m_pPlayer, print_center, hudText);
}

int CWeaponInventory::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_inventory";

	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 1;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = WEAPON_INVENTORY;
	p->iWeight = INVENTORY_WEIGHT;
	return 1;
}
