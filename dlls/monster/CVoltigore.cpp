#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "CSprite.h"
#include "weapons.h"
#include "decals.h"

// TODO:
// new classification for racex
// accurate beam color/width and sprite size

#define EVENT_SHOOT_ORB 1
#define EVENT_SLASH_BOTH 12
#define EVENT_SLASH_RIGHT 13

#define MELEE_ATTACK1_DISTANCE 128
#define SHOCK_CHARGE_BEAMS 3
#define SHOCK_FLY_BEAMS 7
#define SHOCK_DEATH_BEAMS 12

#define SHOCK_SPRITE "sprites/blueflare2.spr"
#define BEAM_SPRITE "sprites/lgtning.spr"
#define GIB_MODEL "models/vgibs.mdl"
#define SPORE_EXPLODE_SPRITE "sprites/spore_exp_01.spr"
#define SPORE_EXPLODE_SPRITE2 "sprites/tinyspit.spr"
#define SPORE_EXPLODE_SOUND MOD_SND_FOLDER "weapons/splauncher_impact.wav"
#define SHOCK_SOUND "debris/beamstart1.wav"

#define HEAD_ATTACHEMENT 1
#define ARM_LEFT_ATTACHEMENT 2
#define ARM_RIGHT_ATTACHEMENT 3
#define SHOCK_ATTACHMENT 4

int sporeExplodeSprIdx = 0;

class CVoltigore : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int Classify(void);
	const char* DisplayName();
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	void EXPORT ExplodeThink(void);
	void Killed(entvars_t* pevAttacker, int iGib);
	int IgnoreConditions(void);
	Schedule_t* GetScheduleOfType(int Type);
	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-80, -80, 0);
		pev->absmax = pev->origin + Vector(80, 80, 90);
	}
	void ShowChargeBeam();
	void HideChargeBeam();

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

private:
	float m_rangeAttackCooldown; // next time a range attack can be considered
	CSprite* m_handShock;
	CBeam* m_pBeam[SHOCK_CHARGE_BEAMS];
	CBeam* m_pDeathBeam[SHOCK_DEATH_BEAMS];
	float explodeTime;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pEventSounds[];
};

class CVoltigoreShock : public CBaseEntity
{
public:
	void Spawn(void);

	static void Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	void Touch(CBaseEntity* pOther);
	void EXPORT Fly(void);
	void EXPORT Shock(void);

private:
	CBeam* m_pBeam[SHOCK_FLY_BEAMS];
	int beamUpdateIdx;
	int shocksLeft;
};

Vector RandomBeamPoint(entvars_t* owner);

LINK_ENTITY_TO_CLASS(monster_alien_voltigore, CVoltigore);
LINK_ENTITY_TO_CLASS(voltigoreshock, CVoltigoreShock);

const char* CVoltigore::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CVoltigore::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CVoltigore::pIdleSounds[] =
{
	MOD_SND_FOLDER "voltigore/voltigore_idle1.wav",
	MOD_SND_FOLDER "voltigore/voltigore_idle2.wav",
	MOD_SND_FOLDER "voltigore/voltigore_idle3.wav"
};

const char* CVoltigore::pAlertSounds[] =
{
	MOD_SND_FOLDER "voltigore/voltigore_alert1.wav",
	MOD_SND_FOLDER "voltigore/voltigore_alert2.wav",
	MOD_SND_FOLDER "voltigore/voltigore_alert3.wav"
};

const char* CVoltigore::pPainSounds[] =
{
	MOD_SND_FOLDER "voltigore/voltigore_pain1.wav",
	MOD_SND_FOLDER "voltigore/voltigore_pain2.wav",
	MOD_SND_FOLDER "voltigore/voltigore_pain3.wav",
	MOD_SND_FOLDER "voltigore/voltigore_pain4.wav",
};

const char* CVoltigore::pEventSounds[] =
{
	// TODO: move these out of the model so that mp_soundvariety can limit them
	MOD_SND_FOLDER "voltigore/voltigore_attack_melee1.wav",
	MOD_SND_FOLDER "voltigore/voltigore_attack_melee2.wav",
	MOD_SND_FOLDER "voltigore/voltigore_attack_shock.wav",
	MOD_SND_FOLDER "voltigore/voltigore_run_grunt1.wav",
	MOD_SND_FOLDER "voltigore/voltigore_run_grunt2.wav",
	MOD_SND_FOLDER "voltigore/voltigore_footstep1.wav",
	MOD_SND_FOLDER "voltigore/voltigore_footstep2.wav",
	MOD_SND_FOLDER "voltigore/voltigore_footstep3.wav",
	MOD_SND_FOLDER "voltigore/voltigore_die1.wav",
	MOD_SND_FOLDER "voltigore/voltigore_die2.wav",
	MOD_SND_FOLDER "voltigore/voltigore_die3.wav",
	MOD_SND_FOLDER "voltigore/voltigore_eat.wav"
};


int	CVoltigore::Classify(void)
{
	return CBaseMonster::Classify(CLASS_ALIEN_MONSTER);
}

const char* CVoltigore::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Voltigore";
}

void CVoltigore::SetYawSpeed(void)
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
	default:
		ys = 180;
	}

	pev->yaw_speed = ys * gSkillData.yawspeedMult;
}

void CVoltigore::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case EVENT_SLASH_BOTH:
	case EVENT_SLASH_RIGHT:
	{
		bool isRightSwing = pEvent->event == EVENT_SLASH_RIGHT;
		float damage = gSkillData.voltigoreDmgPunch;
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
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else // Play a random attack miss sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		break;
	}
	case EVENT_SHOOT_ORB:
	{
		HideChargeBeam();

		Vector handOrigin, handAngles;
		GetAttachment(SHOCK_ATTACHMENT-1, handOrigin, handAngles);

		UTIL_MakeVectors(pev->angles);
		Vector vecThrowDir = ((m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs) - handOrigin).Normalize();
		vecThrowDir.x += RANDOM_FLOAT(-0.01, 0.01);
		vecThrowDir.y += RANDOM_FLOAT(-0.01, 0.01);
		vecThrowDir.z += RANDOM_FLOAT(-0.01, 0.01);

		CVoltigoreShock::Shoot(pev, handOrigin, vecThrowDir * 1000);

		m_rangeAttackCooldown = gpGlobals->time + RANDOM_FLOAT(4, 6);
		break;
	}
	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

void CVoltigore::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), GetModel());
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_YELLOW;
	if (!pev->health) pev->health = gSkillData.voltigoreHealth;
	pev->view_ofs = Vector(0, 0, 0);// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.0;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();

	m_handShock = CSprite::SpriteCreate(SHOCK_SPRITE, pev->origin, FALSE);
	m_handShock->SetScale(0.3f);
	m_handShock->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, 0);
	m_handShock->SetAttachment(edict(), SHOCK_ATTACHMENT);

	for (int i = 0; i < SHOCK_CHARGE_BEAMS; i++ ) {
		m_pBeam[i] = CBeam::BeamCreate(BEAM_SPRITE, 30);
		m_pBeam[i]->EntsInit(entindex(), entindex());
		m_pBeam[i]->SetStartAttachment(i+1); // head, left hand, right hand
		m_pBeam[i]->SetEndAttachment(4);
		m_pBeam[i]->SetColor(255, 16, 128);
		m_pBeam[i]->SetBrightness(255);
		m_pBeam[i]->SetNoise(80);
	}

	HideChargeBeam();
}

void CVoltigore::ShowChargeBeam() {
	for (int i = 0; i < SHOCK_CHARGE_BEAMS; i++) {
		if (m_pBeam[i])
			m_pBeam[i]->pev->effects &= ~EF_NODRAW;
	}
	if (m_handShock)
		m_handShock->TurnOn();
}

void CVoltigore::HideChargeBeam() {
	for (int i = 0; i < SHOCK_CHARGE_BEAMS; i++) {
		if (m_pBeam[i])
			m_pBeam[i]->pev->effects |= EF_NODRAW;
	}
	if (m_handShock)
		m_handShock->TurnOff();
}

void CVoltigore::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/voltigore.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_MODEL(GIB_MODEL);
	PRECACHE_MODEL(BEAM_SPRITE);
	PRECACHE_MODEL(SHOCK_SPRITE);
	PRECACHE_MODEL(SPORE_EXPLODE_SPRITE);
	sporeExplodeSprIdx = PRECACHE_MODEL(SPORE_EXPLODE_SPRITE2);

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);

	// not affected by mp_soundvariety (but should be)
	for (int i = 0; i < ARRAYSIZE(pEventSounds); i++) \
		PRECACHE_SOUND((char*)pEventSounds[i]);
	
	PRECACHE_SOUND(SPORE_EXPLODE_SOUND);
	PRECACHE_SOUND(SHOCK_SOUND);
}

BOOL CVoltigore::CheckMeleeAttack1(float flDot, float flDist)
{
	if (flDist <= MELEE_ATTACK1_DISTANCE) {
		return TRUE;
	}

	return FALSE;
}

BOOL CVoltigore::CheckRangeAttack1(float flDot, float flDist)
{
	if (m_rangeAttackCooldown < gpGlobals->time)
	{
		return TRUE;
	}
	return FALSE;
}

void CVoltigore::ExplodeThink(void) {
	CBaseMonster::MonsterThink();

	if (explodeTime < gpGlobals->time) {
		for (int i = 0; i < SHOCK_DEATH_BEAMS; i++) {
			UTIL_Remove(m_pDeathBeam[i]);
			m_pDeathBeam[i] = NULL;
		}

		EMIT_SOUND(ENT(pev), CHAN_BODY, SPORE_EXPLODE_SOUND, 1, ATTN_NORM);
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 0.5, ATTN_NORM);

		pev->health = -100; // speeds up the gibs
		g_vecAttackDir = Vector(0,0,-1);
		pev->absmin.z += 16; // prevent gibs from spawning inside the floor
		CGib::SpawnRandomGibs(pev, 10, GIB_MODEL, 9, 0);

		CSprite* expSprite = CSprite::SpriteCreate(SPORE_EXPLODE_SPRITE, pev->origin + Vector(0,0,64), TRUE);
		expSprite->SetScale(4.0f);
		expSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 128, 0);
		expSprite->AnimateAndDie(17);

		Vector sprayPos = pev->origin + Vector(0, 0, 0);
		Vector sprayDir = Vector(0, 0, 1);

		const int count = 200;
		const int speed = 96;
		const int noise = 128;
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, sprayPos);
		WRITE_BYTE(TE_SPRITE_SPRAY);
			WRITE_COORD(sprayPos.x);
			WRITE_COORD(sprayPos.y);
			WRITE_COORD(sprayPos.z);
			WRITE_COORD(sprayDir.x);
			WRITE_COORD(sprayDir.y);
			WRITE_COORD(sprayDir.z);
			WRITE_SHORT(sporeExplodeSprIdx);
			WRITE_BYTE(count);
			WRITE_BYTE(speed);
			WRITE_BYTE(noise);
		MESSAGE_END();

		::RadiusDamage(pev->origin, pev, pev, gSkillData.voltigoreDmgExplode, 512, 0, DMG_POISON | DMG_ACID);

		SetThink(&CVoltigoreShock::SUB_Remove);
		pev->nextthink = gpGlobals->time;

		return;
	}

	SetThink(&CVoltigore::ExplodeThink); // prevent base class from stopping the Think calls due to being dead
	pev->nextthink = gpGlobals->time + 0.1;
}

void CVoltigore::Killed(entvars_t* pevAttacker, int iGib)
{
	if (pev->deadflag != DEAD_NO)
		return;

	UTIL_Remove(m_handShock);
	m_handShock = NULL;
	for (int i = 0; i < SHOCK_CHARGE_BEAMS; i++) {
		UTIL_Remove(m_pBeam[i]);
		m_pBeam[i] = NULL;
	}

	for (int i = 0; i < SHOCK_DEATH_BEAMS; i++) {
		m_pDeathBeam[i] = CBeam::BeamCreate(BEAM_SPRITE, 30);
		m_pDeathBeam[i]->PointsInit(RandomBeamPoint(pev), pev->origin + Vector(0,0,32));
		m_pDeathBeam[i]->SetColor(255, 16, 128);
		m_pDeathBeam[i]->SetBrightness(128);
		m_pDeathBeam[i]->SetNoise(80);
	}

	CBaseMonster::Killed(pevAttacker, GIB_NEVER);

	if (ShouldGibMonster(iGib)) {
		explodeTime = gpGlobals->time + 0.1;
	} else {
		explodeTime = gpGlobals->time + RANDOM_FLOAT(4, 7);
	}
	
	SetThink(&CVoltigore::ExplodeThink);
	pev->nextthink = gpGlobals->time;
}

int CVoltigore::IgnoreConditions(void)
{
	int iIgnore = CBaseMonster::IgnoreConditions();
	iIgnore |= (bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE); // no flinching
	return iIgnore;
}

Schedule_t* CVoltigore::GetScheduleOfType(int Type) {
	HideChargeBeam();

	if (Type == SCHED_RANGE_ATTACK1) {
		ShowChargeBeam();
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

void CVoltigore::PainSound(void)
{
	int pitch = 100 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CVoltigore::AlertSound(void)
{
	int pitch = 100 + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CVoltigore::IdleSound(void)
{
	int pitch = 100 + RANDOM_LONG(-5, 5);

	// Play a random idle sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch);
}

//
// voltigore shock
//

void CVoltigoreShock::Spawn(void)
{
	pev->movetype = MOVETYPE_FLY;
	pev->classname = MAKE_STRING("voltigoreshock");

	pev->solid = SOLID_BBOX;
	pev->rendermode = kRenderTransAdd;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), SHOCK_SPRITE);
	pev->frame = 0;
	pev->scale = 0.3f;
	beamUpdateIdx = 0;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	for (int i = 0; i < SHOCK_FLY_BEAMS; i++) {
		m_pBeam[i] = CBeam::BeamCreate(BEAM_SPRITE, 30);
		m_pBeam[i]->PointEntInit(RandomBeamPoint(pev), entindex());
		m_pBeam[i]->SetColor(255, 16, 128);
		m_pBeam[i]->SetBrightness(255);
		m_pBeam[i]->SetNoise(80);
	}

	EMIT_SOUND(ENT(pev), CHAN_BODY, SHOCK_SOUND, 1, 0.6f);
}

void CVoltigoreShock::Fly(void)
{
	pev->nextthink = gpGlobals->time + 0.1;

	for (int i = 0; i < 2; i++) {
		m_pBeam[beamUpdateIdx]->PointEntInit(RandomBeamPoint(pev), entindex());
		beamUpdateIdx = (beamUpdateIdx + 1) % SHOCK_FLY_BEAMS;
	}
}

void CVoltigoreShock::Shock(void)
{
	if (shocksLeft == 0) {
		for (int i = 0; i < SHOCK_FLY_BEAMS; i++) {
			UTIL_Remove(m_pBeam[i]);
		}

		SetThink(&CVoltigoreShock::SUB_Remove);
		pev->nextthink = gpGlobals->time;

		return;
	}

	pev->nextthink = gpGlobals->time + 0.1;

	entvars_t* pevOwner = pev->owner ? VARS(pev->owner) : NULL;

	// TODO: damage and class ignore
	RadiusDamage(pev->origin, pev, pevOwner, gSkillData.voltigoreDmgBeam, 128, CLASS_ALIEN_MONSTER, DMG_SHOCK);
	shocksLeft--;
}

void CVoltigoreShock::Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity)
{
	CVoltigoreShock* pSpit = GetClassPtr((CVoltigoreShock*)NULL);
	pSpit->Spawn();

	UTIL_SetOrigin(pSpit->pev, vecStart);
	pSpit->pev->velocity = vecVelocity;
	pSpit->pev->owner = ENT(pevOwner);

	pSpit->SetThink(&CVoltigoreShock::Fly);
	pSpit->pev->nextthink = gpGlobals->time + 0.1;
}

void CVoltigoreShock::Touch(CBaseEntity* pOther)
{
	shocksLeft = 5;
	SetThink(&CVoltigoreShock::Shock);
	pev->velocity = g_vecZero;
	pev->nextthink = gpGlobals->time;
}

Vector RandomBeamPoint(entvars_t* owner) {
	Vector vecSrc = owner->origin;
	float radius = 1024;
	int iLoops = 0;
	Vector bestPos;
	bool foundAnyPoint = false;
	bool foundAnyDistantPoint = false;

	for (iLoops = 0; iLoops < 10; iLoops++)
	{
		Vector vecDir1 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		vecDir1 = vecDir1.Normalize();
		TraceResult	tr;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir1 * radius, ignore_monsters, ENT(owner), &tr);

		if (!foundAnyPoint) {
			foundAnyPoint = true;
			bestPos = tr.vecEndPos;
		}

		if ((tr.vecEndPos - vecSrc).Length() < 80)
			continue;

		if (!foundAnyDistantPoint) {
			foundAnyDistantPoint = true;
			bestPos = tr.vecEndPos;
		}

		if (tr.flFraction == 1.0)
			continue;

		return tr.vecEndPos;
	}

	return bestPos;
}
