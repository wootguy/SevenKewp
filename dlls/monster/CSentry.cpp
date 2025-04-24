#include "extdll.h"
#include "util.h"
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

LINK_ENTITY_TO_CLASS(monster_sentry, CSentry)

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
	InitModel();
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
	float bulletRange = V_max(m_flSightRange, TURRET_RANGE);
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, bulletRange, BULLET_MONSTER_MP5, 1);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM); break;
	}
	PLAY_DISTANT_SOUND(edict(), DISTANT_9MM);

	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

int CSentry::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (IsImmune(pevAttacker, flDamage))
		return 0;

	if (!m_iOn)
	{
		SetThink(&CSentry::Deploy);
		SetUse(NULL);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	float oldHealth = pev->health;
	pev->health = V_min(pev->max_health, pev->health - flDamage);

	GiveScorePoints(pevAttacker, oldHealth - pev->health);

	if (pev->health <= 0)
	{
		CBaseMonster::Killed(pev, GIB_NEVER); // for monstermaker death notice + death trigger
		g_pGameRules->DeathNotice(this, pevAttacker, pevInflictor);

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
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->deadflag != DEAD_DEAD)
	{
		pev->deadflag = DEAD_DEAD;

		DeathSound();

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
		Vector ori = vecSrc + Vector(RANDOM_FLOAT(-16, 16), RANDOM_FLOAT(-16, 16), -32);
		UTIL_Smoke(ori, g_sModelIndexSmoke, 15, 8);
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

	UpdateShockEffect();
}

void CSentry::Deploy(void)
{
	CBaseTurret::Deploy();

	// undo hitbox extension
	pev->maxs.z = m_iDeployHeight;
	pev->mins.z = 0;
	SetSize(pev->mins, pev->maxs);
}
