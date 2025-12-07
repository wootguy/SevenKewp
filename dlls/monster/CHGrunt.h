#include "extdll.h"
#include "CBaseGrunt.h"

#define HEAD_GROUP					1
#define HEAD_GRUNT					0
#define HEAD_COMMANDER				1
#define HEAD_SHOTGUN				2
#define HEAD_M203					3
#define GUN_GROUP					2
#define GUN_MP5						0
#define GUN_SHOTGUN					1
#define GUN_NONE					2
#define GUN_ROCKETLAUNCHER			3
#define GUN_SNIPERRIFLE				4

class EXPORT CHGrunt : public CBaseGrunt
{
public:
	void Spawn(void);
	void Precache(void);
	int Classify(void);
	const char* DisplayName();
	void PainSound(void);
	void DeathSound(void);
	void IdleSound(void);
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();
	void PlaySentenceSound(int sentenceType);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	void StartMonster(void);

private:
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
	static const char* pGruntSentences[];
};

class EXPORT CHGruntRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_human_grunt"; };
};

class EXPORT CDeadHGrunt : public CBaseDead
{
public:
	void Spawn(void);
	int	Classify(void) { return	CBaseMonster::Classify(CLASS_HUMAN_MILITARY); }
	int GetPoseSequence() { return LookupSequence(m_szPoses[clamp(m_iPose, 0, (int)ARRAY_SZ(m_szPoses) - 1)]); }

	static const char* m_szPoses[3];
};
