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

	static const char* pGruntSentences[];
};