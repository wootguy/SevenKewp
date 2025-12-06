#pragma once
#include "CBasePlayerWeapon.h"
#include "CProjectileCustom.h"
#include "custom_weapon.h"
#include "event_args.h"
#include "shared_util.h"

#define WC_SERVER_EVENT_QUEUE_SZ 32

// number of seconds to delay replaying of events while the prediction code waits for a new state
// from the server
#define MAX_PREDICTION_WAIT 0.1f

struct WcDelayEvent {
	int eventIdx;
	float fireTime;
	bool leftHand;
	bool akimboFire; // event was triggered by both hands at the same time
};

struct SoundMapping {
	int idx;
	const char* path;
};

enum PredictionDataSendMode {
	WC_PRED_SEND_INIT,	// initialize prediction data for first pickup
	WC_PRED_SEND_WEP,	// only send weapon prediction data
	WC_PRED_SEND_EVT	// only send event prediction data
};

class EXPORT CWeaponCustom : public CBasePlayerWeapon {
public:
	CustomWeaponParams params;

	float m_flChargeStartPrimary; // time weapon started charging for a primary attack (0 = not started)
	float m_flChargeStartSecondary; // time weapon started charging for a secondary attack (0 = not started)
	float m_lastBeamUpdate;
	float m_laserOnTime; // turn laser on after this time
	bool m_bInAkimboReload;
	bool m_bWantAkimboReload; // want to keep reloading until both guns are full
	bool m_hasLaserAttachment;
	const char* animExt; // third person player animation set
	const char* animExtZoom; // animation set used while zoomed
	const char* animExtAkimbo; // animation set used while in akimbo mode
	const char* pmodelAkimbo; // thirdperson model used in akimbo mode
	const char* wmodelAkimbo; // world model used in akimbo mode
	const char* wrongClientWeapon; // weapon given to players who don't have the correct client to use this one
	int ammoFreqs[3]; // ammo frequency counters for each attack
	int animCount; // counter for ordered weapon aimations

	// for prediction code, don't spam events/toggles while waiting for the new server state
	float m_lastZoomToggle;
	float m_lastLaserToggle;
	float m_lastLaserState;
	float m_lastDeploy;
	float m_lastChargeDown; // lazy fix for chargedown effects playing twice
	int m_runningKickbackPred; // 1 = first frame of prediction, 2 = stop after next runfuncs
	Vector m_kickbackPredVel;
	bool m_primaryCalled;
	bool m_secondaryCalled;
	bool m_waitForNextRunfuncs; // don't attack until the next g_runfuncs call
	int m_bulletFireCount; // for odd/even effects (m_iClip is unreliable)
	bool m_hasPredictionData; // was the client sent a prediction message for this weapon?
	bool m_waitForAttackRelease; // wait for attack button to release before attacking again

	int m_akimboAnim;
	float m_akimboAnimTime;
	float m_akimboLastEventFrame; // last frame we checked for animation events
	bool m_lastCanAkimbo; // was akimbo possible during the last think?

	WcDelayEvent eventQueue[WC_SERVER_EVENT_QUEUE_SZ]; // for playing server-side events with a delay

	EHANDLE m_hLaserSpot;

	void Spawn() override;
	void Precache() override;
	void PrecacheEvents() override;
	void AddEvent(WepEvt evt);
	BOOL UseDecrement(void) override { return TRUE; }
	BOOL IsSevenKewpWeapon() { return TRUE; }
	BOOL IsWeaponCustom() { return TRUE; }
	BOOL IsAkimboWeapon() override { return params.flags & FL_WC_WEP_AKIMBO; }
	int SecondaryAmmoIndex(void) { return m_iSecondaryAmmoType; }
	virtual BOOL IsClientWeapon() { 
		CBasePlayer* m_pPlayer = GetPlayer();
		return m_pPlayer && m_pPlayer->IsSevenKewpClient();
	}
	CWeaponCustom* MyWeaponCustomPtr(void) { return this; }
	int AddToPlayer(CBasePlayer* pPlayer) override;
	const char* GetAnimSet();
	void UpdateAnimSet();
	BOOL Deploy() override;
	void Holster(int skiplocal) override;
	void Reload() override;
	void WeaponIdle() override;
	void ItemPostFrame() override;
	const char* GetModelP() override;
	const char* GetModelW() override;

	void PrimaryAttack(void) override;
	void SecondaryAttack(void) override;
	void TertiaryAttack(void) override;
	virtual void PrimaryAttackCustom(void) {} // override this to add custom server-side logic to your weapon.
	virtual void SecondaryAttackCustom(void) {} // override this to add custom server-side logic to your weapon
	virtual void TertiaryAttackCustom(void) {} // override this to add custom server-side logic to your weapon
	virtual void WeaponIdleCustom(void) {} // override this to add custom server-side logic after the weapon has idled
	bool CommonAttack(int attackIdx, int* clip, bool leftHand, bool akimboFire); // true if attacked
	void Cooldown(int attackIdx, int overrideMillis=-1);
	bool Chargeup(int attackIdx, bool leftHand, bool akimboFire);
	void FinishAttack(int attackIdx);
	void FailAttack(int attackIdx, bool leftHand, bool akimboFire);
	void KickbackPrediction();
	void ToggleZoom(int zoomFov, int zoomFov2);
	void ToggleLaser();
	void HideLaser(bool hideNotUnhide);
	void CancelZoom();
	bool CheckTracer(int idx, Vector& vecSrc, Vector forward, Vector right, int iTracerFreq);
	void SendPredictionData(edict_t* target, PredictionDataSendMode sendMode=WC_PRED_SEND_INIT);
	inline bool HasPredictionData(edict_t* target) { return m_predDataSent[m_iId] & PLRBIT(target); }
	bool IsPredicted();

	void ProcessEvents(int trigger, int triggerArg, bool leftHand = false, bool akimboFire = false, int clipLeft=0);
	void QueueDelayedEvent(int eventIdx, float fireTime, bool leftHand, bool akimboFire);
	void PlayDelayedEvents();
	void CancelDelayedEvents();

	void PlayEvent_Bullets(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand, bool akimboFire);
	void PlayEvent_Projectile(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_Kickback(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_SetGravity(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_Sound(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand, bool akimboFire);
	void PlayEvent_EjectShell(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand);
	void PlayEvent_PunchAngle(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_WepAnim(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand);
	void PlayEvent_Cooldown(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_ToggleAkimbo(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_HideLaser(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_DLight(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent(int eventIdx, bool leftHand, bool akimboFire);

	float GetActiveMovespeedMult();
	float WallTime();

	// repurposing these weapon vars so I don't have to network something new to the client
	BOOL CanAkimbo() { return m_flStartThrow != 0; }
	inline void SetCanAkimbo(bool canAkimbo) { m_flStartThrow = canAkimbo ? 1 : 0; }
	BOOL IsAkimbo() { return m_fireState != 0; }
	void SetAkimbo(bool akimbo);
	void SendAkimboAnim(int iAnim);

	BOOL IsLaserOn() { return m_flReleaseThrow != 0; }
	void SetLaser(bool enable);
	void UpdateLaser();
	bool IsPrimaryAltActive();
	CustomWeaponShootOpts& GetShootOpts(int attackIdx);
	float GetCurrentAccuracyMultiplier();
	void GetCurrentAccuracy(float& accuracyX, float& accuracyY, float& accuracyX2, float& accuracyY2);

	int AddDuplicate(CBasePlayerItem* pOriginal) override;
	inline bool IsExclusiveHold() { return params.flags & FL_WC_WEP_EXCLUSIVE_HOLD; }

	static int m_tracerCount[32];
	static uint32_t m_predDataSent[MAX_WEAPONS]; // bitfields indicating which players received prediction data
};