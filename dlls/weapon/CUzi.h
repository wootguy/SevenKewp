#pragma once
#include "CWeaponCustom.h"

#define UZI_DEFAULT_GIVE 32
#define UZI_MAX_CLIP 32

enum UziAnim
{
	UZI_IDLE1 = 0,
	UZI_IDLE2,
	UZI_DEPLOY,
	UZI_DEPLOY2,
	UZI_SHOOT,
	UZI_RELOAD,
	UZI_AKIMBO_IDLE,
	UZI_AKIMBO_PULL,
	UZI_AKIMBO_DEPLOY,
	UZI_AKIMBO_DEPLOY2,
	UZI_AKIMBO_HOLSTER,
	UZI_AKIMBO_RELOAD_RIGHT,
	UZI_AKIMBO_FIRE_RIGHT,
};

class CUzi : public CWeaponCustom
{
public:
	void Spawn() override;
	void Precache() override;
	void PrecacheEvents() override;
	int GetItemInfo(ItemInfo* p);
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount);
	const char* DisplayName() override { return "Uzi"; }
};