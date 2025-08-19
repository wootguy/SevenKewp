#include "CM16.h"

//LINK_ENTITY_TO_CLASS(weapon_m16, CM16)

void CM16::Spawn()
{
	pev->classname = MAKE_STRING("weapon_m16"); // hack to allow for old names
	m_iId = WEAPON_M16;
	m_iDefaultAmmo = M16_DEFAULT_GIVE;
	CWeaponCustom::Spawn();
}

void CM16::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_m16a2.mdl";
	m_defaultModelP = "models/p_m16.mdl";
	m_defaultModelW = "models/w_m16.mdl";
	CBasePlayerWeapon::Precache();

	int m_iShell = PRECACHE_MODEL("models/saw_shell.mdl");
	int drawSnd = PRECACHE_SOUND("weapons/m16_draw.wav");
	int shootSnd = PRECACHE_SOUND("weapons/m16_single.wav");
	int launchSnd1 = PRECACHE_SOUND("weapons/glauncher.wav");
	int launchSnd2 = PRECACHE_SOUND("weapons/glauncher2.wav");
	int reloadSnd1 = PRECACHE_SOUND("weapons/m16_magout_metallic.wav");
	int reloadSnd2 = PRECACHE_SOUND("weapons/m16_magin_metallic.wav");
	int reloadSnd3 = PRECACHE_SOUND("weapons/m16_charge.wav");

	animExt = "mp5";
	wrongClientWeapon = "weapon_9mmAR";

	params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY;
	params.vmodel = MODEL_INDEX(GetModelV());
	params.deployAnim = M16_DRAW;
	params.maxClip = M16_MAX_CLIP;
	params.reloadStage[0] = { M16_RELOAD_M16, 3370 };
	params.idles[0] = { M16_IDLE, 80, 3400 };
	params.idles[1] = { M16_FIDGET, 20, 2870 };

	CustomWeaponShootOpts& primary = params.shootOpts[0];
	primary.ammoCost = 3;
	primary.cooldown = 500;

	CustomWeaponShootOpts& secondary = params.shootOpts[1];
	secondary.ammoCost = 1;
	secondary.cooldown = 1000;

	float spread = VECTOR_CONE_3DEGREES.x;
	int bulletf = 0;
	int dmg = gSkillData.sk_plr_556_bullet;

	AddEvent(WepEvt().Primary().Bullets(3, 80, dmg, spread, spread, 2, WC_FLASH_NORMAL, bulletf));
	AddEvent(WepEvt().BulletFired().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_556, WC_AIVOL_LOUD));
	AddEvent(WepEvt().BulletFired().PunchRandom(1.5f, 0));
	AddEvent(WepEvt().BulletFired().EjectShell(m_iShell, 20, -12, 14));
	AddEvent(WepEvt().BulletFired().WepAnim(M16_SHOOT_1).AddAnim(M16_SHOOT_2));

	AddEvent(WepEvt().Secondary().WepAnim(M16_LAUNCH));
	AddEvent(WepEvt().Secondary().PunchSet(-10, 0));
	AddEvent(WepEvt().Secondary()
		.PlaySound(launchSnd1, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_556, WC_AIVOL_LOUD)
		.AddSound(launchSnd2));
	AddEvent(WepEvt().Secondary()
		.Projectile(WC_PROJECTILE_ARGRENADE, 800)
		.ProjPhysics(0.5f));

	AddEvent(WepEvt().Reload().Delay(300).IdleSound(reloadSnd1));
	AddEvent(WepEvt().Reload().Delay(1000).IdleSound(reloadSnd2));
	AddEvent(WepEvt().Reload().Delay(2200).IdleSound(reloadSnd3));

	AddEvent(WepEvt().Deploy().IdleSound(drawSnd));

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_m16.txt");
	PrecacheEvents();
}

void CM16::PrecacheEvents()
{
	CWeaponCustom::PrecacheEvents();
}

int CM16::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_m16";
	p->pszAmmo1 = "556";
	p->iMaxClip = M16_MAX_CLIP;
	p->iMaxAmmo1 = 600;
	p->pszAmmo2 = "ARgrenades";
	p->iMaxAmmo2 = gSkillData.sk_ammo_max_argrenades;
	p->iSlot = 2;
	p->iPosition = 4;
	p->iId = WEAPON_M16;
	p->iWeight = M16_WEIGHT;
	return true;
}

void CM16::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	ammoEntName = "ammo_556clip";
	dropAmount = 30;
}
