#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "nodes.h"
#include "monsters.h"
#include "schedule.h"
#include "customentity.h"
#include "weapons.h"
#include "CGargantua.h"

#define BABYGARG_FLAME_LENGTH 130
#define BABYGARG_FLAME_WIDTH 120
#define BABYGARG_FLAME_WIDTH2 70

class CBabyGarg : public CGargantua
{
public:
	void Spawn(void);
	void Precache(void);

	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-32, -32, 0);
		pev->absmax = pev->origin + Vector(32, 32, 96);
	}
	void Killed(entvars_t* pevAttacker, int iGib);

	void PainSound();
	void AttackSound();
	void BeamSound(int idx);
	void FootSound();
	void StompSound();
	void BreatheSound();

private:
	static const char* pBeamAttackSounds[];
	static const char* pFootSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pStompSounds[];
	static const char* pBreatheSounds[];
	static const char* pDeathSounds[];
};

LINK_ENTITY_TO_CLASS(monster_babygarg, CBabyGarg);

const char* CBabyGarg::pBeamAttackSounds[] =
{
	"babygarg/gar_flameoff1.wav",
	"babygarg/gar_flameon1.wav",
	"babygarg/gar_flamerun1.wav",
};

const char* CBabyGarg::pFootSounds[] =
{
	"babygarg/gar_step1.wav",
	"babygarg/gar_step2.wav",
};


const char* CBabyGarg::pIdleSounds[] =
{
	"babygarg/gar_idle1.wav",
	"babygarg/gar_idle2.wav",
	"babygarg/gar_idle3.wav",
	"babygarg/gar_idle4.wav",
	"babygarg/gar_idle5.wav",
};


const char* CBabyGarg::pAttackSounds[] =
{
	"babygarg/gar_attack1.wav",
	"babygarg/gar_attack2.wav",
	"babygarg/gar_attack3.wav",
};

const char* CBabyGarg::pAlertSounds[] =
{
	"babygarg/gar_alert1.wav",
	"babygarg/gar_alert2.wav",
	"babygarg/gar_alert3.wav",
};

const char* CBabyGarg::pPainSounds[] =
{
	"babygarg/gar_pain1.wav",
	"babygarg/gar_pain2.wav",
	"babygarg/gar_pain3.wav",
};

const char* CBabyGarg::pStompSounds[] =
{
	"babygarg/gar_stomp1.wav",
};

const char* CBabyGarg::pBreatheSounds[] =
{
	"babygarg/gar_breathe1.wav",
	"babygarg/gar_breathe2.wav",
	"babygarg/gar_breathe3.wav",
};

const char* CBabyGarg::pDeathSounds[] =
{
	"babygarg/gar_die1.wav",
	"babygarg/gar_die2.wav"
};

void CBabyGarg::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), "models/babygarg.mdl");
	UTIL_SetSize(pev, Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = gSkillData.gargantuaHealth;
	//pev->view_ofs		= Vector ( 0, 0, 96 );// taken from mdl file
	m_flFieldOfView = -0.2;// width of forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
	
	m_pEyeGlow = CSprite::SpriteCreate(GARG_EYE_SPRITE_NAME, pev->origin, FALSE);
	m_pEyeGlow->SetTransparency(kRenderGlow, 255, 255, 255, 0, kRenderFxNoDissipation);
	m_pEyeGlow->SetAttachment(edict(), 1);
	m_pEyeGlow->SetScale(0.5);
	EyeOff();
	m_seeTime = gpGlobals->time + 5;
	m_flameTime = gpGlobals->time + 2;

	explodeOnDeath = false;
	shakeOnStep = false;
	flameLength = BABYGARG_FLAME_LENGTH;
	flameWidth = BABYGARG_FLAME_WIDTH;
	flameWidth2 = BABYGARG_FLAME_WIDTH2;
	meleeAttackHeight = 64;
	sparkSpeed = 200;

	slashDamage = gSkillData.babyGargDmgSlash;
	fireDamage = gSkillData.babyGargDmgFire;
	stompDamage = gSkillData.babyGargDmgStomp;
}

void CBabyGarg::Precache()
{
	int i;

	PRECACHE_MODEL("models/babygarg.mdl");
	PRECACHE_MODEL("models/babygargf.mdl");

	for (i = 0; i < ARRAYSIZE(pBeamAttackSounds); i++)
		PRECACHE_SOUND((char*)pBeamAttackSounds[i]);

	for (i = 0; i < ARRAYSIZE(pFootSounds); i++)
		PRECACHE_SOUND((char*)pFootSounds[i]);

	for (i = 0; i < ARRAYSIZE(pIdleSounds); i++)
		PRECACHE_SOUND((char*)pIdleSounds[i]);

	for (i = 0; i < ARRAYSIZE(pAlertSounds); i++)
		PRECACHE_SOUND((char*)pAlertSounds[i]);

	for (i = 0; i < ARRAYSIZE(pPainSounds); i++)
		PRECACHE_SOUND((char*)pPainSounds[i]);

	for (i = 0; i < ARRAYSIZE(pAttackSounds); i++)
		PRECACHE_SOUND((char*)pAttackSounds[i]);

	for (i = 0; i < ARRAYSIZE(pStompSounds); i++)
		PRECACHE_SOUND((char*)pStompSounds[i]);

	for (i = 0; i < ARRAYSIZE(pBreatheSounds); i++)
		PRECACHE_SOUND((char*)pBreatheSounds[i]);

	for (i = 0; i < ARRAYSIZE(pDeathSounds); i++)
		PRECACHE_SOUND((char*)pDeathSounds[i]);

	PrecacheCommon();
}

void CBabyGarg::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) {
	CBaseMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

int CBabyGarg::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) {
	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CBabyGarg::Killed(entvars_t* pevAttacker, int iGib)
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pDeathSounds[RANDOM_LONG(0, ARRAYSIZE(pDeathSounds) - 1)], 1.0, ATTN_GARG, 0, PITCH_NORM);
	CGargantua::Killed(pevAttacker, GIB_NEVER);
}

void CBabyGarg::PainSound() {
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds) - 1)], 1.0, ATTN_GARG, 0, PITCH_NORM);
}

void CBabyGarg::AttackSound() {
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_LONG(0, ARRAYSIZE(pAttackSounds) - 1)], 1.0, ATTN_GARG, 0, PITCH_NORM);
}

void CBabyGarg::BeamSound(int idx) {
	if (idx == 1) {
		EMIT_SOUND_DYN(edict(), CHAN_BODY, pBeamAttackSounds[1], 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
	else {
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, pBeamAttackSounds[idx], 1.0, ATTN_NORM, 0, PITCH_NORM);
	}
}

void CBabyGarg::FootSound() {
	EMIT_SOUND_DYN(edict(), CHAN_BODY, pFootSounds[RANDOM_LONG(0, ARRAYSIZE(pFootSounds) - 1)], 1.0, ATTN_GARG, 0, PITCH_NORM + RANDOM_LONG(-10, 10));
}

void CBabyGarg::StompSound() {
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, pStompSounds[RANDOM_LONG(0, ARRAYSIZE(pStompSounds) - 1)], 1.0, ATTN_GARG, 0, PITCH_NORM + RANDOM_LONG(-10, 10));
}

void CBabyGarg::BreatheSound() {
	EMIT_SOUND_DYN(edict(), CHAN_VOICE, pBreatheSounds[RANDOM_LONG(0, ARRAYSIZE(pBreatheSounds) - 1)], 1.0, ATTN_GARG, 0, PITCH_NORM + RANDOM_LONG(-10, 10));
}
