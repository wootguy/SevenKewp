#include "extdll.h"
#include "plane.h"
#include "util.h"
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
#include "game.h"

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
	"fgrunt/pain1.wav",
	"fgrunt/pain2.wav",
	"fgrunt/pain3.wav",
	"fgrunt/pain4.wav",
	"fgrunt/pain5.wav",
	"fgrunt/pain6.wav",
};

const char* CBaseGruntOp4::pDieSounds[] =
{
	"fgrunt/death1.wav",
	"fgrunt/death2.wav",
	"fgrunt/death3.wav",
	"fgrunt/death4.wav",
	"fgrunt/death5.wav",
	"fgrunt/death6.wav",
};

const char* CBaseGruntOp4::pGrenSounds[] =
{
	"fgrunt/incoming.wav",
	"fgrunt/grenade.wav",
};

const char* CBaseGruntOp4::pAlertSounds[] =
{
	"fgrunt/down.wav",
	//"fgrunt/bogies.wav",
	"fgrunt/hostiles.wav",
};

const char* CBaseGruntOp4::pMonsterSounds[] =
{
	"fgrunt/alert.wav",
	"fgrunt/bogies.wav",
};

const char* CBaseGruntOp4::pCoverSounds[] =
{
	//"fgrunt/clear.wav",
	"fgrunt/cover.wav",
	"fgrunt/staydown.wav",
};

const char* CBaseGruntOp4::pThrowSounds[] =
{
	"fgrunt/fire.wav",
	//"fgrunt/grenade.wav",
};

const char* CBaseGruntOp4::pChargeSounds[] =
{
	"fgrunt/go.wav",
	"fgrunt/move.wav",
	"fgrunt/getsome.wav",
};

const char* CBaseGruntOp4::pTauntSounds[] =
{
	"fgrunt/nothing.wav",
	//"fgrunt/getsome.wav",
	"fgrunt/corners.wav",
	"fgrunt/backup.wav",
	//"fgrunt/oneshot.wav",
	//"fgrunt/mister.wav",
};

const char* CBaseGruntOp4::pShotSounds[] =
{
	"fgrunt/friendly.wav",
	"fgrunt/watchfire.wav",
	//"fgrunt/zone.wav",
	"fgrunt/watchit.wav",
	//"fgrunt/check.wav",
	"fgrunt/wantsome.wav",
};

const char* CBaseGruntOp4::pMadSounds[] =
{
	"fgrunt/ass.wav",
	"fgrunt/mister.wav",
};

const char* CBaseGruntOp4::pKillSounds[] =
{
	"fgrunt/sniper.wav",
	"fgrunt/tag.wav",
	"fgrunt/corporal.wav",
	"fgrunt/talking.wav",
	"fgrunt/take.wav",
	//"fgrunt/area.wav",
	//"fgrunt/check.wav",
	//"fgrunt/wantsome.wav",
	"fgrunt/oneshot.wav",
};

const char* CBaseGruntOp4::pAnswerSounds[] =
{
	"fgrunt/roger.wav",
	"fgrunt/quiet.wav",
	"fgrunt/no.wav",
};

const char* CBaseGruntOp4::pQuestionSounds[] =
{
	"fgrunt/seensquad.wav",
	"fgrunt/current.wav",
	"fgrunt/coverup.wav",
	"fgrunt/chicken.wav",
	"fgrunt/charge.wav",
	"fgrunt/bfeeling.wav",
};

const char* CBaseGruntOp4::pIdleSounds[] =
{
	"fgrunt/guard.wav",
	"fgrunt/mission.wav",
	"fgrunt/babysitting.wav",
	//"fgrunt/coverup.wav",
	"fgrunt/now.wav",
	"fgrunt/short.wav",
	"fgrunt/outof.wav",
	"fgrunt/lost.wav",
	"fgrunt/fubar.wav",
	"fgrunt/dogs.wav",
	"fgrunt/disney.wav",
	"fgrunt/checkrecon.wav",
	//"fgrunt/seensquad.wav",
};

const char* CBaseGruntOp4::pOkSounds[] =
{
	"fgrunt/damage.wav",
	"fgrunt/six.wav",
	"fgrunt/moving.wav",
	"fgrunt/gotit.wav",
	"fgrunt/hellout.wav",
	"fgrunt/sir.wav",
	"fgrunt/right.wav",
	"fgrunt/command.wav",
	"fgrunt/notfail.wav",
	"fgrunt/yes.wav",
};

const char* CBaseGruntOp4::pWaitSounds[] =
{
	"fgrunt/secure.wav",
	"fgrunt/orders.wav",
	"fgrunt/scout.wav",
	"fgrunt/guardduty.wav",
};

const char* CBaseGruntOp4::pStopSounds[] =
{
	"fgrunt/post.wav",
	"fgrunt/stay.wav",
	"fgrunt/leave.wav",
};

const char* CBaseGruntOp4::pScaredSounds[] =
{
	"fgrunt/hearsome.wav",
	"fgrunt/hear.wav",
};

const char* CBaseGruntOp4::pHelloSounds[] =
{
	"fgrunt/sir_01.wav",
	"fgrunt/tosee.wav",
	"fgrunt/hellosir.wav",
};

const char* CBaseGruntOp4::pCureSounds[] =
{
	"fgrunt/needmedic.wav",
	"fgrunt/hurt.wav",
	"fgrunt/hell.wav",
	"fgrunt/feel.wav",
};

const char* CBaseGruntOp4::pWoundSounds[] =
{
	"fgrunt/imhit.wav",
	"fgrunt/fwound.wav",
	"fgrunt/makeit.wav",
};

const char* CBaseGruntOp4::pMortalSounds[] =
{
	"fgrunt/hitbad.wav",
	"fgrunt/critical.wav",
	"fgrunt/sdamage.wav",
};

const char* CBaseGruntOp4::pAttackSounds[] =
{
	"fgrunt/flank.wav",
	"fgrunt/suppressing.wav",
	"fgrunt/sweep.wav",
	//"fgrunt/wantsome.wav",
	"fgrunt/freaks.wav",
	"fgrunt/recon.wav",
	"fgrunt/rapidfire.wav",
	"fgrunt/marines.wav",
	"fgrunt/moveup.wav",
	//"fgrunt/go.wav",
	"fgrunt/covering.wav",
	//"fgrunt/ass.wav",
};

const char* CBaseGruntOp4::pPokSounds[] =
{
	//"fgrunt/hell.wav",
	//"fgrunt/sir_01.wav",
	//"fgrunt/hellosir.wav",
	"fgrunt/corporal_01.wav",
};

const char* CBaseGruntOp4::pCheckSounds[] =
{
	"fgrunt/check.wav",
	"fgrunt/zone.wav",
};

const char* CBaseGruntOp4::pClearSounds[] =
{
	"fgrunt/clear.wav",
	"fgrunt/area.wav",
};

void CBaseGruntOp4::ShuffleSoundArrays() {
	// Notes on shuffled arrays:
	// don't use a classname because child classes will cause more shuffling during map load
	// don't allow sounds to be shared between shuffled arrays, or else precache count
	// will flucuate on each map load
	if (g_shuffledMonsterSounds.hasKey("op4grunt")) {
		return;
	}
	g_shuffledMonsterSounds.put("op4grunt");

	SHUFFLE_SOUND_ARRAY(pPainSounds);
	SHUFFLE_SOUND_ARRAY(pDieSounds);
	SHUFFLE_SOUND_ARRAY(pGrenSounds);
	SHUFFLE_SOUND_ARRAY(pAlertSounds);
	SHUFFLE_SOUND_ARRAY(pMonsterSounds);
	SHUFFLE_SOUND_ARRAY(pCoverSounds);
	SHUFFLE_SOUND_ARRAY(pThrowSounds);
	SHUFFLE_SOUND_ARRAY(pChargeSounds);
	SHUFFLE_SOUND_ARRAY(pTauntSounds);
	SHUFFLE_SOUND_ARRAY(pShotSounds);
	SHUFFLE_SOUND_ARRAY(pMadSounds);
	SHUFFLE_SOUND_ARRAY(pKillSounds);
	SHUFFLE_SOUND_ARRAY(pOkSounds);
	SHUFFLE_SOUND_ARRAY(pWaitSounds);
	SHUFFLE_SOUND_ARRAY(pAttackSounds)
	SHUFFLE_SOUND_ARRAY(pCheckSounds);
	SHUFFLE_SOUND_ARRAY(pClearSounds);
	SHUFFLE_SOUND_ARRAY(pIdleSounds);
	SHUFFLE_SOUND_ARRAY(pStopSounds);
	SHUFFLE_SOUND_ARRAY(pScaredSounds);
	SHUFFLE_SOUND_ARRAY(pHelloSounds);
	SHUFFLE_SOUND_ARRAY(pCureSounds);
	SHUFFLE_SOUND_ARRAY(pAnswerSounds);
	SHUFFLE_SOUND_ARRAY(pQuestionSounds);
	SHUFFLE_SOUND_ARRAY(pPokSounds);
	SHUFFLE_SOUND_ARRAY(pWoundSounds);
	SHUFFLE_SOUND_ARRAY(pMortalSounds);
}

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
	TalkInit();

	BasePrecache();

	ShuffleSoundArrays();

	PRECACHE_SOUND_ARRAY(pDieSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);

	int oldSoundVariety = soundvariety.value;
	
	// speech requires so many sounds that any requested limit in sound variety should
	// reduce speech arrays to 1 instantly
	soundvariety.value = soundvariety.value > 0 ? 1 : soundvariety.value;

	if (mp_npcidletalk.value) {
		PRECACHE_SOUND_ARRAY(pCheckSounds);
		PRECACHE_SOUND_ARRAY(pIdleSounds);
		PRECACHE_SOUND_ARRAY(pScaredSounds);
		PRECACHE_SOUND_ARRAY(pHelloSounds);
		PRECACHE_SOUND_ARRAY(pCureSounds);
		PRECACHE_SOUND_ARRAY(pAnswerSounds);
		PRECACHE_SOUND_ARRAY(pQuestionSounds);
		PRECACHE_SOUND_ARRAY(pWoundSounds);
		PRECACHE_SOUND_ARRAY(pMortalSounds);
		PRECACHE_SOUND_ARRAY(pClearSounds);
	}

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
	PRECACHE_SOUND_ARRAY(pAttackSounds)
	PRECACHE_SOUND_ARRAY(pStopSounds);
	PRECACHE_SOUND_ARRAY(pPokSounds);

	// follow/unfollow sounds should have more variety because players activate them often
	soundvariety.value = (oldSoundVariety > 0 && oldSoundVariety > 3) ? 3 : soundvariety.value;
	PRECACHE_SOUND_ARRAY(pOkSounds);
	PRECACHE_SOUND_ARRAY(pWaitSounds);

	soundvariety.value = oldSoundVariety;

	PRECACHE_SOUND("fgrunt/medic.wav");
}

void CBaseGruntOp4::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation)
{
	if (!pszSentence)
		return;

	Talk(duration);

	CTalkSquadMonster::g_talkWaitTime = gpGlobals->time + duration + 2.0;

	const char* sample = "";

	int oldSoundVariety = soundvariety.value;
	soundvariety.value = soundvariety.value > 0 ? 1 : soundvariety.value;

	bool isIdleSound = false;

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
		isIdleSound = true;
	}
	else if (!strcmp(pszSentence, "FG_QUESTION")) {
		sample = RANDOM_SOUND_ARRAY(pQuestionSounds);
		isIdleSound = true;
	}
	else if (!strcmp(pszSentence, "FG_IDLE")) {
		sample = RANDOM_SOUND_ARRAY(pIdleSounds);
		isIdleSound = true;
	}
	else if (!strcmp(pszSentence, "FG_OK")) {
		soundvariety.value = (oldSoundVariety > 0 && oldSoundVariety > 3) ? 3 : soundvariety.value;
		sample = RANDOM_SOUND_ARRAY(pOkSounds);
	}
	else if (!strcmp(pszSentence, "FG_WAIT")) {
		soundvariety.value = (oldSoundVariety > 0 && oldSoundVariety > 3) ? 3 : soundvariety.value;
		sample = RANDOM_SOUND_ARRAY(pWaitSounds);
	}
	else if (!strcmp(pszSentence, "FG_STOP")) {
		sample = RANDOM_SOUND_ARRAY(pStopSounds);
	}
	else if (!strcmp(pszSentence, "FG_SCARED")) {
		sample = RANDOM_SOUND_ARRAY(pScaredSounds);
		isIdleSound = true;
	}
	else if (!strcmp(pszSentence, "FG_HELLO")) {
		sample = RANDOM_SOUND_ARRAY(pHelloSounds);
		isIdleSound = true;
	}
	else if (!strcmp(pszSentence, "!FG_CUREA") || !strcmp(pszSentence, "!FG_CUREB") || !strcmp(pszSentence, "!FG_CUREC")) {
		sample = RANDOM_SOUND_ARRAY(pCureSounds);
		isIdleSound = true;
	}
	else if (!strcmp(pszSentence, "FG_WOUND")) {
		sample = RANDOM_SOUND_ARRAY(pWoundSounds);
		isIdleSound = true;
	}
	else if (!strcmp(pszSentence, "FG_MORTAL")) {
		sample = RANDOM_SOUND_ARRAY(pMortalSounds);
		isIdleSound = true;
	}
	else if (!strcmp(pszSentence, "FG_ATTACK")) {
		sample = RANDOM_SOUND_ARRAY(pAttackSounds);
	}
	else if (!strcmp(pszSentence, "FG_POK")) {
		sample = RANDOM_SOUND_ARRAY(pPokSounds);
	}
	else if (!strcmp(pszSentence, "FG_CHECK")) {
		sample = RANDOM_SOUND_ARRAY(pCheckSounds);
		isIdleSound = true;
	}
	else if (!strcmp(pszSentence, "FG_CLEAR")) {
		sample = RANDOM_SOUND_ARRAY(pClearSounds);
		isIdleSound = true;
	}
	else {
		ALERT(at_console, "Invalid sentence: %s\n", pszSentence);
		return;
	}

	soundvariety.value = oldSoundVariety;

	if (!isIdleSound || mp_npcidletalk.value)
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
