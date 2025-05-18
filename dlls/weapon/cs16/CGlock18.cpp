#include	"extdll.h"
#include	"util.h"
#include "CGlock18.h"

int m_usFireGlock18;

LINK_ENTITY_TO_CLASS(weapon_glock18, CGLOCK18)

#define HUD_PRINTCENTER 4
void ClientPrint(entvars_t* client, int msg_dest, const char* msg_name, const char* param1 = nullptr, const char* param2 = nullptr, const char* param3 = nullptr, const char* param4 = nullptr);
void ClientPrint(entvars_t* client, int msg_dest, const char* msg_name, const char* param1, const char* param2, const char* param3, const char* param4)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgTextMsg, nullptr, client);
	WRITE_BYTE(msg_dest);
	WRITE_STRING(msg_name);
	if (param1)
		WRITE_STRING(param1);
	if (param2)
		WRITE_STRING(param2);
	if (param3)
		WRITE_STRING(param3);
	if (param4)
		WRITE_STRING(param4);
	MESSAGE_END();
}

void CGLOCK18::Spawn()
{
	Precache();

	m_iId = WEAPON_GLOCK18;
	SET_MODEL(edict(), "models/w_glock18.mdl");

	//m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_iDefaultAmmo = GLOCK18_DEFAULT_GIVE;
	m_bBurstFire = false;

	m_iGlock18ShotsFired = 0;
	m_flGlock18Shoot = 0;
	m_flAccuracy = 0.9f;

#ifdef REGAMEDLL_API
	CSPlayerWeapon()->m_flBaseDamage = GLOCK18_DAMAGE;
#endif

	// Get ready to fall down
	FallInit();

	// extend
	CBasePlayerWeapon::Spawn();
}

void CGLOCK18::Precache()
{
	m_hasHandModels = false;
	m_defaultModelV = "models/v_glock18.mdl";
	m_defaultModelP = "models/p_glock18.mdl"; // TODO: no cs p models?
	m_defaultModelW = "models/w_glock18.mdl";
	CBasePlayerWeapon::Precache();

	//PRECACHE_MODEL("models/shield/v_shield_glock18.mdl");
	PRECACHE_SOUND("weapons/glock18-1.wav");
	PRECACHE_SOUND("weapons/glock18-2.wav");
	PRECACHE_SOUND("weapons/clipout1.wav");
	PRECACHE_SOUND("weapons/clipin1.wav");
	PRECACHE_SOUND("weapons/sliderelease1.wav");
	PRECACHE_SOUND("weapons/slideback1.wav");
	PRECACHE_SOUND("weapons/357_cock1.wav");
	PRECACHE_SOUND("weapons/de_clipin.wav");
	PRECACHE_SOUND("weapons/de_clipout.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	
	PrecacheEvents();
}

void CGLOCK18::PrecacheEvents() {
	PRECACHE_GENERIC("events/glock18.sc");
	m_usFireGlock18 = PRECACHE_EVENT(1, "events/glock18.sc");
}

int CGLOCK18::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = MAX_AMMO_9MM_CS16;
	p->pszAmmo2 = nullptr;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GLOCK18_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_GLOCK18;
	p->iFlags = 0;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

BOOL CGLOCK18::Deploy()
{
	//m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	m_bBurstFire = false;
	m_iGlock18ShotsFired = 0;
	m_flGlock18Shoot = 0;
	m_flAccuracy = 0.9f;
	//m_fMaxSpeed = GLOCK18_MAX_SPEED;

	
	/*
	m_pPlayer->m_bShieldDrawn = false;
	
	if (m_pPlayer->HasShield())
	{
		m_iWeaponState &= ~WPNSTATE_GLOCK18_BURST_MODE;
		return DefaultDeploy("models/shield/v_shield_glock18.mdl", "models/shield/p_shield_glock18.mdl", GLOCK18_SHIELD_DRAW, "shieldgun", UseDecrement() != FALSE);
	}
	else if (RANDOM_LONG(0, 1))
	{
		return DefaultDeploy("models/v_glock18.mdl", "models/p_glock18.mdl", GLOCK18_DRAW, "onehanded", UseDecrement() != FALSE);
	}
	*/

	if (RANDOM_LONG(0, 1))
		return DefaultDeploy("models/v_glock18.mdl", "models/p_glock18.mdl", GLOCK18_DRAW, "onehanded", UseDecrement() != FALSE);

	return DefaultDeploy("models/v_glock18.mdl", "models/p_glock18.mdl", GLOCK18_DRAW2, "onehanded", UseDecrement() != FALSE);
}

void CGLOCK18::SecondaryAttack()
{
	/*
	if (ShieldSecondaryFire(GLOCK18_SHIELD_UP, GLOCK18_SHIELD_DOWN))
	{
		return;
	}
	*/
	CBasePlayer* plr = GetPlayer();
	if (!plr)
		return;

	if (m_iWeaponState & WPNSTATE_GLOCK18_BURST_MODE)
	{
		ClientPrint(plr->pev, HUD_PRINTCENTER, "#Switch_To_SemiAuto");
		m_iWeaponState &= ~WPNSTATE_GLOCK18_BURST_MODE;
	}
	else
	{
		ClientPrint(plr->pev, HUD_PRINTCENTER, "#Switch_To_BurstFire");
		m_iWeaponState |= WPNSTATE_GLOCK18_BURST_MODE;
	}

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3f;
}

void CGLOCK18::PrimaryAttack()
{
	CBasePlayer* plr = GetPlayer();
	if (!plr)
		return;

	if (m_iWeaponState & WPNSTATE_GLOCK18_BURST_MODE)
	{
		if (!(plr->pev->flags & FL_ONGROUND))
		{
			GLOCK18Fire(1.2 * (1 - m_flAccuracy), 0.5, TRUE);
		}
		else if (plr->pev->velocity.Length2D() > 0)
		{
			GLOCK18Fire(0.185 * (1 - m_flAccuracy), 0.5, TRUE);
		}
		else if (plr->pev->flags & FL_DUCKING)
		{
			GLOCK18Fire(0.095 * (1 - m_flAccuracy), 0.5, TRUE);
		}
		else
		{
			GLOCK18Fire(0.3 * (1 - m_flAccuracy), 0.5, TRUE);
		}
	}
	else
	{
		if (!(plr->pev->flags & FL_ONGROUND))
		{
			GLOCK18Fire(1.0 * (1 - m_flAccuracy), 0.2, FALSE);
		}
		else if (plr->pev->velocity.Length2D() > 0)
		{
			GLOCK18Fire(0.165 * (1 - m_flAccuracy), 0.2, FALSE);
		}
		else if (plr->pev->flags & FL_DUCKING)
		{
			GLOCK18Fire(0.075 * (1 - m_flAccuracy), 0.2, FALSE);
		}
		else
		{
			GLOCK18Fire(0.1 * (1 - m_flAccuracy), 0.2, FALSE);
		}
	}
}

void CGLOCK18::GLOCK18Fire(float flSpread, float flCycleTime, BOOL bFireBurst)
{
	CBasePlayer* plr = GetPlayer();
	if (!plr)
		return;

	Vector vecAiming, vecSrc, vecDir;
	int flag;

	if (bFireBurst)
	{
		m_iGlock18ShotsFired = 0;
	}
	else
	{
		if (++m_iShotsFired > 1)
		{
			return;
		}

		flCycleTime -= 0.05f;
	}

	if (m_flLastFire)
	{
		// Mark the time of this shot and determine the accuracy modifier based on the last shot fired...
		m_flAccuracy -= (0.325f - (gpGlobals->time - m_flLastFire)) * 0.275f;

		if (m_flAccuracy > 0.9f)
		{
			m_flAccuracy = 0.9f;
		}
		else if (m_flAccuracy < 0.6f)
		{
			m_flAccuracy = 0.6f;
		}
	}

	m_flLastFire = gpGlobals->time;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = GetNextAttackDelay(0.2);
		}

		/*
		if (TheBots)
		{
			TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
		}
		*/

		return;
	}

	m_iClip--;
	plr->pev->effects |= EF_MUZZLEFLASH;

	// player "shoot" animation
	plr->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(plr->pev->v_angle + plr->pev->punchangle);

	// non-silenced
	plr->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	plr->m_iWeaponFlash = NORMAL_GUN_FLASH;

	vecSrc = plr->GetGunPosition();
	vecAiming = gpGlobals->v_forward;

#ifdef REGAMEDLL_API
	float flBaseDamage = CSPlayerWeapon()->m_flBaseDamage;
#else
	float flBaseDamage = GLOCK18_DAMAGE;
#endif
	vecDir = plr->FireBulletsCS16(vecSrc, vecAiming, flSpread, 8192, 1, BULLET_PLAYER_9MM, flBaseDamage, GLOCK18_RANGE_MODIFER, plr->pev, true, plr->random_seed);

#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif

	PLAYBACK_EVENT_FULL(flag, plr->edict(), m_usFireGlock18, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y,
		int(plr->pev->punchangle.x * 100), int(plr->pev->punchangle.y * 100), m_iClip == 0, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(flCycleTime);

	if (!m_iClip && plr->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		// HEV suit - indicate out of ammo condition
		plr->SetSuitUpdate("!HEV_AMO0", SUIT_SENTENCE, SUIT_REPEAT_OK);
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;

	if (bFireBurst)
	{
		// Fire off the next two rounds
		m_iGlock18ShotsFired++;
		m_flGlock18Shoot = gpGlobals->time + 0.1f;
	}

#ifdef REGAMEDLL_ADD
	KickBack(0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0); // dummy call, API useful
#endif
	//ResetPlayerShieldAnim();
}

void CGLOCK18::Reload()
{
	CBasePlayer* plr = GetPlayer();
	if (!plr)
		return;

	int iResult;
	if (plr->ammo_9mm <= 0)
		return;

	//if (plr->HasShield())
	//	iResult = GLOCK18_SHIELD_RELOAD;
	if (RANDOM_LONG(0, 1))
		iResult = GLOCK18_RELOAD;
	else
		iResult = GLOCK18_RELOAD2;

	if (DefaultReload(iMaxClip(), iResult, GLOCK18_RELOAD_TIME))
	{
		plr->SetAnimation(PLAYER_RELOAD);
		m_flAccuracy = 0.9;
	}
}

void CGLOCK18::WeaponIdle()
{
	CBasePlayer* plr = GetPlayer();
	if (!plr)
		return;

	int iAnim;
	float flRand;

	ResetEmptySound();
	plr->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
	{
		return;
	}
	
	/*
	if (plr->HasShield())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0f;

		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		{
			SendWeaponAnim(GLOCK18_SHIELD_IDLE_UP, UseDecrement() != FALSE);
		}
	}
	// only idle if the slid isn't back
	else if (m_iClip)
	*/
	if (m_iClip)
	{
		flRand = RANDOM_FLOAT(0, 1);

		if (flRand <= 0.3f)
		{
			iAnim = GLOCK18_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0625f;
		}
		else if (flRand <= 0.6f)
		{
			iAnim = GLOCK18_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.75f;
		}
		else
		{
			iAnim = GLOCK18_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5f;
		}

		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
	}
}
