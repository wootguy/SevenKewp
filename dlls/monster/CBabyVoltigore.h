#pragma once
#include "extdll.h"
#include "monsters.h"

#define EVENT_SLASH_BOTH 12
#define EVENT_SLASH_RIGHT 13
#define EVENT_RUN_SOUND 14

#define MELEE_ATTACK1_DISTANCE 64

#define BASE_SOUND_PITCH 180

class EXPORT CBabyVoltigore : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int Classify(void);
	const char* DisplayName();
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	Schedule_t* GetScheduleOfType(int Type);
	int IgnoreConditions(void);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();

private:
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pRunSounds[];
};
