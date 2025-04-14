#include "extdll.h"
#include "util.h"
#include "effects.h"
#include "weapons.h"
#include "explode.h"
#include "CBasePlayer.h"
#include "CFuncTank.h"
#include "CBasePlayerItem.h"
#include "monsters.h"


TYPEDESCRIPTION	CFuncTank::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTank, m_yawCenter, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_yawRate, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_yawRange, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_yawTolerance, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_pitchCenter, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_pitchRate, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_pitchRange, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_pitchTolerance, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_fireLast, FIELD_TIME),
	DEFINE_FIELD(CFuncTank, m_fireRate, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_lastSightTime, FIELD_TIME),
	DEFINE_FIELD(CFuncTank, m_persist, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_minRange, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_maxRange, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_barrelPos, FIELD_VECTOR),
	DEFINE_FIELD(CFuncTank, m_spriteScale, FIELD_FLOAT),
	DEFINE_FIELD(CFuncTank, m_iszSpriteSmoke, FIELD_STRING),
	DEFINE_FIELD(CFuncTank, m_iszSpriteFlash, FIELD_STRING),
	DEFINE_FIELD(CFuncTank, m_bulletType, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTank, m_sightOrigin, FIELD_VECTOR),
	DEFINE_FIELD(CFuncTank, m_spread, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTank, m_hController, FIELD_EHANDLE),
	DEFINE_FIELD(CFuncTank, m_vecControllerUsePos, FIELD_VECTOR),
	DEFINE_FIELD(CFuncTank, m_flNextAttack, FIELD_TIME),
	DEFINE_FIELD(CFuncTank, m_iBulletDamage, FIELD_INTEGER),
	DEFINE_FIELD(CFuncTank, m_iszMaster, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CFuncTank, CBaseEntity)

Vector gTankSpread[] =
{
	Vector(0, 0, 0),		// perfect
	Vector(0.025, 0.025, 0.025),	// small cone
	Vector(0.05, 0.05, 0.05),  // medium cone
	Vector(0.1, 0.1, 0.1),	// large cone
	Vector(0.25, 0.25, 0.25),	// extra-large cone
};
#define MAX_FIRING_SPREADS ARRAYSIZE(gTankSpread)


void CFuncTank::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	Precache();

	pev->movetype = MOVETYPE_PUSH;  // so it doesn't get pushed by anything
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	m_yawCenter = pev->angles.y;
	m_pitchCenter = pev->angles.x;

	if (IsActive())
		pev->nextthink = pev->ltime + 1.0;

	m_sightOrigin = BarrelPosition(true); // Point at the end of the barrel

	if (m_fireRate <= 0)
		m_fireRate = 1;
	if (m_spread > (int)MAX_FIRING_SPREADS)
		m_spread = 0;

	pev->oldorigin = pev->origin;
}


void CFuncTank::Precache(void)
{
	if (m_iszSpriteSmoke)
		PRECACHE_MODEL((char*)STRING(m_iszSpriteSmoke));
	if (m_iszSpriteFlash)
		PRECACHE_MODEL((char*)STRING(m_iszSpriteFlash));

	if (pev->noise)
		PRECACHE_SOUND((char*)STRING(pev->noise));
}


void CFuncTank::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "yawrate"))
	{
		m_yawRate = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "yawrange"))
	{
		m_yawRange = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "yawtolerance"))
	{
		m_yawTolerance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitchrange"))
	{
		m_pitchRange = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitchrate"))
	{
		m_pitchRate = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitchtolerance"))
	{
		m_pitchTolerance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "firerate"))
	{
		m_fireRate = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "barrel"))
	{
		m_barrelPos.x = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "barrely"))
	{
		m_barrelPos.y = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "barrelz"))
	{
		m_barrelPos.z = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spritescale"))
	{
		m_spriteScale = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spritesmoke"))
	{
		m_iszSpriteSmoke = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spriteflash"))
	{
		m_iszSpriteFlash = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "rotatesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "persistence"))
	{
		m_persist = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bullet"))
	{
		m_bulletType = (TANKBULLET)atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bullet_damage"))
	{
		m_iBulletDamage = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "firespread"))
	{
		m_spread = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "minRange"))
	{
		m_minRange = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "maxRange"))
	{
		m_maxRange = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_iszMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_monster_bioweapon"))
	{
		m_iRelation[CLASS_ALIEN_BIOWEAPON] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_player_bioweapon"))
	{
		m_iRelation[CLASS_PLAYER_BIOWEAPON] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_player_ally"))
	{
		m_iRelation[CLASS_PLAYER_ALLY] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_insect"))
	{
		m_iRelation[CLASS_INSECT] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_alien_predator"))
	{
		m_iRelation[CLASS_ALIEN_PREDATOR] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_alien_prey"))
	{
		m_iRelation[CLASS_ALIEN_PREY] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_alien_monster"))
	{
		m_iRelation[CLASS_ALIEN_MONSTER] = atoi(pkvd->szValue);

		// TODO: add keyvalues for these?
		m_iRelation[CLASS_ALIEN_RACE_X] = m_iRelation[CLASS_ALIEN_MONSTER];
		m_iRelation[CLASS_ALIEN_RACE_X_PITDRONE] = m_iRelation[CLASS_ALIEN_MONSTER];
		
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_alien_passive"))
	{
		m_iRelation[CLASS_ALIEN_PASSIVE] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_alien_militar")) // TODO: typo in sven fgd?
	{
		m_iRelation[CLASS_ALIEN_MILITARY] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_human_militar")) // TODO: typo in sven fgd?
	{
		m_iRelation[CLASS_HUMAN_MILITARY] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_human_passive"))
	{
		m_iRelation[CLASS_HUMAN_PASSIVE] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_machine"))
	{
		m_iRelation[CLASS_MACHINE] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_none"))
	{
		m_iRelation[CLASS_NONE] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "relation_player"))
	{
		m_iRelation[CLASS_PLAYER] = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

////////////// START NEW STUFF //////////////

//==================================================================================
// TANK CONTROLLING
BOOL CFuncTank::OnControls(entvars_t* pevTest)
{
	if (!(pev->spawnflags & SF_TANK_CANCONTROL))
		return FALSE;

	if ((m_vecControllerUsePos - pevTest->origin).Length() < 30)
		return TRUE;

	return FALSE;
}

BOOL CFuncTank::StartControl(CBasePlayer* pController)
{
	CBasePlayer* m_pController = (CBasePlayer*)m_hController.GetEntity();

	if (m_pController != NULL)
		return FALSE;

	// Team only or disabled?
	if (m_iszMaster)
	{
		if (!UTIL_IsMasterTriggered(m_iszMaster, pController))
			return FALSE;
	}

	ALERT(at_console, "using TANK!\n");

	m_hController = pController;
	if (pController->m_pActiveItem)
	{
		CBasePlayerItem* item = (CBasePlayerItem*)pController->m_pActiveItem.GetEntity();
		item->Holster();
		pController->pev->weaponmodel = 0;
		pController->pev->viewmodel = 0;

	}

	pController->m_iHideHUD |= HIDEHUD_WEAPONS;
	m_vecControllerUsePos = pController->pev->origin;

	pev->nextthink = pev->ltime + 0.1;

	return TRUE;
}

void CFuncTank::StopControl()
{
	CBasePlayer* m_pController = (CBasePlayer*)m_hController.GetEntity();

	// TODO: bring back the controllers current weapon
	if (!m_pController)
		return;

	if (m_pController->m_pActiveItem)
		((CBasePlayerItem*)m_pController->m_pActiveItem.GetEntity())->Deploy();

	ALERT(at_console, "stopped using TANK\n");

	m_pController->m_iHideHUD &= ~HIDEHUD_WEAPONS;

	pev->nextthink = 0;
	m_hController = NULL;

	if (IsActive())
		pev->nextthink = pev->ltime + 1.0;
}

// Called each frame by the player's ItemPostFrame
void CFuncTank::ControllerPostFrame(void)
{
	CBasePlayer* m_pController = (CBasePlayer*)m_hController.GetEntity();

	if (!m_pController || gpGlobals->time < m_flNextAttack)
		return;

	if (m_pController->pev->button & IN_ATTACK)
	{
		Vector vecForward;
		UTIL_MakeVectorsPrivate(pev->angles, vecForward, NULL, NULL);

		m_fireLast = gpGlobals->time - (1 / m_fireRate) - 0.01;  // to make sure the gun doesn't fire too many bullets

		Fire(BarrelPosition(true), vecForward, m_pController->pev);

		// HACKHACK -- make some noise (that the AI can hear)
		if (m_pController && m_pController->IsPlayer())
			((CBasePlayer*)m_pController)->m_iWeaponVolume = LOUD_GUN_VOLUME;

		m_flNextAttack = gpGlobals->time + (1 / m_fireRate);
	}
}
////////////// END NEW STUFF //////////////


void CFuncTank::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBasePlayer* m_pController = (CBasePlayer*)m_hController.GetEntity();

	if (pev->spawnflags & SF_TANK_CANCONTROL)
	{  // player controlled turret

		if (pActivator->Classify() != CLASS_PLAYER)
			return;

		if (value == 2 && useType == USE_SET)
		{
			ControllerPostFrame();
		}
		else if (!m_pController && useType != USE_OFF)
		{
			((CBasePlayer*)pActivator)->m_pTank = this;
			StartControl((CBasePlayer*)pActivator);
		}
		else
		{
			StopControl();
		}
	}
	else
	{
		if (!ShouldToggle(useType, IsActive()))
			return;

		if (IsActive())
			TankDeactivate();
		else
			TankActivate();
	}
}

int CFuncTank::IRelationship(CBaseEntity* pTarget) {
	if (pev->spawnflags & SF_TANK_USE_RELATIONS) {
		int clazz = pTarget->Classify();
		
		if (clazz < 0 || clazz >= 16) {
			return R_NO;
		}

		return m_iRelation[clazz];
	}
	
	return pTarget->IsPlayer() ? R_DL : R_NO;
}

CBaseEntity* CFuncTank::FindTarget(Vector forward)
{
	TraceResult tr;

	// target entity that is most aligned with the tank aim direction
	CBaseEntity* bestTarget = NULL;
	float bestDot = -1.0f;

	const int iDistance = m_maxRange ? m_maxRange : 8192;
	Vector delta = Vector(iDistance, iDistance, iDistance);
	CBaseEntity* pList[100];
	CBaseEntity* pSightEnt = NULL;// the current visible entity that we're dealing with

	Vector barrelBase = BarrelPosition(false);

	// Find only monsters/clients in box, NOT limited to PVS
	int count = UTIL_EntitiesInBox(pList, 100, pev->origin - delta, pev->origin + delta, FL_CLIENT | FL_MONSTER | FL_POSSIBLE_TARGET, true);
	
	for (int i = 0; i < count; i++)
	{
		pSightEnt = pList[i];

		if ((pSightEnt->pev->flags & FL_NOTARGET) || pSightEnt == this) {
			continue;
		}
		if (pSightEnt->pev->spawnflags & SF_MONSTER_PRISONER) {
			continue;
		}
		if (IRelationship(pSightEnt) <= R_NO) {
			continue;
		}

		Vector targetPosition = pSightEnt->IsPlayer() ? pSightEnt->EyePosition() : pSightEnt->Center();

		if (!InRange((targetPosition - barrelBase).Length())) {
			continue;
		}

		Vector dir = (targetPosition - pev->origin).Normalize();
		float dot = DotProduct(forward, dir);

		UTIL_TraceLine(barrelBase, targetPosition, dont_ignore_monsters, edict(), &tr);
		bool hasLineOfSight = tr.flFraction >= 1.0f || tr.pHit == pSightEnt->edict();

		if (hasLineOfSight && dot > bestDot) {
			bestTarget = pSightEnt;
			bestDot = dot;
		}
	}

	return bestTarget;
}


BOOL CFuncTank::InRange(float range)
{
	if (range < m_minRange)
		return FALSE;
	if (m_maxRange > 0 && range > m_maxRange)
		return FALSE;

	return TRUE;
}

void CFuncTank::Think(void)
{
	pev->avelocity = g_vecZero;
	TrackTarget();

	if (fabs(pev->avelocity.x) > 1 || fabs(pev->avelocity.y) > 1)
		StartRotSound();
	else
		StopRotSound();
}

void CFuncTank::TrackTarget(void)
{
	TraceResult tr;
	BOOL updateTime = FALSE;
	Vector angles, direction, targetPosition;
	Vector barrelEnd = BarrelPosition(true);
	CBasePlayer* m_pController = (CBasePlayer*)m_hController.GetEntity();
	CBaseEntity* pTarget = NULL;

	Vector forward, right, up;
	UTIL_MakeVectorsPrivate(pev->angles, forward, right, up);

	// Get a position to aim for
	if (m_pController)
	{
		// Tanks attempt to mirror the player's angles
		angles = m_pController->pev->v_angle;
		angles[0] = 0 - angles[0];
		pev->nextthink = pev->ltime + 0.05;
	}
	else
	{
		if (IsActive())
			pev->nextthink = pev->ltime + 0.1;
		else
			return;

		pTarget = FindTarget(forward);

		if (!pTarget)
		{
			if (IsActive())
				pev->nextthink = pev->ltime + 0.5f;	// idle
			m_fireLast = 0;
			return;
		}

		// Calculate angle needed to aim at target
		updateTime = TRUE;
		m_sightOrigin = UpdateTargetPosition(pTarget);

		// Track sight origin

// !!! I'm not sure what i changed
		direction = m_sightOrigin - pev->origin;
		//		direction = m_sightOrigin - barrelEnd;
		angles = UTIL_VecToAngles(direction);

		// Calculate the additional rotation to point the end of the barrel at the target (not the gun's center) 
		AdjustAnglesForBarrel(angles, direction.Length());
	}

	angles.x = -angles.x;

	// Force the angles to be relative to the center position
	angles.y = m_yawCenter + UTIL_AngleDistance(angles.y, m_yawCenter);
	angles.x = m_pitchCenter + UTIL_AngleDistance(angles.x, m_pitchCenter);

	// Limit against range in y
	if (angles.y > m_yawCenter + m_yawRange)
	{
		angles.y = m_yawCenter + m_yawRange;
		updateTime = FALSE;	// Don't update if you saw the player, but out of range
	}
	else if (angles.y < (m_yawCenter - m_yawRange))
	{
		angles.y = (m_yawCenter - m_yawRange);
		updateTime = FALSE; // Don't update if you saw the player, but out of range
	}

	if (updateTime)
		m_lastSightTime = gpGlobals->time;

	// Move toward target at rate or less
	float distY = UTIL_AngleDistance(angles.y, pev->angles.y);
	pev->avelocity.y = distY * 10;
	if (pev->avelocity.y > m_yawRate)
		pev->avelocity.y = m_yawRate;
	else if (pev->avelocity.y < -m_yawRate)
		pev->avelocity.y = -m_yawRate;

	// Limit against range in x
	if (angles.x > m_pitchCenter + m_pitchRange)
		angles.x = m_pitchCenter + m_pitchRange;
	else if (angles.x < m_pitchCenter - m_pitchRange)
		angles.x = m_pitchCenter - m_pitchRange;

	// Move toward target at rate or less
	float distX = UTIL_AngleDistance(angles.x, pev->angles.x);
	pev->avelocity.x = distX * 10;

	if (pev->avelocity.x > m_pitchRate)
		pev->avelocity.x = m_pitchRate;
	else if (pev->avelocity.x < -m_pitchRate)
		pev->avelocity.x = -m_pitchRate;

	if (m_pController)
		return;

	if (CanFire() && ((fabs(distX) < m_pitchTolerance && fabs(distY) < m_yawTolerance) || (pev->spawnflags & SF_TANK_LINEOFSIGHT)))
	{
		BOOL fire = FALSE;
		UTIL_MakeVectorsPrivate(pev->angles, forward, NULL, NULL);

		if (pev->spawnflags & SF_TANK_LINEOFSIGHT)
		{
			float length = direction.Length();
			UTIL_TraceLine(barrelEnd, barrelEnd + forward * length, dont_ignore_monsters, edict(), &tr);
			if (pTarget && tr.pHit == pTarget->edict())
				fire = TRUE;
		}
		else
			fire = TRUE;

		if (fire)
		{
			Fire(BarrelPosition(true), forward, pev);
		}
		else
			m_fireLast = 0;
	}
	else
		m_fireLast = 0;
}


// If barrel is offset, add in additional rotation
void CFuncTank::AdjustAnglesForBarrel(Vector& angles, float distance)
{
	float r2, d2;


	if (m_barrelPos.y != 0 || m_barrelPos.z != 0)
	{
		distance -= m_barrelPos.z;
		d2 = distance * distance;
		if (m_barrelPos.y)
		{
			r2 = m_barrelPos.y * m_barrelPos.y;
			angles.y += (180.0 / M_PI) * atan2(m_barrelPos.y, sqrt(d2 - r2));
		}
		if (m_barrelPos.z)
		{
			r2 = m_barrelPos.z * m_barrelPos.z;
			angles.x += (180.0 / M_PI) * atan2(-m_barrelPos.z, sqrt(d2 - r2));
		}
	}
}


// Fire targets and spawn sprites
void CFuncTank::Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker)
{
	if (m_fireLast != 0)
	{
		if (m_iszSpriteSmoke)
		{
			CSprite* pSprite = CSprite::SpriteCreate(GET_MODEL(STRING(m_iszSpriteSmoke)), barrelEnd, TRUE);
			pSprite->AnimateAndDie(RANDOM_FLOAT(15.0, 20.0));
			pSprite->SetTransparency(kRenderTransAlpha, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, 255, kRenderFxNone);
			pSprite->pev->velocity.z = RANDOM_FLOAT(40, 80);
			pSprite->SetScale(m_spriteScale);
		}
		if (m_iszSpriteFlash)
		{
			CSprite* pSprite = CSprite::SpriteCreate(GET_MODEL(STRING(m_iszSpriteFlash)), barrelEnd, TRUE);
			pSprite->AnimateAndDie(60);
			pSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);
			pSprite->SetScale(m_spriteScale);

			// Hack Hack, make it stick around for at least 100 ms.
			pSprite->pev->nextthink += 0.1;
		}
		SUB_UseTargets(this, USE_TOGGLE, 0);
	}
	m_fireLast = gpGlobals->time;
}


void CFuncTank::TankTrace(const Vector& vecStart, const Vector& vecForward, const Vector& vecSpread, TraceResult& tr)
{
	float x, y;
	GetCircularGaussianSpread(x, y);

	Vector vecDir = vecForward +
		x * vecSpread.x * gpGlobals->v_right +
		y * vecSpread.y * gpGlobals->v_up;
	Vector vecEnd;

	vecEnd = vecStart + vecDir * 4096;
	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, edict(), &tr);
}


void CFuncTank::StartRotSound(void)
{
	if (!pev->noise || (pev->spawnflags & SF_TANK_SOUNDON))
		return;
	pev->spawnflags |= SF_TANK_SOUNDON;
	EMIT_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->noise), 0.85, ATTN_NORM);
}


void CFuncTank::StopRotSound(void)
{
	if (pev->spawnflags & SF_TANK_SOUNDON)
		STOP_SOUND(edict(), CHAN_STATIC, (char*)STRING(pev->noise));
	pev->spawnflags &= ~SF_TANK_SOUNDON;
}