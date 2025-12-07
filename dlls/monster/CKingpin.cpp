#include "extdll.h"
#include "util.h"
#include "CKingpin.h"

int orbSprite;
int orbShockWaveSpriteIdx;
int orbFollowSprite;
int kingBeamSpriteIdx;

const int kingpinPitch = 160;

LINK_ENTITY_TO_CLASS(kingpin_plasma_ball, CKingpinBall)

void CKingpinBall::Spawn(void)
{
	m_hOwner = Instance(pev->owner);

	Precache();
	m_flFieldOfView = VIEW_FIELD_FULL;
	pev->movetype = MOVETYPE_BOUNCE;
	pev->solid = SOLID_BBOX;
	
	pev->gravity = FLT_MIN;
	pev->friction = 2.0f;

	SET_MODEL(ENT(pev), ORB_SPRITE);
	pev->rendermode = kRenderTransAdd;
	pev->rendercolor.x = 255;
	pev->rendercolor.y = 255;
	pev->rendercolor.z = 255;
	pev->renderamt = 255;
	pev->scale = 1.3f;

	pev->flags |= FL_NOTARGET;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	UTIL_SetOrigin(pev, pev->origin);

	SetThink(&CKingpinBall::HuntThink);
	pev->nextthink = gpGlobals->time + 0.1;

	pev->dmgtime = gpGlobals->time;
	pev->framerate = 10.0f;
	m_lastThink = gpGlobals->time;
	m_powerlevel = 1.0f;
}

void CKingpinBall::Precache(void)
{
	CBaseMonster::Precache();
	orbSprite = PRECACHE_MODEL(ORB_SPRITE);
	orbFollowSprite = PRECACHE_MODEL(ORB_TRAIL_SPRITE);
	orbShockWaveSpriteIdx = PRECACHE_MODEL(SHOCK_WAVE_SPRITE);
	PRECACHE_SOUND(ORB_MOVE_SOUND);
	PRECACHE_SOUND(ORB_EXPLODE_SOUND);
	m_spriteFrames = MODEL_FRAMES(orbSprite);
}

void CKingpinBall::Activate() {
	SetTouch(&CKingpinBall::ExplodeTouch);
	UTIL_BeamFollow(entindex(), orbFollowSprite, 5, 10, RGBA(255, 255, 255), MSG_ALL);
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, ORB_MOVE_SOUND, 1.0f, ATTN_NORM, 0, 99);
	m_isActive = true;
	pev->scale = 1.3f;
}

void CKingpinBall::HuntThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1f;

	UTIL_ELight(entindex(), 0, pev->origin, 64 * m_powerlevel, RGBA(255, 255, 255), 2, 0);

	const float lifeTime = 8;
	float timeLeft = lifeTime - (gpGlobals->time - pev->dmgtime);

	if (timeLeft < 0)
	{
		SetTouch(NULL);
		SetThink(NULL);
		UTIL_Remove(this);
		return;
	}

	if (timeLeft < 2.0f) {
		m_powerlevel = timeLeft / 2.0f;
		float q = 1.0f - m_powerlevel;
		m_powerlevel = 1.0f - (q*q); // accelerate

		pev->renderamt = m_powerlevel * 255.0f;
		pev->scale = 0.5f + m_powerlevel * 0.8f;
		float vol = m_powerlevel + 0.2f;
		int pitch = m_powerlevel * 20 + 80;
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, ORB_MOVE_SOUND, vol, ATTN_NORM, SND_CHANGE_PITCH | SND_CHANGE_VOL, pitch);
	}

	float flInterval = gpGlobals->time - m_lastThink;
	m_lastThink = gpGlobals->time;

	pev->frame = normalizeRangef(pev->frame + pev->framerate*flInterval, 0, m_spriteFrames);

	if (!m_isActive) {
		pev->scale = V_min(1.3f, pev->scale + 0.05f);
		pev->nextthink = gpGlobals->time + 0.05f;
		m_animate = !m_animate;
		return;
	}

	if (!m_hEnemy) {
		Look(1024);
		m_hEnemy = BestVisibleEnemy();
	}

	if (m_hEnemy) {
		m_lastDir = (m_hEnemy->Center() - pev->origin).Normalize();
		MovetoTarget(m_hEnemy->Center());

		if (!m_hEnemy->IsAlive()) {
			m_hEnemy = NULL;
		}
	}
	else {
		MovetoTarget(pev->origin + m_lastDir * 512);
	}
}

void CKingpinBall::MovetoTarget(Vector vecTarget) {
	Vector idealVelocity = (vecTarget - pev->origin).Normalize() * 300 * m_powerlevel;
	pev->velocity = pev->velocity * 0.2f + idealVelocity * 0.8f;
}

void CKingpinBall::ExplodeTouch(CBaseEntity* pOther)
{
	float radius = (ORB_ATTACK_RADIUS + 50) / 0.3f;
	RGBA color = RGBA(170, 120, 160, 255);
	UTIL_BeamCylinder(pev->origin, radius, orbShockWaveSpriteIdx, 0, 0, 2, 12, 0, color, 0);
	UTIL_BeamDisk(pev->origin, radius, orbShockWaveSpriteIdx, 0, 0, 2, 12, 0, color, 0);
	UTIL_DLight(pev->origin, 40, RGB(170, 120, 160), 100, 64);
	UTIL_ELight(entindex(), 0, pev->origin, 128, RGBA(170, 120, 160), 2, 10, MSG_PVS, pev->origin);

	::RadiusDamage(pev->origin, pev, VARS(pev->owner), gSkillData.sk_kingpin_plasma_blast, ORB_ATTACK_RADIUS, CLASS_NONE, DMG_ALWAYSGIB | DMG_ENERGYBEAM);

	UTIL_EmitAmbientSound(edict(), pev->origin, ORB_EXPLODE_SOUND, 1.0, ATTN_NORM, 0, RANDOM_LONG(75, 80));

	UTIL_Remove(this);
}

int	CKingpinBall::Classify(void)
{
	return m_hOwner ? m_hOwner->Classify() : CBaseMonster::Classify(CLASS_NONE);
}

void CKingpinBall::UpdateOnRemove(void) {
	STOP_SOUND(edict(), CHAN_WEAPON, ORB_MOVE_SOUND);
	CBaseMonster::UpdateOnRemove();
}



LINK_ENTITY_TO_CLASS(monster_kingpin, CKingpin)

const char* CKingpin::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CKingpin::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CKingpin::pAttackSounds[] =
{
	"agrunt/ag_attack2.wav",
	"agrunt/ag_attack3.wav",
};

const char* CKingpin::pDieSounds[] =
{
	"agrunt/ag_die2.wav",
};

const char* CKingpin::pPainSounds[] =
{
	"agrunt/ag_pain1.wav",
	"agrunt/ag_pain2.wav",
	"agrunt/ag_pain3.wav",
};

const char* CKingpin::pIdleSounds[] =
{
	"agrunt/ag_die1.wav",
	"agrunt/ag_idle1.wav",
};

const char* CKingpin::pAlertSounds[] =
{
	"agrunt/ag_alert2.wav",
	"agrunt/ag_alert3.wav",
	"agrunt/ag_alert4.wav",
};

enum
{
	TASK_SET_TELEPORT_DEST = LAST_COMMON_TASK + 1,	// find a place nearby to teleport to
	TASK_TELEPORT,
};

Task_t	tlOrbAttack[] =
{
	{ TASK_STOP_MOVING,				0			},
	{ TASK_RANGE_ATTACK2,			(float)0	},
};

Schedule_t	slOrbAttack[] =
{
	{
		tlOrbAttack,
		ARRAYSIZE(tlOrbAttack),
		0,
		0,
		"KINGPIN_ORB_ATTACK"
	},
};

Task_t	tlTeleport[] =
{
	{ TASK_STOP_MOVING,				0			},
	{ TASK_SET_TELEPORT_DEST,		(float)0	},
	{ TASK_TELEPORT,				(float)0	},
};

Schedule_t	slTeleport[] =
{
	{
		tlTeleport,
		ARRAYSIZE(tlTeleport),
		0,
		0,
		"KINGPIN_TELEPORT"
	},
};

DEFINE_CUSTOM_SCHEDULES(CKingpin)
{
	slOrbAttack,
	slTeleport
};

IMPLEMENT_CUSTOM_SCHEDULES(CKingpin, CBaseMonster)

void CKingpin::Spawn()
{
	Precache();

	InitModel();
	SetSize(Vector(-24, -24, 0), Vector(24, 24, 72));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->view_ofs = Vector(0, 0, 0);// position of the eyes relative to monster's origin.
	m_flFieldOfView = VIEW_FIELD_FULL;
	m_MonsterState = MONSTERSTATE_NONE;

	MonsterInit();

	if (CBaseEntity::IRelationship(Classify(), CLASS_PLAYER) == R_AL) {
		m_beamColor = Vector(16, 255, 255);
	}
	else {
		m_beamColor = Vector(255, 0, 255);
	}

	for (int i = 0; i < 4; i++) {
		CSprite* spr = CSprite::SpriteCreate(KINGPIN_EYE_SPRITE, pev->origin, FALSE);
		spr->SetAttachment(edict(), i+1);
		spr->pev->rendermode = kRenderTransAdd;
		spr->pev->renderamt = 255;
		spr->pev->scale = 0.2f;
		spr->pev->framerate = 20.0f;
		spr->TurnOn();

		m_eyes[i].h_spr = spr;
		m_eyes[i].iAttachment = i;
	}

	m_eyes[0].angle = 0; // front
	m_eyes[1].angle = 180; // back
	m_eyes[2].angle = -90; // right
	m_eyes[3].angle = 90; //left

	m_numAttachments = GetAttachmentCount();
}

void CKingpin::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/kingpin.mdl";
	PRECACHE_MODEL(GetModel());
	kingBeamSpriteIdx = PRECACHE_MODEL(KINGPIN_BEAM_SPRITE);
	m_iSpitSprite = PRECACHE_MODEL("sprites/tinyspit.spr");
	PRECACHE_MODEL(KINGPIN_TELE_SPRITE);
	PRECACHE_MODEL(KINGPIN_FLARE_SPRITE);
	PRECACHE_MODEL(KINGPIN_EYE_SPRITE);

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDieSounds);

	PRECACHE_SOUND(KINGPIN_BEAM_SOUND);
	PRECACHE_SOUND(KINGPIN_TELE_SOUND_IN);
	PRECACHE_SOUND(KINGPIN_TELE_SOUND_OUT);
	PRECACHE_SOUND(ORB_GROW_SOUND);

	UTIL_PrecacheOther("kingpin_plasma_ball");
}

int	CKingpin::Classify(void)
{
	return CBaseMonster::Classify(CLASS_ALIEN_MONSTER);
}

const char* CKingpin::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Kingpin";
}

const char* CKingpin::GetTaskName(int taskIdx) {
	switch (taskIdx) {
	case TASK_SET_TELEPORT_DEST: return "TASK_SET_TELEPORT_DEST";
	case TASK_TELEPORT: return "TASK_TELEPORT";
	default:
		return CBaseMonster::GetTaskName(taskIdx);
	}
}

void CKingpin::SetYawSpeed(void)
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
	default:
		ys = 100;
	}

	pev->yaw_speed = ys * gSkillData.sk_yawspeed_mult;
}

void CKingpin::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case KINGPIN_AE_CONJURE_ORB: {
		if (!m_orb) {
			Vector orbPosition = pev->origin + Vector(0, 0, 64);
			m_orb = Create("kingpin_plasma_ball", orbPosition, g_vecZero, true, edict());
			m_orb->pev->scale = 0.1f;
			pev->framerate = 0.3f; // slow down to let the orb grow
			EMIT_SOUND_DYN(m_orb->edict(), CHAN_WEAPON, ORB_GROW_SOUND, 1.0, ATTN_NORM, 0, RANDOM_LONG(95, 105));
		}
		break;
	}
	case KINGPIN_AE_SHOOT_ORB: {
		if (m_orb) {
			MAKE_VECTORS(pev->angles);
			CKingpinBall* orb = (CKingpinBall*)m_orb.GetEntity();
			orb->m_lastDir = gpGlobals->v_forward;
			orb->Activate();
			m_orb = NULL; // allow conjuring another orb
		}

		m_nextOrb = gpGlobals->time + 8;
		pev->framerate = 1.0f;
		break;
	}
	case KINGPIN_AE_SWING_LEFT:
	case KINGPIN_AE_SWING_RIGHT: {
		bool isLeftSwing = pEvent->event == KINGPIN_AE_SWING_LEFT;
		float damage = gSkillData.sk_kingpin_melee;
		CBaseEntity* pHurt = CheckTraceHullAttack(KINGPIN_MELEE_DISTANCE, damage, DMG_SLASH);

		if (pHurt) {
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT)) {
				pHurt->pev->punchangle.x = 5;
				pHurt->pev->punchangle.z = isLeftSwing ? -18 : 18;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * (isLeftSwing ? -100 : 100);
			}

			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		break;
	}
	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

Schedule_t* CKingpin::GetSchedule(void)
{
	if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
	{
		// never flinch or retreat from damage
		ClearConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE);
	}

	int nearbyEnemies = 0;

	CBaseEntity* pOther = NULL;
	while ((pOther = UTIL_FindEntityInSphere(pOther, pev->origin, 256)) != NULL) {
		if ((pOther->IsNormalMonster() || pOther->IsPlayer()) && pOther->IsAlive() 
			&& IRelationship(pOther) > R_NO && !FBitSet(pOther->pev->flags, FL_NOTARGET))
		{
			nearbyEnemies++;
		}
	}

	if (m_nextTele < gpGlobals->time && (nearbyEnemies > 1 || HasConditions(bits_COND_ENEMY_OCCLUDED))) {
		return slTeleport;
	}

	return CBaseMonster::GetSchedule();
}

Schedule_t* CKingpin::GetScheduleOfType(int Type) {
	if (Type == SCHED_MELEE_ATTACK2) {
		AttackSound();
	}

	switch (Type) {
	case SCHED_CHASE_ENEMY_FAILED:
		if (m_nextOrb < gpGlobals->time && HasConditions(bits_COND_SEE_ENEMY)) {
			return slOrbAttack;
		}
		else if (m_nextTele < gpGlobals->time) {
			return slTeleport;
		}
		else {
			return CBaseMonster::GetScheduleOfType(Type);
		}
	case SCHED_MELEE_ATTACK1:
		AttackSound();
		return CBaseMonster::GetScheduleOfType(Type);
	case SCHED_RANGE_ATTACK2:
		return slOrbAttack;
	default:
		return CBaseMonster::GetScheduleOfType(Type);
	}
}

void CKingpin::CancelOrb() {
	if (m_orb && !((CKingpinBall*)m_orb.GetEntity())->m_isActive) {
		UTIL_Remove(m_orb);
	}
}

void CKingpin::ScheduleChange(void) {
	CancelOrb();
	m_teleportPhase = 0;
}

void CKingpin::UpdateOnRemove(void) {
	CancelOrb();

	for (int i = 0; i < 4; i++) {
		UTIL_Remove(m_eyes[i].h_spr);
		UTIL_Remove(m_eyes[i].h_flare);
	}

	CBaseMonster::UpdateOnRemove();
}

void CKingpin::LaserEyesThink() {
	std::vector<CBaseEntity*> potentialEyeTargets;

	CBaseEntity* pOther = NULL;
	while ((pOther = UTIL_FindEntityInSphere(pOther, pev->origin, 2048)) != NULL) {
		if ((pOther->IsNormalMonster() || pOther->IsPlayer()) && pOther->IsAlive()
			&& IRelationship(pOther) > R_NO && !FBitSet(pOther->pev->flags, FL_NOTARGET))
		{
			potentialEyeTargets.push_back(pOther);
		}
	}

	bool didBeam = false;

	for (int i = 0; i < 4 && !didBeam; i++) {
		kingpin_eye_t& eye = m_eyes[i];

		Vector attachOri, attachAngles;

		if (eye.iAttachment < m_numAttachments) {
			GetAttachment(eye.iAttachment, attachOri, attachAngles);
		}
		else {
			attachOri = Center();
		}
		
		MAKE_VECTORS(Vector(0, pev->angles.y + eye.angle, 0));
		Vector eyeDir = gpGlobals->v_forward;

		//te_debug_beam(attachOri, attachOri + eyeDir * 64, 1, RGBA(0, 255, 0));

		const float rechargeTime = 4.0f;
		float timeLeft = rechargeTime - (gpGlobals->time - eye.lastAttack);

		CBaseEntity* spr = eye.h_spr;
		if (!spr) {
			continue;
		}

		if (timeLeft < 0) {
			// can attack now, search for targets

			TraceResult tr;

			bool canShoot = (gpGlobals->time - m_teleportTime > 1.0f) && m_teleportPhase == 0;

			for (int k = 0; k < (int)potentialEyeTargets.size() && canShoot; k++) {

				CBaseEntity* target = potentialEyeTargets[k];

				UTIL_TraceLine(attachOri, target->Center(), dont_ignore_monsters, edict(), &tr);
				CBaseEntity* phit = CBaseEntity::Instance(tr.pHit);

				if (phit != target) {
					continue;
				}

				Vector targetDir = target->Center() - attachOri;

				if (DotProduct(eyeDir, targetDir) < 0) {
					continue; // not in FOV of eye
				}

				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, KINGPIN_BEAM_SOUND, 1.0, ATTN_NORM, 0, RANDOM_LONG(95, 105));
				CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_GUN_VOLUME, 0.3);

				RGBA color = RGBA(m_beamColor.x, m_beamColor.y, m_beamColor.z, 150);
				UTIL_BeamEntPoint(entindex(), eye.iAttachment + 1, target->Center(), kingBeamSpriteIdx,
					0, 0, 2, 50, 32, color, 0, MSG_PVS, pev->origin);

				CSprite* flr = CSprite::SpriteCreate(KINGPIN_FLARE_SPRITE, pev->origin, FALSE);
				flr->SetAttachment(edict(), eye.iAttachment + 1);
				flr->pev->rendermode = kRenderTransAdd;
				flr->pev->renderamt = 255;
				flr->pev->scale = 0.8f;
				eye.h_flare = flr;

				phit->TakeDamage(pev, pev, gSkillData.sk_kingpin_lightning, DMG_ENERGYBEAM);

				eye.lastAttack = gpGlobals->time;
				didBeam = true;
				break;
			}

			spr->pev->renderamt = 255;
			spr->pev->scale = 0.2f;
		}
		else {
			float powerLevel = (rechargeTime - timeLeft) / rechargeTime;
			powerLevel = powerLevel * powerLevel * powerLevel * powerLevel; // ease in
			spr->pev->renderamt = powerLevel * 128.0f + 128.0f;
			spr->pev->scale = powerLevel * 0.15f + 0.05f;
			spr->pev->renderfx = 0;

			if (eye.h_flare) {
				eye.h_flare->pev->scale *= 0.5f;

				if (eye.h_flare->pev->scale < 0.02f) {
					UTIL_Remove(eye.h_flare);
				}
			}
		}
	}
}

void CKingpin::ProjectileDeflectThink() {
	CBaseEntity* pOther = NULL;
	while ((pOther = UTIL_FindEntityInSphere(pOther, pev->origin, 300)) != NULL) {\
		if ((pOther->pev->effects & EF_NODRAW) || pOther->pev->solid == SOLID_NOT
			|| pOther->pev->movetype != MOVETYPE_BOUNCE || !pOther->pev->owner) {
			continue;
		}
		
		CBaseEntity* owner = Instance(pOther->pev->owner);
		if (owner && owner != this && owner->IsMonster() && IRelationship(owner) != R_AL) {
			Vector awayDir = (pOther->pev->origin - pev->origin).Normalize();

			if (DotProduct(awayDir, pOther->pev->velocity.Normalize()) < -0.2f) {
				// move perpindicular to self if projectile is coming straight towards me
				MAKE_VECTORS(UTIL_VecToAngles(pOther->pev->velocity));

				if (DotProduct(awayDir, gpGlobals->v_right) > DotProduct(awayDir, gpGlobals->v_right*-1)) {
					awayDir = gpGlobals->v_right;
				}
				else {
					awayDir = gpGlobals->v_right*-1;
				}
			}

			pOther->pev->velocity = pOther->pev->velocity + awayDir * 300;
			pOther->pev->angles = UTIL_VecToAngles(pOther->pev->velocity.Normalize());

			UTIL_BeamEnts(entindex(), 0, pOther->entindex(), 0, false, kingBeamSpriteIdx, 0, 0, 1, 16, 32,
				RGBA(255, 255, 255), 0);
		}
	}
}

void CKingpin::MonsterThink(void) {	
	if (IsAlive()) {
		LaserEyesThink();
		ProjectileDeflectThink();
	}
	
	CBaseMonster::MonsterThink();
}

void CKingpin::GibMonster() {
	CBaseMonster::GibMonster();

	float height = pev->maxs.z * 2;
	Vector ori = pev->origin;
	Vector dir = Vector(0, 0, 1);

	for (int i = 0; i < 2; i++) {
		Vector sprayOri = ori + Vector(0, 0, 40 + 48 * i);
		UTIL_SpriteSpray(sprayOri, dir, m_iSpitSprite, 20, 100 - 50 * i, 40 + 40 * i);
	}

	for (int i = 0; i < 4; i++) {
		SpawnBlood(ori + Vector(0, 0, 32 + height * 0.1f + height * 0.2f * i), BloodColor(), 255);
	}
}

void CKingpin::StartTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_DIE:
		CBaseMonster::StartTask(pTask);

		pev->renderfx = kRenderFxGlowShell;
		pev->renderamt = 1;
		pev->rendercolor = m_beamColor * 0.5f;

		for (int i = 0; i < 4; i++) {
			UTIL_Remove(m_eyes[i].h_flare);
		}
		break;
	case TASK_TELEPORT:
		ClearShockEffect();
		m_iOldRenderFX = pev->renderfx;
		m_iOldRenderMode = pev->rendermode;
		m_OldRenderColor = pev->rendercolor;
		m_flOldRenderAmt = pev->renderamt;

		pev->renderfx = kRenderFxGlowShell;
		pev->renderamt = 230;
		pev->rendermode = kRenderTransTexture;
		pev->rendercolor = Vector(0, 128, 64);
		pev->solid = SOLID_NOT;
		pev->movetype = MOVETYPE_NOCLIP;
		pev->velocity = Vector(0, 0, 0);
		pev->takedamage = DAMAGE_NO;

		m_teleportTime = gpGlobals->time;
		m_teleportPhase = 1;

		SetActivity(ACT_DIESIMPLE);

		UTIL_EmitAmbientSound(edict(), m_teleportDst, KINGPIN_TELE_SOUND_IN, 1.0f, ATTN_NORM, 0, 100);

		break;
	case TASK_SET_TELEPORT_DEST:
	{
		const float teleUpwardOffset = 64;
		float bestDist = 0;
		Vector startPos = pev->origin + Vector(0, 0, teleUpwardOffset);
		Vector bestPos = g_vecZero;
		bool foundPos = false;
		float step = 20;
		float angle = RANDOM_FLOAT(0, 360);
		float vertAngles[2] = { -40, 0 };
		TraceResult tr;

		TRACE_MONSTER_HULL(edict(), pev->origin, startPos, ignore_monsters, NULL, &tr);
		if (tr.flFraction < 1.0f) {
			TaskFail();
			break;
		}

		for (int k = 0; k < 2 && !foundPos; k++) {
			for (int i = 0; i < 360.0f / step; i++) {
				angle += step;
				Vector angles = Vector(vertAngles[k], angle, 0);
				MAKE_VECTORS(angles);

				TRACE_MONSTER_HULL(edict(), startPos, startPos + gpGlobals->v_forward * 1024, ignore_monsters, NULL, &tr);

				float dist = (tr.vecEndPos - startPos).Length();

				if (dist < 256) {
					//te_debug_beam(startPos, tr.vecEndPos, 10, RGBA(255, 0, 0));
					continue; // too close to current position
				}

				// drop to the floor
				Vector dropFromPos = tr.vecEndPos;
				TRACE_MONSTER_HULL(edict(), dropFromPos, dropFromPos - Vector(0, 0, 1024), ignore_monsters, NULL, &tr);

				// move upward a bit
				float dropDist = (tr.vecEndPos - dropFromPos).Length();
				Vector telePos = tr.vecEndPos + Vector(0, 0, V_min(teleUpwardOffset, dropDist));

				if (POINT_CONTENTS(telePos) != CONTENTS_EMPTY && foundPos) {
					continue; // try to stay out of water/lava
				}

				if (dist > bestDist) {
					bestDist = dist;
					bestPos = telePos;
					foundPos = true;
				}
				//te_debug_beam(pev->origin, telePos, 10, RGBA(255, 255, 0));
			}
		}

		if (foundPos) {
			//te_debug_beam(startPos, bestPos, 10, RGBA(0, 255, 0));
			m_teleportSrc = startPos;
			m_teleportDst = bestPos;
			TaskComplete();
		}
		else {
			TaskFail();
		}
		
		break;
	}
	default:
		CBaseMonster::StartTask(pTask);
		break;
	}
}

void CKingpin::RunTask(Task_t* pTask)
{
	const Vector teleEffectOffset = Vector(0, 0, 40);

	switch (pTask->iTask)
	{
	case TASK_DIE: {
		CBaseMonster::RunTask(pTask);
		pev->framerate = 1.5f;

		RGBA color = RGBA(m_beamColor.x, m_beamColor.y, m_beamColor.z, 150);

		for (int i = 0; i < 4; i++) {
			kingpin_eye_t& eye = m_eyes[i];
			Vector attachOri, attachAngles;
			GetAttachment(eye.iAttachment, attachOri, attachAngles);

			CBaseEntity* spr = eye.h_spr;
			if (spr) {
				spr->pev->scale = V_max(0.2f, spr->pev->scale + 0.03f);
				spr->pev->renderamt = 255;
			}

			float x, y;
			GetCircularGaussianSpread(x, y);

			TraceResult tr;
			UTIL_TraceLine(attachOri, attachOri - Vector(x*256, y*256, 256), ignore_monsters, NULL, &tr);

			UTIL_BeamEntPoint(entindex(), eye.iAttachment + 1, tr.vecEndPos, kingBeamSpriteIdx, 0, 0, 2, 20, 128,
				color, 0, MSG_PVS, pev->origin);

		}

		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, KINGPIN_BEAM_SOUND, 0.5, ATTN_NORM, 0, RANDOM_LONG(80, 120));

		if (pev->frame > 200) {
			pev->renderfx = kRenderFxExplode;
			SetThink(&CKingpin::GibMonster);
			pev->nextthink = gpGlobals->time + 0.15f;
		}

		break;
	}
	case TASK_TELEPORT:
		pev->renderamt = V_max(1, pev->renderamt - 10);

		if (m_teleportPhase == 1 && gpGlobals->time - m_teleportTime > 1.0f) {
			CSprite* spr;
			
			spr = CSprite::SpriteCreate(KINGPIN_TELE_SPRITE, m_teleportSrc + teleEffectOffset, FALSE);
			spr->pev->rendermode = kRenderTransAdd;
			spr->pev->renderamt = 255;
			spr->pev->scale = 1.5f;
			spr->AnimateAndDie(15.0f);

			spr = CSprite::SpriteCreate(KINGPIN_TELE_SPRITE, m_teleportDst + teleEffectOffset, FALSE);
			spr->pev->rendermode = kRenderTransAdd;
			spr->pev->renderamt = 255;
			spr->pev->scale = 1.5f;
			spr->AnimateAndDie(15.0f);

			UTIL_EmitAmbientSound(spr->edict(), m_teleportSrc, KINGPIN_TELE_SOUND_OUT, 1.0f, ATTN_NORM, 0, 100);

			m_teleportPhase = 2;
		}
		if (gpGlobals->time - m_teleportTime > 2.0f) {
			pev->renderfx = m_iOldRenderFX;
			pev->renderamt = m_flOldRenderAmt;
			pev->rendermode = m_iOldRenderMode;
			pev->rendercolor = m_OldRenderColor;
			pev->solid = SOLID_SLIDEBOX;
			pev->movetype = MOVETYPE_STEP;
			pev->takedamage = DAMAGE_YES;

			m_nextTele = gpGlobals->time + 5.0f;

			UTIL_BeamPoints(m_teleportSrc + teleEffectOffset, m_teleportDst + teleEffectOffset,
				kingBeamSpriteIdx, 0, 0, 2, 80, 64, RGBA(255, 255, 255, 150), 0, MSG_PVS, pev->origin);

			UTIL_SetOrigin(pev, m_teleportDst);

			CBaseEntity* pList[32];
			int count = UTIL_EntitiesInBox(pList, 32, pev->origin + pev->mins, pev->origin + pev->maxs, FL_MONSTER | FL_CLIENT, true, true);
			for (int i = 0; i < count; i++) {
				if (pList[i] == this) {
					continue;
				}

				pList[i]->TakeDamage(pev, pev, gSkillData.sk_kingpin_telefrag, DMG_ALWAYSGIB | DMG_ENERGYBEAM);
			}

			SetActivity(ACT_IDLE);

			m_teleportTime = gpGlobals->time; // for cooling down other actions
			m_teleportPhase = 0;

			TaskComplete();
		}
		
		break;
	default:
		CBaseMonster::RunTask(pTask);
		break;
	}
}

BOOL CKingpin::CheckRangeAttack1(float flDot, float flDist)
{
	return FALSE;
}

BOOL CKingpin::CheckRangeAttack2(float flDot, float flDist)
{
	if (m_nextOrb < gpGlobals->time && flDist > ORB_ATTACK_RADIUS + 128) {
		return TRUE;
	}
	return FALSE;
}

BOOL CKingpin::CheckMeleeAttack1(float flDot, float flDist)
{
	if (flDist <= KINGPIN_MELEE_DISTANCE) {
		return TRUE;
	}

	return FALSE;
}

BOOL CKingpin::CheckMeleeAttack2(float flDot, float flDist)
{
	return FALSE;
}

void CKingpin::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	// this thing dies too easily if taking headshot damage on its giant head
	if (ptr->iHitgroup == HITGROUP_HEAD) {
		ptr->iHitgroup = HITGROUP_CHEST;
	}

	CBaseMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

void CKingpin::PainSound(void)
{
	int pitch = RANDOM_LONG(kingpinPitch, kingpinPitch+10);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CKingpin::DeathSound(void)
{
	int pitch = RANDOM_LONG(kingpinPitch, kingpinPitch + 10);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDieSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CKingpin::AlertSound(void)
{
	int pitch = RANDOM_LONG(kingpinPitch, kingpinPitch + 10);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CKingpin::IdleSound(void)
{
	int pitch = RANDOM_LONG(kingpinPitch, kingpinPitch + 10);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CKingpin::AttackSound(void) {
	int pitch = RANDOM_LONG(150, 160);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CKingpin::StartFollowingSound() {
	int pitch = RANDOM_LONG(kingpinPitch, kingpinPitch + 10);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CKingpin::StopFollowingSound() {
	int pitch = RANDOM_LONG(kingpinPitch, kingpinPitch + 10);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CKingpin::CantFollowSound() {
	int pitch = RANDOM_LONG(kingpinPitch, kingpinPitch + 10);
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAttackSounds), 1.0, ATTN_NORM, 0, pitch);
}
