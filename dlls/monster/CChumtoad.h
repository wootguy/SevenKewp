#pragma once
#include "extdll.h"
#include "util.h"
#include "monsters.h"

#define TOXIC_SOUND "ambience/disgusting.wav"
#define TOXIC_SOUND2 "doors/aliendoor1.wav"
#define TOXIC_SPRITE "sprites/puff1.spr"
#define FOLLOW_SOUND "chumtoad/follow.wav"
#define UNFOLLOW_SOUND "chumtoad/unfollow.wav"

#define TOXIC_START_DISTANCE 128

#define SMOKE_TIME 3.0f

enum
{
	TASK_START_CLOUD = LAST_COMMON_TASK + 1,
	TASK_STOP_CLOUD,
};

class EXPORT CChumtoad : public CBaseMonster
{
public:
	void Spawn(void) override;
	void Precache(void) override;
	int Classify(void) override;
	const char* DisplayName() override;
	void SetYawSpeed(void) override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void PrescheduleThink() override;
	void StartTask(Task_t* pTask) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	const char* GetTaskName(int taskIdx) override;
	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }

	BOOL CheckRangeAttack1(float flDot, float flDist) override { return FALSE; }
	BOOL CheckRangeAttack2(float flDot, float flDist) override { return FALSE; }
	BOOL CheckMeleeAttack1(float flDot, float flDist) override;

	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;

	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	CUSTOM_SCHEDULES;

	float nextCloudEmit;
	int smokeColor;
	bool stopSmoking;
	int m_iSmokeSpr;
};
