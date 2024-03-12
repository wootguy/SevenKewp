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
#include "CBaseGrunt.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"

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

#define SENTENCE_VOLUME 1.0f

// TODO:
// - add reload events to model so the sounds play at the right time
// - add 5031 events to uzi animation (for dual muzzle flashes)
// - add shoot events to the minigun animation?
// - change minigun muzzle flash event type (use minigun attachement)
// - remove ACT_IDLE from minigun idle in model, so it never happens randomly without a minigun
// - add weapon drop events to model

class CBodyGuard: public CBaseGrunt
{
public:
	void Spawn(void);
	void Precache(void);
	void InitAiFlags();
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	void GibMonster(void);
	void ShuffleSoundArrays();

	void SetActivity(Activity NewActivity);
	int GetActivitySequence(Activity NewActivity);
	Schedule_t* GetScheduleOfType(int Type);

	CUSTOM_SCHEDULES;

	void PlaySentenceSound(int sentenceType);
	void PlaySentence(const char* pszSentence, float duration, float volume, float attenuation);
	int	Classify(void);
	const char* DisplayName();
	int ISoundMask(void);
	BOOL NoFriendlyFire(void) { return TRUE; } // friendly fire is allowed
	void AlertSound();
	void PainSound(void);
	void DeathSound(void);
	void DeclineFollowing();
	void TalkInit();
	int ObjectCaps();
	void OnTaskComplete(Task_t task);

	void PrescheduleThink(void);

	MONSTERSTATE GetIdealState()
	{
		return CTalkSquadMonster::GetIdealState();
	}

	int minigunShootSeq;
	int minigunSpinupSeq;
	float nextMinigunShoot;
	bool minigunIsSpinning;

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

LINK_ENTITY_TO_CLASS(monster_bodyguard, CBodyGuard);

const char* CBodyGuard::pGruntSentences[] =
{
	"BG_GREN", // grenade scared grunt
	"BG_ALERT", // sees player
	"BG_MONSTER", // sees monster
	"BG_COVER", // running to cover
	"BG_THROW", // about to throw grenade
	"BG_CHARGE",  // running out to get the enemy
	"BG_TAUNT", // say rude things
	"BG_SHOT", // attacked by player
	"BG_MAD", // friendly fire revenge
	"BG_KILL" // killed an enemy
};

const char* CBodyGuard::pPainSounds[] =
{
	MOD_SND_FOLDER "bodyguard/pain1.wav",
	MOD_SND_FOLDER "bodyguard/pain2.wav",
	MOD_SND_FOLDER "bodyguard/pain3.wav",
	MOD_SND_FOLDER "bodyguard/pain4.wav"
};

const char* CBodyGuard::pDeathSounds[] =
{
	MOD_SND_FOLDER "bodyguard/die1.wav",
	MOD_SND_FOLDER "bodyguard/die2.wav",
	MOD_SND_FOLDER "bodyguard/die3.wav"
};

const char* CBodyGuard::pShotSounds[] =
{
	MOD_SND_FOLDER "bodyguard/hellaimingat.wav",
	MOD_SND_FOLDER "bodyguard/watchyourfire.wav",
	MOD_SND_FOLDER "bodyguard/wounded1.wav",
};

const char* CBodyGuard::pMadSounds[] =
{
	MOD_SND_FOLDER "bodyguard/bringiton.wav",
	MOD_SND_FOLDER "bodyguard/heyoverhere.wav",
	MOD_SND_FOLDER "bodyguard/primalroar.wav",
};

const char* CBodyGuard::pKillSounds[] =
{
	MOD_SND_FOLDER "bodyguard/whathelljoke.wav",
	MOD_SND_FOLDER "bodyguard/layeggs.wav",
	MOD_SND_FOLDER "bodyguard/ashestoashes.wav",
};

const char* CBodyGuard::pIdleSounds[] =
{
	MOD_SND_FOLDER "bodyguard/pathetic.wav",
	MOD_SND_FOLDER "bodyguard/mondays.wav",
	MOD_SND_FOLDER "bodyguard/shownupdrunk.wav",
	MOD_SND_FOLDER "bodyguard/shootface.wav",
	MOD_SND_FOLDER "bodyguard/nicesuit.wav",
};

const char* CBodyGuard::pOkSounds[] =
{
	MOD_SND_FOLDER "bodyguard/letsmove.wav",
	MOD_SND_FOLDER "bodyguard/babysitter.wav",
	MOD_SND_FOLDER "bodyguard/yougotit.wav",
};

const char* CBodyGuard::pWaitSounds[] =
{
	MOD_SND_FOLDER "bodyguard/stop3.wav",
	MOD_SND_FOLDER "bodyguard/laterfellas.wav",
	MOD_SND_FOLDER "bodyguard/waitinrighthere.wav",
};

const char* CBodyGuard::pStopSounds[] =
{
	MOD_SND_FOLDER "bodyguard/stop1.wav",
	MOD_SND_FOLDER "bodyguard/stop2.wav",
	MOD_SND_FOLDER "bodyguard/stop3.wav",
};

const char* CBodyGuard::pScaredSounds[] =
{
	MOD_SND_FOLDER "bodyguard/hellareyoudoing.wav",
	MOD_SND_FOLDER "bodyguard/knockitoff.wav",
	MOD_SND_FOLDER "bodyguard/backoff.wav",
};

const char* CBodyGuard::pHelloSounds[] =
{
	MOD_SND_FOLDER "bodyguard/hello.wav",
	MOD_SND_FOLDER "bodyguard/whatsgoinon.wav",
	MOD_SND_FOLDER "bodyguard/someonescoming.wav",
};

const char* CBodyGuard::pCureSounds[] =
{
	MOD_SND_FOLDER "bodyguard/looklikeshit.wav",
	MOD_SND_FOLDER "bodyguard/hellhappenedyou.wav",
	MOD_SND_FOLDER "bodyguard/notaninjury.wav",
};

const char* CBodyGuard::pQuestionSounds[] =
{
	MOD_SND_FOLDER "bodyguard/babysitter.wav",
	MOD_SND_FOLDER "bodyguard/pathetic.wav",
	MOD_SND_FOLDER "bodyguard/whathelljoke.wav",
};

const char* CBodyGuard::pAnswerSounds[] =
{
	MOD_SND_FOLDER "bodyguard/imbusy.wav",
	MOD_SND_FOLDER "bodyguard/palimworking.wav",
	MOD_SND_FOLDER "bodyguard/yougotit.wav",
};

void CBodyGuard::GibMonster(void)
{
	if (m_hWaitMedic)
	{
		CTalkSquadMonster* pMedic = m_hWaitMedic->MyTalkSquadMonsterPointer();

		if (pMedic->pev->deadflag != DEAD_NO)
			m_hWaitMedic = nullptr;
		else
			pMedic->HealMe(nullptr);
	}

	DropEquipment(0, true);

	CBaseMonster::GibMonster();
}

void CBodyGuard::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case HGRUNT_AE_DROP_GUN:
		if (DropEquipment(0, false))
			SetBodygroup(2, 0);
		break;

	case BG_SHOOT_UZIS_EVENT1:
	case BG_SHOOT_UZIS_EVENT2:
	case BG_SHOOT_UZIS_EVENT3:
	case BG_SHOOT_UZIS_EVENT4:
	case BG_SHOOT_PISTOL_EVENT:
		Shoot(pEvent->event == BG_SHOOT_UZIS_EVENT1);
		break;

	default:
		CBaseGrunt::HandleAnimEvent(pEvent);
		break;
	}
}

void CBodyGuard::PrescheduleThink(void) {
	CBaseGrunt::PrescheduleThink();

	if (pev->sequence == minigunShootSeq && gpGlobals->time >= nextMinigunShoot) {
		pev->nextthink = nextMinigunShoot = gpGlobals->time + 0.07f;
		Shoot(false);
	}
	if (pev->sequence == minigunSpinupSeq) {
		PointAtEnemy();
	}
}

void CBodyGuard::OnTaskComplete(Task_t task) {
	// the model is missing events for most of the reload animations, so reloading on task completion instead
	if (task.iTask == TASK_PLAY_SEQUENCE) {
		int seq = (int)task.flData;

		switch(seq) {
			case ACT_RELOAD:
				Reload();
				break;
			default:
				break;
		}		
	}
}

void CBodyGuard::Spawn()
{
	BaseSpawn();

	m_afCapability = bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_HEAR;

	if (pev->weapons <= 0 || pev->weapons > 7) {
		pev->weapons = RANDOM_LONG(1, 7);
	}

	switch(pev->weapons)
	{
	case 1:
		m_iEquipment |= MEQUIP_GLOCK;
		m_cClipSize = GLOCK_CLIP_SIZE;
		break;
	case 2:
		m_iEquipment |= MEQUIP_DEAGLE;
		m_cClipSize = DEAGLE_CLIP_SIZE;
		break;
	case 3:
		m_iEquipment |= MEQUIP_SHOTGUN;
		m_cClipSize = SHOTGUN_CLIP_SIZE;
		break;
	case 4:
		m_iEquipment |= MEQUIP_AKIMBO_UZIS;
		m_cClipSize = UZI_CLIP_SIZE;
		break;
	case 5:
		m_iEquipment |= MEQUIP_MP5;
		m_cClipSize = MP5_CLIP_SIZE;
		break;
	case 6:
		m_iEquipment |= MEQUIP_SNIPER;
		m_cClipSize = SNIPER_CLIP_SIZE;
		m_flDistTooFar = 4096.0;
		m_flDistLook = 4096.0;
		maxShootDist = 4096.0;
		break;
	case 7:
		m_iEquipment |= MEQUIP_MINIGUN;
		m_cClipSize = INT_MAX;
		break;
	default:
		m_cClipSize = 0;
		break;
	}

	m_cAmmoLoaded = m_cClipSize;

	pev->body = 0;
	SetBodygroup(2, pev->weapons);

	pev->skin = RANDOM_LONG(0, 1);

	m_flMedicWaitTime = gpGlobals->time;

	// get voice pitch
	m_voicePitch = pev->skin == 0 ? 100 : 90;

	minigunShootSeq = LookupSequence("shoot_minigun");
	minigunSpinupSeq = LookupSequence("shoot_minigun_spinup");
}

void CBodyGuard::ShuffleSoundArrays() {
	if (g_shuffledMonsterSounds.count(STRING(pev->classname))) {
		return;
	}
	g_shuffledMonsterSounds.insert(STRING(pev->classname));

	SHUFFLE_SOUND_ARRAY(pShotSounds);
	SHUFFLE_SOUND_ARRAY(pMadSounds);
	SHUFFLE_SOUND_ARRAY(pKillSounds);
	SHUFFLE_SOUND_ARRAY(pIdleSounds);
	SHUFFLE_SOUND_ARRAY(pStopSounds);
	SHUFFLE_SOUND_ARRAY(pScaredSounds);
	SHUFFLE_SOUND_ARRAY(pHelloSounds);
	SHUFFLE_SOUND_ARRAY(pCureSounds);
	SHUFFLE_SOUND_ARRAY(pQuestionSounds);
	SHUFFLE_SOUND_ARRAY(pAnswerSounds);
}

void CBodyGuard::Precache()
{
	CBaseGrunt::BasePrecache();

	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

	ShuffleSoundArrays();

	int oldSoundVariety = soundvariety.value;

	// speech requires so many sounds that any requested limit in sound variety should
	// reduce speech arrays to 1 instantly
	soundvariety.value = soundvariety.value > 0 ? 1 : soundvariety.value;

	PRECACHE_SOUND_ARRAY(pShotSounds);
	PRECACHE_SOUND_ARRAY(pMadSounds);
	PRECACHE_SOUND_ARRAY(pKillSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pStopSounds);
	PRECACHE_SOUND_ARRAY(pScaredSounds);
	PRECACHE_SOUND_ARRAY(pHelloSounds);
	PRECACHE_SOUND_ARRAY(pCureSounds);
	PRECACHE_SOUND_ARRAY(pQuestionSounds);
	PRECACHE_SOUND_ARRAY(pAnswerSounds);

	// follow/unfollow sounds should have more variety because players activate them often
	soundvariety.value = (oldSoundVariety > 0 && oldSoundVariety > 3) ? 3 : soundvariety.value;
	PRECACHE_SOUND_ARRAY(pOkSounds);
	PRECACHE_SOUND_ARRAY(pWaitSounds);

	soundvariety.value = oldSoundVariety;

	m_defaultModel = "models/bgman.mdl";
	PRECACHE_MODEL(GetModel());

	TalkInit();
}

void CBodyGuard::InitAiFlags() {
	// default to HL Grunt AI
	canBeMadAtPlayer = true;
	waitForEnemyFire = false;
	runFromHeavyDamage = false;
	canCallMedic = false;
	maxShootDist = 2048;
}

DEFINE_CUSTOM_SCHEDULES(CBodyGuard)
{
	slGruntRangeAttack1B
};

IMPLEMENT_CUSTOM_SCHEDULES(CBodyGuard, CBaseGrunt);


Schedule_t* CBodyGuard::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_RANGE_ATTACK1:
		if (HasEquipment(MEQUIP_MINIGUN) && !minigunIsSpinning) {
			minigunIsSpinning = true;
			return &slMinigunSpinup[0];
		}
		return &slGruntRangeAttack1C[0]; // prevent crouching or angry idle animations
	default:
		if (minigunIsSpinning) {
			minigunIsSpinning = false;
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, MOD_SND_FOLDER "hassault/hw_spindown.wav", 1.0, ATTN_NORM, 0, m_voicePitch);
			return &slMinigunSpindown[0];
		}
		return CBaseGrunt::GetScheduleOfType(Type);
	}
}

void CBodyGuard::SetActivity(Activity NewActivity) {
	CBaseGrunt::SetActivity(NewActivity);

	if (NewActivity == ACT_THREAT_DISPLAY) {
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, MOD_SND_FOLDER "hassault/hw_spinup.wav", 1.0, ATTN_NORM, 0, m_voicePitch);
		PointAtEnemy();
	}
}

int CBodyGuard::GetActivitySequence(Activity NewActivity) {
	int iSequence = ACTIVITY_NOT_AVAILABLE;

	switch (NewActivity)
	{
	case ACT_IDLE:
	{
		if (HasEquipment(MEQUIP_MINIGUN)) {
			iSequence = LookupSequence("idle_minigun");
		}
		else {
			iSequence = LookupActivity(NewActivity);
		}
		break;
	}
	case ACT_WALK:
	case ACT_RUN:
		if (HasEquipment(MEQUIP_MINIGUN)) {
			iSequence = LookupSequence("walk_minigun");
		}
		else {
			iSequence = LookupActivity(NewActivity);
		}
		break;
	case ACT_RANGE_ATTACK1:
		switch(pev->weapons) {
			case 1: iSequence = LookupSequence("shoot_pistol"); break;
			case 2: iSequence = LookupSequence("shoot_deagle"); break;
			case 3: iSequence = LookupSequence("shoot_shotgun"); break;
			case 4: iSequence = LookupSequence("shoot_uzis"); break;
			case 5: iSequence = LookupSequence("shoot_mp5"); break;
			case 6: iSequence = LookupSequence("shoot_sniper"); break;
			case 7: iSequence = LookupSequence("shoot_minigun"); break;
			default: break;
		}
		
		break;
	case ACT_DISARM:
		iSequence = LookupSequence("shoot_minigun_spindown");
		break;
	case ACT_RELOAD:
		switch (pev->weapons) {
			case 1: 
			case 2: iSequence = LookupSequence("reload_pistol"); break;
			case 3: iSequence = LookupSequence("reload_shotgun"); break;
			case 4: iSequence = LookupSequence("reload_uzis"); break;
			case 5: iSequence = LookupSequence("reload_mp5");
			case 6: iSequence = LookupSequence("reload_sniper"); break;
			case 7: iSequence = LookupSequence("shoot_minigun"); break;
			default: break;
		}
		break;
	default:
		iSequence = LookupActivity(NewActivity);
		break;
	}

	return iSequence;
}

void CBodyGuard::PlaySentenceSound(int sentenceType) {
	if (sentenceType >= ARRAYSIZE(pGruntSentences)) {
		return;
	}
	PlaySentence(pGruntSentences[sentenceType], 3.0f, SENTENCE_VOLUME, GRUNT_ATTN);
}

void CBodyGuard::PlaySentence(const char* pszSentence, float duration, float volume, float attenuation)
{
	if (!pszSentence)
		return;

	Talk(duration);

	CTalkSquadMonster::g_talkWaitTime = gpGlobals->time + duration + 2.0;

	const char* sample = "";

	int oldSoundVariety = soundvariety.value;
	soundvariety.value = soundvariety.value > 0 ? 1 : soundvariety.value;

	if (!strcmp(pszSentence, "BG_SHOT")) {
		sample = RANDOM_SOUND_ARRAY(pShotSounds);
	}
	else if (!strcmp(pszSentence, "BG_MAD")) {
		sample = RANDOM_SOUND_ARRAY(pMadSounds);
	}
	else if (!strcmp(pszSentence, "BG_KILL")) {
		sample = RANDOM_SOUND_ARRAY(pKillSounds);
	}
	else if (!strcmp(pszSentence, "BG_IDLE")) {
		sample = RANDOM_SOUND_ARRAY(pIdleSounds);
	}
	else if (!strcmp(pszSentence, "BG_OK")) {
		soundvariety.value = (oldSoundVariety > 0 && oldSoundVariety > 3) ? 3 : soundvariety.value;
		sample = RANDOM_SOUND_ARRAY(pOkSounds);
	}
	else if (!strcmp(pszSentence, "BG_WAIT")) {
		soundvariety.value = (oldSoundVariety > 0 && oldSoundVariety > 3) ? 3 : soundvariety.value;
		sample = RANDOM_SOUND_ARRAY(pWaitSounds);
	}
	else if (!strcmp(pszSentence, "BG_STOP")) {
		sample = RANDOM_SOUND_ARRAY(pStopSounds);
	}
	else if (!strcmp(pszSentence, "BG_SCARED")) {
		sample = RANDOM_SOUND_ARRAY(pScaredSounds);
	}
	else if (!strcmp(pszSentence, "BG_HELLO")) {
		sample = RANDOM_SOUND_ARRAY(pHelloSounds);
	}
	else if (!strcmp(pszSentence, "!BG_CUREA") || !strcmp(pszSentence, "!BG_CUREB") || !strcmp(pszSentence, "!BG_CUREC")) {
		sample = RANDOM_SOUND_ARRAY(pCureSounds);
	}
	else if (!strcmp(pszSentence, "BG_QUESTION")) {
		sample = RANDOM_SOUND_ARRAY(pQuestionSounds);
	}
	else if (!strcmp(pszSentence, "BG_ANSWER")) {
		sample = RANDOM_SOUND_ARRAY(pAnswerSounds);
	}
	else {
		ALERT(at_console, "Invalid sentence: %s\n", pszSentence);
		return;
	}

	soundvariety.value = oldSoundVariety;

	EMIT_SOUND_DYN(edict(), CHAN_VOICE, sample, volume, attenuation, 0, GetVoicePitch());

	// If you say anything, don't greet the player - you may have already spoken to them
	SetBits(m_bitsSaid, bit_saidHelloPlayer);
}

int	CBodyGuard::Classify(void)
{
	return	CBaseMonster::Classify(CLASS_PLAYER_ALLY);
}

const char* CBodyGuard::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Body Guard";
}

int CBodyGuard::ISoundMask(void)
{
	return	bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER |
		bits_SOUND_CARCASS |
		bits_SOUND_MEAT |
		bits_SOUND_GARBAGE;
}

void CBodyGuard::AlertSound()
{
	if (m_hEnemy && FOkToSpeak())
	{
		PlaySentence("BG_ATTACK", RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_NORM);
	}
}

void CBodyGuard::PainSound(void)
{
	if (gpGlobals->time > m_flNextPainTime)
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
		m_flNextPainTime = gpGlobals->time + 1;
	}
}

void CBodyGuard::DeathSound(void)
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_NORM, 0, m_voicePitch);
}

void CBodyGuard::DeclineFollowing()
{
	PlaySentence("BG_POK", 2, VOL_NORM, ATTN_NORM);
}

void CBodyGuard::TalkInit()
{
	CTalkSquadMonster::TalkInit();

	m_szGrp[TLK_ANSWER] = "BG_ANSWER";
	m_szGrp[TLK_QUESTION] = "BG_QUESTION";
	m_szGrp[TLK_IDLE] = "BG_IDLE";
	m_szGrp[TLK_STARE] = "BG_IDLE";
	m_szGrp[TLK_USE] = "BG_OK";
	m_szGrp[TLK_UNUSE] = "BG_WAIT";
	m_szGrp[TLK_STOP] = "BG_STOP";

	m_szGrp[TLK_NOSHOOT] = "BG_SCARED";
	m_szGrp[TLK_HELLO] = "BG_HELLO";

	m_szGrp[TLK_PLHURT1] = "!BG_CUREA";
	m_szGrp[TLK_PLHURT2] = "!BG_CUREB";
	m_szGrp[TLK_PLHURT3] = "!BG_CUREC";

	m_szGrp[TLK_PHELLO] = NULL;	//"BA_PHELLO";		// UNDONE
	m_szGrp[TLK_PIDLE] = NULL;	//"BA_PIDLE";			// UNDONE
	m_szGrp[TLK_PQUESTION] = NULL;		// UNDONE

	m_szGrp[TLK_SMELL] = NULL;

	m_szGrp[TLK_WOUND] = NULL;
	m_szGrp[TLK_MORTAL] = NULL;
}

int CBodyGuard::ObjectCaps()
{
	return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
}
