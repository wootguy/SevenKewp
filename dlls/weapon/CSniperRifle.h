#pragma once
#include "CWeaponCustom.h"

#define M40A1_DEFAULT_GIVE 5
#define M40A1_MAX_CLIP 5

enum M40A1Anim
{
	M40A1_DRAW = 0,
	M40A1_SLOWIDLE,
	M40A1_FIRE,
	M40A1_FIRE_LAST,
	M40A1_RELOAD1,
	M40A1_RELOAD2,
	M40A1_RELOAD3,
	M40A1_SLOWIDLE2,
	M40A1_HOLSTER,
};

class CSniperRifle : public CWeaponCustom
{
public:
	void Spawn() override;
	void Precache() override;
	void PrecacheEvents() override;
	int GetItemInfo(ItemInfo* p);
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount);
	const char* DisplayName() override { return "M40A1"; }
	const char* GetDeathNoticeWeapon() { return "weapon_crossbow"; }
};