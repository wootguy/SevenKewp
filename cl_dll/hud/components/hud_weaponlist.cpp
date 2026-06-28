#include "hud.h"
#include "cl_util.h"
#include "vgui_TeamFortressViewport.h"
#include "ammohistory.h"
#include "wc_params.h"
#include "ModPlayerState.h"
#include "shared_util.h"
#include "triangleapi.h"
#include "HashMap.h"
#include "com_weapons.h"
#include "gfx_util.h"
#include "GL/gl.h"

int giBucketHeight, giBucketWidth; // weapon bucket width and height
int giABHeight, giABWidth; // weapon icon ammo bar width and height

DECLARE_COMMAND(m_WeaponList, Slot1);
DECLARE_COMMAND(m_WeaponList, Slot2);
DECLARE_COMMAND(m_WeaponList, Slot3);
DECLARE_COMMAND(m_WeaponList, Slot4);
DECLARE_COMMAND(m_WeaponList, Slot5);
DECLARE_COMMAND(m_WeaponList, Slot6);
DECLARE_COMMAND(m_WeaponList, Slot7);
DECLARE_COMMAND(m_WeaponList, Slot8);
DECLARE_COMMAND(m_WeaponList, Slot9);
DECLARE_COMMAND(m_WeaponList, Slot10);
DECLARE_COMMAND(m_WeaponList, Close);
DECLARE_COMMAND(m_WeaponList, NextWeapon);
DECLARE_COMMAND(m_WeaponList, PrevWeapon);


// for aborting an action while holding an exclusive weapon
bool ExclusiveWeaponAbort() {
	if (g_pActiveWeapon && IsExclusiveWeapon(g_pActiveWeapon->iId)) {
		gEngfuncs.pfnCenterPrint("Drop this weapon to select another.");
		return true;
	}

	return false;
}


void CHudWeaponList::SlotInput(int iSlot)
{
	if (gViewPort && gViewPort->SlotInput(iSlot))
		return;

	if (ExclusiveWeaponAbort()) {
		return;
	}

	SelectSlot(iSlot, FALSE, 1);
}

void CHudWeaponList::UserCmd_Slot1(void)
{
	SlotInput(0);
}

void CHudWeaponList::UserCmd_Slot2(void)
{
	SlotInput(1);
}

void CHudWeaponList::UserCmd_Slot3(void)
{
	SlotInput(2);
}

void CHudWeaponList::UserCmd_Slot4(void)
{
	SlotInput(3);
}

void CHudWeaponList::UserCmd_Slot5(void)
{
	SlotInput(4);
}

void CHudWeaponList::UserCmd_Slot6(void)
{
	SlotInput(5);
}

void CHudWeaponList::UserCmd_Slot7(void)
{
	SlotInput(6);
}

void CHudWeaponList::UserCmd_Slot8(void)
{
	SlotInput(7);
}

void CHudWeaponList::UserCmd_Slot9(void)
{
	SlotInput(8);
}

void CHudWeaponList::UserCmd_Slot10(void)
{
	SlotInput(9);
}

void CHudWeaponList::UserCmd_Close(void)
{
	if (m_pActiveSel)
	{
		m_pLastSel = m_pActiveSel;
		m_pActiveSel = NULL;
		PlaySound(RemapFile("common/wpn_hudoff.wav"), 1);
	}
	else
		EngineClientCmd("escape");
}

// Selects the next item in the weapon menu
void CHudWeaponList::UserCmd_NextWeapon(void)
{
	if (gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return;

	if (!m_pActiveSel || m_pActiveSel == (WEAPON*)1)
		m_pActiveSel = g_pActiveWeapon;

	int pos = 0;
	int slot = 0;
	if (m_pActiveSel)
	{
		pos = m_pActiveSel->iSlotPos + 1;
		slot = m_pActiveSel->iSlot;
	}

	for (int loop = 0; loop <= 1; loop++)
	{
		for (; slot < MAX_WEAPON_SLOTS; slot++)
		{
			for (; pos < MAX_WEAPON_POSITIONS; pos++)
			{
				WEAPON* wsp = gWR.GetWeaponSlot(slot, pos);

				if (wsp && (gWR.HasAmmo(wsp) || (wsp->iFlags & WEAPON_FLAGS_SELECTONEMPTY)))
				{
					m_pActiveSel = wsp;
					return;
				}
			}

			pos = 0;
		}

		slot = 0;  // start looking from the first slot again
	}

	m_pActiveSel = NULL;
}

// Selects the previous item in the menu
void CHudWeaponList::UserCmd_PrevWeapon(void)
{
	if (gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return;

	if (!m_pActiveSel || m_pActiveSel == (WEAPON*)1)
		m_pActiveSel = g_pActiveWeapon;

	int pos = MAX_WEAPON_POSITIONS - 1;
	int slot = MAX_WEAPON_SLOTS - 1;
	if (m_pActiveSel)
	{
		pos = m_pActiveSel->iSlotPos - 1;
		slot = m_pActiveSel->iSlot;
	}

	for (int loop = 0; loop <= 1; loop++)
	{
		for (; slot >= 0; slot--)
		{
			for (; pos >= 0; pos--)
			{
				WEAPON* wsp = gWR.GetWeaponSlot(slot, pos);

				if (wsp && (gWR.HasAmmo(wsp) || (wsp->iFlags & WEAPON_FLAGS_SELECTONEMPTY)))
				{
					m_pActiveSel = wsp;
					return;
				}
			}

			pos = MAX_WEAPON_POSITIONS - 1;
		}

		slot = MAX_WEAPON_SLOTS - 1;
	}

	m_pActiveSel = NULL;
}



int CHudWeaponList::Init(void)
{
	HOOK_COMMAND("slot1", Slot1);
	HOOK_COMMAND("slot2", Slot2);
	HOOK_COMMAND("slot3", Slot3);
	HOOK_COMMAND("slot4", Slot4);
	HOOK_COMMAND("slot5", Slot5);
	HOOK_COMMAND("slot6", Slot6);
	HOOK_COMMAND("slot7", Slot7);
	HOOK_COMMAND("slot8", Slot8);
	HOOK_COMMAND("slot9", Slot9);
	HOOK_COMMAND("slot10", Slot10);
	HOOK_COMMAND("cancelselect", Close);
	HOOK_COMMAND("invnext", NextWeapon);
	HOOK_COMMAND("invprev", PrevWeapon);

	gHUD.AddHudElem(this);

	Reset();

	m_iFlags |= HUD_ACTIVE; //!!!

	// controls whether or not weapons can be selected in one keypress
	CVAR_CREATE("hud_fastswitch", "0", FCVAR_ARCHIVE);

	gWR.Init();
	gHR.Init();

	return 1;
};

void CHudWeaponList::Reset(void)
{
	m_iFlags |= HUD_ACTIVE; //!!!

	m_pActiveSel = NULL;
	gWR.Reset();
	gHR.Reset();
}

int CHudWeaponList::VidInit(void)
{
	// Load sprites for buckets (top row of weapon menu)
	m_HUD_bucket0 = gHUD.GetSpriteIndex("bucket1");
	m_HUD_selection = gHUD.GetSpriteIndex("selection");

	giBucketWidth = gHUD.GetSpriteRect(m_HUD_bucket0).right - gHUD.GetSpriteRect(m_HUD_bucket0).left;
	giBucketHeight = gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top;

	// If we've already loaded weapons, let's get new sprites
	gWR.LoadAllWeaponSprites();

	ResetCustomWeaponStates();
	g_weaponHudDirs.clear();

	int nScale = 1;

#if !defined( _TFC )
	if (ScreenWidth > 2560 && ScreenHeight > 1600)
		nScale = 4;
	else if (ScreenWidth >= 1280 && ScreenHeight > 720)
		nScale = 3;
	else
#endif
		if (ScreenWidth >= 640)
			nScale = 2;

	if (gHUD.m_pCvarHudScale->value > 0) {
		nScale = clamp(gHUD.m_pCvarHudScale->value, 1, 4);
	}

	giABWidth = 10 * nScale;
	giABHeight = 2 * nScale;

	return 1;
}

void CHudWeaponList::Think(void)
{
	if (gHUD.m_fPlayerDead)
		return;

	if (gHUD.m_iWeaponBits != gWR.iOldWeaponBits)
	{
		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;

		for (int i = MAX_WEAPONS - 1; i > 0; i--)
		{
			WEAPON* p = gWR.GetWeapon(i);

			if (p)
			{
				if (gHUD.m_iWeaponBits & (1ULL << p->iId))
					gWR.PickupWeapon(p);
				else
					gWR.DropWeapon(p);
			}
		}
	}

	if (!m_pActiveSel)
		return;

	// has the player selected one?
	if (gHUD.m_iKeyBits & IN_ATTACK)
	{
		if (m_pActiveSel != (WEAPON*)1)
		{
			if (!ExclusiveWeaponAbort()) {
				ServerCmd(m_pActiveSel->szName);
				m_weaponselect = m_pActiveSel->iId;
			}
		}

		m_pLastSel = m_pActiveSel;
		m_pActiveSel = NULL;
		gHUD.m_iKeyBits &= ~IN_ATTACK;

		PlaySound(RemapFile("common/wpn_select.wav"), 1);
	}

	if (ExclusiveWeaponAbort() && (m_pActiveSel || (gHUD.m_iKeyBits & IN_ATTACK))) {
		m_pActiveSel = NULL;
		gHUD.m_iKeyBits &= ~IN_ATTACK;
	}
}

int CHudWeaponList::Draw(float flTime)
{
	if (!(gHUD.m_iWeaponBits & (1ULL << (WEAPON_SUIT))))
		return 1;

	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return 1;

	// Draw Weapon Menu
	DrawWList(flTime);

	return 1;
}

int CHudWeaponList::DrawBar(int x, int y, int width, int height, float f)
{
	int r, g, b;

	if (f < 0)
		f = 0;
	if (f > 1)
		f = 1;

	if (f)
	{
		int w = f * width;

		// Always show at least one pixel if we have ammo.
		if (w <= 0)
			w = 1;
		UnpackRGB(r, g, b, RGB_GREENISH);
		FillRGBA(x, y, w, height, r, g, b, 255);
		x += w;
		width -= w;
	}

	UnpackRGB(r, g, b, gHUD.GetHudColor());

	FillRGBA(x, y, width, height, r, g, b, 128);

	return (x + width);
}

void CHudWeaponList::DrawAmmoBars(WEAPON* p, int x, int y, int width, int height)
{
	if (!p)
		return;

	if (p->iAmmoType != -1)
	{
		if (!gWR.CountAmmo(p->iAmmoType))
			return;

		float f = (float)gWR.CountAmmo(p->iAmmoType) / (float)p->iMax1;

		x = DrawBar(x, y, width, height, f);


		// Do we have secondary ammo too?

		if (p->iAmmo2Type != -1)
		{
			f = (float)gWR.CountAmmo(p->iAmmo2Type) / (float)p->iMax2;

			x += 5; //!!!

			DrawBar(x, y, width, height, f);
		}
	}
}

int CHudWeaponList::DrawWList(float flTime)
{
	int r, g, b, x, y, a, i;

	if (!m_pActiveSel)
		return 0;

	int iActiveSlot;

	if (m_pActiveSel == (WEAPON*)1)
		iActiveSlot = -1;	// current slot has no weapons
	else
		iActiveSlot = m_pActiveSel->iSlot;

	x = 10; //!!!
	y = 10; //!!!


	// Ensure that there are available choices in the active slot
	if (iActiveSlot > 0)
	{
		if (!gWR.GetFirstPos(iActiveSlot))
		{
			m_pActiveSel = (WEAPON*)1;
			iActiveSlot = -1;
		}
	}

	// Draw top line
	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		int iWidth;

		UnpackRGB(r, g, b, gHUD.GetHudColor());

		if (iActiveSlot == i)
			a = 255;
		else
			a = 192;

		ScaleColors(r, g, b, 255);
		SPR_Set(gHUD.GetSprite(m_HUD_bucket0 + i), r, g, b);

		// make active slot wide enough to accomodate gun pictures
		if (i == iActiveSlot)
		{
			WEAPON* p = gWR.GetFirstPos(iActiveSlot);
			if (p)
				iWidth = p->rcActive.right - p->rcActive.left;
			else
				iWidth = giBucketWidth;
		}
		else
			iWidth = giBucketWidth;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_bucket0 + i));

		x += iWidth + 5;
	}


	a = 128; //!!!
	x = 10;

	// Draw all of the buckets
	for (i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		y = giBucketHeight + 10;

		// If this is the active slot, draw the bigger pictures,
		// otherwise just draw boxes
		if (i == iActiveSlot)
		{
			WEAPON* p = gWR.GetFirstPos(i);

			int iWidth = giBucketWidth;
			if (p)
				iWidth = p->rcActive.right - p->rcActive.left;

			for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
			{
				p = gWR.GetWeaponSlot(i, iPos);

				if (!p || !p->iId)
					continue;

				bool akimbo = g_prediction.weapon.canAkimbo;
				HSPRITE activeSpr = akimbo ? p->hAkimboActive : p->hActive;
				HSPRITE inactiveSpr = akimbo ? p->hAkimboInactive : p->hInactive;
				wrect_t rcActive = akimbo ? p->rcAkimboActive : p->rcActive;
				wrect_t rcInactive = akimbo ? p->rcAkimboInactive : p->rcInactive;

				UnpackRGB(r, g, b, gHUD.GetHudColor());

				// Draw Weapon if Red if no ammo
				if (gWR.HasAmmo(p))
					ScaleColors(r, g, b, 192);
				else
				{
					UnpackRGB(r, g, b, RGB_REDISH);
					ScaleColors(r, g, b, 128);
				}

				// if active, then we must have ammo.

				if (m_pActiveSel == p)
				{
					SPR_Set(activeSpr, r, g, b);
					SPR_DrawAdditive(0, x, y, &rcActive);

					SPR_Set(gHUD.GetSprite(m_HUD_selection), r, g, b);
					SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_selection));
				}
				else
				{
					SPR_Set(inactiveSpr, r, g, b);
					SPR_DrawAdditive(0, x, y, &rcInactive);
				}

				// Draw Ammo Bars

				DrawAmmoBars(p, x + giABWidth / 2, y, giABWidth, giABHeight);

				y += p->rcActive.bottom - p->rcActive.top + 5;
			}

			x += iWidth + 5;

		}
		else
		{
			// Draw Row of weapons.

			UnpackRGB(r, g, b, gHUD.GetHudColor());

			for (int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++)
			{
				WEAPON* p = gWR.GetWeaponSlot(i, iPos);

				if (!p || !p->iId)
					continue;

				if (gWR.HasAmmo(p))
				{
					UnpackRGB(r, g, b, gHUD.GetHudColor());
					a = 128;
				}
				else
				{
					UnpackRGB(r, g, b, RGB_REDISH);
					a = 96;
				}

				FillRGBA(x, y, giBucketWidth, giBucketHeight, r, g, b, a);

				y += giBucketHeight + 5;
			}

			x += giBucketWidth + 5;
		}
	}

	return 1;

}

void CHudWeaponList::SelectSlot(int iSlot, int fAdvance, int iDirection)
{
	if (gHUD.m_Menu.m_fMenuDisplayed && (fAdvance == FALSE) && (iDirection == 1))
	{ // menu is overriding slot use commands
		gHUD.m_Menu.SelectMenuItem(iSlot + 1);  // slots are one off the key numbers
		return;
	}

	if (iSlot > MAX_WEAPON_SLOTS)
		return;

	if (gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL))
		return;

	if (!(gHUD.m_iWeaponBits & (1ULL << (WEAPON_SUIT))))
		return;

	if (!(gHUD.m_iWeaponBits & ~(1ULL << (WEAPON_SUIT))))
		return;

	WEAPON* p = NULL;
	bool fastSwitch = CVAR_GET_FLOAT("hud_fastswitch") != 0;

	if ((m_pActiveSel == NULL) || (m_pActiveSel == (WEAPON*)1) || (iSlot != m_pActiveSel->iSlot))
	{
		PlaySound(RemapFile("common/wpn_hudon.wav"), 1);
		p = gWR.GetFirstPos(iSlot);

		if (p && fastSwitch) // check for fast weapon switch mode
		{
			// if fast weapon switch is on, then weapons can be selected in a single keypress
			// but only if there is only one item in the bucket
			WEAPON* p2 = gWR.GetNextActivePos(p->iSlot, p->iSlotPos);
			if (!p2)
			{	// only one active item in bucket, so change directly to weapon
				if (!ExclusiveWeaponAbort()) {
					ServerCmd(p->szName);
					m_weaponselect = p->iId;
				}
				return;
			}
		}
	}
	else
	{
		PlaySound(RemapFile("common/wpn_moveselect.wav"), 1);
		if (m_pActiveSel)
			p = gWR.GetNextActivePos(m_pActiveSel->iSlot, m_pActiveSel->iSlotPos);
		if (!p)
			p = gWR.GetFirstPos(iSlot);
	}


	if (!p)  // no selection found
	{
		// just display the weapon list, unless fastswitch is on just ignore it
		if (!fastSwitch)
			m_pActiveSel = (WEAPON*)1;
		else
			m_pActiveSel = NULL;
	}
	else
		m_pActiveSel = p;
}