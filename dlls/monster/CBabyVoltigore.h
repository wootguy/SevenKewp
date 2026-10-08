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
	void Spawn(void) override;
	void Precache(void) override;
	void SetYawSpeed(void) override;
	int Classify(void) override;
	const char* DisplayName() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void StartTask(Task_t* pTask) override;
	int IgnoreConditions(void) override;
	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }

	void PainSound(void) override;
	void AlertSound(void) override;
	void IdleSound(void) override;
	void AttackSound(void);
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;

private:
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pRunSounds[];
};
