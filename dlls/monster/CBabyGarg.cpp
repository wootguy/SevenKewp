#include "extdll.h"
#include "util.h"
#include "nodes.h"
#include "monsters.h"
#include "schedule.h"
#include "customentity.h"
#include "weapons.h"
#include "CBabyGarg.h"
#include "effects.h"
#include "scriptevent.h"

LINK_ENTITY_TO_CLASS(monster_babygarg, CBabyGarg)

const char* CBabyGarg::pBeamAttackSounds[] =
{
	"garg/gar_flameoff1.wav",
	"garg/gar_flameon1.wav",
	"garg/gar_flamerun1.wav",
};

const char* CBabyGarg::pFootSounds[] =
{
	"garg/gar_step1.wav",
	"garg/gar_step2.wav",
};


const char* CBabyGarg::pIdleSounds[] =
{
	"garg/gar_idle1.wav",
	"garg/gar_idle2.wav",
	"garg/gar_idle3.wav",
	"garg/gar_idle4.wav",
	"garg/gar_idle5.wav",
};


const char* CBabyGarg::pAttackSounds[] =
{
	"garg/gar_attack1.wav",
	"garg/gar_attack2.wav",
	"garg/gar_attack3.wav",
};

const char* CBabyGarg::pAlertSounds[] =
{
	"garg/gar_alert1.wav",
	"garg/gar_alert2.wav",
	"garg/gar_alert3.wav",
};

const char* CBabyGarg::pPainSounds[] =
{
	"garg/gar_pain1.wav",
	"garg/gar_pain2.wav",
	"garg/gar_pain3.wav",
};

const char* CBabyGarg::pStompSounds[] =
{
	"garg/gar_stomp1.wav",
};

const char* CBabyGarg::pBreatheSounds[] =
{
	"garg/gar_breathe1.wav",
	"garg/gar_breathe2.wav",
	"garg/gar_breathe3.wav",
};

const char* CBabyGarg::pDeathSounds[] =
{
	"garg/gar_die1.wav",
	"garg/gar_die2.wav"
};

void CBabyGarg::Spawn()
{
	Precache();

	InitModel();
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	//pev->view_ofs		= Vector ( 0, 0, 96 );// taken from mdl file
	m_flFieldOfView = -0.2;// width of forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();
	
	m_hEyeGlow = CSprite::SpriteCreate(GARG_EYE_SPRITE_NAME, pev->origin, FALSE);
	CSprite* m_pEyeGlow = (CSprite*)m_hEyeGlow.GetEntity();

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

	slashDamage = gSkillData.sk_babygargantua_dmg_slash;
	fireDamage = gSkillData.sk_babygargantua_dmg_fire;
	stompDamage = gSkillData.sk_babygargantua_dmg_stomp;
}

void CBabyGarg::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/babygarg.mdl";
	PRECACHE_MODEL(GetModel());
	
	// TODO: Friendly variant, but just as a skin to reduce model count
	//PRECACHE_MODEL("models/babygargf.mdl");

	// should not be affected by mp_soundvariety
	PRECACHE_SOUND(pBeamAttackSounds[0]);
	PRECACHE_SOUND(pBeamAttackSounds[1]);
	PRECACHE_SOUND(pBeamAttackSounds[2]);

	PRECACHE_SOUND_ARRAY(pFootSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pStompSounds);
	PRECACHE_SOUND_ARRAY(pBreatheSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);

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
	if (pev->deadflag != DEAD_DEAD)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDeathSounds), 1.0, ATTN_GARG, 0, 250);
	CGargantua::Killed(pevAttacker, iGib);
}

void CBabyGarg::PainSound() {
	if (gpGlobals->time - m_lastPainSound > 3.0f) {
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_GARG, 0, BABYGARG_PITCH);
		m_lastPainSound = gpGlobals->time;
	}
}

void CBabyGarg::AttackSound() {
	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_GUN_VOLUME, 0.3);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_GARG, 0, BABYGARG_PITCH);
}

void CBabyGarg::BeamSound(int idx) {
	if (idx == 1) {
		EMIT_SOUND_DYN(edict(), CHAN_BODY, pBeamAttackSounds[1], 1.0, ATTN_NORM, 0, BABYGARG_PITCH);
	}
	else {
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, pBeamAttackSounds[idx], 1.0, ATTN_NORM, 0, BABYGARG_PITCH);
	}
}

void CBabyGarg::FootSound() {
	EMIT_SOUND_DYN(edict(), CHAN_BODY, RANDOM_SOUND_ARRAY(pFootSounds), 1.0, ATTN_GARG, 0, BABYGARG_PITCH + RANDOM_LONG(-10, 10));
}

void CBabyGarg::StompSound() {
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pStompSounds), 1.0, ATTN_GARG, 0, BABYGARG_PITCH + RANDOM_LONG(-10, 10));
}

void CBabyGarg::BreatheSound() {
	EMIT_SOUND_DYN(edict(), CHAN_VOICE, RANDOM_SOUND_ARRAY(pBreatheSounds), 1.0, ATTN_GARG, 0, BABYGARG_PITCH + RANDOM_LONG(-10, 10));
}

void CBabyGarg::StartFollowingSound() {
	EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_GARG, 0, BABYGARG_PITCH);
}

void CBabyGarg::StopFollowingSound() {
	EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_GARG, 0, BABYGARG_PITCH);
}

void CBabyGarg::CantFollowSound() {
	EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_GARG, 0, BABYGARG_PITCH);
}

void CBabyGarg::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case SCRIPT_EVENT_SOUND_VOICE:
		EMIT_SOUND_DYN(edict(), CHAN_VOICE, pEvent->options, 1.0, ATTN_IDLE, 0, BABYGARG_PITCH);
		break;
	default:
		CGargantua::HandleAnimEvent(pEvent);
		break;
	}
}