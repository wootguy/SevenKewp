#include "CMinigun.h"

LINK_ENTITY_TO_CLASS(weapon_minigun, CMinigun)

void CMinigun::Spawn()
{
	pev->classname = MAKE_STRING("weapon_minigun"); // hack to allow for old names
	m_iId = WEAPON_MINIGUN;
	m_iDefaultAmmo = MINIGUN_DEFAULT_GIVE;
	CWeaponCustom::Spawn();
}

void CMinigun::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_minigun.mdl";
	m_defaultModelP = "models/p_minigun.mdl";
	m_defaultModelW = "models/w_minigun.mdl";
	CBasePlayerWeapon::Precache();

	int m_iShell = PRECACHE_MODEL("models/saw_shell.mdl");
	int m_iLink = PRECACHE_MODEL("models/saw_link.mdl");
	int shootSnd = PRECACHE_SOUND("hassault/hw_shoot2.wav");
	int spinupSnd = PRECACHE_SOUND("hassault/hw_spinup.wav");
	int spindownSnd = PRECACHE_SOUND("hassault/hw_spindown.wav");
	int spinSnd = PRECACHE_SOUND("hassault/hw_spin2.wav");

	animExt = "saw";
	wrongClientWeapon = "weapon_9mmAR";

	params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY | FL_WC_WEP_LINK_CHARGEUPS
		| FL_WC_WEP_PRIMARY_PRIORITY | FL_WC_WEP_EXCLUSIVE_HOLD | FL_WC_WEP_USE_ONLY;
	params.vmodel = MODEL_INDEX(GetModelV());
	params.deployAnim = MINIGUN_DRAW;
	params.idles[0] = { MINIGUN_GENTLEIDLE, 85, 6200 };
	params.idles[1] = { MINIGUN_IDLE, 10, 6200 };
	params.idles[2] = { MINIGUN_IDLE2, 5, 6200 };
	params.moveSpeedMult = FLOAT_TO_MOVESPEED_MULT(0.6f);
	params.jumpPower = 200;

	CustomWeaponShootOpts& primary = params.shootOpts[0];
	primary.ammoCost = 1;
	primary.cooldown = 50;
	primary.cooldownFail = 200;
	primary.chargeTime = 800;
	primary.chargeCancelTime = 800;

	CustomWeaponShootOpts& secondary = params.shootOpts[1];
	secondary.cooldown = 200;
	secondary.chargeTime = 800;
	secondary.chargeCancelTime = 800;
	secondary.flags = FL_WC_SHOOT_NO_ATTACK;

	float spread = VECTOR_CONE_4DEGREES.x;
	int bulletf = 0;
	int dmg = gSkillData.sk_plr_556_bullet;

	AddEvent(WepEvt().Primary().WepAnim(MINIGUN_SPINFIRE));
	AddEvent(WepEvt().Primary().Delay(00).Bullets(1, 1, dmg, spread, spread, 2, WC_FLASH_NORMAL, bulletf));
	AddEvent(WepEvt().Primary().Delay(25).Bullets(1, 1, dmg, spread, spread, 2, WC_FLASH_NORMAL, bulletf));
	
	AddEvent(WepEvt().BulletFired().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_556, WC_AIVOL_LOUD));
	AddEvent(WepEvt().BulletFired().RotateView(1.3f, 1.3f).StandOnly());
	AddEvent(WepEvt().BulletFired().RotateView(1.0f, 1.0f).DuckOnly());
	AddEvent(WepEvt().PrimaryEven().EjectShell(m_iLink, 14, -12, 4));
	AddEvent(WepEvt().PrimaryOdd().EjectShell(m_iShell, 14, -12, 4));

	AddEvent(WepEvt().PrimaryCharge().WepAnim(MINIGUN_SPINUP));
	AddEvent(WepEvt().PrimaryCharge().Delay(200).WepAnim(MINIGUN_P_SPIN, 0, FL_WC_ANIM_PMODEL));
	AddEvent(WepEvt().PrimaryCharge().IdleSound(spinupSnd));
	AddEvent(WepEvt().PrimaryStop().WepAnim(MINIGUN_SPINDOWN));
	AddEvent(WepEvt().PrimaryStop().Delay(1000).WepAnim(MINIGUN_P_IDLE, 0, FL_WC_ANIM_PMODEL));
	AddEvent(WepEvt().PrimaryStop().IdleSound(spindownSnd));
	AddEvent(WepEvt().PrimaryStop().Cooldown(2030, FL_WC_COOLDOWN_IDLE));
	AddEvent(WepEvt().PrimaryStop().Cooldown(1500, FL_WC_COOLDOWN_PRIMARY | FL_WC_COOLDOWN_SECONDARY));

	AddEvent(WepEvt().PrimaryFail().WepAnim(MINIGUN_SPINIDLE, 0, FL_WC_ANIM_NO_RESET));
	AddEvent(WepEvt().PrimaryFail().IdleSound(spinSnd, 0.5f));

	AddEvent(WepEvt().Secondary().WepAnim(MINIGUN_SPINIDLE, 0, FL_WC_ANIM_NO_RESET));
	AddEvent(WepEvt().Secondary().IdleSound(spinSnd, 0.5f));

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_minigun.txt");
	PrecacheEvents();
}

void CMinigun::PrecacheEvents()
{
	CWeaponCustom::PrecacheEvents();
}

int CMinigun::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_minigun";
	p->pszAmmo1 = "556";
	p->iMaxClip = WEAPON_NOCLIP;
	p->iMaxAmmo1 = 600;
	p->iSlot = 5;
	p->iPosition = 2;
	p->iId = WEAPON_MINIGUN;
	p->iFlags = ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_NOAUTORELOAD;
	p->iWeight = MINIGUN_WEIGHT;
	return true;
}

void CMinigun::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	ammoEntName = "ammo_556";
	dropAmount = 100;
}
