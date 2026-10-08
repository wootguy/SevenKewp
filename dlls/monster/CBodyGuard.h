#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseGrunt.h"

#define MP5_CLIP_SIZE				36 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define GLOCK_CLIP_SIZE				9
#define DEAGLE_CLIP_SIZE			7
#define UZI_CLIP_SIZE				32
#define SHOTGUN_CLIP_SIZE			8
#define SNIPER_CLIP_SIZE			1

#define BG_RELOAD_EVENT 2
#define BG_SHOOT_PISTOL_EVENT 3
#define BG_SHOOT_UZIS_EVENT1 4
#define BG_SHOOT_UZIS_EVENT2 5
#define BG_SHOOT_UZIS_EVENT3 6
#define BG_SHOOT_UZIS_EVENT4 8
#define BG_DROP_WEAPON 2001

#define BODYGUARD_SENTENCE_VOLUME 1.0f

class EXPORT CBodyGuard: public CBaseGrunt
{
public:
	void Spawn(void) override;
	void Precache(void) override;
	void InitAiFlags() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	void GibMonster(void) override;
	void Killed(entvars_t* pevAttacker, int iGib) override;
	void ShuffleSoundArrays();

	int GetActivitySequence(Activity NewActivity) override;
	Schedule_t* GetScheduleOfType(int Type) override;

	CUSTOM_SCHEDULES;

	void PlaySentenceSound(int sentenceType) override;
	void PlaySentence(const char* pszSentence, float duration, float volume, float attenuation) override;
	int	Classify(void) override;
	const char* DisplayName() override;
	int ISoundMask(void) override;
	BOOL NoFriendlyFire(void) override { return TRUE; } // friendly fire is allowed
	void AlertSound() override;
	void PainSound(void) override;
	void DeathSound(void) override;
	void DeclineFollowing() override;
	void TalkInit();
	void OnTaskComplete(Task_t task) override;

	MONSTERSTATE GetIdealState() override
	{
		return CTalkSquadMonster::GetIdealState();
	}

private:
	static const char* pGruntSentences[];
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
	static const char* pShotSounds[];
	static const char* pMadSounds[];
	static const char* pKillSounds[];
	static const char* pIdleSounds[];
	static const char* pOkSounds[];
	static const char* pWaitSounds[];
	static const char* pStopSounds[];
	static const char* pScaredSounds[];
	static const char* pHelloSounds[];
	static const char* pCureSounds[];
	static const char* pQuestionSounds[];
	static const char* pAnswerSounds[];
};
