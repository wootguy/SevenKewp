#pragma once
#include "CWeaponCustom.h"

#define M16_DEFAULT_GIVE 30
#define M16_MAX_CLIP 30

enum M16Anim
{
	M16_DRAW = 0,
	M16_HOLSTER,
	M16_IDLE,
	M16_FIDGET,
	M16_SHOOT_1,
	M16_SHOOT_2,
	M16_RELOAD_M16,
	M16_LAUNCH,
	M16_RELOAD_M203,
};

class CM16 : public CWeaponCustom
{
public:
	void Spawn() override;
	void Precache() override;
	void PrecacheEvents() override;
	int GetItemInfo(ItemInfo* p);
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount);
	const char* DisplayName() override { return "M16"; }
	const char* GetDeathNoticeWeapon() { return "weapon_9mmAR"; }
	virtual int MergedModelBody() { return MERGE_MDL_W_M16; }
};
