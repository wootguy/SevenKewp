#include "CWeaponCustom.h"
#include "hlds_hooks.h"

#ifdef CLIENT_DLL
#include "../cl_dll/hud_iface.h"
#include "../game_shared/prediction_files.h"
#include "eng_wrappers.h"
#include "../common/pmtrace.h"
#include "../pm_shared/pm_defs.h"
extern int g_runfuncs;
extern int g_runningKickbackPred;
extern int g_last_attack_mode;
extern Vector g_vApplyVel;
void UpdateZoomCrosshair(int id, bool zoom);
void WC_EV_LocalSound(int sndIdx, int chan, int pitch, float vol, float attn, int panning, int flags, Vector* origin);
void WC_EV_EjectShell(WepEvt& evt, bool leftHand);
void WC_EV_PunchAngle(WepEvt& evt, int seed, float attackTime);
void WC_EV_WepAnim(WepEvt& evt, int wepid, int animIdx);
pmtrace_t WC_EV_FireBullets(float spreadX, float spreadY, bool showTracer, int tracerColor, bool gunshotDecal, bool textureSound, int iShot, int iDamage);
void EV_LaserOff();
void WC_EV_Dlight(WepEvt& evt, Vector pos);
uint32_t GetTimeAtCmd(uint32_t cmdId);
Vector WC_GetAim(float spreadX, float spreadY);
void EV_MuzzleFlash(void);
cl_entity_t* WC_GetPlayer();
void EV_EgonFlareCallback(struct tempent_s* ent, float frametime, float currenttime);
void EV_HLDM_GunshotDecalEffects(Vector pos, bool playSound);
void HUD_PlaySound(const char* sound, float volume);
#define PRINTF(msg, ...) gEngfuncs.Con_Printf(msg, ##__VA_ARGS__)
#define PRINTD(msg, ...) gEngfuncs.Con_DPrintf(msg, ##__VA_ARGS__)
#else
int g_runfuncs = 1;
#define PRINTF(fmt, ...)
#include "game.h"
#endif


void CWeaponCustom::Spawn() {
	Precache();

	m_iDefaultAmmo = params.ammoInfo[0].maxClip ? params.ammoInfo[0].maxClip : params.ammoInfo[0].defaultGive;

	ItemInfo info;
	info.iId = 0;
	if (GetItemInfo(&info))
		m_iId = info.iId;

	if (params.classname)
		pev->classname = params.classname;

	if (m_iId <= 0) {
		ALERT(at_error, "Custom weapon '%s' was not registered! Removing it from the world.\n",
			STRING(pev->classname));
		UTIL_Remove(this);
		return;
	}

	SetWeaponModelW();
	FallInit();// get ready to fall down.

	if (params.flags & FL_WC_WEP_USE_ONLY) {
		SetTouch(&CBaseEntity::ItemBounceTouch);
	}
}

void CWeaponCustom::Precache() {
	PrecacheEvents();

#ifndef CLIENT_DLL
	CBasePlayerWeapon::Precache();

	if (params.pmodelAkimbo) {
		PRECACHE_MODEL(STRING(params.pmodelAkimbo));
	}
	if (params.wmodelAkimbo) {
		bool hasMergeBody = mp_mergemodels.value && MergedModelBodyAkimbo() != -1;
		if (!hasMergeBody || UTIL_MapReplacesModel(STRING(params.wmodelAkimbo))) {
			PRECACHE_MODEL(STRING(params.wmodelAkimbo));
		}
	}

	if (params.wrongClientWeapon) {
		UTIL_PrecacheOther(STRING(params.wrongClientWeapon));
	}

	for (int i = 0; i < params.numEvents; i++) {
		WepEvt& evt = params.events[i];
		if (evt.evtType == WC_EVT_PROJECTILE && evt.proj.entity_class) {
			UTIL_PrecacheOther(STRING(evt.proj.entity_class));
		}
	}

	for (int i = 0; i < 2; i++) {
		if (params.ammoInfo[i].dropEnt)
			UTIL_PrecacheOther(STRING(params.ammoInfo[i].dropEnt));
	}
#endif
}

void CWeaponCustom::PrecacheEvents() {
	events.m_weapon = this;
	
#ifndef CLIENT_DLL
	m_configPath = g_customWeaponConfigs.get(STRING(pev->classname));

	if (m_configPath) {
		UTIL_ParseCustomWeaponConfig(m_configPath, params);

		if (strcmp(STRING(pev->classname), STRING(params.classname))) {
			if (params.flags & FL_WC_WEP_AKIMBO) {
				// enable akimbo mode and world model if an "akimbo" alias is used
				if (strstr(STRING(pev->classname), "akimbo")) {
					SetCanAkimbo(true);
					SetAkimbo(true);
				}
			}

			pev->classname = params.classname; // undo alias
		}
		
		params.maxClip = params.ammoInfo[0].maxClip;

		if (m_iId <= 0) {
			int* id = g_weaponClassIds.get(STRING(pev->classname));
			m_iId = id ? *id : 0;
		}

		m_defaultModelV = STRING(params.defaultModelV);
		m_defaultModelP = STRING(params.defaultModelP);
		m_defaultModelW = STRING(params.defaultModelW);

		int* mergedBody = g_merged_models.get(m_defaultModelW);
		m_mergedModelBody = mergedBody ? *mergedBody : -1;

		m_hasHandModels = params.flags & FL_WC_WEP_HAND_MODELS;
	}
	else {
		m_mergedModelBody = -1;

		if (m_defaultModelV) {
			params.vmodel = MODEL_INDEX(GetModelV());
			params.defaultModelV = ALLOC_STRING(m_defaultModelV);
		}
		if (m_defaultModelP)
			params.defaultModelP = ALLOC_STRING(m_defaultModelP);
		if (m_defaultModelW)
			params.defaultModelW = ALLOC_STRING(m_defaultModelW);
	}

	params.classname = pev->classname;

	if (params.hudFolder) {
		m_hudPath = ALLOC_STRING(UTIL_VarArgs("%s/%s", STRING(params.hudFolder), STRING(pev->classname)));
		
		// TODO: do this once
		PRECACHE_HUD_FILES(UTIL_VarArgs("sprites/%s.txt", STRING(m_hudPath)));
	}
	else {
		m_hudPath = ALLOC_STRING(UTIL_VarArgs("%s", STRING(pev->classname)));
	}

	/*
	// for converting weapon classes to configs
	if (!strcmp("weapon_as_jetpack", STRING(pev->classname))) {
		params.displayName = ALLOC_STRING(DisplayName());
		params.killFeedIcon = ALLOC_STRING(GetDeathNoticeWeapon());
		params.hudFolder = ALLOC_STRING("poke646");
		params.slot = 1;
		params.slotPosition = -1;
		params.weight = 5;
		params.flags |= FL_WC_WEP_ALLOW_HL;
		params.ammoInfo[0].type = ALLOC_STRING("9mm");
		params.ammoInfo[0].dropEnt = ALLOC_STRING("ammo_uziclip");
		params.ammoInfo[0].dropAmt = 32;
		//params.ammoInfo[1].type = ALLOC_STRING("uranium");
		//params.ammoInfo[1].dropEnt = ALLOC_STRING("ammo_gaussclip");
		//params.ammoInfo[1].dropAmt = 20;

		UTIL_TestConfig(this);


		const char* path = UTIL_VarArgs("dump/%s.txt", STRING(pev->classname));
		UTIL_DumpCustomWeaponConfig(path, params, true);
	}
	*/
#endif
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
	if (!pPlayer->UseSevenKewpGuns() && params.wrongClientWeapon) {
		if (pPlayer->HasNamedPlayerItem(STRING(params.wrongClientWeapon))) {
			return 0;
		}

		pPlayer->GiveNamedItem(STRING(params.wrongClientWeapon));
		m_pickupPlayers |= PLRBIT(pPlayer->edict());
		return 0;
	}

	if (params.flags & FL_WC_WEP_AKIMBO)
		SetAkimboClip(m_iDefaultAmmo);

	if (pPlayer->IsSevenKewpClient())
		UTIL_SendCustomWeaponPredictionData(pPlayer->edict(), this, WC_PRED_SEND_INIT);
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
	const char* validAnimExt = STRING(params.animExt);

	if (IsAkimbo())
		validAnimExt = STRING(params.animExtAkimbo);
	else if (m_pPlayer->m_iFOV != 0 && params.animExtZoom)
		validAnimExt = STRING(params.animExtZoom);

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

	m_chargeStartCmdTime = 0;
	m_chargeStopCmdTime = 0;
	m_lastCharge = 0;
	m_lastBeamUpdate = 0;
	m_fInReload = false;
	m_bInAkimboReload = false;
	m_fInSpecialReload = 0;
	m_chargeSoundEvt = 0;
	events.animCount = 0;
	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	ClearChargedStates();
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
	g_last_attack_mode = 1;

#else
	const char* animSet = GetAnimSet();

	const char* vmodel = GetModelV();

	studiohdr_t* mdl = GET_MODEL_PTR(PRECACHE_MODEL(vmodel));
	m_hasLaserAttachment = mdl && mdl->numattachments > params.laser.attachment;

	ret = DefaultDeploy(vmodel, GetModelP(), deployAnim, animSet, 1);
	
	if (!IsPredicted())
		SendWeaponAnimSpec(deployAnim);
#endif

	if (IsAkimbo())
		SendAkimboAnim(deployAnim);

	int deployTime = IsAkimbo() ? params.akimbo.akimboDeployTime : params.deployTime;
	int deployAnimTime = IsAkimbo() ? params.akimbo.akimboDeployAnimTime : params.deployAnimTime;
	if (!deployAnimTime)
		deployAnimTime = deployTime ? deployTime : 1000; // default
	if (!deployTime)
		deployTime = 500; // default

	float nextAttack = UTIL_WeaponTimeBase() + deployTime * 0.001f;
	float nextIdle = UTIL_WeaponTimeBase() + deployAnimTime * 0.001f;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = nextAttack;
	m_flTimeWeaponIdle = nextIdle;
	
	if (IsLaserOn()) {
		m_laserOnTime = WallTime() + m_flTimeWeaponIdle;
	}

	m_pPlayer->m_flNextAttack = 0; // allow thinking during deployment

	if (WallTime() - m_lastDeploy > MAX_PREDICTION_WAIT) {
		int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
		events.ProcessEvents(WC_TRIG_DEPLOY, akimboArg);
	}

	m_lastDeploy = WallTime();

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
	events.KillBeams();
	m_lastDeploy = WallTime();
}

void CWeaponCustom::Reload() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_fInReload)
		return;
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 && m_fInSpecialReload != 2)
		return;

	bool canAkimboReload = IsAkimbo() && GetAkimboClip() < params.ammoInfo[0].maxClip;
	bool shotgunReload = params.flags & FL_WC_WEP_SHOTGUN_RELOAD;

	if (AreAnyAttacksCharging())
		return;
	if (m_iClip == -1)
		return;
	if (m_iClip >= params.ammoInfo[0].maxClip && !canAkimboReload && m_fInSpecialReload == 0) {
		m_bWantAkimboReload = false;
		return;
	}
	if (m_flNextPrimaryAttack > 0)
		return;

	WeaponCustomReload* reloadStage = &params.reloadStage[0];

	if (IsAkimbo()) {
		reloadStage = &params.akimbo.reload;
	}
	else if (shotgunReload) {
		if (m_fInSpecialReload == 2) {
			reloadStage = &params.reloadStage[2];
		}
		else if (m_fInSpecialReload == 1) {
			reloadStage = &params.reloadStage[1];
		}
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

	// allow shooting while doing the final pump after reloading, like with the HL shotty
	if (m_fInSpecialReload == 2)
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = 0;

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
		
		if (shotgunReload) {
			if (m_fInSpecialReload == 1)
				events.ProcessEvents(WC_TRIG_RELOAD, akimboArg);
			else if (m_fInSpecialReload == 2) {
				events.ProcessEvents(WC_TRIG_RELOAD_FINISH, akimboArg);
			}
		}
		else {
			events.ProcessEvents(WC_TRIG_RELOAD, akimboArg);
		}
		
		if (m_iClip == 0) {
			events.ProcessEvents(WC_TRIG_RELOAD_EMPTY, akimboArg);
		}
		else {
			events.ProcessEvents(WC_TRIG_RELOAD_NOT_EMPTY, akimboArg);
		}

		if (IsLaserOn()) {
			HideLaser(true);
			m_laserOnTime = WallTime() + totalReloadTime * 0.001f;
		}

		events.m_bulletFireCount = 0;
	}

	m_pPlayer->m_flNextAttack = 0; // keep calling post frame for complex reloads
}

void CWeaponCustom::WeaponIdle() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_lastCanAkimbo != (bool)CanAkimbo()) {
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

	m_primaryCalled = false;
	m_secondaryCalled = false;
	m_primaryFired = false;
	m_secondaryFired = false;

	m_lastCanAkimbo = CanAkimbo();

	if (m_fInReload)
		return;

	if (AreAnyAttacksCharging()) {
		return;
	}

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
	int idleCount = 4;

	if (IsAkimbo())
		idles = params.akimbo.idles;
	else if (IsLaserOn())
		idles = params.laser.idles;

	if (params.flags & FL_WC_WEP_EMPTY_IDLES) {
		idleCount = 2;

		if (m_iClip == 0) {
			idles = idles + 2;
		}
	}
	else {
		if (m_iClip == 0 && params.ammoInfo[0].maxClip) {
			return; // assume weapon should stay on the "fire last" animation
		}
	}

	int idleSum = 0;
	for (int i = 0; i < idleCount; i++) {
		idleSum += idles[i].weight;
	}

	int idleRnd = (UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0) + 0.005f) * idleSum;

	for (int i = 0; i < idleCount; i++) {
		WeaponCustomIdle& idle = idles[i];
		idleRnd -= idle.weight;

		if (idleRnd <= 0) {
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + idle.time * 0.001f;
			SendWeaponAnim(idle.anim, 1, pev->body);
			SendWeaponAnimSpec(idle.anim);
			break;
		}
	}

	WeaponIdleCustom();
}

void CWeaponCustom::ItemPostFrame() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_nextMeleeDecal && m_nextMeleeDecal < gpGlobals->time) {
		m_nextMeleeDecal = 0;
		DecalGunshot(&m_meleeDecalPos, BULLET_PLAYER_CROWBAR);
	}
	events.UpdateBeams();

	bool reloadFinished = m_fInReload && m_flNextPrimaryAttack <= 0;

	// reload prediction
	if (reloadFinished) {

		// special shotgun reloading
		if (params.flags & FL_WC_WEP_SHOTGUN_RELOAD) {
			bool wantAbort = (m_pPlayer->pev->button & IN_ATTACK) | (m_pPlayer->pev->button & IN_ATTACK2);

			if (wantAbort && m_iClip > 0) {
				m_fInSpecialReload = 0;
				m_fInReload = FALSE;
				return;
			}

			if (m_fInSpecialReload == 0) {
				// finished raising gun, now start loading shells
				m_fInSpecialReload = 1;
				m_fInReload = FALSE;
				Reload(); // start the next animation
				return;
			}
			else if (m_fInSpecialReload == 1) {
				// loading shells
				if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 && m_iClip < params.ammoInfo[0].maxClip) {
					m_iClip++;
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;
					m_pPlayer->TabulateAmmo();
				}

				if (m_iClip >= params.ammoInfo[0].maxClip || m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0) {
					// play the finishing animation
					m_fInSpecialReload = 2;
				}
				
				m_fInReload = FALSE;
				Reload();
				return;
			}
		}

		// complete a simple reload.
		int& clip = m_bInAkimboReload ? m_chargeReady : m_iClip;
		int j = V_min(params.ammoInfo[0].maxClip - clip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
		clip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
		m_pPlayer->TabulateAmmo();
		
		m_fInReload = FALSE;
		m_fInSpecialReload = 0;


		if (IsAkimbo()) {
			if (m_bInAkimboReload) {
				SendWeaponAnim(params.akimbo.deployAnim, 1, pev->body);
				SendWeaponAnimSpec(params.akimbo.deployAnim);
			}
			else {
				SendAkimboAnim(params.akimbo.deployAnim);
			}

			bool attackInterrupt = m_pPlayer->pev->button & (IN_ATTACK | IN_ATTACK2);
			bool canReloadOtherGun = m_iClip < params.ammoInfo[0].maxClip || GetAkimboClip() < params.ammoInfo[0].maxClip;

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

	events.PlayDelayedEvents();
	UpdateLaser();
}

const char* CWeaponCustom::GetModelP() {
	return params.pmodelAkimbo && IsAkimbo() ? STRING(params.pmodelAkimbo) : CBasePlayerWeapon::GetModelP();
}

const char* CWeaponCustom::GetModelW() {
#ifndef CLIENT_DLL
	if (params.wmodelAkimbo && CanAkimbo()) {
		bool hasMergeBody = mp_mergemodels.value && MergedModelBodyAkimbo() != -1;
		return hasMergeBody && !UTIL_MapReplacesModel(STRING(params.wmodelAkimbo)) ? MERGED_ITEMS_MODEL : STRING(params.wmodelAkimbo);
	}
#endif

	return CBasePlayerWeapon::GetModelW();
}

int* CWeaponCustom::GetAttackClip(int attackIdx) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return &m_iClip;

	CustomWeaponShootOpts& opts = params.shootOpts[attackIdx];

	static int nullclip;
	int* clip = &nullclip;

	if (attackIdx == 0) {
		clip = IsAkimbo() ? &m_chargeReady : &m_iClip;

		if (m_iClip == -1 && m_iPrimaryAmmoType != -1)
			clip = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
	}
	if (attackIdx == 1) {
		if (IsAkimbo()) {
			clip = &m_iClip;

			if (m_iClip == -1 && m_iPrimaryAmmoType != -1)
				clip = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
		}
		else {
			clip = m_iSecondaryAmmoType >= 0 ? &m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] : &nullclip;
		}
	}

	switch (opts.ammoPool) {
	case WC_AMMOPOOL_PRIMARY_CLIP: clip = &m_iClip; break;
	case WC_AMMOPOOL_PRIMARY_RESERVE: clip = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]; break;
	case WC_AMMOPOOL_SECONDARY_RESERVE: clip = &m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]; break;
	default: break;
	}

	return clip;
}

void CWeaponCustom::PrimaryAttack() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	bool isAttackStart = !m_primaryCalled;

	m_primaryCalled = true;
	m_primaryFired = false;
	CustomWeaponShootOpts& opts = params.shootOpts[0];

	if ((opts.flags & FL_WC_SHOOT_NEED_AKIMBO) && !CanAkimbo())
		return;

	if (params.flags & FL_WC_WEP_HAS_PRIMARY) {
		int* clip = GetAttackClip(0);
		int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;

		if (CommonAttack(0, clip, IsAkimbo(), false)) {
			m_primaryFired = true;
			int trig = IsPrimaryAltActive() ? WC_TRIG_PRIMARY_ALT : WC_TRIG_PRIMARY;

			if (isAttackStart) {
				events.ProcessEvents(WC_TRIG_PRIMARY_START, 0);
				m_attackStartCmdTime = CmdTime();
			}
			events.ProcessEvents(trig, akimboArg, IsAkimbo(), false, *clip);
			events.FireAmmoEvents(opts.ammoPool ? opts.ammoPool : (int)WC_AMMOPOOL_PRIMARY_CLIP);

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

	if (!(params.flags & (FL_WC_WEP_HAS_SECONDARY | FL_WC_WEP_AKIMBO))) {
		if (m_pPlayer->pev->button & IN_ATTACK) {
			PrimaryAttack();
		}
		else {
			WeaponIdle();
		}
		return;
	}

	bool isAttackStart = !m_secondaryCalled;

	m_secondaryCalled = true;
	m_secondaryFired = false;

	CustomWeaponShootOpts& opts = params.shootOpts[1];

	if ((opts.flags & FL_WC_SHOOT_NEED_AKIMBO) && !CanAkimbo())
		return;

	if (IsAkimbo()) {
		bool fireBoth = (m_pPlayer->pev->button & IN_ATTACK) && GetAkimboClip() >= params.shootOpts[0].ammoCost;
		int* clip = GetAttackClip(0);
		int primaryTrig = IsPrimaryAltActive() ? WC_TRIG_PRIMARY_ALT : WC_TRIG_PRIMARY;

		if (CommonAttack(0, &m_iClip, false, fireBoth)) {
			m_secondaryFired = true;

			if (isAttackStart) {
				events.ProcessEvents(WC_TRIG_SECONDARY_START, WC_TRIG_SHOOT_ARG_AKIMBO, false, fireBoth, *clip);
				m_attackStartCmdTime = CmdTime();
			}
			events.ProcessEvents(primaryTrig, WC_TRIG_SHOOT_ARG_AKIMBO, false, fireBoth, *clip);
			events.FireAmmoEvents(opts.ammoPool ? opts.ammoPool : (int)WC_AMMOPOOL_PRIMARY_CLIP);

			if (*clip < 0)
				*clip = 0;
		}
		
		m_pPlayer->random_seed++; // so bullets don't hit the same spot (seed only updates once per cmd)
		if (fireBoth && CommonAttack(0, &m_chargeReady, true, fireBoth)) {
			m_secondaryFired = true;

			if (isAttackStart)
				events.ProcessEvents(WC_TRIG_SECONDARY_START, WC_TRIG_SHOOT_ARG_AKIMBO, true, fireBoth, *clip);
			events.ProcessEvents(primaryTrig, WC_TRIG_SHOOT_ARG_AKIMBO, true, fireBoth, *clip);

			if (m_chargeReady < 0)
				m_chargeReady = 0;
		}
	}
	else if (params.flags & FL_WC_WEP_HAS_SECONDARY) {
		int* clip = GetAttackClip(1);
		int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;

		if (CommonAttack(1, clip, false, false)) {
			m_secondaryFired = true;

			if (isAttackStart)
				events.ProcessEvents(WC_TRIG_SECONDARY_START, 0);
			events.ProcessEvents(WC_TRIG_SECONDARY, akimboArg, *clip);
			events.FireAmmoEvents(opts.ammoPool ? opts.ammoPool : (int)WC_AMMOPOOL_SECONDARY_RESERVE);
		}
	}

	KickbackPrediction();
}

void CWeaponCustom::TertiaryAttack() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponShootOpts& opts = params.shootOpts[2];

	if ((opts.flags & FL_WC_SHOOT_NEED_AKIMBO) && !CanAkimbo())
		return;

	if (params.flags & FL_WC_WEP_HAS_TERTIARY) {
		int* clip = GetAttackClip(2);

		if (CommonAttack(2, clip, false, false)) {
			int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;

			events.ProcessEvents(WC_TRIG_TERTIARY, akimboArg, false, false, *clip);
			events.FireAmmoEvents(opts.ammoPool);

			if (*clip < 0)
				*clip = 0;
		}
	}
}

bool CWeaponCustom::CommonAttack(int attackIdx, int* clip, bool leftHand, bool akimboFire) {
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	if (opts.flags & FL_WC_SHOOT_NO_AUTOFIRE) {
		if (attackIdx == 0 && !(m_pPlayer->m_afButtonPressed & IN_ATTACK))
			return false;
		if (attackIdx == 1 && !(m_pPlayer->m_afButtonPressed & IN_ATTACK2))
			return false;
	}

	bool isNormalAttack = !(opts.flags & FL_WC_SHOOT_NO_ATTACK);
	bool isMelee = opts.flags & FL_WC_SHOOT_IS_MELEE;

	int clipLeft = *clip;

	m_bWantAkimboReload = false;

	if (isNormalAttack && m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD && !(opts.flags & FL_WC_SHOOT_UNDERWATER)) {
		FailAttack(attackIdx, leftHand, akimboFire, true);
		return false;
	}

	bool needFullCost = opts.flags & FL_WC_SHOOT_NEED_FULL_COST;
	bool ammoSpendsDuringCharge = opts.chargeTime > 0 && opts.chargeAmmoMode == WC_CHARGE_AMMO_LOAD;

	if (clipLeft < opts.ammoCost && needFullCost && !ammoSpendsDuringCharge) {
		FailAttack(attackIdx, leftHand, akimboFire, true);

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 && m_iClip != -1) {
			m_flNextPrimaryAttack = 0; // force the reload
			Reload();
		}
		return false;
	}

	bool forceFireChargedShot = GetChargedState(attackIdx) == WC_CHARGE_STATE_DISCHARGING
		&& opts.chargeMode != WC_CHARGEUP_CONSTANT;
	
	if (!forceFireChargedShot && clipLeft <= 0 && opts.ammoCost > 0) {
		if (!m_fInReload) {
			FailAttack(attackIdx, leftHand, akimboFire, true);
		}
		return false;
	}

	if (!Chargeup(attackIdx, clip, leftHand, akimboFire))
		return false;

	if (isNormalAttack && !forceFireChargedShot) {
		if (clipLeft <= 0 && opts.ammoCost > 0) {
			if (!m_fInReload) {
				FailAttack(attackIdx, leftHand, akimboFire, true);
			}
			return false;
		}
	}

	bool ammoSpentInChargeup = opts.chargeTime && opts.chargeAmmoMode == WC_CHARGE_AMMO_LOAD;

	if (!ammoSpentInChargeup) {
		if (ammoFreqs[attackIdx]++ >= opts.ammoFreq) {
			ammoFreqs[attackIdx] = 0;

			// must be here for prediction. Cannot be modified in an event or filtered by g_runfuncs=1.
			*clip -= opts.ammoCost;
		}
	}

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

	if (isMelee) {
		MeleeAttack(attackIdx);
	}

	switch (attackIdx) {
	case 0: PrimaryAttackCustom(); break;
	case 1: SecondaryAttackCustom(); break;
	case 2: TertiaryAttackCustom(); break;
	default: break;
	}

	if (opts.chargeMode == WC_CHARGEUP_SINGLE || opts.chargeMode == WC_CHARGEUP_HOLD
		|| opts.chargeMode == WC_CHARGEUP_SINGLE_HOLD) {
		// don't stay charged up in once mode
		Chargedown(attackIdx);
	}

	return true;
}

void CWeaponCustom::Cooldown(int attackIdx, int overrideMillis) {
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	int millis = overrideMillis != -1 ? overrideMillis : opts.cooldown;

	if (opts.dischargedCooldown) {
		uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
		float t = V_min(chargeMillis / (float)opts.chargeTime, 1.0f);

		int startCooldown = opts.dischargedCooldown;
		int endCooldown = opts.cooldown;
		millis = startCooldown + (endCooldown - startCooldown) * t;
	}

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

float CWeaponCustom::GetChargeProgress(float chargeTime) {
	uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
	return V_min(chargeMillis / chargeTime, 1.0f);
}

void CWeaponCustom::PlayChargeSound(float t) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_chargeSoundEvt) {
		WepEvt& evt = params.events[clamp(m_chargeSoundEvt, 0, MAX_WC_EVENTS - 1)];
		int channel = evt.playSound.channel;
		int soundIdx = evt.playSound.sound;

		int pitchRange = evt.playSound.pitchMax - evt.playSound.pitchMin;

		int pitch = evt.playSound.pitchMin + pitchRange * t;

#ifdef CLIENT_DLL
		if (g_runfuncs)
			WC_EV_LocalSound(soundIdx, channel, pitch, 1, ATTN_NORM, 0, SND_CHANGE_PITCH, NULL);
#else
		if (IsPredicted()) {
			uint32_t messageTargets = 0xffffffff & ~PLRBIT(m_pPlayer->edict());
			StartSound(m_pPlayer->edict(), channel, INDEX_SOUND(soundIdx), 1, ATTN_NORM,
				SND_FL_PREDICTED | SND_CHANGE_PITCH, pitch, m_pPlayer->pev->origin, messageTargets);
		}
		else {
			StartSound(m_pPlayer->edict(), channel, INDEX_SOUND(soundIdx), 1, ATTN_NORM,
				SND_CHANGE_PITCH, pitch, m_pPlayer->pev->origin, 0xffffffff);
		}
#endif
	}
}

bool CWeaponCustom::Chargeup(int attackIdx, int* clip, bool leftHand, bool akimboFire) {
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);
	
	if (GetChargedState(attackIdx) == WC_CHARGE_STATE_DISCHARGING && opts.chargeMode != WC_CHARGEUP_CONSTANT)
		return true;

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	if (!opts.chargeTime)
		return true;

	if (params.flags & FL_WC_WEP_LINK_CHARGEUPS) {
		attackIdx = 0;
	}

	if (GetChargedState(attackIdx) == WC_CHARGE_STATE_NONE || GetChargedState(attackIdx) == WC_CHARGE_STATE_DISCHARGING) {
		// resume charging if not finished charging down
		int skipChargeUpTime = 0;
		if (opts.chargeDownTime) {
			int chargeDownTime = CmdTime() - m_chargeStopCmdTime;
			skipChargeUpTime = V_max(0, opts.chargeDownTime - chargeDownTime);
			skipChargeUpTime *= opts.chargeTime / (float)opts.chargeDownTime;
		}

		m_chargeStartCmdTime = CmdTime() - skipChargeUpTime;
		m_chargeStartClip = *clip;
		m_lastCharge = GetChargeProgress(opts.chargeTime) - 0.001f;
		SetChargedState(attackIdx, WC_CHARGE_STATE_CHARGING);
		int e = (attackIdx == 0) ? WC_TRIG_PRIMARY_CHARGE : WC_TRIG_SECONDARY_CHARGE;
		events.ProcessEvents(e, 0, leftHand, akimboFire);
		m_pPlayer->ApplyEffects();
	}

	uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
	float t = GetChargeProgress(opts.chargeTime);

	for (int i = 0; i <= 10; i++) {
		float p = i * 0.1f;
		if (t >= p && m_lastCharge < p) {
			int trig = attackIdx == 0 ? WC_TRIG_PRIMARY_CHARGE : WC_TRIG_SECONDARY_CHARGE;
			events.ProcessEvents(trig, i+1, leftHand, false);
		}
	}

	m_lastCharge = t;

	if (opts.ammoCost && opts.chargeAmmoMode == WC_CHARGE_AMMO_LOAD) {
		int cost = V_max(1, (t * opts.ammoCost) + 0.5f);
		*clip = V_max(0, m_chargeStartClip - cost);

		if (*clip == 0) {
			SetChargedState(attackIdx, WC_CHARGE_STATE_DISCHARGING);
			return true;
		}
	}

	PlayChargeSound(t);

	if (opts.overchargeTime && chargeMillis >= opts.overchargeTime) {
		if (opts.overchargeMode == WC_OVERCHARGE_CANCEL) {
			FailAttack(attackIdx, leftHand, akimboFire, false);
		}
		
		int chargeState = GetChargedState(attackIdx);
		if (chargeState != WC_CHARGE_STATE_OVERCHARGED) {
			int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
			int ievt = attackIdx == 0 ? WC_TRIG_PRIMARY_OVERCHARGE : WC_TRIG_SECONDARY_OVERCHARGE;
			events.ProcessEvents(ievt, akimboArg, leftHand, false);
			SetChargedState(attackIdx, WC_CHARGE_STATE_OVERCHARGED);
		}

		if (opts.overchargeMode == WC_OVERCHARGE_CANCEL) {
			SetChargedState(attackIdx, WC_CHARGE_STATE_NONE);
			return false;
		}
	}

	if (opts.chargeMode == WC_CHARGEUP_HOLD) {
		return false; // wait for attack button to be released. Attack will start during idle.
	}

	bool minigunModeChargedEnough = opts.minChargeShootTime
		&& chargeMillis >= opts.minChargeShootTime;

	return chargeMillis >= opts.chargeTime || minigunModeChargedEnough;
}

void CWeaponCustom::Chargedown(int attackIdx) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int ievt = attackIdx == 0 ? WC_TRIG_PRIMARY_STOP : WC_TRIG_SECONDARY_STOP;

	//PRINTF("Charging down... %d\n", g_runfuncs);

	events.CancelDelayedEvents(attackIdx == 0 ? WC_TRIG_PRIMARY_CHARGE : WC_TRIG_SECONDARY_CHARGE);
	events.ProcessEvents(ievt, 0, false, false);

	// prevent idling immediately after chargedown in case an animation needs to play
	// otherwise the prediction code will idle before the server syncs the new idle delay
	m_flTimeWeaponIdle = V_max(m_flTimeWeaponIdle, 0.5f);

	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);
	if (opts.chargeDownTime) {
		int chargeMillis = CmdTime() - m_chargeStartCmdTime;
		int skipChargeDownTime = V_max(0, opts.chargeTime - chargeMillis);
		if (opts.chargeTime) {
			skipChargeDownTime *= opts.chargeDownTime / (float)opts.chargeTime;
		}
		m_chargeStopCmdTime = CmdTime() - skipChargeDownTime;
		int chargeDownTime = CmdTime() - m_chargeStopCmdTime;
		m_lastCharge = (1.0f - V_min(chargeDownTime / (float)opts.chargeDownTime, 1.0f)) + 0.001f;

		SetChargedState(attackIdx, WC_CHARGE_STATE_DISCHARGING);
	}
	else {
		SetChargedState(attackIdx, WC_CHARGE_STATE_NONE);
	}

	m_pPlayer->ApplyEffects();
}

void CWeaponCustom::FinishAttack(int attackIdx) {
	if (params.flags & FL_WC_WEP_LINK_CHARGEUPS) {
		attackIdx = 0;
	}

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	//bool attackCalled = attackIdx == 0 ? m_primaryCalled : m_secondaryCalled;
	bool attackFired = attackIdx == 0 ? m_primaryFired : m_secondaryFired;

	if (attackFired && g_runfuncs) {
		events.KillBeams(attackIdx);
	}

	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	if (attackIdx == 0) {
		if (opts.chargeDownTime && GetChargedState(attackIdx) == WC_CHARGE_STATE_DISCHARGING) {
			int chargeDownTime = CmdTime() - m_chargeStopCmdTime;
			float t = 1.0f - V_min(chargeDownTime / (float)opts.chargeDownTime, 1.0f);

			// spin down the charge
			if (chargeDownTime < opts.chargeDownTime) {
				PlayChargeSound(t);
			}
			else {
				SetChargedState(attackIdx, WC_CHARGE_STATE_NONE);
			}

			for (int i = 0; i <= 10; i++) {
				float p = i * 0.1f;
				if (t <= p && m_lastCharge > p) {
					int trig = attackIdx == 0 ? WC_TRIG_PRIMARY_CHARGE : WC_TRIG_SECONDARY_CHARGE;
					events.ProcessEvents(trig, i + 12, false, false);
				}
			}

			m_lastCharge = t;
		}
	}

	// not a charging attack. Call the stop event.
	if (attackFired && !opts.chargeTime) {
		int ievt = attackIdx == 0 ? WC_TRIG_PRIMARY_STOP : WC_TRIG_SECONDARY_STOP;
		events.CancelDelayedEvents(attackIdx == 0 ? WC_TRIG_PRIMARY_START : WC_TRIG_SECONDARY_START);
		events.ProcessEvents(ievt, 0, false, false);
		return;
	}
	
	if (GetChargedState(attackIdx) == WC_CHARGE_STATE_NONE
		|| GetChargedState(attackIdx) == WC_CHARGE_STATE_DISCHARGING) {
		return;
	}

	uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
	uint32_t cancelMillis = params.shootOpts[attackIdx].chargeCancelTime;
	bool isChargedEnough = chargeMillis >= cancelMillis;

	if (cancelMillis) {
		float t = V_min(chargeMillis / (float)opts.chargeTime, 1.0f);

		if (opts.ammoCost && opts.chargeAmmoMode == WC_CHARGE_AMMO_LOAD) {
			int cost = V_max(1, (t * opts.ammoCost) + 0.5f);
			int* clip = GetAttackClip(attackIdx);
			*clip = m_chargeStartClip - cost;
		}
	}

	if (!cancelMillis || isChargedEnough) {
		if (opts.chargeMode == WC_CHARGEUP_SINGLE || opts.chargeMode == WC_CHARGEUP_SINGLE_HOLD
			|| opts.chargeMode == WC_CHARGEUP_HOLD)
		{
			SetChargedState(attackIdx, WC_CHARGE_STATE_DISCHARGING);
			// fire a single shot every chargeup
			if (attackIdx == 0) {
				PrimaryAttack();
			}
			else {
				SecondaryAttack();
			}
		}

		Chargedown(attackIdx);
	}
}

void CWeaponCustom::PlayEmptySound(int attackIdx)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	if (m_iPlayEmptySound)
	{
#ifdef CLIENT_DLL
		if (g_runfuncs) {
			if (opts.emptySound) {
				WC_EV_LocalSound(opts.emptySound, CHAN_STATIC, 100, 1.0f, ATTN_NORM, 0, 0, NULL);
			}
			else {
				HUD_PlaySound(RemapFile("weapons/357_cock1.wav"), 0.8);
			}
		}
#else
		const char* emptySound = opts.emptySound ? INDEX_SOUND(opts.emptySound) : "weapons/357_cock1.wav";
		// send sound to all players except the shooter, who is predicting the sound locally
		edict_t* plr = m_pPlayer->edict();
		uint32_t messageTargets = 0xffffffff & ~PLRBIT(plr);
		StartSound(plr, CHAN_WEAPON, emptySound, 0.8f,
			ATTN_NORM, SND_FL_PREDICTED, 100, m_pPlayer->pev->origin, messageTargets);
#endif
		m_iPlayEmptySound = 0;
	}
}

void CWeaponCustom::FailAttack(int attackIdx, bool leftHand, bool akimboFire, bool ammoClick) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	if (ammoClick) {
		Cooldown(-1, 150);
		if (!(opts.flags & FL_WC_SHOOT_IS_MELEE))
			PlayEmptySound(attackIdx);
	}

	int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
	int ievt = attackIdx == 0 ? WC_TRIG_PRIMARY_FAIL : WC_TRIG_SECONDARY_FAIL;
	events.ProcessEvents(ievt, akimboArg, leftHand, false);
	
	events.CancelDelayedEvents(attackIdx == 0 ? WC_TRIG_PRIMARY_START : WC_TRIG_SECONDARY_START);
	events.CancelDelayedEvents(attackIdx == 0 ? WC_TRIG_PRIMARY_CHARGE : WC_TRIG_SECONDARY_CHARGE);

	if (events.KillBeams(attackIdx)) {
		int ievt2 = attackIdx == 0 ? WC_TRIG_PRIMARY_STOP : WC_TRIG_SECONDARY_STOP;
		events.ProcessEvents(ievt2, akimboArg, leftHand, false);
	}

	if (m_chargeSoundEvt) {
		WepEvt& evt = params.events[clamp(m_chargeSoundEvt, 0, MAX_WC_EVENTS - 1)];
		int channel = evt.playSound.channel;
		int soundIdx = evt.playSound.sound;

#ifdef CLIENT_DLL
		WC_EV_LocalSound(soundIdx, channel, 100, 1, ATTN_NORM, 0, SND_STOP, NULL);
#else
		if (IsPredicted()) {
			uint32_t messageTargets = 0xffffffff & ~PLRBIT(m_pPlayer->edict());
			StartSound(m_pPlayer->edict(), channel, INDEX_SOUND(soundIdx), 1, ATTN_NORM,
				SND_STOP, 100, m_pPlayer->pev->origin, messageTargets);
		}
		else {
			StartSound(m_pPlayer->edict(), channel, INDEX_SOUND(soundIdx), 1, ATTN_NORM,
				SND_STOP, 100, m_pPlayer->pev->origin, 0xffffffff);
		}
#endif
	}

	if (opts.cooldownFail)
		Cooldown(attackIdx, opts.cooldownFail);

	if (opts.chargeTime) {
		m_flTimeWeaponIdle = 0.2f;
	}
}

void CWeaponCustom::PlayRandomSound(CBasePlayer* plr, uint16_t sounds[4]) {
#ifndef CLIENT_DLL
	int soundCount = 4;

	for (int i = 0; i < 4; i++) {
		if (sounds[i] == 0) {
			soundCount = i;
			break;
		}
	}

	if (soundCount == 0)
		return;

	const char* snd = INDEX_SOUND( sounds[RANDOM_LONG(0, soundCount - 1)] );
	StartSound(plr->edict(), CHAN_WEAPON, snd, 1, ATTN_NORM, 0, 100, plr->pev->origin, 0xffffffff);
#endif
}

void CWeaponCustom::MeleeAttack(int attackIdx) {
#ifndef CLIENT_DLL
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	Vector offset = opts.melee.attackOffset.x * gpGlobals->v_forward
		+ opts.melee.attackOffset.y * gpGlobals->v_up
		+ opts.melee.attackOffset.z * gpGlobals->v_right;

	Vector vecSrc = m_pPlayer->GetGunPosition() + offset;
	Vector vecEnd = vecSrc + gpGlobals->v_forward * opts.melee.range;

	SolidifyNearbyCorpses(false);
	lagcomp_begin(m_pPlayer);

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

	if (tr.flFraction >= 1.0) {
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction < 1.0) {
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}

	m_pPlayer->WaterSplashTrace(vecSrc, 32, head_hull, 0.4f);

	lagcomp_end();
	SolidifyNearbyCorpses(true);

	if (tr.flFraction >= 1.0) {
		MeleeMiss(m_pPlayer);
		PlayRandomSound(m_pPlayer, opts.melee.missSounds);

		if (opts.melee.missCooldown) {
			Cooldown(attackIdx, opts.melee.missCooldown);
		}
	}
	else {
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		if (MeleeHit(m_pPlayer, pEntity)) {
			return;
		}

		ClearMultiDamage();
		pEntity->TraceAttack(m_pPlayer->pev, GetDamage(opts.melee.damage), gpGlobals->v_forward, &tr, opts.melee.damageBits);
		ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

		// play thwack, smack, or dong sound
		float flVol = 1.0;

		events.ProcessEvents(WC_TRIG_IMPACT, events.GetImpactArg(attackIdx, true, true));
		AttackTrace(m_pPlayer, attackIdx, vecSrc, tr, true);

		if (MeleeIsFlesh(pEntity)) {
			MeleeHitFlesh(m_pPlayer, pEntity);
			PlayRandomSound(m_pPlayer, opts.melee.hitFleshSounds);

			m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
			flVol = 0.1;
		}
		else {
			MeleeHitWall(m_pPlayer, pEntity);
			PlayRandomSound(m_pPlayer, opts.melee.hitWallSounds);
			TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);
			m_meleeDecalPos = tr; // delay the decal a bit
			m_nextMeleeDecal = gpGlobals->time + opts.melee.decalDelay * 0.001f;
		}

		if (opts.melee.hitCooldown) {
			Cooldown(attackIdx, opts.melee.hitCooldown);
		}

		m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
	}

#endif
}

bool CWeaponCustom::MeleeIsFlesh(CBaseEntity* pEntity) {
	return pEntity && !pEntity->IsMachine() && !pEntity->IsBSPModel() &&
		(pEntity->Classify() != CLASS_NONE || pEntity->IsPlayerCorpse());
}

void CWeaponCustom::KickbackPrediction() {
	if (AreAnyAttacksCharging())
		return; // prevent extra kickback from the prediction system re-running commands

	// If prediction is broken and you see prediction sending you crazy far and snapping back
	// then it means cooldowns aren't working and kickback is stacking on replayed commands.
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

void CWeaponCustom::ToggleZoom(int zoomFov, int zoomFov2) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int newFov = 0;

	if (m_pPlayer->m_iFOV) {
		if (m_pPlayer->m_iFOV == zoomFov && zoomFov2) {
			newFov = zoomFov2;
		}
	}
	else {
		newFov = zoomFov;
	}

	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = newFov;

#ifdef CLIENT_DLL
	UpdateZoomCrosshair(m_iId, m_pPlayer->m_iFOV != 0);
#else
	UpdateAnimSet();
#endif

	m_lastZoomToggle = gpGlobals->time;
}

void CWeaponCustom::ToggleLaser(bool enable) {
	SetLaser(enable);
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
		ToggleZoom(0, 0);
	}
}

bool CWeaponCustom::IsPredicted() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	return !(params.flags & FL_WC_WEP_NO_PREDICTION) && m_pPlayer->IsSevenKewpClient();
}

int CWeaponCustom::GetAttackIdx(WepEvt& evt) {
	switch (evt.trigger) {
	default:
	case WC_TRIG_PRIMARY:
	case WC_TRIG_PRIMARY_CHARGE:
	case WC_TRIG_PRIMARY_STOP:
	case WC_TRIG_PRIMARY_FAIL:
		return 0;
	case WC_TRIG_SECONDARY:
	case WC_TRIG_SECONDARY_CHARGE:
	case WC_TRIG_SECONDARY_STOP:
	case WC_TRIG_SECONDARY_FAIL:
		return 1;
		break;
	case WC_TRIG_TERTIARY:
		return 2;
	case WC_TRIG_PRIMARY_ALT:
		return 3;
	}
}

float CWeaponCustom::GetActiveMovespeedMult() {
	// TODO: This multiplier is used server-side only. clients rubber-band when beginning a chargeup.
	// You might think duplicating the gauss or kickback prediction code would work for maxspeed,
	// but it doesn't. Updating maxspeed somehow retroactively affects previous movement simulations
	// which rubber bands you the instant that maxspeed is changed. Bascially creating the opposite
	// problem. Your job, should you choose to accept it, is to figure out why velocity and maxspeed
	// aren't predicted similarly despite being reset and updated just like any other state var.
	// I was able to get the beginning of a chargeup to feel right by saving a timestamp of when the
	// chargeup began and applying the multiplier in PM_Move() only when (pmove->time + ping) was
	// greater than that timestamp. That broke a lot of other predictions but gives a clue on how it
	// can work. PM_Move() is constantly repeating a small section of time and maxspeed changes affect
	// all steps of that time window instead of just after the point it changed.

	if (GetChargedState(0) != WC_CHARGE_STATE_NONE) {
		return MOVESPEED_MULT_TO_FLOAT(params.shootOpts[0].chargeMoveSpeedMult);
	}
	if (GetChargedState(1) != WC_CHARGE_STATE_NONE) {
		return MOVESPEED_MULT_TO_FLOAT(params.shootOpts[1].chargeMoveSpeedMult);
	}

	return 1.0f;
}

float CWeaponCustom::WallTime() {
#ifdef CLIENT_DLL
	return gEngfuncs.GetClientTime();
#else
	return g_engfuncs.pfnTime();
#endif
}

uint32_t CWeaponCustom::CmdTime() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	return m_pPlayer->m_cmdTime;
}

void CWeaponCustom::SetAkimbo(bool akimbo) {
	m_fireState = akimbo ? (m_fireState | FL_WC_STATE_IS_AKIMBO) : (m_fireState & ~FL_WC_STATE_IS_AKIMBO);

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
	m_fireState = enable ? (m_fireState | FL_WC_STATE_LASER) : (m_fireState & ~FL_WC_STATE_LASER);

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
		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			CBasePlayer* plr = UTIL_PlayerByIndex(i);

			if (!plr) {
				continue;
			}

			int beamWidth = 3;

			if (plr == m_pPlayer && IsPredicted())
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

bool CWeaponCustom::IsPrimaryAltActive() {
	if (!(params.flags & FL_WC_WEP_HAS_ALT_PRIMARY)) {
		return false;
	}

	if (WallTime() - m_lastAltToggle > 0.1f) {
		m_lastAltState = (m_fireState & FL_WC_STATE_PRIMARY_ALT); // debounce
	}
	
	return m_lastAltState;
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

	if (!pPlayer->UseSevenKewpGuns() && params.wrongClientWeapon) {
		return 0;
	}

	CWeaponCustom* wep = pOriginal ? pOriginal->MyWeaponCustomPtr() : NULL;

	if (wep && wep->IsAkimboWeapon() && !wep->CanAkimbo()) {
		wep->SetCanAkimbo(true);

		// wait for the player to stop shooting before deploying akimbo mode
		//wep->SetAkimbo(true);

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

void CWeaponCustom::SetCanAkimbo(bool canAkimbo) {
	if (canAkimbo) {
		m_fireState = m_fireState | FL_WC_STATE_CAN_AKIMBO;
	}
	else {
		m_fireState = m_fireState & ~FL_WC_STATE_CAN_AKIMBO;
	}
}

WcAttackState CWeaponCustom::GetChargedState(int attackIdx) {
	return (WcAttackState)((m_fInAttack >> (attackIdx * 4)) & 0xf);
}

void CWeaponCustom::SetChargedState(int attackIdx, WcAttackState newState) {
	int shift = attackIdx * 4;
	int mask = 0xf << shift;
	newState = (WcAttackState)((newState & 0xf) << shift);
	m_fInAttack = (m_fInAttack & ~mask) | newState;
}

void CWeaponCustom::SetPrimaryAlt(bool enable) {
	if (enable) {
		m_fireState = m_fireState | FL_WC_STATE_PRIMARY_ALT;
	}
	else {
		m_fireState = m_fireState & ~FL_WC_STATE_PRIMARY_ALT;
	}

	m_lastAltToggle = WallTime();
}

void CWeaponCustom::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	if (secondary) {
		if (params.ammoInfo[1].dropEnt) {
			ammoEntName = STRING(params.ammoInfo[1].dropEnt);
			dropAmount = params.ammoInfo[1].dropAmt;
		}
	}
	else {
		if (params.ammoInfo[0].dropEnt) {
			ammoEntName = STRING(params.ammoInfo[0].dropEnt);
			dropAmount = params.ammoInfo[0].dropAmt;
		}
	}
}

int CWeaponCustom::GetItemInfo(ItemInfo* p) {
	bool hideAmmo2 = params.flags & FL_WC_WEP_HIDE_SECONDARY_AMMO;
	int zoomFlag = (params.flags & FL_WC_WEP_FORCE_ZOOM_SPRITE) ? WEP_FLAG_USE_ZOOM_CROSSHAIR : 0;

	int flags = 0;
	if (params.flags & FL_WC_WEP_NO_AUTOSWITCHEMPTY) {
		flags |= ITEM_FLAG_NOAUTOSWITCHEMPTY;
	}
	if (params.flags & FL_WC_WEP_NO_AUTORELOAD) {
		flags |= ITEM_FLAG_NOAUTORELOAD;
	}
	if (params.flags & FL_WC_WEP_SELECTONEMPTY) {
		flags |= ITEM_FLAG_SELECTONEMPTY;
	}
	if (params.flags & FL_WC_WEP_EXHAUSITBLE) {
		flags |= ITEM_FLAG_EXHAUSTIBLE;
	}

	p->iSlot = params.slot;
	p->iPosition = params.slotPosition;
	p->pszAmmo1 = params.ammoInfo[0].type ? STRING(params.ammoInfo[0].type) : NULL;
	p->pszAmmo2 = !hideAmmo2 && params.ammoInfo[1].type ? STRING(params.ammoInfo[1].type) : NULL;
	p->pszName = STRING(m_hudPath);
	p->iMaxClip = params.ammoInfo[0].maxClip;
	p->iId = m_iId;
	p->iFlags = flags;
	p->iWeight = params.weight;
	p->iFlagsEx = zoomFlag;
	p->fAccuracyDeg = 0;
	p->fAccuracyDeg2 = 0;
	p->fAccuracyDegY = 0;
	p->fAccuracyDegY2 = 0;
	p->iId = m_iId;

	return 1;
}

LINK_ENTITY_TO_CLASS(weapon_custom_ini, CWeaponCustom)