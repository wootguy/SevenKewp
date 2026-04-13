#pragma once
#include "CBasePlayerWeapon.h"
#include "CProjectileCustom.h"
#include "custom_weapon.h"
#include "event_args.h"
#include "shared_util.h"

#ifdef CLIENT_DLL
#include "beamdef.h"
struct tempent_s;
typedef struct tempent_s TEMPENTITY;
#endif

#define WC_SERVER_EVENT_QUEUE_SZ 32

// number of seconds to delay replaying of events while the prediction code waits for a new state
// from the server
#define MAX_PREDICTION_WAIT 0.1f

#define MAX_WC_BEAMS 16

// Same as a TraceResult result except using entity indexes for delayed event safety
struct WcTrace {
	int		fAllSolid;			// if true, plane is not valid
	int		fStartSolid;		// if true, the initial point was in a solid area
	int		fInOpen;
	int		fInWater;
	float	flFraction;			// time completed, 1.0 = didn't hit anything
	Vector	vecEndPos;			// final position
	float	flPlaneDist;
	Vector	vecPlaneNormal;		// surface normal at impact
	int		pHit;				// entity the surface is on
	int		iHitgroup;			// 0 == generic, non zero is specific body part
};

struct WcDelayEvent {
	int eventIdx;
	float fireTime;
	bool leftHand;
	bool akimboFire; // event was triggered by both hands at the same time
	WcTrace tr;
};

struct SoundMapping {
	int idx;
	const char* path;
};

enum PredictionDataSendMode {
	WC_PRED_SEND_INIT,	// initialize prediction data for first pickup
	WC_PRED_SEND_WEP,	// only send weapon prediction data
	WC_PRED_SEND_EVT,	// only send event prediction data
	WC_PRED_SEND_BOTH	// send weapon and event data
};

enum WcAttackState {
	WC_CHARGE_STATE_NONE,
	WC_CHARGE_STATE_CHARGING,
	WC_CHARGE_STATE_DISCHARGING, // firing by force regardless of ammo and charge progress
	WC_CHARGE_STATE_OVERCHARGED,
};

struct WcBeam {
	int attackIdx; // set to -1 for non-constant beams, else used to kill the beam when a button is released
	float spreadX;
	float spreadY;
	float nextAttack;
	float creationTime;
	WepEvt evt;

#ifdef CLIENT_DLL
	BEAM* pBeam;
#else
	EHANDLE h_beam;
#endif

	bool isFree();
};

struct WcSprite {
	float creationTime;
	float killTime;
	int beamId; // will die when its parent beam does

#ifdef CLIENT_DLL
	TEMPENTITY* pSprite;
#else
	EHANDLE h_sprite;
#endif

	bool IsAlive();
	void Kill();
};

// m_fireState flags
#define FL_WC_STATE_PRIMARY_ALT	(1<<0)	// using alternate primary fire settings
#define FL_WC_STATE_LASER		(1<<1)	// laser is enabled
#define FL_WC_STATE_IS_AKIMBO	(1<<2)	// currently in akimbo mode
#define FL_WC_STATE_CAN_AKIMBO	(1<<3)	// can enable akimbo mode

// m_inAttack flags
#define FL_WC_INATTACK_PRIMARY_CALLED 0

class EXPORT CWeaponCustom : public CBasePlayerWeapon {
public:
	CustomWeaponParams params;

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
	const char* wrongClientWeapon; // weapon given to players who don't have the correct client to use this one. Do not use aliases here.
	int ammoFreqs[3]; // ammo frequency counters for each attack
	int animCount; // counter for ordered weapon aimations

	// server-side state
	TraceResult m_meleeDecalPos;
	float m_nextMeleeDecal;
	WcBeam m_beams[MAX_WC_BEAMS];
	WcSprite m_beamImpactSprite;

	// for prediction code, don't spam events/toggles while waiting for the new server state
	float m_lastZoomToggle;
	float m_lastLaserToggle;
	bool m_lastAltState;
	float m_lastAltToggle;
	float m_lastDeploy;
	int m_runningKickbackPred; // 1 = first frame of prediction, 2 = stop after next runfuncs
	Vector m_kickbackPredVel;
	uint32_t m_chargeStartCmdTime; // CMD time that begun a chargeup
	int m_chargeStartClip; // ammo started with when chargeup began
	bool m_primaryCalled;
	bool m_secondaryCalled;
	bool m_primaryFired;
	bool m_secondaryFired;
	int m_bulletFireCount; // for odd/even effects (m_iClip is unreliable)
	bool m_hasPredictionData; // was the client sent a prediction message for this weapon?
	int m_chargeSoundEvt; // event index to load charge sound details from

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
	BOOL IsSevenKewpWeapon() { return !(params.flags & FL_WC_WEP_NO_PREDICTION); }
	BOOL IsWeaponCustom() { return TRUE; }
	BOOL IsAkimboWeapon() override { return params.flags & FL_WC_WEP_AKIMBO; }
	int SecondaryAmmoIndex(void) { return m_iSecondaryAmmoType; }
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

	int* GetAttackClip(int attackIdx);
	void PrimaryAttack(void) override;
	void SecondaryAttack(void) override;
	void TertiaryAttack(void) override;
	bool CommonAttack(int attackIdx, int* clip, bool leftHand, bool akimboFire); // true if attacked
	void Cooldown(int attackIdx, int overrideMillis=-1);
	
	void FinishAttack(int attackIdx);
	void PlayEmptySound(int attackIdx);
	void FailAttack(int attackIdx, bool leftHand, bool akimboFire, bool ammoClick);
	void PlayRandomSound(CBasePlayer* plr, uint16_t sounds[4]); // generic server side sound playback

	//
	//  Override the methods below to add custom server-side logic to your weapon
	//
	virtual void CustomServerEvent(WepEvt& evt, CBasePlayer* m_pPlayer) {}
	virtual BOOL IsClientWeapon() {
		CBasePlayer* m_pPlayer = GetPlayer();
		return m_pPlayer && m_pPlayer->IsSevenKewpClient();
	}
	virtual void PrimaryAttackCustom(void) {} 
	virtual void SecondaryAttackCustom(void) {}
	virtual void TertiaryAttackCustom(void) {}
	virtual void WeaponIdleCustom(void) {} // called after the weapon has idled
	virtual bool Chargeup(int attackIdx, int* clip, bool leftHand, bool akimboFire);
	virtual void Chargedown(int attackIdx);
	virtual void MeleeAttack(int attackIdx);
	virtual void MeleeMiss(CBasePlayer* plr) { } // called when a melee attack misses
	virtual bool MeleeIsFlesh(CBaseEntity* target); // true if target should make a flesh sound (may be NULL)
	virtual bool MeleeHit(CBasePlayer* plr, CBaseEntity* target) { return false; } // return true to override default melee hit logic
	virtual void MeleeHitFlesh(CBasePlayer* plr, CBaseEntity* target) {} // called when a melee attack hits a flesh entity
	virtual void MeleeHitWall(CBasePlayer* plr, CBaseEntity* target) {} // called when a melee attack hits a hard surface
	virtual void AttackTrace(CBasePlayer* plr, int attackIdx, Vector vecSrc, TraceResult& tr) {} // called after every attack trace

	float GetChargeMult(WepEvt& evt, int flagMask);
	void KickbackPrediction();
	void ToggleZoom(int zoomFov, int zoomFov2);
	void ToggleLaser(bool enable);
	void HideLaser(bool hideNotUnhide);
	void CancelZoom();
	bool CheckTracer(int idx, Vector& vecSrc, Vector forward, Vector right, int iTracerFreq);
	void SendPredictionData(edict_t* target, PredictionDataSendMode sendMode=WC_PRED_SEND_INIT);
	inline bool HasPredictionData(edict_t* target) { return m_predDataSent[m_iId] & PLRBIT(target); }
	bool IsPredicted();
	int GetAttackIdx(WepEvt& evt); // TODO: store this info in the event
	int GetImpactArg(int attackIdx, bool impactMonster, bool impactWorld);
	WcBeam* AllocBeam();
	void UpdateBeams();
	bool KillBeams(int attackIdx=-1); // set attack idx to only kill constant beams attached to an attack button. True if any beams killed
	Vector BeamAttack(WcBeam& beam, CBasePlayer* m_pPlayer);
	void QuakeMuzzleFlash(CBasePlayer* plr);

	void FireAmmoEvents(int ammoPool);

	void ProcessEvents(int trigger, int triggerArg, bool leftHand = false, bool akimboFire = false, int clipLeft = 0, WcTrace* tr = NULL);
	void QueueDelayedEvent(int eventIdx, float fireTime, bool leftHand, bool akimboFire, WcTrace* tr);
	void PlayDelayedEvents();
	void CancelDelayedEvents(int trigger);

	void PlayEvent_Bullets(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand, bool akimboFire);
	void PlayEvent_Beam(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_Projectile(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_Kickback(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_SetGravity(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_Sound(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand, bool akimboFire);
	void PlayEvent_EjectShell(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand);
	void PlayEvent_PunchAngle(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_WepAnim(WepEvt& evt, CBasePlayer* m_pPlayer, bool leftHand);
	void PlayEvent_Cooldown(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_ToggleState(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_HideLaser(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_DLight(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_MuzzleFlash(WepEvt& evt, CBasePlayer* m_pPlayer);
	void PlayEvent_SpriteTrail(WepEvt& evt, CBasePlayer* m_pPlayer, WcTrace* tr);
	void PlayEvent_Decal(WepEvt& evt, CBasePlayer* m_pPlayer, WcTrace* tr);
	void PlayEvent(int eventIdx, bool leftHand, bool akimboFire, WcTrace* tr);

	float GetActiveMovespeedMult();
	float WallTime();

	// Time accumulated in user commands. Synced between the client and server for each user cmd.
	// Prefer using this over the decrement timers like m_flNextIdleTime because those are buggy and
	// end up skipping/duplicating events and breaking movement prediction. Not necessary for events
	// triggered by buttons because button states don't accumulate error like float timers do.
	uint32_t CmdTime();

	// repurposing these weapon vars so I don't have to network something new to the client
	BOOL CanAkimbo() { return (m_fireState & FL_WC_STATE_CAN_AKIMBO) != 0; }
	inline void SetCanAkimbo(bool canAkimbo) { 
		m_fireState = canAkimbo ? (m_fireState | FL_WC_STATE_CAN_AKIMBO) : (m_fireState & ~FL_WC_STATE_CAN_AKIMBO);
	}
	BOOL IsAkimbo() { return (m_fireState & FL_WC_STATE_IS_AKIMBO) != 0; }
	void SetAkimbo(bool akimbo);
	void SendAkimboAnim(int iAnim);
	inline WcAttackState GetChargedState(int attackIdx) { return (WcAttackState)((m_fInAttack >> (attackIdx*4)) & 0xf); }
	inline void SetChargedState(int attackIdx, WcAttackState newState) {
		int shift = attackIdx * 4;
		int mask = 0xf << shift;
		newState = (WcAttackState)((newState & 0xf) << shift);
		m_fInAttack = (m_fInAttack & ~mask) | newState;
	}
	inline void ClearChargedStates() { m_fInAttack = 0; }
	inline bool AreAnyAttacksCharging() { return m_fInAttack != 0; }

	BOOL IsLaserOn() { return (m_fireState & FL_WC_STATE_LASER) != 0; }
	void SetLaser(bool enable);
	void UpdateLaser();
	inline void SetPrimaryAlt(bool enable) {
		m_fireState = enable ? (m_fireState | FL_WC_STATE_PRIMARY_ALT) : (m_fireState & ~FL_WC_STATE_PRIMARY_ALT);
		m_lastAltToggle = WallTime();
	}
	bool IsPrimaryAltActive();
	CustomWeaponShootOpts& GetShootOpts(int attackIdx);
	float GetCurrentAccuracyMultiplier();
	void GetCurrentAccuracy(float& accuracyX, float& accuracyY, float& accuracyX2, float& accuracyY2);

	int AddDuplicate(CBasePlayerItem* pOriginal) override;
	inline bool IsExclusiveHold() { return params.flags & FL_WC_WEP_EXCLUSIVE_HOLD; }

	static int m_tracerCount[32];
	static uint32_t m_predDataSent[MAX_WEAPONS]; // bitfields indicating which players received prediction data
};