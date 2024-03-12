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

const char* CBaseGruntOp4::pPainSounds[] =
{
	MOD_SND_FOLDER "fgrunt/pain1.wav",
	MOD_SND_FOLDER "fgrunt/pain2.wav",
	MOD_SND_FOLDER "fgrunt/pain3.wav",
	MOD_SND_FOLDER "fgrunt/pain4.wav",
	MOD_SND_FOLDER "fgrunt/pain5.wav",
	MOD_SND_FOLDER "fgrunt/pain6.wav",
};

const char* CBaseGruntOp4::pDieSounds[] =
{
	MOD_SND_FOLDER "fgrunt/death1.wav",
	MOD_SND_FOLDER "fgrunt/death2.wav",
	MOD_SND_FOLDER "fgrunt/death3.wav",
	MOD_SND_FOLDER "fgrunt/death4.wav",
	MOD_SND_FOLDER "fgrunt/death5.wav",
	MOD_SND_FOLDER "fgrunt/death6.wav",
};

const char* CBaseGruntOp4::pGrenSounds[] =
{
	MOD_SND_FOLDER "fgrunt/incoming.wav",
	MOD_SND_FOLDER "fgrunt/grenade.wav",
};

const char* CBaseGruntOp4::pAlertSounds[] =
{
	MOD_SND_FOLDER "fgrunt/alert.wav",
	MOD_SND_FOLDER "fgrunt/down.wav",
	MOD_SND_FOLDER "fgrunt/bogies.wav",
	MOD_SND_FOLDER "fgrunt/hostiles.wav",
};

const char* CBaseGruntOp4::pMonsterSounds[] =
{
	MOD_SND_FOLDER "fgrunt/hostiles.wav",
	MOD_SND_FOLDER "fgrunt/bogies.wav",
};

const char* CBaseGruntOp4::pCoverSounds[] =
{
	MOD_SND_FOLDER "fgrunt/clear.wav",
	MOD_SND_FOLDER "fgrunt/cover.wav",
	MOD_SND_FOLDER "fgrunt/staydown.wav",
};

const char* CBaseGruntOp4::pThrowSounds[] =
{
	MOD_SND_FOLDER "fgrunt/fire.wav",
	MOD_SND_FOLDER "fgrunt/grenade.wav",
};

const char* CBaseGruntOp4::pChargeSounds[] =
{
	MOD_SND_FOLDER "fgrunt/go.wav",
	MOD_SND_FOLDER "fgrunt/move.wav",
	MOD_SND_FOLDER "fgrunt/getsome.wav",
};

const char* CBaseGruntOp4::pTauntSounds[] =
{
	MOD_SND_FOLDER "fgrunt/nothing.wav",
	MOD_SND_FOLDER "fgrunt/getsome.wav",
	MOD_SND_FOLDER "fgrunt/corners.wav",
	MOD_SND_FOLDER "fgrunt/backup.wav",
	MOD_SND_FOLDER "fgrunt/oneshot.wav",
	MOD_SND_FOLDER "fgrunt/mister.wav",
};

const char* CBaseGruntOp4::pShotSounds[] =
{
	MOD_SND_FOLDER "fgrunt/friendly.wav",
	MOD_SND_FOLDER "fgrunt/watchfire.wav",
	MOD_SND_FOLDER "fgrunt/zone.wav",
	MOD_SND_FOLDER "fgrunt/watchit.wav",
	MOD_SND_FOLDER "fgrunt/check.wav",
	MOD_SND_FOLDER "fgrunt/wantsome.wav",
};

const char* CBaseGruntOp4::pMadSounds[] =
{
	MOD_SND_FOLDER "fgrunt/ass.wav",
	MOD_SND_FOLDER "fgrunt/mister.wav",
};

const char* CBaseGruntOp4::pKillSounds[] =
{
	MOD_SND_FOLDER "fgrunt/sniper.wav",
	MOD_SND_FOLDER "fgrunt/tag.wav",
	MOD_SND_FOLDER "fgrunt/corporal.wav",
	MOD_SND_FOLDER "fgrunt/talking.wav",
	MOD_SND_FOLDER "fgrunt/take.wav",
	MOD_SND_FOLDER "fgrunt/area.wav",
	MOD_SND_FOLDER "fgrunt/check.wav",
	MOD_SND_FOLDER "fgrunt/wantsome.wav",
	MOD_SND_FOLDER "fgrunt/oneshot.wav",
};

const char* CBaseGruntOp4::pAnswerSounds[] =
{
	MOD_SND_FOLDER "fgrunt/roger.wav",
	MOD_SND_FOLDER "fgrunt/quiet.wav",
	MOD_SND_FOLDER "fgrunt/no.wav",
};

const char* CBaseGruntOp4::pQuestionSounds[] =
{
	MOD_SND_FOLDER "fgrunt/seensquad.wav",
	MOD_SND_FOLDER "fgrunt/current.wav",
	MOD_SND_FOLDER "fgrunt/coverup.wav",
	MOD_SND_FOLDER "fgrunt/chicken.wav",
	MOD_SND_FOLDER "fgrunt/charge.wav",
	MOD_SND_FOLDER "fgrunt/bfeeling.wav",
};

const char* CBaseGruntOp4::pIdleSounds[] =
{
	MOD_SND_FOLDER "fgrunt/guard.wav",
	MOD_SND_FOLDER "fgrunt/mission.wav",
	MOD_SND_FOLDER "fgrunt/babysitting.wav",
	MOD_SND_FOLDER "fgrunt/coverup.wav",
	MOD_SND_FOLDER "fgrunt/now.wav",
	MOD_SND_FOLDER "fgrunt/short.wav",
	MOD_SND_FOLDER "fgrunt/outof.wav",
	MOD_SND_FOLDER "fgrunt/lost.wav",
	MOD_SND_FOLDER "fgrunt/fubar.wav",
	MOD_SND_FOLDER "fgrunt/dogs.wav",
	MOD_SND_FOLDER "fgrunt/disney.wav",
	MOD_SND_FOLDER "fgrunt/checkrecon.wav",
	MOD_SND_FOLDER "fgrunt/seensquad.wav",
};

const char* CBaseGruntOp4::pOkSounds[] =
{
	MOD_SND_FOLDER "fgrunt/damage.wav",
	MOD_SND_FOLDER "fgrunt/six.wav",
	MOD_SND_FOLDER "fgrunt/moving.wav",
	MOD_SND_FOLDER "fgrunt/gotit.wav",
	MOD_SND_FOLDER "fgrunt/hellout.wav",
	MOD_SND_FOLDER "fgrunt/sir.wav",
	MOD_SND_FOLDER "fgrunt/right.wav",
	MOD_SND_FOLDER "fgrunt/command.wav",
	MOD_SND_FOLDER "fgrunt/notfail.wav",
	MOD_SND_FOLDER "fgrunt/yes.wav",
};

const char* CBaseGruntOp4::pWaitSounds[] =
{
	MOD_SND_FOLDER "fgrunt/secure.wav",
	MOD_SND_FOLDER "fgrunt/orders.wav",
	MOD_SND_FOLDER "fgrunt/scout.wav",
	MOD_SND_FOLDER "fgrunt/guardduty.wav",
};

const char* CBaseGruntOp4::pStopSounds[] =
{
	MOD_SND_FOLDER "fgrunt/post.wav",
	MOD_SND_FOLDER "fgrunt/stay.wav",
	MOD_SND_FOLDER "fgrunt/leave.wav",
};

const char* CBaseGruntOp4::pScaredSounds[] =
{
	MOD_SND_FOLDER "fgrunt/hearsome.wav",
	MOD_SND_FOLDER "fgrunt/hear.wav",
};

const char* CBaseGruntOp4::pHelloSounds[] =
{
	MOD_SND_FOLDER "fgrunt/sir_01.wav",
	MOD_SND_FOLDER "fgrunt/tosee.wav",
	MOD_SND_FOLDER "fgrunt/hellosir.wav",
};

const char* CBaseGruntOp4::pCureSounds[] =
{
	MOD_SND_FOLDER "fgrunt/needmedic.wav",
	MOD_SND_FOLDER "fgrunt/hurt.wav",
	MOD_SND_FOLDER "fgrunt/hell.wav",
	MOD_SND_FOLDER "fgrunt/feel.wav",
};

const char* CBaseGruntOp4::pWoundSounds[] =
{
	MOD_SND_FOLDER "fgrunt/imhit.wav",
	MOD_SND_FOLDER "fgrunt/fwound.wav",
	MOD_SND_FOLDER "fgrunt/makeit.wav",
};

const char* CBaseGruntOp4::pMortalSounds[] =
{
	MOD_SND_FOLDER "fgrunt/hitbad.wav",
	MOD_SND_FOLDER "fgrunt/critical.wav",
	MOD_SND_FOLDER "fgrunt/sdamage.wav",
};

const char* CBaseGruntOp4::pAttackSounds[] =
{
	MOD_SND_FOLDER "fgrunt/flank.wav",
	MOD_SND_FOLDER "fgrunt/suppressing.wav",
	MOD_SND_FOLDER "fgrunt/sweep.wav",
	MOD_SND_FOLDER "fgrunt/wantsome.wav",
	MOD_SND_FOLDER "fgrunt/freaks.wav",
	MOD_SND_FOLDER "fgrunt/recon.wav",
	MOD_SND_FOLDER "fgrunt/rapidfire.wav",
	MOD_SND_FOLDER "fgrunt/marines.wav",
	MOD_SND_FOLDER "fgrunt/moveup.wav",
	MOD_SND_FOLDER "fgrunt/go.wav",
	MOD_SND_FOLDER "fgrunt/covering.wav",
	MOD_SND_FOLDER "fgrunt/ass.wav",
};

const char* CBaseGruntOp4::pPokSounds[] =
{
	MOD_SND_FOLDER "fgrunt/hell.wav",
	MOD_SND_FOLDER "fgrunt/sir_01.wav",
	MOD_SND_FOLDER "fgrunt/hellosir.wav",
	MOD_SND_FOLDER "fgrunt/corporal_01.wav",
};

const char* CBaseGruntOp4::pCheckSounds[] =
{
	MOD_SND_FOLDER "fgrunt/check.wav",
	MOD_SND_FOLDER "fgrunt/zone.wav",
};

const char* CBaseGruntOp4::pClearSounds[] =
{
	MOD_SND_FOLDER "fgrunt/clear.wav",
	MOD_SND_FOLDER "fgrunt/area.wav",
};


void CBaseGruntOp4::InitAiFlags(void) {
	canBeMadAtPlayer = true;
	waitForEnemyFire = true;
	runFromHeavyDamage = true;
	canCallMedic = true;
	maxShootDist = 2048;
}

void CBaseGruntOp4::PlaySentenceSound(int sentenceType) {
	PlaySentence(pGruntSentences[sentenceType], 3.0f, ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN);
	//SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[sentenceType], ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
}

int	CBaseGruntOp4::Classify(void)
{
	return CBaseMonster::Classify(CLASS_PLAYER_ALLY);
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
				PlaySentence("FG_CHECK", 3.0f, ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN);
				g_fAllyQuestion = 1;
				break;
			case 1: // question
				PlaySentence("FG_QUESTION", 3.0f, ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN);
				g_fAllyQuestion = 2;
				break;
			case 2: // statement
				PlaySentence("FG_IDLE", 3.0f, ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN);
				break;
			}
		}
		else
		{
			switch (g_fAllyQuestion)
			{
			case 1: // check in
				PlaySentence("FG_CLEAR", 3.0f, ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN);
				break;
			case 2: // question 
				PlaySentence("FG_ANSWER", 3.0f, ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN);
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
		EMIT_SOUND(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1, ATTN_NORM);

		m_flNextPainTime = gpGlobals->time + 1;
	}
}

void CBaseGruntOp4::DeathSound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDieSounds), 1, ATTN_IDLE);
}

void CBaseGruntOp4::DeclineFollowing()
{
	PlaySentence("FG_POK", 2, VOL_NORM, ATTN_NORM);
}

void CBaseGruntOp4::Precache()
{
	BasePrecache();

	PRECACHE_SOUND_ARRAY(pDieSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pGrenSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pMonsterSounds);
	PRECACHE_SOUND_ARRAY(pCoverSounds);
	PRECACHE_SOUND_ARRAY(pThrowSounds);
	PRECACHE_SOUND_ARRAY(pChargeSounds);
	PRECACHE_SOUND_ARRAY(pTauntSounds);
	PRECACHE_SOUND_ARRAY(pShotSounds);
	PRECACHE_SOUND_ARRAY(pMadSounds);
	PRECACHE_SOUND_ARRAY(pKillSounds);
	PRECACHE_SOUND_ARRAY(pAnswerSounds);
	PRECACHE_SOUND_ARRAY(pQuestionSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pOkSounds);
	PRECACHE_SOUND_ARRAY(pWaitSounds);
	PRECACHE_SOUND_ARRAY(pStopSounds);
	PRECACHE_SOUND_ARRAY(pScaredSounds);
	PRECACHE_SOUND_ARRAY(pHelloSounds);
	PRECACHE_SOUND_ARRAY(pCureSounds);
	PRECACHE_SOUND_ARRAY(pWoundSounds);
	PRECACHE_SOUND_ARRAY(pMortalSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pPokSounds);
	PRECACHE_SOUND_ARRAY(pCheckSounds);
	PRECACHE_SOUND_ARRAY(pClearSounds);

	PRECACHE_SOUND(MOD_SND_FOLDER "fgrunt/medic.wav");

	TalkInit();
}

void CBaseGruntOp4::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation)
{
	if (!pszSentence)
		return;

	Talk(duration);

	CTalkSquadMonster::g_talkWaitTime = gpGlobals->time + duration + 2.0;

	const char* sample = "";

	if (!strcmp(pszSentence, "FG_GREN")) {
		sample = RANDOM_SOUND_ARRAY(pGrenSounds);
	}
	else if (!strcmp(pszSentence, "FG_ALERT")) {
		sample = RANDOM_SOUND_ARRAY(pAlertSounds);
	}
	else if (!strcmp(pszSentence, "FG_MONSTER")) {
		sample = RANDOM_SOUND_ARRAY(pMonsterSounds);
	}
	else if (!strcmp(pszSentence, "FG_COVER")) {
		sample = RANDOM_SOUND_ARRAY(pCoverSounds);
	}
	else if (!strcmp(pszSentence, "FG_THROW")) {
		sample = RANDOM_SOUND_ARRAY(pThrowSounds);
	}
	else if (!strcmp(pszSentence, "FG_CHARGE")) {
		sample = RANDOM_SOUND_ARRAY(pChargeSounds);
	}
	else if (!strcmp(pszSentence, "FG_TAUNT")) {
		sample = RANDOM_SOUND_ARRAY(pTauntSounds);
	}
	else if (!strcmp(pszSentence, "FG_SHOT")) {
		sample = RANDOM_SOUND_ARRAY(pShotSounds);
	}
	else if (!strcmp(pszSentence, "FG_MAD")) {
		sample = RANDOM_SOUND_ARRAY(pMadSounds);
	}
	else if (!strcmp(pszSentence, "FG_KILL")) {
		sample = RANDOM_SOUND_ARRAY(pKillSounds);
	}
	else if (!strcmp(pszSentence, "FG_ANSWER")) {
		sample = RANDOM_SOUND_ARRAY(pAnswerSounds);
	}
	else if (!strcmp(pszSentence, "FG_QUESTION")) {
		sample = RANDOM_SOUND_ARRAY(pQuestionSounds);
	}
	else if (!strcmp(pszSentence, "FG_IDLE")) {
		sample = RANDOM_SOUND_ARRAY(pIdleSounds);
	}
	else if (!strcmp(pszSentence, "FG_OK")) {
		sample = RANDOM_SOUND_ARRAY(pOkSounds);
	}
	else if (!strcmp(pszSentence, "FG_WAIT")) {
		sample = RANDOM_SOUND_ARRAY(pWaitSounds);
	}
	else if (!strcmp(pszSentence, "FG_STOP")) {
		sample = RANDOM_SOUND_ARRAY(pStopSounds);
	}
	else if (!strcmp(pszSentence, "FG_SCARED")) {
		sample = RANDOM_SOUND_ARRAY(pScaredSounds);
	}
	else if (!strcmp(pszSentence, "FG_HELLO")) {
		sample = RANDOM_SOUND_ARRAY(pHelloSounds);
	}
	else if (!strcmp(pszSentence, "!FG_CUREA") || !strcmp(pszSentence, "!FG_CUREB") || !strcmp(pszSentence, "!FG_CUREC")) {
		sample = RANDOM_SOUND_ARRAY(pCureSounds);
	}
	else if (!strcmp(pszSentence, "FG_WOUND")) {
		sample = RANDOM_SOUND_ARRAY(pWoundSounds);
	}
	else if (!strcmp(pszSentence, "FG_MORTAL")) {
		sample = RANDOM_SOUND_ARRAY(pMortalSounds);
	}
	else if (!strcmp(pszSentence, "FG_ATTACK")) {
		sample = RANDOM_SOUND_ARRAY(pAttackSounds);
	}
	else if (!strcmp(pszSentence, "FG_POK")) {
		sample = RANDOM_SOUND_ARRAY(pPokSounds);
	}
	else if (!strcmp(pszSentence, "FG_CHECK")) {
		sample = RANDOM_SOUND_ARRAY(pCheckSounds);
	}
	else if (!strcmp(pszSentence, "FG_CLEAR")) {
		sample = RANDOM_SOUND_ARRAY(pClearSounds);
	}
	else {
		ALERT(at_console, "Invalid sentence: %s\n", pszSentence);
		return;
	}

	EMIT_SOUND_DYN(edict(), CHAN_VOICE, sample, volume, attenuation, 0, GetVoicePitch());

	// If you say anything, don't greet the player - you may have already spoken to them
	SetBits(m_bitsSaid, bit_saidHelloPlayer);
}

void CBaseGruntOp4::TalkInit()
{
	CTalkSquadMonster::TalkInit();

	m_szGrp[TLK_ANSWER] = "FG_ANSWER";
	m_szGrp[TLK_QUESTION] = "FG_QUESTION";
	m_szGrp[TLK_IDLE] = "FG_IDLE";
	m_szGrp[TLK_STARE] = NULL; // "FG_STARE";
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
	m_szGrp[TLK_PQUESTION] = NULL;// "FG_PQUEST";		// UNDONE

	m_szGrp[TLK_SMELL] = NULL; // "FG_SMELL";

	m_szGrp[TLK_WOUND] = "FG_WOUND";
	m_szGrp[TLK_MORTAL] = "FG_MORTAL";

	// get voice for head - just one barney voice for now
	m_voicePitch = 100;
}

int CBaseGruntOp4::ObjectCaps()
{
	return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
}
