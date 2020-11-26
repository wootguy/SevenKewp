#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "env/CSoundEnt.h"

// TODO:
// skill values (sven doesn't have any for this monster?)

#define EVENT_SLASH_BOTH 12
#define EVENT_SLASH_RIGHT 13
#define EVENT_RUN_SOUND 14

#define MELEE_ATTACK1_DISTANCE 64

#define BASE_SOUND_PITCH 180

class CBabyVoltigore : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int Classify(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	Schedule_t* GetScheduleOfType(int Type);
	int IgnoreConditions(void);

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);

private:
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pRunSounds[];
};

LINK_ENTITY_TO_CLASS(monster_alien_babyvoltigore, CBabyVoltigore);

const char* CBabyVoltigore::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CBabyVoltigore::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CBabyVoltigore::pAttackSounds[] =
{
	"voltigore/voltigore_attack_melee1.wav",
	"voltigore/voltigore_attack_melee2.wav"
};

const char* CBabyVoltigore::pIdleSounds[] =
{
	"voltigore/voltigore_idle1.wav",
	"voltigore/voltigore_idle2.wav",
	"voltigore/voltigore_idle3.wav"
};

const char* CBabyVoltigore::pAlertSounds[] =
{
	"voltigore/voltigore_alert1.wav",
	"voltigore/voltigore_alert2.wav",
	"voltigore/voltigore_alert3.wav"
};

const char* CBabyVoltigore::pPainSounds[] =
{
	"voltigore/voltigore_pain1.wav",
	"voltigore/voltigore_pain2.wav",
	"voltigore/voltigore_pain3.wav",
	"voltigore/voltigore_pain4.wav"
};

const char* CBabyVoltigore::pRunSounds[] =
{
	"voltigore/voltigore_run_grunt1.wav",
	"voltigore/voltigore_run_grunt2.wav"
};

int	CBabyVoltigore::Classify(void)
{
	return CLASS_ALIEN_MONSTER;
}

void CBabyVoltigore::SetYawSpeed(void)
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
	default:
		ys = 1000;
	}

	pev->yaw_speed = ys;
}

void CBabyVoltigore::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case EVENT_SLASH_BOTH:
	case EVENT_SLASH_RIGHT:
	{
		bool isRightSwing = pEvent->event == EVENT_SLASH_RIGHT;
		float damage = gSkillData.voltigoreDmgPunch * 0.5f;
		CBaseEntity* pHurt = CheckTraceHullAttack(MELEE_ATTACK1_DISTANCE, damage, DMG_SLASH);

		if (pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				if (isRightSwing) {
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->punchangle.z = -18;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
				}
				else {
					pHurt->pev->punchangle.x = 18;
				}
			}
			// Play a random attack hit sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackHitSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackHitSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else // Play a random attack miss sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pAttackMissSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackMissSounds) - 1)], 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		break;
	}
	case EVENT_RUN_SOUND:
	{
		int pitch = BASE_SOUND_PITCH + RANDOM_LONG(-5, 5);
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, pRunSounds[RANDOM_LONG(0, ARRAYSIZE(pRunSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
		break;
	}
	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

Schedule_t* CBabyVoltigore::GetScheduleOfType(int Type) {
	if (Type == SCHED_MELEE_ATTACK1) {
		AttackSound();
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

void CBabyVoltigore::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/baby_voltigore.mdl");
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 36));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = 80;
	pev->view_ofs = Vector(0, 0, 0);// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.0;
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
}

void CBabyVoltigore::Precache()
{
	PRECACHE_MODEL("models/baby_voltigore.mdl");

	for (int i = 0; i < ARRAYSIZE(pAttackHitSounds); i++)
		PRECACHE_SOUND((char*)pAttackHitSounds[i]);

	for (int i = 0; i < ARRAYSIZE(pAttackMissSounds); i++)
		PRECACHE_SOUND((char*)pAttackMissSounds[i]);

	for (int i = 0; i < ARRAYSIZE(pAttackSounds); i++)
		PRECACHE_SOUND((char*)pAttackSounds[i]);

	for (int i = 0; i < ARRAYSIZE(pIdleSounds); i++)
		PRECACHE_SOUND((char*)pIdleSounds[i]);

	for (int i = 0; i < ARRAYSIZE(pAlertSounds); i++)
		PRECACHE_SOUND((char*)pAlertSounds[i]);

	for (int i = 0; i < ARRAYSIZE(pPainSounds); i++)
		PRECACHE_SOUND((char*)pPainSounds[i]);

	for (int i = 0; i < ARRAYSIZE(pRunSounds); i++)
		PRECACHE_SOUND((char*)pRunSounds[i]);
}

void CBabyVoltigore::PainSound(void)
{
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
}

void CBabyVoltigore::AlertSound(void)
{
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_LONG(0, ARRAYSIZE(pAlertSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
}

void CBabyVoltigore::IdleSound(void)
{
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(-5, 5);

	// Play a random idle sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pIdleSounds[RANDOM_LONG(0, ARRAYSIZE(pIdleSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
}

void CBabyVoltigore::AttackSound(void)
{
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(-5, 5);

	// Play a random attack sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
}

int CBabyVoltigore::IgnoreConditions(void)
{
	int iIgnore = CBaseMonster::IgnoreConditions();
	iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE); // no flinching
	return iIgnore;
}
