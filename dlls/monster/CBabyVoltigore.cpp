#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "schedule.h"
#include "env/CSoundEnt.h"
#include "CBabyVoltigore.h"

// TODO:
// skill values (sven doesn't have any for this monster?)

LINK_ENTITY_TO_CLASS(monster_alien_babyvoltigore, CBabyVoltigore)

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
	return CBaseMonster::Classify(CLASS_ALIEN_MONSTER);
}

const char* CBabyVoltigore::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Baby Voltigore";
}

void CBabyVoltigore::SetYawSpeed(void)
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
	default:
		ys = 90;
	}

	pev->yaw_speed = ys * gSkillData.sk_yawspeed_mult;
}

void CBabyVoltigore::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case EVENT_SLASH_BOTH:
	case EVENT_SLASH_RIGHT:
	{
		bool isRightSwing = pEvent->event == EVENT_SLASH_RIGHT;
		float damage = gSkillData.sk_voltigore_dmg_punch * 0.5f;
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

	InitModel();
	SetSize(Vector(-16, -16, 0), Vector(16, 16, 36));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BloodColorAlien();
	pev->view_ofs = Vector(0, 0, 0);// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.0;
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
}

void CBabyVoltigore::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/baby_voltigore.mdl";
	PRECACHE_MODEL(GetModel());

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pRunSounds);
}

void CBabyVoltigore::PainSound(void)
{
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CBabyVoltigore::AlertSound(void)
{
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CBabyVoltigore::IdleSound(void)
{
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(-5, 5);

	// Play a random idle sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CBabyVoltigore::AttackSound(void)
{
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(-5, 5);

	// Play a random attack sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch);
}

int CBabyVoltigore::IgnoreConditions(void)
{
	int iIgnore = CBaseMonster::IgnoreConditions();
	iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE); // no flinching
	return iIgnore;
}

void CBabyVoltigore::StartFollowingSound() {
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(-5, 5);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CBabyVoltigore::StopFollowingSound() {
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(0, 9);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CBabyVoltigore::CantFollowSound() {
	int pitch = BASE_SOUND_PITCH + RANDOM_LONG(0, 9);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}