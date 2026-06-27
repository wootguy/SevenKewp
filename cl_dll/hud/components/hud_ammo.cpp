/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
//
// Ammo.cpp
//
// implementation of CHudAmmo class
//

#include "hud.h"
#include "cl_util.h"

#include <string.h>
#include <stdio.h>

#include "ammohistory.h"
#include "wc_params.h"
#include "ModPlayerState.h"
#include "shared_util.h"
#include "triangleapi.h"
#include "HashMap.h"
#include "com_weapons.h"
#include "gfx_util.h"
#include "GL/gl.h"

// width of ammo fonts
#define AMMO_SMALL_WIDTH 10
#define AMMO_LARGE_WIDTH 20

#define HISTORY_DRAW_TIME	"5"

void ResetCustomWeaponStates();

int CHudAmmo::Init(void)
{
	gHUD.AddHudElem(this);

	Reset();
	ResetCustomWeaponStates();

	CVAR_CREATE( "hud_drawhistory_time", HISTORY_DRAW_TIME, 0 );

	m_iFlags |= HUD_ACTIVE; //!!!

	return 1;
};

void CHudAmmo::Reset(void)
{
	m_fFade = 0;
	m_iFlags |= HUD_ACTIVE; //!!!

	gHUD.m_iHideHUDDisplay = 0;
}

//-------------------------------------------------------------------------
// Drawing code
//-------------------------------------------------------------------------

int CHudAmmo::Draw(float flTime)
{
	int a, x, y, r, g, b;

	if (!(gHUD.m_iWeaponBits & (1ULL<<(WEAPON_SUIT)) ))
		return 1;

	if ( (gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL )) )
		return 1;

	// Draw ammo pickup history
	gHR.DrawAmmoHistory( flTime );

	if (!(m_iFlags & HUD_ACTIVE))
		return 0;

	if (!g_pActiveWeapon)
		return 0;

	WEAPON *pw = g_pActiveWeapon; // shorthand

	// SPR_Draw Ammo
	if ((pw->iAmmoType < 0) && (pw->iAmmo2Type < 0))
		return 0;

	a = (int)V_max(MIN_ALPHA, m_fFade);

	int iClip = 0;
	int iClip2 = -1;
	int primaryAmmo = 0;
	int secondaryAmmo = 0;
	int iAkimboClip = 0;
	int stateIconIdx = GetCustomWeaponStateIconIdx();
	if (!GetPredictedAmmoCount(pw->iId, iClip, iClip2, primaryAmmo, secondaryAmmo, iAkimboClip)) {
		iClip = pw->iClip;
		primaryAmmo = gWR.CountAmmo(pw->iAmmoType);
		secondaryAmmo = gWR.CountAmmo(pw->iAmmo2Type);
		iAkimboClip = -1;
		iClip2 = -1;
	}

	// brighten up the hud when ammo info changes
	static int lastClip, lastClip2, lastPrimaryAmmo, lastSecondaryAmmo, lastAkimboClip, lastStateIcon;
	if (iClip != lastClip || iClip2 != lastClip2 || primaryAmmo != lastPrimaryAmmo
		|| secondaryAmmo != lastSecondaryAmmo || iAkimboClip != lastAkimboClip || lastStateIcon != stateIconIdx) {
		m_fFade = 200.0f;
		lastClip = iClip;
		lastClip2 = iClip2;
		lastPrimaryAmmo = primaryAmmo;
		lastSecondaryAmmo = secondaryAmmo;
		lastAkimboClip = iAkimboClip;
		lastStateIcon = stateIconIdx;
	}
	else if (m_fFade > 0) {
		m_fFade -= (gHUD.m_flTimeDelta * 20);
	}
	
	unsigned long hudcolor = gHUD.GetHudColor();

	UnpackRGB(r,g,b, hudcolor);

	ScaleColors(r, g, b, a );

	y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight/2;
	y += (int)(gHUD.m_iFontHeight * 0.2f);

	// Does weapon have any ammo at all?
	if (pw->iAmmoType > 0)
	{
		int clip = iAkimboClip >= 0 ? iAkimboClip : iClip;
		DrawAmmoInfo(x, y, clip, primaryAmmo, pw->hAmmo, pw->rcAmmo, 3);
	}

	bool hasSecondaryRow = false;

	// Does weapon have seconday ammo?
	if (iAkimboClip >= 0) {
		hasSecondaryRow = true;
		y -= gHUD.m_iFontHeight + gHUD.m_iFontHeight / 4;
		DrawAmmoInfo(x, y, -1, iClip, pw->hAmmo, pw->rcAmmo, 0);
	}
	else if (pw->iAmmo2Type > 0) 
	{
		y -= gHUD.m_iFontHeight + gHUD.m_iFontHeight / 4;

		if (pw->iMaxClip2 > 0 && iClip2 >= 0)
		{
			hasSecondaryRow = true;
			DrawAmmoInfo(x, y, iClip2, secondaryAmmo, pw->hAmmo2, pw->rcAmmo2, 2);
		}
		else if (secondaryAmmo > 0)
		{
			hasSecondaryRow = true;
			DrawAmmoInfo(x, y, -1, secondaryAmmo, pw->hAmmo2, pw->rcAmmo2, 0);
		}
		// else hide ammo row if there's no ammo left
	}

	gHR.iHistoryOffset = (32 + (gHR.iHistoryGap * 2));
	
	// Weapon state sprite shown above ammo info
	if (stateIconIdx >= 0 && stateIconIdx < pw->numCustomIcons) {
		CustomHudIcon& icon = pw->customIcons[stateIconIdx];
		SPR_Set(icon.hSprite, r, g, b);

		int AmmoWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;
		int iIconHeight = icon.rect.bottom - icon.rect.top;
		int iIconWidth = icon.rect.right - icon.rect.left;
		int iOffset = iIconHeight / 8;
		x = ScreenWidth - AmmoWidth - iIconWidth;
		y -= gHUD.m_iFontHeight + gHUD.m_iFontHeight / 4;

		gHR.iHistoryOffset += iIconHeight;

		SPR_DrawAdditive(0, x, y - iOffset, &icon.rect);
	}

	return 1;
}

void CHudAmmo::DrawAmmoInfo(int& x, int& y, int iClip, int iAmmo, HSPRITE hAmmo, wrect_t rcAmmo, int minDigits) {
	int r, g, b;
	int a = (int)V_max(MIN_ALPHA, m_fFade);
	unsigned long hudcolor = gHUD.GetHudColor();
	UnpackRGB(r, g, b, hudcolor);
	ScaleColors(r, g, b, a);
	int AmmoWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;

	int iIconWidth = rcAmmo.right - rcAmmo.left;

	if (iClip >= 0)
	{
		int digitFlags = 0;
		int baseWidth = 6 * AmmoWidth; // includes '|'
		x = ScreenWidth - baseWidth - iIconWidth;

		if (iAmmo >= 1000 || minDigits >= 4) {
			digitFlags |= DHN_4DIGITS;
			x -= 4 * AmmoWidth;
		}
		else if (iAmmo >= 100 || minDigits >= 3) {
			digitFlags |= DHN_3DIGITS;
			x -= 3 * AmmoWidth;
		}
		else if (iAmmo >= 10 || minDigits >= 2) {
			digitFlags |= DHN_2DIGITS;
			x -= 2 * AmmoWidth;
		}
		else {
			x -= 1 * AmmoWidth;
		}

		// room for the number and the '|' and the current ammo
		x = gHUD.DrawHudNumber(x, y, DHN_DRAWZERO | DHN_4DIGITS, iClip, r, g, b);

		wrect_t rc;
		rc.top = 0;
		rc.left = 0;
		rc.right = AmmoWidth;
		rc.bottom = 100;

		int iBarWidth = AmmoWidth / 10;

		x += AmmoWidth / 2;

		UnpackRGB(r, g, b, hudcolor);

		// draw the | bar
		FillRGBA(x, y, iBarWidth, gHUD.m_iFontHeight, r, g, b, a);

		x += iBarWidth + AmmoWidth / 2;

		// GL Seems to need this
		ScaleColors(r, g, b, a);
		x = gHUD.DrawHudNumber(x, y, DHN_DRAWZERO | digitFlags, iAmmo, r, g, b);
	}
	else
	{
		// SPR_Draw a bullets only line
		x = ScreenWidth - 5 * AmmoWidth - iIconWidth;
		x = gHUD.DrawHudNumber(x, y, DHN_DRAWZERO | DHN_4DIGITS, iAmmo, r, g, b);
	}

	if (hAmmo) {
		// Draw the ammo Icon
		SPR_Set(hAmmo, r, g, b);
		int iOffset = (rcAmmo.bottom - rcAmmo.top) / 8;
		SPR_DrawAdditive(0, x, y - iOffset, &rcAmmo);
	}
}

