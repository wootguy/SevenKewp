#include "hud.h"
#include "cl_util.h"
#include "ammohistory.h"
#include "com_weapons.h"
#include "weapon_res.h"

StringMap g_weaponHudDirs;
WeaponsResource gWR;
WEAPON* g_pActiveWeapon;

/* =================================
	GetSpriteList

Finds and returns the matching
sprite name 'psz' and resolution 'iRes'
in the given sprite list 'pList'
iCount is the number of items in the pList
================================= */
client_sprite_t* GetSpriteList(client_sprite_t* pList, const char* psz, int iRes, int iCount)
{
	if (!pList)
		return NULL;

	int i = iCount;
	client_sprite_t* p = pList;

	while (i--)
	{
		if ((p->iRes == iRes) && (!strcmp(psz, p->szName)))
			return p;
		p++;
	}

	return NULL;
}

void WeaponsResource::LoadAllWeaponSprites(void)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (rgWeapons[i].iId)
			LoadWeaponSprites(&rgWeapons[i]);
	}
}

int WeaponsResource::CountAmmo(int iId)
{
	if (iId < 0)
		return 0;

	return riAmmo[iId];
}

int WeaponsResource::HasAmmo(WEAPON* p)
{
	if (!p)
		return FALSE;

	// weapons with no max ammo can always be selected
	if (p->iMax1 == -1)
		return TRUE;

	return (p->iAmmoType == -1) || p->iClip > 0 || CountAmmo(p->iAmmoType)
		|| CountAmmo(p->iAmmo2Type);// || (p->iFlags & WEAPON_FLAGS_SELECTONEMPTY);
}


void WeaponsResource::LoadWeaponSprites(WEAPON* pWeapon)
{
	int i;

	int iRes = gHUD.GetDesiredSpriteRes();

#ifdef VANILLA_HL
	char sz[128];
#else
	char sz[144];
#endif

	if (gHUD.m_pCvarHudScale->value > 0) {
		int nScale = clamp(gHUD.m_pCvarHudScale->value, 1, 4);
	}

	if (!pWeapon)
		return;

	memset(&pWeapon->rcActive, 0, sizeof(wrect_t));
	memset(&pWeapon->rcInactive, 0, sizeof(wrect_t));
	memset(&pWeapon->rcAmmo, 0, sizeof(wrect_t));
	memset(&pWeapon->rcAmmo2, 0, sizeof(wrect_t));
	pWeapon->hInactive = 0;
	pWeapon->hActive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	const char* customDir = g_weaponHudDirs.get(pWeapon->szName);

	if (customDir && customDir[0]) {
		const char* wepName = pWeapon->szName;
		const char* last = strrchr(pWeapon->szName, '/');
		if (last) {
			wepName = last + 1; // strip folder path from non-default weapons (hlcoop/ hack)
		}
		snprintf(sz, sizeof(sz), "sprites/%s/%s.txt", customDir, wepName);
	}
	else {
		snprintf(sz, sizeof(sz), "sprites/%s.txt", pWeapon->szName);
	}

	client_sprite_t* pList = SPR_GetList(sz, &i);

	if (!pList)
		return;

	client_sprite_t* p;

	p = GetSpriteList(pList, "crosshair", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hCrosshair = SPR_Load(sz);
		pWeapon->rcCrosshair = p->rc;
	}
	else
		pWeapon->hCrosshair = (int)NULL;

	p = GetSpriteList(pList, "autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAutoaim = SPR_Load(sz);
		pWeapon->rcAutoaim = p->rc;
	}
	else
		pWeapon->hAutoaim = 0;

	p = GetSpriteList(pList, "zoom", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedCrosshair = SPR_Load(sz);
		pWeapon->rcZoomedCrosshair = p->rc;
	}
	else
	{
		pWeapon->hZoomedCrosshair = pWeapon->hCrosshair; //default to non-zoomed crosshair
		pWeapon->rcZoomedCrosshair = pWeapon->rcCrosshair;
	}

	p = GetSpriteList(pList, "zoom_autoaim", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hZoomedAutoaim = SPR_Load(sz);
		pWeapon->rcZoomedAutoaim = p->rc;
	}
	else
	{
		pWeapon->hZoomedAutoaim = pWeapon->hZoomedCrosshair;  //default to zoomed crosshair
		pWeapon->rcZoomedAutoaim = pWeapon->rcZoomedCrosshair;
	}

	p = GetSpriteList(pList, "weapon", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hInactive = SPR_Load(sz);
		pWeapon->rcInactive = p->rc;

		gHR.iHistoryGap = V_max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hInactive = 0;

	p = GetSpriteList(pList, "weapon_s", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hActive = SPR_Load(sz);
		pWeapon->rcActive = p->rc;
	}
	else
		pWeapon->hActive = 0;

	p = GetSpriteList(pList, "akimbo", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAkimboInactive = SPR_Load(sz);
		pWeapon->rcAkimboInactive = p->rc;
	}
	else
		pWeapon->hAkimboInactive = 0;

	p = GetSpriteList(pList, "akimbo_s", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAkimboActive = SPR_Load(sz);
		pWeapon->rcAkimboActive = p->rc;
	}
	else
		pWeapon->hAkimboActive = 0;

	p = GetSpriteList(pList, "ammo", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo = SPR_Load(sz);
		pWeapon->rcAmmo = p->rc;

		gHR.iHistoryGap = V_max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hAmmo = 0;

	p = GetSpriteList(pList, "ammo2", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo2 = SPR_Load(sz);
		pWeapon->rcAmmo2 = p->rc;

		gHR.iHistoryGap = V_max(gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top);
	}
	else
		pWeapon->hAmmo2 = 0;

	// add any unknown icon names too so they can be used by custom weapons
	pWeapon->numCustomIcons = 0;
	if (pList) {
		int count = i;
		client_sprite_t* p = pList;

		while (i--)
		{
			if ((p->iRes == iRes) && !g_default_weapon_hud_icon_names.hasKey(p->szName)) {
				if (pWeapon->numCustomIcons < MAX_CUSTOM_HUD_ICONS) {
					sprintf(sz, "sprites/%s.spr", p->szSprite);
					pWeapon->customIcons[pWeapon->numCustomIcons].hSprite = SPR_Load(sz);
					pWeapon->customIcons[pWeapon->numCustomIcons].rect = p->rc;
					pWeapon->numCustomIcons++;
				}
				else {
					PRINTF("Not loading icon '%s' for weapon '%s' (max of %d custom icons)\n",
						p->szName, pWeapon->szName, MAX_CUSTOM_HUD_ICONS);
				}
			}
			p++;
		}
	}

	gHR.iHistoryOffset = (32 + (gHR.iHistoryGap * 2));
}

// Returns the first weapon for a given slot.
WEAPON* WeaponsResource::GetFirstPos(int iSlot)
{
	WEAPON* pret = NULL;

	for (int i = 0; i < MAX_WEAPON_POSITIONS; i++)
	{
		if (rgSlots[iSlot][i] && (HasAmmo(rgSlots[iSlot][i]) || (rgSlots[iSlot][i]->iFlags & WEAPON_FLAGS_SELECTONEMPTY)))
		{
			pret = rgSlots[iSlot][i];
			break;
		}
	}

	return pret;
}


WEAPON* WeaponsResource::GetNextActivePos(int iSlot, int iSlotPos)
{
	if (iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS)
		return NULL;

	WEAPON* p = gWR.rgSlots[iSlot][iSlotPos + 1];

	if (!p || (!gWR.HasAmmo(p) && !(p->iFlags & WEAPON_FLAGS_SELECTONEMPTY)))
		return GetNextActivePos(iSlot, iSlotPos + 1);

	return p;
}

//
// Helper function to return a Ammo pointer from id
//

HSPRITE* WeaponsResource::GetAmmoPicFromWeapon(int iAmmoId, wrect_t& rect)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (rgWeapons[i].iAmmoType == iAmmoId)
		{
			rect = rgWeapons[i].rcAmmo;
			return &rgWeapons[i].hAmmo;
		}
		else if (rgWeapons[i].iAmmo2Type == iAmmoId)
		{
			rect = rgWeapons[i].rcAmmo2;
			return &rgWeapons[i].hAmmo2;
		}
	}

	return NULL;
}

