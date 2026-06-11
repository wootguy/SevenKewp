#pragma once
#include "CBasePlayerWeapon.h"
#include "CProjectileCustom.h"
#include "wc_params.h"
#include "wc_schema.h"
#include "wc_config.h"
#include "wc_net.h"
#include "event_args.h"
#include "shared_util.h"
#include "CWeaponEvents.h"

// number of seconds to delay replaying of events while the prediction code waits for a new state
// from the server
#define MAX_PREDICTION_WAIT 0.1f

struct SoundMapping {
	int idx;
	const char* path;
};

enum WcAttackState {
	WC_CHARGE_STATE_NONE,
	WC_CHARGE_STATE_CHARGING,
	WC_CHARGE_STATE_DISCHARGING, // firing by force regardless of ammo and charge progress, or charging down in constant mode
	WC_CHARGE_STATE_OVERCHARGED,
};

// m_fireState flags (32 bits)
#define FL_WC_STATE_PRIMARY_ALT		(1<<0)	// using alternate primary fire settings
#define FL_WC_STATE_LASER			(1<<1)	// laser is enabled
#define FL_WC_STATE_ZOOM			(1<<2)	// weapon is zoomed in
#define FL_WC_STATE_ZOOM_FURTHER	(1<<3)	// zoomed in even further (level 3 if both zoom flags set, else level 2)
#define FL_WC_STATE_IS_AKIMBO		(1<<4)	// currently in akimbo mode
#define FL_WC_STATE_CAN_AKIMBO		(1<<5)	// can enable akimbo mode
#define FL_WC_STATE_FIRST_DEPLOYED	(1<<6)	// weapon was deployed for the first time
#define FL_WC_STATE_WANT_RELOAD		(1<<7)	// weapon should reload at the next idle

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
	const char* m_configPath;	// weapon config to load parameters from
	string_t m_hudPath;
	int m_mergedModelBody;
	int ammoFreqs[3]; // ammo frequency counters for each attack

	// server-side state
	TraceResult m_meleeDecalPos;
	float m_nextMeleeDecal;

	// shared state
	float m_lastLaserToggle;
	float m_lastDeploy;
	int m_runningKickbackPred; // 1 = first frame of prediction, 2 = stop after next runfuncs
	Vector m_kickbackPredVel;
	uint32_t m_chargeStartCmdTime; // CMD time that begun a chargeup
	uint32_t m_chargeStopCmdTime; // CMD time that a discharge began
	uint32_t m_attackStartCmdTime; // CMD time that an attack began
	float m_lastCharge; // last charge progress calculated for chargeup/chargedown
	int m_chargeStartClip; // ammo started with when chargeup began
	bool m_primaryCalled;
	bool m_secondaryCalled;
	bool m_primaryFired;
	bool m_secondaryFired;
	bool m_hasPredictionData; // was the client sent a prediction message for this weapon?
	int m_chargeSoundEvt; // event index to load charge sound details from
	int m_lastAnim;			// last animation set by an event
	float m_idleTime;		// idle time for the last played event

	int m_akimboAnim;
	float m_akimboAnimTime;
	float m_akimboLastEventFrame; // last frame we checked for animation events
	bool m_lastCanAkimbo; // was akimbo possible during the last think?

	EHANDLE m_hLaserSpot;

	CWeaponEvents events;

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
	float GetChargeProgress(float chargeTime);
	void PlayChargeSound(float t);

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
	virtual void AttackTrace(CBasePlayer* plr, int attackIdx, Vector vecSrc, TraceResult& tr, bool isRicochet) {} // called after every attack trace
	virtual void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount);
	virtual const char* DisplayName() override { return STRING(params.displayName); }
	virtual const char* GetDeathNoticeWeapon() { return STRING(params.killFeedIcon); }
	virtual int MergedModelBody() { return m_mergedModelBody; }
	virtual int GetItemInfo(ItemInfo* p);

	void KickbackPrediction();
	void ToggleLaser(bool enable);
	void HideLaser(bool hideNotUnhide);
	bool IsPredicted();
	int GetAttackIdx(WepEvt& evt); // TODO: store this info in the event
	studiohdr_t* GetViewModelHeader();
	float GetActiveMovespeedMult();
	float WallTime();

	// Time accumulated in user commands. Synced between the client and server for each user cmd.
	// Prefer using this over the decrement timers like m_flNextIdleTime because those are buggy and
	// end up skipping/duplicating events and breaking movement prediction. Not necessary for events
	// triggered by buttons because button states don't accumulate error like float timers do.
	uint32_t CmdTime();

	// repurposing these weapon vars so I don't have to network something new to the client
	BOOL CanAkimbo() { return GetState(FL_WC_STATE_CAN_AKIMBO); }
	void SetCanAkimbo(bool canAkimbo);
	BOOL IsAkimbo() { return GetState(FL_WC_STATE_IS_AKIMBO); }
	bool IsIronSights() { return GetZoom() != 0 && (params.flags & FL_WC_WEP_IRON_SIGHTS_ZOOM); }
	void SetState(int stateBits, bool state);
	bool GetState(int stateBits);
	void SetAkimbo(bool akimbo);
	void SendAkimboAnim(int iAnim);
	WcAttackState GetChargedState(int attackIdx);
	void SetChargedState(int attackIdx, WcAttackState newState);
	inline void ClearChargedStates() { m_fInAttack = 0; }
	inline bool AreAnyAttacksCharging() { return m_fInAttack != 0; }

	BOOL IsLaserOn() { return GetState(FL_WC_STATE_LASER); }
	int GetZoom(); // returns current zoom level
	int CycleZoom(int attackIdx, bool forceCancelZoom=false);
	void SetLaser(bool enable);
	void UpdateLaser();
	bool IsPrimaryAltActive();
	CustomWeaponShootOpts& GetShootOpts(int attackIdx);

	int AddDuplicate(CBasePlayerItem* pOriginal) override;
	inline bool IsExclusiveHold() { return params.flags & FL_WC_WEP_EXCLUSIVE_HOLD; }
};

extern "C" EXPORT void weapon_custom_ini(entvars_t* pev);