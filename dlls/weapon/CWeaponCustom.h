#pragma once
#include "CBasePlayerWeapon.h"
#include "custom_weapon.h"

// marks sound indexes that are used by custom weapons. Thes need to be sent to clients
// so that they know which sound path to use, given an index. The client has no API
// for playing a sound by index with a specific pitch.
extern bool g_customWeaponSounds[MAX_PRECACHE_SOUND];

class CWeaponCustom : public CBasePlayerWeapon {
public:
	CustomWeaponParams params;

	float m_flReloadStart;
	float m_flReloadEnd; // for preventing reload loops
	bool m_bInReload; // like m_fInReload but allows the weapon to think during the reload
	const char* animExt;
	const char* wrongClientWeapon; // weapon given to players who don't have the correct client to use this one

	void Spawn() override;
	void Precache() override;
	void PrecacheEvents() override;
	void AddEvent(WepEvt evt);
	BOOL UseDecrement(void) override { return TRUE; }
	BOOL IsSevenKewpWeapon() { return TRUE; }
	int AddToPlayer(CBasePlayer* pPlayer) override;
	BOOL Deploy() override;
	void Reload() override;
	void WeaponIdle() override;
	void ItemPostFrame() override;

	void PrimaryAttack(void) override;
	void SecondaryAttack(void) override;
	bool CommonAttack(int attackIdx); // true if attacked
	void Cooldown(int millis);
	bool CheckTracer(int idx, Vector& vecSrc, Vector forward, Vector right, int iTracerFreq);
	Vector ProcessBulletEvent(WepEvt& evt, CBasePlayer* m_pPlayer);
	void ProcessKickbackEvent(WepEvt& evt, CBasePlayer* m_pPlayer);
	void ProcessSoundEvent(WepEvt& evt, CBasePlayer* m_pPlayer);
	void ProcessEvents(int trigger, int triggerArg);
	void SendPredictionData(edict_t* target);

	// get all HL clients as a bitfield for sound messages. Excludes the given player.
	// This is used to send sounds to clients that can't play custom sevenkewp events.
	static uint32_t GetOtherHlClients(edict_t* plr);

	// same event used for all custom weapons
	static void PrecacheEvent();
	static int m_usCustom;
	static char m_soundPaths[MAX_PRECACHE][256];
	static int m_tracerCount[32];
};