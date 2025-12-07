#pragma once
#include "extdll.h"
#include "CBaseGrunt.h"

#define GUN_GROUP					2
#define GUN_MP5						0
#define GUN_SHOTGUN					1
#define GUN_NONE					2
#define GUN_ROCKETLAUNCHER			3
#define GUN_SNIPERRIFLE				4

#define HGRUNT_9MMAR				( 1 << 0)
#define HGRUNT_HANDGRENADE			( 1 << 1)
#define HGRUNT_GRENADELAUNCHER		( 1 << 2)
#define HGRUNT_SHOTGUN				( 1 << 3)

#define	SENTENCE_VOLUME (float)0.35 // volume of grunt sentences

#define RGRUNT_FOLLOW_SOUND "buttons/button3.wav"
#define RGRUNT_UNFOLLOW_SOUND "buttons/button2.wav"

class EXPORT CRoboGrunt : public CBaseGrunt
{
public:
	void Spawn(void);
	void Precache(void);
	int Classify(void);
	const char* DisplayName();
	BOOL IsMachine() { return 1; } // ignore classification overrides
	void RunTask(Task_t* pTask);
	int BloodColor(void) { return DONT_BLEED; }
	void GibMonster(void);
	void Killed(entvars_t* pevAttacker, int iGib);
	void ExplodeThink(void);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	void DeathSound(void);
	void IdleSound(void);
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();
	void PlaySentenceSound(int sentenceType);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	const char* GetDeathNoticeWeapon();

	float m_explodeTime;
	int m_iHeadshotSpr;
	bool m_didExplosion;

private:
	static const char* pDeathSounds[];
	static const char* pGruntSentences[];

	// using sounds instead of sentences for compatibility with vanilla half-life clients
	static const char* pAlertSounds[];
	static const char* pAnswerSounds[];
	static const char* pChargeSounds[];
	static const char* pCheckSounds[];
	static const char* pClearSounds[];
	static const char* pCoverSounds[];
	static const char* pGrenSounds[];
	static const char* pMonstSounds[];
	static const char* pQuestSounds[];
	static const char* pTauntSounds[];
	static const char* pThrowSounds[];
};

class CRoboGruntRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_robogrunt"; };
};
