#pragma once
#include "CWeaponCustom.h"

#define M249_DEFAULT_GIVE 50
#define M249_MAX_CLIP 50

enum M249Anim
{
	M249_SLOWIDLE = 0,
	M249_IDLE2,
	M249_RELOAD_START,
	M249_RELOAD_END,
	M249_HOLSTER,
	M249_DRAW,
	M249_SHOOT1,
	M249_SHOOT2,
	M249_SHOOT3
};

class CM249 : public CWeaponCustom
{
public:
	void Spawn() override;
	void Precache() override;
	void PrecacheEvents() override;
	int GetItemInfo(ItemInfo* p);
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount);
	const char* DisplayName() override { return "M249"; }
};
