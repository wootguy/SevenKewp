#include "CBaseGrunt.h"

#define HAS_SHOTGUN_FLAG 8
//#define	ALLY_GRUNT_SENTENCE_VOLUME	(float)0.35 // volume of grunt sentences
#define	ALLY_GRUNT_SENTENCE_VOLUME	(float)1.0 // volume of grunt sentences

class CBaseGruntAlly : public CBaseGrunt {
public:
	int	Classify(void);
	int ISoundMask(void);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	void PlaySentenceSound(int sentenceType);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void IdleSound(void);
	void AlertSound();
	void PainSound(void);
	void DeathSound(void);
	void DeclineFollowing();
	virtual void Precache();
	void TalkInit();
	Schedule_t* GetMonsterStateSchedule(void);
	virtual int ObjectCaps();
	virtual void Killed(entvars_t* pevAttacker, int iGib);


	BOOL m_lastAttackCheck;
	float m_flPlayerDamage;

	static const char* pGruntSentences[];

	bool canHaveGrenadeLauncher; // TODO: every grunt should use the same weapon flags
};