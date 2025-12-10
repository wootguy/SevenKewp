#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "weapons.h"
#include "customentity.h"
#include "defaultai.h"
#include "CSoundEnt.h"
#include "CAGrunt.h"
#include "CTor.h"
#include "effects.h"

// TODO:
// range attack if can't reach target
// increase fps of most animations

LINK_ENTITY_TO_CLASS(monster_alien_tor, CTor)

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
	"tor/tor-attack1.wav",
	"tor/tor-attack2.wav"
};

const char* CTor::pIdleSounds[] =
{
	"tor/tor-idle.wav",
	"tor/tor-idle2.wav",
	"tor/tor-idle3.wav"
};

const char* CTor::pAlertSounds[] =
{
	"tor/tor-alerted.wav"
};

const char* CTor::pPainSounds[] =
{
	"tor/tor-pain.wav"
};

const char* CTor::pRunSounds[] =
{
	"tor/tor-foot.wav"
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
	//{ TASK_FIND_COVER_FROM_ENEMY,	(float)0	},
	//{ TASK_RUN_PATH,				(float)0	},
	//{ TASK_WAIT_FOR_MOVEMENT,		(float)0	}
};

Schedule_t	slSummonAttack[] =
{
	{
		tlSummonAttack,
		ARRAYSIZE(tlSummonAttack),
		0, 

		0,
		"TOR_SUMMON_ATTACK"
	},
};

// uninterruptable melee attack
Task_t	tlSlamAttack[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_MELEE_ATTACK1,		(float)0		},
};

Schedule_t	slSlamAttack[] =
{
	{
		tlSlamAttack,
		ARRAYSIZE(tlSlamAttack),
		0,
		0,
		"TOR_SLAM_ATTACK"
	},
};

DEFINE_CUSTOM_SCHEDULES(CTor)
{
	slSummonAttack,
	slSlamAttack
};

IMPLEMENT_CUSTOM_SCHEDULES(CTor, CBaseMonster)

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

	pev->yaw_speed = ys * gSkillData.sk_yawspeed_mult;
}

void CTor::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case EVENT_SLAM:
		SlamAttack();
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_GUN_VOLUME, 0.3);
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
		m_failedMelees = 0;
		break;
	case EVENT_SUMMON_GRUNT:
		//ALERT(at_console, "SUMMON EVENT\n");
		break;
	case EVENT_STAFF_SWING:
	case EVENT_STAFF_STAB:
	{
		bool isRightSwing = pEvent->event == EVENT_STAFF_SWING;
		CBaseEntity* pHurt = CheckTraceHullAttack(MELEE_ATTACK_DISTANCE, gSkillData.sk_tor_punch, DMG_SLASH);
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_GUN_VOLUME, 0.3);

		if (pHurt)
		{
			m_failedMelees = 0;
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
		else { // Play a random attack miss sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			m_failedMelees++;
			ALERT(at_console, "OOF MISSED %d\n", m_failedMelees);
		}
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

Schedule_t* CTor::GetSchedule(void)
{
	if (HasConditions(bits_COND_HEAVY_DAMAGE))
	{
		ClearConditions(bits_COND_HEAVY_DAMAGE);
		return GetScheduleOfType(SCHED_SMALL_FLINCH);
	}

	if (HasConditions(bits_COND_LIGHT_DAMAGE))
	{
		ClearConditions(bits_COND_LIGHT_DAMAGE);
		// never flinch or retreat from light damage
		return CBaseMonster::GetSchedule();
	}

	return CBaseMonster::GetSchedule();
}

Schedule_t* CTor::GetScheduleOfType(int Type) {
	switch (Type) {
	case SCHED_MELEE_ATTACK1:
		return &slSlamAttack[0];
	case SCHED_MELEE_ATTACK2:
		AttackSound();
		break;
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
			CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_GUN_VOLUME, 0.3);

			Vector vecSrc, angles;
			GetAttachment(0, vecSrc, angles);
			CBaseEntity* target = m_hEnemy;
			
			if (target) {
				Vector vecDir1 = (target->BodyTarget(pev->origin) - vecSrc).Normalize();

				TraceResult	tr;
				UTIL_TraceLine(vecSrc, vecSrc + vecDir1 * TOR_SHOOT_RANGE, dont_ignore_monsters, edict(), &tr);

				CBeam* beam = CBeam::BeamCreate(SHOOT_BEAM_SPRITE, 50);
				beam->PointsInit(vecSrc, tr.vecEndPos);
				beam->SetFlags(SF_BEAM_TEMPORARY);
				beam->SetColor(beamColor1.x, beamColor1.y, beamColor1.z);
				beam->SetBrightness(150);
				beam->SetNoise(10);
				beam->SetScrollRate(150);
				beam->LiveForTime(0.5f);

				CBeam* beam2 = CBeam::BeamCreate(SHOOT_BEAM_SPRITE, 50);
				beam2->PointsInit(vecSrc, tr.vecEndPos);
				beam2->SetFlags(SF_BEAM_TEMPORARY | BEAM_FSINE);
				beam2->SetColor(beamColor2.x, beamColor2.y, beamColor2.z);
				beam2->SetBrightness(150);
				beam2->SetNoise(15);
				beam2->SetScrollRate(150);
				beam2->LiveForTime(0.5f);

				CBaseEntity* phit = CBaseEntity::Instance(tr.pHit);
				if (phit) {
					phit->TakeDamage(pev, pev, gSkillData.sk_tor_energybeam, DMG_ENERGYBEAM);

					if (phit->IsMonster() && (phit->pev->movetype == MOVETYPE_STEP || phit->IsPlayer())) {
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
	bool shouldMelee = flDist < MELEE_CHASE_DISTANCE && m_failedMelees < 2;
	if (!startedSummon && !shouldMelee && flDist < TOR_SHOOT_RANGE) {
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

		if ((bitsDamageType & DMG_BULLET) && RANDOM_LONG(0, 1) == 0)
		{
			Vector vecTracerDir = vecDir;

			vecTracerDir.x += RANDOM_FLOAT(-0.3, 0.3);
			vecTracerDir.y += RANDOM_FLOAT(-0.3, 0.3);
			vecTracerDir.z += RANDOM_FLOAT(-0.3, 0.3);

			vecTracerDir = vecTracerDir * -512;

			UTIL_Tracer(ptr->vecEndPos, ptr->vecEndPos + vecTracerDir);
		}

		flDamage -= 20;
		if (flDamage <= 0)
			flDamage = 0.1;// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
		
		AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);
	}
	else
	{
		CBaseMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
	}
}

void CTor::Spawn()
{
	Precache();

	InitModel();
	SetSize(Vector(-24, -24, 0), Vector(24, 24, 72));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BloodColorAlien();
	pev->view_ofs = Vector(0, 0, 0);// position of the eyes relative to monster's origin.
	m_flFieldOfView = 0.0;
	m_MonsterState = MONSTERSTATE_NONE;
	nextShoot = nextBeamBurst = nextBeam = shotsFired = burstShotsFired = 0;

	summon_cname = ALLOC_STRING(SUMMON_CLASSNAME);
	numChildren = 0;
	m_flinchChance = 33;

	MonsterInit();

	if (CBaseEntity::IRelationship(Classify(), CLASS_PLAYER) == R_AL) {
		beamColor1 = Vector(16, 255, 255);
		beamColor2 = Vector(128, 255, 255);
	}
	else {
		beamColor1 = Vector(96, 128, 16);
		beamColor2 = Vector(96, 255, 16);
	}	
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

void CTor::StartFollowingSound() {
	int pitch = 100 + RANDOM_LONG(0, 9);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CTor::StopFollowingSound() {
	int pitch = 100 + RANDOM_LONG(0, 9);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CTor::CantFollowSound() {
	int pitch = 100 + RANDOM_LONG(0, 9);
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
			pEntity->TakeDamage(pev, pev, gSkillData.sk_tor_sonicblast * launchPower, DMG_SONIC);

			if (pEntity->IsPlayer()) {
				pEntity->pev->punchangle.x = 10;
			}
		}
	}

	int pitch = 100 + RANDOM_LONG(-5, 5);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pSlamSounds), 1.0, ATTN_NORM, 0, pitch);

	float radius = (SLAM_ATTACK_RADIUS + 50) / 0.3f;
	UTIL_BeamCylinder(pev->origin, radius, slamSpriteIdx, 0, 0, 2, 12, 0, RGBA(255, 255, 255, 255), 0);

	PLAY_DISTANT_SOUND(edict(), DISTANT_BOOM);
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
		ALERT(at_console, "Failed to summon\n");
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
	
	CBaseEntity* ent = CBaseEntity::Instance(pent);
	CBaseMonster* mon = ent->MyMonsterPointer();
	if (mon) {
		mon->m_Classify = m_Classify;
		mon->m_IsPlayerAlly = m_IsPlayerAlly;
	}

	DispatchSpawn(ENT(pevCreate));
	pevCreate->owner = edict(); // TODO: this causes alien grunts to be owned by their hornets when tor dies

	
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
