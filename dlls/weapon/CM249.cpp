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
#include "CM249.h"

LINK_ENTITY_TO_CLASS(weapon_m249, CM249)
LINK_ENTITY_TO_CLASS(weapon_saw, CM249)

void CM249::Spawn()
{
	pev->classname = MAKE_STRING("weapon_m249"); // hack to allow for old names
	Precache();
	SetWeaponModelW();
	m_iId = WEAPON_M249;
	m_iDefaultAmmo = M249_DEFAULT_GIVE;
	FallInit();
}

void CM249::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_saw.mdl";
	m_defaultModelP = "models/p_saw.mdl";
	m_defaultModelW = "models/w_saw.mdl";
	CBasePlayerWeapon::Precache();

	int m_iShell = PRECACHE_MODEL("models/saw_shell.mdl");
	int m_iLink = PRECACHE_MODEL("models/saw_link.mdl");
	int shootSnd = PRECACHE_SOUND("weapons/saw_fire1.wav");
	int reloadSnd1 = PRECACHE_SOUND("weapons/saw_reload.wav");
	int reloadSnd2 = PRECACHE_SOUND("weapons/saw_reload2.wav");

	animExt = "saw";
	wrongClientWeapon = "weapon_9mmAR";

	params.flags = FL_WC_WEP_HAS_PRIMARY;
	params.vmodel = MODEL_INDEX(GetModelV());
	params.deployAnim = M249_DRAW;
	params.maxClip = M249_MAX_CLIP;
	params.reloadStage[0] = { M249_RELOAD_START, 1330 + 2470 };
	params.idles[0] = { M249_SLOWIDLE, 95, 5000 };
	params.idles[1] = { M249_IDLE2, 5, 6160 };

	CustomWeaponShootOpts& primary = params.shootOpts[0];
	primary.ammoCost = 1;
	primary.cooldown = 67;

	float spread = VECTOR_CONE_6DEGREES.x;
	int bulletf = FL_WC_BULLETS_MUZZLE_FLASH | FL_WC_BULLETS_DYNAMIC_SPREAD;
	int btype = BULLET_PLAYER_9MM;

	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).WepAnim(M249_SHOOT1, M249_SHOOT1 + 2));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_556));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).PunchRandom(2, 1));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).Kickback(35));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).Bullets(1, gSkillData.sk_plr_556_bullet, spread, spread, btype, 1, bulletf));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_EVEN).EjectShell(m_iShell, 14, -12, 4));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_ODD).EjectShell(m_iLink, 14, -12, 4));

	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 0, 0).SetBody(8));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 0, 1).SetBody(7));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 0, 2).SetBody(6));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 0, 3).SetBody(5));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 0, 4).SetBody(4));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 0, 5).SetBody(3));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 0, 6).SetBody(2));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 0, 7).SetBody(1));

	AddEvent(WepEvt(WC_TRIG_RELOAD, 16).PlaySound(reloadSnd1, CHAN_WEAPON, 1.0f, ATTN_IDLE, 100));
	AddEvent(WepEvt(WC_TRIG_RELOAD, 1330).PlaySound(reloadSnd2, CHAN_WEAPON, 1.0f, ATTN_IDLE, 100));
	AddEvent(WepEvt(WC_TRIG_RELOAD, 1330).SetBody(0));
	AddEvent(WepEvt(WC_TRIG_RELOAD, 1330).WepAnim(M249_RELOAD_END));

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_m249.txt");
	PrecacheEvents();
}

void CM249::PrecacheEvents()
{
	CWeaponCustom::PrecacheEvents();
}

int CM249::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_m249";
	p->pszAmmo1 = "556";
	p->iMaxClip = M249_MAX_CLIP;
	p->iMaxAmmo1 = 600;
	p->iSlot = 5;
	p->iPosition = 0;
	p->iId = WEAPON_M249;
	p->iWeight = M249_WEIGHT;
	return true;
}

void CM249::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	ammoEntName = "ammo_556";
	dropAmount = 100;
}
