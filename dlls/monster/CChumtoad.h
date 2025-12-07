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
	void Spawn(void);
	void Precache(void);
	int Classify(void);
	const char* DisplayName();
	void SetYawSpeed(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	void PrescheduleThink();
	void StartTask(Task_t* pTask);
	Schedule_t* GetScheduleOfType(int Type);
	const char* GetTaskName(int taskIdx);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	BOOL CheckRangeAttack1(float flDot, float flDist) { return FALSE; }
	BOOL CheckRangeAttack2(float flDot, float flDist) { return FALSE; }
	BOOL CheckMeleeAttack1(float flDot, float flDist);

	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();

	CUSTOM_SCHEDULES;

	float nextCloudEmit;
	int smokeColor;
	bool stopSmoking;
	int m_iSmokeSpr;
};
