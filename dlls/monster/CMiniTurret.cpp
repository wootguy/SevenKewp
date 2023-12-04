#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "effects.h"
#include "CBaseTurret.h"

class CMiniTurret : public CBaseTurret
{
public:
	void Spawn();
	void Precache(void);
	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy);
};


LINK_ENTITY_TO_CLASS(monster_miniturret, CMiniTurret);


void CMiniTurret::Spawn()
{
	Precache();
	SET_MODEL(ENT(pev), GetModel());
	pev->health = gSkillData.miniturretHealth;
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = 0;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn();
	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	UTIL_SetSize(pev, Vector(-16, -16, -m_iRetractHeight), Vector(16, 16, m_iRetractHeight));

	SetThink(&CMiniTurret::Initialize);
	pev->nextthink = gpGlobals->time + 0.3;
}


void CMiniTurret::Precache()
{
	CBaseTurret::Precache();
	m_defaultModel = "models/miniturret.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_SOUND("weapons/hks1.wav");
	PRECACHE_SOUND("weapons/hks2.wav");
	PRECACHE_SOUND("weapons/hks3.wav");
}

void CMiniTurret::Shoot(Vector& vecSrc, Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_9MM, 1);

	switch (RANDOM_LONG(0, 2))
	{
	case 0: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks1.wav", 1, ATTN_NORM); break;
	case 1: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks2.wav", 1, ATTN_NORM); break;
	case 2: EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/hks3.wav", 1, ATTN_NORM); break;
	}
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}
