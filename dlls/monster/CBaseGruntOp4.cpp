#include "extdll.h"
#include "plane.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "defaultai.h"
#include "animation.h"
#include "weapons.h"
#include "CTalkSquadMonster.h"
#include "CBaseGruntOp4.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
#include "explode.h"

int g_fAllyQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

const char* CBaseGruntOp4::pGruntSentences[] =
{
	"FG_GREN", // grenade scared grunt
	"FG_ALERT", // sees player
	"FG_MONSTER", // sees monster
	"FG_COVER", // running to cover
	"FG_THROW", // about to throw grenade
	"FG_CHARGE",  // running out to get the enemy
	"FG_TAUNT", // say rude things
	"FG_SHOT", // attacked by player
	"FG_MAD", // friendly fire revenge
	"FG_KILL"
};

void CBaseGruntOp4::InitAiFlags(void) {
	canBeMadAtPlayer = true;
	waitForEnemyFire = true;
	runFromHeavyDamage = true;
	canCallMedic = true;
}

void CBaseGruntOp4::PlaySentenceSound(int sentenceType) {
	SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[sentenceType], ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
}

int	CBaseGruntOp4::Classify(void)
{
	// Is Player Ally? works inverted for friendly monsters
	if (m_IsPlayerAlly)
		return CLASS_HUMAN_MILITARY;
	else
		return m_Classify ? CTalkSquadMonster::Classify() : CLASS_PLAYER_ALLY;
}

int CBaseGruntOp4::ISoundMask(void)
{
	return	bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER |
		bits_SOUND_CARCASS |
		bits_SOUND_MEAT |
		bits_SOUND_GARBAGE;
}

BOOL CBaseGruntOp4::NoFriendlyFire(void) {
	return TRUE; // friendly fire is allowed
}

void CBaseGruntOp4::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	//PCV absorbs some damage types
	if ((ptr->iHitgroup == HITGROUP_CHEST || ptr->iHitgroup == HITGROUP_STOMACH)
		&& (bitsDamageType & (DMG_BLAST | DMG_BULLET | DMG_SLASH)))
	{
		flDamage *= 0.5;
	}

	// should probably skip the CBaseGrunt TraceAttack since op4 grunt models have no helmet hitboxes
	CBaseGrunt::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

void CBaseGruntOp4::IdleSound(void)
{
	if (FOkToSpeak() && (g_fAllyQuestion || RANDOM_LONG(0, 1)))
	{
		if (!g_fAllyQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0, 2))
			{
			case 0: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CHECK", ALLY_GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fAllyQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "FG_QUEST", ALLY_GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fAllyQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "FG_IDLE", ALLY_GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fAllyQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CLEAR", ALLY_GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "FG_ANSWER", ALLY_GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fAllyQuestion = 0;
		}
		JustSpoke();
	}
}

void CBaseGruntOp4::AlertSound()
{
	if (m_hEnemy && FOkToSpeak())
	{
		PlaySentence("FG_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_NORM);
	}
}

void CBaseGruntOp4::PainSound(void)
{
	if (gpGlobals->time > m_flNextPainTime)
	{
#if 0
		if (RANDOM_LONG(0, 99) < 5)
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "FG_PAIN", ALLY_GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
				JustSpoke();
				return;
			}
		}
#endif 
		switch (RANDOM_LONG(0, 7))
		{
		case 0:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain3.wav", 1, ATTN_NORM);
			break;
		case 1:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain4.wav", 1, ATTN_NORM);
			break;
		case 2:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain5.wav", 1, ATTN_NORM);
			break;
		case 3:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain1.wav", 1, ATTN_NORM);
			break;
		case 4:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain2.wav", 1, ATTN_NORM);
			break;
		case 5:
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/pain6.wav", 1, ATTN_NORM);
			break;
		}

		m_flNextPainTime = gpGlobals->time + 1;
	}
}

void CBaseGruntOp4::DeathSound(void)
{
	switch (RANDOM_LONG(0, 5))
	{
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death1.wav", 1, ATTN_IDLE);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death2.wav", 1, ATTN_IDLE);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death3.wav", 1, ATTN_IDLE);
		break;
	case 3:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death4.wav", 1, ATTN_IDLE);
		break;
	case 4:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death5.wav", 1, ATTN_IDLE);
		break;
	case 5:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "fgrunt/death6.wav", 1, ATTN_IDLE);
		break;
	}
}

void CBaseGruntOp4::DeclineFollowing()
{
	PlaySentence("FG_POK", 2, VOL_NORM, ATTN_NORM);
}

void CBaseGruntOp4::Precache()
{
	PRECACHE_SOUND("fgrunt/death1.wav");
	PRECACHE_SOUND("fgrunt/death2.wav");
	PRECACHE_SOUND("fgrunt/death3.wav");
	PRECACHE_SOUND("fgrunt/death4.wav");
	PRECACHE_SOUND("fgrunt/death5.wav");
	PRECACHE_SOUND("fgrunt/death6.wav");

	PRECACHE_SOUND("fgrunt/pain1.wav");
	PRECACHE_SOUND("fgrunt/pain2.wav");
	PRECACHE_SOUND("fgrunt/pain3.wav");
	PRECACHE_SOUND("fgrunt/pain4.wav");
	PRECACHE_SOUND("fgrunt/pain5.wav");
	PRECACHE_SOUND("fgrunt/pain6.wav");

	PRECACHE_SOUND("fgrunt/medic.wav");

	BasePrecache();

	TalkInit();
}

void CBaseGruntOp4::TalkInit()
{
	CTalkSquadMonster::TalkInit();

	m_szGrp[TLK_ANSWER] = "FG_ANSWER";
	m_szGrp[TLK_QUESTION] = "FG_QUESTION";
	m_szGrp[TLK_IDLE] = "FG_IDLE";
	m_szGrp[TLK_STARE] = "FG_STARE";
	m_szGrp[TLK_USE] = "FG_OK";
	m_szGrp[TLK_UNUSE] = "FG_WAIT";
	m_szGrp[TLK_STOP] = "FG_STOP";

	m_szGrp[TLK_NOSHOOT] = "FG_SCARED";
	m_szGrp[TLK_HELLO] = "FG_HELLO";

	m_szGrp[TLK_PLHURT1] = "!FG_CUREA";
	m_szGrp[TLK_PLHURT2] = "!FG_CUREB";
	m_szGrp[TLK_PLHURT3] = "!FG_CUREC";

	m_szGrp[TLK_PHELLO] = NULL;	//"BA_PHELLO";		// UNDONE
	m_szGrp[TLK_PIDLE] = NULL;	//"BA_PIDLE";			// UNDONE
	m_szGrp[TLK_PQUESTION] = "FG_PQUEST";		// UNDONE

	m_szGrp[TLK_SMELL] = "FG_SMELL";

	m_szGrp[TLK_WOUND] = "FG_WOUND";
	m_szGrp[TLK_MORTAL] = "FG_MORTAL";

	// get voice for head - just one barney voice for now
	m_voicePitch = 100;
}

int CBaseGruntOp4::ObjectCaps()
{
	return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
}
