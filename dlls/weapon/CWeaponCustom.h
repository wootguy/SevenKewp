#pragma once
#include "CBasePlayerWeapon.h"
#include "custom_weapon.h"
#include "event_args.h"

#define WC_SERVER_EVENT_QUEUE_SZ 32

// number of seconds to delay replaying of events while the prediction code waits for a new state
// from the server
#define MAX_PREDICTION_WAIT 0.5f

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

class CWeaponCustom : public CBasePlayerWeapon {
public:
	CustomWeaponParams params;

	float m_flReloadStart;
	float m_flReloadEnd; // for preventing reload loops
	bool m_bInReload; // like m_fInReload but allows the weapon to think during the reload
	bool m_bInAkimboReload;
	bool m_bWantAkimboReload;
	const char* animExt; // third person player animation set
	const char* animExtZoom; // animation set used while zoomed
	const char* animExtAkimbo; // animation set used while in akimbo mode
	const char* pmodelAkimbo; // thirdperson model used in akimbo mode
	const char* wmodelAkimbo; // world model used in akimbo mode
	const char* wrongClientWeapon; // weapon given to players who don't have the correct client to use this one

	// for prediction code, don't spam events/toggles while waiting for the new server state
	float m_lastZoomToggle;
	float m_lastDeploy;
	int m_runningKickbackPred; // 1 = first frame of prediction, 2 = stop after next runfuncs
	Vector m_kickbackPredVel;

	int m_akimboAnim;
	float m_akimboAnimTime;
	float m_akimboLastEventFrame; // last frame we checked for animation events
	bool m_lastCanAkimbo; // was akimbo possible during the last think?

	WcDelayEvent eventQueue[WC_SERVER_EVENT_QUEUE_SZ]; // for playing server-side events with a delay

	void Spawn() override;
	void Precache() override;
	void PrecacheEvents() override;
	void AddEvent(WepEvt evt);
	BOOL UseDecrement(void) override { return TRUE; }
	BOOL IsSevenKewpWeapon() { return TRUE; }
	BOOL IsWeaponCustom() { return TRUE; }
	BOOL IsAkimboWeapon() override { return params.flags & FL_WC_WEP_AKIMBO; }
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
	bool CommonAttack(int attackIdx, int& clip, bool leftHand, bool akimboFire); // true if attacked
	void Cooldown(int attackIdx, int overrideMillis=-1);
	void ToggleZoom(int zoomFov);
	void CancelZoom();
	bool CheckTracer(int idx, Vector& vecSrc, Vector forward, Vector right, int iTracerFreq);
	Vector ProcessBulletEvent(WepEvt& evt, CBasePlayer* m_pPlayer);
	void ProcessKickbackEvent(WepEvt& evt, CBasePlayer* m_pPlayer);
	int ProcessSoundEvent(WepEvt& evt, CBasePlayer* m_pPlayer); // returns sound index to play
	void ProcessEvents(int trigger, int triggerArg, bool leftHand=false, bool akimboFire=false);
	void SendPredictionData(edict_t* target);

	void QueueDelayedEvent(int eventIdx, float fireTime, bool leftHand, bool akimboFire);
	void PlayDelayedEvents();
	void PlayEvent(int eventIdx, bool leftHand, bool akimboFire);
	float WallTime();

	// repurposing these weapon vars so I don't have to network something new to the client
	BOOL CanAkimbo() { return m_flStartThrow != 0; }
	inline void SetCanAkimbo(bool canAkimbo) { m_flStartThrow = canAkimbo ? 1 : 0; }
	BOOL IsAkimbo() { return m_fireState != 0; }
	void SetAkimbo(bool akimbo);
	void SendAkimboAnim(int iAnim);
	int AddDuplicate(CBasePlayerItem* pOriginal) override;

	// get all HL clients as a bitfield for sound messages. Excludes the given player.
	// This is used to send sounds to clients that can't play custom sevenkewp events.
	static uint32_t GetOtherHlClients(edict_t* plr);
	
	static int SendSoundMappingChunk(CBasePlayer* target, std::vector<SoundMapping>& chunk);
	static void SendSoundMapping(CBasePlayer* target);

	// same event used for all custom weapons
	static void PrecacheEvent();
	static int m_usCustom;
	static char m_soundPaths[MAX_PRECACHE][256];
	static int m_tracerCount[32];

	// marks sound indexes that are used by custom weapons. Thes need to be sent to clients
	// so that they know which sound path to use, given an index. The client has no API
	// for playing a sound by index with a specific pitch.
	static bool m_customWeaponSounds[MAX_PRECACHE_SOUND];
};