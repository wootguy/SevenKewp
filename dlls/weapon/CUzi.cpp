#include "CUzi.h"

LINK_ENTITY_TO_CLASS(weapon_uzi, CUzi)
LINK_ENTITY_TO_CLASS(weapon_uziakimbo, CUzi)

void CUzi::Spawn()
{
	pev->classname = MAKE_STRING("weapon_uzi"); // hack to allow for old names
	Precache();
	SetWeaponModelW();
	m_iId = WEAPON_UZI;
	m_iDefaultAmmo = UZI_DEFAULT_GIVE;
	FallInit();
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

	animExt = "python";
	wrongClientWeapon = "weapon_9mmAR";

	params.flags = FL_WC_WEP_HAS_PRIMARY;
	params.vmodel = MODEL_INDEX(GetModelV());
	params.deployAnim = UZI_DEPLOY;
	params.deployTime = 1280;
	params.maxClip = UZI_MAX_CLIP;
	params.reloadStage[0] = { UZI_RELOAD, 2600 };
	params.idles[0] = { UZI_IDLE2, 10, 6730 };
	params.idles[1] = { UZI_IDLE3, 90, 5000 };

	CustomWeaponShootOpts& primary = params.shootOpts[0];
	primary.ammoCost = 1;
	primary.cooldown = 70;

	float spread = VECTOR_CONE_6DEGREES.x;
	int bulletf = FL_WC_BULLETS_DYNAMIC_SPREAD;
	int btype = BULLET_PLAYER_9MM;

	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).WepAnim(UZI_SHOOT));
	
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY)
		.PlaySound(shootSnd1, CHAN_WEAPON, 1.0f, ATTN_NORM, 96, 104, DISTANT_9MM, WC_AIVOL_NORMAL)
		.AddSound(shootSnd2)
		.AddSound(shootSnd3));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).PunchRandom(2, 0));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).Bullets(1, gSkillData.sk_plr_9mm_bullet, spread, spread, btype, 2, bulletf));

	AddEvent(WepEvt(WC_TRIG_RELOAD, 300).PlaySound(reloadSnd1, CHAN_ITEM, 1.0f, ATTN_IDLE, 100));
	AddEvent(WepEvt(WC_TRIG_RELOAD, 1200).PlaySound(reloadSnd2, CHAN_ITEM, 1.0f, ATTN_IDLE, 100));
	AddEvent(WepEvt(WC_TRIG_RELOAD, 2000).PlaySound(reloadSnd3, CHAN_ITEM, 1.0f, ATTN_IDLE, 100));

	AddEvent(WepEvt(WC_TRIG_DEPLOY, 100).PlaySound(deploySnd, CHAN_ITEM, 1.0f, ATTN_IDLE, 100));

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
