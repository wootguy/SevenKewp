#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "weapons.h"
#include "customentity.h"
#include "defaultai.h"
#include "CSoundEnt.h"
#include "CAGrunt.h"
#include "CBaseMonster.h"

// TODO:
// range attack if can't reach target
// increase fps of most animations

#define EVENT_SLAM 1
#define EVENT_STAFF_SWING 2
#define EVENT_SHOOT 3
#define EVENT_SUMMON_GRUNT 4
#define EVENT_STAFF_STAB 7
#define EVENT_STEP_RIGHT 10
#define EVENT_STEP_LEFT 11

#define MELEE_ATTACK_DISTANCE 96
#define MELEE_CHASE_DISTANCE 300

#define SLAM_CHECK_DISTANCE 150 // how close enemies need to be for a slam to be considered
#define SLAM_ATTACK_RADIUS 300 // radius of the attack
#define SLAM_ATTACK_ENEMY_COUNT 2 // minimum enemies nearby needed to do a slam

#define MAX_BEAM_SHOTS  5
#define SHOOT_RANGE 4096

#define SUMMON_DISTANCE 256
#define SUMMON_HEIGHT 80
#define MAX_POSSIBLE_CHILDREN 128
#define MAX_ALLOWED_CHILDREN 3

#define SHOCK_WAVE_SPRITE "sprites/shockwave.spr"
#define SHOOT_SOUND MOD_SND_FOLDER "tor/tor-staff-discharge.wav"
#define SHOOT_BEAM_SPRITE "sprites/xenobeam.spr"
#define PORTAL_SPRITE "sprites/exit1.spr"
#define PORTAL_BEAM_SPRITE "sprites/lgtning.spr"
#define PORTAL_SOUND "debris/beamstart8.wav"
#define PORTAL_SOUND2 "debris/beamstart7.wav"
#define SUMMON_SOUND MOD_SND_FOLDER "tor/tor-summon.wav"
#define SUMMON_CLASSNAME "monster_alien_grunt"

int slamSpriteIdx = 0;

class CTor : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int Classify(void);
	const char* DisplayName();
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	Schedule_t* GetScheduleOfType(int Type);
	void MonsterThink(void);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack2(float flDot, float flDist);
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckMeleeAttack2(float flDot, float flDist);
	int LookupActivity(int activity);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	void DeathNotice(entvars_t* pevChild);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);
	void AttackSound(void);

#if 0
	// debugging schedules
	virtual void ScheduleChange(void) {
		const char* schedName = m_pSchedule != NULL ? m_pSchedule->pName : "NULL";
		println("\nSchedule changing from <%s> because:", schedName);

		if (m_MonsterState != m_IdealMonsterState) {
			println("- monster state changing from %d to %d", m_MonsterState, m_IdealMonsterState);
		}
		if (!FScheduleValid()) {
			if (m_pSchedule == NULL) {
				println("- Schedule is NULL");
				return;
			}
			if (HasConditions(bits_COND_SCHEDULE_DONE)) {
				println("- Schedule is finished");
			}
			if (HasConditions(bits_COND_TASK_FAILED)) {
				println("- Task failed");
			}
			if (HasConditions(m_pSchedule->iInterruptMask)) {
				println("- Interrupted by %d", m_pSchedule->iInterruptMask);
			}
		}
	}
#endif

	CUSTOM_SCHEDULES;

private:
	void SlamAttack();
	bool GetSummonPos(Vector& pos);
	void StartSummon();
	void SpawnGrunt();

	int shotsFired;
	float nextShoot; // next time allowed to begin shooting
	float nextBeamBurst; // next time a burst shot will be fired
	float nextBeam; // next time a single beam will be fired
	int burstShotsFired; // number of shots fired in the current burst
	float nextSummon; // next time a grunt can be spawned
	float nextSummonSpawn; // time a grunt will be summoned if still doing the summon activity
	bool startedSummon;
	int numChildren;
	string_t summon_cname;
	Vector summonPos;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pAttackSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pRunSounds[];
	static const char* pSlamSounds[];
};

LINK_ENTITY_TO_CLASS(monster_alien_tor, CTor);
LINK_ENTITY_TO_CLASS(monster_kingpin, CTor);

const char* CTor::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CTor::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CTor::pAttackSounds[] =
{
	MOD_SND_FOLDER "tor/tor-attack1.wav",
	MOD_SND_FOLDER "tor/tor-attack2.wav"
};

const char* CTor::pIdleSounds[] =
{
	MOD_SND_FOLDER "tor/tor-idle.wav",
	MOD_SND_FOLDER "tor/tor-idle2.wav",
	MOD_SND_FOLDER "tor/tor-idle3.wav"
};

const char* CTor::pAlertSounds[] =
{
	MOD_SND_FOLDER "tor/tor-alerted.wav"
};

const char* CTor::pPainSounds[] =
{
	MOD_SND_FOLDER "tor/tor-pain.wav"
};

const char* CTor::pRunSounds[] =
{
	MOD_SND_FOLDER "tor/tor-foot.wav"
};

const char* CTor::pSlamSounds[] =
{
	"houndeye/he_blast1.wav",
	"houndeye/he_blast2.wav",
	"houndeye/he_blast3.wav"
};

Task_t	tlSummonAttack[] =
{
	{ TASK_STOP_MOVING,				0			},
	{ TASK_RANGE_ATTACK2,			(float)0	},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0	},
	{ TASK_RUN_PATH,				(float)0	},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0	}
};

Schedule_t	slSummonAttack[] =
{
	{
		tlSummonAttack,
		ARRAYSIZE(tlSummonAttack),
		0, 

		0,
		"Summon Attack"
	},
};

DEFINE_CUSTOM_SCHEDULES(CTor)
{
	slSummonAttack
};

IMPLEMENT_CUSTOM_SCHEDULES(CTor, CBaseMonster);

int	CTor::Classify(void)
{
	return CBaseMonster::Classify(CLASS_ALIEN_MILITARY);
}

const char* CTor::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Tor";
}

void CTor::SetYawSpeed(void)
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

void CTor::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case EVENT_SLAM:
		SlamAttack();
		break;
	case EVENT_SHOOT:
		nextBeam = nextBeamBurst = gpGlobals->time;

		// pause sequence to prevent triggering more of these events
		pev->framerate = 0.001f; 
		m_fSequenceFinished = false;

		shotsFired++;
		if (shotsFired >= MAX_BEAM_SHOTS) {
			shotsFired = 0;
			nextShoot = gpGlobals->time + 3.0f;
		}
		break;
	case EVENT_SUMMON_GRUNT:
		//println("SUMMON EVENT");
		break;
	case EVENT_STAFF_SWING:
	case EVENT_STAFF_STAB:
	{
		bool isRightSwing = pEvent->event == EVENT_STAFF_SWING;
		CBaseEntity* pHurt = CheckTraceHullAttack(MELEE_ATTACK_DISTANCE, gSkillData.torDmgPunch, DMG_SLASH);

		if (pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				if (isRightSwing) {
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->punchangle.z = 18;
					pHurt->pev->velocity = pHurt->pev->velocity + (gpGlobals->v_right + gpGlobals->v_forward + gpGlobals->v_up) * 200;
				}
				else {
					pHurt->pev->punchangle.x = 18;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100;
				}
			}
			// Play a random attack hit sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else // Play a random attack miss sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		break;
	}
	case EVENT_STEP_LEFT:
	case EVENT_STEP_RIGHT:
	{
		int pitch = pEvent->event == EVENT_STEP_RIGHT ? 100 : 120;
		EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, RANDOM_SOUND_ARRAY(pRunSounds), 1.0, ATTN_NORM, 0, pitch);
		break;
	}
	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

Schedule_t* CTor::GetScheduleOfType(int Type) {
	if (Type == SCHED_RANGE_ATTACK2) {
		return slSummonAttack;
	}
	if (Type == SCHED_MELEE_ATTACK2) {
		AttackSound();
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

void CTor::MonsterThink(void) {
	if (nextBeamBurst && nextBeamBurst < gpGlobals->time) {
		pev->framerate = 0.001f;

		if (nextBeam < gpGlobals->time) {
			nextBeam = gpGlobals->time + 0.05;
			burstShotsFired++;
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, SHOOT_SOUND, 1.0, ATTN_NORM, 0, 100);

			Vector vecSrc, angles;
			GetAttachment(0, vecSrc, angles);
			CBaseEntity* target = m_hEnemy;
			
			if (target) {
				Vector vecDir1 = (target->pev->origin - vecSrc).Normalize();

				TraceResult	tr;
				UTIL_TraceLine(vecSrc, vecSrc + vecDir1 * SHOOT_RANGE, dont_ignore_monsters, edict(), &tr);

				CBeam* beam = CBeam::BeamCreate(SHOOT_BEAM_SPRITE, 50);
				beam->PointsInit(vecSrc, tr.vecEndPos);
				beam->SetFlags(SF_BEAM_TEMPORARY);
				beam->SetColor(96, 128, 16);
				beam->SetBrightness(150);
				beam->SetNoise(10);
				beam->SetScrollRate(150);
				beam->LiveForTime(0.5f);

				CBeam* beam2 = CBeam::BeamCreate(SHOOT_BEAM_SPRITE, 50);
				beam2->PointsInit(vecSrc, tr.vecEndPos);
				beam2->SetFlags(SF_BEAM_TEMPORARY | BEAM_FSINE);
				beam2->SetColor(96, 255, 16);
				beam2->SetBrightness(150);
				beam2->SetNoise(15);
				beam2->SetScrollRate(150);
				beam2->LiveForTime(0.5f);

				CBaseEntity* phit = CBaseEntity::Instance(tr.pHit);
				if (phit) {
					phit->TakeDamage(pev, pev, gSkillData.torDmgEnergyBeam, DMG_ENERGYBEAM);

					if (phit->IsMonster()) {
						phit->pev->velocity.z += (phit->pev->flags & FL_ONGROUND) ? 200 : 200;
					}
				}
			}
			
			if (burstShotsFired >= 3) {
				nextBeam = 0;
				nextBeamBurst = 0;
				burstShotsFired = 0;

				// finish the shoot task
				pev->framerate = 1.0f;
				m_fSequenceFinished = true;
			}
		}
	}

	if (m_Activity == ACT_RANGE_ATTACK2) {
		if (nextSummonSpawn && nextSummonSpawn < gpGlobals->time) {
			SpawnGrunt();
		}
		if (!startedSummon && nextSummon < gpGlobals->time) {
			StartSummon();
		}
		if (m_fSequenceFinished) {
			startedSummon = false;
		}
	} else {
		nextSummonSpawn = 0;
	}

	CBaseMonster::MonsterThink();
}

BOOL CTor::CheckRangeAttack1(float flDot, float flDist)
{
	if (!startedSummon && flDist > MELEE_CHASE_DISTANCE && flDist < SHOOT_RANGE) {
		if (gpGlobals->time > nextShoot)
			return TRUE;
	}

	//shotsFired = 0;
	return FALSE;
}

BOOL CTor::CheckRangeAttack2(float flDot, float flDist)
{
	if (startedSummon) {
		return TRUE;
	}

	Vector dummy;
	if (numChildren < MAX_ALLOWED_CHILDREN && nextSummon < gpGlobals->time && GetSummonPos(dummy)) {
		return TRUE;
	}
	return FALSE;
}

BOOL CTor::CheckMeleeAttack1(float flDot, float flDist)
{
	int nearbyEnemies = 0;

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, SLAM_CHECK_DISTANCE)) != NULL)
	{
		if (pEntity->IsMonster() && pEntity->IsAlive() && IRelationship(pEntity) >= R_DL)
		{
			nearbyEnemies++;
			if (nearbyEnemies >= SLAM_ATTACK_ENEMY_COUNT) {
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CTor::CheckMeleeAttack2(float flDot, float flDist)
{
	if (flDist <= MELEE_ATTACK_DISTANCE) {
		return TRUE;
	}

	return FALSE;
}

int CTor::LookupActivity(int activity)
{
	ASSERT(activity != 0);
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	switch (activity) {
	case ACT_RANGE_ATTACK1:
	{
		return ::LookupActivityWithOffset(pmodel, pev, activity, 1);
	}
	default:
		return ::LookupActivity(pmodel, pev, activity);
	}
}

void CTor::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if (ptr->iHitgroup == 10 && (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB)))
	{
		// hit armor
		if (pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0, 10) < 1))
		{
			UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(1, 2));
			pev->dmgtime = gpGlobals->time;
		}

		if (RANDOM_LONG(0, 1) == 0)
		{
			Vector vecTracerDir = vecDir;

			vecTracerDir.x += RANDOM_FLOAT(-0.3, 0.3);
			vecTracerDir.y += RANDOM_FLOAT(-0.3, 0.3);
			vecTracerDir.z += RANDOM_FLOAT(-0.3, 0.3);

			vecTracerDir = vecTracerDir * -512;

			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, ptr->vecEndPos);
				WRITE_BYTE(TE_TRACER);
				WRITE_COORD(ptr->vecEndPos.x);
				WRITE_COORD(ptr->vecEndPos.y);
				WRITE_COORD(ptr->vecEndPos.z);

				WRITE_COORD(vecTracerDir.x);
				WRITE_COORD(vecTracerDir.y);
				WRITE_COORD(vecTracerDir.z);
			MESSAGE_END();
		}

		flDamage -= 20;
		if (flDamage <= 0)
			flDamage = 0.1;// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
	}
	else
	{
		SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.
		TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
	}

	AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);
}

void CTor::Spawn()
{
	Precache();

	SET_MODEL(ENT(pev), GetModel());
	SetSize(Vector(-24, -24, 0), Vector(24, 24, 88));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	if (!pev->health) pev->health = gSkillData.torHealth;
	pev->view_ofs = Vector(0, 0, 0);// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.0;
	m_MonsterState = MONSTERSTATE_NONE;
	nextShoot = nextBeamBurst = nextBeam = shotsFired = burstShotsFired = 0;

	summon_cname = ALLOC_STRING(SUMMON_CLASSNAME);
	numChildren = 0;

	MonsterInit();
}

void CTor::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/tor.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_MODEL(SHOOT_BEAM_SPRITE);
	PRECACHE_MODEL(PORTAL_BEAM_SPRITE);
	PRECACHE_MODEL(PORTAL_SPRITE);
	slamSpriteIdx = PRECACHE_MODEL(SHOCK_WAVE_SPRITE);

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pRunSounds);
	PRECACHE_SOUND_ARRAY(pSlamSounds);

	PRECACHE_SOUND(SHOOT_SOUND);
	PRECACHE_SOUND(SUMMON_SOUND);
	PRECACHE_SOUND(PORTAL_SOUND);
	PRECACHE_SOUND(PORTAL_SOUND2);

	UTIL_PrecacheOther(SUMMON_CLASSNAME);
}

void CTor::PainSound(void)
{
	int pitch = 100 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CTor::AlertSound(void)
{
	int pitch = 100 + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CTor::IdleSound(void)
{
	int pitch = 100 + RANDOM_LONG(-5, 5);

	// Play a random idle sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CTor::AttackSound(void) {
	int pitch = 100 + RANDOM_LONG(-5, 5);

	// Play a random attack sound
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CTor::SlamAttack() {
	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, SLAM_ATTACK_RADIUS)) != NULL)
	{
		if (pEntity->IsMonster() && pEntity->IsAlive() && pEntity->entindex() != entindex() && IRelationship(pEntity) >= R_DL)
		{
			Vector delta = pEntity->pev->origin - pev->origin;
			Vector pushDir = delta.Normalize();
			pushDir.z = 0;

			float launchPower = 1.0f - ((delta.Length() - 64) * 0.5f / SLAM_ATTACK_RADIUS);
			launchPower = fmaxf(0.7f, launchPower);
			float pushPower = 1.0f - launchPower;

			Vector launchForce = Vector(0, 0, 1) * 1000 * launchPower;
			Vector pushForce = pushDir * 1000 * pushPower;

			pEntity->pev->velocity = pEntity->pev->velocity + launchForce + pushForce;
			pEntity->TakeDamage(pev, pev, gSkillData.torDmgSonicBlast * launchPower, DMG_SONIC);

			if (pEntity->IsPlayer()) {
				pEntity->pev->punchangle.x = 10;
			}
		}
	}

	int pitch = 100 + RANDOM_LONG(-5, 5);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pSlamSounds), 1.0, ATTN_NORM, 0, pitch);

	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_BEAMCYLINDER);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 16);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z + 16 + (SLAM_ATTACK_RADIUS + 50) / 0.3f); // reach damage radius over .3 seconds
		WRITE_SHORT(slamSpriteIdx);
		WRITE_BYTE(0); // startframe
		WRITE_BYTE(0); // framerate
		WRITE_BYTE(2); // life
		WRITE_BYTE(12);  // width
		WRITE_BYTE(0);   // noise

		WRITE_BYTE(255);	// red
		WRITE_BYTE(255);	// green
		WRITE_BYTE(255);	// blue

		WRITE_BYTE(255); //brightness
		WRITE_BYTE(0);		// speed
	MESSAGE_END();
}

bool CTor::GetSummonPos(Vector& pos) {
	const int attempts = 8;

	Vector startPos = pev->origin;
	startPos.z += (pev->maxs.z - pev->mins.z)*0.5f;

	for (int i = 0; i < attempts; i++) {
		// random point on circle
		float c = RANDOM_FLOAT(0, 2*M_PI);
		float x = cos(c) * SUMMON_DISTANCE;
		float y = sin(c) * SUMMON_DISTANCE;
		float z = startPos.z + SUMMON_HEIGHT;

		Vector checkPos = pev->origin + Vector(x, y, z);

		TraceResult tr;
		UTIL_TraceHull(startPos, checkPos, ignore_monsters, large_hull, ENT(pev), &tr);

		if (tr.fStartSolid || tr.flFraction < 0.5f) {
			continue;
		}

		// move a little bit inward to avoid spawning inside a wall/ceiling
		Vector dir = (checkPos - pev->origin).Normalize();
		pos = tr.vecEndPos - dir*40;

		return true; 
	}

	return false;
}

void CTor::StartSummon() {
	if (!GetSummonPos(summonPos)) {
		println("Failed to summon");
		return;
	}

	nextSummon = gpGlobals->time + RANDOM_FLOAT(5.0f, 10.0f);
	nextSummonSpawn = gpGlobals->time + 2.0f;

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, SUMMON_SOUND, 1.0, ATTN_NORM, 0, 100);

	Vector startPos = pev->origin;
	startPos.z += (pev->maxs.z - pev->mins.z) * 0.5f;

	for (int i = 0; i < 3; i++) {
		CBeam* beam = CBeam::BeamCreate(SHOOT_BEAM_SPRITE, 30);
		beam->PointsInit(startPos, summonPos);
		beam->SetFlags(BEAM_FSHADEOUT);
		beam->SetColor(96, 255, 32);
		beam->SetBrightness(80);
		beam->SetNoise(80);
		beam->LiveForTime(2.0f);
	}

	CSprite* portalSprite = CSprite::SpriteCreate(PORTAL_SPRITE, summonPos, TRUE);
	portalSprite->SetScale(2.0f);
	portalSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 128, 0);
	portalSprite->AnimateAndDie(10);
	EMIT_SOUND_DYN(ENT(portalSprite->pev), CHAN_ITEM, PORTAL_SOUND, 1.0, ATTN_NORM, 0, 100);

	summonPos.z -= 40;
}

void CTor::SpawnGrunt() {
	edict_t* pent = CREATE_NAMED_ENTITY(summon_cname);

	if (FNullEnt(pent))
	{
		ALERT(at_console, UTIL_VarArgs("NULL Ent '%s' spawned by TOR!\n", summon_cname));
		return;
	}

	entvars_t* pevCreate = VARS(pent);
	pevCreate->origin = summonPos;
	pevCreate->angles = pev->angles;
	SetBits(pevCreate->spawnflags, SF_MONSTER_FALL_TO_GROUND);

	DispatchSpawn(ENT(pevCreate));
	pevCreate->owner = edict();

	CBaseEntity* ent = CBaseEntity::Instance(pent);
	CAGrunt* agrunt = dynamic_cast<CAGrunt*>(ent);
	agrunt->playLandAnim = true;

	EMIT_SOUND_DYN(ENT(ent->pev), CHAN_ITEM, PORTAL_SOUND2, 1.0, ATTN_NORM, 0, 100);

	CBaseEntity* pList[256]; // only telefrag a max of 256 entites. SHOULD be way more than enough...
	int count = UTIL_EntitiesInBox(pList, 256, agrunt->pev->absmin, agrunt->pev->absmax, FL_MONSTER | FL_CLIENT, true);
	for (int i = 0; i < count; i++)
	{
		if (pList[i]->entindex() == agrunt->entindex() || pList[i]->pev->takedamage == DAMAGE_NO) {
			continue;
		}

		g_pevLastInflictor = pev;
		pList[i]->Killed(pev, GIB_ALWAYS);
	}

	nextSummonSpawn = 0;
	numChildren++;
}

void CTor::DeathNotice(entvars_t* pevChild) {
	numChildren--;
	if (numChildren < 0) {
		numChildren = 0;
	}
}
