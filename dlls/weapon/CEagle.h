#pragma once
#include "CWeaponCustom.h"

#define EAGLE_DEFAULT_GIVE 7
#define EAGLE_MAX_CLIP 7

enum EagleAnim
{
	EAGLE_IDLE1 = 0,
	EAGLE_IDLE2,
	EAGLE_IDLE3,
	EAGLE_IDLE4,
	EAGLE_IDLE5,
	EAGLE_SHOOT,
	EAGLE_SHOOT_EMPTY,
	EAGLE_RELOAD,
	EAGLE_RELOAD_NOSHOT,
	EAGLE_DRAW,
	EAGLE_HOLSTER,
};

class CEagle : public CWeaponCustom
{
public:
	void Spawn() override;
	void Precache() override;
	void PrecacheEvents() override;
	int GetItemInfo(ItemInfo* p);
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount);
	const char* DisplayName() override { return "Desert Eagle"; }
	const char* GetDeathNoticeWeapon() { return "weapon_357"; }
};