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
void WC_EV_LocalSound(int sndIdx, int chan, int pitch, float vol, float attn, int panning, int flags);
void WC_EV_EjectShell(WepEvt& evt, bool leftHand);
void WC_EV_PunchAngle(WepEvt& evt, int seed);
void WC_EV_WepAnim(WepEvt& evt, int wepid, int animIdx);
pmtrace_t WC_EV_FireBullets(float spreadX, float spreadY, bool showTracer, int tracerColor, bool gunshotDecal, bool textureSound, int iShot, int iDamage);
void EV_LaserOff();
void WC_EV_Dlight(WepEvt& evt);
uint32_t GetTimeAtCmd(uint32_t cmdId);
Vector WC_GetGunPosition();
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

int CWeaponCustom::m_tracerCount[32];
uint32_t CWeaponCustom::m_predDataSent[MAX_WEAPONS];
TraceResult g_traces[256];

// convert a client-side trace struct to the server-side kind
#ifdef CLIENT_DLL
WcTrace ConvertTrace(pmtrace_t tr) {
	WcTrace out;

	out.fAllSolid = tr.allsolid;
	out.fStartSolid = tr.startsolid;
	out.fInOpen = tr.inopen;
	out.fInWater = tr.inwater;
	out.flFraction = tr.fraction;
	out.vecEndPos = tr.endpos;
	out.flPlaneDist = tr.plane.dist;
	out.vecPlaneNormal = tr.plane.normal;
	out.iHitgroup = tr.hitgroup;
	out.pHit = gEngfuncs.pEventAPI->EV_IndexFromTrace(&tr);

	return out;
}
#else
WcTrace ConvertTrace(TraceResult tr) {
	WcTrace out;

	out.fAllSolid = tr.fAllSolid;
	out.fStartSolid = tr.fStartSolid;
	out.fInOpen = tr.fInOpen;
	out.fInWater = tr.fInWater;
	out.flFraction = tr.flFraction;
	out.vecEndPos = tr.vecEndPos;
	out.flPlaneDist = tr.flPlaneDist;
	out.vecPlaneNormal = tr.vecPlaneNormal;
	out.iHitgroup = tr.iHitgroup;
	out.pHit = tr.pHit ? ENTINDEX(tr.pHit) : 0;

	return out;
}
#endif

bool WcBeam::isFree() {
#ifdef CLIENT_DLL
	return !pBeam || pBeam->die < gEngfuncs.GetClientTime();
#else
	return !h_beam.GetEntity();
#endif
}

void WcSprite::Kill() {
#ifdef CLIENT_DLL
	if (pSprite) {
		pSprite->die = gEngfuncs.GetClientTime();
		pSprite->callback = &EV_EgonFlareCallback;
		pSprite->fadeSpeed = 2.0;			// fade out will take 0.5 sec
		pSprite->tentOffset.x = 10.0;		// scaling speed per second
		pSprite->tentOffset.y = 0.1;			// min time between two scales
		pSprite->tentOffset.z = pSprite->die;	// the last callback run time
		pSprite->flags = FTENT_FADEOUT | FTENT_CLIENTCUSTOM;
		pSprite = NULL;
	}
#else
	if (h_sprite.GetEntity()) {
		CSprite* spr = (CSprite*)h_sprite.GetEntity();
		spr->Expand(10, 500);
		h_sprite = NULL;
	}
#endif
}

bool WcSprite::IsAlive() {
#ifdef CLIENT_DLL
	return pSprite;
#else
	return h_sprite.GetEntity();
#endif
}

void CWeaponCustom::Spawn() {
	Precache();

	m_iDefaultAmmo = params.maxClip;

	ItemInfo info;
	info.iId = 0;
	if (GetItemInfo(&info))
		m_iId = info.iId;

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
}

void CWeaponCustom::PrecacheEvents() {
#ifndef CLIENT_DLL
	if (pmodelAkimbo)
		PRECACHE_MODEL(pmodelAkimbo);
	if (wmodelAkimbo) {
		bool hasMergeBody = mp_mergemodels.value && MergedModelBodyAkimbo() != -1;
		if (!hasMergeBody || UTIL_MapReplacesModel(wmodelAkimbo)) {
			PRECACHE_MODEL(wmodelAkimbo);
		}
	}

	if (wrongClientWeapon) {
		UTIL_PrecacheOther(wrongClientWeapon);
	}
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
	if (!pPlayer->UseSevenKewpGuns() && wrongClientWeapon) {
		if (pPlayer->HasNamedPlayerItem(wrongClientWeapon)) {
			return 0;
		}

		pPlayer->GiveNamedItem(wrongClientWeapon);
		m_pickupPlayers |= PLRBIT(pPlayer->edict());
		return 0;
	}

	if (params.flags & FL_WC_WEP_AKIMBO)
		SetAkimboClip(m_iDefaultAmmo);

	if (pPlayer->IsSevenKewpClient())
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
	else if (m_pPlayer->m_iFOV != 0 && animExtZoom)
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

	m_chargeStartCmdTime = 0;
	m_lastBeamUpdate = 0;
	m_fInReload = false;
	m_bInAkimboReload = false;
	m_fInSpecialReload = 0;
	m_chargeSoundEvt = 0;
	animCount = 0;
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
		ProcessEvents(WC_TRIG_DEPLOY, akimboArg);
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
	KillBeams();
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

	bool canAkimboReload = IsAkimbo() && GetAkimboClip() < params.maxClip;
	bool shotgunReload = params.flags & FL_WC_WEP_SHOTGUN_RELOAD;

	if (AreAnyAttacksCharging())
		return;
	if (m_iClip == -1)
		return;
	if (m_iClip >= params.maxClip && !canAkimboReload && m_fInSpecialReload == 0) {
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
				ProcessEvents(WC_TRIG_RELOAD, akimboArg);
			else if (m_fInSpecialReload == 2) {
				ProcessEvents(WC_TRIG_RELOAD_FINISH, akimboArg);
			}
		}
		else {
			ProcessEvents(WC_TRIG_RELOAD, akimboArg);
		}
		
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

		m_bulletFireCount = 0;
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
		if (m_iClip == 0 && params.maxClip) {
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
	UpdateBeams();

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
				if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 && m_iClip < params.maxClip) {
					m_iClip++;
					m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 1;
					m_pPlayer->TabulateAmmo();
				}

				if (m_iClip >= params.maxClip || m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] == 0) {
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
		int j = V_min(params.maxClip - clip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
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
#ifndef CLIENT_DLL
	if (wmodelAkimbo && CanAkimbo()) {
		bool hasMergeBody = mp_mergemodels.value && MergedModelBodyAkimbo() != -1;
		return hasMergeBody && !UTIL_MapReplacesModel(wmodelAkimbo) ? MERGED_ITEMS_MODEL : wmodelAkimbo;
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

			if (isAttackStart)
				ProcessEvents(WC_TRIG_PRIMARY_START, 0);
			ProcessEvents(trig, akimboArg, IsAkimbo(), false, *clip);
			FireAmmoEvents(opts.ammoPool ? opts.ammoPool : WC_AMMOPOOL_PRIMARY_CLIP);

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

			if (isAttackStart)
				ProcessEvents(WC_TRIG_SECONDARY_START, WC_TRIG_SHOOT_ARG_AKIMBO, false, fireBoth, *clip);
			ProcessEvents(primaryTrig, WC_TRIG_SHOOT_ARG_AKIMBO, false, fireBoth, *clip);
			FireAmmoEvents(opts.ammoPool ? opts.ammoPool : WC_AMMOPOOL_PRIMARY_CLIP);

			if (*clip < 0)
				*clip = 0;
		}
		
		m_pPlayer->random_seed++; // so bullets don't hit the same spot (seed only updates once per cmd)
		if (fireBoth && CommonAttack(0, &m_chargeReady, true, fireBoth)) {
			m_secondaryFired = true;

			if (isAttackStart)
				ProcessEvents(WC_TRIG_SECONDARY_START, WC_TRIG_SHOOT_ARG_AKIMBO, true, fireBoth, *clip);
			ProcessEvents(primaryTrig, WC_TRIG_SHOOT_ARG_AKIMBO, true, fireBoth, *clip);

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
				ProcessEvents(WC_TRIG_SECONDARY_START, 0);
			ProcessEvents(WC_TRIG_SECONDARY, akimboArg, *clip);
			FireAmmoEvents(opts.ammoPool ? opts.ammoPool : WC_AMMOPOOL_SECONDARY_RESERVE);
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

			ProcessEvents(WC_TRIG_TERTIARY, akimboArg, false, false, *clip);
			FireAmmoEvents(opts.ammoPool);

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

	bool forceFireChargedShot = GetChargedState(attackIdx) == WC_CHARGE_STATE_DISCHARGING;
	bool ammoSpendsDuringCharge = opts.chargeTime > 0 && opts.chargeAmmoMode == WC_CHARGE_AMMO_LOAD;
	if (!forceFireChargedShot && clipLeft <= 0 && opts.ammoCost > 0) {
		if (!m_fInReload) {
			FailAttack(attackIdx, leftHand, akimboFire, true);
		}
		return false;
	}

	if (!Chargeup(attackIdx, clip, leftHand, akimboFire))
		return false;

	if (isNormalAttack && !forceFireChargedShot) {
		bool needFullCost = opts.flags & FL_WC_SHOOT_NEED_FULL_COST;
		
		if (clipLeft < opts.ammoCost && needFullCost && !ammoSpendsDuringCharge) {
			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0 && m_iClip != -1)
				Reload();
			else {
				FailAttack(attackIdx, leftHand, akimboFire, true);
			}
			return false;
		}

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

float CWeaponCustom::GetChargeMult(WepEvt& evt, int flagMask) {
	int attackIdx = GetAttackIdx(evt);
	if (params.flags & FL_WC_WEP_LINK_CHARGEUPS) {
		attackIdx = 0;
	}
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	if (!opts.chargeTime)
		return 1.0f;

	if (!(opts.chargeFlags & flagMask))
		return 1.0f;

	uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
	return V_min(chargeMillis / (float)opts.chargeTime, 1.0f);
}

bool CWeaponCustom::Chargeup(int attackIdx, int* clip, bool leftHand, bool akimboFire) {
	if (GetChargedState(attackIdx) == WC_CHARGE_STATE_DISCHARGING)
		return true;
	
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	if (!opts.chargeTime)
		return true;

	if (params.flags & FL_WC_WEP_LINK_CHARGEUPS) {
		attackIdx = 0;
	}

	if (GetChargedState(attackIdx) == WC_CHARGE_STATE_NONE) {
		m_chargeStartCmdTime = CmdTime();
		m_chargeStartClip = *clip;
		SetChargedState(attackIdx, WC_CHARGE_STATE_CHARGING);
		int e = (attackIdx == 0) ? WC_TRIG_PRIMARY_CHARGE : WC_TRIG_SECONDARY_CHARGE;
		ProcessEvents(e, 0, leftHand, akimboFire);
		m_pPlayer->ApplyEffects();
	}

	uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
	float t = V_min(chargeMillis / (float)opts.chargeTime, 1.0f);

	if (opts.ammoCost && opts.chargeAmmoMode == WC_CHARGE_AMMO_LOAD) {
		int cost = V_max(1, (t * opts.ammoCost) + 0.5f);
		*clip = V_max(0, m_chargeStartClip - cost);

		if (*clip == 0) {
			SetChargedState(attackIdx, WC_CHARGE_STATE_DISCHARGING);
			return true;
		}
	}

	if (m_chargeSoundEvt) {
		WepEvt& evt = params.events[clamp(m_chargeSoundEvt, 0, MAX_WC_EVENTS-1)];
		int channel = evt.playSound.channel;
		int soundIdx = evt.playSound.sound;
		
		int pitchRange = evt.playSound.pitchMax - evt.playSound.pitchMin;
		
		int pitch = evt.playSound.pitchMin + pitchRange*t;

#ifdef CLIENT_DLL
		WC_EV_LocalSound(soundIdx, channel, pitch, 1, ATTN_NORM, 0, SND_CHANGE_PITCH);
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

	if (opts.overchargeTime && chargeMillis >= opts.overchargeTime) {
		if (opts.overchargeMode == WC_OVERCHARGE_CANCEL) {
			FailAttack(attackIdx, leftHand, akimboFire, false);
		}
		
		int chargeState = GetChargedState(attackIdx);
		if (chargeState != WC_CHARGE_STATE_OVERCHARGED) {
			int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
			int ievt = attackIdx == 0 ? WC_TRIG_PRIMARY_OVERCHARGE : WC_TRIG_SECONDARY_OVERCHARGE;
			ProcessEvents(ievt, akimboArg, leftHand, false);
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

	return chargeMillis >= opts.chargeTime;
}

void CWeaponCustom::Chargedown(int attackIdx) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	int ievt = attackIdx == 0 ? WC_TRIG_PRIMARY_STOP : WC_TRIG_SECONDARY_STOP;

	//PRINTF("Charging down... %d\n", g_runfuncs);

	CancelDelayedEvents(attackIdx == 0 ? WC_TRIG_PRIMARY_CHARGE : WC_TRIG_SECONDARY_CHARGE);
	ProcessEvents(ievt, 0, false, false);

	// prevent idling immediately after chargedown in case an animation needs to play
	// otherwise the prediction code will idle before the server syncs the new idle delay
	m_flTimeWeaponIdle = V_max(m_flTimeWeaponIdle, 0.5f);

	SetChargedState(attackIdx, WC_CHARGE_STATE_NONE);
	m_chargeSoundEvt = 0;

	m_pPlayer->ApplyEffects();
}

void CWeaponCustom::FinishAttack(int attackIdx) {
	if (params.flags & FL_WC_WEP_LINK_CHARGEUPS) {
		attackIdx = 0;
	}

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	bool attackCalled = attackIdx == 0 ? m_primaryCalled : m_secondaryCalled;
	bool attackFired = attackIdx == 0 ? m_primaryFired : m_secondaryFired;

	if (m_primaryFired && g_runfuncs) {
		KillBeams(attackIdx);
	}

	// not a charging attack. Call the stop event.
	CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);
	if (attackFired && !opts.chargeTime) {
		int ievt = attackIdx == 0 ? WC_TRIG_PRIMARY_STOP : WC_TRIG_SECONDARY_STOP;
		CancelDelayedEvents(attackIdx == 0 ? WC_TRIG_PRIMARY_START : WC_TRIG_SECONDARY_START);
		ProcessEvents(ievt, 0, false, false);
		return;
	}
	
	if (GetChargedState(attackIdx) == WC_CHARGE_STATE_NONE) {
		return;
	}

	uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
	uint32_t cancelMillis = params.shootOpts[attackIdx].chargeCancelTime;
	bool isChargedEnough = chargeMillis >= cancelMillis;

	if (cancelMillis) {
		uint32_t chargeMillis = CmdTime() - m_chargeStartCmdTime;
		float t = V_min(chargeMillis / (float)opts.chargeTime, 1.0f);

		if (opts.ammoCost && opts.chargeAmmoMode == WC_CHARGE_AMMO_LOAD) {
			int cost = V_max(1, (t * opts.ammoCost) + 0.5f);
			int* clip = GetAttackClip(attackIdx);
			*clip = m_chargeStartClip - cost;
		}
	}

	if (!cancelMillis || isChargedEnough) {
		CustomWeaponShootOpts& opts = GetShootOpts(attackIdx);

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
				WC_EV_LocalSound(opts.emptySound, CHAN_STATIC, 100, 1.0f, ATTN_NORM, 0, 0);
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
	ProcessEvents(ievt, akimboArg, leftHand, false);
	
	CancelDelayedEvents(attackIdx == 0 ? WC_TRIG_PRIMARY_START : WC_TRIG_SECONDARY_START);
	CancelDelayedEvents(attackIdx == 0 ? WC_TRIG_PRIMARY_CHARGE : WC_TRIG_SECONDARY_CHARGE);

	if (KillBeams(attackIdx)) {
		int ievt2 = attackIdx == 0 ? WC_TRIG_PRIMARY_STOP : WC_TRIG_SECONDARY_STOP;
		ProcessEvents(ievt2, akimboArg, leftHand, false);
	}

	if (m_chargeSoundEvt) {
		WepEvt& evt = params.events[clamp(m_chargeSoundEvt, 0, MAX_WC_EVENTS - 1)];
		int channel = evt.playSound.channel;
		int soundIdx = evt.playSound.sound;

#ifdef CLIENT_DLL
		WC_EV_LocalSound(soundIdx, channel, 100, 1, ATTN_NORM, 0, SND_STOP);
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

		ProcessEvents(WC_TRIG_IMPACT, GetImpactArg(attackIdx, true, true));
		AttackTrace(m_pPlayer, attackIdx, vecSrc, tr);

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

int CWeaponCustom::GetImpactArg(int attackIdx, bool impactMonster, bool impactWorld) {
	switch (attackIdx) {
	default:
	case 0:
		if (impactMonster && impactWorld)	return WC_TRIG_IMPACT_PRIMARY_ANY;
		if (impactMonster)					return WC_TRIG_IMPACT_PRIMARY_MONSTER;
		return WC_TRIG_IMPACT_PRIMARY_WORLD;
	case 1:
		if (impactMonster && impactWorld)	return WC_TRIG_IMPACT_SECONDARY_ANY;
		if (impactMonster)					return WC_TRIG_IMPACT_SECONDARY_MONSTER;
		return WC_TRIG_IMPACT_SECONDARY_WORLD;
	case 2:
		if (impactMonster && impactWorld)	return WC_TRIG_IMPACT_TERTIARY_ANY;
		if (impactMonster)					return WC_TRIG_IMPACT_TERTIARY_MONSTER;
		return WC_TRIG_IMPACT_TERTIARY_WORLD;
	case 3:
		if (impactMonster && impactWorld)	return WC_TRIG_IMPACT_PRIMARY_ALT_ANY;
		if (impactMonster)					return WC_TRIG_IMPACT_PRIMARY_ALT_MONSTER;
		return WC_TRIG_IMPACT_PRIMARY_ALT_WORLD;
	}
}

WcBeam* CWeaponCustom::AllocBeam() {
	for (int i = 0; i < MAX_WC_BEAMS; i++) {
		if (m_beams[i].isFree()) {
			memset(&m_beams[i], 0, sizeof(WcBeam));
			return &m_beams[i];
		}
	}

	ALERT(at_warning, "Exceeded max weapon custom beams (%d)\n", MAX_WC_BEAMS);
	return NULL;
}

void CWeaponCustom::UpdateBeams() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

#ifdef CLIENT_DLL
	Vector vecSrc = WC_GetGunPosition();
#else
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
#endif

	for (int i = 0; i < MAX_WC_BEAMS; i++) {
		if (m_beams[i].isFree()) {
			continue;
		}
		WcBeam& beam = m_beams[i];
		bool isConstantBeam = beam.evt.beam.life == 0;

		if (beam.nextAttack && beam.nextAttack < WallTime()) {
			Vector attackPos = BeamAttack(beam, m_pPlayer);

			if (!isConstantBeam) {
#ifdef CLIENT_DLL
				beam.pBeam->target = attackPos;
#else
				CBeam* pbeam = (CBeam*)beam.h_beam.GetEntity();
				pbeam->SetStartPos(attackPos);
#endif
			}
		}

		if (beam.evt.beam.altMode != WC_BEAM_ANIM_DISABLED && beam.evt.beam.altTime) {
			float dur = beam.evt.beam.altTime * 0.001f;
			float p = normalizeRangef(gpGlobals->time - beam.creationTime, 0, dur) / dur; // progress
			float q = 1.0f - p; // progress left

			int t = p * 10000; // convert to int for modulo op later
			int freq = dur * 10000;  // time to alternate (half a cycle)

			switch (beam.evt.beam.altMode)
			{
			case WC_BEAM_ANIM_LINEAR:
			case WC_BEAM_ANIM_LINEAR_TOGGLE: p = p; break;
			case WC_BEAM_ANIM_TOGGLE: p = (p < 0.5) ? 0 : 1; break;
			case WC_BEAM_ANIM_EASE_IN_OUT:   p = p * p * p / (p * p * p + q * q * q); break;
			}

			if (beam.evt.beam.altMode != WC_BEAM_ANIM_TOGGLE && t % (freq * 2) >= freq)
				p = 1.0f - p;

			RGBA C;
			int width, noise, scroll;

			{	// color interp
				RGBA A = beam.evt.beam.color;
				RGBA B = beam.evt.beam.colorAlt;
				int dr = ((int)B.r - (int)A.r) * p + 0.5f;
				int dg = ((int)B.g - (int)A.g) * p + 0.5f;
				int db = ((int)B.b - (int)A.b) * p + 0.5f;
				int da = ((int)B.a - (int)A.a) * p + 0.5f;
				C = RGBA(A.r + dr, A.g + dg, A.b + db, A.a + da);
			}
			{	// width interp
				int a = beam.evt.beam.width;
				int b = beam.evt.beam.widthAlt;
				int d = (b - a) * p + 0.5f;
				width = a + d;
			}
			{	// noise interp
				int a = beam.evt.beam.noise;
				int b = beam.evt.beam.noiseAlt;
				int d = (b - a) * p + 0.5f;
				noise = a + d;
			}
			{	// scroll interp
				int a = beam.evt.beam.scrollRate;
				int b = beam.evt.beam.scrollRateAlt;
				int d = (b - a) * p + 0.5f;
				scroll = a + d;
			}

#ifdef CLIENT_DLL
			beam.pBeam->r = C.r / 255.0f;
			beam.pBeam->g = C.g / 255.0f;
			beam.pBeam->b = C.b / 255.0f;
			beam.pBeam->brightness = C.a / 255.0f;
			beam.pBeam->width = width / 10.0f;
			beam.pBeam->amplitude = noise / 100.0f;
			beam.pBeam->speed = scroll;
#else
			CBeam* pbeam = (CBeam*)beam.h_beam.GetEntity();
			if (pbeam) {
				pbeam->SetColor(C.r, C.g, C.b);
				pbeam->SetBrightness(C.a);
				pbeam->SetWidth(width);
				pbeam->SetNoise(noise);
				pbeam->SetScrollRate(scroll);
			}
#endif
		}

		if (!isConstantBeam)
			continue;

#ifdef CLIENT_DLL
		Vector vecDir = WC_GetAim(beam.spreadX, beam.spreadY);
		Vector vecEnd = vecSrc + vecDir * beam.evt.beam.distance;

		pmtrace_t tr;
		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);

		beam.pBeam->target = tr.endpos;

		if (m_beamImpactSprite.pSprite) {
			if (i == m_beamImpactSprite.beamId) {
				m_beamImpactSprite.pSprite->entity.origin = tr.endpos;
			}
			if (m_beamImpactSprite.killTime && m_beamImpactSprite.killTime < gpGlobals->time) {
				m_beamImpactSprite.Kill();
			}
		}
#else
		Vector vecDir = vecAiming + beam.spreadX * gpGlobals->v_right + beam.spreadY * gpGlobals->v_up;
		Vector vecEnd = vecSrc + vecDir * beam.evt.beam.distance;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

		CBeam* pbeam = (CBeam*)beam.h_beam.GetEntity();
		if (pbeam) {
			pbeam->SetStartPos(tr.vecEndPos);
			if (m_beamImpactSprite.h_sprite) {
				if (i == m_beamImpactSprite.beamId) {
					UTIL_SetOrigin(m_beamImpactSprite.h_sprite->pev, tr.vecEndPos);
				}
				if (m_beamImpactSprite.killTime && m_beamImpactSprite.killTime < gpGlobals->time) {
					m_beamImpactSprite.Kill();
				}
			}
		}
#endif
	}
}

bool CWeaponCustom::KillBeams(int attackIdx) {
	bool anyKilled = false;

	for (int i = 0; i < MAX_WC_BEAMS; i++) {
		if (m_beams[i].isFree())
			continue;

		if (attackIdx == -1 || (attackIdx == m_beams[i].attackIdx)) {
#ifdef CLIENT_DLL
			m_beams[i].pBeam->die = 0.0f;
#else
			UTIL_Remove(m_beams[i].h_beam);
#endif
			memset(&m_beams[i], 0, sizeof(WcBeam));
			anyKilled = true;

			if (m_beamImpactSprite.IsAlive() && i == m_beamImpactSprite.beamId) {
				m_beamImpactSprite.Kill();
			}
		}
	}

	if (attackIdx == -1) {
		m_beamImpactSprite.Kill();
	}

	return anyKilled;
}

Vector CWeaponCustom::BeamAttack(WcBeam& beam, CBasePlayer* m_pPlayer) {
	Vector spread(SPREAD_TO_FLOAT(beam.evt.beam.spreadX), SPREAD_TO_FLOAT(beam.evt.beam.spreadY), 0);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	bool isPredicted = IsPredicted();
	BULLET_PREDICTION predFlag = isPredicted ? BULLETPRED_EVENTLESS : BULLETPRED_NONE;

	float damage = beam.evt.beam.damage * GetChargeMult(beam.evt, FL_WC_CHARGE_DAMAGE);

	lagcomp_begin(m_pPlayer);
	Vector vecDir = m_pPlayer->FireBulletsPlayer(1, vecSrc, vecAiming, spread, beam.evt.beam.distance,
		BULLET_BEAM, 0, damage, m_pPlayer->pev, m_pPlayer->random_seed, g_traces, predFlag);
	lagcomp_end();

	beam.spreadX = vecDir.x;
	beam.spreadY = vecDir.y;

#ifdef CLIENT_DLL
	vecSrc = WC_GetGunPosition();
	Vector vecEnd = vecSrc + WC_GetAim(beam.spreadX, beam.spreadY) * beam.evt.beam.distance;
	pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);
	Vector endPos = tr.endpos;
	WcTrace evTrace = ConvertTrace(tr);
#else
	Vector endPos = g_traces[0].vecEndPos;
	WcTrace evTrace = ConvertTrace(g_traces[0]);
#endif

	int attackIdx = GetAttackIdx(beam.evt);
	ProcessEvents(WC_TRIG_IMPACT, GetImpactArg(attackIdx, true, true), false, false, 0, &evTrace);
	AttackTrace(m_pPlayer, attackIdx, vecSrc, g_traces[0]);

	// make splashes if this isn't an infinite beam
	if (beam.evt.beam.life != 0) {
		float splashSize = 0.3f;
		if (damage > 50) {
			splashSize = 0.5f;
		}
		else if (damage > 8) {
			splashSize = 0.4f;
		}
		edict_t* skipEnt = m_pPlayer->IsSevenKewpClient() ? m_pPlayer->edict() : NULL;
		UTIL_WaterSplashTrace(vecSrc, endPos, splashSize, 2, skipEnt);
	}

	beam.attackIdx = beam.evt.beam.life ? -1 : attackIdx;
	beam.nextAttack = !beam.evt.beam.life ? WallTime() + beam.evt.beam.freq * 0.001f : 0;

	return endPos;
}

void CWeaponCustom::QuakeMuzzleFlash(CBasePlayer* plr) {
#ifdef CLIENT_DLL
	EV_MuzzleFlash();
#else
	plr->pev->effects |= EF_MUZZLEFLASH;
#endif
}


void CWeaponCustom::SendPredictionData(edict_t* target, PredictionDataSendMode sendMode) {
#ifndef CLIENT_DLL
	if (params.flags & FL_WC_WEP_NO_PREDICTION) {
		return;
	}

	if (HasPredictionData(target) && sendMode == WC_PRED_SEND_INIT) {
		//ALERT(at_console, "PLayer already has the prediction data\n");
		return;
	}

	if (sendMode != WC_PRED_SEND_EVT) {
		MESSAGE_BEGIN(MSG_ONE, gmsgCustomWeapon, NULL, target);
		WRITE_BYTE(m_iId);
		WRITE_LONG(params.flags);
		WRITE_SHORT(params.maxClip);

		WRITE_SHORT(params.vmodel);
		WRITE_BYTE(params.deployAnim);
		WRITE_SHORT(params.deployTime);
		WRITE_SHORT(params.deployAnimTime);

		for (int k = 0; k < 3; k++) {
			WeaponCustomReload& reload = params.reloadStage[k];
			WRITE_BYTE(reload.anim);
			WRITE_SHORT(reload.time);

			if (k == 2 && !(params.flags & FL_WC_WEP_SHOTGUN_RELOAD))
				break;
		}

		for (int k = 0; k < 4; k++) {
			WeaponCustomIdle& idle = params.idles[k];
			WRITE_BYTE(idle.anim);
			WRITE_BYTE(idle.weight);
			WRITE_SHORT(idle.time);
		}

		if (params.flags & FL_WC_WEP_AKIMBO) {
			for (int k = 0; k < 4; k++) {
				WeaponCustomIdle& idle = params.akimbo.idles[k];
				WRITE_BYTE(idle.anim);
				WRITE_BYTE(idle.weight);
				WRITE_SHORT(idle.time);
			}

			WRITE_BYTE(params.akimbo.reload.anim);
			WRITE_SHORT(params.akimbo.reload.time);

			WRITE_BYTE(params.akimbo.deployAnim);
			WRITE_SHORT(params.akimbo.deployTime);
			WRITE_BYTE(params.akimbo.akimboDeployAnim);
			WRITE_SHORT(params.akimbo.akimboDeployTime);
			WRITE_SHORT(params.akimbo.akimboDeployAnimTime);
			WRITE_BYTE(params.akimbo.holsterAnim);
			WRITE_SHORT(params.akimbo.holsterTime);
			WRITE_SHORT(params.akimbo.accuracyX);
			WRITE_SHORT(params.akimbo.accuracyY);
		}

		if (params.flags & FL_WC_WEP_HAS_LASER) {
			for (int k = 0; k < 4; k++) {
				WeaponCustomIdle& idle = params.laser.idles[k];
				WRITE_BYTE(idle.anim);
				WRITE_BYTE(idle.weight);
				WRITE_SHORT(idle.time);
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
			WRITE_BYTE(opts.flags);
			WRITE_BYTE(opts.ammoCost);
			WRITE_BYTE(opts.ammoFreq);
			WRITE_BYTE(opts.ammoPool);
			WRITE_SHORT(opts.cooldown);
			WRITE_SHORT(opts.cooldownFail);
			WRITE_SHORT(opts.accuracyX);
			WRITE_SHORT(opts.accuracyY);
			WRITE_SHORT(opts.emptySound);

			WRITE_BYTE((opts.chargeMode << 4) | (opts.chargeAmmoMode << 2) | opts.overchargeMode);
			if (opts.chargeMode != WC_CHARGEUP_NONE) {
				WRITE_BYTE(opts.chargeFlags);
				WRITE_SHORT(opts.chargeTime);
				WRITE_SHORT(opts.overchargeTime);
				WRITE_SHORT(opts.chargeCancelTime);
			}
		}
		MESSAGE_END();
	}

	int mainBytes = LastMsgSize();

	if (sendMode != WC_PRED_SEND_WEP) {
		MESSAGE_BEGIN(MSG_ONE, gmsgCustomWeaponEvents, NULL, target);
		WRITE_BYTE(m_iId);
		WRITE_BYTE(params.numEvents);

		for (int k = 0; k < params.numEvents; k++) {
			WepEvt& evt = params.events[k];
			uint16_t packedHeader = (evt.hasDelay << 15) | (evt.triggerArg << 10) | (evt.trigger << 5) | evt.evtType;
			WRITE_SHORT(packedHeader);
			if (evt.hasDelay) {
				WRITE_SHORT(evt.delay);
			}

			switch (evt.evtType) {
			case WC_EVT_IDLE_SOUND: {
				uint16_t packedFlags = (evt.idleSound.sound << 7) | evt.idleSound.volume;
				WRITE_SHORT(packedFlags);
				break;
			}
			case WC_EVT_PLAY_SOUND: {
				uint16_t packedFlags = evt.playSound.sound << 7 | evt.playSound.channel << 4
					| (evt.playSound.aiVol << 2) | evt.playSound.flags;
				WRITE_SHORT(packedFlags);
				WRITE_BYTE(evt.playSound.volume);
				WRITE_BYTE(evt.playSound.attn);
				WRITE_BYTE(evt.playSound.pitchMin);
				WRITE_BYTE(evt.playSound.pitchMax);

				WRITE_BYTE(evt.playSound.numAdditionalSounds);
				for (int i = 0; i < evt.playSound.numAdditionalSounds; i++) {
					WRITE_SHORT(evt.playSound.additionalSounds[i]);
				}

				//WRITE_BYTE(evt.playSound.distantSound); // not needed for prediction
				break;
			}
			case WC_EVT_EJECT_SHELL: {
				uint16_t packedFlags = (evt.ejectShell.model << 4) | (evt.ejectShell.sound << 2)
					| (evt.ejectShell.hasVel << 1) | evt.ejectShell.hasRand;
				WRITE_SHORT(packedFlags);
				WRITE_BYTE(evt.ejectShell.offsetForward);
				WRITE_BYTE(evt.ejectShell.offsetUp);
				WRITE_BYTE(evt.ejectShell.offsetRight);
				if (evt.ejectShell.hasVel) {
					WRITE_BYTE(evt.ejectShell.velForward);
					WRITE_BYTE(evt.ejectShell.velUp);
					WRITE_BYTE(evt.ejectShell.velRight);
				}
				if (evt.ejectShell.hasRand) {
					WRITE_BYTE(evt.ejectShell.dirRand);
					WRITE_BYTE(evt.ejectShell.speedRand);
				}
				break;
			}
			case WC_EVT_PUNCH:
				WRITE_BYTE(evt.punch.flags);
				WRITE_SHORT(evt.punch.x);
				WRITE_SHORT(evt.punch.y);
				WRITE_SHORT(evt.punch.z);
				break;
			case WC_EVT_SET_BODY:
				WRITE_BYTE(evt.setBody.newBody);
				break;
			case WC_EVT_WEP_ANIM: {
				uint8_t packedAnimHeader = (evt.anim.flags << 3) | (evt.anim.akimbo);
				WRITE_BYTE(packedAnimHeader);
				WRITE_BYTE(evt.anim.numAnim);
				for (int i = 0; i < evt.anim.numAnim; i++) {
					WRITE_BYTE(evt.anim.anims[i]);
				}
				break;
			}
			case WC_EVT_BULLETS: {
				WRITE_BYTE(evt.bullets.count);
				WRITE_SHORT(evt.bullets.burstDelay);
				WRITE_SHORT(evt.bullets.damage);
				WRITE_SHORT(evt.bullets.spreadX);
				WRITE_SHORT(evt.bullets.spreadY);
				WRITE_BYTE((evt.bullets.tracerFreq << 4) | evt.bullets.tracerColor);

				uint8_t packedFlags = (evt.bullets.flags << 4) | evt.bullets.flashSz;
				WRITE_BYTE(packedFlags);
				break;
			}
			case WC_EVT_BEAM: {
				uint16_t packedFlags = (evt.beam.sprite << 7) | (evt.beam.attachment << 4) | evt.beam.flags;
				WRITE_SHORT(packedFlags);
				WRITE_SHORT(evt.beam.life);
				WRITE_SHORT(evt.beam.spreadX);
				WRITE_SHORT(evt.beam.spreadY);
				WRITE_SHORT(evt.beam.damage);
				WRITE_SHORT(evt.beam.distance);
				WRITE_SHORT(evt.beam.freq);
				WRITE_BYTE((evt.beam.id << 4) | (evt.beam.altMode << 1) | evt.beam.hasImpactSprite);
				WRITE_BYTE(evt.beam.width);
				WRITE_BYTE(evt.beam.noise);
				WRITE_BYTE(evt.beam.scrollRate);
				WRITE_BYTE(evt.beam.color.r);
				WRITE_BYTE(evt.beam.color.g);
				WRITE_BYTE(evt.beam.color.b);
				WRITE_BYTE(evt.beam.color.a);

				if (evt.beam.altMode != WC_BEAM_ANIM_DISABLED) {
					WRITE_SHORT(evt.beam.altTime);
					WRITE_BYTE(evt.beam.widthAlt);
					WRITE_BYTE(evt.beam.noiseAlt);
					WRITE_BYTE(evt.beam.scrollRateAlt);
					WRITE_BYTE(evt.beam.colorAlt.r);
					WRITE_BYTE(evt.beam.colorAlt.g);
					WRITE_BYTE(evt.beam.colorAlt.b);
					WRITE_BYTE(evt.beam.colorAlt.a);
				}

				if (evt.beam.hasImpactSprite) {
					uint16_t packed = (evt.beam.impactSprite << 7) | evt.beam.impactSpriteFps;
					WRITE_SHORT(packed);
					WRITE_BYTE(evt.beam.impactSpriteScale);
					WRITE_BYTE(evt.beam.impactSpriteColor.r);
					WRITE_BYTE(evt.beam.impactSpriteColor.g);
					WRITE_BYTE(evt.beam.impactSpriteColor.b);
					WRITE_BYTE(evt.beam.impactSpriteColor.a);
				}
				break;
			}
			case WC_EVT_KICKBACK:
				WRITE_SHORT(evt.kickback.pushForce);
				WRITE_BYTE(evt.kickback.back);
				WRITE_BYTE(evt.kickback.right);
				WRITE_BYTE(evt.kickback.up);
				WRITE_BYTE(evt.kickback.globalUp);
				break;
			case WC_EVT_TOGGLE_STATE: {
				uint16_t packedFlags = (evt.toggleState.stateBits << 2) | evt.toggleState.toggleMode;
				WRITE_SHORT(packedFlags);
				break;
			}
			case WC_EVT_TOGGLE_ZOOM:
				WRITE_BYTE(evt.zoomToggle.zoomFov);
				WRITE_BYTE(evt.zoomToggle.zoomFov2);
				break;
			case WC_EVT_HIDE_LASER:
				WRITE_SHORT(evt.laserHide.millis);
				break;
			case WC_EVT_COOLDOWN:
				WRITE_SHORT(evt.cooldown.millis);
				WRITE_BYTE(evt.cooldown.targets);
				break;
			case WC_EVT_SET_GRAVITY:
				WRITE_SHORT(evt.setGravity.gravity);
				break;
			case WC_EVT_DLIGHT:
				WRITE_BYTE(evt.dlight.r);
				WRITE_BYTE(evt.dlight.g);
				WRITE_BYTE(evt.dlight.b);
				WRITE_BYTE(evt.dlight.radius);
				WRITE_BYTE(evt.dlight.life);
				WRITE_BYTE(evt.dlight.decayRate);
				break;
			case WC_EVT_MUZZLEFLASH:
				WRITE_BYTE(evt.muzzleFlash.brightness);
				break;
			case WC_EVT_SPRITETRAIL:
				WRITE_SHORT(evt.spriteTrail.sprite);
				WRITE_BYTE(evt.spriteTrail.count);
				WRITE_BYTE(evt.spriteTrail.scale);
				WRITE_BYTE(evt.spriteTrail.speed);
				WRITE_BYTE(evt.spriteTrail.speedNoise);
				break;
			case WC_EVT_DECAL:
				WRITE_BYTE(evt.decal.decalIdx);
				WRITE_BYTE(evt.decal.isGunshot);
			case WC_EVT_PROJECTILE:
			case WC_EVT_SERVER:
				break;
			default:
				ALERT(at_error, "Invalid custom weapon event type %d\n", evt.evtType);
				break;
			}
		}
		MESSAGE_END();
	}

	int evBytes = LastMsgSize();
	ALERT(at_console, "Sent %d prediction bytes for %s (%d + %d evt)\n",
		evBytes + mainBytes, STRING(pev->classname), mainBytes, evBytes);

	m_predDataSent[m_iId] |= PLRBIT(target);
#endif
}


void CWeaponCustom::FireAmmoEvents(int ammoPool) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;
	if (!g_runfuncs)
		return;

	switch (ammoPool) {
	case WC_AMMOPOOL_PRIMARY_CLIP:
		ProcessEvents(WC_TRIG_PRIMARY_CLIP_SP, m_bulletFireCount++, IsAkimbo(), false, m_iClip);
		ProcessEvents(WC_TRIG_PRIMARY_CLIPSIZE, m_iClip, IsAkimbo(), false, m_iClip);
		break;
	case WC_AMMOPOOL_PRIMARY_RESERVE: break;
	case WC_AMMOPOOL_SECONDARY_RESERVE: break;
	default: break;
	}
}


void CWeaponCustom::ProcessEvents(int trigger, int triggerArg, bool leftHand, bool akimboFire, int clipLeft, WcTrace* tr) {
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
		case WC_TRIG_RELOAD:
		case WC_TRIG_RELOAD_EMPTY:
		case WC_TRIG_RELOAD_NOT_EMPTY:
		case WC_TRIG_DEPLOY:
			argMatch = evt.triggerArg == WC_TRIG_SHOOT_ARG_ALWAYS || triggerArg == evt.triggerArg;
			break;
		case WC_TRIG_PRIMARY_CLIPSIZE:
		case WC_TRIG_IMPACT:
			argMatch = triggerArg == evt.triggerArg;
			break;
		case WC_TRIG_PRIMARY_CLIP_SP:
		case WC_TRIG_SECONDARY_CLIP_SP:
			switch (evt.triggerArg) {
			case WC_TRIG_CLIP_ARG_ODD:
				argMatch = triggerArg % 2 != 0;
				break;
			case WC_TRIG_CLIP_ARG_EVEN:
				argMatch = triggerArg % 2 == 0;
				break;
			case WC_TRIG_CLIP_ARG_NOT_EMPTY:
				argMatch = clipLeft > 0;
				break;
			}
			break;
		default:
			break;
		}

		if (!argMatch)
			continue;

		if (evt.delay == 0) {
			PlayEvent(i, leftHand, akimboFire, tr);

			if (evt.evtType == WC_EVT_BULLETS && evt.bullets.burstDelay) {
				float burstDelay = 0;
				int additionalBullets = evt.bullets.count - 1;
				if (clipLeft < 0) // clip went negative due to exceeding cost for a burst
					additionalBullets += clipLeft;
				for (int k = 0; k < additionalBullets; k++) {
					burstDelay += evt.bullets.burstDelay * 0.001f;
					QueueDelayedEvent(i, WallTime() + burstDelay, leftHand, akimboFire, tr);
				}
			}
		}
		else {
			QueueDelayedEvent(i, WallTime() + evt.delay * 0.001f, leftHand, akimboFire, tr);
		}
	}
}

void CWeaponCustom::QueueDelayedEvent(int eventIdx, float fireTime, bool leftHand, bool akimboFire, WcTrace* tr) {
	for (int i = 0; i < WC_SERVER_EVENT_QUEUE_SZ; i++) {
		WcDelayEvent& qevt = eventQueue[i];

		// find an empty slot
		if (qevt.fireTime == 0) {
			qevt.eventIdx = eventIdx;
			qevt.fireTime = fireTime;
			qevt.leftHand = leftHand;
			qevt.akimboFire = akimboFire;
			
			if (tr) {
				qevt.tr = *tr;
			} else {
				memset(&qevt.tr, 0, sizeof(WcTrace));
			}
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
		spread = spread * GetCurrentAccuracyMultiplier();
	}

	if (evt.bullets.flashSz) {
		WepEvt flash = WepEvt();
		flash.muzzleFlash.brightness = evt.bullets.flashSz;
		PlayEvent_MuzzleFlash(flash, m_pPlayer);
	}

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	bool isPredicted = IsPredicted();
	BULLET_PREDICTION predFlag = isPredicted ? BULLETPRED_EVENTLESS : BULLETPRED_NONE;

	float damage = evt.bullets.damage * GetChargeMult(evt, FL_WC_CHARGE_DAMAGE);

	lagcomp_begin(m_pPlayer);
	Vector vecDir = m_pPlayer->FireBulletsPlayer(evt.bullets.count, vecSrc, vecAiming, spread, 8192,
		BULLET_PLAYER_9MM, evt.bullets.tracerFreq, evt.bullets.damage, m_pPlayer->pev,
		m_pPlayer->random_seed, g_traces, predFlag);
	lagcomp_end();

	int attackIdx = GetAttackIdx(evt);

#ifdef CLIENT_DLL
	int eidx = 0;
#else
	int eidx = m_pPlayer->entindex() - 1;
#endif

	bool showTracer = CheckTracer(eidx, vecSrc, vecDir, gpGlobals->v_right, evt.bullets.tracerFreq);

#ifdef CLIENT_DLL
	bool decal = !(evt.bullets.flags & FL_WC_BULLETS_NO_DECAL);
	bool texSound = !(evt.bullets.flags & FL_WC_BULLETS_NO_SOUND);
	
	for (ULONG iShot = 1; iShot <= evt.bullets.count; iShot++)
	{
		//Use player's random seed.
		// get circular gaussian spread
		int r = m_pPlayer->random_seed;
		float x = UTIL_SharedRandomFloat(r + iShot, -0.5, 0.5) + UTIL_SharedRandomFloat(r + (1 + iShot), -0.5, 0.5);
		float y = UTIL_SharedRandomFloat(r + (2 + iShot), -0.5, 0.5) + UTIL_SharedRandomFloat(r + (3 + iShot), -0.5, 0.5);
		float z = x * x + y * y;

		bool playTexSound = texSound && iShot < 6; // don't stack too many sounds
		pmtrace_t tr = WC_EV_FireBullets(x * spread.x, y * spread.y, showTracer, evt.bullets.tracerColor,
			decal, playTexSound, iShot, evt.bullets.damage);
		
		WcTrace evTrace = ConvertTrace(tr);
		ProcessEvents(WC_TRIG_IMPACT, GetImpactArg(attackIdx, true, true), false, false, 0, &evTrace);
	}

	if (evt.bullets.flashSz)
		EV_MuzzleFlash();
#else
	
	for (int i = 0; i < evt.bullets.count; i++) {
		WcTrace evTrace = ConvertTrace(g_traces[i]);
		ProcessEvents(WC_TRIG_IMPACT, GetImpactArg(attackIdx, true, true), false, false, 0, &evTrace);
		AttackTrace(m_pPlayer, attackIdx, vecSrc, g_traces[i]);
	}

	if (showTracer) {
		for (int i = 1; i < gpGlobals->time; i++) {
			CBasePlayer* listener = UTIL_PlayerByIndex(i);

			if (!listener) {
				continue;
			}

			if ((m_pPlayer != listener || !isPredicted) && m_pPlayer->InPAS(listener->edict())) {
				for (int k = 0; k < evt.bullets.count; k++) {
					UTIL_Tracer(vecSrc, g_traces[k].vecEndPos, evt.bullets.tracerColor, MSG_ONE_UNRELIABLE, listener->edict());
				}
			}
		}
	}
#endif

	int akimboArg = IsAkimbo() ? WC_TRIG_SHOOT_ARG_AKIMBO : WC_TRIG_SHOOT_ARG_NOT_AKIMBO;
	ProcessEvents(WC_TRIG_BULLET_FIRED, akimboArg, leftHand, akimboFire);
}

void CWeaponCustom::PlayEvent_Beam(WepEvt& evt, CBasePlayer* m_pPlayer) {
	WcBeam* wcbeam;
	if (evt.beam.id == 0)
		wcbeam = AllocBeam();
	else
		wcbeam = &m_beams[V_min(MAX_WC_BEAMS - 1, evt.beam.id)];

	if (!wcbeam)
		return;

	wcbeam->evt = evt;

	bool newBeam = evt.beam.id == 0 || wcbeam->isFree();

	Vector endPos;
	if (newBeam) {
		endPos = BeamAttack(*wcbeam, m_pPlayer);
		wcbeam->creationTime = gpGlobals->time;
		if (evt.beam.hasImpactSprite)
			m_beamImpactSprite.Kill();
	}
	else {
		if (evt.beam.hasImpactSprite)
			m_beamImpactSprite.killTime = evt.beam.life ? gpGlobals->time + evt.beam.life : 0;
	}

#ifdef CLIENT_DLL
	float life = evt.beam.life ? evt.beam.life * 0.001f : 99999;

	if (newBeam) {
		// create new beam
		cl_entity_t* player = WC_GetPlayer();
		int idx = player->index;

		float r = (evt.beam.color.r / 255.0f);
		float g = (evt.beam.color.g / 255.0f);
		float b = (evt.beam.color.b / 255.0f);
		float a = (evt.beam.color.a / 255.0f);

		BEAM* beam = gEngfuncs.pEfxAPI->R_BeamEntPoint(idx | (0x1000 * evt.beam.attachment),
			endPos, evt.beam.sprite, life, evt.beam.width / 10.0f, evt.beam.noise / 100.0f, a, evt.beam.scrollRate,
			0, 0, r, g, b);

		if (evt.beam.flags & FL_WC_BEAM_SPIRAL) beam->flags |= BEAM_FSINE;
		if (evt.beam.flags & FL_WC_BEAM_OPAQUE) beam->flags |= BEAM_FSOLID;
		if (evt.beam.flags & FL_WC_BEAM_SHADEIN) beam->flags |= BEAM_FSHADEIN;
		if (evt.beam.flags & FL_WC_BEAM_SHADEOUT) beam->flags |= BEAM_FSHADEOUT;
		
		wcbeam->pBeam = beam;
		wcbeam->pBeam->die = gEngfuncs.GetClientTime() + life;
		wcbeam->pBeam->target = endPos;

		if (evt.beam.hasImpactSprite) {
			RGBA c = evt.beam.impactSpriteColor;

			TEMPENTITY* pFlare = gEngfuncs.pEfxAPI->R_TempSprite(endPos, Vector(0,0,0),
				evt.beam.impactSpriteScale*0.1f, evt.beam.impactSprite, kRenderGlow,
				kRenderFxNoDissipation, c.a / 255.0f, 99999, FTENT_SPRCYCLE | FTENT_PERSIST);

			m_beamImpactSprite.creationTime = gpGlobals->time;
			m_beamImpactSprite.pSprite = pFlare;
			m_beamImpactSprite.beamId = evt.beam.id ? evt.beam.id : -1;
			m_beamImpactSprite.killTime = evt.beam.life ? gpGlobals->time + evt.beam.life : 0;
		}
	}
	else {
		wcbeam->pBeam->die = gEngfuncs.GetClientTime() + life;
	}
#else
	if (newBeam) {
		// create new beam
		CBeam* beam = CBeam::BeamCreate(INDEX_MODEL(evt.beam.sprite), evt.beam.width);
		beam->PointEntInit(endPos, m_pPlayer->entindex());
		beam->SetEndAttachment(evt.beam.attachment);
		beam->SetNoise(evt.beam.noise);
		beam->SetColor(evt.beam.color.r, evt.beam.color.g, evt.beam.color.b);
		beam->SetBrightness(evt.beam.color.a);
		beam->SetScrollRate(evt.beam.scrollRate);

		int flags = 0;
		if (evt.beam.flags & FL_WC_BEAM_SPIRAL) flags |= BEAM_FSINE;
		if (evt.beam.flags & FL_WC_BEAM_OPAQUE) flags |= BEAM_FSOLID;
		if (evt.beam.flags & FL_WC_BEAM_SHADEIN) flags |= BEAM_FSHADEIN;
		if (evt.beam.flags & FL_WC_BEAM_SHADEOUT) flags |= BEAM_FSHADEOUT;
		beam->SetFlags(flags);

		if (evt.beam.life)
			beam->LiveForTime(evt.beam.life * 0.001f);
		beam->m_hidePlayers = PLRBIT(m_pPlayer->edict());
		wcbeam->h_beam = beam;

		if (evt.beam.hasImpactSprite) {
			CSprite* spr = CSprite::SpriteCreate(INDEX_MODEL(evt.beam.impactSprite), endPos, TRUE);
			RGBA c = evt.beam.impactSpriteColor;
			spr->SetColor(c.r, c.g, c.b);
			spr->SetBrightness(c.a);
			spr->SetScale(evt.beam.impactSpriteScale);
			spr->pev->rendermode = kRenderGlow;
			spr->pev->renderfx = kRenderFxNoDissipation;
			spr->pev->framerate = evt.beam.impactSpriteFps;
			spr->m_hidePlayers = PLRBIT(m_pPlayer->edict());

			m_beamImpactSprite.creationTime = gpGlobals->time;
			m_beamImpactSprite.beamId = evt.beam.id ? evt.beam.id : -1;
			m_beamImpactSprite.killTime = evt.beam.life ? gpGlobals->time + evt.beam.life : 0;
			m_beamImpactSprite.h_sprite = spr;
		}
	}
	else {
		// update existing beam
		CBeam* beam = (CBeam*)wcbeam->h_beam.GetEntity();
		
		if (evt.beam.life)
			beam->LiveForTime(evt.beam.life * 0.1f);
		else {
			beam->SetThink(NULL);
			beam->pev->nextthink = 0;
		}
	}
#endif
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
		dir.z * evt.proj.speed * vecDir;

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
		shootEnt = CCrossbowBolt::BoltCreate();
		shootEnt->pev->origin = projectile_ori;
		shootEnt->pev->angles = m_pPlayer->pev->v_angle;
		shootEnt->pev->owner = m_pPlayer->edict();

		if (m_pPlayer->pev->waterlevel == 3)
		{
			shootEnt->pev->velocity = vecDir * BOLT_WATER_VELOCITY;
			shootEnt->pev->speed = BOLT_WATER_VELOCITY;
		}
		else
		{
			shootEnt->pev->velocity = vecDir * BOLT_AIR_VELOCITY;
			shootEnt->pev->speed = BOLT_AIR_VELOCITY;
		}
		shootEnt->pev->avelocity.z = 10;
		break;
	case WC_PROJECTILE_HVR:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_HVR Not implemented\n");
		break;
	case WC_PROJECTILE_SHOCK:
		ALERT(at_error, "WeaponCustom: WC_PROJECTILE_SHOCK Not implemented\n");
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
	case WC_PROJECTILE_CUSTOM: {
		const char* clazz = evt.proj.entity_class ? STRING(evt.proj.entity_class) : "custom_projectile";
		shootEnt = CBaseEntity::Create(clazz, projectile_ori, projectile_dir_angles, false);
		shootEnt->pev->velocity = projectile_velocity;
		shootEnt->pev->owner = m_pPlayer->edict();

		CProjectileCustom* cproj = shootEnt->MyProjectileCustomPtr();
		if (cproj) {
			cproj->world_event = (WeaponCustomProjectileAction)evt.proj.world_event;
			cproj->monster_event = (WeaponCustomProjectileAction)evt.proj.monster_event;
			cproj->air_friction = evt.proj.air_friction;
			cproj->water_friction = evt.proj.water_friction;
			cproj->damage = evt.proj.damage * GetChargeMult(evt, FL_WC_CHARGE_DAMAGE);
			cproj->damageType = evt.proj.damageBits;
			cproj->expire_time = evt.proj.life ? gpGlobals->time + evt.proj.life : 0;
			cproj->pev->movetype = evt.proj.gravity != 0 ? MOVETYPE_BOUNCE : MOVETYPE_BOUNCEMISSILE;
			cproj->flags = evt.proj.flags;
			cproj->move_snd = evt.proj.move_snd;

			if (!evt.proj.model) {
				cproj->pev->rendermode = 1; // don't render the no-precache model
			}

			float size = evt.proj.size;
			cproj->pev->mins = Vector(-size, -size, -size);
			cproj->pev->maxs = Vector(size, size, size);

			cproj->Configure(m_pPlayer, this, evt);
		}

		edict_t* ed = shootEnt->edict();
		DispatchSpawnGame(ed);
		shootEnt = CBaseEntity::Instance(ed);
		break;
	}
	case WC_PROJECTILE_OTHER: {
		shootEnt = CBaseEntity::Create(STRING(evt.proj.entity_class), projectile_ori, projectile_dir_angles, false);
		shootEnt->pev->velocity = projectile_velocity;
		shootEnt->pev->owner = m_pPlayer->edict();
		edict_t* ed = shootEnt->edict();
		DispatchSpawnGame(ed);
		shootEnt = CBaseEntity::Instance(ed);
		break;
	}
	default:
		ALERT(at_error, "WeaponCustom: Unknown projectile type %d\n", evt.proj.type);
		break;
	}

	
	if (shootEnt)
	{
		AddWaterPhysicsEnt(shootEnt, 1, 0);

		if (evt.proj.follow_mode != WC_PROJ_FOLLOW_NONE)
		{
			//EHANDLE h_plr = m_pPlayer->edict();
			//EHANDLE h_proj = shootEnt->edict();
			//float dur = evt.proj.follow_time[1];
			ALERT(at_error, "WeaponCustom: Projectile follow mode not implemented\n");
			//g_Scheduler.SetTimeout("projectile_follow_aim", evt.proj.follow_time[0], h_plr, h_proj, @state.active_opts, dur);
		}

		if (evt.proj.model) {
			SET_MODEL(shootEnt->edict(), INDEX_MODEL(evt.proj.model));
		}

		//EHANDLE mdlHandle = shootEnt->edict();
		EHANDLE sprHandle;

		CProjectileCustom* cproj = shootEnt->MyProjectileCustomPtr();

		// TODO: Kill this when follow target dies (and its not a custom entity)
		if (evt.proj.sprite)
		{
			if (cproj) {
				Vector ori = shootEnt->pev->origin;
				RGBA c = evt.proj.sprite_color;
				CBaseEntity* spr = CBaseEntity::Create("env_sprite", ori, g_vecZero, false);
				spr->pev->model = evt.proj.sprite;
				spr->pev->rendermode = 5;
				spr->pev->renderamt = c.a;
				spr->pev->rendercolor = c.ToVector();
				spr->pev->scale = evt.proj.sprite_scale;
				spr->pev->movetype = MOVETYPE_FOLLOW;
				spr->pev->aiment = shootEnt->edict();
				spr->pev->skin = shootEnt->entindex();
				spr->pev->body = 0; // attachement point
				edict_t* ed = spr->edict();
				DispatchSpawnGame(ed);
				cproj->spriteAttachment = sprHandle = ed;
			}
			else {
				ALERT(at_error, "Projectile sprite not implemented for non-custom projectiles\n");
			}
			
		}

		// attach a trail
		if (evt.proj.trail_spr) {
			UTIL_BeamFollow(shootEnt->entindex(), MODEL_INDEX(STRING(evt.proj.trail_spr)), evt.proj.trail_life,
				evt.proj.trail_width, evt.proj.trail_color);
		}

		if (evt.proj.life && evt.proj.type != WC_PROJECTILE_CUSTOM)
			ALERT(at_error, "WeaponCustom: Non-custom projectile life not implemented\n");

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
		shootEnt->pev->angles = shootEnt->pev->angles + (Vector)evt.proj.angles;

		if (!shootEnt->pev->gravity && shootEnt->pev->movetype == MOVETYPE_BOUNCE) {
			shootEnt->pev->gravity = FLT_MIN;
		}
	}

	// remove weapon from player if they threw it
	if (evt.proj.type == WC_PROJECTILE_WEAPON) {
		UTIL_Remove(this);
	}
#endif
}

void CWeaponCustom::PlayEvent_Kickback(WepEvt& evt, CBasePlayer* m_pPlayer) {
	float force = evt.kickback.pushForce * GetChargeMult(evt, FL_WC_CHARGE_KICKBACK);
	float backForce = (evt.kickback.back / 100.0f) * force;
	float rightForce = (evt.kickback.right / 100.0f) * force;
	float upForce = (evt.kickback.up / 100.0f) * force;
	float globalUpForce = (evt.kickback.globalUp / 100.0f) * force;
	static Vector globalUp = Vector(0, 0, 1);

#ifdef CLIENT_DLL
	Vector forward, right, up;
	gEngfuncs.pfnAngleVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle, forward, right, up);
	m_runningKickbackPred = 1;
	m_kickbackPredVel = (forward * -backForce) + (right * rightForce) + (up * upForce) + (globalUp * globalUpForce);
#else
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	m_pPlayer->pev->velocity = m_pPlayer->pev->velocity + 
		gpGlobals->v_forward * -backForce +
		gpGlobals->v_right * rightForce +
		gpGlobals->v_up * upForce +
		globalUp * globalUpForce;
#endif
}

void CWeaponCustom::PlayEvent_SetGravity(WepEvt& evt, CBasePlayer* m_pPlayer) {
	// TODO: update monster effects code to use this, like with movespeed
	m_pPlayer->pev->gravity = evt.setGravity.gravity / 1000.0f;
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

		if (evt.playSound.flags & FL_WC_SOUND_CHARGE_PITCH)
			pitch = evt.playSound.pitchMin;

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

	WC_EV_LocalSound(idx, channel, pitch, volume, attn, panning, 0);
#else
	
	if (IsPredicted()) {
		uint32_t messageTargets = 0xffffffff & ~PLRBIT(m_pPlayer->edict());
		StartSound(m_pPlayer->edict(), channel, INDEX_SOUND(idx), volume, attn,
			SND_FL_PREDICTED, pitch, m_pPlayer->pev->origin, messageTargets);
	}
	else {
		StartSound(m_pPlayer->edict(), channel, INDEX_SOUND(idx), volume, attn,
			0, pitch, m_pPlayer->pev->origin, 0xffffffff);
	}	

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

	if (evt.ejectShell.hasVel) {
		Vector newForward = evt.ejectShell.velForward * forward;
		Vector newUp = evt.ejectShell.velUp * up;
		Vector newRight = evt.ejectShell.velRight * right;
		Vector svel = newForward + newUp + newRight;

		float speedMult = 5;

		if (evt.ejectShell.hasRand) {
			Vector dir = svel.Normalize();
			int r = evt.ejectShell.dirRand;
			dir.x += RANDOM_FLOAT(-r, r) * 0.01f;
			dir.y += RANDOM_FLOAT(-r, r) * 0.01f;
			dir.z += RANDOM_FLOAT(-r, r) * 0.01f;
			speedMult += RANDOM_FLOAT(0, evt.ejectShell.speedRand);
			ShellVelocity = vel + dir * svel.Length() * speedMult;
		}
		else {
			ShellVelocity = vel + svel * speedMult;
		}
	}

	bool predicted = IsPredicted();

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (!plr || (plr == m_pPlayer && predicted))
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
	if (!m_pPlayer->IsSevenKewpClient()) {
		float punchAngleX = FP_10_6_TO_FLOAT(evt.punch.x);
		float punchAngleY = FP_10_6_TO_FLOAT(evt.punch.y);
		float punchAngleZ = FP_10_6_TO_FLOAT(evt.punch.z);

		if (!(evt.punch.flags & FL_WC_PUNCH_NO_RETURN)) {
			if (evt.punch.flags & FL_WC_PUNCH_ADD) {
				m_pPlayer->pev->punchangle = m_pPlayer->pev->punchangle + Vector(punchAngleX, punchAngleY, punchAngleZ);
			}
			else if (evt.punch.flags & FL_WC_PUNCH_SET) {
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
	}
#endif
}

void CWeaponCustom::PlayEvent_WepAnim(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand) {
	if (!evt.anim.numAnim)
		return;

	int idx = 0;

	if (evt.anim.flags & FL_WC_ANIM_ORDERED) {
		idx = (animCount++ % evt.anim.numAnim);
	}
	else {
		idx = UTIL_SharedRandomLong(m_pPlayer->random_seed, 0, evt.anim.numAnim - 1);
	}

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

void CWeaponCustom::PlayEvent_ToggleState(WepEvt& evt, CBasePlayer* m_pPlayer) {
	bool toggleOn = evt.toggleState.toggleMode == WC_TOGGLE_STATE_ON;
	bool toggleFlip = evt.toggleState.toggleMode == WC_TOGGLE_STATE_TOGGLE;

	if (evt.toggleState.stateBits & FL_WC_STATE_IS_AKIMBO) {
		SetAkimbo(toggleOn || (toggleFlip && !IsAkimbo()));

		if (IsAkimbo()) {
			SendAkimboAnim(params.akimbo.deployAnim);
		}
		else {
			Deploy();
		}
	}

	if (evt.toggleState.stateBits & FL_WC_STATE_LASER) {
		ToggleLaser(toggleOn || (toggleFlip && !IsLaserOn()));
		ProcessEvents(IsLaserOn() ? WC_TRIG_LASER_ON : WC_TRIG_LASER_OFF, 0);
	}

	if (evt.toggleState.stateBits & FL_WC_STATE_PRIMARY_ALT) {
		SetPrimaryAlt(toggleOn || (toggleFlip && !IsPrimaryAltActive()));
	}
}

void CWeaponCustom::PlayEvent_HideLaser(WepEvt& evt, CBasePlayer* m_pPlayer) {
	if (IsLaserOn()) {
		HideLaser(true);
		m_laserOnTime = WallTime() + evt.laserHide.millis * 0.001f;
	}
}

void CWeaponCustom::PlayEvent_DLight(WepEvt& evt, CBasePlayer* m_pPlayer) {
#ifdef CLIENT_DLL
	WC_EV_Dlight(evt);
#else
	bool isPredicted = IsPredicted();

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (!plr || (plr == m_pPlayer && isPredicted) || !m_pPlayer->InPVS(plr->edict()))
			continue;

		MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, plr->edict());
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD_VECTOR(m_pPlayer->pev->origin);
		WRITE_BYTE(evt.dlight.radius);
		WRITE_BYTE(evt.dlight.r);
		WRITE_BYTE(evt.dlight.g);
		WRITE_BYTE(evt.dlight.b);
		WRITE_BYTE(evt.dlight.life);
		WRITE_BYTE(evt.dlight.decayRate);
		MESSAGE_END();
	}
#endif
}

void CWeaponCustom::PlayEvent_MuzzleFlash(WepEvt& evt, CBasePlayer* m_pPlayer) {
	switch (evt.muzzleFlash.brightness) {
	case WC_FLASH_DIM:
		m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
		QuakeMuzzleFlash(m_pPlayer);
		break;
	case WC_FLASH_NORMAL:
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
		QuakeMuzzleFlash(m_pPlayer);
		break;
	case WC_FLASH_BRIGHT:
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		QuakeMuzzleFlash(m_pPlayer);
		break;
	default:
		break;
	}
}

void CWeaponCustom::PlayEvent_SpriteTrail(WepEvt& evt, CBasePlayer* m_pPlayer, WcTrace* tr) {

	Vector start, end;

	if (tr) {
		start = tr->vecEndPos;
		end = start + tr->vecPlaneNormal;
	}
	else {
#ifdef CLIENT_DLL
		start = WC_GetGunPosition();
		end = start + WC_GetAim(0, 0);
#else
		start = m_pPlayer->GetGunPosition();
		end = start + m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
#endif		
	}

#ifdef CLIENT_DLL
	gEngfuncs.pEfxAPI->R_Sprite_Trail(TE_SPRITETRAIL, start, end, evt.spriteTrail.sprite,
		evt.spriteTrail.count, 0.0, evt.spriteTrail.scale / 10.0f, evt.spriteTrail.speedNoise*20,
		255, evt.spriteTrail.speed*5);
#else
	bool isPredicted = IsPredicted();

	for (int i = 1; i < gpGlobals->time; i++) {
		CBasePlayer* listener = UTIL_PlayerByIndex(i);

		if (!listener) {
			continue;
		}

		if ((m_pPlayer != listener || !isPredicted) && UTIL_TestPVS(start, listener->edict())) {
			UTIL_SpriteTrail(start, end, evt.spriteTrail.sprite, evt.spriteTrail.count, 0,
				evt.spriteTrail.scale, evt.spriteTrail.speed, evt.spriteTrail.speedNoise,
				MSG_ONE_UNRELIABLE, NULL, listener->edict());
		}
	}
	
#endif
}

void CWeaponCustom::PlayEvent_Decal(WepEvt& evt, CBasePlayer* m_pPlayer, WcTrace* tr) {
	Vector pos;

	if (tr) {
		pos = tr->vecEndPos;
	}
	else {
#ifdef CLIENT_DLL
		pos = WC_GetGunPosition();
#else
		pos = m_pPlayer->GetGunPosition();
#endif		
	}

#ifdef CLIENT_DLL
	if (evt.decal.isGunshot) {
		EV_HLDM_GunshotDecalEffects(pos, true);
	}

	gEngfuncs.pEfxAPI->R_DecalShoot(gEngfuncs.pEfxAPI->Draw_DecalIndex(evt.decal.decalIdx),
		tr ? tr->pHit : 0, 0, pos, 0);
#else
	bool isPredicted = IsPredicted();
	edict_t* ed = tr ? INDEXENT(tr->pHit) : NULL;

	for (int i = 1; i < gpGlobals->time; i++) {
		CBasePlayer* listener = UTIL_PlayerByIndex(i);

		if (!listener) {
			continue;
		}

		if ((m_pPlayer != listener || !isPredicted)) {
			if (evt.decal.isGunshot) {
				UTIL_GunshotDecal(ed ? ENTINDEX(ed) : 0, pos, evt.decal.decalIdx, MSG_ONE_UNRELIABLE,
					NULL, listener->edict());
			}
			else {
				UTIL_Decal(ed ? ENTINDEX(ed) : 0, pos, evt.decal.decalIdx, MSG_ONE_UNRELIABLE,
					NULL, listener->edict());
			}
		}
	}
	
#endif
}

void CWeaponCustom::PlayEvent(int eventIdx, bool leftHand, bool akimboFire, WcTrace* tr) {
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
	case WC_EVT_BEAM:
		PlayEvent_Beam(evt, m_pPlayer);
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
		if (evt.playSound.flags & FL_WC_SOUND_CHARGE_PITCH) {
			m_chargeSoundEvt = eventIdx;
		}
		break;
	case WC_EVT_TOGGLE_ZOOM:
		ToggleZoom(evt.zoomToggle.zoomFov, evt.zoomToggle.zoomFov2);
		ProcessEvents(m_pPlayer->m_iFOV ? WC_TRIG_ZOOM_IN : WC_TRIG_ZOOM_OUT, 0);
		break;
	case WC_EVT_TOGGLE_STATE:
		PlayEvent_ToggleState(evt, m_pPlayer);
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
	case WC_EVT_SET_GRAVITY:
		PlayEvent_SetGravity(evt, m_pPlayer);
		break;
	case WC_EVT_DLIGHT:
		PlayEvent_DLight(evt, m_pPlayer);
		break;
	case WC_EVT_MUZZLEFLASH:
		PlayEvent_MuzzleFlash(evt, m_pPlayer);
		break;
	case WC_EVT_SPRITETRAIL:
		PlayEvent_SpriteTrail(evt, m_pPlayer, tr);
		break;
	case WC_EVT_DECAL:
		PlayEvent_Decal(evt, m_pPlayer, tr);
		break;
	case WC_EVT_SERVER:
		CustomServerEvent(evt, m_pPlayer);
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

		PlayEvent(qevt.eventIdx, qevt.leftHand, qevt.akimboFire, &qevt.tr);
		qevt.fireTime = 0; // free the slot
	}
}

void CWeaponCustom::CancelDelayedEvents(int trigger) {
	if (!g_runfuncs)
		return;

	for (int i = 0; i < WC_SERVER_EVENT_QUEUE_SZ; i++) {
		WcDelayEvent& qevt = eventQueue[i];	
		WepEvt& evt = params.events[qevt.eventIdx];

		if (evt.trigger == trigger) {
			qevt.fireTime = 0;
		}
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

float CWeaponCustom::GetCurrentAccuracyMultiplier() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 1.0f;
	
	float multiplier = 1.0f;

	Vector flatVelocity = m_pPlayer->pev->velocity;
	flatVelocity.z = 0;

	bool isMoving = flatVelocity.Length() > 200;

	if (!(m_pPlayer->pev->flags & FL_ONGROUND)) {
		multiplier *= 3.0f;
	}
	else if (m_pPlayer->pev->flags & FL_DUCKING) {
		multiplier *= 0.5f;
	}
	else if (isMoving) {
		multiplier *= 2.0f;
	}

	return multiplier;
}

void CWeaponCustom::GetCurrentAccuracy(float& accuracyX, float& accuracyY, float& accuracyX2, float& accuracyY2) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;
	
	if (IsPrimaryAltActive()) {
		accuracyX = params.shootOpts[3].accuracyX * 0.01f;
		accuracyY = params.shootOpts[3].accuracyY * 0.01f;
	}
	else if (IsAkimbo()) {
		accuracyX = params.akimbo.accuracyX * 0.01f;
		accuracyY = params.akimbo.accuracyY * 0.01f;
	}
	else {
		accuracyX = params.shootOpts[0].accuracyX * 0.01f;
		accuracyY = params.shootOpts[0].accuracyY * 0.01f;
	}

	bool hasSecondary = params.flags & FL_WC_WEP_HAS_SECONDARY;
	bool secondaryShoots = !(params.shootOpts[1].flags & FL_WC_SHOOT_NO_ATTACK);

	if (hasSecondary && secondaryShoots) {
		accuracyX2 = params.shootOpts[1].accuracyX * 0.01f;
		accuracyY2 = params.shootOpts[1].accuracyY * 0.01f;
	}
	else {
		accuracyX2 = accuracyX;
		accuracyY2 = accuracyY;
	}

	if (params.flags & FL_WC_WEP_DYNAMIC_ACCURACY) {
		float multiplier = GetCurrentAccuracyMultiplier();

		accuracyX *= multiplier;
		accuracyY *= multiplier;
		accuracyX2 *= multiplier;
		accuracyY2 *= multiplier;
	}
}

int CWeaponCustom::AddDuplicate(CBasePlayerItem* pOriginal) {
	CBasePlayer* pPlayer = pOriginal ? pOriginal->GetPlayer() : NULL;
	if (!pPlayer)
		return 0;

	if (!pPlayer->UseSevenKewpGuns() && wrongClientWeapon) {
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