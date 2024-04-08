#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "effects.h"
#include "CBaseTurret.h"

TYPEDESCRIPTION	CBaseTurret::m_SaveData[] =
{
	DEFINE_FIELD(CBaseTurret, m_flMaxSpin, FIELD_FLOAT),
	DEFINE_FIELD(CBaseTurret, m_iSpin, FIELD_INTEGER),

	DEFINE_FIELD(CBaseTurret, m_hEyeGlow, FIELD_EHANDLE),
	DEFINE_FIELD(CBaseTurret, m_eyeBrightness, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_iDeployHeight, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_iRetractHeight, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_iMinPitch, FIELD_INTEGER),

	DEFINE_FIELD(CBaseTurret, m_iBaseTurnRate, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_fTurnRate, FIELD_FLOAT),
	DEFINE_FIELD(CBaseTurret, m_iOrientation, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_iOn, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_fBeserk, FIELD_INTEGER),
	DEFINE_FIELD(CBaseTurret, m_iAutoStart, FIELD_INTEGER),


	DEFINE_FIELD(CBaseTurret, m_vecLastSight, FIELD_POSITION_VECTOR),
	DEFINE_FIELD(CBaseTurret, m_flLastSight, FIELD_TIME),
	DEFINE_FIELD(CBaseTurret, m_flMaxWait, FIELD_FLOAT),
	DEFINE_FIELD(CBaseTurret, m_iSearchSpeed, FIELD_INTEGER),

	DEFINE_FIELD(CBaseTurret, m_flStartYaw, FIELD_FLOAT),
	DEFINE_FIELD(CBaseTurret, m_vecCurAngles, FIELD_VECTOR),
	DEFINE_FIELD(CBaseTurret, m_vecGoalAngles, FIELD_VECTOR),

	DEFINE_FIELD(CBaseTurret, m_flPingTime, FIELD_TIME),
	DEFINE_FIELD(CBaseTurret, m_flSpinUpTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CBaseTurret, CBaseMonster);

const char* CBaseTurret::pDieSounds[] =
{
	"turret/tu_die.wav",
	"turret/tu_die2.wav",
	"turret/tu_die3.wav",
};

void CBaseTurret::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "maxsleep"))
	{
		m_flMaxWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "orientation"))
	{
		m_iOrientation = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

	}
	else if (FStrEq(pkvd->szKeyName, "searchspeed"))
	{
		m_iSearchSpeed = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

	}
	else if (FStrEq(pkvd->szKeyName, "turnrate"))
	{
		m_iBaseTurnRate = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "style") ||
		FStrEq(pkvd->szKeyName, "height") ||
		FStrEq(pkvd->szKeyName, "value1") ||
		FStrEq(pkvd->szKeyName, "value2") ||
		FStrEq(pkvd->szKeyName, "value3"))
		pkvd->fHandled = TRUE;
	else
		CBaseMonster::KeyValue(pkvd);
}

void CBaseTurret::Spawn()
{
	Precache();
	pev->nextthink = gpGlobals->time + 1;
	pev->movetype = MOVETYPE_FLY;
	pev->sequence = 0;
	pev->frame = 0;
	pev->solid = SOLID_SLIDEBOX;
	pev->takedamage = DAMAGE_AIM;
	SetHealth();

	SetBits(pev->flags, FL_MONSTER);
	SetUse(&CBaseTurret::TurretUse);

	if ((pev->spawnflags & SF_MONSTER_TURRET_AUTOACTIVATE)
		&& !(pev->spawnflags & SF_MONSTER_TURRET_STARTINACTIVE))
	{
		m_iAutoStart = TRUE;
	}

	// setup friendly skin
	bool oldOn = m_iOn;
	m_iOn = true;
	InitModel();
	m_iOn = oldOn;

	ResetSequenceInfo();
	SetBoneController(0, 0);
	SetBoneController(1, 0);
	m_flFieldOfView = VIEW_FIELD_FULL;
	// m_flSightRange = TURRET_RANGE;
}

void CBaseTurret::Precache()
{
	CBaseMonster::Precache();

	PRECACHE_SOUND_ARRAY(pDieSounds);

	PRECACHE_SOUND("turret/tu_fire1.wav");
	PRECACHE_SOUND("turret/tu_ping.wav");
	PRECACHE_SOUND("turret/tu_active2.wav");
	
	// PRECACHE_SOUND ("turret/tu_retract.wav"); // just use deploy sound to save memory
	PRECACHE_SOUND("turret/tu_deploy.wav");
	PRECACHE_SOUND("turret/tu_spinup.wav");
	PRECACHE_SOUND("turret/tu_spindown.wav");
	PRECACHE_SOUND("turret/tu_search.wav");
	PRECACHE_SOUND("turret/tu_alert.wav");

	m_iGibModel = PRECACHE_MODEL("models/mechgibs.mdl");
}

void CBaseTurret::Initialize(void)
{
	// sounds don't work if the turret if the origin is perfectly aligned to a surface (bug with PAS?)
	// so move it away a little bit
	const float PAS_EPSILON = 0.1f; // just a guess that worked for me
	TraceResult trUp, trDown;
	TRACE_LINE(pev->origin, pev->origin + Vector(0, 0, 1), ignore_monsters, NULL, &trUp);
	TRACE_LINE(pev->origin, pev->origin - Vector(0, 0, 1), ignore_monsters, NULL, &trDown);
	if (trUp.flFraction <= PAS_EPSILON) {
		UTIL_SetOrigin(pev, pev->origin - Vector(0, 0, PAS_EPSILON));
	}
	else if (trDown.flFraction <= PAS_EPSILON) {
		UTIL_SetOrigin(pev, pev->origin + Vector(0, 0, PAS_EPSILON));
	}

	m_iOn = 0;
	m_fBeserk = 0;
	m_iSpin = 0;

	SetBoneController(0, 0);
	SetBoneController(1, 0);

	if (m_iBaseTurnRate == 0) m_iBaseTurnRate = TURRET_TURNRATE;
	if (m_flMaxWait == 0) m_flMaxWait = TURRET_MAXWAIT;
	m_flStartYaw = pev->angles.y;
	if (m_iOrientation == 1)
	{
		pev->idealpitch = 180;
		pev->angles.x = 180;
		pev->view_ofs.z = -pev->view_ofs.z;
		pev->effects |= EF_INVLIGHT;
		pev->angles.y = pev->angles.y + 180;
		if (pev->angles.y > 360)
			pev->angles.y = pev->angles.y - 360;
	}

	m_vecGoalAngles.x = 0;

	if (m_iAutoStart)
	{
		m_flLastSight = gpGlobals->time + m_flMaxWait;
		SetThink(&CBaseTurret::AutoSearchThink);
		pev->nextthink = gpGlobals->time + .1;
	}
	else
		SetThink(&CBaseTurret::SUB_DoNothing);
}

void CBaseTurret::TurretUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_iOn))
		return;

	if (m_iOn)
	{
		m_hEnemy = NULL;
		pev->nextthink = gpGlobals->time + 0.1;
		m_iAutoStart = FALSE;// switching off a turret disables autostart
		//!!!! this should spin down first!!BUGBUG
		SetThink(&CBaseTurret::Retire);
	}
	else
	{
		pev->nextthink = gpGlobals->time + 0.1; // turn on delay

		// if the turret is flagged as an autoactivate turret, re-enable it's ability open self.
		if (pev->spawnflags & SF_MONSTER_TURRET_AUTOACTIVATE)
		{
			m_iAutoStart = TRUE;
		}

		SetThink(&CBaseTurret::Deploy);
	}
}

void CBaseTurret::Ping(void)
{
	// make the pinging noise every second while searching
	if (m_flPingTime == 0)
		m_flPingTime = gpGlobals->time + 1;
	else if (m_flPingTime <= gpGlobals->time)
	{
		m_flPingTime = gpGlobals->time + 1;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "turret/tu_ping.wav", 1, ATTN_NORM);
		EyeOn();
	}
	else if (m_eyeBrightness > 0)
	{
		EyeOff();
	}
}

void CBaseTurret::EyeOn()
{
	CSprite* m_pEyeGlow = (CSprite*)m_hEyeGlow.GetEntity();

	if (m_pEyeGlow)
	{
		if (m_eyeBrightness != 255)
		{
			m_eyeBrightness = 255;
		}
		m_pEyeGlow->SetBrightness(m_eyeBrightness);
	}
}

void CBaseTurret::EyeOff()
{
	CSprite* m_pEyeGlow = (CSprite*)m_hEyeGlow.GetEntity();

	if (m_pEyeGlow)
	{
		if (m_eyeBrightness > 0)
		{
			m_eyeBrightness = V_max(0, m_eyeBrightness - 30);
			m_pEyeGlow->SetBrightness(m_eyeBrightness);
		}
	}
}

void CBaseTurret::DeathSound() {
	EMIT_SOUND(ENT(pev), CHAN_BODY, RANDOM_SOUND_ARRAY(pDieSounds), 1.0, ATTN_NORM);
	EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", 0, 0, SND_STOP, 100);
}

void CBaseTurret::ActiveThink(void)
{
	int fAttack = 0;
	Vector vecDirToEnemy;

	pev->nextthink = gpGlobals->time + 0.1;
	StudioFrameAdvance();
	FCheckAITrigger();

	if ((!m_iOn) || (m_hEnemy == NULL))
	{
		m_hEnemy = NULL;
		m_flLastSight = gpGlobals->time + m_flMaxWait;
		SetThink(&CBaseTurret::SearchThink);
		return;
	}

	// if it's dead, look for something new
	if (!m_hEnemy->IsAlive())
	{
		if (!m_flLastSight)
		{
			m_flLastSight = gpGlobals->time + 0.5; // continue-shooting timeout
		}
		else
		{
			if (gpGlobals->time > m_flLastSight)
			{
				m_hEnemy = NULL;
				m_flLastSight = gpGlobals->time + m_flMaxWait;
				SetThink(&CBaseTurret::SearchThink);
				return;
			}
		}
	}

	Vector vecMid = pev->origin + pev->view_ofs;
	Vector vecMidEnemy = m_hEnemy->BodyTarget(vecMid);

	// Look for our current enemy
	int fEnemyVisible = FBoxVisible(pev, m_hEnemy->pev, vecMidEnemy);

	vecDirToEnemy = vecMidEnemy - vecMid;	// calculate dir and dist to enemy
	float flDistToEnemy = vecDirToEnemy.Length();

	Vector vec = UTIL_VecToAngles(vecMidEnemy - vecMid);

	// Current enmey is not visible.
	if (!fEnemyVisible || (flDistToEnemy > TURRET_RANGE))
	{
		if (!m_flLastSight)
			m_flLastSight = gpGlobals->time + 0.5;
		else
		{
			// Should we look for a new target?
			if (gpGlobals->time > m_flLastSight)
			{
				m_hEnemy = NULL;
				m_flLastSight = gpGlobals->time + m_flMaxWait;
				SetThink(&CBaseTurret::SearchThink);
				return;
			}
		}
		fEnemyVisible = 0;
	}
	else
	{
		m_vecLastSight = vecMidEnemy;
	}

	UTIL_MakeAimVectors(m_vecCurAngles);

	/*
	ALERT( at_console, "%.0f %.0f : %.2f %.2f %.2f\n",
		m_vecCurAngles.x, m_vecCurAngles.y,
		gpGlobals->v_forward.x, gpGlobals->v_forward.y, gpGlobals->v_forward.z );
	*/

	Vector vecLOS = vecDirToEnemy; //vecMid - m_vecLastSight;
	vecLOS = vecLOS.Normalize();

	// Is the Gun looking at the target
	if (DotProduct(vecLOS, gpGlobals->v_forward) <= 0.866) // 30 degree slop
		fAttack = FALSE;
	else
		fAttack = TRUE;

	// fire the gun
	if (m_iSpin && ((fAttack) || (m_fBeserk)))
	{
		Vector vecSrc, vecAng;
		GetAttachment(0, vecSrc, vecAng);
		SetTurretAnim(TURRET_ANIM_FIRE);
		Shoot(vecSrc, gpGlobals->v_forward);
	}
	else
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
	}

	//move the gun
	if (m_fBeserk)
	{
		if (RANDOM_LONG(0, 9) == 0)
		{
			m_vecGoalAngles.y = RANDOM_FLOAT(0, 360);
			m_vecGoalAngles.x = RANDOM_FLOAT(0, 90) - 90 * m_iOrientation;
			TakeDamage(pev, pev, 1, DMG_GENERIC); // don't beserk forever
			return;
		}
	}
	else if (fEnemyVisible)
	{
		if (vec.y > 360)
			vec.y -= 360;

		if (vec.y < 0)
			vec.y += 360;

		//ALERT(at_console, "[%.2f]", vec.x);

		if (vec.x < -180)
			vec.x += 360;

		if (vec.x > 180)
			vec.x -= 360;

		// now all numbers should be in [1...360]
		// pin to turret limitations to [-90...15]

		if (m_iOrientation == 0)
		{
			if (vec.x > 90)
				vec.x = 90;
			else if (vec.x < m_iMinPitch)
				vec.x = m_iMinPitch;
		}
		else
		{
			if (vec.x < -90)
				vec.x = -90;
			else if (vec.x > -m_iMinPitch)
				vec.x = -m_iMinPitch;
		}

		// ALERT(at_console, "->[%.2f]\n", vec.x);

		m_vecGoalAngles.y = vec.y;
		m_vecGoalAngles.x = vec.x;

	}

	SpinUpCall();
	MoveTurret();
}

void CBaseTurret::Deploy(void)
{
	pev->nextthink = gpGlobals->time + 0.1;
	StudioFrameAdvance();

	if (pev->sequence != TURRET_ANIM_DEPLOY)
	{
		m_iOn = 1;
		SetTurretAnim(TURRET_ANIM_DEPLOY);
		EMIT_SOUND(ENT(pev), CHAN_BODY, "turret/tu_deploy.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
		SUB_UseTargets(this, USE_ON, 0);
	}

	if (m_fSequenceFinished)
	{
		pev->maxs.z = m_iDeployHeight;
		pev->mins.z = -m_iDeployHeight;
		SetSize(pev->mins, pev->maxs);

		m_vecCurAngles.x = 0;

		if (m_iOrientation == 1)
		{
			m_vecCurAngles.y = UTIL_AngleMod(pev->angles.y + 180);
		}
		else
		{
			m_vecCurAngles.y = UTIL_AngleMod(pev->angles.y);
		}

		SetTurretAnim(TURRET_ANIM_SPIN);
		pev->framerate = 0;
		SetThink(&CBaseTurret::SearchThink);
	}

	m_flLastSight = gpGlobals->time + m_flMaxWait;
}

void CBaseTurret::Retire(void)
{
	// make the turret level
	m_vecGoalAngles.x = 0;
	m_vecGoalAngles.y = m_flStartYaw;

	pev->nextthink = gpGlobals->time + 0.1;

	StudioFrameAdvance();

	EyeOff();

	if (!MoveTurret())
	{
		if (m_iSpin)
		{
			SpinDownCall();
		}
		else if (pev->sequence != TURRET_ANIM_RETIRE)
		{
			SetTurretAnim(TURRET_ANIM_RETIRE);
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "turret/tu_deploy.wav", TURRET_MACHINE_VOLUME, ATTN_NORM, 0, 120);
			SUB_UseTargets(this, USE_OFF, 0);
		}
		else if (m_fSequenceFinished)
		{
			m_iOn = 0;
			m_flLastSight = 0;
			SetTurretAnim(TURRET_ANIM_NONE);
			pev->maxs.z = m_iRetractHeight;
			pev->mins.z = -m_iRetractHeight;
			SetSize(pev->mins, pev->maxs);
			if (m_iAutoStart)
			{
				SetThink(&CBaseTurret::AutoSearchThink);
				pev->nextthink = gpGlobals->time + .1;
			}
			else
				SetThink(&CBaseTurret::SUB_DoNothing);
		}
	}
	else
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
	}
}

void CBaseTurret::SetTurretAnim(TURRET_ANIM anim)
{
	if (pev->sequence != anim)
	{
		switch (anim)
		{
		case TURRET_ANIM_FIRE:
		case TURRET_ANIM_SPIN:
			if (pev->sequence != TURRET_ANIM_FIRE && pev->sequence != TURRET_ANIM_SPIN)
			{
				pev->frame = 0;
			}
			break;
		default:
			pev->frame = 0;
			break;
		}

		pev->sequence = anim;
		ResetSequenceInfo();

		switch (anim)
		{
		case TURRET_ANIM_RETIRE:
			pev->frame = 255;
			pev->framerate = -1.0;
			break;
		case TURRET_ANIM_DIE:
			pev->framerate = 1.0;
			break;
		default:
			break;
		}
		//ALERT(at_console, "Turret anim #%d\n", anim);
	}
}

//
// This search function will sit with the turret deployed and look for a new target. 
// After a set amount of time, the barrel will spin down. After m_flMaxWait, the turret will
// retact.
//
void CBaseTurret::SearchThink(void)
{
	// ensure rethink
	SetTurretAnim(TURRET_ANIM_SPIN);
	StudioFrameAdvance();
	FCheckAITrigger();
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_flSpinUpTime == 0 && m_flMaxSpin)
		m_flSpinUpTime = gpGlobals->time + m_flMaxSpin;

	Ping();

	// If we have a target and we're still healthy
	if (m_hEnemy != NULL)
	{
		if (!m_hEnemy->IsAlive())
			m_hEnemy = NULL;// Dead enemy forces a search for new one
	}


	// Acquire Target
	if (m_hEnemy == NULL)
	{
		Look(TURRET_RANGE);
		m_hEnemy = BestVisibleEnemy();
	}

	// If we've found a target, spin up the barrel and start to attack
	if (m_hEnemy != NULL)
	{
		m_flLastSight = 0;
		m_flSpinUpTime = 0;
		SetThink(&CBaseTurret::ActiveThink);
	}
	else
	{
		// Are we out of time, do we need to retract?
		if (gpGlobals->time > m_flLastSight)
		{
			//Before we retrace, make sure that we are spun down.
			m_flLastSight = 0;
			m_flSpinUpTime = 0;
			SetThink(&CBaseTurret::Retire);
		}
		// should we stop the spin?
		else if ((m_flSpinUpTime) && (gpGlobals->time > m_flSpinUpTime))
		{
			SpinDownCall();
		}

		// generic hunt for new victims
		m_vecGoalAngles.y = (m_vecGoalAngles.y + 0.1 * m_fTurnRate);
		if (m_vecGoalAngles.y >= 360)
			m_vecGoalAngles.y -= 360;
		MoveTurret();
	}
}

// 
// This think function will deploy the turret when something comes into range. This is for
// automatically activated turrets.
//
void CBaseTurret::AutoSearchThink(void)
{
	// ensure rethink
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.3;

	// If we have a target and we're still healthy

	if (m_hEnemy != NULL)
	{
		if (!m_hEnemy->IsAlive())
			m_hEnemy = NULL;// Dead enemy forces a search for new one
	}

	// Acquire Target

	if (m_hEnemy == NULL)
	{
		Look(TURRET_RANGE);
		m_hEnemy = BestVisibleEnemy();
	}

	if (m_hEnemy != NULL)
	{
		SetThink(&CBaseTurret::Deploy);
		EMIT_SOUND(ENT(pev), CHAN_BODY, "turret/tu_alert.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
	}
}

void CBaseTurret::TurretDeath(void)
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->deadflag != DEAD_DEAD)
	{
		pev->deadflag = DEAD_DEAD;

		DeathSound();

		if (m_iOrientation == 0)
			m_vecGoalAngles.x = -15;
		else
			m_vecGoalAngles.x = -90;

		SetTurretAnim(TURRET_ANIM_DIE);

		EyeOn();
	}

	EyeOff();

	if (pev->dmgtime + RANDOM_FLOAT(0, 2) > gpGlobals->time)
	{
		// lots of smoke
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(RANDOM_FLOAT(pev->absmin.x, pev->absmax.x));
		WRITE_COORD(RANDOM_FLOAT(pev->absmin.y, pev->absmax.y));
		WRITE_COORD(pev->origin.z - m_iOrientation * 64);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(25); // scale * 10
		WRITE_BYTE(10 - m_iOrientation * 5); // framerate
		MESSAGE_END();
	}

	if (pev->dmgtime + RANDOM_FLOAT(0, 5) > gpGlobals->time)
	{
		Vector vecSrc = Vector(RANDOM_FLOAT(pev->absmin.x, pev->absmax.x), RANDOM_FLOAT(pev->absmin.y, pev->absmax.y), 0);
		if (m_iOrientation == 0)
			vecSrc = vecSrc + Vector(0, 0, RANDOM_FLOAT(pev->origin.z, pev->absmax.z));
		else
			vecSrc = vecSrc + Vector(0, 0, RANDOM_FLOAT(pev->absmin.z, pev->origin.z));

		UTIL_Sparks(vecSrc);
	}

	if (m_fSequenceFinished && !MoveTurret() && pev->dmgtime + 5 < gpGlobals->time)
	{
		pev->framerate = 0;
		SetThink(NULL);
	}
}

void CBaseTurret::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	if (ptr->iHitgroup == 10)
	{
		// hit armor
		if (pev->dmgtime != gpGlobals->time || (RANDOM_LONG(0, 10) < 1))
		{
			UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(1, 2));
			pev->dmgtime = gpGlobals->time;
		}

		flDamage = 0.1;// don't hurt the monster much, but allow bits_COND_LIGHT_DAMAGE to be generated
	}
	else if (pev->takedamage && flDamage > 0) {
		Vector pos = ptr->vecEndPos;
		Vector dir = ptr->vecPlaneNormal;
		bool isBlast = bitsDamageType & DMG_BLAST;

		UTIL_Shrapnel(pos, dir, flDamage, bitsDamageType);

		if (isBlast && flDamage > pev->health) {
			// turret was effectively gibbed
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pos);
			WRITE_BYTE(TE_BREAKMODEL);
			WRITE_COORD(pos.x);
			WRITE_COORD(pos.y);
			WRITE_COORD(pos.z);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(0);
			WRITE_COORD(dir.x);
			WRITE_COORD(dir.y);
			WRITE_COORD(dir.z);
			WRITE_BYTE(15); // randomization
			WRITE_SHORT(m_iGibModel); // model id#
			WRITE_BYTE(16);
			WRITE_BYTE(1);// duration 0.1 seconds
			WRITE_BYTE(BREAK_METAL); // flags
			MESSAGE_END();
		}
	}

	if (!pev->takedamage)
		return;

	AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType);
}

// take damage. bitsDamageType indicates type of damage sustained, ie: DMG_BULLET
int CBaseTurret::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (IsImmune(pevAttacker))
		return 0;

	if (!m_iOn)
		flDamage /= 10.0;

	GiveScorePoints(pevAttacker, flDamage);

	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		CBaseMonster::Killed(pev, GIB_NEVER); // for monstermaker death notice + death trigger

		pev->health = 0;
		pev->takedamage = DAMAGE_NO;
		pev->dmgtime = gpGlobals->time;
		pev->movetype = MOVETYPE_FLY;

		ClearBits(pev->flags, FL_MONSTER); // why are they set in the first place???

		SetUse(NULL);
		SetThink(&CBaseTurret::TurretDeath);
		SUB_UseTargets(this, USE_ON, 0); // wake up others
		pev->nextthink = gpGlobals->time + 0.1;

		return 0;
	}

	if (pev->health <= 10)
	{
		if (m_iOn && (1 || RANDOM_LONG(0, 0x7FFF) > 800))
		{
			m_fBeserk = 1;
			SetThink(&CBaseTurret::SearchThink);
		}
	}

	return 1;
}

int CBaseTurret::MoveTurret(void)
{
	int state = 0;
	// any x movement?

	if (m_vecCurAngles.x != m_vecGoalAngles.x)
	{
		float flDir = m_vecGoalAngles.x > m_vecCurAngles.x ? 1 : -1;

		m_vecCurAngles.x += 0.1 * m_fTurnRate * flDir;

		// if we started below the goal, and now we're past, peg to goal
		if (flDir == 1)
		{
			if (m_vecCurAngles.x > m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}
		else
		{
			if (m_vecCurAngles.x < m_vecGoalAngles.x)
				m_vecCurAngles.x = m_vecGoalAngles.x;
		}

		if (m_iOrientation == 0)
			SetBoneController(1, -m_vecCurAngles.x);
		else
			SetBoneController(1, m_vecCurAngles.x);
		state = 1;
	}

	if (m_vecCurAngles.y != m_vecGoalAngles.y)
	{
		float flDir = m_vecGoalAngles.y > m_vecCurAngles.y ? 1 : -1;
		float flDist = fabs(m_vecGoalAngles.y - m_vecCurAngles.y);

		if (flDist > 180)
		{
			flDist = 360 - flDist;
			flDir = -flDir;
		}
		if (flDist > 30)
		{
			if (m_fTurnRate < m_iBaseTurnRate * 10)
			{
				m_fTurnRate += m_iBaseTurnRate;
			}
		}
		else if (m_fTurnRate > 45)
		{
			m_fTurnRate -= m_iBaseTurnRate;
		}
		else
		{
			m_fTurnRate += m_iBaseTurnRate;
		}

		m_vecCurAngles.y += 0.1 * m_fTurnRate * flDir;

		if (m_vecCurAngles.y < 0)
			m_vecCurAngles.y += 360;
		else if (m_vecCurAngles.y >= 360)
			m_vecCurAngles.y -= 360;

		if (flDist < (0.05 * m_iBaseTurnRate))
			m_vecCurAngles.y = m_vecGoalAngles.y;

		//ALERT(at_console, "%.2f -> %.2f\n", m_vecCurAngles.y, y);
		if (m_iOrientation == 0)
			SetBoneController(0, m_vecCurAngles.y - pev->angles.y);
		else
			SetBoneController(0, pev->angles.y - 180 - m_vecCurAngles.y);
		state = 1;
	}

	if (!state)
		m_fTurnRate = m_iBaseTurnRate;

	//ALERT(at_console, "(%.2f, %.2f)->(%.2f, %.2f)\n", m_vecCurAngles.x, 
	//	m_vecCurAngles.y, m_vecGoalAngles.x, m_vecGoalAngles.y);
	return state;
}

//
// ID as a machine
//
int	CBaseTurret::Classify(void)
{
	if (m_iOn || m_iAutoStart)
		return	CBaseMonster::Classify(CLASS_MACHINE);
	return CLASS_NONE;
}
