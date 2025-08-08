#include "CWeaponCustom.h"

#ifdef CLIENT_DLL
#include "../cl_dll/hud_iface.h"
#define CLALERT(fmt, ...) gEngfuncs.Con_Printf(fmt, __VA_ARGS__)
#include "eng_wrappers.h"
#else
#define CLALERT(fmt, ...)
#endif

int CWeaponCustom::m_usCustom = 0;
char CWeaponCustom::m_soundPaths[MAX_PRECACHE][256];

bool g_customWeaponSounds[MAX_PRECACHE_SOUND];

void CWeaponCustom::Spawn() {
	Precache();
	SetWeaponModelW();
	FallInit();// get ready to fall down.
}

void CWeaponCustom::Precache() {
	PrecacheEvents();
}

void CWeaponCustom::PrecacheEvents() {
	for (int i = 0; i < params.numEvents; i++) {
		WepEvt& evt = params.events[i];
		if (evt.evtType == WC_EVT_PLAY_SOUND && evt.playSound.sound)
			g_customWeaponSounds[evt.playSound.sound] = true;
	}
}

void CWeaponCustom::AddEvent(WepEvt evt) {
	if (params.numEvents >= MAX_CUSTOM_WEAPON_EVENTS) {
		ALERT(at_error, "Exceeded max custom weapon events for %s\n", STRING(pev->classname));
		return;
	}

	params.events[params.numEvents++] = evt;
}

void CWeaponCustom::PrecacheEvent() {
	m_usCustom = PRECACHE_EVENT(1, "events/customwep.sc");
	PRECACHE_GENERIC("events/customwep.sc");
}

int CWeaponCustom::AddToPlayer(CBasePlayer* pPlayer) {
	SendPredictionData(pPlayer->edict());
	return CBasePlayerWeapon::AddToPlayer(pPlayer);
}

BOOL CWeaponCustom::Deploy()
{
	m_flReloadEnd = 0;
	m_bInReload = false;

#ifdef CLIENT_DLL
	CBasePlayer* m_pPlayer = GetPlayer();

	if (!CanDeploy())
		return FALSE;

	m_pPlayer->pev->viewmodel = params.vmodel;

	SendWeaponAnim(params.deployAnim, 1, pev->body);

	g_irunninggausspred = false;
	m_pPlayer->m_flNextAttack = 0.5;
	m_flReloadEnd = 0;
	m_flTimeWeaponIdle = 1.0;
	ProcessEvents(WC_TRIG_DEPLOY, 0);
	return TRUE;
#else
	return DefaultDeploy(STRING(g_indexModels[params.vmodel]), m_defaultModelP, params.deployAnim, animExt, 1);
#endif
}

void CWeaponCustom::Reload() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_bInReload)
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	int j = V_min(params.maxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	if (j == 0)
		return;

	if (gpGlobals->time - m_flReloadEnd < 0.350f) {
		return; // will prevent auto-reload loops for players with less than about 250 ping
	}

	SendWeaponAnim(params.reloadStage[0].anim, 1, pev->body);

	CLALERT("Start Reload %d %d\n", m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	m_bInReload = true;

	ProcessEvents(WC_TRIG_RELOAD, 0);

	int totalReloadTime = params.reloadStage[0].time;
	Cooldown(totalReloadTime);
	m_pPlayer->m_flNextAttack = 0; // keep calling post frame for complex reloads
	m_flReloadStart = gpGlobals->time;
	m_flReloadEnd = gpGlobals->time + totalReloadTime*0.001f;
}

void CWeaponCustom::WeaponIdle() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_bInReload)
		return;

	ResetEmptySound();

	// Update auto-aim
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int idleSum = 0;
	for (int i = 0; i < 4; i++) {
		idleSum += params.idles[i].weight;
	}

	int idleRnd = (UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0) + 0.005f) * idleSum;

	for (int i = 0; i < 4; i++) {
		WeaponCustomIdle& idle = params.idles[i];
		idleRnd -= idle.weight;

		if (idleRnd <= 0) {
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + idle.time * 0.001f;
			SendWeaponAnim(idle.anim, 1, pev->body);
			break;
		}
	}
}

void CWeaponCustom::ItemPostFrame() {
	CBasePlayerWeapon::ItemPostFrame();

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	// update reload stage
	if (m_bInReload && gpGlobals->time >= m_flReloadEnd) {
		// complete the reload.
		int j = V_min(params.maxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
		CLALERT("Finish Reload1 %d %d\n", m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
		CLALERT("Finish Reload2 %d %d\n", m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
		m_pPlayer->TabulateAmmo();
		m_bInReload = false;
		m_flReloadEnd = gpGlobals->time;
		return;
	}
}

void CWeaponCustom::PrimaryAttack() {
	if (params.flags & FL_WC_WEP_HAS_PRIMARY) {
		if (CommonAttack(0)) {
			ProcessEvents(WC_TRIG_SHOOT_PRIMARY, 0);
			ProcessEvents(WC_TRIG_SHOOT_PRIMARY_CLIPSIZE, m_iClip);

			if (m_iClip % 2 == 0) {
				ProcessEvents(WC_TRIG_SHOOT_PRIMARY_EVEN, 0);
			}
			else {
				ProcessEvents(WC_TRIG_SHOOT_PRIMARY_ODD, 0);
			}
		}
	}
}

void CWeaponCustom::SecondaryAttack() {
	if (params.flags & FL_WC_WEP_HAS_SECONDARY) {
		if (CommonAttack(1)) {
			ProcessEvents(WC_TRIG_SHOOT_SECONDARY, 0);
		}
	}
}

bool CWeaponCustom::CommonAttack(int attackIdx) {
	CustomWeaponShootOpts& opts = params.shootOpts[attackIdx];
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	int clipLeft = m_iClip;

	if (attackIdx == 1) {
		clipLeft = m_iSecondaryAmmoType >= 0 ? m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] : 0;
	}

	if (clipLeft < opts.ammoCost) {
		if (!m_fInReload)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		}

		return false;
	}

	if (m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD && !(opts.flags & FL_WC_SHOOT_UNDERWATER))
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15;
		return false;
	}

	m_iClip -= opts.ammoCost;
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	if (clipLeft <= 0)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		{
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", SUIT_REPEAT_OK, 0);
		}
	}

	Cooldown(opts.cooldown);

	return true;
}

void CWeaponCustom::Cooldown(int millis) {
	float nextAttack = UTIL_WeaponTimeBase() + millis * 0.001f;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = nextAttack;
	m_flTimeWeaponIdle = nextAttack + 1.0f;
}

Vector CWeaponCustom::ProcessBulletEvent(WepEvt& evt, CBasePlayer* m_pPlayer) {
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

	if (evt.bullets.flags & FL_WC_BULLETS_MUZZLE_FLASH) {
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	}

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	lagcomp_begin(m_pPlayer);
	Vector vecDir = m_pPlayer->FireBulletsPlayer(evt.bullets.count, vecSrc, vecAiming, spread, 8192,
		evt.bullets.btype, evt.bullets.tracerFreq, evt.bullets.damage, m_pPlayer->pev, m_pPlayer->random_seed);
	lagcomp_end();

	return vecDir;
}

void CWeaponCustom::ProcessKickbackEvent(WepEvt& evt, CBasePlayer* m_pPlayer) {
#ifdef CLIENT_DLL
	g_irunninggausspred = true; // for prediction
#endif
	Vector vecAiming = m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);
	m_pPlayer->pev->velocity = m_pPlayer->pev->velocity - vecAiming * evt.kickback.pushForce;
}

void CWeaponCustom::ProcessEvents(int trigger, int triggerArg) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	for (int i = 0; i < params.numEvents; i++) {
		WepEvt& evt = params.events[i];

		if (evt.trigger != trigger)
			continue;
		if (trigger == WC_TRIG_SHOOT_PRIMARY_CLIPSIZE && triggerArg != evt.triggerArg)
			continue;

		Vector vecDir;

		switch (evt.evtType) {
		case WC_EVT_SET_BODY:
			pev->body = evt.setBody.newBody;
			break;
		case WC_EVT_BULLETS:
			vecDir = ProcessBulletEvent(evt, m_pPlayer);
			break;
		case WC_EVT_KICKBACK:
			ProcessKickbackEvent(evt, m_pPlayer);
			break;
		}

		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usCustom, evt.delay * 0.001f,
			(float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, m_iId, i, 0, 0);
	}
}

void CWeaponCustom::SendPredictionData(edict_t* target) {
#ifndef CLIENT_DLL
	int sentBytes = 0;
	MESSAGE_BEGIN(MSG_ONE, gmsgCustomWeapon, NULL, target);
	WRITE_BYTE(m_iId); sentBytes += 1;
	WRITE_BYTE(params.flags); sentBytes += 1;
	WRITE_SHORT(params.maxClip); sentBytes += 2;

	WRITE_SHORT(params.vmodel); sentBytes += 2;
	WRITE_BYTE(params.deployAnim); sentBytes += 1;

	for (int k = 0; k < 3; k++) {
		WeaponCustomReload& reload = params.reloadStage[k];
		WRITE_BYTE(reload.anim); sentBytes += 1;
		WRITE_SHORT(reload.time); sentBytes += 2;

		if (!(params.flags & FL_WC_WEP_SHOTGUN_RELOAD))
			break;
	}

	for (int k = 0; k < 4; k++) {
		WeaponCustomIdle& idle = params.idles[k];
		WRITE_BYTE(idle.anim); sentBytes += 1;
		WRITE_BYTE(idle.weight); sentBytes += 1;
		WRITE_SHORT(idle.time); sentBytes += 2;
	}

	for (int k = 0; k < 2; k++) {
		if (!(params.flags & FL_WC_WEP_HAS_PRIMARY) && k == 0)
			continue;
		if (!(params.flags & FL_WC_WEP_HAS_SECONDARY) && k == 1)
			continue;

		CustomWeaponShootOpts& opts = params.shootOpts[k];
		WRITE_BYTE(opts.flags); sentBytes += 1;
		WRITE_BYTE(opts.ammoCost); sentBytes += 1;
		WRITE_SHORT(opts.cooldown); sentBytes += 2;
	}

	WRITE_BYTE(params.numEvents); sentBytes += 1;
	for (int k = 0; k < params.numEvents; k++) {
		WepEvt& evt = params.events[k];
		uint32_t packedHeader = (evt.evtType << 28) | (evt.trigger << 24) | (evt.triggerArg << 14) | evt.delay;
		WRITE_LONG(packedHeader); sentBytes += 4;

		switch (evt.evtType) {
		case WC_EVT_PLAY_SOUND:
			WRITE_SHORT(evt.playSound.sound); sentBytes += 2;
			WRITE_BYTE(evt.playSound.volume); sentBytes += 1;
			WRITE_BYTE(evt.playSound.pitchMin); sentBytes += 1;
			WRITE_BYTE(evt.playSound.pitchMax); sentBytes += 1;
			break;
		case WC_EVT_EJECT_SHELL:
			WRITE_SHORT(evt.ejectShell.model); sentBytes += 2;
			WRITE_SHORT(evt.ejectShell.offsetForward); sentBytes += 2;
			WRITE_SHORT(evt.ejectShell.offsetUp); sentBytes += 2;
			WRITE_SHORT(evt.ejectShell.offsetRight); sentBytes += 2;
			break;
		case WC_EVT_PUNCH_SET:
		case WC_EVT_PUNCH_RANDOM:
			WRITE_SHORT(evt.punch.x); sentBytes += 2;
			WRITE_SHORT(evt.punch.y); sentBytes += 2;
			WRITE_SHORT(evt.punch.z); sentBytes += 2;
			break;
		case WC_EVT_SET_BODY:
			WRITE_BYTE(evt.setBody.newBody); sentBytes += 1;
			break;
		case WC_EVT_WEP_ANIM:
			WRITE_BYTE(evt.anim.animMin); sentBytes += 1;
			WRITE_BYTE(evt.anim.animMax); sentBytes += 1;
			break;
		case WC_EVT_BULLETS:
			WRITE_BYTE(evt.bullets.count); sentBytes += 1;
			//WRITE_SHORT(evt.bullets.damage); sentBytes += 2; // not needed for prediction
			WRITE_SHORT(evt.bullets.spreadX); sentBytes += 2;
			WRITE_SHORT(evt.bullets.spreadY); sentBytes += 2;
			WRITE_BYTE(evt.bullets.btype); sentBytes += 1;
			WRITE_BYTE(evt.bullets.tracerFreq); sentBytes += 1;
			WRITE_BYTE(evt.bullets.flags); sentBytes += 1;
			break;
		case WC_EVT_KICKBACK:
			WRITE_SHORT(evt.kickback.pushForce); sentBytes += 2;
			break;
		default:
			ALERT(at_error, "Invalid custom weapon event type %d\n", evt.evtType);
			break;
		}
	}
	MESSAGE_END();

	ALERT(at_console, "Sent %d prediction bytes\n", sentBytes);
#endif
}