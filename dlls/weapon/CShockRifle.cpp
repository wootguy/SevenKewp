/***
 *
 *	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
 *
 *	This product contains software technology licensed from Id
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/
#include "user_messages.h"

#ifndef CLIENT_DLL
#include "CShockBeam.h"
#endif

#include "CShockRifle.h"

/*
BEGIN_DATAMAP(CShockRifle)
// This isn't restored in the original
DEFINE_FIELD(m_flRechargeTime, FIELD_TIME),
	END_DATAMAP();
*/

LINK_ENTITY_TO_CLASS(weapon_shockrifle, CShockRifle)

void CShockRifle::Spawn()
{
	Precache();
	m_iId = WEAPON_SHOCKRIFLE;
	SET_MODEL(ENT(pev), GetModelW());
	m_iDefaultAmmo = SHOCKRIFLE_DEFAULT_GIVE;

	FallInit();// get ready to fall down.

	pev->sequence = 0;
	pev->animtime = gpGlobals->time;
	pev->framerate = 1;
}

void CShockRifle::Precache()
{
	// NOTE: these precaches are duplicated in monster_shockroach to prevent a precache loop
	// keep both classes in sync!

	m_defaultModelV = "models/v_shock.mdl";
	m_defaultModelP = "models/p_shock.mdl";
	m_defaultModelW = "models/w_shock_rifle.mdl";
	CBasePlayerWeapon::Precache();

	m_iSpriteTexture = PRECACHE_MODEL("sprites/shockwave.spr");
	
	PRECACHE_MODEL("sprites/lgtning.spr");
	PRECACHE_SOUND("weapons/shock_fire.wav");
	PRECACHE_SOUND("weapons/shock_draw.wav");
	PRECACHE_SOUND("weapons/shock_recharge.wav");
	PRECACHE_SOUND("weapons/shock_discharge.wav");
	m_waterExplodeSpr = PRECACHE_MODEL("sprites/xspark2.spr");

	//m_usShockRifle = PRECACHE_EVENT(1, "events/shock.sc");

	UTIL_PrecacheOther("shock_beam");
	UTIL_PrecacheOther("monster_shockroach");

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_shockrifle.txt");
}

BOOL CShockRifle::Deploy()
{
	/*
	if (g_Skill.GetValue("shockrifle_fast") != 0)
	{
		m_flRechargeTime = gpGlobals->time + 0.25;
	}
	else
	{
		m_flRechargeTime = gpGlobals->time + 0.5;
	}
	*/

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return FALSE;

	// don't reset recharge time to allow recharge when not held
	//m_flRechargeTime = gpGlobals->time + 0.25;

	RechargeAmmo(true);

	for (int i = 0; i < 3; i++) {
		UTIL_Remove(h_beams[i]);

		CBeam* beam = CBeam::BeamCreate("sprites/lgtning.spr", 8);
		beam->EntsInit(m_pPlayer->entindex(), m_pPlayer->entindex());
		beam->SetStartAttachment(1);
		beam->SetEndAttachment(2 + i);
		beam->SetColor(0, 253, 253);
		beam->SetNoise(64);
		beam->SetBrightness(0);
		h_beams[i] = beam;
	}

	return DefaultDeploy(GetModelV(), GetModelP(), SHOCKRIFLE_DRAW, "gauss");
}

void CShockRifle::Holster(int skiplocal)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_fInReload = false;

	SetThink(nullptr);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.0;

	SendWeaponAnim(SHOCKRIFLE_HOLSTER);
	
	for (int i = 0; i < 3; i++) {
		UTIL_Remove(h_beams[i]);
	}

	if (0 == m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 1;
	}
}

void CShockRifle::WeaponIdle()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	Reload();

	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flSoundDelay != 0 && gpGlobals->time >= m_flSoundDelay)
	{
		m_flSoundDelay = 0;
	}

	// This used to be completely broken. It used the current game time instead of the weapon time base, which froze the idle animation.
	// It also never handled IDLE3, so it only ever played IDLE1, and then only animated it when you held down secondary fire.
	// This is now fixed.
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;

	const float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0, 1);

	if (flRand <= 0.75)
	{
		iAnim = SHOCKRIFLE_IDLE3;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 51.0 / 15.0;
	}
	else
	{
		iAnim = SHOCKRIFLE_IDLE1;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 101.0 / 30.0;
	}

	SendWeaponAnim(iAnim);
}

void CShockRifle::BeamAttack(bool isSecondary) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD)
	{
		// Water goes zap.
		const float flVolume = RANDOM_FLOAT(0.8, 0.9);

		EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/shock_discharge.wav", flVolume, ATTN_NONE);

		const int ammoCount = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

		UTIL_Explosion(pev->origin, m_waterExplodeSpr, 30, 50, 2 | 4 | 8);

		RadiusDamage(pev->origin, m_pPlayer->pev, m_pPlayer->pev, ammoCount * 100, ammoCount * 150, CLASS_NONE, DMG_ALWAYSGIB | DMG_BLAST);

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 0;

		return;
	}

	Reload();

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;

	m_flRechargeTime = gpGlobals->time + 1.0;

	/*
	int flags;

#if defined(CLIENT_WEAPONS)
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT(flags, m_pPlayer->edict(), m_usShockRifle);
	*/

	SendWeaponAnim(SHOCKRIFLE_FIRE);
	EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/shock_fire.wav", 1, ATTN_NORM);

	ToggleChargeBeams(true);
	m_lastAttack = gpGlobals->time;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

#ifndef CLIENT_DLL
	Vector vecAnglesAim = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	
	if (isSecondary) {
		float x, y;
		GetCircularGaussianSpread(x, y);
		vecAnglesAim.x += x * 3;
		vecAnglesAim.y += y * 3;
		m_pPlayer->pev->punchangle.x = -2;
	}

	UTIL_MakeVectors(vecAnglesAim);

	Vector vecHead = m_pPlayer->GetGunPosition();

	Vector vecSrc = vecHead +
		gpGlobals->v_forward * 16 +
		gpGlobals->v_right * 9 +
		gpGlobals->v_up * -7;

	// adjust beam direction so that it lands in the center of the crosshair at the impact point
	// otherwise it will be slightly low and to the right
	TraceResult tr;
	UTIL_TraceLine(vecHead, vecHead + gpGlobals->v_forward * 4096, dont_ignore_monsters, edict(), &tr);
	Vector targetdir = (tr.vecEndPos - vecSrc).Normalize();
	Vector beamAngles = UTIL_VecToAngles(targetdir);
	beamAngles.x *= -1;

	// Update auto-aim
	//m_pPlayer->GetAutoaimVectorFromPoint(vecSrc, AUTOAIM_10DEGREES);
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	CShockBeam* pBeam = CShockBeam::CreateShockBeam(vecSrc, beamAngles, m_pPlayer);

	UTIL_SetOrigin(pBeam->m_hBeam1->pev, pBeam->pev->origin);
#endif

	/*
	if (g_Skill.GetValue("shockrifle_fast") != 0)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	}
	else
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
	}
	*/

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2f;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33;
}

void CShockRifle::PrimaryAttack()
{
	BeamAttack(false);
}

void CShockRifle::SecondaryAttack()
{
	BeamAttack(true);
}

void CShockRifle::Reload()
{
	RechargeAmmo(true);
}

void CShockRifle::ItemPostFrame()
{
	CBasePlayerWeapon::ItemPostFrame();

	Reload();

	if (gpGlobals->time - m_lastAttack > 0.1f) {
		ToggleChargeBeams(false);
	}
}

void CShockRifle::ToggleChargeBeams(bool enabled) {
	for (int i = 0; i < 3; i++) {
		CBeam* beam = (CBeam*)h_beams[i].GetEntity();
		if (beam) {
			beam->SetBrightness(enabled ? 255 : 0);
		}
	}
}

void CShockRifle::RechargeAmmo(bool bLoud)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int ammoCount = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
	bool didRecharge = false;

	while (ammoCount < SHOCKRIFLE_DEFAULT_GIVE && m_flRechargeTime < gpGlobals->time) {
		++ammoCount;
		didRecharge = true;

		/*
		if (g_Skill.GetValue("shockrifle_fast") != 0)
		{
			m_flRechargeTime += 0.25;
		}
		else
		{
			m_flRechargeTime += 0.5;
		}
		*/

		m_flRechargeTime += 0.25;
	}

	if (didRecharge) {
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/shock_recharge.wav", VOL_NORM, ATTN_NORM);
	}

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = ammoCount;
}

int CShockRifle::AddToPlayer(CBasePlayer* pPlayer)
{
	if (CBasePlayerWeapon::AddToPlayer(pPlayer))
	{
#ifndef CLIENT_DLL
		if (g_pGameRules->IsMultiplayer())
		{
			// in multiplayer, all hivehands come full. 
			pPlayer->m_rgAmmo[PrimaryAmmoIndex()] = SHOCKRIFLE_DEFAULT_GIVE;
		}
#endif
		return TRUE;
	}
	return FALSE;
}

void CShockRifle::UpdateOnRemove(void) {
	for (int i = 0; i < 3; i++) {
		UTIL_Remove(h_beams[i]);
	}
}

int CShockRifle::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_shockrifle";

	p->pszAmmo1 = "shock";
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 2; // should be 6 but that slot doesn't exist in vanilla HL
	p->iPosition = 3;
	p->iId = WEAPON_SHOCKRIFLE;
	p->iWeight = SHOCKRIFLE_WEIGHT;
	p->iFlags = ITEM_FLAG_NOAUTORELOAD | ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;

	p->fAccuracyDeg = 0;
	p->fAccuracyDeg2 = 6;
	p->iFlagsEx = WEP_FLAG_SECONDARY_ACCURACY;
	return 1;
}