#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "effects.h"
#include "CBaseTurret.h"

//=========================================================
// Sentry gun - smallest turret, placed near grunt entrenchments
//=========================================================
class CSentry : public CBaseTurret
{
public:
	void Spawn();
	void Precache(void);
	const char* DisplayName();
	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void EXPORT SentryTouch(CBaseEntity* pOther);
	void EXPORT SentryDeath(void);
	void EXPORT DropInit(void);
	void Deploy(void);

};

LINK_ENTITY_TO_CLASS(monster_sentry, CSentry);

void CSentry::Precache()
{
	CBaseTurret::Precache();
	m_defaultModel = "models/sentry.mdl";
	PRECACHE_MODEL(GetModel());

	PRECACHE_SOUND("weapons/hks1.wav");
	PRECACHE_SOUND("weapons/hks2.wav");
	PRECACHE_SOUND("weapons/hks3.wav");
}

const char* CSentry::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Sentry Turret";
}

void CSentry::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), GetModel());
	if (!pev->health) pev->health = gSkillData.sentryHealth;
	m_HackedGunPos = Vector(0, 0, 48);
	pev->view_ofs.z = 48;
	m_flMaxWait = 1E6;
	m_flMaxSpin = 1E6;

	CBaseTurret::Spawn();
	m_iRetractHeight = 64;
	m_iDeployHeight = 64;
	m_iMinPitch = -60;
	SetSize(Vector(-16, -16, 0), Vector(16, 16, m_iRetractHeight));

	SetTouch(&CSentry::SentryTouch);
	SetThink(&CSentry::DropInit);
	pev->nextthink = gpGlobals->time + 0.3;
}

void CSentry::DropInit() {
	// not doing this in Spawn() in case a func_wall floor spawns after the sentry during map init
	DROP_TO_FLOOR(edict());

	Initialize();
}

void CSentry::Shoot(Vector& vecSrc, Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_MP5, 1);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

int CSentry::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (IsImmune(pevAttacker))
		return 0;

	if (!m_iOn)
	{
		SetThink(&CSentry::Deploy);
		SetUse(NULL);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	GiveScorePoints(pevAttacker, flDamage);

	pev->health -= flDamage;
	if (pev->health <= 0)
	{
		pev->health = 0;
		pev->takedamage = DAMAGE_NO;
		pev->dmgtime = gpGlobals->time;

		ClearBits(pev->flags, FL_MONSTER); // why are they set in the first place???

		SetUse(NULL);
		SetThink(&CSentry::SentryDeath);
		SUB_UseTargets(this, USE_ON, 0); // wake up others
		pev->nextthink = gpGlobals->time + 0.1;

		return 0;
	}

	return 1;
}


void CSentry::SentryTouch(CBaseEntity* pOther)
{
	if (pOther && (pOther->IsPlayer() || (pOther->pev->flags & FL_MONSTER)))
	{
		TakeDamage(pOther->pev, pOther->pev, 0, 0);
	}
}


void CSentry::SentryDeath(void)
{
	BOOL iActive = FALSE;

	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->deadflag != DEAD_DEAD)
	{
		pev->deadflag = DEAD_DEAD;

		float flRndSound = RANDOM_FLOAT(0, 1);

		if (flRndSound <= 0.33)
			EMIT_SOUND(ENT(pev), CHAN_BODY, "turret/tu_die.wav", 1.0, ATTN_NORM);
		else if (flRndSound <= 0.66)
			EMIT_SOUND(ENT(pev), CHAN_BODY, "turret/tu_die2.wav", 1.0, ATTN_NORM);
		else
			EMIT_SOUND(ENT(pev), CHAN_BODY, "turret/tu_die3.wav", 1.0, ATTN_NORM);

		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", 0, 0, SND_STOP, 100);

		SetBoneController(0, 0);
		SetBoneController(1, 0);

		SetTurretAnim(TURRET_ANIM_DIE);

		pev->solid = SOLID_NOT;
		pev->angles.y = UTIL_AngleMod(pev->angles.y + RANDOM_LONG(0, 2) * 120);

		EyeOn();
	}

	EyeOff();

	Vector vecSrc, vecAng;
	GetAttachment(1, vecSrc, vecAng);

	if (pev->dmgtime + RANDOM_FLOAT(0, 2) > gpGlobals->time)
	{
		// lots of smoke
		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(vecSrc.x + RANDOM_FLOAT(-16, 16));
		WRITE_COORD(vecSrc.y + RANDOM_FLOAT(-16, 16));
		WRITE_COORD(vecSrc.z - 32);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(15); // scale * 10
		WRITE_BYTE(8); // framerate
		MESSAGE_END();
	}

	if (pev->dmgtime + RANDOM_FLOAT(0, 8) > gpGlobals->time)
	{
		UTIL_Sparks(vecSrc);
	}

	if (m_fSequenceFinished && pev->dmgtime + 5 < gpGlobals->time)
	{
		pev->framerate = 0;
		SetThink(NULL);
	}
}

void CSentry::Deploy(void)
{
	CBaseTurret::Deploy();

	// undo hitbox extension
	pev->maxs.z = m_iDeployHeight;
	pev->mins.z = 0;
	SetSize(pev->mins, pev->maxs);
}
