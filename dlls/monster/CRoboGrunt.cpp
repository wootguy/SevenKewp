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
#include "explode.h"

#define GUN_GROUP					2
#define GUN_MP5						0
#define GUN_SHOTGUN					1
#define GUN_NONE					2

#define HGRUNT_9MMAR				( 1 << 0)
#define HGRUNT_HANDGRENADE			( 1 << 1)
#define HGRUNT_GRENADELAUNCHER		( 1 << 2)
#define HGRUNT_SHOTGUN				( 1 << 3)

#define	SENTENCE_VOLUME (float)0.35 // volume of grunt sentences

class CRoboGrunt : public CBaseGrunt
{
public:
	void Spawn(void);
	void Precache(void);
	int Classify(void);
	const char* DisplayName();
	BOOL IsMachine() { return 1; } // ignore classification overrides
	void RunTask(Task_t* pTask);
	int BloodColor(void) { return DONT_BLEED; }
	void GibMonster(void);
	void Killed(entvars_t* pevAttacker, int iGib);
	void EXPORT ExplodeThink(void);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	void DeathSound(void);
	void IdleSound(void);
	void PlaySentenceSound(int sentenceType);
	void HandleAnimEvent(MonsterEvent_t* pEvent);

	float m_explodeTime;

private:
	static const char* pDeathSounds[];
	static const char* pGruntSentences[];

	// using sounds instead of sentences for compatibility with vanilla half-life clients
	static const char* pAlertSounds[];
	static const char* pAnswerSounds[];
	static const char* pChargeSounds[];
	static const char* pCheckSounds[];
	static const char* pClearSounds[];
	static const char* pCoverSounds[];
	static const char* pGrenSounds[];
	static const char* pMonstSounds[];
	static const char* pQuestSounds[];
	static const char* pTauntSounds[];
	static const char* pThrowSounds[];
};

class CRoboGruntRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_robogrunt"; };
};

LINK_ENTITY_TO_CLASS(monster_robogrunt, CRoboGrunt);
LINK_ENTITY_TO_CLASS(monster_robogrunt_repel, CRoboGruntRepel);

const char* CRoboGrunt::pDeathSounds[] =
{
	"turret/tu_die.wav",
	"turret/tu_die2.wav",
	"turret/tu_die3.wav",
};

const char* CRoboGrunt::pAlertSounds[] =
{
	MOD_SND_FOLDER "rgrunt/rb_alert0.wav",
	MOD_SND_FOLDER "rgrunt/rb_alert1.wav",
};
const char* CRoboGrunt::pAnswerSounds[] =
{
	"vox/green.wav",
	MOD_SND_FOLDER "rgrunt/rb_answer1.wav",
};
const char* CRoboGrunt::pChargeSounds[] =
{
	"vox/accelerating.wav",
	"vox/engage.wav",
};
const char* CRoboGrunt::pCheckSounds[] =
{
	MOD_SND_FOLDER "rgrunt/rb_check0.wav",
	MOD_SND_FOLDER "rgrunt/rb_check1.wav",
};
const char* CRoboGrunt::pClearSounds[] =
{
	MOD_SND_FOLDER "rgrunt/rb_clear0.wav",
	MOD_SND_FOLDER "rgrunt/rb_clear1.wav",
};
const char* CRoboGrunt::pCoverSounds[] =
{
	"vox/dadeda.wav",
	"vox/bizwarn.wav",
};
const char* CRoboGrunt::pGrenSounds[] =
{
	MOD_SND_FOLDER "rgrunt/rb_gren0.wav",
	MOD_SND_FOLDER "rgrunt/rb_gren1.wav",
};
const char* CRoboGrunt::pMonstSounds[] =
{
	"vox/alert.wav",
	MOD_SND_FOLDER "rgrunt/rb_monst1.wav",
};
const char* CRoboGrunt::pQuestSounds[] =
{
	"vox/status.wav",
	MOD_SND_FOLDER "rgrunt/rb_quest1.wav",
};
const char* CRoboGrunt::pTauntSounds[] =
{
	MOD_SND_FOLDER "rgrunt/rb_taunt0.wav",
	MOD_SND_FOLDER "rgrunt/rb_taunt1.wav",
};
const char* CRoboGrunt::pThrowSounds[] =
{
	MOD_SND_FOLDER "rgrunt/rb_throw0.wav",
};

const char* CRoboGrunt::pGruntSentences[] =
{
	"HG_GREN", // grenade scared grunt
	"HG_ALERT", // sees player
	"HG_MONSTER", // sees monster
	"HG_COVER", // running to cover
	"HG_THROW", // about to throw grenade
	"HG_CHARGE",  // running out to get the enemy
	"HG_TAUNT", // say rude things
};

void CRoboGrunt::Spawn() {
	BaseSpawn();

	// get voice pitch
	if (RANDOM_LONG(0, 1))
		m_voicePitch = 120 + RANDOM_LONG(0, 9);
	else
		m_voicePitch = 115;

	if (pev->weapons == 0)
	{
		// initialize to original values
		pev->weapons = HGRUNT_9MMAR | HGRUNT_HANDGRENADE;
		// pev->weapons = HGRUNT_SHOTGUN;
		// pev->weapons = HGRUNT_9MMAR | HGRUNT_GRENADELAUNCHER;
	}

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
}

void CRoboGrunt::Precache()
{
	BasePrecache();

	m_defaultModel = "models/rgrunt.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_MODEL("models/computergibs.mdl");

	PRECACHE_SOUND_ARRAY(pDeathSounds);

	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pAnswerSounds);
	PRECACHE_SOUND_ARRAY(pChargeSounds);
	PRECACHE_SOUND_ARRAY(pCheckSounds);
	PRECACHE_SOUND_ARRAY(pClearSounds);
	PRECACHE_SOUND_ARRAY(pCoverSounds);
	PRECACHE_SOUND_ARRAY(pGrenSounds);
	PRECACHE_SOUND_ARRAY(pMonstSounds);
	PRECACHE_SOUND_ARRAY(pQuestSounds);
	PRECACHE_SOUND_ARRAY(pTauntSounds);
	PRECACHE_SOUND_ARRAY(pThrowSounds);
}

int	CRoboGrunt::Classify(void)
{
	return	CBaseMonster::Classify(CLASS_MACHINE);
}

const char* CRoboGrunt::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Robot Grunt";
}

void CRoboGrunt::GibMonster(void)
{
	ExplosionCreate(pev->origin + Vector(0, 0, 8), pev->angles, edict(), 90, true);
	CBaseGrunt::GibMonster();
}

void CRoboGrunt::Killed(entvars_t* pevAttacker, int iGib)
{
	CBaseGrunt::Killed(pevAttacker, iGib);

	if (ShouldGibMonster(iGib)) {
		m_explodeTime = 0.1;
		return;
	}
	if (m_explodeTime > 0) {
		return;
	}

	EMIT_SOUND(ENT(pev), CHAN_BODY, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_NORM);
	SetThink(&CRoboGrunt::ExplodeThink);
	pev->nextthink = gpGlobals->time + 0.1;
	m_explodeTime = gpGlobals->time + 3.0f + RANDOM_FLOAT(0, 3.0f);
}

void CRoboGrunt::ExplodeThink(void)
{
	CBaseMonster::MonsterThink();

	if (gpGlobals->time > m_explodeTime) {
		GibMonster();
		return;
	}

	// lots of smoke
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_SMOKE);
	WRITE_COORD(RANDOM_FLOAT(pev->absmin.x, pev->absmax.x));
	WRITE_COORD(RANDOM_FLOAT(pev->absmin.y, pev->absmax.y));
	WRITE_COORD(pev->origin.z + 16);
	WRITE_SHORT(g_sModelIndexSmoke);
	WRITE_BYTE(25); // scale * 10
	WRITE_BYTE(10); // framerate
	MESSAGE_END();

	pev->nextthink = gpGlobals->time + 0.2;
}

void CRoboGrunt::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_DIE:
	{
		if (m_fSequenceFinished && pev->frame >= 255)
		{
			pev->deadflag = DEAD_DEAD;
			StopAnimation();

			if (!BBoxFlat())
			{
				// a bit of a hack. If a corpses' bbox is positioned such that being left solid so that it can be attacked will
				// block the player on a slope or stairs, the corpse is made nonsolid. 
//					pev->solid = SOLID_NOT;
				UTIL_SetSize(pev, Vector(-4, -4, 0), Vector(4, 4, 1));
			}
			else // !!!HACKHACK - put monster in a thin, wide bounding box until we fix the solid type/bounding volume problem
				UTIL_SetSize(pev, Vector(pev->mins.x, pev->mins.y, pev->mins.z), Vector(pev->maxs.x, pev->maxs.y, pev->mins.z + 1));
		}
		break;
	}
	default:
		CBaseGrunt::RunTask(pTask);
	}
}

void CRoboGrunt::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	// reduce bullet damage
	if (flDamage > 0) {
		UTIL_Ricochet(ptr->vecEndPos, 1.0);

		if (pev->deadflag != DEAD_DEAD) {
			if (!(bitsDamageType & DMG_ENERGYBEAM))
				flDamage *= 0.2;
		}
		else if (RANDOM_LONG(0, 4) == 0) { // 25% chance damage triggers explosion on death
			GibMonster();
		}
	}

	ptr->iHitgroup = HITGROUP_GENERIC; // no weak points
	bitsDamageType &= ~DMG_BLOOD; // never bleed

	CBaseGrunt::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

void CRoboGrunt::DeathSound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1, ATTN_NORM);
}

void CRoboGrunt::IdleSound(void)
{
	if (FOkToSpeak() && (g_fGruntQuestion || RANDOM_LONG(0, 1)))
	{
		if (!g_fGruntQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0, 1))
			{
			case 0: // check in
				//SENTENCEG_PlayRndSz(ENT(pev), "HG_CHECK", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pCheckSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				g_fGruntQuestion = 1;
				break;
			case 1: // question
				//SENTENCEG_PlayRndSz(ENT(pev), "HG_QUEST", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pQuestSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				g_fGruntQuestion = 2;
				break;
			}
		}
		else
		{
			switch (g_fGruntQuestion)
			{
			case 1: // check in
				//SENTENCEG_PlayRndSz(ENT(pev), "HG_CLEAR", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pClearSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				break;
			case 2: // question 
				//SENTENCEG_PlayRndSz(ENT(pev), "HG_ANSWER", SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAnswerSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				break;
			}
			g_fGruntQuestion = 0;
		}
		JustSpoke();
	}
}

void CRoboGrunt::PlaySentenceSound(int sentenceType) {
	if (sentenceType >= ARRAYSIZE(pGruntSentences)) {
		return;
	}

	//SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[sentenceType], SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);

	switch (sentenceType) {
	case 0:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pGrenSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		break;
	case 1:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		break;
	case 2:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pMonstSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		break;
	case 3:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pCoverSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		break;
	case 4:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pThrowSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		break;
	case 5:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pChargeSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		break;
	case 6:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pTauntSounds), SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		break;
	}
}

void CRoboGrunt::HandleAnimEvent(MonsterEvent_t* pEvent)
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
