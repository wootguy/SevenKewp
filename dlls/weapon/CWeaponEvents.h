#pragma once
#include "custom_weapon.h"
#include "Platform.h"

#ifdef CLIENT_DLL
#include "../common/pmtrace.h"
#include "beamdef.h"
struct tempent_s;
typedef struct tempent_s TEMPENTITY;
#endif

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

#define WC_SERVER_EVENT_QUEUE_SZ 32
#define MAX_WC_BEAMS 16

#ifdef CLIENT_DLL
WcTrace ConvertTrace(pmtrace_t tr);
#else
WcTrace ConvertTrace(TraceResult tr);
#endif

class CWeaponCustom;

class EXPORT CWeaponEvents {
public:
	CWeaponCustom* m_weapon;
	WcDelayEvent eventQueue[WC_SERVER_EVENT_QUEUE_SZ]; // for playing server-side events with a delay
	
	WcBeam m_beams[MAX_WC_BEAMS];
	WcSprite m_beamImpactSprite;

	int animCount; // counter for ordered weapon aimations
	int m_bulletFireCount; // for odd/even effects (m_iClip is unreliable)

	static int m_tracerCount[32];

	CWeaponEvents() {}

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

	float GetCurrentAccuracyMultiplier();
	void GetCurrentAccuracy(float& accuracyX, float& accuracyY, float& accuracyX2, float& accuracyY2);
	int GetImpactArg(int attackIdx, bool impactMonster, bool impactWorld);
	float GetChargeMult(WepEvt& evt, int flagMask);
	Vector BeamAttack(WcBeam& beam, CBasePlayer* m_pPlayer);
	void FireAmmoEvents(int ammoPool);
	WcBeam* AllocBeam();
	void UpdateBeams();
	bool KillBeams(int attackIdx=-1); // set attack idx to only kill constant beams attached to an attack button. True if any beams killed
	bool CheckTracer(int idx, Vector& vecSrc, Vector forward, Vector right, int iTracerFreq);
	void QuakeMuzzleFlash(CBasePlayer* plr);
};