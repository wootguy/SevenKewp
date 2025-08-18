#include "CWeaponCustom.h"

#ifdef CLIENT_DLL
#include "../cl_dll/hud_iface.h"
#include "eng_wrappers.h"
extern int g_runfuncs;
extern int g_runningKickbackPred;
extern Vector g_vApplyVel;
void UpdateZoomCrosshair(int id, bool zoom);
void WC_EV_LocalSound(WepEvt& evt, int sndIdx, int chan, int pitch, float vol, float attn, int panning);
void WC_EV_EjectShell(WepEvt& evt, bool leftHand);
void WC_EV_PunchAngle(WepEvt& evt, int seed);
void WC_EV_WepAnim(WepEvt& evt, int wepid, int animIdx);
void WC_EV_Bullets(WepEvt& evt, float spreadX, float spreadY, bool showTracer, bool decal, bool texSound);
void EV_LaserOn(const char* dotSprite, float dotSz, const char* beamSprite, float beamWidth);
void EV_LaserOff();
#else
int g_runfuncs = 1;
#define PRINTF(fmt, ...)
#include "game.h"
#endif

char CWeaponCustom::m_soundPaths[MAX_PRECACHE][256];
int CWeaponCustom::m_tracerCount[32];
bool CWeaponCustom::m_customWeaponSounds[MAX_PRECACHE_SOUND];

void CWeaponCustom::Spawn() {
	Precache();
	SetWeaponModelW();
	FallInit();// get ready to fall down.

	if (params.flags & FL_WC_WEP_USE_ONLY) {
		SetTouch(&CBaseEntity::ItemBounceTouch);
	}
}

void CWeaponCustom::Precache() {
	PrecacheEvents();
}

void CWeaponCustom::PrecacheEvents() {
	for (int i = 0; i < params.numEvents; i++) {
		WepEvt& evt = params.events[i];
		if (evt.evtType == WC_EVT_PLAY_SOUND) {
			if (evt.playSound.sound)
				m_customWeaponSounds[evt.playSound.sound] = true;
			for (int k = 0; k < evt.playSound.numAdditionalSounds; k++) {
				if (evt.playSound.additionalSounds[k] && k < MAX_WC_RANDOM_SELECTION)
					m_customWeaponSounds[evt.playSound.additionalSounds[k]] = true;
			}
		}
		if (evt.evtType == WC_EVT_IDLE_SOUND) {
			if (evt.idleSound.sound)
				m_customWeaponSounds[evt.idleSound.sound] = true;
		}
	}

	if (pmodelAkimbo)
		PRECACHE_MODEL(pmodelAkimbo);
	if (wmodelAkimbo)
		PRECACHE_MODEL(wmodelAkimbo);

	if (wrongClientWeapon)
		UTIL_PrecacheOther(wrongClientWeapon);
}

void CWeaponCustom::AddEvent(WepEvt evt) {
	if (params.numEvents >= MAX_WC_EVENTS) {
		ALERT(at_error, "Exceeded max custom weapon events for %s\n", STRING(pev->classname));
		return;
	}

	params.events[params.numEvents++] = evt;
}

int CWeaponCustom::AddToPlayer(CBasePlayer* pPlayer) {
#ifndef CLIENT_DLL
	if (!pPlayer->IsSevenKewpClient()) {
		if (pPlayer->HasNamedPlayerItem(wrongClientWeapon)) {
			return 0;
		}

		pPlayer->GiveNamedItem(wrongClientWeapon);
		CBasePlayerItem* wep = pPlayer->GetNamedPlayerItem(wrongClientWeapon);

		if (!wep || pPlayer->m_sentSevenKewpNotice || !mp_sevenkewp_client_notice.value)
			return 0;

		std::string clientReq = UTIL_SevenKewpClientString(MIN_SEVENKEWP_VERSION);
		UTIL_ClientPrint(pPlayer, print_chat, UTIL_VarArgs(
			"The \"%s\" requires the \"%s\" client. You were given the \"%s\" instead. Check your console for more info.\n",
			DisplayName(), clientReq.c_str(), wep->DisplayName()));

		pPlayer->SendSevenKewpClientNotice();

		return 0;
	}

	if (params.flags & FL_WC_WEP_AKIMBO)
		SetAkimboClip(m_iDefaultAmmo);

	SendPredictionData(pPlayer->edict());
#endif
	
	if (CBasePlayerWeapon::AddToPlayer(pPlayer)) {
		if (params.flags & FL_WC_WEP_EXCLUSIVE_HOLD) {
			pPlayer->SwitchWeapon(this);
		}
		return 1;
	}

	return 0;
}

const char* CWeaponCustom::GetAnimSet() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return NULL;

#ifndef CLIENT_DLL
	const char* validAnimExt = animExt;

	if (IsAkimbo())
		validAnimExt = animExtAkimbo;
	else if (m_pPlayer->m_iFOV != 0)
		validAnimExt = animExtZoom;

	if (m_pPlayer->m_playerModelAnimSet != PMODEL_ANIMS_HALF_LIFE_COOP) {
		// half-life models are missing animations for some weapons, so fallback to valid HL anims
		if (!strcmp(validAnimExt, "saw")) {
			validAnimExt = "mp5";
		}
		else if (!strcmp(validAnimExt, "sniper")) {
			validAnimExt = "shotgun";
		}
		else if (!strcmp(validAnimExt, "sniperscope")) {
			validAnimExt = "onehanded";
		}
		else if (!strcmp(validAnimExt, "uzis")) {
			validAnimExt = "onehanded";
		}
	}

	return validAnimExt;
#else
	return NULL;
#endif
}

void CWeaponCustom::UpdateAnimSet() {
#ifndef CLIENT_DLL
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	strcpy_safe(m_pPlayer->m_szAnimExtention, GetAnimSet(), sizeof(m_pPlayer->m_szAnimExtention));
#endif
}

BOOL CWeaponCustom::Deploy()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return FALSE;

	m_flChargeStartPrimary = 0;
	m_flChargeStartSecondary = 0;
	m_bulletFireCount = 0;
	m_lastBeamUpdate = 0;
	m_fInReload = false;
	m_bInAkimboReload = false;
	int ret = TRUE;

	m_pPlayer->SetThirdPersonWeaponAnim(0);

	int deployAnim = IsAkimbo() ? params.akimbo.akimboDeployAnim : params.deployAnim;

#ifdef CLIENT_DLL
	if (!CanDeploy())
		return FALSE;

	m_pPlayer->pev->viewmodel = params.vmodel;

	SendWeaponAnim(deployAnim, 1, pev->body);

	g_irunninggausspred = false;
	m_pPlayer->m_flNextAttack = 0.5;
	m_flTimeWeaponIdle = 1.0;

	if (IsLaserOn()) {
		// re-create the effect
		HideLaser(false);
	}
#else
	const char* animSet = GetAnimSet();

	studiohdr_t* mdl = GET_MODEL_PTR(PRECACHE_MODEL(GetModelV()));
	m_hasLaserAttachment = mdl && mdl->numattachments > params.laser.attachment;

	ret = DefaultDeploy(STRING(g_indexModels[params.vmodel]), GetModelP(), deployAnim, animSet, 1);
#endif

	if (IsAkimbo())
		SendAkimboAnim(deployAnim);

	int deployTime = IsAkimbo() ? params.akimbo.akimboDeployTime : params.deployTime;
	if (!deployTime)
		deployTime = 500; // default

	float nextAttack = UTIL_WeaponTimeBase() + deployTime * 0.001f;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = nextAttack;
	m_flTimeWeaponIdle = nextAttack + 0.5f;

	m_pPlayer->m_flNextAttack = 0; // allow thinking during deployment

	if (gpGlobals->time - m_lastDeploy > MAX_PREDICTION_WAIT) {
		int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
		ProcessEvents(WC_TRIG_DEPLOY, akimboArg);
	}

	if (params.jumpPower)
		m_pPlayer->SetJumpPower(params.jumpPower);

	return ret;
}

void CWeaponCustom::Holster(int skiplocal) {
	CBasePlayerWeapon::Holster();
	CancelZoom();
	UTIL_Remove(m_hLaserSpot);
#ifdef CLIENT_DLL
	EV_LaserOff();
#endif
}

void CWeaponCustom::Reload() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_fInReload)
		return;
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	bool canAkimboReload = IsAkimbo() && GetAkimboClip() < params.maxClip;

	if (m_flChargeStartPrimary || m_flChargeStartSecondary)
		return;
	if (m_iClip == -1)
		return;
	if (m_iClip >= params.maxClip && !canAkimboReload) {
		m_bWantAkimboReload = false;
		return;
	}
	if (m_flNextPrimaryAttack > 0)
		return;

	WeaponCustomReload* reloadStage = &params.reloadStage[0];

	if (IsAkimbo()) {
		reloadStage = &params.akimbo.reload;
	}
	else if (m_iClip == 0 && params.reloadStage[1].time) {
		// in normal reload mode, the second reload stage is for empty reloads
		reloadStage = &params.reloadStage[1];
	}

	CancelZoom();

	//CLALERT("Start Reload %d %d\n", m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	m_fInReload = TRUE;

	int totalReloadTime = reloadStage->time;
	float nextAttack = totalReloadTime * 0.001f;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = m_flTimeWeaponIdle = nextAttack;


	if (IsAkimbo()) {
		m_bInAkimboReload = IsAkimbo() && GetAkimboClip() < m_iClip;

		if (m_bInAkimboReload) {
			SendAkimboAnim(reloadStage->anim);
			SendWeaponAnim(params.akimbo.holsterAnim, 1, pev->body);
			SendWeaponAnimSpec(params.akimbo.holsterAnim);
			m_pPlayer->SetAnimation(PLAYER_RELOAD2, totalReloadTime * 0.001f);
		}
		else {
			SendAkimboAnim(params.akimbo.holsterAnim);
			SendWeaponAnim(reloadStage->anim, 1, pev->body);
			SendWeaponAnimSpec(reloadStage->anim);
			m_pPlayer->SetAnimation(PLAYER_RELOAD3, totalReloadTime * 0.001f);
		}
	}
	else {
		SendWeaponAnim(reloadStage->anim, 1, pev->body);
		SendWeaponAnimSpec(reloadStage->anim);
		m_pPlayer->SetAnimation(PLAYER_RELOAD, totalReloadTime * 0.001f);
	}

	if (g_runfuncs) {
		int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
		ProcessEvents(WC_TRIG_RELOAD, akimboArg);

		if (m_iClip == 0) {
			ProcessEvents(WC_TRIG_RELOAD_EMPTY, akimboArg);
		}
		else {
			ProcessEvents(WC_TRIG_RELOAD_NOT_EMPTY, akimboArg);
		}

		if (IsLaserOn()) {
			HideLaser(true);
			m_laserOnTime = WallTime() + totalReloadTime * 0.001f;
		}
	}

	m_pPlayer->m_flNextAttack = 0; // keep calling post frame for complex reloads
}

void CWeaponCustom::WeaponIdle() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_lastCanAkimbo != CanAkimbo()) {
		if (!g_runfuncs) {
			return;
		}
		SetAkimbo(CanAkimbo());
		Deploy();
		m_lastCanAkimbo = CanAkimbo();
		return;
	}

	FinishAttack(0);
	FinishAttack(1);

	if (m_waitForNextRunfuncs) {
		if (!g_runfuncs) {
			return;
		}
		m_waitForNextRunfuncs = false;
		return;
	}

	m_primaryCalled = false;
	m_secondaryCalled = false;

	m_lastCanAkimbo = CanAkimbo();

	if (m_fInReload)
		return;

	ResetEmptySound();

	// Update auto-aim
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_bWantAkimboReload && g_runfuncs) {
		Reload();
		return;
	}

	WeaponCustomIdle* idles = params.idles;
	if (IsAkimbo())
		idles = params.akimbo.idles;
	else if (IsLaserOn())
		idles = params.laser.idles;

	int idleSum = 0;
	for (int i = 0; i < 4; i++) {
		idleSum += idles[i].weight;
	}

	int idleRnd = (UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0) + 0.005f) * idleSum;

	for (int i = 0; i < 4; i++) {
		WeaponCustomIdle& idle = idles[i];
		idleRnd -= idle.weight;

		if (idleRnd <= 0) {
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + idle.time * 0.001f;
			SendWeaponAnim(idle.anim, 1, pev->body);
			SendWeaponAnimSpec(idle.anim);
			break;
		}
	}
}

void CWeaponCustom::ItemPostFrame() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	bool reloadFinished = m_fInReload && m_flNextPrimaryAttack <= 0;

	// reload prediction
	if (reloadFinished) {
		// complete the reload.
		int& clip = m_bInAkimboReload ? m_chargeReady : m_iClip;
		int j = V_min(params.maxClip - clip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
		clip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
		m_pPlayer->TabulateAmmo();
		
		m_fInReload = FALSE;


		if (IsAkimbo()) {
			if (m_bInAkimboReload) {
				SendWeaponAnim(params.akimbo.deployAnim, 1, pev->body);
				SendWeaponAnimSpec(params.akimbo.deployAnim);
			}
			else {
				SendAkimboAnim(params.akimbo.deployAnim);
			}

			bool attackInterrupt = m_pPlayer->pev->button & (IN_ATTACK | IN_ATTACK2);
			bool canReloadOtherGun = m_iClip < params.maxClip || GetAkimboClip() < params.maxClip;

			// wait for the holstered gun to deploy before reloading it or idling
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.0f + params.akimbo.deployTime * 0.001f;
			
			if (!attackInterrupt && canReloadOtherGun) {
				m_bWantAkimboReload = true;
			}
		}
	}

	m_pPlayer->m_flNextAttack = 1; // prevent reload logic triggering
	CBasePlayerWeapon::ItemPostFrame();
	m_pPlayer->m_flNextAttack = 0;

	if (!g_runfuncs) {
		return;
	}

	PlayDelayedEvents();
	UpdateLaser();
}

const char* CWeaponCustom::GetModelP() {
	return pmodelAkimbo && IsAkimbo() ? pmodelAkimbo : CBasePlayerWeapon::GetModelP();
}

const char* CWeaponCustom::GetModelW() {
	return wmodelAkimbo && CanAkimbo() ? wmodelAkimbo : CBasePlayerWeapon::GetModelW();
}

void CWeaponCustom::PrimaryAttack() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_waitForNextRunfuncs && !g_runfuncs) {
		return;
	}

	m_waitForNextRunfuncs = false;
	m_primaryCalled = true;

	if ((params.shootOpts[0].flags & FL_WC_SHOOT_NEED_AKIMBO) && !CanAkimbo())
		return;

	if (params.flags & FL_WC_WEP_HAS_PRIMARY) {
		int* clip = IsAkimbo() ? &m_chargeReady : &m_iClip;

		if (m_iClip == -1 && m_iPrimaryAmmoType != -1)
			clip = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

		int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;

		if (CommonAttack(0, clip, IsAkimbo(), false)) {
			int trig = IsPrimaryAltActive() ? WC_TRIG_PRIMARY_ALT : WC_TRIG_PRIMARY;
			ProcessEvents(trig, akimboArg, IsAkimbo(), false, *clip);

			if ((m_bulletFireCount++ % 2) == 0) {
				ProcessEvents(WC_TRIG_PRIMARY_EVEN, akimboArg, IsAkimbo(), false, *clip);
			}
			else {
				ProcessEvents(WC_TRIG_PRIMARY_ODD, akimboArg, IsAkimbo(), false, *clip);
			}

			ProcessEvents(WC_TRIG_PRIMARY_CLIPSIZE, *clip, IsAkimbo(), false, *clip);

			if (*clip != 0) {
				ProcessEvents(WC_TRIG_PRIMARY_NOT_EMPTY, akimboArg, IsAkimbo(), false, *clip);
			}

			if (*clip < 0)
				*clip = 0;
		}
	}

	KickbackPrediction();
}

void CWeaponCustom::SecondaryAttack() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if ((params.flags & FL_WC_WEP_PRIMARY_PRIORITY) && (m_pPlayer->pev->button & IN_ATTACK)) {
		PrimaryAttack();
		return;
	}

	if (m_waitForNextRunfuncs && !g_runfuncs)
		return;

	m_waitForNextRunfuncs = false;
	m_secondaryCalled = true;

	if ((params.shootOpts[1].flags & FL_WC_SHOOT_NEED_AKIMBO) && !CanAkimbo())
		return;

	if (IsAkimbo()) {
		bool fireBoth = (m_pPlayer->pev->button & IN_ATTACK) && GetAkimboClip() >= params.shootOpts[0].ammoCost;
		int* clip = &m_iClip;
		int primaryTrig = IsPrimaryAltActive() ? WC_TRIG_PRIMARY_ALT : WC_TRIG_PRIMARY;

		if (m_iClip == -1 && m_iPrimaryAmmoType != -1)
			clip = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

		if (CommonAttack(0, &m_iClip, false, fireBoth)) {
			ProcessEvents(primaryTrig, WC_TRIG_SHOOT_ARG_AKIMBO, false, fireBoth, *clip);

			if (*clip < 0)
				*clip = 0;
		}
		
		m_pPlayer->random_seed++; // so bullets don't hit the same spot (seed only updates once per cmd)
		if (fireBoth && CommonAttack(0, &m_chargeReady, true, fireBoth)) {
			ProcessEvents(primaryTrig, WC_TRIG_SHOOT_ARG_AKIMBO, true, fireBoth, *clip);

			if (m_chargeReady < 0)
				m_chargeReady = 0;
		}
	}
	else if (params.flags & FL_WC_WEP_HAS_SECONDARY) {
		static int nullclip;
		int* clip2 = m_iSecondaryAmmoType >= 0 ? &m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] : &nullclip;
		int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;

		if (CommonAttack(1, clip2, false, false)) {
			ProcessEvents(WC_TRIG_SECONDARY, akimboArg, *clip2);
		}
	}

	KickbackPrediction();
}

void CWeaponCustom::TertiaryAttack() {
	if ((params.shootOpts[2].flags & FL_WC_SHOOT_NEED_AKIMBO) && !CanAkimbo())
		return;

	if (params.flags & FL_WC_WEP_HAS_TERTIARY) {
		int tclip = 0;
		if (CommonAttack(2, &tclip, false, false)) {
			int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;

			ProcessEvents(WC_TRIG_TERTIARY, akimboArg, false, false, tclip);

			if (tclip < 0)
				tclip = 0;
		}
	}
}

bool CWeaponCustom::CommonAttack(int attackIdx, int* clip, bool leftHand, bool akimboFire) {
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	bool isNormalAttack = !(opts.flags & FL_WC_SHOOT_NO_ATTACK);

	int clipLeft = *clip;

	m_bWantAkimboReload = false;

	if (attackIdx == 1) {
		clipLeft = m_iSecondaryAmmoType >= 0 ? m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] : 0;
	}

	if (!Chargeup(attackIdx, leftHand, akimboFire))
		return false;

	if (isNormalAttack) {
		if (clipLeft <= 0) {
			if (!m_fInReload) {
				Cooldown(-1, 150);
				PlayEmptySound();
				FailAttack(attackIdx, leftHand, akimboFire);
			}
			return false;
		}

		if (m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD && !(opts.flags & FL_WC_SHOOT_UNDERWATER)) {
			Cooldown(-1, 150);
			PlayEmptySound();
			FailAttack(attackIdx, leftHand, akimboFire);
			return false;
		}
	}

	// must be here for prediction. Cannot be modified in an event or filtered by g_runfuncs=1.
	*clip -= opts.ammoCost;

	int ammoIdx = attackIdx == 0 ? m_iPrimaryAmmoType : m_iSecondaryAmmoType;

	if (isNormalAttack) {
		if (IsAkimbo()) {
			if (akimboFire)
				m_pPlayer->SetAnimation(PLAYER_ATTACK1);
			else if (leftHand)
				m_pPlayer->SetAnimation(PLAYER_ATTACK2);
			else
				m_pPlayer->SetAnimation(PLAYER_ATTACK3);
		}
		else {
			m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		}

		if (clipLeft <= 0 && ammoIdx >= 0 && m_pPlayer->m_rgAmmo[ammoIdx] <= 0) {
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", SUIT_REPEAT_OK, 0);
		}
	}

	Cooldown(attackIdx);

	return true;
}

void CWeaponCustom::Cooldown(int attackIdx, int overrideMillis) {
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	int millis = overrideMillis != -1 ? overrideMillis : opts.cooldown;
	float nextAttack = UTIL_WeaponTimeBase() + millis * 0.001f;
	
	if (attackIdx != -1) {
		if (params.flags & FL_WC_WEP_UNLINK_COOLDOWNS) {
			if (attackIdx == 0) {
				m_flNextPrimaryAttack = nextAttack;
			}
			else if (attackIdx == 1) {
				m_flNextSecondaryAttack = nextAttack;
			}
			else {
				m_flNextTertiaryAttack = nextAttack;
			}
		}
		else {
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = nextAttack;
		}
	}

	bool noAttack = (opts.flags & FL_WC_SHOOT_NO_ATTACK);
	bool alwaysIdleCooldown = opts.flags & FL_WC_SHOOT_COOLDOWN_IDLE;
		
	if (alwaysIdleCooldown || !noAttack)
		m_flTimeWeaponIdle = nextAttack + 1.0f;
}

bool CWeaponCustom::Chargeup(int attackIdx, bool leftHand, bool akimboFire) {
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	if (!opts.chargeTime)
		return true;

	if (params.flags & FL_WC_WEP_LINK_CHARGEUPS) {
		attackIdx = 0;
	}

	float& chargeStart = attackIdx == 0 ? m_flChargeStartPrimary : m_flChargeStartSecondary;

	if (!chargeStart) {
		chargeStart = WallTime();
		int e = (attackIdx == 0) ? WC_TRIG_PRIMARY_CHARGE : WC_TRIG_SECONDARY_CHARGE;
		ProcessEvents(e, 0, leftHand, akimboFire);
	}

	return WallTime() - chargeStart > opts.chargeTime * 0.001f;
}

void CWeaponCustom::FinishAttack(int attackIdx) {
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	if (params.flags & FL_WC_WEP_LINK_CHARGEUPS) {
		attackIdx = 0;
	}

	if (!g_runfuncs) {
		return;
	}

	bool attackCalled = attackIdx == 0 ? m_primaryCalled : m_secondaryCalled;
	float& chargeVar = attackIdx == 0 ? m_flChargeStartPrimary : m_flChargeStartSecondary;
	int ievt = attackIdx == 0 ? WC_TRIG_PRIMARY_STOP : WC_TRIG_SECONDARY_STOP;

	if (!attackCalled && chargeVar) {
		uint16_t cancelMillis = params.shootOpts[0].chargeCancelTime;
		float cancelTime = chargeVar + cancelMillis * 0.001f;

		if (!cancelMillis || WallTime() > cancelTime) {
			chargeVar = 0;
			m_waitForNextRunfuncs = true;
			CancelDelayedEvents();
			ProcessEvents(ievt, 0, false, false);
		}
	}
}

void CWeaponCustom::FailAttack(int attackIdx, bool leftHand, bool akimboFire) {
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
	int ievt = attackIdx == 0 ? WC_TRIG_PRIMARY_FAIL : WC_TRIG_SECONDARY_FAIL;

	ProcessEvents(ievt, akimboArg, leftHand, false);

	if (opts.cooldownFail)
		Cooldown(attackIdx, opts.cooldownFail);
}

void CWeaponCustom::KickbackPrediction() {
#ifdef CLIENT_DLL
	if (m_runningKickbackPred) {
		g_runningKickbackPred = 1;
		g_vApplyVel = m_kickbackPredVel;

		if (m_runningKickbackPred == 2 && g_runfuncs) {
			m_runningKickbackPred = 0;
			m_kickbackPredVel = Vector();
			g_runningKickbackPred = 0;
		}

		if (m_runningKickbackPred == 1)
			m_runningKickbackPred = 2;
	}
#endif
}

void CWeaponCustom::ToggleZoom(int zoomFov) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_pPlayer->m_iFOV) {
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	}
	else {
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = zoomFov;
	}

#ifdef CLIENT_DLL
	UpdateZoomCrosshair(m_iId, m_pPlayer->m_iFOV != 0);
#else
	UpdateAnimSet();
#endif

	m_lastZoomToggle = gpGlobals->time;
}

void CWeaponCustom::ToggleLaser() {
	SetLaser(!IsLaserOn());
	m_lastLaserToggle = WallTime();
	
#ifdef CLIENT_DLL
	if (!IsLaserOn()) {
		m_laserOnTime = 0;
	}

	HideLaser(!IsLaserOn());
#endif
}

void CWeaponCustom::HideLaser(bool hideNotUnhide) {
#ifdef CLIENT_DLL
	if (hideNotUnhide) {
		EV_LaserOff();
	}
	else {
		EV_LaserOn(params.laser.dotSprite, params.laser.dotSz / 10.0f,
			params.laser.beamSprite, params.laser.beamWidth / 10.0f,
			params.laser.attachment);
	}
#endif
}

void CWeaponCustom::CancelZoom() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_pPlayer->m_iFOV) {
		ToggleZoom(0);
	}
}

bool CWeaponCustom::CheckTracer(int idx, Vector& vecSrc, Vector forward, Vector right, int iTracerFreq)
{
	if (idx < 0 || idx >= 32) {
		return false;
	}

	if (iTracerFreq != 0 && ((m_tracerCount[idx])++ % iTracerFreq) == 0)
	{
		// adjust for player
		vecSrc = vecSrc + Vector(0,0,-4) + right * 2 + forward * 16;
		return true;
	}

	return false;
}



void CWeaponCustom::SendPredictionData(edict_t* target) {
#ifndef CLIENT_DLL
	int sentBytes = 0;
	MESSAGE_BEGIN(MSG_ONE, gmsgCustomWeapon, NULL, target);
	WRITE_BYTE(m_iId); sentBytes += 1;
	WRITE_SHORT(params.flags); sentBytes += 2;
	WRITE_SHORT(params.maxClip); sentBytes += 2;

	WRITE_SHORT(params.vmodel); sentBytes += 2;
	WRITE_BYTE(params.deployAnim); sentBytes += 1;

	for (int k = 0; k < 3; k++) {
		WeaponCustomReload& reload = params.reloadStage[k];
		WRITE_BYTE(reload.anim); sentBytes += 1;
		WRITE_SHORT(reload.time); sentBytes += 2;

		if (k == 2 && !(params.flags & FL_WC_WEP_SHOTGUN_RELOAD))
			break;
	}

	for (int k = 0; k < 4; k++) {
		WeaponCustomIdle& idle = params.idles[k];
		WRITE_BYTE(idle.anim); sentBytes += 1;
		WRITE_BYTE(idle.weight); sentBytes += 1;
		WRITE_SHORT(idle.time); sentBytes += 2;
	}

	if (params.flags & FL_WC_WEP_AKIMBO) {
		for (int k = 0; k < 4; k++) {
			WeaponCustomIdle& idle = params.akimbo.idles[k];
			WRITE_BYTE(idle.anim); sentBytes += 1;
			WRITE_BYTE(idle.weight); sentBytes += 1;
			WRITE_SHORT(idle.time); sentBytes += 2;
		}

		WRITE_BYTE(params.akimbo.reload.anim); sentBytes += 1;
		WRITE_SHORT(params.akimbo.reload.time); sentBytes += 2;

		WRITE_BYTE(params.akimbo.deployAnim); sentBytes += 1;
		WRITE_SHORT(params.akimbo.deployTime); sentBytes += 2;
		WRITE_BYTE(params.akimbo.akimboDeployAnim); sentBytes += 1;
		WRITE_SHORT(params.akimbo.akimboDeployTime); sentBytes += 2;
		WRITE_BYTE(params.akimbo.holsterAnim); sentBytes += 1;
		WRITE_SHORT(params.akimbo.holsterTime); sentBytes += 2;
	}

	if (params.flags & FL_WC_WEP_HAS_LASER) {
		for (int k = 0; k < 4; k++) {
			WeaponCustomIdle& idle = params.laser.idles[k];
			WRITE_BYTE(idle.anim); sentBytes += 1;
			WRITE_BYTE(idle.weight); sentBytes += 1;
			WRITE_SHORT(idle.time); sentBytes += 2;
		}

		WRITE_SHORT(params.laser.dotSprite);
		WRITE_SHORT(params.laser.beamSprite);
		WRITE_BYTE(params.laser.dotSz);
		WRITE_BYTE(params.laser.beamWidth);
		WRITE_BYTE(params.laser.attachment);
	}

	for (int k = 0; k < 4; k++) {
		if (!(params.flags & FL_WC_WEP_HAS_PRIMARY) && k == 0)
			continue;
		if (!(params.flags & FL_WC_WEP_HAS_SECONDARY) && k == 1)
			continue;
		if (!(params.flags & FL_WC_WEP_HAS_TERTIARY) && k == 2)
			continue;
		if (!(params.flags & FL_WC_WEP_HAS_ALT_PRIMARY) && k == 3)
			continue;

		CustomWeaponShootOpts& opts = params.shootOpts[k];
		WRITE_BYTE(opts.flags); sentBytes += 1;
		WRITE_BYTE(opts.ammoCost); sentBytes += 1;
		WRITE_SHORT(opts.cooldown); sentBytes += 2;
		WRITE_SHORT(opts.cooldownFail); sentBytes += 2;
		WRITE_SHORT(opts.chargeTime); sentBytes += 2;
		WRITE_SHORT(opts.chargeCancelTime); sentBytes += 2;
	}
	MESSAGE_END();

	int mainBytes = sentBytes;
	sentBytes = 0;

	MESSAGE_BEGIN(MSG_ONE, gmsgCustomWeaponEvents, NULL, target);
	WRITE_BYTE(m_iId); sentBytes += 1;
	WRITE_BYTE(params.numEvents); sentBytes += 1;

	for (int k = 0; k < params.numEvents; k++) {
		WepEvt& evt = params.events[k];
		uint16_t packedHeader = (evt.hasDelay << 15) | (evt.triggerArg << 10) | (evt.trigger << 5) | evt.evtType;
		WRITE_SHORT(packedHeader); sentBytes += 2;
		if (evt.hasDelay)
			WRITE_SHORT(evt.delay); sentBytes += 2;

		switch (evt.evtType) {
		case WC_EVT_IDLE_SOUND: {
			uint16_t packedFlags = (evt.idleSound.sound << 7) | evt.idleSound.volume;
			WRITE_SHORT(packedFlags); sentBytes += 2;
			break;
		}
		case WC_EVT_PLAY_SOUND: {
			uint16_t packedFlags = evt.playSound.sound << 5 | evt.playSound.channel << 2 | evt.playSound.aiVol;
			WRITE_SHORT(packedFlags); sentBytes += 2;
			WRITE_BYTE(evt.playSound.volume); sentBytes += 1;
			WRITE_BYTE(evt.playSound.attn); sentBytes += 1;
			WRITE_BYTE(evt.playSound.pitchMin); sentBytes += 1;
			WRITE_BYTE(evt.playSound.pitchMax); sentBytes += 1;

			WRITE_BYTE(evt.playSound.numAdditionalSounds);
			for (int i = 0; i < evt.playSound.numAdditionalSounds; i++) {
				WRITE_SHORT(evt.playSound.additionalSounds[i]);
			}

			//WRITE_BYTE(evt.playSound.distantSound); sentBytes += 1; // not needed for prediction
			break;
		}
		case WC_EVT_EJECT_SHELL:
			WRITE_SHORT(evt.ejectShell.model); sentBytes += 2;
			WRITE_SHORT(evt.ejectShell.offsetForward); sentBytes += 2;
			WRITE_SHORT(evt.ejectShell.offsetUp); sentBytes += 2;
			WRITE_SHORT(evt.ejectShell.offsetRight); sentBytes += 2;
			break;
		case WC_EVT_PUNCH:
			WRITE_BYTE(evt.punch.flags); sentBytes += 1;
			WRITE_SHORT(evt.punch.x); sentBytes += 2;
			WRITE_SHORT(evt.punch.y); sentBytes += 2;
			WRITE_SHORT(evt.punch.z); sentBytes += 2;
			break;
		case WC_EVT_SET_BODY:
			WRITE_BYTE(evt.setBody.newBody); sentBytes += 1;
			break;
		case WC_EVT_WEP_ANIM: {
			uint8_t packedHeader = (evt.anim.flags << 6) | (evt.anim.akimbo << 3) | evt.anim.numAnim;
			WRITE_BYTE(packedHeader); sentBytes += 1;
			for (int i = 0; i < evt.anim.numAnim; i++) {
				WRITE_BYTE(evt.anim.anims[i]); sentBytes += 1;
			}
			break;
		}
		case WC_EVT_BULLETS: {
			WRITE_BYTE(evt.bullets.count); sentBytes += 1;
			WRITE_SHORT(evt.bullets.burstDelay); sentBytes += 2;
			//WRITE_SHORT(evt.bullets.damage); sentBytes += 2; // not needed for prediction
			WRITE_SHORT(evt.bullets.spreadX); sentBytes += 2;
			WRITE_SHORT(evt.bullets.spreadY); sentBytes += 2;
			WRITE_BYTE(evt.bullets.tracerFreq); sentBytes += 1;

			uint8_t packedFlags = (evt.bullets.flags << 4) | evt.bullets.flashSz;
			WRITE_BYTE(packedFlags); sentBytes += 1;
		}
			break;
		case WC_EVT_KICKBACK:
			WRITE_SHORT(evt.kickback.pushForce); sentBytes += 2;
			break;
		case WC_EVT_TOGGLE_ZOOM:
			WRITE_BYTE(evt.zoomToggle.zoomFov); sentBytes += 1;
			break;
		case WC_EVT_HIDE_LASER:
			WRITE_SHORT(evt.laserHide.millis); sentBytes += 2;
			break;
		case WC_EVT_COOLDOWN:
			WRITE_SHORT(evt.cooldown.millis); sentBytes += 2;
			WRITE_BYTE(evt.cooldown.targets); sentBytes += 1;
			break;
		case WC_EVT_TOGGLE_AKIMBO:
		case WC_EVT_TOGGLE_LASER:
		case WC_EVT_PROJECTILE:
			break;
		default:
			ALERT(at_error, "Invalid custom weapon event type %d\n", evt.evtType);
			break;
		}
	}
	MESSAGE_END();

	int evBytes = sentBytes;
	ALERT(at_console, "Sent %d prediction bytes for %s (%d + %d evt)\n",
		evBytes + mainBytes, STRING(pev->classname), mainBytes, evBytes);
#endif
}

int CWeaponCustom::SendSoundMappingChunk(CBasePlayer* target, std::vector<SoundMapping>& chunk) {
	int sentBytes = 0;
#ifndef CLIENT_DLL
	MESSAGE_BEGIN(MSG_ONE, gmsgSoundIdx, NULL, target->pev);
	WRITE_BYTE(chunk.size()); sentBytes += 1;
	for (int k = 0; k < chunk.size(); k++) {
		WRITE_SHORT(chunk[k].idx); sentBytes += 2;
		WRITE_STRING(chunk[k].path); sentBytes += strlen(chunk[k].path) + 1;
	}
	MESSAGE_END();
#endif
	return sentBytes;
}

void CWeaponCustom::SendSoundMapping(CBasePlayer* target) {
	// send over the sound index mapping
#ifndef CLIENT_DLL
	std::vector<SoundMapping> chunk;
	int soundListBytes = 0;
	int soundCount = 0;
	int chunkSz = 1;
	int chunkCount = 0;
	for (int i = 0; i < MAX_PRECACHE_SOUND; i++) {
		if (!m_customWeaponSounds[i]) {
			continue;
		}
		const char* path = INDEX_SOUND(i);
		int addSz = strlen(path) + 2;

		if (chunkSz + addSz > 186) {
			// filled up a message buffer. Send it before continuing.
			soundListBytes += SendSoundMappingChunk(target, chunk);
			chunkSz = 1;
			chunkCount++;
			chunk.clear();
		}
		chunkSz += addSz;

		SoundMapping mapping;
		mapping.idx = i;
		mapping.path = path;
		chunk.push_back(mapping);
	}

	if (chunk.size()) {
		soundListBytes += SendSoundMappingChunk(target, chunk);
		chunkCount++;
	}

	ALERT(at_console, "Sent %d sound list bytes in %d chunks\n", soundListBytes, chunkCount);
#endif
}



void CWeaponCustom::ProcessEvents(int trigger, int triggerArg, bool leftHand, bool akimboFire, int clipLeft) {
#ifdef CLIENT_DLL
	if (!g_runfuncs)
		return;
#endif

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	for (int i = 0; i < params.numEvents; i++) {
		WepEvt& evt = params.events[i];

		if (evt.trigger != trigger)
			continue;

		bool argMatch = true;
		switch (trigger) {

		case WC_TRIG_PRIMARY:
		case WC_TRIG_SECONDARY:
		case WC_TRIG_TERTIARY:
		case WC_TRIG_PRIMARY_EVEN:
		case WC_TRIG_PRIMARY_ODD:
		case WC_TRIG_PRIMARY_NOT_EMPTY:
		case WC_TRIG_RELOAD:
		case WC_TRIG_RELOAD_EMPTY:
		case WC_TRIG_RELOAD_NOT_EMPTY:
		case WC_TRIG_DEPLOY:
			argMatch = evt.triggerArg == WC_TRIG_SHOOT_ARG_ALWAYS || triggerArg == evt.triggerArg;
			break;
		case WC_TRIG_PRIMARY_CLIPSIZE:
			argMatch = triggerArg == evt.triggerArg;
			break;
		default:
			break;
		}

		if (!argMatch)
			continue;

		if (evt.delay == 0) {
			PlayEvent(i, leftHand, akimboFire);

			if (evt.evtType == WC_EVT_BULLETS && evt.bullets.burstDelay) {
				float burstDelay = 0;
				int additionalBullets = evt.bullets.count - 1;
				if (clipLeft < 0) // clip went negative due to exceeding cost for a burst
					additionalBullets += clipLeft;
				for (int k = 0; k < additionalBullets; k++) {
					burstDelay += evt.bullets.burstDelay * 0.001f;
					QueueDelayedEvent(i, WallTime() + burstDelay, leftHand, akimboFire);
				}
			}
		}
		else {
			QueueDelayedEvent(i, WallTime() + evt.delay * 0.001f, leftHand, akimboFire);
		}
	}
}

void CWeaponCustom::QueueDelayedEvent(int eventIdx, float fireTime, bool leftHand, bool akimboFire) {
	for (int i = 0; i < WC_SERVER_EVENT_QUEUE_SZ; i++) {
		WcDelayEvent& qevt = eventQueue[i];

		// find an empty slot
		if (qevt.fireTime == 0) {
			qevt.eventIdx = eventIdx;
			qevt.fireTime = fireTime;
			qevt.leftHand = leftHand;
			qevt.akimboFire = akimboFire;
			//CLALERT("Queue event %d\n", eventIdx);
			return;
		}
	}

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;
	ALERT(at_console, "Server event queue is full for %s on player %s\n", STRING(pev->classname), m_pPlayer->DisplayName());
}

void CWeaponCustom::PlayEvent_Bullets(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand, bool akimboFire) {
	Vector spread(SPREAD_TO_FLOAT(evt.bullets.spreadX), SPREAD_TO_FLOAT(evt.bullets.spreadY), 0);

	if (evt.bullets.flags & FL_WC_BULLETS_DYNAMIC_SPREAD) {
		if ((m_pPlayer->pev->button & IN_DUCK) != 0)
		{
			spread = spread * 0.5f;
		}
		else if ((m_pPlayer->pev->button & (IN_MOVERIGHT | IN_MOVELEFT | IN_FORWARD | IN_BACK)) != 0)
		{
			spread = spread * 2.0f;
		}
	}

	if (evt.bullets.flashSz) {
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

		switch (evt.bullets.flashSz) {
		case WC_FLASH_DIM:
			m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
			break;
		case WC_FLASH_NORMAL:
			m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
			break;
		case WC_FLASH_BRIGHT:
			m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
			break;
		default:
			break;
		}
	}

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	Vector vecEnd;

	lagcomp_begin(m_pPlayer);
	Vector vecDir = m_pPlayer->FireBulletsPlayer(evt.bullets.count, vecSrc, vecAiming, spread, 8192,
		BULLET_PLAYER_9MM, evt.bullets.tracerFreq, evt.bullets.damage, m_pPlayer->pev,
		m_pPlayer->random_seed, &vecEnd, true);
	lagcomp_end();

#ifdef CLIENT_DLL
	int eidx = 0;
#else
	int eidx = m_pPlayer->entindex() - 1;
#endif

	bool showTracer = CheckTracer(eidx, vecSrc, vecDir, gpGlobals->v_right, evt.bullets.tracerFreq);

#ifdef CLIENT_DLL
	bool decal = !(evt.bullets.flags & FL_WC_BULLETS_NO_DECAL);
	bool texSound = !(evt.bullets.flags & FL_WC_BULLETS_NO_SOUND);
	WC_EV_Bullets(evt, vecDir.x, vecDir.y, showTracer, decal, texSound);
#else
	if (showTracer) {
		for (int i = 1; i < gpGlobals->time; i++) {
			CBasePlayer* listener = UTIL_PlayerByIndex(i);

			if (!listener) {
				continue;
			}

			if (m_pPlayer != listener && m_pPlayer->InPAS(listener->edict())) {
				UTIL_Tracer(vecSrc, vecEnd, MSG_ONE_UNRELIABLE, listener->edict());
			}
		}
	}
#endif

	int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
	ProcessEvents(WC_TRIG_BULLET_FIRED, akimboArg, leftHand, akimboFire);
}

void CWeaponCustom::PlayEvent_Projectile(WepEvt& evt, CBasePlayer* m_pPlayer) {
#ifndef CLIENT_DLL
	MAKE_VECTORS(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	float x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
	float y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);

	Vector vForward = gpGlobals->v_forward;
	Vector vRight = gpGlobals->v_right;
	Vector vUp = gpGlobals->v_up;
	Vector vecSpread(evt.proj.spreadX, evt.proj.spreadY, 0);
	
	Vector vecDir = vForward +
		x * vecSpread.x * vRight +
		y * vecSpread.y * vUp;

	// Get amount of player velocity to add to projectile.
	Vector inf = evt.proj.player_vel_inf;
	Vector pvel = m_pPlayer->pev->velocity;
	pvel =	vRight		* DotProduct(vRight,	pvel)	* inf.x +
			vUp			* DotProduct(vUp,		pvel)	* inf.y +
			vForward	* DotProduct(vForward,	pvel)	* inf.z;

	Vector dir = evt.proj.dir;
	Vector projectile_velocity = pvel +
		dir.x * evt.proj.speed * vRight +
		dir.y * evt.proj.speed * Vector(0, 0, 1) +
		dir.z * evt.proj.speed * vForward;

	Vector offsetOpts = evt.proj.offset;
	Vector ofs = vRight * offsetOpts.x + vForward * offsetOpts.y + vUp * offsetOpts.z;
	Vector projectile_ori = m_pPlayer->GetGunPosition() + ofs;
	Vector projectile_dir_angles = UTIL_VecToAngles(projectile_velocity.Normalize());
	float grenadeTime = evt.proj.life != 0 ? evt.proj.life : 3.5f; // timed grenades only
	//if (state.active_opts.windup_time > 0)
	//	grenadeTime = Math.max(0, grenadeTime - (g_Engine.time - state.windupStart));

	CBaseEntity* shootEnt = NULL;
	switch (evt.proj.type) {
	case WC_PROJECTILE_ARGRENADE:
		shootEnt = CGrenade::ShootContact(m_pPlayer->pev, projectile_ori, projectile_velocity);
		break;
	case WC_PROJECTILE_BANANA:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_BANANA Not implemented\n");
		break;
	case WC_PROJECTILE_BOLT:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_BOLT Not implemented\n");
		break;
	case WC_PROJECTILE_HVR:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_HVR Not implemented\n");
		break;
	case WC_PROJECTILE_SHOCK:
		shootEnt = CShockBeam::CreateShockBeam(projectile_ori, projectile_dir_angles, m_pPlayer);
		break;
	case WC_PROJECTILE_HORNET:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_HORNET Not implemented\n");
		break;
	case WC_PROJECTILE_DISPLACER:
		shootEnt = CDisplacerBall::CreateDisplacerBall(projectile_ori, projectile_dir_angles, m_pPlayer);
		break;
	case WC_PROJECTILE_GRENADE:
		shootEnt = CGrenade::ShootTimed(m_pPlayer->pev, projectile_ori, projectile_velocity, grenadeTime);
		break;
	case WC_PROJECTILE_MORTAR:
		shootEnt = ShootMortar(m_pPlayer->edict(), projectile_ori, projectile_velocity);
		break;
	case WC_PROJECTILE_RPG:
		shootEnt = CRpgRocket::CreateRpgRocket(projectile_ori, m_pPlayer->pev->v_angle, m_pPlayer, NULL);
		break;
	case WC_PROJECTILE_WEAPON:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_WEAPON Not implemented\n");
		break;
	case WC_PROJECTILE_TRIPMINE: {
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_TRIPMINE Not implemented\n");
		/*
		// assumes MakeVectors was already called
		Vector vecSrc = m_pPlayer->GetGunPosition();

		// Find a good tripmine location
		TraceResult tr;
		Vector vecEnd = vecSrc + vForward * state.active_opts.max_range;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);
		if (tr.flFraction >= 1.0 && state.active_opts.pev.spawnflags & FL_SHOOT_IF_NOT_MISS != 0)
		{
			state.abortAttack = true;
		}
		else
		{
			Vector tripOri = tr.vecEndPos + tr.vecPlaneNormal * 8;
			Vector angles = UTIL_VecToAngles(tr.vecPlaneNormal);
			angles.x *= -1; // not sure why hlsdk doesn't do this
			const char* tripClass = evt.proj.entity_class ? STRING(evt.proj.entity_class) : "monster_tripmine";
			shootEnt = ShootCustomProjectile(state, tripClass, tripOri, projectile_velocity, angles);
		}
		*/
		break;
	}
	case WC_PROJECTILE_CUSTOM:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_CUSTOM Not implemented\n");
		break;
	case WC_PROJECTILE_OTHER:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_OTHER Not implemented\n");
		break;
	default:
		ALERT(at_error, "WeaponCustom: Unknown projectile type %d\n", evt.proj.type);
		break;
	}

	if (shootEnt)
	{
		if (evt.proj.follow_mode != WC_PROJ_FOLLOW_NONE)
		{
			EHANDLE h_plr = m_pPlayer->edict();
			EHANDLE h_proj = shootEnt->edict();
			float dur = evt.proj.follow_time[1];
			ALERT(at_error, "WeaponCustom: Projectile follow mode not implemented\n");
			//g_Scheduler.SetTimeout("projectile_follow_aim", evt.proj.follow_time[0], h_plr, h_proj, @state.active_opts, dur);
		}

		EHANDLE mdlHandle = shootEnt->edict();
		EHANDLE sprHandle;

		// TODO: Kill this when follow target dies (and its not a custom entity)
		if (evt.proj.sprite)
		{
			Vector ori = shootEnt->pev->origin;
			RGBA c = evt.proj.sprite_color;
			std::string colorString = UTIL_VarArgs("%d %d %d", (int)c.r, (int)c.g, (int)c.b);
			std::string scaleString = UTIL_VarArgs("%.4f", evt.proj.sprite_scale);
			StringMap keyvalues = {
				{"model", INDEX_MODEL(evt.proj.sprite)},
				{"rendermode", "5"},
				{"renderamt", std::to_string(evt.proj.sprite_color[3]).c_str()},
				{"rendercolor", colorString.c_str()},
				{"scale", scaleString.c_str()},
			};
			CBaseEntity* spr = CBaseEntity::Create("env_sprite", ori, g_vecZero, true, NULL, keyvalues);
			spr->pev->movetype = MOVETYPE_FOLLOW;
			spr->pev->aiment = shootEnt->edict();
			spr->pev->skin = shootEnt->entindex();
			spr->pev->body = 0; // attachement point
			sprHandle = spr;
		}

		/*
		WeaponCustomProjectile@ shootEnt_c = cast<WeaponCustomProjectile@>(CastToScriptClass(shootEnt));
		if (shootEnt_c !is null)
			shootEnt_c.spriteAttachment = sprHandle;
		*/

		// attach a trail
		if (evt.proj.trail_spr)
		{
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_BEAMFOLLOW);
			WRITE_SHORT(shootEnt->entindex());
			WRITE_SHORT(evt.proj.trail_spr);
			WRITE_BYTE(evt.proj.trail_life);
			WRITE_BYTE(evt.proj.trail_width);
			WRITE_BYTE(evt.proj.trail_color[0]);
			WRITE_BYTE(evt.proj.trail_color[1]);
			WRITE_BYTE(evt.proj.trail_color[2]);
			WRITE_BYTE(evt.proj.trail_color[3]);
			MESSAGE_END();
		}

		if (evt.proj.life)
			ALERT(at_error, "WeaponCustom: Projectile life not implemented\n");

		/*
		if (evt.proj.life > 0)
			g_Scheduler.SetTimeout("killProjectile", options.life, mdlHandle, sprHandle, state.active_opts);

		if (state.c_wep !is null and (state.c_wep.settings.max_live_projectiles > 0 or state.c_wep.settings.pev.spawnflags & FL_WEP_WAIT_FOR_PROJECTILES != 0))
		{
			state.liveProjectiles++;
			MonitorProjectileLife(state, mdlHandle);
		}
		*/

		if (evt.proj.hasAvel)
			shootEnt->pev->avelocity = evt.proj.avel;

		// TODO: Allow setting projectile class and ally status
		int rel = m_pPlayer->IRelationship(shootEnt);
		bool isFriendly = rel == R_AL || rel == R_NO;
		if (shootEnt->IsMonster() && !isFriendly)
			shootEnt->SetClassification(CLASS_PLAYER_ALLY);

		// TODO: health set here
		shootEnt->pev->friction = 1.0f - evt.proj.elasticity;
		shootEnt->pev->gravity = evt.proj.gravity;

		/*
		if (shootEnt->pev->gravity == 0)
		{
			if (evt.proj.world_event != WC_PROJ_ACT_BOUNCE && evt.proj.monster_event != WC_PROJ_ACT_BOUNCE)
				shootEnt->pev->movetype = MOVETYPE_FLY; // fixes jittering in grapple tongue shoot
			else
				shootEnt->pev->movetype = MOVETYPE_BOUNCEMISSILE;
		}
		*/
	}

	// remove weapon from player if they threw it
	if (evt.proj.type == WC_PROJECTILE_WEAPON) {
		UTIL_Remove(this);
	}
#endif
}

void CWeaponCustom::PlayEvent_Kickback(WepEvt& evt, CBasePlayer* m_pPlayer) {
#ifdef CLIENT_DLL
	Vector forward, right, up;
	gEngfuncs.pfnAngleVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle, forward, right, up);
	m_runningKickbackPred = 1;
	m_kickbackPredVel = forward * -evt.kickback.pushForce;
#else
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecDir = gpGlobals->v_forward;
	m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - vecDir * evt.kickback.pushForce;
#endif
}

void CWeaponCustom::PlayEvent_Sound(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand, bool akimboFire) {
	int channel = CHAN_STATIC;
	int pitch = 100;
	float volume = evt.idleSound.volume / 127.0f;
	float attn = ATTN_IDLE;
	int idx = evt.idleSound.sound;

	if (evt.evtType == WC_EVT_PLAY_SOUND) {
		idx = evt.playSound.sound;
		if (evt.playSound.numAdditionalSounds) {
			int rnd = UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, evt.playSound.numAdditionalSounds);
			if (rnd > 0) {
				idx = evt.playSound.additionalSounds[rnd - 1];
			}
		}

		channel = evt.playSound.channel;
		pitch = UTIL_SharedRandomLong(m_pPlayer->random_seed, evt.playSound.pitchMin, evt.playSound.pitchMax);
		volume = evt.playSound.volume / 255.0f;
		attn = evt.playSound.attn / 64.0f;

		switch (evt.playSound.aiVol) {
		case WC_AIVOL_QUIET:
			m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
			break;
		case WC_AIVOL_NORMAL:
			m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
			break;
		case WC_AIVOL_LOUD:
			m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
			break;
		default:
			break;
		}
	}

#ifdef CLIENT_DLL
	int panning = 0;
	if (akimboFire)
		panning = leftHand ? 1 : 2; // signal the event player to pan the audio

	WC_EV_LocalSound(evt, idx, channel, pitch, volume, attn, panning);
#else
	uint32_t messageTargets = 0xffffffff & ~PLRBIT(m_pPlayer->edict());

	StartSound(m_pPlayer->edict(), channel, INDEX_SOUND(idx), volume, attn,
		SND_FL_PREDICTED, 100, m_pPlayer->pev->origin, messageTargets);

	if (evt.playSound.distantSound) {
		PLAY_DISTANT_SOUND(m_pPlayer->edict(), evt.playSound.distantSound);
	}
#endif
}

void CWeaponCustom::PlayEvent_EjectShell(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand) {
#ifdef CLIENT_DLL
	WC_EV_EjectShell(evt, leftHand);
#else
	Vector ori = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs;
	Vector vel = m_pPlayer->pev->velocity;
	Vector angles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

	Vector forward, right, up;
	UTIL_MakeVectorsPrivate(angles, forward, right, up);

	if (leftHand)
		right = right * -1;

	float forwardScale = evt.ejectShell.offsetForward * 0.01f;
	float upScale = evt.ejectShell.offsetUp * 0.01f;
	float rightScale = evt.ejectShell.offsetRight * 0.01f;

	float fR = RANDOM_FLOAT(50, 70);
	float fU = RANDOM_FLOAT(100, 150);

	Vector ShellVelocity = vel + right * fR + up * fU + forward * 25;
	Vector ShellOrigin = ori + up* upScale + forward*forwardScale + right*rightScale;

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (!plr || plr == m_pPlayer)
			continue;

		if (m_pPlayer->InPAS(plr->edict())) {
			EjectBrass(ShellOrigin, ShellVelocity, angles.y, evt.ejectShell.model, TE_BOUNCE_SHELL, plr->edict());
		}		
	}
#endif
}

void CWeaponCustom::PlayEvent_PunchAngle(WepEvt& evt, CBasePlayer* m_pPlayer) {
	bool ducking = m_pPlayer->pev->flags & FL_DUCKING;

	if (ducking && (evt.punch.flags & FL_WC_PUNCH_STAND))
		return;
	if (!ducking && (evt.punch.flags & FL_WC_PUNCH_DUCK))
		return;

#ifdef CLIENT_DLL
	WC_EV_PunchAngle(evt, m_pPlayer->random_seed);
#else
	float punchAngleX = FP_10_6_TO_FLOAT(evt.punch.x);
	float punchAngleY = FP_10_6_TO_FLOAT(evt.punch.y);
	float punchAngleZ = FP_10_6_TO_FLOAT(evt.punch.z);

	if (!(evt.punch.flags & FL_WC_PUNCH_NO_RETURN)) {
		if (evt.punch.flags & FL_WC_PUNCH_SET) {
			m_pPlayer->pev->punchangle = Vector(punchAngleX, punchAngleY, punchAngleZ);
		}
		else {
			m_pPlayer->pev->punchangle = Vector(
				UTIL_SharedRandomFloat(m_pPlayer->random_seed, -punchAngleX, punchAngleX),
				UTIL_SharedRandomFloat(m_pPlayer->random_seed + 1, -punchAngleY, punchAngleY),
				UTIL_SharedRandomFloat(m_pPlayer->random_seed + 2, -punchAngleZ, punchAngleZ)
			);
		}
	}
#endif
}

void CWeaponCustom::PlayEvent_WepAnim(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand) {
	if (!evt.anim.numAnim)
		return;

	int idx = UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, evt.anim.numAnim - 1);
	int anim = evt.anim.anims[idx];

	bool leftOnly = evt.anim.akimbo == WC_ANIM_LEFT_HAND || (evt.anim.akimbo == WC_ANIM_TRIG_HAND && leftHand);
	if (evt.anim.akimbo == WC_ANIM_BOTH_HANDS || leftOnly) {
		SendAkimboAnim(anim);
		if (leftOnly)
			return; // don't play the right hand event
	}

	if (evt.anim.flags & FL_WC_ANIM_PMODEL) {
		m_pPlayer->SetThirdPersonWeaponAnim(anim);
		return;
	}

#ifdef CLIENT_DLL
	cl_entity_t* vmodel = gEngfuncs.GetViewModel();
	int currentAnim = vmodel ? vmodel->curstate.sequence : 0;
	if ((evt.anim.flags & FL_WC_ANIM_NO_RESET) && anim == currentAnim) {
		return;
	}

	WC_EV_WepAnim(evt, m_iId, anim);
#else
	SendWeaponAnimSpec(anim);
#endif
}

void CWeaponCustom::PlayEvent_Cooldown(WepEvt& evt, CBasePlayer* m_pPlayer) {
	float nextAction = UTIL_WeaponTimeBase() + evt.cooldown.millis * 0.001f;
	if (evt.cooldown.targets & FL_WC_COOLDOWN_PRIMARY) {
		m_flNextPrimaryAttack = nextAction;
	}
	if (evt.cooldown.targets & FL_WC_COOLDOWN_SECONDARY) {
		m_flNextSecondaryAttack = nextAction;
	}
	if (evt.cooldown.targets & FL_WC_COOLDOWN_TERTIARY) {
		m_flNextTertiaryAttack = nextAction;
	}
	if (evt.cooldown.targets & FL_WC_COOLDOWN_IDLE) {
		m_flTimeWeaponIdle = nextAction;
	}
}

void CWeaponCustom::PlayEvent_ToggleAkimbo(WepEvt& evt, CBasePlayer* m_pPlayer) {
	SetAkimbo(!IsAkimbo());

	if (IsAkimbo()) {
		SendAkimboAnim(params.akimbo.deployAnim);
	}
	else {
		Deploy();
	}
}

void CWeaponCustom::PlayEvent_HideLaser(WepEvt& evt, CBasePlayer* m_pPlayer) {
	if (IsLaserOn()) {
		HideLaser(true);
		m_laserOnTime = WallTime() + evt.laserHide.millis * 0.001f;
	}
}

void CWeaponCustom::PlayEvent(int eventIdx, bool leftHand, bool akimboFire) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	Vector vecDir;
	WepEvt& evt = params.events[eventIdx];

	switch (evt.evtType) {
	case WC_EVT_SET_BODY:
		pev->body = evt.setBody.newBody;
		break;
	case WC_EVT_BULLETS:
		PlayEvent_Bullets(evt, m_pPlayer, leftHand, akimboFire);
		break;
	case WC_EVT_PROJECTILE:
		PlayEvent_Projectile(evt, m_pPlayer);
		break;
	case WC_EVT_KICKBACK:
		PlayEvent_Kickback(evt, m_pPlayer);
		break;
	case WC_EVT_EJECT_SHELL:
		PlayEvent_EjectShell(evt, m_pPlayer, leftHand);
		break;
	case WC_EVT_PUNCH:
		PlayEvent_PunchAngle(evt, m_pPlayer);
		break;
	case WC_EVT_IDLE_SOUND:
		PlayEvent_Sound(evt, m_pPlayer, leftHand, akimboFire);
		break;
	case WC_EVT_PLAY_SOUND:
		PlayEvent_Sound(evt, m_pPlayer, leftHand, akimboFire);
		break;
	case WC_EVT_TOGGLE_ZOOM:
		ToggleZoom(evt.zoomToggle.zoomFov);
		ProcessEvents(m_pPlayer->m_iFOV ? WC_TRIG_ZOOM_IN : WC_TRIG_ZOOM_OUT, 0);
		break;
	case WC_EVT_TOGGLE_LASER:
		ToggleLaser();
		ProcessEvents(IsLaserOn() ? WC_TRIG_LASER_ON : WC_TRIG_LASER_OFF, 0);
		break;
	case WC_EVT_HIDE_LASER:
		PlayEvent_HideLaser(evt, m_pPlayer);
		break;
	case WC_EVT_WEP_ANIM:
		PlayEvent_WepAnim(evt, m_pPlayer, leftHand);
		break;
	case WC_EVT_COOLDOWN:
		PlayEvent_Cooldown(evt, m_pPlayer);
		break;
	case WC_EVT_TOGGLE_AKIMBO:
		PlayEvent_ToggleAkimbo(evt, m_pPlayer);
		break;
	default:
		ALERT(at_error, "Unhandled weapon event type %d\n", evt.evtType);
		break;
	}
}

void CWeaponCustom::PlayDelayedEvents() {
#ifdef CLIENT_DLL
	if (!g_runfuncs)
		return;
#endif

	for (int i = 0; i < WC_SERVER_EVENT_QUEUE_SZ; i++) {
		WcDelayEvent& qevt = eventQueue[i];

		if (!qevt.fireTime || qevt.fireTime > WallTime())
			continue;

		PlayEvent(qevt.eventIdx, qevt.leftHand, qevt.akimboFire);
		qevt.fireTime = 0; // free the slot
	}
}

void CWeaponCustom::CancelDelayedEvents() {
	for (int i = 0; i < WC_SERVER_EVENT_QUEUE_SZ; i++) {
		WcDelayEvent& qevt = eventQueue[i];
		qevt.fireTime = 0;
	}
}



float CWeaponCustom::WallTime() {
#ifdef CLIENT_DLL
	return gEngfuncs.GetClientTime();
#else
	return g_engfuncs.pfnTime();
#endif
}

void CWeaponCustom::SetAkimbo(bool akimbo) {
	m_fireState = akimbo ? 1 : 0;

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	UpdateAnimSet();
	m_pPlayer->pev->weaponmodel = MAKE_STRING(GET_MODEL(GetModelP()));
	m_pPlayer->SetAnimation(PLAYER_DEPLOY_WEAPON);
}

void CWeaponCustom::SendAkimboAnim(int iAnim) {
	if (!g_runfuncs)
		return;
	m_akimboAnim = iAnim;
	m_akimboAnimTime = WallTime();
}

void CWeaponCustom::SetLaser(bool enable) {
	m_flReleaseThrow = enable ? 1 : 0;

#ifdef CLIENT_DLL

#else
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (!IsLaserOn()) {
		UTIL_Remove(m_hLaserSpot);
	}
#endif
}

void CWeaponCustom::UpdateLaser() {
	if (m_laserOnTime && m_laserOnTime < WallTime()) {
		m_laserOnTime = 0;
	}
#ifdef CLIENT_DLL
	if (WallTime() - m_lastLaserToggle > 0.5f) {
		if (IsLaserOn())
			HideLaser(m_laserOnTime > 0);
		else
			HideLaser(true);
	}
#endif

#ifndef CLIENT_DLL
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (!IsLaserOn() || m_laserOnTime > 0) {
		UTIL_Remove(m_hLaserSpot);

		if (m_lastBeamUpdate) {
			UTIL_KillBeam(m_pPlayer->entindex(), MSG_PVS, m_pPlayer->pev->origin);
			m_lastBeamUpdate = 0;
		}
		return;
	}

	if (!m_hLaserSpot) {
		m_hLaserSpot = CLaserSpot::CreateSpot(m_pPlayer->edict());
		m_hLaserSpot->m_hidePlayers |= PLRBIT(m_pPlayer->edict());
	}

	CLaserSpot* m_pSpot = (CLaserSpot*)m_hLaserSpot.GetEntity();
	if (!m_pSpot)
		return;

	m_pSpot->pev->scale = params.laser.dotSz * 0.1f;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = gpGlobals->v_forward;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + vecAiming * 8192, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (UTIL_PointContents(tr.vecEndPos) == CONTENTS_SKY) {
		// back up until out of the sky, or else the client won't render the laser beam
		Vector delta = tr.vecEndPos - vecSrc;
		Vector bestPos = tr.vecEndPos;
		for (float f = 0.01f; f <= 1.0f; f += 0.02f) {
			bestPos = tr.vecEndPos - (delta * f);
			if (UTIL_PointContents(bestPos) != CONTENTS_SKY) {
				break;
			}
		}

		m_pSpot->pev->renderamt = 1; // almost invisible, but still rendered so laser beam works
		UTIL_SetOrigin(m_pSpot->pev, bestPos);
	}
	else {
		m_pSpot->pev->renderamt = 255;
		UTIL_SetOrigin(m_pSpot->pev, tr.vecEndPos);
	}

	if (gpGlobals->time - m_lastBeamUpdate >= 0.95f && !(m_pSpot->pev->effects & EF_NODRAW)) {
		// WARNING: Creating a beam entity that uses attachments has caused client crashes before,
		// but I haven't seen that happen yet with TE_BEAMENTS. If this causes crashes again,
		// then revert to using BeamEntPoint (attached to the player, not spot).
		m_lastBeamUpdate = gpGlobals->time;

		// show the beam to everyone except the player, unless they're in a third-person view
		for (int i = 1; i < gpGlobals->maxClients; i++) {
			CBasePlayer* plr = UTIL_PlayerByIndex(i);

			if (!plr) {
				continue;
			}

			int beamWidth = 8;

			if (plr == m_pPlayer && plr->IsSevenKewpClient())
				continue; // own laser is predicted

			// is first-person and owner
			if (plr == m_pPlayer && plr->m_hViewEntity.GetEntity() == plr) {
				if (!m_hasLaserAttachment)
					continue;

				// width 8 looks too thick in first-person view but just thick enough for others
				beamWidth = 1;
			}

			edict_t* ed = plr->edict();
			if (plr == m_pPlayer || m_pPlayer->isVisibleTo(ed) || m_pSpot->isVisibleTo(ed)) {
				UTIL_BeamEnts(m_pSpot->entindex(), 0, m_pPlayer->entindex(), 1, false, g_laserBeamIdx,
					0, 0, 10, beamWidth, 0, RGBA(255, 32, 32, 48), 64, MSG_ONE_UNRELIABLE, NULL, ed);
			}
		}
	}
#endif
}

CustomWeaponShootOpts& CWeaponCustom::GetShootOpts(int attackIdx) {
	if (attackIdx == 0 && IsPrimaryAltActive()) {
		return params.shootOpts[3];
	}
	
	return params.shootOpts[attackIdx];
}

int CWeaponCustom::AddDuplicate(CBasePlayerItem* pOriginal) {
	CBasePlayer* pPlayer = pOriginal ? pOriginal->GetPlayer() : NULL;
	if (!pPlayer)
		return 0;

	CWeaponCustom* wep = pOriginal ? pOriginal->MyWeaponCustomPtr() : NULL;

	if (wep && wep->IsAkimboWeapon() && !wep->CanAkimbo()) {
		wep->SetCanAkimbo(true);

		if (m_isDroppedWeapon) {
			// prevent ammo duping in dropped weapons
			wep->SetAkimboClip(m_iClip);
		}
		else {
			wep->SetAkimboClip(m_iDefaultAmmo);
		}
		
		EMIT_SOUND(ENT(pPlayer->pev), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();
		return 1;
	}

	return CBasePlayerWeapon::AddDuplicate(pOriginal);
}