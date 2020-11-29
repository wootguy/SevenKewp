#include	"extdll.h"
#include	"plane.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"animation.h"
#include	"CTalkSquadMonster.h"
#include	"weapons.h"
#include	"CTalkSquadMonster.h"
#include	"env/CSoundEnt.h"
#include	"effects.h"
#include	"customentity.h"
#include	"CBaseHGrunt.h"

#define	SENTENCE_VOLUME (float)0.8 // volume of grunt sentences

class CHGruntOp4 : public CBaseHGrunt
{
public:
	void Spawn(void);
	void Precache(void);
	int	Classify(void);
	void PainSound(void);
	void DeathSound(void);
	void IdleSound(void);
	void PlaySentenceSound(int sentenceType);

	int	ObjectCaps(void) { return CBaseHGrunt::ObjectCaps() | FCAP_IMPULSE_USE; }

	void StartFollowingSound() {
		SENTENCEG_PlayRndSz(ENT(pev), "FG_OK", SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
	}
	void StopFollowingSound() {
		SENTENCEG_PlayRndSz(ENT(pev), "FG_WAIT", SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
	}
	void CantFollowSound() {
		SENTENCEG_PlayRndSz(ENT(pev), "FG_STOP", SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
	}


private:
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
	static const char* pGruntSentences[];
};

class CHGruntOp4Repel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_human_grunt_ally"; };
};

class CDeadHGruntOp4 : public CBaseDead
{
public:
	void Spawn(void);
	int	Classify(void) { return	CLASS_HUMAN_MILITARY; }
	int GetPoseSequence() { return LookupSequence(m_szPoses[m_iPose]); }

	static const char* m_szPoses[3];
};

// Disabled this class in favor of Solokiller's ally grunt code
// https://github.com/Solokiller/halflife-op4/tree/dev/dlls
//LINK_ENTITY_TO_CLASS(monster_human_grunt_ally, CHGruntOp4);
//LINK_ENTITY_TO_CLASS(monster_grunt_ally_repel, CHGruntRepel);
//LINK_ENTITY_TO_CLASS(monster_grunt_ally_dead, CDeadHGruntOp4);

const char* CHGruntOp4::pPainSounds[] =
{
	"fgrunt/pain1.wav",
	"fgrunt/pain2.wav",
	"fgrunt/pain3.wav",
	"fgrunt/pain4.wav",
	"fgrunt/pain5.wav"
};

const char* CHGruntOp4::pDeathSounds[] =
{
	"fgrunt/death1.wav",
	"fgrunt/death2.wav",
	"fgrunt/death3.wav",
	"fgrunt/death4.wav",
	"fgrunt/death5.wav",
	"fgrunt/death6.wav"
};

const char* CHGruntOp4::pGruntSentences[] =
{
	"FG_GREN", // grenade scared grunt
	"FG_ALERT", // sees player
	"FG_MONSTER", // sees monster
	"FG_COVER", // running to cover
	"FG_THROW", // about to throw grenade
	"FG_CHARGE",  // running out to get the enemy
	"FG_TAUNT", // say rude things
};

const char* CDeadHGruntOp4::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

void CHGruntOp4::Spawn() {
	BaseSpawn("models/hgrunt_opforf.mdl");

	// get voice pitch
	if (RANDOM_LONG(0, 1))
		m_voicePitch = 95 - RANDOM_LONG(0, 7);
	else
		m_voicePitch = 100;
}

void CHGruntOp4::Precache()
{
	PRECACHE_MODEL("models/hgrunt_opforf.mdl");

	for (int i = 0; i < ARRAYSIZE(pPainSounds); i++)
		PRECACHE_SOUND((char*)pPainSounds[i]);

	for (int i = 0; i < ARRAYSIZE(pDeathSounds); i++)
		PRECACHE_SOUND((char*)pDeathSounds[i]);

	BasePrecache();
}

int	CHGruntOp4::Classify(void)
{
	return CLASS_PLAYER_ALLY;
}

void CHGruntOp4::PainSound(void)
{
	if (gpGlobals->time > m_flNextPainTime)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds) - 1)], 1, ATTN_NORM);
		m_flNextPainTime = gpGlobals->time + 1;
	}
}

void CHGruntOp4::DeathSound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, pDeathSounds[RANDOM_LONG(0, ARRAYSIZE(pDeathSounds) - 1)], 1, ATTN_NORM);
}

void CHGruntOp4::IdleSound(void)
{
	if (FOkToSpeak() && (g_fGruntQuestion || RANDOM_LONG(0, 1)))
	{
		if (!g_fGruntQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0, 2))
			{
			case 0: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CHECK", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "FG_QUEST", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "FG_IDLE", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fGruntQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CLEAR", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question
				SENTENCEG_PlayRndSz(ENT(pev), "FG_ANSWER", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

void CHGruntOp4::PlaySentenceSound(int sentenceType) {
	SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[sentenceType], SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
}


void CDeadHGruntOp4::Spawn(void)
{
	CBaseDead::BaseSpawn("models/hgrunt.mdl");
	m_bloodColor = BLOOD_COLOR_RED;

	// map old bodies onto new bodies
	switch (pev->body)
	{
	case 0: // Grunt with Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 1: // Commander with Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 2: // Grunt no Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	case 3: // Commander no Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	}
}