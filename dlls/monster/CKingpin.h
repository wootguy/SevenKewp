#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseMonster.h"

#define	KINGPIN_AE_SWING_RIGHT 1
#define	KINGPIN_AE_SWING_LEFT 2
#define	KINGPIN_AE_CONJURE_ORB 3
#define	KINGPIN_AE_SHOOT_ORB 4

#define KINGPIN_MELEE_DISTANCE 96
#define KINGPIN_MELEE_CHASE_DISTANCE 300

#define KINGPIN_SHOOT_RANGE 2048

#define KINGPIN_BEAM_SOUND "debris/beamstart10.wav"
#define KINGPIN_BEAM_SPRITE "sprites/zbeam2.spr"
#define KINGPIN_FLARE_SPRITE "sprites/flare1.spr"
#define KINGPIN_EYE_SPRITE "sprites/boss_glow.spr"
#define KINGPIN_TELE_SPRITE "sprites/b-tele1.spr"
#define KINGPIN_TELE_SOUND_IN "ambience/port_suckout1.wav" // yes, in.wav makes more sense for out
#define KINGPIN_TELE_SOUND_OUT "ambience/port_suckin1.wav"

#define ORB_ATTACK_RADIUS 300 // radius of the attack
#define ORB_SPRITE "sprites/nhth1.spr"
#define ORB_TRAIL_SPRITE "sprites/laserbeam.spr"
#define ORB_GROW_SOUND "debris/beamstart1.wav"
#define ORB_MOVE_SOUND "x/x_teleattack1.wav"
#define ORB_EXPLODE_SOUND "tor/tor-staff-discharge.wav"
#define SHOCK_WAVE_SPRITE "sprites/shockwave.spr"

class EXPORT CKingpinBall : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	int	Classify(void);
	void HuntThink(void);
	void ExplodeTouch(CBaseEntity* pOther);
	void MovetoTarget(Vector vecTarget);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }
	virtual BOOL IsNormalMonster() { return FALSE; }
	void Activate();
	void UpdateOnRemove(void);
	int m_iTrail;
	EHANDLE m_hOwner;
	Vector m_lastDir;
	bool m_isActive;
	bool m_animate;

	Vector m_velocity; // for interpolation (velocity does not interpolate even with EFLAG_SLERP)
	float m_lastThink;
	int m_spriteFrames;
	float m_powerlevel;
};


struct kingpin_eye_t {
	EHANDLE h_spr;
	EHANDLE h_flare;
	float lastAttack;
	int iAttachment;
	float angle;
};

class EXPORT CKingpin : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int Classify(void);
	const char* DisplayName();
	const char* GetTaskName(int taskIdx);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	Schedule_t* GetSchedule(void);
	Schedule_t* GetScheduleOfType(int Type);
	void ScheduleChange(void);
	void UpdateOnRemove(void);
	void CancelOrb();
	void GibMonster();
	void LaserEyesThink();
	void ProjectileDeflectThink();
	void MonsterThink(void);
	void StartTask(Task_t* pTask);
	void RunTask(Task_t* pTask);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack2(float flDot, float flDist);
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckMeleeAttack2(float flDot, float flDist);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-56, -56, 0);
		pev->absmax = pev->origin + Vector(56, 56, 116);
	}

	void PainSound(void);
	void DeathSound(void);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();

	CUSTOM_SCHEDULES;

private:
	float m_nextBeam; // next time a single beam will be fired
	float m_nextOrb;
	float m_nextTele;
	Vector m_beamColor;
	EHANDLE m_orb;

	Vector m_teleportSrc;
	Vector m_teleportDst;
	float m_teleportTime; // time teleport effect started
	int m_teleportPhase;

	kingpin_eye_t m_eyes[4];
	float m_lastBeam;

	int m_iSpitSprite;
	int m_numAttachments;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pDieSounds[];
};
