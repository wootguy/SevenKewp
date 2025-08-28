#pragma once
#include "CWeaponCustom.h"

#define MINIGUN_DEFAULT_GIVE 100

enum MinigunAnim
{
	MINIGUN_IDLE = 0,
	MINIGUN_IDLE2,
	MINIGUN_GENTLEIDLE,
	MINIGUN_STILLIDLE,
	MINIGUN_DRAW,
	MINIGUN_HOLSTER,
	MINIGUN_SPINUP,
	MINIGUN_SPINDOWN,
	MINIGUN_SPINIDLE,
	MINIGUN_SPINFIRE,
	MINIGUN_SPINIDLEDOWN,
};

enum MinigunPmodelAnim {
	MINIGUN_P_IDLE = 0,
	MINIGUN_P_SPIN
};

class CMinigun : public CWeaponCustom
{
public:
	void Spawn() override;
	void Precache() override;
	void PrecacheEvents() override;
	int GetItemInfo(ItemInfo* p);
	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount);
	const char* DisplayName() override { return "Minigun"; }
	const char* GetDeathNoticeWeapon() { return "weapon_9mmAR"; }
};
