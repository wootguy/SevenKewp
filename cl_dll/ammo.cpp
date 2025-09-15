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
#include "parsemsg.h"
#include "pm_shared.h"

#include <string.h>
#include <stdio.h>

#include "ammohistory.h"
#include "vgui_TeamFortressViewport.h"
#include "custom_weapon.h"
#include "ModPlayerState.h"
#include "shared_util.h"

CustomWeaponParams* GetCustomWeaponParams(int id);
void GetCurrentCustomWeaponAccuracy(int id, float& accuracyX, float& accuracyY, float& accuracyX2, float& accuracyY2, bool& dynamicAccuracy);
void GetCurrentCustomWeaponState(int id, int& akimboClip);
bool CanWeaponAkimbo(int id);
bool IsExclusiveWeapon(int id);
void InitCustomWeapon(int id);

extern int g_last_attack_mode;
extern float g_last_attack_time;
extern vec3_t v_punchangle;

// for aborting an action while holding an exclusive weapon
bool ExclusiveWeaponAbort() {
	if (gHUD.m_Ammo.m_pWeapon && IsExclusiveWeapon(gHUD.m_Ammo.m_pWeapon->iId)) {
		gEngfuncs.pfnCenterPrint("Drop this weapon to select another.");
		return true;
	}

	return false;
}

WEAPON *gpActiveSel;	// NULL means off, 1 means just the menu bar, otherwise
						// this points to the active weapon menu item
WEAPON *gpLastSel;		// Last weapon menu selection 

client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount);

WeaponsResource gWR;

int g_weaponselect = 0;

void WeaponsResource :: LoadAllWeaponSprites( void )
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if ( rgWeapons[i].iId )
			LoadWeaponSprites( &rgWeapons[i] );
	}
}

int WeaponsResource :: CountAmmo( int iId ) 
{ 
	if ( iId < 0 )
		return 0;

	return riAmmo[iId];
}

int WeaponsResource :: HasAmmo( WEAPON *p )
{
	if ( !p )
		return FALSE;

	// weapons with no max ammo can always be selected
	if ( p->iMax1 == -1 )
		return TRUE;

	return (p->iAmmoType == -1) || p->iClip > 0 || CountAmmo(p->iAmmoType) 
		|| CountAmmo(p->iAmmo2Type) || ( p->iFlags & WEAPON_FLAGS_SELECTONEMPTY );
}


void WeaponsResource :: LoadWeaponSprites( WEAPON *pWeapon )
{
	int i, iRes;

#if !defined( _TFC )
	if (ScreenWidth > 2560 && ScreenHeight > 1600)
		iRes = 2560;
	else if (ScreenWidth >= 1280 && ScreenHeight > 720)
		iRes = 1280;
	else
#endif
	if (ScreenWidth >= 640)
		iRes = 640;
	else
		iRes = 320;

#ifdef VANILLA_HL
	char sz[128];
#else
	char sz[144];
#endif

	if ( !pWeapon )
		return;

	memset( &pWeapon->rcActive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcInactive, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo, 0, sizeof(wrect_t) );
	memset( &pWeapon->rcAmmo2, 0, sizeof(wrect_t) );
	pWeapon->hInactive = 0;
	pWeapon->hActive = 0;
	pWeapon->hAmmo = 0;
	pWeapon->hAmmo2 = 0;

	sprintf(sz, "sprites/%s.txt", pWeapon->szName);
	client_sprite_t *pList = SPR_GetList(sz, &i);

	if (!pList)
		return;

	client_sprite_t *p;
	
	p = GetSpriteList( pList, "crosshair", iRes, i );
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

	p = GetSpriteList( pList, "zoom", iRes, i );
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

		gHR.iHistoryGap = V_max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
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

		gHR.iHistoryGap = V_max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo = 0;

	p = GetSpriteList(pList, "ammo2", iRes, i);
	if (p)
	{
		sprintf(sz, "sprites/%s.spr", p->szSprite);
		pWeapon->hAmmo2 = SPR_Load(sz);
		pWeapon->rcAmmo2 = p->rc;

		gHR.iHistoryGap = V_max( gHR.iHistoryGap, pWeapon->rcActive.bottom - pWeapon->rcActive.top );
	}
	else
		pWeapon->hAmmo2 = 0;

}

// Returns the first weapon for a given slot.
WEAPON *WeaponsResource :: GetFirstPos( int iSlot )
{
	WEAPON *pret = NULL;

	for (int i = 0; i < MAX_WEAPON_POSITIONS; i++)
	{
		if ( rgSlots[iSlot][i] && HasAmmo( rgSlots[iSlot][i] ) )
		{
			pret = rgSlots[iSlot][i];
			break;
		}
	}

	return pret;
}


WEAPON* WeaponsResource :: GetNextActivePos( int iSlot, int iSlotPos )
{
	if ( iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS )
		return NULL;

	WEAPON *p = gWR.rgSlots[ iSlot ][ iSlotPos+1 ];
	
	if ( !p || !gWR.HasAmmo(p) )
		return GetNextActivePos( iSlot, iSlotPos + 1 );

	return p;
}


int giBucketHeight, giBucketWidth, giABHeight, giABWidth; // Ammo Bar width and height

HSPRITE ghsprBuckets;					// Sprite for top row of weapons menu

DECLARE_MESSAGE(m_Ammo, CurWeapon );	// Current weapon and clip
DECLARE_MESSAGE(m_Ammo, CurWeaponX );	// Current weapon and clip (large clip)
DECLARE_MESSAGE(m_Ammo, WeaponList);	// new weapon type
DECLARE_MESSAGE(m_Ammo, WeaponListX);	// new weapon type (extra parameters)
DECLARE_MESSAGE(m_Ammo, CustomWep);		// custom weapon parameters
DECLARE_MESSAGE(m_Ammo, CustomWepEv);	// custom weapon parameters
DECLARE_MESSAGE(m_Ammo, PmodelAnim);	// player model anim
DECLARE_MESSAGE(m_Ammo, WeaponBits);	// currently held weapons
DECLARE_MESSAGE(m_Ammo, SoundIdx);		// sound index to file path mapping
DECLARE_MESSAGE(m_Ammo, AmmoX);			// update known ammo type's count
DECLARE_MESSAGE(m_Ammo, AmmoXX);		// update known ammo type's count (higher max count)
DECLARE_MESSAGE(m_Ammo, AmmoPickup);	// flashes an ammo pickup record
DECLARE_MESSAGE(m_Ammo, WeapPickup);    // flashes a weapon pickup record
DECLARE_MESSAGE(m_Ammo, HideWeapon);	// hides the weapon, ammo, and crosshair displays temporarily
DECLARE_MESSAGE(m_Ammo, ItemPickup);

DECLARE_COMMAND(m_Ammo, Slot1);
DECLARE_COMMAND(m_Ammo, Slot2);
DECLARE_COMMAND(m_Ammo, Slot3);
DECLARE_COMMAND(m_Ammo, Slot4);
DECLARE_COMMAND(m_Ammo, Slot5);
DECLARE_COMMAND(m_Ammo, Slot6);
DECLARE_COMMAND(m_Ammo, Slot7);
DECLARE_COMMAND(m_Ammo, Slot8);
DECLARE_COMMAND(m_Ammo, Slot9);
DECLARE_COMMAND(m_Ammo, Slot10);
DECLARE_COMMAND(m_Ammo, Close);
DECLARE_COMMAND(m_Ammo, NextWeapon);
DECLARE_COMMAND(m_Ammo, PrevWeapon);

// width of ammo fonts
#define AMMO_SMALL_WIDTH 10
#define AMMO_LARGE_WIDTH 20

#define HISTORY_DRAW_TIME	"5"

void ResetCustomWeaponStates();

ModPlayerState& GetLocalPlayerState() {
	int idx = GetLocalPlayer()->index;
	return g_modPlayerStates[idx];
}

int CHudAmmo::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(CurWeapon);
	HOOK_MESSAGE(CurWeaponX);
	HOOK_MESSAGE(WeaponList);
	HOOK_MESSAGE(WeaponListX);
	HOOK_MESSAGE(CustomWep);
	HOOK_MESSAGE(CustomWepEv);
	HOOK_MESSAGE(PmodelAnim);
	HOOK_MESSAGE(WeaponBits);
	HOOK_MESSAGE(SoundIdx);
	HOOK_MESSAGE(AmmoPickup);
	HOOK_MESSAGE(WeapPickup);
	HOOK_MESSAGE(ItemPickup);
	HOOK_MESSAGE(HideWeapon);
	HOOK_MESSAGE(AmmoX);
	HOOK_MESSAGE(AmmoXX);

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

	Reset();

	CVAR_CREATE( "hud_drawhistory_time", HISTORY_DRAW_TIME, 0 );
	CVAR_CREATE( "hud_fastswitch", "0", FCVAR_ARCHIVE );		// controls whether or not weapons can be selected in one keypress
	
	m_hud_crosshair_mode = gEngfuncs.pfnRegisterVariable("hud_crosshair_mode", "1", FCVAR_ARCHIVE);
	m_hud_crosshair_length = gEngfuncs.pfnRegisterVariable("hud_crosshair_length", "15", FCVAR_ARCHIVE);
	m_hud_crosshair_width = gEngfuncs.pfnRegisterVariable("hud_crosshair_width", "-1", FCVAR_ARCHIVE);
	m_hud_crosshair_border = gEngfuncs.pfnRegisterVariable("hud_crosshair_border", "1", FCVAR_ARCHIVE);

	m_iFlags |= HUD_ACTIVE; //!!!

	gWR.Init();
	gHR.Init();

	return 1;
};

void CHudAmmo::Reset(void)
{
	m_fFade = 0;
	m_iFlags |= HUD_ACTIVE; //!!!

	gpActiveSel = NULL;
	gHUD.m_iHideHUDDisplay = 0;

	gWR.Reset();
	gHR.Reset();
}

int CHudAmmo::VidInit(void)
{
	// Load sprites for buckets (top row of weapon menu)
	m_HUD_bucket0 = gHUD.GetSpriteIndex( "bucket1" );
	m_HUD_selection = gHUD.GetSpriteIndex( "selection" );

	ghsprBuckets = gHUD.GetSprite(m_HUD_bucket0);
	giBucketWidth = gHUD.GetSpriteRect(m_HUD_bucket0).right - gHUD.GetSpriteRect(m_HUD_bucket0).left;
	giBucketHeight = gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top;

	gHR.iHistoryGap = V_max( gHR.iHistoryGap, gHUD.GetSpriteRect(m_HUD_bucket0).bottom - gHUD.GetSpriteRect(m_HUD_bucket0).top);

	// If we've already loaded weapons, let's get new sprites
	gWR.LoadAllWeaponSprites();

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

	giABWidth = 10 * nScale;
	giABHeight = 2 * nScale;

	ResetCustomWeaponStates();

	return 1;
}

//
// Think:
//  Used for selection of weapon menu item.
//
void CHudAmmo::Think(void)
{
	if ( gHUD.m_fPlayerDead )
		return;

	if ( gHUD.m_iWeaponBits != gWR.iOldWeaponBits )
	{
		gWR.iOldWeaponBits = gHUD.m_iWeaponBits;

		for (int i = MAX_WEAPONS-1; i > 0; i-- )
		{
			WEAPON *p = gWR.GetWeapon(i);

			if ( p )
			{
				if ( gHUD.m_iWeaponBits & ( 1ULL << p->iId ) )
					gWR.PickupWeapon( p );
				else
					gWR.DropWeapon( p );
			}
		}
	}

	if (!gpActiveSel)
		return;

	// has the player selected one?
	if (gHUD.m_iKeyBits & IN_ATTACK)
	{
		if (gpActiveSel != (WEAPON *)1)
		{
			if (!ExclusiveWeaponAbort()) {
				ServerCmd(gpActiveSel->szName);
				g_weaponselect = gpActiveSel->iId;
			}
		}

		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		gHUD.m_iKeyBits &= ~IN_ATTACK;

		PlaySound("common/wpn_select.wav", 1);
	}

	if (ExclusiveWeaponAbort() && (gpActiveSel || (gHUD.m_iKeyBits & IN_ATTACK))) {
		gpActiveSel = NULL;
		gHUD.m_iKeyBits &= ~IN_ATTACK;
	}
}

//
// Helper function to return a Ammo pointer from id
//

HSPRITE* WeaponsResource :: GetAmmoPicFromWeapon( int iAmmoId, wrect_t& rect )
{
	for ( int i = 0; i < MAX_WEAPONS; i++ )
	{
		if ( rgWeapons[i].iAmmoType == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo;
			return &rgWeapons[i].hAmmo;
		}
		else if ( rgWeapons[i].iAmmo2Type == iAmmoId )
		{
			rect = rgWeapons[i].rcAmmo2;
			return &rgWeapons[i].hAmmo2;
		}
	}

	return NULL;
}


// Menu Selection Code

void WeaponsResource :: SelectSlot( int iSlot, int fAdvance, int iDirection )
{
	if ( gHUD.m_Menu.m_fMenuDisplayed && (fAdvance == FALSE) && (iDirection == 1) )	
	{ // menu is overriding slot use commands
		gHUD.m_Menu.SelectMenuItem( iSlot + 1 );  // slots are one off the key numbers
		return;
	}

	if ( iSlot > MAX_WEAPON_SLOTS )
		return;

	if ( gHUD.m_fPlayerDead || gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
		return;

	if (!(gHUD.m_iWeaponBits & (1ULL<<(WEAPON_SUIT)) ))
		return;

	if ( ! ( gHUD.m_iWeaponBits & ~(1ULL<<(WEAPON_SUIT)) ))
		return;

	WEAPON *p = NULL;
	bool fastSwitch = CVAR_GET_FLOAT( "hud_fastswitch" ) != 0;

	if ( (gpActiveSel == NULL) || (gpActiveSel == (WEAPON *)1) || (iSlot != gpActiveSel->iSlot) )
	{
		PlaySound( "common/wpn_hudon.wav", 1 );
		p = GetFirstPos( iSlot );

		if ( p && fastSwitch ) // check for fast weapon switch mode
		{
			// if fast weapon switch is on, then weapons can be selected in a single keypress
			// but only if there is only one item in the bucket
			WEAPON *p2 = GetNextActivePos( p->iSlot, p->iSlotPos );
			if ( !p2 )
			{	// only one active item in bucket, so change directly to weapon
				if (!ExclusiveWeaponAbort()) {
					ServerCmd(p->szName);
					g_weaponselect = p->iId;
				}
				return;
			}
		}
	}
	else
	{
		PlaySound("common/wpn_moveselect.wav", 1);
		if ( gpActiveSel )
			p = GetNextActivePos( gpActiveSel->iSlot, gpActiveSel->iSlotPos );
		if ( !p )
			p = GetFirstPos( iSlot );
	}

	
	if ( !p )  // no selection found
	{
		// just display the weapon list, unless fastswitch is on just ignore it
		if ( !fastSwitch )
			gpActiveSel = (WEAPON *)1;
		else
			gpActiveSel = NULL;
	}
	else 
		gpActiveSel = p;
}

//------------------------------------------------------------------------
// Message Handlers
//------------------------------------------------------------------------

//
// AmmoX  -- Update the count of a known type of ammo
// 
int CHudAmmo::MsgFunc_AmmoX(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ( pbuf, iSize );

	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	gWR.SetAmmo( iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_AmmoXX(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int iIndex = READ_BYTE();
	int iCount = (uint16_t)READ_SHORT();

	gWR.SetAmmo(iIndex, abs(iCount));

	return 1;
}

int CHudAmmo::MsgFunc_AmmoPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iIndex = READ_BYTE();
	int iCount = READ_BYTE();

	// Add ammo to the history
	gHR.AddToHistory( HISTSLOT_AMMO, iIndex, abs(iCount) );

	return 1;
}

int CHudAmmo::MsgFunc_WeapPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	int iIndex = READ_BYTE();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_WEAP, iIndex );

	return 1;
}

int CHudAmmo::MsgFunc_ItemPickup( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	const char *szName = READ_STRING();

	// Add the weapon to the history
	gHR.AddToHistory( HISTSLOT_ITEM, szName );

	return 1;
}


int CHudAmmo::MsgFunc_HideWeapon( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	gHUD.m_iHideHUDDisplay = READ_BYTE();

	if (gEngfuncs.IsSpectateOnly())
		return 1;

	if ( gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL ) )
	{
		static wrect_t nullrc;
		gpActiveSel = NULL;
		SetCrosshair( 0, nullrc, 0, 0, 0 );
	}
	else
	{
		if ( m_pWeapon )
			SetCrosshair( m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255 );
	}

	return 1;
}

bool CHudAmmo::IsWeaponZoomed() {
	return gHUD.m_iFOV < 90;
}

void CHudAmmo::UpdateZoomCrosshair(int id, bool zoom, bool autoaimOnTarget) {
	if (id < 1)
		return;

	WEAPON* pWeapon = gWR.GetWeapon(id);

	if (!pWeapon)
		return;

	if (!zoom)
	{ // normal crosshairs
		if (autoaimOnTarget && m_pWeapon->hAutoaim)
			SetCrosshair(m_pWeapon->hAutoaim, m_pWeapon->rcAutoaim, 255, 255, 255);
		else
			SetCrosshair(m_pWeapon->hCrosshair, m_pWeapon->rcCrosshair, 255, 255, 255);
	}
	else
	{ // zoomed crosshairs
		if (autoaimOnTarget && m_pWeapon->hZoomedAutoaim)
			SetCrosshair(m_pWeapon->hZoomedAutoaim, m_pWeapon->rcZoomedAutoaim, 255, 255, 255);
		else
			SetCrosshair(m_pWeapon->hZoomedCrosshair, m_pWeapon->rcZoomedCrosshair, 255, 255, 255);
	}
}

// for updating in prediction code
void UpdateZoomCrosshair(int id, bool zoom) {
	gHUD.m_Ammo.UpdateZoomCrosshair(id, zoom, true);
}

// 
//  CurWeapon: Update hud state with the current weapon and clip count. Ammo
//  counts are updated with AmmoX. Server assures that the Weapon ammo type 
//  numbers match a real ammo type.
//
int CHudAmmo::CurWeapon(int iState, int iId, int iClip) {
	static wrect_t nullrc;
	int fOnTarget = FALSE;
	
	// detect if we're also on target
	if ( iState > 1 )
	{
		fOnTarget = TRUE;
	}

	if ( iId < 1 )
	{
		SetCrosshair(0, nullrc, 0, 0, 0);
		return 0;
	}

	if ( g_iUser1 != OBS_IN_EYE )
	{
		// Is player dead???
		if ((iId == -1) && (iClip == -1))
		{
			gHUD.m_fPlayerDead = TRUE;
			gpActiveSel = NULL;
			return 1;
		}
		gHUD.m_fPlayerDead = FALSE;
	}

	WEAPON *pWeapon = gWR.GetWeapon( iId );

	if ( !pWeapon )
		return 0;

	if ( iClip < -1 )
		pWeapon->iClip = abs(iClip);
	else
		pWeapon->iClip = iClip;


	if ( iState == 0 )	// we're not the current weapon, so update no more
		return 1;

	m_pWeapon = pWeapon;

	UpdateZoomCrosshair(iId, IsWeaponZoomed(), fOnTarget);

	m_fFade = 200.0f; //!!!
	m_iFlags |= HUD_ACTIVE;
	
	return 1;
}

int CHudAmmo::MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = READ_CHAR();

	return CurWeapon(iState, iId, iClip);
}

int CHudAmmo::MsgFunc_CurWeaponX(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	int iState = READ_BYTE();
	int iId = READ_CHAR();
	int iClip = (uint16_t)READ_SHORT();

	return CurWeapon(iState, iId, iClip);
}

//
// WeaponList -- Tells the hud about a new weapon type.
//
int CHudAmmo::MsgFunc_WeaponList(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	
	WEAPON Weapon;

	strncpy( Weapon.szName, READ_STRING(), MAX_WEAPON_NAME );
	Weapon.szName[ sizeof(Weapon.szName) - 1 ] = '\0';

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

	if (Weapon.iId < 0 || Weapon.iId >= MAX_WEAPONS)
		return 0;

	if (Weapon.iSlot < 0 || Weapon.iSlot >= MAX_WEAPON_SLOTS+1)
		return 0;

	if (Weapon.iSlotPos < 0 || Weapon.iSlotPos >= MAX_WEAPON_POSITIONS+1)
		return 0;

	if (Weapon.iAmmoType < -1 || Weapon.iAmmoType >= MAX_AMMO_TYPES)
		return 0;

	if (Weapon.iAmmo2Type < -1 || Weapon.iAmmo2Type >= MAX_AMMO_TYPES)
		return 0;

	if (Weapon.iAmmoType >= 0 && Weapon.iMax1 == 0)
		return 0;

	if (Weapon.iAmmo2Type >= 0 && Weapon.iMax2 == 0)
		return 0;

	gWR.AddWeapon( &Weapon );

	return 1;
}

int CHudAmmo::MsgFunc_WeaponListX(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	uint8_t id = READ_BYTE();
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


// set up parameters for custom weapon prediction
int CHudAmmo::MsgFunc_CustomWep(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int weaponId = READ_BYTE();
	
	if (weaponId < 0 || weaponId >= MAX_WEAPONS)
		return 0;

	
	InitCustomWeapon(weaponId);
	CustomWeaponParams& parms = *GetCustomWeaponParams(weaponId);
	memset(&parms, 0, sizeof(CustomWeaponParams));

	parms.flags = READ_SHORT();
	parms.maxClip = READ_SHORT();

	parms.vmodel = READ_SHORT();
	parms.deployAnim = READ_BYTE();
	parms.deployTime = READ_SHORT();
	parms.deployAnimTime = READ_SHORT();

	for (int k = 0; k < 3; k++) {
		WeaponCustomReload& reload = parms.reloadStage[k];
		reload.anim = READ_BYTE();
		reload.time = READ_SHORT();

		if (k == 2 && !(parms.flags & FL_WC_WEP_SHOTGUN_RELOAD))
			break;
	}

	for (int k = 0; k < 4; k++) {
		WeaponCustomIdle& idle = parms.idles[k];
		idle.anim = READ_BYTE();
		idle.weight = READ_BYTE();
		idle.time = READ_SHORT();
	}

	if (parms.flags & FL_WC_WEP_AKIMBO) {
		for (int k = 0; k < 4; k++) {
			WeaponCustomIdle& idle = parms.akimbo.idles[k];
			idle.anim = READ_BYTE();
			idle.weight = READ_BYTE();
			idle.time = READ_SHORT();
		}

		parms.akimbo.reload.anim = READ_BYTE();
		parms.akimbo.reload.time = READ_SHORT();

		parms.akimbo.deployAnim = READ_BYTE();
		parms.akimbo.deployTime = READ_SHORT();
		parms.akimbo.akimboDeployAnim = READ_BYTE();
		parms.akimbo.akimboDeployTime = READ_SHORT();
		parms.akimbo.akimboDeployAnimTime = READ_SHORT();
		parms.akimbo.holsterAnim = READ_BYTE();
		parms.akimbo.holsterTime = READ_SHORT();
		parms.akimbo.accuracyX = READ_SHORT();
		parms.akimbo.accuracyY = READ_SHORT();
	}

	if (parms.flags & FL_WC_WEP_HAS_LASER) {
		for (int k = 0; k < 4; k++) {
			WeaponCustomIdle& idle = parms.laser.idles[k];
			idle.anim = READ_BYTE();
			idle.weight = READ_BYTE();
			idle.time = READ_SHORT();
		}

		parms.laser.dotSprite = READ_SHORT();
		parms.laser.beamSprite = READ_SHORT();
		parms.laser.dotSz = READ_BYTE();
		parms.laser.beamWidth = READ_BYTE();
		parms.laser.attachment = READ_BYTE();
	}

	for (int i = 0; i < 4; i++) {
		if (!(parms.flags & FL_WC_WEP_HAS_PRIMARY) && i == 0)
			continue;
		if (!(parms.flags & FL_WC_WEP_HAS_SECONDARY) && i == 1)
			continue;
		if (!(parms.flags & FL_WC_WEP_HAS_TERTIARY) && i == 2)
			continue;
		if (!(parms.flags & FL_WC_WEP_HAS_ALT_PRIMARY) && i == 3)
			continue;

		CustomWeaponShootOpts& opts = parms.shootOpts[i];
		opts.flags = READ_BYTE();
		opts.ammoCost = READ_BYTE();
		opts.cooldown = READ_SHORT();
		opts.cooldownFail = READ_SHORT();
		opts.chargeTime = READ_SHORT();
		opts.chargeCancelTime = READ_SHORT();
		opts.accuracyX = READ_SHORT();
		opts.accuracyY = READ_SHORT();
	}

	return 1;
}

int CHudAmmo::MsgFunc_CustomWepEv(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int weaponId = READ_BYTE();

	if (weaponId < 0 || weaponId >= MAX_WEAPONS)
		return 0;

	CustomWeaponParams& parms = *GetCustomWeaponParams(weaponId);

	parms.numEvents = READ_BYTE();
	if (parms.numEvents >= MAX_WC_EVENTS) {
		return 0;
	}

	for (int i = 0; i < parms.numEvents; i++) {
		uint16_t packedHeader = READ_SHORT();
		WepEvt& evt = parms.events[i];
		memset(&evt, 0, sizeof(WepEvt));

		evt.evtType = packedHeader & 0x1F;
		evt.trigger = (packedHeader >> 5) & 0x1F;
		evt.triggerArg = (packedHeader >> 10) & 0x1F;
		evt.hasDelay = packedHeader >> 15;

		if (evt.hasDelay)
			evt.delay = READ_SHORT();

		switch (evt.evtType) {
		case WC_EVT_IDLE_SOUND: {
			uint16_t packedFlags = READ_SHORT();
			evt.idleSound.sound = packedFlags >> 7;
			evt.idleSound.volume = packedFlags & 0x7F;
			break;
		}
		case WC_EVT_PLAY_SOUND: {
			uint16_t packedFlags = READ_SHORT();
			evt.playSound.sound = packedFlags >> 5;
			evt.playSound.channel = (packedFlags >> 2) & 0x7;
			evt.playSound.aiVol = (packedFlags >> 0) & 0x3;
			evt.playSound.volume = READ_BYTE();
			evt.playSound.attn = READ_BYTE();
			evt.playSound.pitchMin = READ_BYTE();
			evt.playSound.pitchMax = READ_BYTE();

			evt.playSound.numAdditionalSounds = READ_BYTE();
			for (int k = 0; k < evt.playSound.numAdditionalSounds && k < MAX_WC_RANDOM_SELECTION; k++) {
				evt.playSound.additionalSounds[k] = READ_SHORT();
			}
			break;
		}
		case WC_EVT_EJECT_SHELL:
			evt.ejectShell.model = READ_SHORT();
			evt.ejectShell.offsetForward = READ_SHORT();
			evt.ejectShell.offsetUp = READ_SHORT();
			evt.ejectShell.offsetRight = READ_SHORT();
			break;
		case WC_EVT_PUNCH:
			evt.punch.flags = READ_BYTE();
			evt.punch.x = READ_SHORT();
			evt.punch.y = READ_SHORT();
			evt.punch.z = READ_SHORT();
			break;
		case WC_EVT_SET_BODY:
			evt.setBody.newBody = READ_BYTE();
			break;
		case WC_EVT_WEP_ANIM: {
			uint8_t packedHeader = READ_BYTE();
			evt.anim.flags = packedHeader >> 6;
			evt.anim.akimbo = (packedHeader >> 3) & 0x7;
			evt.anim.numAnim = packedHeader & 0x7;
			for (int k = 0; k < evt.anim.numAnim && k < MAX_WC_RANDOM_SELECTION; k++) {
				evt.anim.anims[k] = READ_BYTE();
			}
			break;
		}
		case WC_EVT_BULLETS: {
			evt.bullets.count = READ_BYTE();
			evt.bullets.burstDelay = READ_SHORT();
			//evt.bullets.damage = READ_SHORT();
			evt.bullets.spreadX = READ_SHORT();
			evt.bullets.spreadY = READ_SHORT();
			evt.bullets.tracerFreq = READ_BYTE();

			uint8_t packedFlags = READ_BYTE();
			evt.bullets.flags = packedFlags >> 4;
			evt.bullets.flashSz = packedFlags & 0xf;
			break;
		}
		case WC_EVT_KICKBACK:
			evt.kickback.pushForce = READ_SHORT();
			break;
		case WC_EVT_TOGGLE_ZOOM:
			evt.zoomToggle.zoomFov = READ_BYTE();
			break;
		case WC_EVT_HIDE_LASER:
			evt.laserHide.millis = READ_SHORT();
			break;
		case WC_EVT_COOLDOWN:
			evt.cooldown.millis = READ_SHORT();
			evt.cooldown.targets = READ_BYTE();
			break;
		case WC_EVT_TOGGLE_AKIMBO:
		case WC_EVT_TOGGLE_LASER:
		case WC_EVT_PROJECTILE:
			break;
		default:
			gEngfuncs.Con_Printf("Bad custom weapon event type read %d\n", (int)evt.evtType);
			break;
		}
	}

	return 1;
}

int CHudAmmo::MsgFunc_PmodelAnim(const char* pszName, int iSize, void* pbuf)
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

int CHudAmmo::MsgFunc_WeaponBits(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	uint64_t low = (uint32_t)READ_LONG();
	uint64_t high = (uint32_t)READ_LONG();

	ModPlayerState& state = GetLocalPlayerState();
	state.weaponBits = (high << 32) | low;

	return 1;
}

void AddWeaponCustomSoundMapping(int idx, const char* path);

int CHudAmmo::MsgFunc_SoundIdx(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	int soundCount = READ_BYTE();

	for (int i = 0; i < soundCount; i++) {
		int idx = READ_SHORT();
		const char* soundPath = READ_STRING();
		AddWeaponCustomSoundMapping(idx, soundPath);
	}

	return 1;
}

//------------------------------------------------------------------------
// Command Handlers
//------------------------------------------------------------------------
// Slot button pressed
void CHudAmmo::SlotInput( int iSlot )
{
	if ( gViewPort && gViewPort->SlotInput( iSlot ) )
		return;

	if (ExclusiveWeaponAbort()) {
		return;
	}

	gWR.SelectSlot(iSlot, FALSE, 1);
}

void CHudAmmo::UserCmd_Slot1(void)
{
	SlotInput( 0 );
}

void CHudAmmo::UserCmd_Slot2(void)
{
	SlotInput( 1 );
}

void CHudAmmo::UserCmd_Slot3(void)
{
	SlotInput( 2 );
}

void CHudAmmo::UserCmd_Slot4(void)
{
	SlotInput( 3 );
}

void CHudAmmo::UserCmd_Slot5(void)
{
	SlotInput( 4 );
}

void CHudAmmo::UserCmd_Slot6(void)
{
	SlotInput( 5 );
}

void CHudAmmo::UserCmd_Slot7(void)
{
	SlotInput( 6 );
}

void CHudAmmo::UserCmd_Slot8(void)
{
	SlotInput( 7 );
}

void CHudAmmo::UserCmd_Slot9(void)
{
	SlotInput( 8 );
}

void CHudAmmo::UserCmd_Slot10(void)
{
	SlotInput( 9 );
}

void CHudAmmo::UserCmd_Close(void)
{
	if (gpActiveSel)
	{
		gpLastSel = gpActiveSel;
		gpActiveSel = NULL;
		PlaySound("common/wpn_hudoff.wav", 1);
	}
	else
		EngineClientCmd("escape");
}


// Selects the next item in the weapon menu
void CHudAmmo::UserCmd_NextWeapon(void)
{
	if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
		return;

	if ( !gpActiveSel || gpActiveSel == (WEAPON*)1 )
		gpActiveSel = m_pWeapon;

	int pos = 0;
	int slot = 0;
	if ( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos + 1;
		slot = gpActiveSel->iSlot;
	}

	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot < MAX_WEAPON_SLOTS; slot++ )
		{
			for ( ; pos < MAX_WEAPON_POSITIONS; pos++ )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if ( wsp && gWR.HasAmmo(wsp) )
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = 0;
		}

		slot = 0;  // start looking from the first slot again
	}

	gpActiveSel = NULL;
}

// Selects the previous item in the menu
void CHudAmmo::UserCmd_PrevWeapon(void)
{
	if ( gHUD.m_fPlayerDead || (gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) )
		return;

	if ( !gpActiveSel || gpActiveSel == (WEAPON*)1 )
		gpActiveSel = m_pWeapon;

	int pos = MAX_WEAPON_POSITIONS-1;
	int slot = MAX_WEAPON_SLOTS-1;
	if ( gpActiveSel )
	{
		pos = gpActiveSel->iSlotPos - 1;
		slot = gpActiveSel->iSlot;
	}
	
	for ( int loop = 0; loop <= 1; loop++ )
	{
		for ( ; slot >= 0; slot-- )
		{
			for ( ; pos >= 0; pos-- )
			{
				WEAPON *wsp = gWR.GetWeaponSlot( slot, pos );

				if ( wsp && gWR.HasAmmo(wsp) )
				{
					gpActiveSel = wsp;
					return;
				}
			}

			pos = MAX_WEAPON_POSITIONS-1;
		}
		
		slot = MAX_WEAPON_SLOTS-1;
	}

	gpActiveSel = NULL;
}



//-------------------------------------------------------------------------
// Drawing code
//-------------------------------------------------------------------------

int CHudAmmo::Draw(float flTime)
{
	int a, x, y, r, g, b;
	int AmmoWidth;

	if (!(gHUD.m_iWeaponBits & (1ULL<<(WEAPON_SUIT)) ))
		return 1;

	if ( (gHUD.m_iHideHUDDisplay & ( HIDEHUD_WEAPONS | HIDEHUD_ALL )) )
		return 1;

	// Draw Weapon Menu
	DrawWList(flTime);

	// Draw ammo pickup history
	gHR.DrawAmmoHistory( flTime );

	if (!(m_iFlags & HUD_ACTIVE))
		return 0;

	if (!m_pWeapon)
		return 0;

	WEAPON *pw = m_pWeapon; // shorthand

	DrawDynamicCrosshair();

	// SPR_Draw Ammo
	if ((pw->iAmmoType < 0) && (pw->iAmmo2Type < 0))
		return 0;


	int iFlags = DHN_DRAWZERO; // draw 0 values

	AmmoWidth = gHUD.GetSpriteRect(gHUD.m_HUD_number_0).right - gHUD.GetSpriteRect(gHUD.m_HUD_number_0).left;

	a = (int) V_max( MIN_ALPHA, m_fFade );

	if (m_fFade > 0)
		m_fFade -= (gHUD.m_flTimeDelta * 20);

	UnpackRGB(r,g,b, RGB_YELLOWISH);

	ScaleColors(r, g, b, a );

	y = ScreenHeight - gHUD.m_iFontHeight - gHUD.m_iFontHeight/2;
	y += (int)(gHUD.m_iFontHeight * 0.2f);

	int akimboClip = 0;
	GetCurrentCustomWeaponState(pw->iId, akimboClip);

	// Does weapon have any ammo at all?
	if (m_pWeapon->iAmmoType > 0)
	{
		int iIconWidth = m_pWeapon->rcAmmo.right - m_pWeapon->rcAmmo.left;
		
		int clip = akimboClip >= 0 ? akimboClip : pw->iClip;

		if (clip >= 0)
		{
			// room for the number and the '|' and the current ammo
			
			x = ScreenWidth - (8 * AmmoWidth) - iIconWidth;
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, clip, r, g, b);

			wrect_t rc;
			rc.top = 0;
			rc.left = 0;
			rc.right = AmmoWidth;
			rc.bottom = 100;

			int iBarWidth =  AmmoWidth/10;

			x += AmmoWidth/2;

			UnpackRGB(r,g,b, RGB_YELLOWISH);

			// draw the | bar
			FillRGBA(x, y, iBarWidth, gHUD.m_iFontHeight, r, g, b, a);

			x += iBarWidth + AmmoWidth/2;;

			// GL Seems to need this
			ScaleColors(r, g, b, a );
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);		


		}
		else
		{
			// SPR_Draw a bullets only line
			x = ScreenWidth - 4 * AmmoWidth - iIconWidth;
			x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, gWR.CountAmmo(pw->iAmmoType), r, g, b);
		}

		// Draw the ammo Icon
		int iOffset = (m_pWeapon->rcAmmo.bottom - m_pWeapon->rcAmmo.top)/8;
		SPR_Set(m_pWeapon->hAmmo, r, g, b);
		SPR_DrawAdditive(0, x, y - iOffset, &m_pWeapon->rcAmmo);
	}

	// Does weapon have seconday ammo?
	if (akimboClip >= 0) {
		int iIconWidth = m_pWeapon->rcAmmo2.right - m_pWeapon->rcAmmo2.left;
		y -= gHUD.m_iFontHeight + gHUD.m_iFontHeight / 4;
		x = ScreenWidth - 4 * AmmoWidth - iIconWidth;
		x = gHUD.DrawHudNumber(x, y, iFlags | DHN_3DIGITS, pw->iClip, r, g, b);

		// Draw the ammo Icon
		SPR_Set(m_pWeapon->hAmmo, r, g, b);
		int iOffset = (m_pWeapon->rcAmmo.bottom - m_pWeapon->rcAmmo.top) / 8;
		SPR_DrawAdditive(0, x, y - iOffset, &m_pWeapon->rcAmmo);
	}
	else if (pw->iAmmo2Type > 0) 
	{
		int iIconWidth = m_pWeapon->rcAmmo2.right - m_pWeapon->rcAmmo2.left;

		// Do we have secondary ammo?
		if ((pw->iAmmo2Type != 0) && (gWR.CountAmmo(pw->iAmmo2Type) > 0))
		{
			y -= gHUD.m_iFontHeight + gHUD.m_iFontHeight/4;
			x = ScreenWidth - 4 * AmmoWidth - iIconWidth;
			x = gHUD.DrawHudNumber(x, y, iFlags|DHN_3DIGITS, gWR.CountAmmo(pw->iAmmo2Type), r, g, b);

			// Draw the ammo Icon
			SPR_Set(m_pWeapon->hAmmo2, r, g, b);
			int iOffset = (m_pWeapon->rcAmmo2.bottom - m_pWeapon->rcAmmo2.top)/8;
			SPR_DrawAdditive(0, x, y - iOffset, &m_pWeapon->rcAmmo2);
		}
	}

	return 1;
}

// converts an accuracy in degrees to a pixel gap for the crosshair
int CrosshairGapPixels(float accuracyDeg, bool isVertical) {
	int screenW = ScreenWidth;
	int screenH = ScreenHeight;

	float aspect = (float)screenW / (float)screenH;
	float baseAspect = 4.0f / 3.0f; // GoldSrc aspect used in FOV calculation

	// convert GoldSrc fov to real FOV
	float fovXRad43 = gHUD.m_iFOV * (M_PI / 180.0f);
	float fovXRad = 2.0f * atan(tan(fovXRad43 * 0.5f) * (aspect / baseAspect));
	float fovYRad = 2.0f * atan(tan(fovXRad * 0.5f) / aspect);

	// accuracy angle in radians
	float accRad = accuracyDeg * 0.5f * (M_PI / 180.0f);
	float spread = tan(accRad); // for VECTOR_CONE_* math

	if (isVertical) {
		return (screenW * 0.5f) * (tan(accRad) / tan(fovXRad * 0.5f));
	}
	else {
		return (screenH * 0.5f) * (tan(accRad) / tan(fovYRad * 0.5f));
	}
}

void DrawCrossHair(float accuracyX, float accuracyY, int len, int thick, int border) {
	int r, g, b, a;

	int centerX = ScreenWidth / 2;
	int centerY = ScreenHeight / 2;

	int gapX = 10;
	int gapY = 10;
	int hthick = thick / 2;

	int minGap = thick + border * 4;

	gapX = V_max(CrosshairGapPixels(accuracyX, false), minGap);
	gapY = V_max(CrosshairGapPixels(accuracyY, true), minGap);

	if (gapX == minGap && gapY == minGap) {
		//len *= 0.75f;
	}
	a = 200;
	

	if (border > 0) {
		int blen = len + border * 2;
		int bthick = thick + border * 2;
		int bhthick = bthick / 2;
		r = g = b = 0;

		// horizontal
		gEngfuncs.pfnFillRGBABlend(centerX - (gapX + blen - border), centerY - bhthick, blen, bthick, r, g, b, a);
		gEngfuncs.pfnFillRGBABlend(centerX + gapX - border, centerY - bhthick, blen, bthick, r, g, b, a);

		// vertical
		gEngfuncs.pfnFillRGBABlend(centerX - bhthick, centerY - (gapY + blen - border), bthick, blen, r, g, b, a);
		gEngfuncs.pfnFillRGBABlend(centerX - bhthick, centerY + gapY - border, bthick, blen, r, g, b, a);

		// center dot
		gEngfuncs.pfnFillRGBABlend(centerX - bhthick, centerY - bhthick, bthick, bthick, r, g, b, a);
		gEngfuncs.pfnFillRGBABlend(centerX - bhthick, centerY - bhthick, bthick, bthick, r, g, b, a);
	}

	UnpackRGB(r, g, b, RGB_YELLOWISH);

	// horizontal
	gEngfuncs.pfnFillRGBABlend(centerX - (gapX + len), centerY - hthick, len, thick, r, g, b, a);
	gEngfuncs.pfnFillRGBABlend(centerX + gapX, centerY - hthick, len, thick, r, g, b, a);

	// vertical
	gEngfuncs.pfnFillRGBABlend(centerX - hthick, centerY - (gapY + len), thick, len, r, g, b, a);
	gEngfuncs.pfnFillRGBABlend(centerX - hthick, centerY + gapY, thick, len, r, g, b, a);

	// center dot
	gEngfuncs.pfnFillRGBABlend(centerX - hthick, centerY - hthick, thick, thick, r, g, b, a);
	gEngfuncs.pfnFillRGBABlend(centerX - hthick, centerY - hthick, thick, thick, r, g, b, a);
}

void CHudAmmo::DrawDynamicCrosshair() {
	if (!m_pWeapon || m_hud_crosshair_mode->value != 1 || !gHUD.IsSevenKewpServer())
		return;

	WEAPON* pw = m_pWeapon; // shorthand

	if (pw->hZoomedCrosshair && IsWeaponZoomed() && (pw->iFlagsEx & WEP_FLAG_USE_ZOOM_CROSSHAIR))
		return;

	if (g_crosshair_active) {
		static wrect_t nullrc;
		SetCrosshair(0, nullrc, 0, 0, 0);
	}
	
	float accuracyX = pw->accuracyX;
	float accuracyY = pw->accuracyY;
	float accuracyX2 = pw->accuracyX2;
	float accuracyY2 = pw->accuracyY2;
	bool dynamicAccuracy = pw->iFlagsEx & WEP_FLAG_DYNAMIC_ACCURACY;

	GetCurrentCustomWeaponAccuracy(pw->iId, accuracyX, accuracyY, accuracyX2, accuracyY2, dynamicAccuracy);

	float now = gEngfuncs.GetClientTime();
	if (g_last_attack_mode == 2 && (now - g_last_attack_time) < 2.0f) {
		accuracyX = accuracyX2;
		accuracyY = accuracyY2;
	}

	// lerp between accuracy changes
	{
		static int lastWeaponId;
		static float lastAccuracyX;
		static float lastAccuracyY;
		static float nextAccuracyX;
		static float nextAccuracyY;
		static float lerpStart;

		if (lastWeaponId != pw->iId) {
			lastWeaponId = pw->iId;
			lastAccuracyX = accuracyX;
			lastAccuracyY = accuracyY;
			nextAccuracyX = accuracyX;
			nextAccuracyY = accuracyY;
			lerpStart = now;
		}

		if (accuracyX != nextAccuracyX || accuracyY != nextAccuracyY) {
			lastAccuracyX = nextAccuracyX;
			lastAccuracyY = nextAccuracyY;
			nextAccuracyX = accuracyX;
			nextAccuracyY = accuracyY;
			lerpStart = now;
		}

		float t = V_min(0.1f, (now - lerpStart)) / 0.1f;
		accuracyX = lastAccuracyX + (nextAccuracyX - lastAccuracyX) * t;
		accuracyY = lastAccuracyY + (nextAccuracyY - lastAccuracyY) * t;
		accuracyX2 = accuracyX;
		accuracyY2 = accuracyY;
	}
	

	float punch = V_max(fabs(v_punchangle[0]), fabs(v_punchangle[1]));
	if (punch > 0) {
		punch = powf(punch, 0.5f);
		accuracyX += punch;
		accuracyY += punch;
		accuracyX2 += punch;
		accuracyY2 += punch;
	}

	int len = clamp(m_hud_crosshair_length->value, 1, 1000);
	int width = clamp(m_hud_crosshair_width->value, 1, 1000);
	int border = clamp(m_hud_crosshair_border->value, 0, 1000);

	if (m_hud_crosshair_width->value == -1) {
		// auto size
		width = 2;
		if (ScreenHeight <= 768) {
			width = 1;
		}
	}

	DrawCrossHair(accuracyX, accuracyY, len, width, border);
}


//
// Draws the ammo bar on the hud
//
int DrawBar(int x, int y, int width, int height, float f)
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

	UnpackRGB(r, g, b, RGB_YELLOWISH);

	FillRGBA(x, y, width, height, r, g, b, 128);

	return (x + width);
}



void DrawAmmoBar(WEAPON *p, int x, int y, int width, int height)
{
	if ( !p )
		return;
	
	if (p->iAmmoType != -1)
	{
		if (!gWR.CountAmmo(p->iAmmoType))
			return;

		float f = (float)gWR.CountAmmo(p->iAmmoType)/(float)p->iMax1;
		
		x = DrawBar(x, y, width, height, f);


		// Do we have secondary ammo too?

		if (p->iAmmo2Type != -1)
		{
			f = (float)gWR.CountAmmo(p->iAmmo2Type)/(float)p->iMax2;

			x += 5; //!!!

			DrawBar(x, y, width, height, f);
		}
	}
}




//
// Draw Weapon Menu
//
int CHudAmmo::DrawWList(float flTime)
{
	int r,g,b,x,y,a,i;

	if ( !gpActiveSel )
		return 0;

	int iActiveSlot;

	if ( gpActiveSel == (WEAPON *)1 )
		iActiveSlot = -1;	// current slot has no weapons
	else 
		iActiveSlot = gpActiveSel->iSlot;

	x = 10; //!!!
	y = 10; //!!!
	

	// Ensure that there are available choices in the active slot
	if ( iActiveSlot > 0 )
	{
		if ( !gWR.GetFirstPos( iActiveSlot ) )
		{
			gpActiveSel = (WEAPON *)1;
			iActiveSlot = -1;
		}
	}
		
	// Draw top line
	for ( i = 0; i < MAX_WEAPON_SLOTS; i++ )
	{
		int iWidth;

		UnpackRGB(r,g,b, RGB_YELLOWISH);
	
		if ( iActiveSlot == i )
			a = 255;
		else
			a = 192;

		ScaleColors(r, g, b, 255);
		SPR_Set(gHUD.GetSprite(m_HUD_bucket0 + i), r, g, b );

		// make active slot wide enough to accomodate gun pictures
		if ( i == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos(iActiveSlot);
			if ( p )
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
		if ( i == iActiveSlot )
		{
			WEAPON *p = gWR.GetFirstPos( i );

			int iWidth = giBucketWidth;
			if ( p )
				iWidth = p->rcActive.right - p->rcActive.left;

			for ( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				p = gWR.GetWeaponSlot( i, iPos );

				if ( !p || !p->iId )
					continue;

				bool akimbo = CanWeaponAkimbo(p->iId);
				HSPRITE activeSpr = akimbo ? p->hAkimboActive : p->hActive;
				HSPRITE inactiveSpr = akimbo ? p->hAkimboInactive : p->hInactive;
				wrect_t rcActive = akimbo ? p->rcAkimboActive : p->rcActive;
				wrect_t rcInactive = akimbo ? p->rcAkimboInactive : p->rcInactive;

				UnpackRGB( r,g,b, RGB_YELLOWISH );
			
				// if active, then we must have ammo.

				if ( gpActiveSel == p )
				{
					SPR_Set(activeSpr, r, g, b );
					SPR_DrawAdditive(0, x, y, &rcActive);

					SPR_Set(gHUD.GetSprite(m_HUD_selection), r, g, b );
					SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(m_HUD_selection));
				}
				else
				{
					// Draw Weapon if Red if no ammo

					if ( gWR.HasAmmo(p) )
						ScaleColors(r, g, b, 192);
					else
					{
						UnpackRGB(r,g,b, RGB_REDISH);
						ScaleColors(r, g, b, 128);
					}

					SPR_Set(inactiveSpr, r, g, b );
					SPR_DrawAdditive( 0, x, y, &rcInactive);
				}

				// Draw Ammo Bar

				DrawAmmoBar(p, x + giABWidth/2, y, giABWidth, giABHeight);
				
				y += p->rcActive.bottom - p->rcActive.top + 5;
			}

			x += iWidth + 5;

		}
		else
		{
			// Draw Row of weapons.

			UnpackRGB(r,g,b, RGB_YELLOWISH);

			for ( int iPos = 0; iPos < MAX_WEAPON_POSITIONS; iPos++ )
			{
				WEAPON *p = gWR.GetWeaponSlot( i, iPos );
				
				if ( !p || !p->iId )
					continue;

				if ( gWR.HasAmmo(p) )
				{
					UnpackRGB(r,g,b, RGB_YELLOWISH);
					a = 128;
				}
				else
				{
					UnpackRGB(r,g,b, RGB_REDISH);
					a = 96;
				}

				FillRGBA( x, y, giBucketWidth, giBucketHeight, r, g, b, a );

				y += giBucketHeight + 5;
			}

			x += giBucketWidth + 5;
		}
	}	

	return 1;

}


/* =================================
	GetSpriteList

Finds and returns the matching 
sprite name 'psz' and resolution 'iRes'
in the given sprite list 'pList'
iCount is the number of items in the pList
================================= */
client_sprite_t *GetSpriteList(client_sprite_t *pList, const char *psz, int iRes, int iCount)
{
	if (!pList)
		return NULL;

	int i = iCount;
	client_sprite_t *p = pList;
	
	while(i--)
	{
		if ((p->iRes == iRes) && (!strcmp(psz, p->szName)))
			return p;
		p++;
	}

	return NULL;
}
