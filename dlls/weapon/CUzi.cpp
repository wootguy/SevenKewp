#include "CUzi.h"

LINK_ENTITY_TO_CLASS(weapon_uzi, CUzi)
LINK_ENTITY_TO_CLASS(weapon_uziakimbo, CUzi)

void CUzi::Spawn()
{
	SetCanAkimbo(!strcmp("weapon_uziakimbo", STRING(pev->classname)));
	if (CanAkimbo())
		SetAkimbo(true);

	pev->classname = MAKE_STRING("weapon_uzi"); // hack to allow for old names
	m_iId = WEAPON_UZI;
	m_iDefaultAmmo = UZI_DEFAULT_GIVE;
	CWeaponCustom::Spawn();
}

void CUzi::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_uzi.mdl";
	m_defaultModelP = "models/p_uzi.mdl";
	m_defaultModelW = "models/w_uzi.mdl";
	CBasePlayerWeapon::Precache();

	int shootSnd1 = PRECACHE_SOUND("weapons/uzi/shoot1.wav");
	int shootSnd2 = PRECACHE_SOUND("weapons/uzi/shoot2.wav");
	int shootSnd3 = PRECACHE_SOUND("weapons/uzi/shoot3.wav");
	int reloadSnd1 = PRECACHE_SOUND("weapons/uzi/reload1.wav");
	int reloadSnd2 = PRECACHE_SOUND("weapons/uzi/reload2.wav");
	int reloadSnd3 = PRECACHE_SOUND("weapons/uzi/reload3.wav");
	int deploySnd = PRECACHE_SOUND("weapons/uzi/deploy.wav");
	int deploySnd2 = PRECACHE_SOUND("weapons/uzi/deploy1.wav");
	int akimboDeploySnd = PRECACHE_SOUND("weapons/uzi/akimbo_deploy.wav");
	int akimboPullSnd1 = PRECACHE_SOUND("weapons/uzi/akimbo_pull1.wav");
	int akimboPullSnd2 = PRECACHE_SOUND("weapons/uzi/akimbo_pull2.wav");

	int iShell = PRECACHE_MODEL("models/shell.mdl");

	animExt = "onehanded";
	animExtAkimbo = "uzis";
	pmodelAkimbo = "models/p_2uzis.mdl";
	wmodelAkimbo = "models/w_2uzis.mdl";
	wrongClientWeapon = "weapon_9mmAR";

	params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_TERTIARY | FL_WC_WEP_AKIMBO;
	params.vmodel = MODEL_INDEX(GetModelV());
	params.deployAnim = UZI_DEPLOY;
	params.deployTime = 1280;
	params.maxClip = UZI_MAX_CLIP;
	params.reloadStage[0] = { UZI_RELOAD, 2600 };
	params.idles[0] = { UZI_IDLE1, 90, 5000 };
	params.idles[1] = { UZI_IDLE2, 10, 6730 };

	CustomWeaponShootOpts& primary = params.shootOpts[0];
	primary.ammoCost = 1;
	primary.cooldown = 70;

	CustomWeaponShootOpts& tertiary = params.shootOpts[2];
	tertiary.ammoCost = 0;
	tertiary.cooldown = 1870;
	tertiary.flags = FL_WC_SHOOT_NO_ATTACK | FL_WC_SHOOT_NEED_AKIMBO;

	params.akimbo.idles[0] = { UZI_AKIMBO_IDLE, 100, 2000 };
	params.akimbo.deployAnim = UZI_AKIMBO_DEPLOY2;
	params.akimbo.deployTime = 420;
	params.akimbo.holsterAnim = UZI_AKIMBO_HOLSTER;
	params.akimbo.holsterTime = 170;
	params.akimbo.reload.anim = UZI_AKIMBO_RELOAD_RIGHT;
	params.akimbo.reload.time = 2360;
	params.akimbo.akimboDeployAnim = UZI_AKIMBO_DEPLOY;
	params.akimbo.akimboDeployTime = 500;

	float spread = VECTOR_CONE_6DEGREES.x;
	int bulletf = 0;

	AddEvent(WepEvt().Primary().NotAkimbo().WepAnim(UZI_SHOOT));
	AddEvent(WepEvt().Primary().AkimboOnly().WepAnim(UZI_AKIMBO_FIRE_RIGHT, WC_ANIM_TRIG_HAND));
	
	AddEvent(WepEvt().Primary()
		.PlaySound(shootSnd1, CHAN_STATIC, 0.7f, ATTN_NORM, 96, 104, DISTANT_9MM, WC_AIVOL_NORMAL)
		.AddSound(shootSnd2)
		.AddSound(shootSnd3));

	AddEvent(WepEvt().Primary().Bullets(1, 1, gSkillData.sk_plr_9mm_bullet, spread, spread, 2, WC_FLASH_NORMAL, bulletf));
	AddEvent(WepEvt().Primary().PunchRandom(2, 0));
	AddEvent(WepEvt().Primary().EjectShell(iShell, 28, -12, 16));

	AddEvent(WepEvt().Reload().Delay(300).IdleSound(reloadSnd1));
	AddEvent(WepEvt().Reload().NotAkimbo().Delay(800).IdleSound(akimboPullSnd2));
	AddEvent(WepEvt().Reload().AkimboOnly().Delay(1000).IdleSound(reloadSnd2));
	AddEvent(WepEvt().Reload().NotAkimbo().Delay(1200).IdleSound(reloadSnd2));
	AddEvent(WepEvt().Reload().NotAkimbo().Delay(1900).IdleSound(reloadSnd3));

	AddEvent(WepEvt().Deploy().NotAkimbo().Delay(100).IdleSound(deploySnd));

	AddEvent(WepEvt().Deploy().AkimboOnly().IdleSound(akimboDeploySnd));
	AddEvent(WepEvt().Deploy().AkimboOnly().Cooldown(1170, FL_WC_COOLDOWN_IDLE));

	AddEvent(WepEvt().Tertiary().Cooldown(2500, FL_WC_COOLDOWN_IDLE));

	AddEvent(WepEvt().Tertiary().NotAkimbo().WepAnim(UZI_AKIMBO_PULL));
	AddEvent(WepEvt().Tertiary().NotAkimbo().Delay(100).IdleSound(akimboPullSnd2));
	AddEvent(WepEvt().Tertiary().NotAkimbo().Delay(400).IdleSound(akimboPullSnd1));
	AddEvent(WepEvt().Tertiary().NotAkimbo().Delay(1000).IdleSound(deploySnd2));
	AddEvent(WepEvt().Tertiary().NotAkimbo().Delay(1200).ToggleAkimbo());

	AddEvent(WepEvt().Tertiary().AkimboOnly().WepAnim(UZI_AKIMBO_HOLSTER, WC_ANIM_BOTH_HANDS));
	AddEvent(WepEvt().Tertiary().AkimboOnly().IdleSound(deploySnd2));
	AddEvent(WepEvt().Tertiary().AkimboOnly().Delay(500).ToggleAkimbo());

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_uzi.txt");
	PrecacheEvents();

	UTIL_PrecacheOther("ammo_uziclip");
}

void CUzi::PrecacheEvents()
{
	CWeaponCustom::PrecacheEvents();
}

int CUzi::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_uzi";
	p->pszAmmo1 = "9mm";
	p->iMaxClip = UZI_MAX_CLIP;
	p->iMaxAmmo1 = 200;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = WEAPON_UZI;
	p->iWeight = UZI_WEIGHT;
	return true;
}

void CUzi::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	ammoEntName = "ammo_uziclip";
	dropAmount = 32;
}
