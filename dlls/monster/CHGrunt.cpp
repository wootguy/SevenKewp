#include "extdll.h"
#include "plane.h"
#include "util.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "CHGrunt.h"
#include "weapons.h"
#include "env/CSoundEnt.h"
#include "effects.h"
#include "customentity.h"

LINK_ENTITY_TO_CLASS(monster_human_grunt, CHGrunt)
LINK_ENTITY_TO_CLASS(monster_grunt_repel, CHGruntRepel)
LINK_ENTITY_TO_CLASS(monster_hgrunt_dead, CDeadHGrunt)

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
	m_skinFrames = 2;
	BaseSpawn();

	if (m_iEquipment & MEQUIP_SHOTGUN)
	{
		SetBodygroup(GUN_GROUP, GUN_SHOTGUN);
		m_cClipSize = 8;
	}
	else if (FBitSet(pev->weapons, HGRUNT_ROCKETLAUNCHER))
	{
		SetBodygroup(GUN_GROUP, GUN_ROCKETLAUNCHER);
		m_cClipSize = 1;
		m_flDistTooFar = 4096.0;
		m_flDistLook = 4096.0;
		maxShootDist = 4096;
	}
	else if (m_iEquipment & MEQUIP_SNIPER)
	{
		SetBodygroup(GUN_GROUP, GUN_SNIPERRIFLE);
		m_cClipSize = 1;
		m_flDistTooFar = 4096.0;
		m_flDistLook = 4096.0;
		maxShootDist = 4096;
	}
	else
	{
		m_cClipSize = GRUNT_CLIP_SIZE;
	}
	m_cAmmoLoaded = m_cClipSize;

	if (RANDOM_LONG(0, 99) < 80)
		pev->skin = m_skinBase;	// light skin
	else
		pev->skin = m_skinBase + 1;	// dark skin

	if (m_iEquipment & MEQUIP_SHOTGUN)
	{
		SetBodygroup(HEAD_GROUP, HEAD_SHOTGUN);
	}
	else if (FBitSet(pev->weapons, HGRUNT_GRENADELAUNCHER))
	{
		SetBodygroup(HEAD_GROUP, HEAD_M203);
		pev->skin = m_skinBase + 1; // alway dark skin
	}
	if (GetBodygroup(HEAD_GROUP) == HEAD_GRUNT) {
		m_iEquipment |= MEQUIP_HELMET;
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
	if (FBitSet(pev->weapons, HGRUNT_ROCKETLAUNCHER)) {
		m_iEquipment |= MEQUIP_RPG;
	}
	if (FBitSet(pev->weapons, HGRUNT_SNIPERRIFLE)) {
		m_iEquipment |= MEQUIP_SNIPER;
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
		pev->skin = m_skinBase;
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

void CHGrunt::StartFollowingSound() {
	int r = RANDOM_LONG(0, 2);

	switch (r) {
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "!HG_CHARGE1", GRUNT_SENTENCE_VOLUME, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "!HG_CHARGE2", GRUNT_SENTENCE_VOLUME, ATTN_NORM);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "!HG_CHARGE3", GRUNT_SENTENCE_VOLUME, ATTN_NORM);
		break;
	}
	
	JustSpoke();
}

void CHGrunt::StopFollowingSound() {
	int r = RANDOM_LONG(0, 2);

	switch (r) {
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "!HG_QUEST4", GRUNT_SENTENCE_VOLUME, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "!HG_ANSWER0", GRUNT_SENTENCE_VOLUME, ATTN_NORM);
		break;
	case 2:
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "!HG_ANSWER1", GRUNT_SENTENCE_VOLUME, ATTN_NORM);
		break;
	}
	JustSpoke();
}

void CHGrunt::CantFollowSound() {
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "!HG_ANSWER3", GRUNT_SENTENCE_VOLUME, ATTN_NORM);
	JustSpoke();
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
				SENTENCEG_PlayRndSz(ENT(pev), "HG_CHECK", GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "HG_QUEST", GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "HG_IDLE", GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fGruntQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "HG_CLEAR", GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "HG_ANSWER", GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

void CHGrunt::PlaySentenceSound(int sentenceType) {
	if (sentenceType >= (int)ARRAYSIZE(pGruntSentences)) {
		return;
	}
	SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[sentenceType], GRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
	m_bloodColor = BloodColorHuman();

	// map old bodies onto new bodies
	switch (pev->body)
	{
	case 0: // Grunt with Gun
		pev->body = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 1: // Commander with Gun
		pev->body = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_MP5);
		break;
	case 2: // Grunt no Gun
		pev->body = 0;
		SetBodygroup(HEAD_GROUP, HEAD_GRUNT);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	case 3: // Commander no Gun
		pev->body = 0;
		SetBodygroup(HEAD_GROUP, HEAD_COMMANDER);
		SetBodygroup(GUN_GROUP, GUN_NONE);
		break;
	}
}
