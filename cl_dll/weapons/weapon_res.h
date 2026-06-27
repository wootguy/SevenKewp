#pragma once
#include "cdll_int.h"
#include "HashMap.h"

//
// Weapon inventory. Not predicted.
//

#define MAX_WEAPON_POSITIONS 8 // max number of items in each bucket
#define MAX_WEAPON_NAME 128
#define MAX_CUSTOM_HUD_ICONS 16
#define WEAPON_FLAGS_SELECTONEMPTY	1
#define WEAPON_IS_ONTARGET 0x40

typedef int AMMO;

struct CustomHudIcon {
	HSPRITE hSprite;
	wrect_t rect;
};

struct WEAPON
{
	char	szName[MAX_WEAPON_NAME];
	int		iAmmoType;
	int		iAmmo2Type;
	int		iMax1;
	int		iMax2;
	int		iSlot;
	int		iSlotPos;
	int		iFlags;
	int		iId;
	int		iClip;
	int		iMaxClip2;

	int		iCount;		// # of itesm in plist

	float accuracyX;
	float accuracyX2;
	float accuracyY;
	float accuracyY2;
	int iFlagsEx;

	HSPRITE hActive;
	wrect_t rcActive;
	HSPRITE hInactive;
	wrect_t rcInactive;
	HSPRITE hAkimboActive;
	wrect_t rcAkimboActive;
	HSPRITE hAkimboInactive;
	wrect_t rcAkimboInactive;
	HSPRITE	hAmmo;
	wrect_t rcAmmo;
	HSPRITE hAmmo2;
	wrect_t rcAmmo2;
	HSPRITE hCrosshair;
	wrect_t rcCrosshair;
	HSPRITE hAutoaim;
	wrect_t rcAutoaim;
	HSPRITE hZoomedCrosshair;
	wrect_t rcZoomedCrosshair;
	HSPRITE hZoomedAutoaim;
	wrect_t rcZoomedAutoaim;

	CustomHudIcon customIcons[MAX_CUSTOM_HUD_ICONS];
	int numCustomIcons;
};

class WeaponsResource
{
private:
	// Information about weapons & ammo
	WEAPON rgWeapons[MAX_WEAPONS];	// Weapons Array

	// The slots currently in use by weapons. The value is a pointer to the weapon;
	// if it's NULL, no weapon is there.
	WEAPON* rgSlots[MAX_WEAPON_SLOTS + 1][MAX_WEAPON_POSITIONS + 1];
	int riAmmo[MAX_AMMO_TYPES];							// count of each ammo type

public:
	void Init(void)
	{
		memset(rgWeapons, 0, sizeof rgWeapons);
		Reset();
	}

	void Reset(void)
	{
		iOldWeaponBits = 0;
		memset(rgSlots, 0, sizeof rgSlots);
		memset(riAmmo, 0, sizeof riAmmo);
	}

	///// WEAPON /////
	uint64_t	iOldWeaponBits;

	WEAPON* GetWeapon(int iId) { return &rgWeapons[iId]; }
	void AddWeapon(WEAPON* wp)
	{
		rgWeapons[wp->iId] = *wp;
		LoadWeaponSprites(&rgWeapons[wp->iId]);
	}

	void PickupWeapon(WEAPON* wp)
	{
		rgSlots[wp->iSlot][wp->iSlotPos] = wp;
	}

	void DropWeapon(WEAPON* wp)
	{
		rgSlots[wp->iSlot][wp->iSlotPos] = NULL;
	}

	void DropAllWeapons(void)
	{
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			if (rgWeapons[i].iId)
				DropWeapon(&rgWeapons[i]);
		}
	}

	WEAPON* GetWeaponSlot(int slot, int pos) { return rgSlots[slot][pos]; }

	void LoadWeaponSprites(WEAPON* wp);
	void LoadAllWeaponSprites(void);
	WEAPON* GetFirstPos(int iSlot);
	WEAPON* GetNextActivePos(int iSlot, int iSlotPos);

	int HasAmmo(WEAPON* p);

	///// AMMO /////
	AMMO GetAmmo(int iId) { return iId; }

	void SetAmmo(int iId, int iCount) { riAmmo[iId] = iCount; }

	int CountAmmo(int iId);

	HSPRITE* GetAmmoPicFromWeapon(int iAmmoId, wrect_t& rect);
};

client_sprite_t* GetSpriteList(client_sprite_t* pList, const char* psz, int iRes, int iCount);
void HookWeaponMessages();

extern WeaponsResource gWR;

// maps a weapon classname to a custom sprite dir
// done globally because messages reset and mix the weapon pointers too much
extern StringMap g_weaponHudDirs;

extern WEAPON* g_pActiveWeapon;