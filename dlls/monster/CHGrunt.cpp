#include "extdll.h"
#include "plane.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "CTalkSquadMonster.h"
#include "weapons.h"
#include "CTalkSquadMonster.h"
#include "env/CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
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

#define HGRUNT_9MMAR				( 1 << 0)
#define HGRUNT_HANDGRENADE			( 1 << 1)
#define HGRUNT_GRENADELAUNCHER		( 1 << 2)
#define HGRUNT_SHOTGUN				( 1 << 3)

#define	SENTENCE_VOLUME (float)0.35 // volume of grunt sentences

class CHGrunt : public CBaseGrunt
{
public:
	void Spawn(void);
	void Precache(void);
	int Classify(void);
	const char* DisplayName();
	void PainSound(void);
	void DeathSound(void);
	void IdleSound(void);
	void PlaySentenceSound(int sentenceType);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	void StartMonster(void);

private:
	static const char* pPainSounds[];
	static const char* pDeathSounds[];
	static const char* pGruntSentences[];
};

class CHGruntRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_human_grunt"; };
};

class CDeadHGrunt : public CBaseDead
{
public:
	void Spawn(void);
	int	Classify(void) { return	CBaseMonster::Classify(CLASS_HUMAN_MILITARY); }
	int GetPoseSequence() { return LookupSequence(m_szPoses[m_iPose]); }

	static const char* m_szPoses[3];
};

LINK_ENTITY_TO_CLASS(monster_human_grunt, CHGrunt);
LINK_ENTITY_TO_CLASS(monster_grunt_repel, CHGruntRepel);
LINK_ENTITY_TO_CLASS(monster_hgrunt_dead, CDeadHGrunt);

const char* CHGrunt::pPainSounds[] =
{
	"hgrunt/gr_pain1.wav",
	"hgrunt/gr_pain2.wav",
	"hgrunt/gr_pain3.wav",
	"hgrunt/gr_pain4.wav",
	"hgrunt/gr_pain5.wav"
};

const char* CHGrunt::pDeathSounds[] =
{
	"hgrunt/gr_die1.wav",
	"hgrunt/gr_pain2.wav",
	"hgrunt/gr_pain3.wav"
};

const char* CHGrunt::pGruntSentences[] =
{
	"HG_GREN", // grenade scared grunt
	"HG_ALERT", // sees player
	"HG_MONSTER", // sees monster
	"HG_COVER", // running to cover
	"HG_THROW", // about to throw grenade
	"HG_CHARGE",  // running out to get the enemy
	"HG_TAUNT", // say rude things
};

const char* CDeadHGrunt::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

void CHGrunt::Spawn() {
	BaseSpawn();

	if (FBitSet(pev->weapons, HGRUNT_SHOTGUN))
	{
		SetBodygroup(GUN_GROUP, GUN_SHOTGUN);
		m_cClipSize = 8;
	}
	else
	{
		m_cClipSize = GRUNT_CLIP_SIZE;
	}
	m_cAmmoLoaded = m_cClipSize;

	if (RANDOM_LONG(0, 99) < 80)
		pev->skin = 0;	// light skin
	else
		pev->skin = 1;	// dark skin

	if (FBitSet(pev->weapons, HGRUNT_SHOTGUN))
	{
		SetBodygroup(HEAD_GROUP, HEAD_SHOTGUN);
	}
	else if (FBitSet(pev->weapons, HGRUNT_GRENADELAUNCHER))
	{
		SetBodygroup(HEAD_GROUP, HEAD_M203);
		pev->skin = 1; // alway dark skin
	}
}

void CHGrunt::Precache()
{
	// get voice pitch
	if (RANDOM_LONG(0, 1))
		m_voicePitch = 109 + RANDOM_LONG(0, 7);
	else
		m_voicePitch = 100;

	if (pev->weapons == 0)
	{
		// initialize to original values
		pev->weapons = HGRUNT_9MMAR | HGRUNT_HANDGRENADE;
		// pev->weapons = HGRUNT_SHOTGUN;
		// pev->weapons = HGRUNT_9MMAR | HGRUNT_GRENADELAUNCHER;
	}

	// set base equipment flags
	if (FBitSet(pev->weapons, HGRUNT_9MMAR)) {
		m_iEquipment |= MEQUIP_MP5;
	}
	if (FBitSet(pev->weapons, HGRUNT_SHOTGUN)) {
		m_iEquipment |= MEQUIP_SHOTGUN;
	}
	if (FBitSet(pev->weapons, HGRUNT_HANDGRENADE)) {
		m_iEquipment |= MEQUIP_HAND_GRENADE;
	}
	if (FBitSet(pev->weapons, HGRUNT_GRENADELAUNCHER)) {
		m_iEquipment |= MEQUIP_GRENADE_LAUNCHER;
	}
	if (GetBodygroup(HEAD_GROUP) == HEAD_GRUNT) {
		m_iEquipment |= MEQUIP_HELMET;
	}

	BasePrecache();

	m_defaultModel = "models/hgrunt.mdl";
	PRECACHE_MODEL(GetModel());

	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
}

void CHGrunt::StartMonster(void)
{
	CTalkSquadMonster::StartMonster();

	if (IsLeader())
	{
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		pev->skin = 0;
		m_iEquipment &= ~MEQUIP_HELMET;
	}
}

int	CHGrunt::Classify(void)
{
	return	CBaseMonster::Classify(CLASS_HUMAN_MILITARY);
}

const char* CHGrunt::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Human Grunt";
}

void CHGrunt::PainSound(void)
{
	if (gpGlobals->time > m_flNextPainTime)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1, ATTN_NORM);
		m_flNextPainTime = gpGlobals->time + 1;
	}
}

void CHGrunt::DeathSound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1, ATTN_NORM);
}

void CHGrunt::IdleSound(void)
{
	if (FOkToSpeak() && (g_fGruntQuestion || RANDOM_LONG(0, 1)))
	{
		if (!g_fGruntQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0, 2))
			{
			case 0: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "HG_CHECK", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "HG_QUEST", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "HG_IDLE", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fGruntQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "HG_CLEAR", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "HG_ANSWER", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

void CHGrunt::PlaySentenceSound(int sentenceType) {
	if (sentenceType >= ARRAYSIZE(pGruntSentences)) {
		return;
	}
	SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[sentenceType], SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
}

void CHGrunt::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;

	switch (pEvent->event)
	{
	case HGRUNT_AE_DROP_GUN:
		if (DropEquipment(0, false))
			SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	default:
		CBaseGrunt::HandleAnimEvent(pEvent);
		break;
	}
}

void CDeadHGrunt::Spawn(void)
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
