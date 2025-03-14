#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "CSprite.h"
#include "weapons.h"
#include "decals.h"
#include "effects.h"

// TODO:
// new classification for racex
// accurate beam color/width and sprite size

#define VOLTI_EVENT_SHOOT_ORB 1
#define VOLTI_EVENT_SLASH_BOTH 12
#define VOLTI_EVENT_SLASH_RIGHT 13

#define VOLTI_MELEE_ATTACK1_DISTANCE 128
#define VOLTI_SHOCK_CHARGE_BEAMS 3
#define VOLTI_SHOCK_FLY_BEAMS 7
#define VOLTI_SHOCK_DEATH_BEAMS 12

#define VOLTI_SHOCK_SPRITE "sprites/blueflare2.spr"
#define VOLTI_BEAM_SPRITE "sprites/lgtning.spr"
#define VOLTI_SPORE_EXPLODE_SPRITE "sprites/spore_exp_01.spr"
#define VOLTI_SPORE_EXPLODE_SPRITE2 "sprites/tinyspit.spr"
#define VOLTI_SPORE_EXPLODE_SOUND "weapons/splauncher_impact.wav"
#define VOLTI_SHOCK_SOUND "debris/beamstart1.wav"

#define VOLTI_HEAD_ATTACHEMENT 1
#define VOLTI_ARM_LEFT_ATTACHEMENT 2
#define VOLTI_ARM_RIGHT_ATTACHEMENT 3
#define VOLTI_SHOCK_ATTACHMENT 4

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
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();
	const char* GetDeathNoticeWeapon() {
		return IsAlive() ? "weapon_crowbar" : "grenade";
	}
	void UpdateOnRemove(void);
	void RemoveBeams();

	virtual void Revive();

private:
	float m_rangeAttackCooldown; // next time a range attack can be considered
	EHANDLE m_handShock;
	EHANDLE m_pBeam[VOLTI_SHOCK_CHARGE_BEAMS];
	EHANDLE m_pDeathBeam[VOLTI_SHOCK_DEATH_BEAMS];
	float explodeTime;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pDieSounds[];
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
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

private:
	EHANDLE m_pBeam[VOLTI_SHOCK_FLY_BEAMS];
	int beamUpdateIdx;
	int shocksLeft;
};

Vector RandomBeamPoint(entvars_t* owner);

LINK_ENTITY_TO_CLASS(monster_alien_voltigore, CVoltigore)
LINK_ENTITY_TO_CLASS(voltigoreshock, CVoltigoreShock)

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
	"voltigore/voltigore_idle1.wav",
	"voltigore/voltigore_idle2.wav",
	"voltigore/voltigore_idle3.wav"
};

const char* CVoltigore::pAlertSounds[] =
{
	"voltigore/voltigore_alert1.wav",
	"voltigore/voltigore_alert2.wav",
	"voltigore/voltigore_alert3.wav"
};

const char* CVoltigore::pPainSounds[] =
{
	"voltigore/voltigore_pain1.wav",
	"voltigore/voltigore_pain2.wav",
	"voltigore/voltigore_pain3.wav",
	"voltigore/voltigore_pain4.wav",
};

const char* CVoltigore::pDieSounds[] =
{
	"voltigore/voltigore_die1.wav",
	"voltigore/voltigore_die2.wav",
	"voltigore/voltigore_die3.wav",
};

const char* CVoltigore::pEventSounds[] =
{
	// TODO: combine movement sounds?
	"voltigore/voltigore_attack_melee1.wav",
	"voltigore/voltigore_attack_melee2.wav",
	"voltigore/voltigore_attack_shock.wav",
	"voltigore/voltigore_run_grunt1.wav",
	"voltigore/voltigore_run_grunt2.wav",
	"voltigore/voltigore_footstep1.wav",
	"voltigore/voltigore_footstep2.wav",
	"voltigore/voltigore_footstep3.wav",
	"voltigore/voltigore_eat.wav",
	"voltigore/voltigore_communicate3.wav",
	"voltigore/voltigore_attack_shock.wav",
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

	pev->yaw_speed = ys * gSkillData.sk_yawspeed_mult;
}

void CVoltigore::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case VOLTI_EVENT_SLASH_BOTH:
	case VOLTI_EVENT_SLASH_RIGHT:
	{
		bool isRightSwing = pEvent->event == EVENT_SLASH_RIGHT;
		float damage = gSkillData.sk_voltigore_dmg_punch;
		CBaseEntity* pHurt = CheckTraceHullAttack(MELEE_ATTACK1_DISTANCE, damage, DMG_SLASH);
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_GUN_VOLUME, 0.3);

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
	case VOLTI_EVENT_SHOOT_ORB:
	{
		HideChargeBeam();

		int attachIdx = V_min(GetAttachmentCount(), VOLTI_SHOCK_ATTACHMENT) - 1;

		Vector handOrigin, handAngles;
		GetAttachment(attachIdx, handOrigin, handAngles);

		UTIL_MakeVectors(pev->angles);

		UTIL_MakeVectors(pev->angles);
		Vector vecThrowDir = gpGlobals->v_forward;

		if (m_hEnemy) {
			vecThrowDir = ((m_hEnemy->GetTargetOrigin() + m_hEnemy->pev->view_ofs) - handOrigin).Normalize();
		}

		vecThrowDir.x += RANDOM_FLOAT(-0.01, 0.01);
		vecThrowDir.y += RANDOM_FLOAT(-0.01, 0.01);
		vecThrowDir.z += RANDOM_FLOAT(-0.01, 0.01);

		CVoltigoreShock::Shoot(pev, handOrigin, vecThrowDir * 1000);
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_GUN_VOLUME, 0.3);

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

	InitModel();
	SetSize(Vector(-32, -32, 0), Vector(32, 32, 64));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_YELLOW;
	pev->view_ofs = Vector(0, 0, 0);// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.0;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();

	CSprite* sprite = CSprite::SpriteCreate(VOLTI_SHOCK_SPRITE, pev->origin, FALSE);
	sprite->SetScale(0.3f);
	sprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, 0);
	sprite->SetAttachment(edict(), VOLTI_SHOCK_ATTACHMENT);
	m_handShock = sprite;

	for (int i = 0; i < VOLTI_SHOCK_CHARGE_BEAMS; i++ ) {
		CBeam* beam = CBeam::BeamCreate(VOLTI_BEAM_SPRITE, 30);
		beam->EntsInit(entindex(), entindex());
		beam->SetStartAttachment(i+1); // head, left hand, right hand
		beam->SetEndAttachment(4);
		if (CBaseEntity::IRelationship(Classify(), CLASS_PLAYER) == R_AL) {
			beam->SetColor(140, 255, 96);
		}
		else {
			beam->SetColor(255, 16, 128);
		}
		beam->SetBrightness(255);
		beam->SetNoise(80);
		m_pBeam[i] = beam;
	}

	HideChargeBeam();
}

void CVoltigore::ShowChargeBeam() {
	for (int i = 0; i < VOLTI_SHOCK_CHARGE_BEAMS; i++) {
		if (m_pBeam[i])
			m_pBeam[i]->pev->effects &= ~EF_NODRAW;
	}

	CSprite* sprite = (CSprite*)m_handShock.GetEntity();
	if (sprite)
		sprite->TurnOn();
}

void CVoltigore::HideChargeBeam() {
	for (int i = 0; i < VOLTI_SHOCK_CHARGE_BEAMS; i++) {
		if (m_pBeam[i])
			m_pBeam[i]->pev->effects |= EF_NODRAW;
	}

	CSprite* sprite = (CSprite*)m_handShock.GetEntity();
	if (sprite)
		sprite->TurnOff();
}

void CVoltigore::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/voltigore.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_REPLACEMENT_MODEL("models/vgibs.mdl");
	PRECACHE_MODEL(VOLTI_BEAM_SPRITE);
	PRECACHE_MODEL(VOLTI_SHOCK_SPRITE);
	PRECACHE_MODEL(VOLTI_SPORE_EXPLODE_SPRITE);
	sporeExplodeSprIdx = PRECACHE_MODEL(VOLTI_SPORE_EXPLODE_SPRITE2);

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDieSounds);

	// not affected by mp_soundvariety (but should be)
	for (int i = 0; i < (int)ARRAYSIZE(pEventSounds); i++) \
		PRECACHE_SOUND((char*)pEventSounds[i]);
	
	PRECACHE_SOUND(VOLTI_SPORE_EXPLODE_SOUND);
	PRECACHE_SOUND(VOLTI_SHOCK_SOUND);
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
		EMIT_SOUND(ENT(pev), CHAN_BODY, VOLTI_SPORE_EXPLODE_SOUND, 1, ATTN_NORM);
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 0.5, ATTN_NORM);

		pev->health = -100; // speeds up the gibs
		g_vecAttackDir = Vector(0,0,-1);
		pev->absmin.z += 16; // prevent gibs from spawning inside the floor
		CGib::SpawnRandomMergedGibs(pev, 10, MERGE_MDL_VGIBS, 0);

		CSprite* expSprite = CSprite::SpriteCreate(VOLTI_SPORE_EXPLODE_SPRITE, pev->origin + Vector(0,0,64), TRUE);
		expSprite->SetScale(4.0f);
		expSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 128, 0);
		expSprite->AnimateAndDie(17);

		Vector sprayPos = pev->origin + Vector(0, 0, 0);
		Vector sprayDir = Vector(0, 0, 1);

		const int count = 200;
		const int speed = 96;
		const int noise = 128;
		UTIL_SpriteSpray(sprayPos, sprayDir, sporeExplodeSprIdx, count, speed, noise);

		::RadiusDamage(pev->origin, pev, pev, gSkillData.sk_voltigore_dmg_explode, 512, 0, DMG_POISON | DMG_ACID);

		SetThink(&CVoltigore::SUB_Remove);
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
	for (int i = 0; i < VOLTI_SHOCK_CHARGE_BEAMS; i++) {
		UTIL_Remove(m_pBeam[i]);
		m_pBeam[i] = NULL;
	}

	if (!m_pDeathBeam[0]) {
		for (int i = 0; i < VOLTI_SHOCK_DEATH_BEAMS; i++) {

			CBeam* beam = CBeam::BeamCreate(VOLTI_BEAM_SPRITE, 30);
			beam->PointsInit(RandomBeamPoint(pev), pev->origin + Vector(0, 0, 32));
			beam->SetColor(255, 16, 128);
			beam->SetBrightness(128);
			beam->SetNoise(80);
			m_pDeathBeam[i] = beam;
		}
	}
	
	CBaseMonster::Killed(pevAttacker, GIB_NEVER);

	if (ShouldGibMonster(iGib)) {
		explodeTime = gpGlobals->time + 0.1;
	} else {
		explodeTime = gpGlobals->time + RANDOM_FLOAT(4, 6);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDieSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(0, 9));
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

void CVoltigore::StartFollowingSound() {
	int r = RANDOM_LONG(0, 1);

	switch (r) {
	case 0:
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "voltigore/voltigore_attack_melee1.wav", 1, ATTN_NORM);
		break;
	case 1:
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "voltigore/voltigore_attack_melee2.wav", 1, ATTN_NORM);
		break;
	}
}

void CVoltigore::StopFollowingSound() {
	EMIT_SOUND(ENT(pev), CHAN_ITEM, RANDOM_SOUND_ARRAY(pPainSounds), 1, ATTN_NORM);
}

void CVoltigore::CantFollowSound() {
	EMIT_SOUND(ENT(pev), CHAN_ITEM, RANDOM_SOUND_ARRAY(pPainSounds), 1, ATTN_NORM);
}

void CVoltigore::RemoveBeams() {
	UTIL_Remove(m_handShock);
	for (int i = 0; i < VOLTI_SHOCK_CHARGE_BEAMS; i++) {
		UTIL_Remove(m_pBeam[i]);
		m_pBeam[i] = NULL;
	}
	for (int i = 0; i < VOLTI_SHOCK_DEATH_BEAMS; i++) {
		UTIL_Remove(m_pDeathBeam[i]);
		m_pDeathBeam[i] = NULL;
	}
}

void CVoltigore::UpdateOnRemove(void) {
	RemoveBeams();
	CBaseEntity::UpdateOnRemove();
}

void CVoltigore::Revive() {
	RemoveBeams();
	CBaseMonster::Revive();
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

	SET_MODEL(ENT(pev), VOLTI_SHOCK_SPRITE);
	pev->frame = 0;
	pev->scale = 0.3f;
	beamUpdateIdx = 0;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	CBaseEntity* owner = CBaseEntity::Instance(pev->owner);
	CBaseMonster* mon = owner ? owner->MyMonsterPointer() : NULL;
	bool isAllyShock = mon && CBaseEntity::IRelationship(mon->Classify(), CLASS_PLAYER) == R_AL;

	for (int i = 0; i < VOLTI_SHOCK_FLY_BEAMS; i++) {
		CBeam* beam = CBeam::BeamCreate(VOLTI_BEAM_SPRITE, 30);
		beam->PointEntInit(RandomBeamPoint(pev), entindex());

		if (isAllyShock) {
			beam->SetColor(140, 255, 96);
		}
		else {
			beam->SetColor(255, 16, 128);
		}
		
		beam->SetBrightness(255);
		beam->SetNoise(80);

		m_pBeam[i] = beam;
	}

	EMIT_SOUND(ENT(pev), CHAN_BODY, VOLTI_SHOCK_SOUND, 1, 0.6f);
}

void CVoltigoreShock::Fly(void)
{
	pev->nextthink = gpGlobals->time + 0.1;
	ParametricInterpolation(0.1f);

	for (int i = 0; i < 2; i++) {
		CBeam* beam = (CBeam*)m_pBeam[beamUpdateIdx].GetEntity();
		if (beam)
			beam->PointEntInit(RandomBeamPoint(pev), entindex());
		beamUpdateIdx = (beamUpdateIdx + 1) % VOLTI_SHOCK_FLY_BEAMS;
	}
}

void CVoltigoreShock::Shock(void)
{
	if (shocksLeft == 0) {
		for (int i = 0; i < VOLTI_SHOCK_FLY_BEAMS; i++) {
			UTIL_Remove(m_pBeam[i]);
		}

		SetThink(&CVoltigoreShock::SUB_Remove);
		pev->nextthink = gpGlobals->time;

		return;
	}

	pev->nextthink = gpGlobals->time + 0.1;

	entvars_t* pevOwner = pev->owner ? VARS(pev->owner) : NULL;

	// TODO: damage and class ignore
	RadiusDamage(pev->origin, pev, pevOwner, gSkillData.sk_voltigore_dmg_beam, 128, CLASS_ALIEN_MONSTER, DMG_SHOCK);
	shocksLeft--;
}

void CVoltigoreShock::Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity)
{
	CVoltigoreShock* pSpit = GetClassPtr((CVoltigoreShock*)NULL);
	pSpit->pev->owner = ENT(pevOwner);

	pSpit->Spawn();

	UTIL_SetOrigin(pSpit->pev, vecStart);
	pSpit->pev->velocity = vecVelocity;

	pSpit->SetThink(&CVoltigoreShock::Fly);
	pSpit->pev->nextthink = gpGlobals->time + 0.1;
	pSpit->ParametricInterpolation(0.1f);
}

void CVoltigoreShock::Touch(CBaseEntity* pOther)
{
	if (pev->velocity == g_vecZero) {
		return;
	}

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
