#include "CBasePlayerWeapon.h"

// a dirty hack for an inventory menu without custom binds and text menus

enum WeaponInventoryErrors {
	INV_ERROR_NONE,
	INV_ERROR_CANT_ACTIVATE_EVER,		// can't activate item because it isn't activatable
	INV_ERROR_CANT_ACTIVATE_ACTIVE,		// can't activate item because it is already active
	INV_ERROR_CANT_ACTIVATE_COOLDOWN,	// can't activate item because it was activated too recently
	INV_ERROR_CANT_ACTIVATE_LIMIT,		// can't activate item because it was activated too many times
	INV_ERROR_CANT_DROP,				// item can't be dropped manually
	INV_ERROR_ACTIVATED,				// item was activated
};

class CWeaponInventory : public CBasePlayerWeapon
{
public:
	void Spawn();
	void Precache(void);
	BOOL Deploy();
	void Reload(void);
	void Holster(int skiplocal = 0);
	void WeaponIdle(void);
	void PrimaryAttack();
	void SecondaryAttack();
	int GetItemInfo(ItemInfo* p);
	BOOL IsClientWeapon() { return FALSE; }
	virtual const char* GetDeathNoticeWeapon() { return "weapon_inventory"; }

	char m_previousHudText[512];
	float m_lastHudUpdate;
	float m_nextAction;

	int m_itemIdx;
	int m_errorCode; // used to add error text to the hud menu
	string_t m_errorText; // can be used in place of a code
	float m_errorTime; // time an error code was set
	void SetError(int errorCode);
	void SetError(const char* errorText);

	void UpdateInventoryHud();

	BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}	
};