#include "CEagle.h"

LINK_ENTITY_TO_CLASS(weapon_eagle, CEagle)

void CEagle::Spawn()
{
	pev->classname = MAKE_STRING("weapon_eagle"); // hack to allow for old names
	m_iId = WEAPON_EAGLE;
	m_iDefaultAmmo = EAGLE_DEFAULT_GIVE;
	CWeaponCustom::Spawn();
}

void CEagle::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_desert_eagle.mdl";
	m_defaultModelP = "models/p_desert_eagle.mdl";
	m_defaultModelW = "models/w_desert_eagle.mdl";
	CBasePlayerWeapon::Precache();

	int shootSnd = PRECACHE_SOUND("weapons/desert_eagle_fire.wav");
	int reloadSnd = PRECACHE_SOUND("weapons/desert_eagle_reload.wav");
	int laserSnd1 = PRECACHE_SOUND("weapons/desert_eagle_sight.wav");
	int laserSnd2 = PRECACHE_SOUND("weapons/desert_eagle_sight2.wav");
	int dotSpr = PRECACHE_MODEL("sprites/laserdot.spr");
	int beamSpr = PRECACHE_MODEL("sprites/laserbeam.spr");

	animExt = "python";
	wrongClientWeapon = "weapon_357";

	params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY | FL_WC_WEP_UNLINK_COOLDOWNS
		| FL_WC_WEP_HAS_LASER | FL_WC_WEP_HAS_ALT_PRIMARY;
	params.vmodel = MODEL_INDEX(GetModelV());
	params.deployAnim = EAGLE_DRAW;
	params.maxClip = EAGLE_MAX_CLIP;
	params.reloadStage[0] = { EAGLE_RELOAD_NOSHOT, 1680 };
	params.reloadStage[1] = { EAGLE_RELOAD, 1680 };
	params.idles[0] = { EAGLE_IDLE3, 60, 1670 };
	params.idles[1] = { EAGLE_IDLE5, 20, 2030 };
	params.idles[2] = { EAGLE_IDLE1, 10, 2530 };
	params.idles[3] = { EAGLE_IDLE2, 10, 2540 };

	CustomWeaponShootOpts& primary = params.shootOpts[0];
	primary.ammoCost = 1;
	primary.cooldown = 220; // 500 = laser

	CustomWeaponShootOpts& primary_alt = params.shootOpts[3];
	primary_alt.ammoCost = 1;
	primary_alt.cooldown = 500;

	CustomWeaponShootOpts& secondary = params.shootOpts[1];
	secondary.ammoCost = 0;
	secondary.cooldown = 500;
	secondary.flags = FL_WC_SHOOT_NO_ATTACK;

	params.laser.dotSprite = dotSpr;
	params.laser.beamSprite = beamSpr;
	params.laser.dotSz = 5;
	params.laser.beamWidth = 1;
	params.laser.attachment = 2;
	params.laser.idles[0] = { EAGLE_IDLE3, 80, 1670 };
	params.laser.idles[1] = { EAGLE_IDLE5, 10, 2030 };
	params.laser.idles[2] = { EAGLE_IDLE4, 10, 2530 };

	float spread = VECTOR_CONE_4DEGREES.x;
	float spreadLaser = VECTOR_CONE_1DEGREES.x*0.5f;
	int bulletf = FL_WC_BULLETS_DYNAMIC_SPREAD;

	AddEvent(WepEvt().Primary().Bullets(1, 0, gSkillData.sk_plr_357_bullet, spread, spread, 0, WC_FLASH_NORMAL, bulletf));
	AddEvent(WepEvt().PrimaryAlt().Bullets(1, 0, gSkillData.sk_plr_357_bullet, spreadLaser, spreadLaser, 0, WC_FLASH_NORMAL, bulletf));

	AddEvent(WepEvt().BulletFired().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, 0.2f, 94, 109, DISTANT_357, WC_AIVOL_NORMAL));
	AddEvent(WepEvt().BulletFired().PunchSet(-4, 0));
	AddEvent(WepEvt().BulletFired().HideLaser(400));

	AddEvent(WepEvt().PrimaryNotEmpty().WepAnim(EAGLE_SHOOT));
	AddEvent(WepEvt().PrimaryEmpty().WepAnim(EAGLE_SHOOT_EMPTY));

	AddEvent(WepEvt().Secondary().ToggleLaser());

	AddEvent(WepEvt().LaserOn().IdleSound(laserSnd1));
	AddEvent(WepEvt().LaserOff().IdleSound(laserSnd2));

	AddEvent(WepEvt().Reload().Delay(16).IdleSound(reloadSnd));

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_eagle.txt");
	PrecacheEvents();
}

void CEagle::PrecacheEvents()
{
	CWeaponCustom::PrecacheEvents();
}

int CEagle::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_eagle";
	p->pszAmmo1 = "357";
	p->iMaxClip = EAGLE_MAX_CLIP;
	p->iMaxAmmo1 = gSkillData.sk_ammo_max_357;
	p->iSlot = 1;
	p->iPosition = 5;
	p->iId = WEAPON_EAGLE;
	p->iWeight = EAGLE_WEIGHT;
	return true;
}

void CEagle::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	ammoEntName = "ammo_357";
	dropAmount = 6;
}
