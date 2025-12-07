#pragma once
#include "extdll.h"
#include "CBaseMonster.h"

#define EVENT_SLAM 1
#define EVENT_STAFF_SWING 2
#define EVENT_SHOOT 3
#define EVENT_SUMMON_GRUNT 4
#define EVENT_STAFF_STAB 7
#define EVENT_STEP_RIGHT 10
#define EVENT_STEP_LEFT 11

#define MELEE_ATTACK_DISTANCE 96
#define MELEE_CHASE_DISTANCE 300

#define SLAM_CHECK_DISTANCE 150 // how close enemies need to be for a slam to be considered
#define SLAM_ATTACK_RADIUS 300 // radius of the attack
#define SLAM_ATTACK_ENEMY_COUNT 2 // minimum enemies nearby needed to do a slam

#define MAX_BEAM_SHOTS  5
#define TOR_SHOOT_RANGE 4096

#define SUMMON_DISTANCE 256
#define SUMMON_HEIGHT 80
#define MAX_POSSIBLE_CHILDREN 128
#define MAX_ALLOWED_CHILDREN 3

#define SHOCK_WAVE_SPRITE "sprites/shockwave.spr"
#define SHOOT_SOUND "tor/tor-staff-discharge.wav"
#define SHOOT_BEAM_SPRITE "sprites/xenobeam.spr"
#define PORTAL_SPRITE "sprites/exit1.spr"
#define PORTAL_BEAM_SPRITE "sprites/lgtning.spr"
#define PORTAL_SOUND "debris/beamstart8.wav"
#define PORTAL_SOUND2 "debris/beamstart7.wav"
#define SUMMON_SOUND "tor/tor-summon.wav"
#define SUMMON_CLASSNAME "monster_alien_grunt"

int slamSpriteIdx = 0;

class EXPORT CTor : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int Classify(void);
	const char* DisplayName();
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	Schedule_t* GetSchedule(void);
	Schedule_t* GetScheduleOfType(int Type);

	void MonsterThink(void);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack2(float flDot, float flDist);
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckMeleeAttack2(float flDot, float flDist);
	int LookupActivity(int activity);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	void DeathNotice(entvars_t* pevChild);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-24, -24, 0);
		pev->absmax = pev->origin + Vector(24, 24, 88);
	}

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();

	CUSTOM_SCHEDULES;

private:
	void SlamAttack();
	bool GetSummonPos(Vector& pos);
	void StartSummon();
	void SpawnGrunt();

	int shotsFired;
	float nextShoot; // next time allowed to begin shooting
	float nextBeamBurst; // next time a burst shot will be fired
	float nextBeam; // next time a single beam will be fired
	int burstShotsFired; // number of shots fired in the current burst
	float nextSummon; // next time a grunt can be spawned
	float nextSummonSpawn; // time a grunt will be summoned if still doing the summon activity
	bool startedSummon;
	int numChildren;
	string_t summon_cname;
	Vector summonPos;
	Vector beamColor1;
	Vector beamColor2;
	int m_failedMelees; // don't keep meleeing if the player is moving back and forth to avoid it

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pRunSounds[];
	static const char* pSlamSounds[];
};
