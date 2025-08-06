#pragma once
#include "CBasePlayerWeapon.h"

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

class CM249 : public CBasePlayerWeapon
{
public:
	void Spawn() override;

	void Precache() override;

	BOOL Deploy() override;

	void Holster(int skiplocal = 0) override;

	void WeaponIdle() override;

	void PrimaryAttack() override;

	void Reload() override;

	int GetItemInfo(ItemInfo* p);

	BOOL UseDecrement() override
	{
#if defined(CLIENT_WEAPONS)
		return true;
#else
		return false;
#endif
	}

	BOOL IsClientWeapon() { return FALSE; }

	void EV_FireM249(struct event_args_s* args);

private:
	static int RecalculateBody(int iClip);

private:
	float m_flNextAnimTime;

	int m_iShell;

	// Used to alternate between ejecting shells and links.
	bool m_bAlternatingEject = false;
	int m_iLink;
	int m_iSmoke;
	int m_iFire;

	bool m_bReloading;
	float m_flReloadStartTime;
	float m_flReloadStart;
};
