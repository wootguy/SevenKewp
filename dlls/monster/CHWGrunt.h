#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseGrunt.h"

#define HWGRUNT_GUN_GROUP					1

#define HWGRUNT_GUN_MINIGUN					0
#define HWGRUNT_GUN_GLOCK					1
#define HWGRUNT_GUN_DEAGLE					2
#define HWGRUNT_GUN_357						3
#define HWGRUNT_GUN_NONE					4

#define HWGRUNT_GLOCK_CLIP_SIZE				9
#define HWGRUNT_DEAGLE_CLIP_SIZE			7
#define HWGRUNT_PYTHON_CLIP_SIZE			6

#define HWGRUNT_EVENT_SHOOT_PISTOL 1
#define HWGRUNT_EVENT_PICKUP_MINIGUN 12

#define HWGRUNT_MINIGUN_FIND_DISTANCE 1024

#define HWGRUNT_RANDOM_PISTOL 7

enum
{
	SCHED_HWGRUNT_FIND_MINIGUN = LAST_BASE_GRUNT_SCHEDULE + 1,
	SCHED_HWGRUNT_FIND_MINIGUN_FAIL,
	LAST_HWGRUNT_SCHEDULE
};

enum
{
	TASK_GET_PATH_TO_BEST_MINIGUN = LAST_BASE_GRUNT_TASK + 1,
	LAST_HWGRUNT_TASK
};

class EXPORT CHWGrunt : public CBaseGrunt
{
public:
	void Spawn(void);
	void Precache(void);
	int  Classify(void);
	const char* DisplayName();
	void InitAiFlags();
	void PainSound(void);
	void DeathSound(void);
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();
	void Killed(entvars_t* pevAttacker, int iGib);
	void PlaySentenceSound(int sentenceType);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	Schedule_t* GetScheduleOfType(int Type);
	const char* GetTaskName(int taskIdx);
	int GetActivitySequence(Activity NewActivity);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	int LookupActivity(int activity);
	Schedule_t* GetMonsterStateSchedule(void);
	void StartTask(Task_t* pTask);
	void RunTask(Task_t* pTask);
	CBaseEntity* PBestMinigun(void);
	void DropMinigun(Vector vecDir);
	void PickupMinigun();

	CUSTOM_SCHEDULES;

private:
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
	static const char* pGruntSentences[];

	int secondaryClipSize;
	int secondaryBody;
	float nextFindMinigunTime; // next time grunt is allowed to find a minigun
};

class CHWGruntRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_hwgrunt"; };
};
