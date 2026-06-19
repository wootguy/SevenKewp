#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "hlds_hooks.h"
#include "te_effects.h"
#include "CCrowbar.h"
#include "CRpg.h"

#ifdef CLIENT_DLL
#include "../cl_dll/hud_iface.h"
#include "../cl_dll/ev_hldm.h"
#include "../cl_dll/com_weapons.h"
#include "../cl_dll/cl_dll.h"
#include "../common/com_model.h"
#include "../game_shared/prediction_files.h"
extern bool g_cmd_debug_mode;
HSPRITE SPR_Load(const char* path);
#define PRINTF(msg, ...) gEngfuncs.Con_Printf(msg, ##__VA_ARGS__)
#define PRINTD(msg, ...) gEngfuncs.Con_DPrintf(msg, ##__VA_ARGS__)
#else
int g_runfuncs = 1;
#define PRINTF(fmt, ...)
#define PRINTD(fmt, ...)
#endif


void CWeaponCustom::Spawn() {
	Precache();

	m_iDefaultAmmo = defaultParams.ammoInfo[0].maxClip ? defaultParams.ammoInfo[0].maxClip : defaultParams.ammoInfo[0].defaultGive;

	if (defaultParams.ammoInfo[1].maxClip > 0) {
		m_iClip2 = V_min(defaultParams.ammoInfo[1].maxClip, defaultParams.ammoInfo[1].defaultGive);
	}

	ItemInfo info;
	info.iId = 0;
	if (GetItemInfo(&info))
		m_iId = info.iId;

	if (defaultParams.classname)
		pev->classname = defaultParams.classname;

	if (m_iId <= 0) {
		ALERT(at_error, "Custom weapon '%s' was not registered! Removing it from the world.\n",
			STRING(pev->classname));
		UTIL_Remove(this);
		return;
	}

	SetWeaponModelW();
	FallInit();// get ready to fall down.

	if (defaultParams.flags & FL_WC_WEP_USE_ONLY) {
		SetTouch(&CBaseEntity::ItemBounceTouch);
	}
}

void CWeaponCustom::Precache() {
	PrecacheEvents();

#ifndef CLIENT_DLL
	CBasePlayerWeapon::Precache();

	if (defaultParams.pmodelAkimbo) {
		PRECACHE_MODEL(STRING(defaultParams.pmodelAkimbo));
	}
	if (defaultParams.wmodelAkimbo) {
		bool hasMergeBody = mp_mergemodels.value && MergedModelBodyAkimbo() != -1;
		if (!hasMergeBody || UTIL_MapReplacesModel(STRING(defaultParams.wmodelAkimbo))) {
			PRECACHE_MODEL(STRING(defaultParams.wmodelAkimbo));
		}
	}

	if (defaultParams.wrongClientWeapon) {
		UTIL_PrecacheOther(STRING(defaultParams.wrongClientWeapon));
	}

	for (int i = 0; i < defaultParams.numEvents; i++) {
		WepEvt& evt = defaultParams.events[i];
		if (evt.evtType == WC_EVT_PROJECTILE && evt.proj.entity_class) {
			UTIL_PrecacheOther(STRING(evt.proj.entity_class));
		}
	}

	for (int i = 0; i < alternateParams.numEvents; i++) {
		WepEvt& evt = alternateParams.events[i];
		if (evt.evtType == WC_EVT_PROJECTILE && evt.proj.entity_class) {
			UTIL_PrecacheOther(STRING(evt.proj.entity_class));
		}
	}

	for (int i = 0; i < 2; i++) {
		if (defaultParams.ammoInfo[i].dropEnt)
			UTIL_PrecacheOther(STRING(defaultParams.ammoInfo[i].dropEnt));
	}

	if (defaultParams.defaultModelV_zoom)
		defaultParams.vmodel_zoom = PRECACHE_MODEL(STRING(defaultParams.defaultModelV_zoom));
	if (alternateParams.defaultModelV_zoom)
		alternateParams.vmodel_zoom = PRECACHE_MODEL(STRING(alternateParams.defaultModelV_zoom));

	defaultParams.vmodel = MODEL_INDEX(GetModelV());

	if (alternateParams.defaultModelV)
		alternateParams.vmodel = PRECACHE_MODEL(STRING(alternateParams.defaultModelV));
#endif
}

void CWeaponCustom::PrecacheEvents() {
	events.m_weapon = this;
	
#ifndef CLIENT_DLL
	m_configPath = g_customWeaponConfigs.get(STRING(pev->classname));

	if (m_configPath) {
		UTIL_ParseCustomWeaponConfig(m_configPath, defaultParams);

		if (defaultParams.flags & FL_WC_WEP_HAS_ALT_PARAMS) {
			std::string altPath = UTIL_VarArgs("%s.txt", STRING(defaultParams.altParams));
			if (UTIL_ParseCustomWeaponConfig(altPath.c_str(), alternateParams, true)) {
				g_customWeaponConfigsAlt.put(altPath.c_str());
			}
		}

		if (strcmp(STRING(pev->classname), STRING(defaultParams.classname))) {
			if (defaultParams.flags & FL_WC_WEP_AKIMBO) {
				// enable akimbo mode and world model if an "akimbo" alias is used
				if (strstr(STRING(pev->classname), "akimbo")) {
					SetCanAkimbo(true);
					SetAkimbo(true);
				}
			}

			pev->classname = defaultParams.classname; // undo alias
		}

		if (m_iId <= 0) {
			int* id = g_weaponClassIds.get(STRING(pev->classname));
			m_iId = id ? *id : 0;
		}

		m_defaultModelV = STRING(defaultParams.defaultModelV);
		m_defaultModelP = STRING(defaultParams.defaultModelP);
		m_defaultModelW = STRING(defaultParams.defaultModelW);

		int* mergedBody = g_merged_models.get(m_defaultModelW);
		m_mergedModelBody = mergedBody ? *mergedBody : -1;

		m_hasHandModels = defaultParams.flags & FL_WC_WEP_HAND_MODELS;
	}
	else {
		m_mergedModelBody = -1;

		if (m_defaultModelV) {
			defaultParams.defaultModelV = ALLOC_STRING(m_defaultModelV);
		}
		if (m_defaultModelP)
			defaultParams.defaultModelP = ALLOC_STRING(m_defaultModelP);
		if (m_defaultModelW)
			defaultParams.defaultModelW = ALLOC_STRING(m_defaultModelW);
	}

	defaultParams.classname = pev->classname;

	if (defaultParams.hudFolder) {
		m_hudPath = ALLOC_STRING(UTIL_VarArgs("%s/%s", STRING(defaultParams.hudFolder), STRING(pev->classname)));
		
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
	if (defaultParams.numEvents >= MAX_WC_EVENTS) {
		ALERT(at_error, "Exceeded max custom weapon events for %s\n", STRING(pev->classname));
		return;
	}

	defaultParams.events[defaultParams.numEvents++] = evt;
}

int CWeaponCustom::AddToPlayer(CBasePlayer* pPlayer) {
#ifndef CLIENT_DLL
	if (!pPlayer->UseSevenKewpGuns() && defaultParams.wrongClientWeapon) {
		if (pPlayer->HasNamedPlayerItem(STRING(defaultParams.wrongClientWeapon))) {
			return 0;
		}

		pPlayer->GiveNamedItem(STRING(defaultParams.wrongClientWeapon));
		m_pickupPlayers |= PLRBIT(pPlayer->edict());
		return 0;
	}

	if (defaultParams.flags & FL_WC_WEP_AKIMBO)
		SetAkimboClip(m_iDefaultAmmo);

	if (pPlayer->IsSevenKewpClient())
		UTIL_SendCustomWeaponPredictionData(pPlayer->edict(), this, WC_PRED_SEND_INIT);
#endif
	
	if (CBasePlayerWeapon::AddToPlayer(pPlayer)) {
		if (defaultParams.flags & FL_WC_WEP_EXCLUSIVE_HOLD) {
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

	CustomWeaponParams& params = GetActiveParams();

#ifndef CLIENT_DLL
	const char* validAnimExt = STRING(params.animExt);

	if (IsAkimbo())
		validAnimExt = STRING(params.animExtAkimbo);
	else if (m_pPlayer->m_iFOV != 0 && params.animExtZoom)
		validAnimExt = STRING(params.animExtZoom);

	if (m_pPlayer->m_playerModelAnimSet != PMODEL_ANIMS_HALF_LIFE_COOP) {
		// half-life models are missing animations for some weapons, so fallback to valid HL anims
		if (!strcmp(validAnimExt, "saw") || !strcmp(validAnimExt, "m16") || !strcmp(validAnimExt, "minigun")) {
			validAnimExt = "mp5";
		}
		else if (!strcmp(validAnimExt, "sniper")) {
			validAnimExt = "shotgun";
		}
		else if (!strcmp(validAnimExt, "sniperscope") || !strcmp(validAnimExt, "bowscope")) {
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

	CustomWeaponParams& params = GetActiveParams();

	m_chargeStartCmdTime = 0;
	m_chargeStopCmdTime = 0;
	m_lastCharge = 0;
	m_lastBeamUpdate = 0;
	m_fInReload = false;
	m_bInAkimboReload = false;
	m_fInSpecialReload = 0;
	m_chargeSoundEvt = 0;
	events.animCount = 0;
	m_iShotsFired = 0;
	m_bDelayFire = false;
	m_active_cs_recoil_evt = -1;
	m_attackChamberCmdTime = 0;
	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 0;
	ClearChargedStates();
	int ret = TRUE;

	m_pPlayer->SetThirdPersonWeaponAnim(0);

#ifdef CLIENT_DLL
	if (!CanDeploy())
		return FALSE;

	m_pPlayer->pev->viewmodel = params.vmodel;

	// prediction code will be trying to deploy the previous weapon because the server state hasn't
	// synced to the client yet. That causes the other weapon's deploy logic to run immediately
	// after this weapon deploys. Spam this weapon's deploy animation so that the correct anim plays.
	SendWeaponAnim(m_lastAnim, 1, pev->body);

	g_irunninggausspred = false;
	m_pPlayer->m_flNextAttack = 0.5;
	m_flTimeWeaponIdle = 1.0;
	g_last_attack_mode = 1;

#else
	const char* animSet = GetAnimSet();

	const char* vmodel = GetModelV();

	if (!params.vsprite_path && vmodel[0]) {
		studiohdr_t* mdl = GET_MODEL_PTR(PRECACHE_MODEL(vmodel));
		m_hasLaserAttachment = mdl && mdl->numattachments > params.laser.attachment;
	}

	ret = DefaultDeploy(vmodel, GetModelP(), 0, animSet, 1);
#endif

	// default cooldown
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	
	if (IsLaserOn()) {
		m_laserOnTime = WallTime() + m_flTimeWeaponIdle;
	}

	m_pPlayer->m_flNextAttack = 0; // allow thinking during deployment

	if (g_runfuncs && WallTime() - m_lastDeploy > MAX_PREDICTION_WAIT) {
		bool isFirstDeploy = !GetState(FL_WC_STATE_FIRST_DEPLOYED);

		bool handled =
			(IsAkimbo() && events.ProcessEvents(WC_TRIG_DEPLOY, WC_TRIG_DEPLOY_ARG_AKIMBO)) ||
			(GetState(FL_WC_STATE_CHAMBER_NEEDED) && events.ProcessEvents(WC_TRIG_DEPLOY, WC_TRIG_DEPLOY_ARG_CHAMBER)) ||
			(m_iClip == 0 && events.ProcessEvents(WC_TRIG_DEPLOY, WC_TRIG_DEPLOY_ARG_EMPTY)) ||
			(isFirstDeploy && events.ProcessEvents(WC_TRIG_DEPLOY, WC_TRIG_DEPLOY_ARG_FIRST)) ||
			(IsLaserOn() && events.ProcessEvents(WC_TRIG_DEPLOY, WC_TRIG_DEPLOY_ARG_LASER));

		if (!handled) {
			events.ProcessEvents(WC_TRIG_DEPLOY, WC_TRIG_DEPLOY_ARG_DEFAULT);
		}

		SetState(FL_WC_STATE_FIRST_DEPLOYED, true);
	}

	if (params.flags & FL_WC_WEP_IRON_SIGHTS_ZOOM) {
		// never deploy with iron sights mode active
		SetState(FL_WC_STATE_PRIMARY_ALT, false);
	}
	SetState(FL_WC_STATE_SECONDARY_RELOAD | FL_WC_STATE_ABORT_RELOAD | FL_WC_STATE_RELOAD_CLIP_DONE, false);

	// extra idle time added because high ping players are interrupted otherwise
	m_flTimeWeaponIdle = m_idleTime + 0.4f;

	m_lastDeploy = WallTime();

	if (params.jumpPower)
		m_pPlayer->SetJumpPower(params.jumpPower);

	return ret;
}

void CWeaponCustom::Holster(int skiplocal) {
	CBasePlayerWeapon::Holster();
	if (GetZoom())
		CycleZoom(0, true);
	UTIL_Remove(m_hLaserSpot);
#ifdef CLIENT_DLL
	EV_LaserOff();
#endif
	events.KillBeams();
	if (WallTime() - m_lastDeploy > 0.5f) {
		events.CancelDelayedEvents(-1);
	}
	m_lastDeploy = WallTime();
	m_fInReload = TRUE;
	m_fInSpecialReload = 0;
}

bool CWeaponCustom::CanReload(int attackIdx) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	CustomWeaponParams& params = GetActiveParams();

	if (m_fInReload)
		return false;

	bool canAkimboReload = IsAkimbo() && GetAkimboClip() < params.ammoInfo[0].maxClip;

	if (AreAnyAttacksCharging())
		return false;

	if (m_flNextPrimaryAttack > 0)
		return false;

	if ((attackIdx == 1 || (attackIdx == 0 && GetFlag(FL_WC_WEP_RELOAD2_IS_DEFAULT))) && !IsAkimbo()) {
		if (m_iClip2 == -1)
			return false;
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
			return false;
		if (m_iClip2 >= params.ammoInfo[1].maxClip)
			return false;
	}
	else {
		if (m_iClip >= params.ammoInfo[0].maxClip && !canAkimboReload && m_fInSpecialReload == 0) {
			m_bWantAkimboReload = false;
			return false;
		}
		if (m_iClip == -1)
			return false;
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 && m_fInSpecialReload != WC_RELOAD_STAGE_PUMP)
			return false;
	}

	return true;
}

void CWeaponCustom::LoadClip(int maxAmount, bool secondary) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;
	
	CustomWeaponParams& params = GetActiveParams();

	int& clip = secondary ? m_iClip2 : (m_bInAkimboReload ? m_chargeReady : m_iClip);
	int ammoType = secondary ? m_iSecondaryAmmoType : m_iPrimaryAmmoType;
	int ammoIdx = secondary ? 1 : 0;

	int amount = V_min(params.ammoInfo[ammoIdx].maxClip - clip, m_pPlayer->m_rgAmmo[ammoType]);
	
	if (maxAmount >= 0)
		amount = V_min(maxAmount, amount);

	if (amount <= 0)
		return;

	clip += amount;
	m_pPlayer->m_rgAmmo[ammoType] -= amount;

	m_pPlayer->TabulateAmmo();
}

void CWeaponCustom::Reload() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponParams& params = GetActiveParams();

	if (GetFlag(FL_WC_WEP_HAS_E_R_TOGGLE) && (m_pPlayer->pev->button & IN_USE)) {
		if (m_flNextPrimaryAttack <= 0 && QueueStateToggles(4)) {
			Cooldown(-1, params.erToggleCooldown);
		}
		return;
	}

	if (!CanReload(GetState(FL_WC_STATE_SECONDARY_RELOAD) ? 1 : 0)) {
		return;
	}

	bool shotgunReload = params.flags & FL_WC_WEP_SHOTGUN_RELOAD;

	// exit iron sights before reloading
	if (IsIronSights()) {
		SetState(FL_WC_STATE_WANT_RELOAD, true);

		int* clip = GetAttackClip(1);
		if (CommonAttack(1, clip, false, false)) { // TODO: configure this
			events.FireShootEvents(WC_TRIG_SECONDARY, false, clip, false);
			QueueStateToggles(1);
		}
		return;
	}

	WeaponCustomReload* reloadStage = &params.reloadStage[WC_RELOAD_STAGE_START];

	if (GetFlag(FL_WC_WEP_RELOAD2_IS_DEFAULT)) {
		m_fInSpecialReload = WC_RELOAD_STAGE_SECONDARY;
		reloadStage = &params.reloadStage[WC_RELOAD_STAGE_SECONDARY];
	}

	if (GetState(FL_WC_STATE_SECONDARY_RELOAD)) {
		m_fInSpecialReload = WC_RELOAD_STAGE_SECONDARY;
		reloadStage = &params.reloadStage[WC_RELOAD_STAGE_SECONDARY];
	}
	else if (IsAkimbo()) {
		m_fInSpecialReload = WC_RELOAD_STAGE_AKIMBO;
		reloadStage = &params.reloadStage[WC_RELOAD_STAGE_AKIMBO];
	}
	else if (shotgunReload && m_fInSpecialReload != 0 && m_fInSpecialReload != WC_RELOAD_STAGE_START_EMPTY) {
		if (m_fInSpecialReload == WC_RELOAD_STAGE_PUMP) {
			reloadStage = &params.reloadStage[WC_RELOAD_STAGE_PUMP];
		}
		else if (m_fInSpecialReload == WC_RELOAD_STAGE_SHELL) {
			reloadStage = &params.reloadStage[WC_RELOAD_STAGE_SHELL];
		}
	}
	else if (m_iClip == 0 && params.reloadStage[WC_RELOAD_STAGE_START_EMPTY].time) {
		m_fInSpecialReload = WC_RELOAD_STAGE_START_EMPTY;
		reloadStage = &params.reloadStage[WC_RELOAD_STAGE_START_EMPTY];
	}

	if (m_fInSpecialReload == WC_RELOAD_STAGE_SECONDARY)
		SetState(FL_WC_STATE_SECONDARY_RELOAD, true);

	m_reloadStageCmdTime[m_fInSpecialReload] = CmdTime();

	if (GetZoom())
		CycleZoom(0, true);

	//CLALERT("Start Reload %d %d\n", m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	m_fInReload = TRUE;
	m_iShotsFired = 0;
	m_bDelayFire = false;

	int totalReloadTime = reloadStage->time;
	float nextAttack = totalReloadTime * 0.001f;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = m_flTimeWeaponIdle = nextAttack;

	// allow shooting while doing the final pump after reloading, like with the HL shotty
	if (m_fInSpecialReload == WC_RELOAD_STAGE_PUMP && !(params.flags & FL_WC_WEP_SHOTGUN_SMOOTH_CANCEL))
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

	if (GetState(FL_WC_STATE_SECONDARY_RELOAD)) {
		events.ProcessEvents(WC_TRIG_RELOAD_SECONDARY, 0);

		events.m_bulletFireCount2 = 0;
	}
	else {
		bool doReloadStart = true;

		if (IsAkimbo()) {
			//events.ProcessEvents(WC_TRIG_RELOAD_AKIMBO, akimboArg);
		}
		else if (shotgunReload) {
			doReloadStart = false;

			if (m_fInSpecialReload == WC_RELOAD_STAGE_SHELL)
				events.FireShootEvents(WC_TRIG_RELOAD_SHELL, false, NULL, false);
			else if (m_fInSpecialReload == WC_RELOAD_STAGE_PUMP) {
				events.FireShootEvents(WC_TRIG_RELOAD_FINISH, false, NULL, false);
			}
			else {
				doReloadStart = true;
			}
		}

		if (doReloadStart) {
			events.FireShootEvents(WC_TRIG_RELOAD, false, NULL, false);

			if (m_iClip == 0) {
				events.FireShootEvents(WC_TRIG_RELOAD_EMPTY, false, NULL, false);
			}
			else {
				events.FireShootEvents(WC_TRIG_RELOAD_NOT_EMPTY, false, NULL, false);
			}
		}

		events.m_bulletFireCount = 0;
	}

	SetState(FL_WC_STATE_RELOAD_CLIP_DONE, false);

	if (g_runfuncs && IsLaserOn()) {
		HideLaser(true);
		m_laserOnTime = WallTime() + totalReloadTime * 0.001f;
	}

	m_pPlayer->m_flNextAttack = 0; // keep calling post frame for complex reloads
}

void CWeaponCustom::ReloadThink() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponParams& params = GetActiveParams();
	WeaponCustomReload& reloadStage = params.reloadStage[m_fInSpecialReload];

	if (m_fInReload && reloadStage.loadTime) {
		uint32_t timePassed = CmdTime() - m_reloadStageCmdTime[m_fInSpecialReload];
		if (!GetState(FL_WC_STATE_RELOAD_CLIP_DONE) && timePassed > reloadStage.loadTime) {
			int loadAmount = (params.flags & FL_WC_WEP_SHOTGUN_RELOAD) ? 1 : -1;
			LoadClip(loadAmount, GetState(FL_WC_STATE_SECONDARY_RELOAD));
			SetState(FL_WC_STATE_RELOAD_CLIP_DONE, true);
		}

		if (reloadStage.chamberTime && GetState(FL_WC_STATE_RELOAD_CLIP_DONE)) {
			bool chambered = timePassed < reloadStage.chamberTime;
			SetState(FL_WC_STATE_CHAMBER_NEEDED, chambered);
		}
	}

	bool reloadFinished = m_fInReload && m_flNextPrimaryAttack <= 0;
	if (!reloadFinished)
		return;

	// special shotgun reloading
	if (params.flags & FL_WC_WEP_SHOTGUN_RELOAD) {
		bool wantAbort = (m_pPlayer->pev->button & IN_ATTACK) | (m_pPlayer->pev->button & IN_ATTACK2);
		bool smoothCancel = params.flags & FL_WC_WEP_SHOTGUN_SMOOTH_CANCEL;

		if (wantAbort && !smoothCancel && m_iClip > 0) {
			m_fInSpecialReload = 0;
			m_fInReload = FALSE;
			return;
		}

		if (m_fInSpecialReload == 0 || m_fInSpecialReload == WC_RELOAD_STAGE_START_EMPTY) {
			// finished raising gun, now start loading shells
			m_fInSpecialReload = WC_RELOAD_STAGE_SHELL;
			m_fInReload = FALSE;

			if (wantAbort && smoothCancel) {
				SetState(FL_WC_STATE_ABORT_RELOAD, true);
				m_fInSpecialReload = WC_RELOAD_STAGE_PUMP;
			}

			Reload(); // start the next animation
			return;
		}
		else if (m_fInSpecialReload == WC_RELOAD_STAGE_SHELL) {
			// loading shells
			if (!GetState(FL_WC_STATE_RELOAD_CLIP_DONE))
				LoadClip(1, false);

			if (m_iClip >= params.ammoInfo[0].maxClip || m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0) {
				// play the finishing animation
				m_fInSpecialReload = WC_RELOAD_STAGE_PUMP;
			}
			else if (wantAbort && smoothCancel) {
				SetState(FL_WC_STATE_ABORT_RELOAD, true);
				m_fInSpecialReload = WC_RELOAD_STAGE_PUMP;
			}

			m_fInReload = FALSE;
			Reload();
			return;
		}
	}

	if (GetState(FL_WC_STATE_ABORT_RELOAD)) {
		SetState(FL_WC_STATE_SECONDARY_RELOAD | FL_WC_STATE_ABORT_RELOAD | FL_WC_STATE_RELOAD_CLIP_DONE, false);
		m_fInReload = FALSE;
		m_fInSpecialReload = 0;
		return;
	}

	// complete a simple reload.
	if (!GetState(FL_WC_STATE_RELOAD_CLIP_DONE))
		LoadClip(-1, GetState(FL_WC_STATE_SECONDARY_RELOAD));

	m_fInReload = FALSE;
	m_fInSpecialReload = 0;
	SetState(FL_WC_STATE_SECONDARY_RELOAD | FL_WC_STATE_ABORT_RELOAD | FL_WC_STATE_RELOAD_CLIP_DONE, false);

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

	if (GetState(FL_WC_STATE_WANT_RELOAD) && m_flNextPrimaryAttack <= 0) {
		SetState(FL_WC_STATE_WANT_RELOAD, false);
		Reload();
		return;
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_bWantAkimboReload && g_runfuncs) {
		Reload();
		return;
	}

	bool handled =
		(IsAkimbo() && events.ProcessEvents(WC_TRIG_IDLE, WC_TRIG_IDLE_ARG_AKIMBO)) ||
		(GetZoom() && !m_iClip && events.ProcessEvents(WC_TRIG_IDLE, WC_TRIG_IDLE_ARG_ZOOM_EMPTY)) ||
		(GetZoom() && events.ProcessEvents(WC_TRIG_IDLE, WC_TRIG_IDLE_ARG_ZOOM)) ||
		(!m_iClip && events.ProcessEvents(WC_TRIG_IDLE, WC_TRIG_IDLE_ARG_EMPTY)) ||
		(IsLaserOn() && events.ProcessEvents(WC_TRIG_IDLE, WC_TRIG_IDLE_ARG_LASER));

	if (!handled) {
		events.ProcessEvents(WC_TRIG_IDLE, WC_TRIG_IDLE_ARG_DEFAULT);
	}

	m_flTimeWeaponIdle = m_idleTime;

	WeaponIdleCustom();
}

void CWeaponCustom::ItemPostFrame() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponParams& params = GetActiveParams();

	if (m_nextMeleeDecal && m_nextMeleeDecal < gpGlobals->time) {
		m_nextMeleeDecal = 0;
		DecalGunshot(&m_meleeDecalPos, BULLET_PLAYER_CROWBAR);
	}
	events.UpdateBeams();

	PlayDelayedStateToggles();
	ReloadThink();

	if (GetState(FL_WC_STATE_CHAMBER_NEEDED)) {
		if (m_attackChamberCmdTime && CmdTime() > m_attackChamberCmdTime) {
			SetState(FL_WC_STATE_CHAMBER_NEEDED, false);
			m_attackChamberCmdTime = 0;
		}
		if (m_pPlayer->m_flNextAttack <= 0 && m_flNextPrimaryAttack <= 0 && m_flNextSecondaryAttack <= 0 && m_flNextTertiaryAttack <= 0) {
			SetState(FL_WC_STATE_CHAMBER_NEEDED, false);
		}
	}

	// cs recoil cooldown
	if (!(m_pPlayer->pev->button & (IN_ATTACK | IN_ATTACK2 | IN_ATTACK3))) {
		if (m_bDelayFire) {
			m_bDelayFire = false;

			m_flNextShotsFiredDec = 0.4f;
		}

		//if (IsSecondaryWeapon(m_iId)) {
		if (false) {
			m_iShotsFired = 0;
		}
		else
		{
			if (m_iShotsFired > 0 && m_flNextShotsFiredDec <= 0)
			{
				m_flNextShotsFiredDec = 0.0225f;
				m_iShotsFired--;
			}
		}
	}

	m_pPlayer->m_flNextAttack = 1; // prevent reload logic triggering
	CBasePlayerWeapon::ItemPostFrame();
	m_pPlayer->m_flNextAttack = 0;

	if (!g_runfuncs) {
		return;
	}

	UpdateStateHudSprite();
	events.PlayDelayedEvents();
	UpdateLaser();
}

const char* CWeaponCustom::GetModelP() {
	return defaultParams.pmodelAkimbo && IsAkimbo() ? STRING(defaultParams.pmodelAkimbo) : CBasePlayerWeapon::GetModelP();
}

const char* CWeaponCustom::GetModelW() {
#ifndef CLIENT_DLL
	if (defaultParams.wmodelAkimbo && CanAkimbo()) {
		bool hasMergeBody = mp_mergemodels.value && MergedModelBodyAkimbo() != -1;
		return hasMergeBody && !UTIL_MapReplacesModel(STRING(defaultParams.wmodelAkimbo)) ? MERGED_ITEMS_MODEL : STRING(defaultParams.wmodelAkimbo);
	}
#endif

	return CBasePlayerWeapon::GetModelW();
}

int* CWeaponCustom::GetAttackClip(int attackIdx) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return &m_iClip;

	CustomWeaponParams& params = GetActiveParams();
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	static int nullclip;
	int* clip = &nullclip;

	if (attackIdx == 0) {
		clip = IsAkimbo() ? &m_chargeReady : &m_iClip;

		if (m_iClip == -1 && m_iPrimaryAmmoType != -1)
			clip = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
	}
	if (attackIdx == 1) {
		if (IsAkimbo()) {
			if (m_iClip >= 0)
				clip = &m_iClip;
			else if (m_iPrimaryAmmoType != -1)
				clip = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
		}
		else {
			if (params.ammoInfo[1].maxClip > 0 && m_iClip2 >= 0)
				clip = &m_iClip2;
			else if (m_iSecondaryAmmoType != -1)
				clip = &m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType];
		}
	}

	switch (opts.ammoPool) {
	case WC_AMMOPOOL_PRIMARY_CLIP: clip = &m_iClip; break;
	case WC_AMMOPOOL_PRIMARY_RESERVE: clip = &m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]; break;
	case WC_AMMOPOOL_SECONDARY_CLIP: clip = &m_iClip2; break;
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
	CustomWeaponShootOpts& opts = GetShootOpts(0);

	if ((opts.flags & FL_WC_SHOOT_NEED_AKIMBO) && !CanAkimbo())
		return;

	if (GetFlag(FL_WC_WEP_HAS_PRIMARY)) {
		int* clip = GetAttackClip(0);

		if (CommonAttack(0, clip, IsAkimbo(), false)) {
			m_primaryFired = true;
			int trig = IsPrimaryAltActive() ? WC_TRIG_PRIMARY_ALT : WC_TRIG_PRIMARY;
			int attackIdx = IsPrimaryAltActive() ? 3 : 0;

			if (isAttackStart) {
				events.ProcessEvents(WC_TRIG_PRIMARY_START, 0);
				m_attackStartCmdTime = CmdTime();
			}
			events.FireShootEvents(trig, IsAkimbo(), clip, false);
			events.FireAmmoEvents(opts.ammoPool ? opts.ammoPool : (int)WC_AMMOPOOL_PRIMARY_CLIP, attackIdx);

			if (*clip < 0)
				*clip = 0;

			QueueStateToggles(0);
		}
	}

	KickbackPrediction();
}

void CWeaponCustom::SecondaryAttack() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (GetFlag(FL_WC_WEP_PRIMARY_PRIORITY) && (m_pPlayer->pev->button & IN_ATTACK)) {
		PrimaryAttack();
		return;
	}

	if (!GetFlag(FL_WC_WEP_HAS_SECONDARY | FL_WC_WEP_AKIMBO)) {
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

	CustomWeaponParams& params = GetActiveParams();
	CustomWeaponShootOpts& opts = GetShootOpts(1);

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
			events.ProcessEvents(primaryTrig, WC_TRIG_SHOOT_ARG_ALWAYS, false, fireBoth, *clip);
			events.FireAmmoEvents(opts.ammoPool ? opts.ammoPool : (int)WC_AMMOPOOL_PRIMARY_CLIP, 0);

			if (*clip < 0)
				*clip = 0;
		}
		
		m_pPlayer->random_seed++; // so bullets don't hit the same spot (seed only updates once per cmd)
		if (fireBoth && CommonAttack(0, &m_chargeReady, true, fireBoth)) {
			m_secondaryFired = true;

			if (isAttackStart)
				events.ProcessEvents(WC_TRIG_SECONDARY_START, WC_TRIG_SHOOT_ARG_AKIMBO, true, fireBoth, *clip);
			events.ProcessEvents(primaryTrig, WC_TRIG_SHOOT_ARG_AKIMBO, true, fireBoth, *clip);
			events.ProcessEvents(primaryTrig, WC_TRIG_SHOOT_ARG_ALWAYS, true, fireBoth, *clip);

			if (m_chargeReady < 0)
				m_chargeReady = 0;
		}
	}
	else if (params.flags & FL_WC_WEP_HAS_SECONDARY) {
		int* clip = GetAttackClip(1);

		if (CommonAttack(1, clip, false, false)) {
			m_secondaryFired = true;

			if (isAttackStart)
				events.ProcessEvents(WC_TRIG_SECONDARY_START, 0);
			events.FireShootEvents(WC_TRIG_SECONDARY, false, clip, false);
			events.FireAmmoEvents(opts.ammoPool ? opts.ammoPool : (int)WC_AMMOPOOL_SECONDARY_CLIP, 1);
		
			QueueStateToggles(1);
		}
	}

	KickbackPrediction();
}

void CWeaponCustom::TertiaryAttack() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponParams& params = GetActiveParams();
	CustomWeaponShootOpts& opts = params.shootOpts[2];

	if ((opts.flags & FL_WC_SHOOT_NEED_AKIMBO) && !CanAkimbo())
		return;

	if (params.flags & FL_WC_WEP_HAS_TERTIARY) {
		int* clip = GetAttackClip(2);

		if (CommonAttack(2, clip, false, false)) {
			events.FireShootEvents(WC_TRIG_TERTIARY, false, clip, false);
			events.FireAmmoEvents(opts.ammoPool, 2);

			if (*clip < 0)
				*clip = 0;

			QueueStateToggles(2);
		}
	}
}

bool CWeaponCustom::CommonAttack(int attackIdx, int* clip, bool leftHand, bool akimboFire) {
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	int clipLeft = *clip;

	bool semiAutoAttack = !(opts.flags & FL_WC_SHOOT_NO_ATTACK) && GetState(FL_WC_STATE_SEMI_AUTO);

	if ((opts.flags & FL_WC_SHOOT_NO_AUTOFIRE) || semiAutoAttack || clipLeft < opts.ammoCost) {
		if (GetState(FL_WC_STATE_WANT_RELOAD)) {
			// exiting iron-sights for a reload. Allow the "attack"
		}
		else if (attackIdx == 0 && !(m_pPlayer->m_afButtonPressed & IN_ATTACK))
			return false;
		else if (attackIdx == 1 && m_iClip2 == 0 && CanReload(1)) {
			SetState(FL_WC_STATE_SECONDARY_RELOAD, true);
			Reload();
			return false;
		}
		else if (attackIdx == 1 && !(m_pPlayer->m_afButtonPressed & IN_ATTACK2))
			return false;
	}

	bool isNormalAttack = !(opts.flags & FL_WC_SHOOT_NO_ATTACK);
	bool isMelee = opts.flags & FL_WC_SHOOT_IS_MELEE;
	bool inWater = m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD;

	m_bWantAkimboReload = false;

	if (isNormalAttack && inWater && !(opts.flags & FL_WC_SHOOT_UNDERWATER)) {
		FailAttack(attackIdx, leftHand, akimboFire, true);
		return false;
	}

	bool needFullCost = opts.flags & FL_WC_SHOOT_NEED_FULL_COST;
	bool ammoSpendsDuringCharge = opts.chargeTime > 0 && opts.chargeAmmoMode == WC_CHARGE_AMMO_LOAD;

	if (clipLeft < opts.ammoCost && needFullCost && !ammoSpendsDuringCharge) {
		FailAttack(attackIdx, leftHand, akimboFire, true);

		if (attackIdx == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 && m_iClip != -1) {
			m_flNextPrimaryAttack = 0; // force the reload
			SetState(FL_WC_STATE_SECONDARY_RELOAD, false);
			Reload();
		}
		if (attackIdx == 1 && CanReload(1)) {
			SetState(FL_WC_STATE_SECONDARY_RELOAD, true);
			Reload();
		}
		return false;
	}

	bool forceFireChargedShot = GetChargedState(attackIdx) == WC_CHARGE_STATE_DISCHARGING
		&& opts.chargeMode != WC_CHARGEUP_CONSTANT;
	
	if (!forceFireChargedShot && clipLeft <= 0 && opts.ammoCost > 0) {
		if (!m_fInReload) {
			if (attackIdx == 1 && CanReload(1)) {
				SetState(FL_WC_STATE_SECONDARY_RELOAD, true);
				Reload();
			}
			else {
				FailAttack(attackIdx, leftHand, akimboFire, true);
			}
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

		if (opts.chamberTime && *clip > 0) {
			m_attackChamberCmdTime = CmdTime() + opts.chamberTime;
			SetState(FL_WC_STATE_CHAMBER_NEEDED, true);
		}

		// for cs recoil
		// Using an event index will break prediction during the transition to a new index.
		// CS weapons only use one recoil set, and weapon mode toggles usually take 200ms
		// or so, so it should be fine.
		m_bDelayFire = true;
		m_iShotsFired++;
		if (m_iShotsFired > 15) {
			m_iShotsFired = 15;
		}

		if (m_active_cs_recoil_evt >= 0) {
			WepEvt& recoilEvt = defaultParams.events[m_active_cs_recoil_evt];
			int direction_change = events.GetActiveCstrikeRecoil(recoilEvt, m_pPlayer)[6];
			if (direction_change > 0 && !UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, direction_change)) {
				m_iDirection = m_iDirection ? 0 : 1; // pull gun left/right
			}
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
	int millis = overrideMillis;

	if (attackIdx != -1 && overrideMillis == -1) {
		CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

		millis = opts.cooldown;

		if (opts.cooldownOverride[WC_COOLDOWN_WATER]) {
			CBasePlayer* m_pPlayer = GetPlayer();
			if (m_pPlayer && m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD) {
				millis = opts.cooldownOverride[WC_COOLDOWN_WATER];
			}
		}

		if (opts.dischargedCooldown) {
			uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
			float t = V_min(chargeMillis / (float)opts.chargeTime, 1.0f);

			int startCooldown = opts.dischargedCooldown;
			int endCooldown = opts.cooldown;
			millis = startCooldown + (endCooldown - startCooldown) * t;
		}
	}

	float nextAttack = UTIL_WeaponTimeBase() + millis * 0.001f;

	if (GetFlag(FL_WC_WEP_UNLINK_COOLDOWNS)) {
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

	if (overrideMillis != -1)
		m_flTimeWeaponIdle = nextAttack + 1.0f;

	if (attackIdx != -1) {
		CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

		bool noAttack = (opts.flags & FL_WC_SHOOT_NO_ATTACK);
		bool alwaysIdleCooldown = opts.flags & FL_WC_SHOOT_COOLDOWN_IDLE;

		if (alwaysIdleCooldown || !noAttack)
			m_flTimeWeaponIdle = nextAttack + 1.0f;

		// default override cooldowns
		if (overrideMillis == -1) {
			if (opts.cooldownOverride[WC_COOLDOWN_PRIMARY]) {
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + opts.cooldownOverride[WC_COOLDOWN_PRIMARY] * 0.001f;
			}
			if (opts.cooldownOverride[WC_COOLDOWN_SECONDARY]) {
				m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + opts.cooldownOverride[WC_COOLDOWN_SECONDARY] * 0.001f;
			}
			if (opts.cooldownOverride[WC_COOLDOWN_TERTIARY]) {
				m_flNextTertiaryAttack = UTIL_WeaponTimeBase() + opts.cooldownOverride[WC_COOLDOWN_TERTIARY] * 0.001f;
			}
			if (opts.cooldownOverride[WC_COOLDOWN_IDLE]) {
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + opts.cooldownOverride[WC_COOLDOWN_IDLE] * 0.001f;
			}
		}
	}
}

float CWeaponCustom::GetChargeProgress(float chargeTime) {
	uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
	return V_min(chargeMillis / chargeTime, 1.0f);
}

void CWeaponCustom::PlayChargeSound(float t) {
	CustomWeaponParams& params = GetActiveParams();
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
	CustomWeaponParams& params = GetActiveParams();
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
	CustomWeaponParams& params = GetActiveParams();

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

	CustomWeaponParams& params = GetActiveParams();
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	if (ammoClick) {
		Cooldown(-1, 150);
		if (!(opts.flags & FL_WC_SHOOT_IS_MELEE))
			PlayEmptySound(attackIdx);
	}

	int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;

	int ievt = WC_TRIG_SECONDARY_FAIL;
	if (attackIdx == 0)
		ievt = IsPrimaryAltActive() ? WC_TRIG_PRIMARY_ALT_FAIL : WC_TRIG_PRIMARY_FAIL;
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

	if (opts.cooldownOverride[WC_COOLDOWN_FAIL])
		Cooldown(attackIdx, opts.cooldownOverride[WC_COOLDOWN_FAIL]);

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

	Vector offset = INT32_VEC3_TO_VECTOR(opts.melee.attackOffset);
	offset = offset.x * gpGlobals->v_forward
		+ offset.y * gpGlobals->v_up
		+ offset.z * gpGlobals->v_right;

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

		WcTrace evTrace = ConvertTrace(tr, vecSrc);
		events.FireImpactEvents(WC_TRIG_IMPACT, attackIdx, &evTrace);
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

void CWeaponCustom::ToggleLaser(bool enable) {
	SetLaser(enable);
	
#ifdef CLIENT_DLL
	if (!IsLaserOn()) {
		m_laserOnTime = 0;
	}

	HideLaser(!IsLaserOn());
#endif
}

void CWeaponCustom::HideLaser(bool hideNotUnhide) {
#ifdef CLIENT_DLL
	CustomWeaponParams& params = GetActiveParams();

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

bool CWeaponCustom::IsPredicted() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	return !(defaultParams.flags & FL_WC_WEP_NO_PREDICTION) && m_pPlayer->IsSevenKewpClient();
}

int CWeaponCustom::GetAttackIdx(WepEvt& evt) {
	switch (evt.trigger) {
	default:
	case WC_TRIG_PRIMARY:
	case WC_TRIG_PRIMARY_CHARGE:
	case WC_TRIG_PRIMARY_STOP:
	case WC_TRIG_PRIMARY_FAIL:
	case WC_TRIG_PRIMARY_ALT_FAIL:
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
	CustomWeaponParams& params = GetActiveParams();

	if (GetChargedState(0) != WC_CHARGE_STATE_NONE) {
		return D100_TO_FLOAT(params.shootOpts[0].chargeMoveSpeedMult);
	}
	if (GetChargedState(1) != WC_CHARGE_STATE_NONE) {
		return D100_TO_FLOAT(params.shootOpts[1].chargeMoveSpeedMult);
	}
	if (GetZoom()) {
		return D100_TO_FLOAT(params.zoomMoveSpeedMult);
	}

	return 1.0f;
}

std::string CWeaponCustom::GetStateString() {
	std::string state;

	if (GetState(FL_WC_STATE_FIRST_DEPLOYED)) state += "First deployed + ";
	if (GetState(FL_WC_STATE_ALT_PARAMS)) state += "Alt params + ";
	if (GetState(FL_WC_STATE_PRIMARY_ALT)) state += "Primary alt + ";
	if (GetState(FL_WC_STATE_SEMI_AUTO)) state += "Semiauto + ";
	if (GetState(FL_WC_STATE_CAN_AKIMBO)) state += "Can akimbo + ";
	if (GetState(FL_WC_STATE_IS_AKIMBO)) state += "Is akimbo + ";
	if (GetState(FL_WC_STATE_LASER)) state += "Laser + ";
	if (GetState(FL_WC_STATE_ZOOM)) state += "Zoom + ";
	if (GetState(FL_WC_STATE_ZOOM_FURTHER)) state += "Zoom further + ";
	if (GetState(FL_WC_STATE_SECONDARY_RELOAD)) state += "Secondary reload + ";
	if (GetState(FL_WC_STATE_WANT_RELOAD)) state += "Want reload + ";
	if (GetState(FL_WC_STATE_ABORT_RELOAD)) state += "Abort reload + ";
	if (GetState(FL_WC_STATE_RELOAD_CLIP_DONE)) state += "Reload clip done + ";
	if (GetState(FL_WC_STATE_CHAMBER_NEEDED)) state += "Chamber needed + ";

	if (state.size())
		state = state.substr(0, state.size() - 3);

	int queuedState = m_fireState >> 16;
	if (queuedState) {
		state += " (Queued attack " + std::to_string(queuedState) + " states)\n";
	}

	return state;
}

std::string CWeaponCustom::GetChargeStatesString() {
	std::string states;

	static std::string names[WC_ATTACK_TYPES] = { "Primary", "Secondary", "Tertiary", "Primary Alt"};
	for (int i = 0; i < WC_ATTACK_TYPES; i++) {
		int istate = GetChargedState(i);
		switch (istate) {
		case WC_CHARGE_STATE_NONE: states += ""; break;
		case WC_CHARGE_STATE_CHARGING: states += names[i] + " charging + "; break;
		case WC_CHARGE_STATE_DISCHARGING: states += names[i] + " discharge + "; break;
		case WC_CHARGE_STATE_OVERCHARGED: states += names[i] + " overcharged + "; break;
		}
	}

	if (states.size())
		states = states.substr(0, states.size() - 3);
	else
		states = "(none)";

	return states;
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
	SetState(FL_WC_STATE_IS_AKIMBO, akimbo);

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
	SetState(FL_WC_STATE_LASER, enable);

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

int CWeaponCustom::GetZoom() {
	uint8_t zoom_lo = GetState(FL_WC_STATE_ZOOM) ? 1 : 0;
	uint8_t zoom_hi = GetState(FL_WC_STATE_ZOOM_FURTHER) ? 1 : 0;
	return (zoom_hi << 1) | zoom_lo;
}

int CWeaponCustom::CycleZoom(int attackIdx, bool forceCancelZoom) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	CustomWeaponParams& params = GetActiveParams();
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	int zoomLevel = forceCancelZoom ? 0 : (GetZoom() + 1) % (opts.toggle.zoomLevels + 1);
	SetState(FL_WC_STATE_ZOOM, zoomLevel & 1);
	SetState(FL_WC_STATE_ZOOM_FURTHER, zoomLevel & 2);

	int zoomFov = zoomLevel ? opts.toggle.zoomFov[zoomLevel - 1] : 0;
	m_pPlayer->pev->fov = m_pPlayer->m_iFOV = zoomFov;

#ifdef CLIENT_DLL
	UpdateZoomCrosshair(m_iId, m_pPlayer->m_iFOV != 0);
#else
	UpdateAnimSet();
#endif

	// use non-empty events if there are no empty events defined
	if (m_iClip != 0 || !events.ProcessEvents(WC_TRIG_STATE, WC_TRIG_ARG_STATE_ZOOM_OUT_EMPTY + zoomLevel)) {
		events.ProcessEvents(WC_TRIG_STATE, WC_TRIG_ARG_STATE_ZOOM_OUT + zoomLevel);
	}
	if (m_iClip != 0 || !events.ProcessEvents(WC_TRIG_STATE, WC_TRIG_ARG_STATE_ZOOM_EMPTY)) {
		events.ProcessEvents(WC_TRIG_STATE, WC_TRIG_ARG_STATE_ZOOM);
	}

	if (params.zoomMoveSpeedMult) {
		m_pPlayer->ApplyEffects();
	}

	return zoomLevel;
}

void CWeaponCustom::UpdateLaser() {
	if (m_laserOnTime && m_laserOnTime < WallTime()) {
		m_laserOnTime = 0;
	}
#ifdef CLIENT_DLL
	if (IsLaserOn())
		HideLaser(m_laserOnTime > 0);
	else
		HideLaser(true);
#endif

#ifndef CLIENT_DLL
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponParams& params = GetActiveParams();

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
	if (!GetFlag(FL_WC_WEP_HAS_ALT_PRIMARY)) {
		return false;
	}
	
	return GetState(FL_WC_STATE_PRIMARY_ALT);
}

CustomWeaponParams& CWeaponCustom::GetActiveParams() {
	return GetState(FL_WC_STATE_ALT_PARAMS) ? alternateParams : defaultParams;
}

bool CWeaponCustom::GetFlag(int flagBit) {
	CustomWeaponParams& params = GetActiveParams();
	return params.flags & flagBit;
}

CustomWeaponShootOpts& CWeaponCustom::GetShootOpts(int attackIdx) {
	CustomWeaponParams& params = GetActiveParams();

	if (attackIdx < 0 || attackIdx >= WC_ATTACK_TYPES) {
		PRINTF("Invalid attack index %d\n", attackIdx);
		ALERT(at_error, "Invalid attack index %d\n", attackIdx);
		return params.shootOpts[0];
	}
	
	if (attackIdx == 0 && IsPrimaryAltActive()) {
		return params.shootOpts[3];
	}
	
	return params.shootOpts[attackIdx];
}

int CWeaponCustom::AddDuplicate(CBasePlayerItem* pOriginal) {
	CBasePlayer* pPlayer = pOriginal ? pOriginal->GetPlayer() : NULL;
	if (!pPlayer)
		return 0;

	if (!pPlayer->UseSevenKewpGuns() && defaultParams.wrongClientWeapon) {
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
	SetState(FL_WC_STATE_CAN_AKIMBO, canAkimbo);
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

void CWeaponCustom::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	CustomWeaponParams& params = GetActiveParams();
	
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
	CustomWeaponParams& params = GetActiveParams();

	bool hideAmmo2 = params.flags & FL_WC_WEP_HIDE_SECONDARY_AMMO;
	int zoomFlag = (params.flags & FL_WC_WEP_FORCE_ZOOM_SPRITE) ? WEP_FLAG_USE_ZOOM_CROSSHAIR : 0;
	zoomFlag |= (params.flags & FL_WC_WEP_IRON_SIGHTS_ZOOM) ? WEP_FLAG_NO_ZOOM_CROSSHAIR : 0;

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
	p->iMaxClip = params.ammoInfo[0].maxClip ? params.ammoInfo[0].maxClip : -1;
	p->iMaxClip2 = params.ammoInfo[1].maxClip;
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

studiohdr_t* CWeaponCustom::GetViewModelHeader() {
	CustomWeaponParams& params = GetActiveParams();
	
	if (params.vsprite_path)
		return NULL;

#ifdef CLIENT_DLL
	model_s* mdl = gEngfuncs.hudGetModelByIndex(GetActiveViewModelIdx());
	return (studiohdr_t*)(mdl ? mdl->cache.data : NULL);
#else
	return GET_MODEL_PTR(GetActiveViewModelIdx());
#endif
}

WeaponCustomToggle& CWeaponCustom::GetActiveToggle(int toggleIdx) {
	if (toggleIdx != 4) {
		CustomWeaponShootOpts& opts = GetShootOpts(toggleIdx);
		return opts.toggle;
	}

	CustomWeaponParams& params = GetActiveParams();

	return params.erToggle;
}

bool CWeaponCustom::QueueStateToggles(int toggleIdx) {
	WeaponCustomToggle& toggle = GetActiveToggle(toggleIdx);
	if (!toggle.stateBits)
		return false;

	uint32_t packed = (toggleIdx+1) & 0x7;
	m_fireState = (packed << 16) | ((uint32_t)m_fireState & 0xffff);

	if (g_runfuncs) {
		m_stateChangeCmdTime = CmdTime();
	}

	return true;
}

void CWeaponCustom::PlayDelayedStateToggles() {
	int delayIdx = m_fireState >> 16;
	if (!delayIdx)
		return; // no states queued for toggling
	
	CustomWeaponParams& params = GetActiveParams();

	int attackIdx = delayIdx - 1;
	WeaponCustomToggle& toggle = GetActiveToggle(attackIdx);

	if (!toggle.stateBits)
		return;

	bool togglingOn = false;
	switch (toggle.mode) {
	case WC_TOGGLE_STATE_OFF:
		togglingOn = false;
		break;
	case WC_TOGGLE_STATE_ON:
		togglingOn = true;
		break;
	case WC_TOGGLE_STATE_TOGGLE:
		togglingOn = !(m_fireState & toggle.stateBits);
		break;
	}

	int delay = togglingOn ? toggle.onDelay : toggle.offDelay;
	uint32_t toggleTime = m_stateChangeCmdTime + delay;

	if (CmdTime() - m_stateChangeCmdTime >= delay) {
		DoStateToggles(attackIdx);
		m_fireState &= 0xffff; // clear queue
	}
}

void CWeaponCustom::DoStateToggles(int attackIdx) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponParams& params = GetActiveParams();
	WeaponCustomToggle& toggle = GetActiveToggle(attackIdx);

	if (!toggle.stateBits)
		return;

	uint32_t toggleBits = toggle.stateBits;

	int trig = GetZoom() ? WC_TRIG_STATE_ZOOMED : WC_TRIG_STATE;

	if (toggleBits & FL_WC_STATE_ZOOM) {
		int zoomLevel = CycleZoom(toggle.zoomLevels);
		toggleBits &= ~FL_WC_STATE_ZOOM;

		if (params.vmodel_zoom) {
			string_t vmodel = zoomLevel ? params.defaultModelV_zoom : params.defaultModelV;
			m_pPlayer->pev->viewmodel = MAKE_STRING(GET_MODEL(STRING(vmodel)));
		}
	}

	switch (toggle.mode) {
	case WC_TOGGLE_STATE_OFF:
		SetState(toggleBits, false);
		break;
	case WC_TOGGLE_STATE_ON:
		SetState(toggleBits, true);
		break;
	case WC_TOGGLE_STATE_TOGGLE: {
		SetState(toggleBits, !GetState(toggleBits));
		break;
	}
	}

	if (toggleBits & FL_WC_STATE_IS_AKIMBO) {
		SetAkimbo(GetState(FL_WC_STATE_IS_AKIMBO));

		if (GetState(FL_WC_STATE_IS_AKIMBO)) {
			SendAkimboAnim(params.akimbo.deployAnim);
		}
		else {
			Deploy();
		}
	}

	if (toggleBits & FL_WC_STATE_LASER) {
		ToggleLaser(GetState(FL_WC_STATE_LASER));
		int arg = GetState(FL_WC_STATE_LASER) ? WC_TRIG_ARG_STATE_LASER_ON : WC_TRIG_ARG_STATE_LASER_OFF;
		events.ProcessEvents(trig, arg, 0);
		events.ProcessEvents(trig, WC_TRIG_ARG_STATE_LASER, 0);
	}

	if (toggleBits & FL_WC_STATE_SEMI_AUTO) {
		int arg = GetState(FL_WC_STATE_SEMI_AUTO) ? WC_TRIG_ARG_STATE_SEMI_AUTO_ON : WC_TRIG_ARG_STATE_SEMI_AUTO_OFF;
		events.ProcessEvents(trig, arg, 0);
		events.ProcessEvents(trig, WC_TRIG_ARG_STATE_SEMI_AUTO, 0);
		// TODO: prevent infinite loop if triggering a similar state change
	}

	if (toggleBits & FL_WC_STATE_ALT_PARAMS) {
		int arg = GetState(FL_WC_STATE_ALT_PARAMS) ? WC_TRIG_ARG_STATE_ALT_PARAMS_ON : WC_TRIG_ARG_STATE_ALT_PARAMS_OFF;
		int forceParams = GetState(FL_WC_STATE_ALT_PARAMS) ? WC_PARAMS_DEFAULT : WC_PARAMS_ALTERNATE;
		events.ProcessEvents(trig, arg, false, false, 0, NULL, forceParams);
		events.ProcessEvents(trig, WC_TRIG_ARG_STATE_ALT_PARAMS, false, false, 0, NULL, forceParams);
		UpdateAnimSet();
	}
}

void CWeaponCustom::SetState(uint16_t stateBits, bool state) {
	if (state)
		m_fireState |= stateBits;
	else
		m_fireState &= ~stateBits;
}

bool CWeaponCustom::GetState(int stateBits) {
	return m_fireState & stateBits;
}

void CWeaponCustom::UpdateStateHudSprite() {
#ifdef CLIENT_DLL
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	CustomWeaponParams& params = GetActiveParams();

	if (params.flags & FL_WC_WEP_HAS_STATE_SPRITE) {
		if (GetState(FL_WC_STATE_SEMI_AUTO)) {
			m_stateIconIdx = params.stateIcon.semiAutoIconIdx;
		}
		else if (GetState(FL_WC_STATE_PRIMARY_ALT)) {
			m_stateIconIdx = params.stateIcon.primaryAltIconIdx;
		}
		else {
			m_stateIconIdx = params.stateIcon.defaultIconIdx;
		}
	}

	if (params.vsprite_path) {
		ViewModelSprite& spr = m_viewModelSpr;

		if (!spr.hSprite && !spr.loadFailed) {
			const char* sprPath = GetDeltaString(params.vsprite_path);
			if (sprPath && sprPath[0]) {
				const char* loadPath = UTIL_VarArgs("sprites/%s.spr", sprPath);
				spr.hSprite = SPR_Load(loadPath);

				if (!spr.hSprite) {
					PRINTD("Failed to load HUD sprite %s\n", loadPath);
					spr.loadFailed = true;
					return;
				}
			}
		}

		float timeScale = (WallTime() - spr.lastUpdate) / 0.025f;
		spr.lastUpdate = WallTime();

		float bobX = cosf(spr.bobTime) * 16;
		float bobY = fabs(sinf(spr.bobTime) * 16);

		bool wantMove = m_pPlayer->pev->button & (IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
		bool moving = m_pPlayer->pev->velocity.Length() >= 10;
		bool shooting = m_flNextPrimaryAttack > 0;

		//PRINTF("BOB: %.2f %.2f\n", spr.bobTime, spr.moveScale);

		if (shooting) {
			spr.moveScale = 0;
		}
		else if (!wantMove || !moving)
		{
			spr.moveScale -= 0.1f * timeScale;
			if (spr.moveScale < 0)
				spr.moveScale = 0;
		}
		else
		{
			spr.bobTime += 0.1f * timeScale;
			spr.moveScale += 0.1f * timeScale;
			if (spr.moveScale > 1.0f)
				spr.moveScale = 1.0f;
		}

		bobX *= spr.moveScale;
		bobY *= spr.moveScale;

		float deployTime = WallTime() - m_lastDeploy;
		float deployY = 0;
		if (deployTime < 0.5f) {
			deployY = (0.5f - deployTime) * spr.h;
		}

		spr.color = gPlayerSim.light_color;

		if (spr.brighten > 0) {
			spr.brighten -= timeScale;
			int v = spr.brighten * 64;

			spr.color.r = V_min(255, spr.color.r + v);
			spr.color.g = V_min(255, spr.color.g + v);
			spr.color.b = V_min(255, spr.color.b + v);
		}

		// add gamma
		float gamma = 1.0f / 2.2f;
		spr.color.r = V_min(255, powf(spr.color.r / 255.0f, gamma) * 255.0f);
		spr.color.g = V_min(255, powf(spr.color.g / 255.0f, gamma) * 255.0f);
		spr.color.b = V_min(255, powf(spr.color.b / 255.0f, gamma) * 255.0f);

		float animTime = (CmdTime() - spr.animTime) * 0.001f;
		float fps = spr.fps * 0.01f;
		int frameIdx = clampi(animTime * fps, 0, spr.anim.arrSz - 1);
		spr.frame = spr.anim.arr[frameIdx];
		spr.x = (int8_t)spr.animOfsX.arr[frameIdx] + bobX;
		spr.y = (int8_t)spr.animOfsY.arr[frameIdx] + bobY + deployY;
		spr.w = gEngfuncs.pfnSPR_Width(spr.hSprite, spr.frame);
		spr.h = gEngfuncs.pfnSPR_Height(spr.hSprite, spr.frame);
		spr.scale = D100_TO_FLOAT(params.vsprite_base_scale);
		//PRINTF("T %.2f, FPS %.1f, FRAME %d, OFS: %d %d\n", animTime, fps, frameIdx, spr.x, spr.y);
	}
#endif
}

int CWeaponCustom::GetActiveViewModelIdx() {
	CustomWeaponParams& params = GetActiveParams();	
	return params.vmodel_zoom && GetZoom() ? params.vmodel_zoom : params.vmodel;
}

LINK_ENTITY_TO_CLASS(weapon_custom_ini, CWeaponCustom)