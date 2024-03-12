#include "CBaseGrunt.h"

#define HAS_SHOTGUN_FLAG 8
//#define	ALLY_GRUNT_SENTENCE_VOLUME	(float)0.35 // volume of grunt sentences
#define	ALLY_GRUNT_SENTENCE_VOLUME	(float)1.0 // volume of grunt sentences

class CBaseGruntOp4 : public CBaseGrunt {
public:
	void InitAiFlags(void);
	int	Classify(void);
	int ISoundMask(void);
	BOOL NoFriendlyFire(void);
	void PlaySentenceSound(int sentenceType);
	virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	void IdleSound(void);
	void AlertSound();
	void PainSound(void);
	void DeathSound(void);
	void DeclineFollowing();
	virtual void Precache();
	void TalkInit();
	virtual int ObjectCaps();
	void PlaySentence(const char* pszSentence, float duration, float volume, float attenuation);

private:
	static const char* pGruntSentences[];
	static const char* pPainSounds[];
	static const char* pDieSounds[];
	static const char* pGrenSounds[];
	static const char* pAlertSounds[];
	static const char* pMonsterSounds[];
	static const char* pCoverSounds[];
	static const char* pThrowSounds[];
	static const char* pChargeSounds[];
	static const char* pTauntSounds[];
	static const char* pShotSounds[];
	static const char* pMadSounds[];
	static const char* pKillSounds[];
	static const char* pAnswerSounds[];
	static const char* pQuestionSounds[];
	static const char* pIdleSounds[];
	static const char* pOkSounds[];
	static const char* pWaitSounds[];
	static const char* pStopSounds[];
	static const char* pScaredSounds[];
	static const char* pHelloSounds[];
	static const char* pCureSounds[];
	static const char* pWoundSounds[];
	static const char* pMortalSounds[];
	static const char* pAttackSounds[];
	static const char* pPokSounds[];
	static const char* pCheckSounds[];
	static const char* pClearSounds[];
};