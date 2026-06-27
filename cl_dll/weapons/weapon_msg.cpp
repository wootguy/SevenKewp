#include "hud.h"
#include "cl_util.h"
#include "ammohistory.h"
#include "com_weapons.h"
#include "parsemsg.h"
#include "pm_shared.h"
#include "wc_net.h"
#include "ModPlayerState.h"

int __MsgFunc_AmmoX(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	gWR.SetAmmo(iIndex, abs(iCount));

	return 1;
}

int __MsgFunc_AmmoXX(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();
	int iCount = (uint16_t)READ_SHORT();

	gWR.SetAmmo(iIndex, abs(iCount));

	return 1;
}

int __MsgFunc_AmmoPickup(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	// Add ammo to the history
	gHR.AddToHistory(HISTSLOT_AMMO, iIndex, abs(iCount));

	return 1;
}

int __MsgFunc_WeapPickup(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	int iIndex = READ_BYTE();

	// Add the weapon to the history
	gHR.AddToHistory(HISTSLOT_WEAP, iIndex);

	return 1;
}

int __MsgFunc_ItemPickup(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	const char* szName = READ_STRING();

	// Add the weapon to the history
	gHR.AddToHistory(HISTSLOT_ITEM, szName);

	return 1;
}

int __MsgFunc_HideWeapon(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	gHUD.m_iHideHUDDisplay = READ_BYTE();

	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL))
	{
		static wrect_t nullrc;
		gHUD.m_WeaponList.m_pActiveSel = NULL;
		SetCrosshair(0, nullrc, 0, 0, 0);
	}
	else
	{
		if (g_pActiveWeapon)
			SetCrosshair(g_pActiveWeapon->hCrosshair, g_pActiveWeapon->rcCrosshair, 255, 255, 255);
	}

	return 1;
}

// 
//  CurWeapon: Update hud state with the current weapon and clip count. Ammo
//  counts are updated with AmmoX. Server assures that the Weapon ammo type 
//  numbers match a real ammo type.
//
int CurWeapon(int iState, int iId, int iClip) {
	static wrect_t nullrc;
	int fOnTarget = FALSE;

	// detect if we're also on target
	if (iState > 1)
	{
		fOnTarget = TRUE;
	}

	if (iId < 1)
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
		return 0;
	}

	if (g_iUser1 != OBS_IN_EYE)
	{
		// Is player dead???
		if ((iId == -1) && (iClip == -1))
		{
			gHUD.m_fPlayerDead = TRUE;
			gHUD.m_WeaponList.m_pActiveSel = NULL;
			return 1;
		}
		gHUD.m_fPlayerDead = FALSE;
	}

	WEAPON* pWeapon = gWR.GetWeapon(iId);

	if (!pWeapon)
		return 0;

	if (iClip < -1)
		pWeapon->iClip = abs(iClip);
	else
		pWeapon->iClip = iClip;


	if (iState == 0)	// we're not the current weapon, so update no more
		return 1;

	g_pActiveWeapon = pWeapon;

	gHUD.m_Crosshair.UpdateZoomCrosshair(iId, gHUD.m_Crosshair.IsWeaponZoomed(), fOnTarget);

	gHUD.m_Ammo.m_iFlags |= HUD_ACTIVE;

	return 1;
}

int __MsgFunc_CurWeapon(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = READ_CHAR();

	return CurWeapon(iState, iId, iClip);
}

int __MsgFunc_CurWeaponX(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = (uint16_t)READ_SHORT();

	return CurWeapon(iState, iId, iClip);
}

//
// WeaponList -- Tells the hud about a new weapon type.
//
int __MsgFunc_WeaponList(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	WEAPON Weapon;

	strncpy(Weapon.szName, READ_STRING(), MAX_WEAPON_NAME);
	Weapon.szName[sizeof(Weapon.szName) - 1] = '\0';

	Weapon.iAmmoType = (int)READ_CHAR();

	Weapon.iMax1 = READ_BYTE();
	if (Weapon.iMax1 == 255)
		Weapon.iMax1 = -1;

	Weapon.iAmmo2Type = READ_CHAR();
	Weapon.iMax2 = READ_BYTE();
	if (Weapon.iMax2 == 255)
		Weapon.iMax2 = -1;

	Weapon.iSlot = READ_CHAR();
	Weapon.iSlotPos = READ_CHAR();
	Weapon.iId = READ_CHAR();
	Weapon.iFlags = READ_BYTE();
	Weapon.iClip = 0;
	Weapon.iMaxClip2 = 0;

	if (Weapon.iId < 0 || Weapon.iId >= MAX_WEAPONS)
		return 0;

	if (Weapon.iSlot < 0 || Weapon.iSlot >= MAX_WEAPON_SLOTS + 1)
		return 0;

	if (Weapon.iSlotPos < 0 || Weapon.iSlotPos >= MAX_WEAPON_POSITIONS + 1)
		return 0;

	if (Weapon.iAmmoType < -1 || Weapon.iAmmoType >= MAX_AMMO_TYPES)
		return 0;

	if (Weapon.iAmmo2Type < -1 || Weapon.iAmmo2Type >= MAX_AMMO_TYPES)
		return 0;

	if (Weapon.iAmmoType >= 0 && Weapon.iMax1 == 0)
		return 0;

	if (Weapon.iAmmo2Type >= 0 && Weapon.iMax2 == 0)
		return 0;

	gWR.AddWeapon(&Weapon);

	SetItemInfo(&Weapon);

	return 1;
}

int __MsgFunc_WeaponListX(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	uint8_t id = READ_BYTE();
	uint16_t maxAmmo1 = READ_SHORT();
	uint16_t maxAmmo2 = READ_SHORT();
	uint16_t maxClip2 = READ_SHORT();
	uint8_t flags = READ_BYTE();
	float accuracy = READ_SHORT() * 0.01f;
	float accuracy2 = READ_SHORT() * 0.01f;
	float accuracyY = READ_SHORT() * 0.01f;
	float accuracyY2 = READ_SHORT() * 0.01f;

	if (id >= MAX_WEAPONS) {
		PRINTF("WeaponListX: Invalid weapon ID %d\n", (int)id);
		return 1;
	}

	WEAPON* pWeapon = gWR.GetWeapon(id);

	if (!pWeapon)
		return 1;

	pWeapon->iMax1 = maxAmmo1;
	pWeapon->iMax2 = maxAmmo2;
	pWeapon->iMaxClip2 = maxClip2;

	pWeapon->iFlagsEx = flags;
	bool hasSecondaryAccuracy = pWeapon->iFlagsEx & WEP_FLAG_SECONDARY_ACCURACY;

	pWeapon->accuracyX = accuracy;
	pWeapon->accuracyX2 = hasSecondaryAccuracy ? accuracy2 : accuracy;
	pWeapon->accuracyY = pWeapon->accuracyX;
	pWeapon->accuracyY2 = pWeapon->accuracyX2;

	if (flags & WEP_FLAG_VERTICAL_ACCURACY) {
		pWeapon->accuracyY = accuracyY;

		if (hasSecondaryAccuracy)
			pWeapon->accuracyY2 = accuracyY2;
	}

	return 1;
}

int __MsgFunc_CustomWep(const char* pszName, int iSize, void* pbuf)
{
	return UTIL_ReadCustomWeaponPredictionData(pszName, iSize, pbuf);
}

int __MsgFunc_CustomWepAk(const char* pszName, int iSize, void* pbuf)
{
	return UTIL_ReadCustomWeaponPredictionAttackData(pszName, iSize, pbuf);
}

int __MsgFunc_CustomWepEv(const char* pszName, int iSize, void* pbuf)
{
	return UTIL_ReadCustomWeaponPredictionEventData(pszName, iSize, pbuf);
}

int __MsgFunc_CustomHud(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int weaponId = READ_BYTE();
	const char* customDir = READ_STRING();

	WEAPON* pWeapon = gWR.GetWeapon(weaponId);

	if (pWeapon) {
		g_weaponHudDirs.put(pWeapon->szName, customDir);
		gWR.LoadWeaponSprites(pWeapon);
	}


	return 1;
}

int __MsgFunc_PmodelAnim(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int playeridx = READ_BYTE();

	if (playeridx < 0 || playeridx > 32)
		return 0;

	ModPlayerState& state = g_modPlayerStates[playeridx];
	state.pmodelanim = READ_BYTE();
	state.pmodelAnimTime = gEngfuncs.GetClientTime();

	return 1;
}

int __MsgFunc_WeaponBits(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	uint64_t low = (uint32_t)READ_LONG();
	uint64_t high = (uint32_t)READ_LONG();

	ModPlayerState& state = GetLocalPlayerState();
	state.weaponBits = (high << 32) | low;

	return 1;
}

int __MsgFunc_MatsPath(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	const char* materialsFile = READ_STRING();

	if (materialsFile[0]) {
		int loaded = LoadCustomMaterials(materialsFile);
		//PRINTF("Loaded %d custom materials from file\n%s\n", loaded, fpath);
	}

	const char* hudFile = READ_STRING();

	gHUD.ReplaceHudSprites(hudFile);

	return 1;
}


void HookWeaponMessages() {
	HOOK_MESSAGE(CurWeapon);
	HOOK_MESSAGE(CurWeaponX);
	HOOK_MESSAGE(WeaponList);
	HOOK_MESSAGE(WeaponListX);
	HOOK_MESSAGE(CustomWep);
	HOOK_MESSAGE(CustomWepAk);
	HOOK_MESSAGE(CustomWepEv);
	HOOK_MESSAGE(CustomHud);
	HOOK_MESSAGE(PmodelAnim);
	HOOK_MESSAGE(WeaponBits);
	HOOK_MESSAGE(MatsPath);
	HOOK_MESSAGE(AmmoPickup);
	HOOK_MESSAGE(WeapPickup);
	HOOK_MESSAGE(ItemPickup);
	HOOK_MESSAGE(HideWeapon);
	HOOK_MESSAGE(AmmoX);
	HOOK_MESSAGE(AmmoXX);
}