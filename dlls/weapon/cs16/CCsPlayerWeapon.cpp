#include "CCsPlayerWeapon.h"

void CCsPlayerWeapon::ItemPostFrame() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int usableButtons = m_pPlayer->pev->button;

	if (!HasSecondaryAttack())
	{
		usableButtons &= ~IN_ATTACK2;
	}


	if (m_flGlock18Shoot != 0)
	{
		FireRemaining(m_iGlock18ShotsFired, m_flGlock18Shoot, TRUE);
	}
	/*
	else if (gpGlobals->time > m_flFamasShoot && m_flFamasShoot != 0)
	{
		FireRemaining(m_iFamasShotsFired, m_flFamasShoot, FALSE);
	}
	*/

	// Return zoom level back to previous zoom level before we fired a shot.
	// This is used only for the AWP and Scout
	if (m_flNextPrimaryAttack <= UTIL_WeaponTimeBase())
	{
		if (m_pPlayer->m_bResumeZoom)
		{
			m_pPlayer->m_iFOV = m_pPlayer->m_iLastZoom;
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV;

			if (m_pPlayer->m_iFOV == m_pPlayer->m_iLastZoom)
			{
				// return the fade level in zoom.
				m_pPlayer->m_bResumeZoom = false;
			}
		}
	}

	if (m_pPlayer->m_flEjectBrass != 0 && m_pPlayer->m_flEjectBrass <= gpGlobals->time)
	{
		m_pPlayer->m_flEjectBrass = 0;
		EjectBrassLate();
	}

	if (!(m_pPlayer->pev->button & IN_ATTACK))
	{
		m_flLastFireTime = 0;
	}

	/*
	if (m_pPlayer->HasShield())
	{
		if (m_fInReload && (m_pPlayer->pev->button & IN_ATTACK2))
		{
			SecondaryAttack();
			m_pPlayer->pev->button &= ~IN_ATTACK2;
			m_fInReload = FALSE;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase();
		}
	}
	*/

	if (m_fInReload && m_pPlayer->m_flNextAttack <= UTIL_WeaponTimeBase())
	{
		// complete the reload.
		int j = V_min(iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

		// Add them to the clip
		m_iClip += j;

#ifdef REGAMEDLL_ADD
		// Do not remove bpammo of the player,
		// if cvar allows to refill bpammo on during reloading the weapons
		if (refill_bpammo_weapons.value < 3.0f)
#endif
		{
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
		}

		m_pPlayer->TabulateAmmo();
		m_fInReload = FALSE;
	}

	if ((usableButtons & IN_ATTACK2) && CanAttack(m_flNextSecondaryAttack, UTIL_WeaponTimeBase(), UseDecrement())
#ifdef REGAMEDLL_FIXES
		&& !m_pPlayer->m_bIsDefusing // In-line: I think it's fine to block secondary attack, when defusing. It's better then blocking speed resets in weapons.
#endif
		)
	{
		if (pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()])
		{
			m_fFireOnEmpty = TRUE;
		}

		SecondaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if ((m_pPlayer->pev->button & IN_ATTACK) && CanAttack(m_flNextPrimaryAttack, UTIL_WeaponTimeBase(), UseDecrement()))
	{
		if ((m_iClip == 0 && pszAmmo1()) || (iMaxClip() == WEAPON_NOCLIP && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
		{
			m_fFireOnEmpty = TRUE;
		}

		m_pPlayer->TabulateAmmo();

		// Can't shoot during the freeze period
		// Always allow firing in single player
		if (
#ifdef REGAMEDLL_API
			m_pPlayer->CSPlayer()->m_bCanShootOverride ||
#endif
			//(m_pPlayer->m_bCanShoot && g_pGameRules->IsMultiplayer() && !g_pGameRules->IsFreezePeriod() && !m_pPlayer->m_bIsDefusing) || !g_pGameRules->IsMultiplayer())
			(m_pPlayer->m_bCanShoot && g_pGameRules->IsMultiplayer() && !m_pPlayer->m_bIsDefusing) || !g_pGameRules->IsMultiplayer())
		{
			// don't fire underwater
			if (m_pPlayer->pev->waterlevel == 3 && (iFlags() & ITEM_FLAG_NOFIREUNDERWATER))
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = GetNextAttackDelay(0.15);
			}
			else
			{
				PrimaryAttack();
			}
		}
	}
	else if ((m_pPlayer->pev->button & IN_RELOAD) && iMaxClip() != WEAPON_NOCLIP && !m_fInReload && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		if (m_flFamasShoot == 0 && m_flGlock18Shoot == 0)
		{
			//if (!(m_iWeaponState & WPNSTATE_SHIELD_DRAWN))
			{
				// reload when reload is pressed, or if no buttons are down and weapon is empty.
				Reload();
			}
		}
	}
	else if (!(usableButtons & (IN_ATTACK | IN_ATTACK2)))
	{
		// no fire buttons down

		// The following code prevents the player from tapping the firebutton repeatedly
		// to simulate full auto and retaining the single shot accuracy of single fire
		if (m_bDelayFire)
		{
			m_bDelayFire = false;

			if (m_iShotsFired > 15)
			{
				m_iShotsFired = 15;
			}

			m_flDecreaseShotsFired = gpGlobals->time + 0.4f;
		}

		m_fFireOnEmpty = FALSE;

		// if it's a pistol then set the shots fired to 0 after the player releases a button
		//if (IsSecondaryWeapon(m_iId))
		if (IsSecondaryWeapon())
		{
			m_iShotsFired = 0;
		}
		else
		{
			if (m_iShotsFired > 0 && m_flDecreaseShotsFired < gpGlobals->time)
			{
				m_flDecreaseShotsFired = gpGlobals->time + 0.0225f;
				m_iShotsFired--;

#ifdef REGAMEDLL_FIXES
				// Reset accuracy
				if (m_iShotsFired == 0)
				{
					m_flAccuracy = GetBaseAccuracy((WeaponIdType)m_iId);
				}
#endif
			}
		}

		if (!IsUseable() && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
		{
#if 0
			// weapon isn't useable, switch.
			if (!(iFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon(m_pPlayer, this))
			{
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3f;
				return;
			}
#endif
		}
		else
		{
			//if (!(m_iWeaponState & WPNSTATE_SHIELD_DRAWN))
			{
				// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
				if (!m_iClip && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
				{
					if (m_flFamasShoot == 0 && m_flGlock18Shoot == 0)
					{
						Reload();
						return;
					}
				}
			}
		}

#ifdef BUILD_LATEST
		HandleInfiniteAmmo();
#endif

		WeaponIdle();
		return;
	}

#ifdef BUILD_LATEST
	HandleInfiniteAmmo();
#endif

	// catch all
	if (ShouldWeaponIdle())
	{
		WeaponIdle();
	}
}

void CCsPlayerWeapon::EjectBrassLate()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int soundType;
	Vector vecUp, vecRight, vecShellVelocity;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	vecUp = RANDOM_FLOAT(100, 150) * gpGlobals->v_up;
	vecRight = RANDOM_FLOAT(50, 70) * gpGlobals->v_right;

	vecShellVelocity = (m_pPlayer->pev->velocity + vecRight + vecUp) + gpGlobals->v_forward * 25;
	//soundType = (m_iId == WEAPON_XM1014 || m_iId == WEAPON_M3) ? 2 : 1;
	soundType = 1;

	//EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -9 + gpGlobals->v_forward * 16, gpGlobals->v_right * -9,
	//	vecShellVelocity, pev->angles.y, m_iShell, soundType, m_pPlayer->entindex());

	EjectBrass(pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_up * -9 + gpGlobals->v_forward * 16,
		vecShellVelocity, pev->angles.y, m_iShell, soundType);
}

void CCsPlayerWeapon::FireRemaining(int& shotsFired, float& shootTime, BOOL bIsGlock)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	float nexttime = 0.1f;
	if (--m_iClip < 0)
	{
		m_iClip = 0;
		shotsFired = 3;
		shootTime = 0;
		return;
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	int flag;
#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif

#ifdef REGAMEDLL_API
	float flBaseDamage = CSPlayerWeapon()->m_flBaseDamage;
#else
	//float flBaseDamage = bIsGlock ? GLOCK18_DAMAGE : FAMAS_DAMAGE;
	float flBaseDamage = GLOCK18_DAMAGE;
#endif

	if (bIsGlock)
	{
		vecDir = m_pPlayer->FireBulletsCS16(vecSrc, gpGlobals->v_forward, 0.05, 8192, 1, BULLET_PLAYER_9MM, flBaseDamage, 0.9, m_pPlayer->pev, true, m_pPlayer->random_seed);
#ifndef REGAMEDLL_FIXES
		--m_pPlayer->ammo_9mm;
#endif
		PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireGlock18, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y,
			int(m_pPlayer->pev->punchangle.x * 10000), int(m_pPlayer->pev->punchangle.y * 10000), m_iClip == 0, FALSE);
	}
	else
	{
		//vecDir = m_pPlayer->FireBulletsCS16(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, 2, BULLET_PLAYER_556MM, flBaseDamage, 0.96, m_pPlayer->pev, false, m_pPlayer->random_seed);
#ifndef REGAMEDLL_FIXES
		//--m_pPlayer->ammo_556nato;
#endif

#ifdef REGAMEDLL_ADD
		// HACKHACK: client-side weapon prediction fix
		if (!(iFlags() & ITEM_FLAG_NOFIREUNDERWATER) && m_pPlayer->pev->waterlevel == 3)
			flag = 0;
#endif

		//PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireFamas, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y,
		//	int(m_pPlayer->pev->punchangle.x * 10000000), int(m_pPlayer->pev->punchangle.y * 10000000), FALSE, FALSE);
	}

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	if (++shotsFired != 3)
	{
		shootTime = gpGlobals->time + nexttime;
	}
	else
		shootTime = 0;
}

bool CCsPlayerWeapon::HasSecondaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

#ifdef REGAMEDLL_API
	if (CSPlayerWeapon()->m_iStateSecondaryAttack != WEAPON_SECONDARY_ATTACK_NONE)
	{
		switch (CSPlayerWeapon()->m_iStateSecondaryAttack)
		{
		case WEAPON_SECONDARY_ATTACK_SET:
			return true;
		case WEAPON_SECONDARY_ATTACK_BLOCK:
			return false;
		default:
			break;
		}
	}
#endif

	/*
	if (m_pPlayer && m_pPlayer->HasShield())
	{
		return true;
	}
	*/

	/*
	switch (m_iId)
	{
	case WEAPON_AK47:
	case WEAPON_XM1014:
	case WEAPON_MAC10:
	case WEAPON_ELITE:
	case WEAPON_FIVESEVEN:
	case WEAPON_MP5N:
#ifdef BUILD_LATEST_FIXES
	case WEAPON_UMP45:
#endif
	case WEAPON_M249:
	case WEAPON_M3:
	case WEAPON_TMP:
	case WEAPON_DEAGLE:
	case WEAPON_P228:
	case WEAPON_P90:
	case WEAPON_C4:
	case WEAPON_GALIL:
		return false;
	default:
		break;
	}
	*/

	return true;
}