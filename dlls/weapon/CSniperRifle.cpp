#include "CSniperRifle.h"

LINK_ENTITY_TO_CLASS(weapon_sniperrifle, CSniperRifle)

void CSniperRifle::Spawn()
{
	pev->classname = MAKE_STRING("weapon_sniperrifle"); // hack to allow for old names
	Precache();
	SetWeaponModelW();
	m_iId = WEAPON_M40A1;
	m_iDefaultAmmo = M40A1_DEFAULT_GIVE;
	FallInit();
}

void CSniperRifle::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_m40a1.mdl";
	m_defaultModelP = "models/p_m40a1.mdl";
	m_defaultModelW = "models/w_m40a1.mdl";
	CBasePlayerWeapon::Precache();

	int shootSnd = PRECACHE_SOUND("weapons/sniper_fire.wav");
	int zoomSnd = PRECACHE_SOUND("weapons/sniper_zoom.wav");
	int reloadSnd1 = PRECACHE_SOUND("weapons/sniper_reload_first_seq.wav");
	int reloadSnd2 = PRECACHE_SOUND("weapons/sniper_reload_second_seq.wav");
	int boltSnd1 = PRECACHE_SOUND("weapons/sniper_bolt1.wav");
	int boltSnd2 = PRECACHE_SOUND("weapons/sniper_bolt2.wav");

	animExt = "sniper";
	animExtZoom = "sniperscope";
	wrongClientWeapon = "weapon_crossbow";

	params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY | FL_WC_WEP_UNLINK_COOLDOWNS;
	params.vmodel = MODEL_INDEX(GetModelV());
	params.deployAnim = M40A1_DRAW;
	params.maxClip = M40A1_MAX_CLIP;
	params.reloadStage[0] = { M40A1_RELOAD3, 2353 };
	params.reloadStage[1] = { M40A1_RELOAD1, 4102 };
	params.idles[0] = { M40A1_SLOWIDLE, 100, 4348 };
	//params.idles[1] = { M40A1_SLOWIDLE2, 20, 4348 }; // bolt is in the wrong position

	CustomWeaponShootOpts& primary = params.shootOpts[0];
	primary.ammoCost = 1;
	primary.cooldown = 2000;

	CustomWeaponShootOpts& secondary = params.shootOpts[1];
	secondary.ammoCost = 0;
	secondary.cooldown = 500;
	secondary.flags = FL_WC_SHOOT_NO_ATTACK;

	float spread = VECTOR_CONE_1DEGREES.x * 0.5f;
	int bulletf = FL_WC_BULLETS_DYNAMIC_SPREAD;
	int btype = BULLET_PLAYER_9MM;

	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_NOT_EMPTY).WepAnim(M40A1_FIRE));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 0, 0).WepAnim(M40A1_FIRE_LAST));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_NOT_EMPTY, 620)
		.PlaySound(boltSnd1, CHAN_ITEM, 1.0f, ATTN_IDLE, 100));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, 620, 0)
		.PlaySound(boltSnd2, CHAN_ITEM, 1.0f, ATTN_IDLE, 100));
	
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY)
		.PlaySound(shootSnd, CHAN_WEAPON, 1.0f, 0.2f, 94, 109, DISTANT_NONE, WC_AIVOL_NORMAL));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).Bullets(1, gSkillData.sk_plr_762_bullet, spread, spread, btype, 1, bulletf));
	AddEvent(WepEvt(WC_TRIG_SHOOT_PRIMARY).PunchSet(-2, 0));

	AddEvent(WepEvt(WC_TRIG_SHOOT_SECONDARY).PlaySound(zoomSnd, CHAN_ITEM, 1.0f, ATTN_IDLE, 100));
	AddEvent(WepEvt(WC_TRIG_SHOOT_SECONDARY).ToggleZoom(18));

	AddEvent(WepEvt(WC_TRIG_RELOAD, 16).PlaySound(reloadSnd1, CHAN_ITEM, 1.0f, ATTN_IDLE, 100));
	AddEvent(WepEvt(WC_TRIG_RELOAD_EMPTY, 2324).PlaySound(reloadSnd2, CHAN_ITEM, 1.0f, ATTN_IDLE, 100));
	AddEvent(WepEvt(WC_TRIG_RELOAD_EMPTY, 2324).WepAnim(M40A1_RELOAD2));

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_sniperrifle.txt");
	PrecacheEvents();
}

void CSniperRifle::PrecacheEvents()
{
	CWeaponCustom::PrecacheEvents();
}

int CSniperRifle::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_sniperrifle";
	p->pszAmmo1 = "762";
	p->iMaxClip = M40A1_MAX_CLIP;
	p->iMaxAmmo1 = 20;
	p->iSlot = 5;
	p->iPosition = 1;
	p->iId = WEAPON_M40A1;
	p->iWeight = M40A1_WEIGHT;
	return true;
}

void CSniperRifle::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	ammoEntName = "ammo_762";
	dropAmount = 5;
}
