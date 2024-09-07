#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "effects.h"
#include "CBaseTurret.h"

#define TURRET_GLOW_SPRITE "sprites/flare3.spr"

class CTurret : public CBaseTurret
{
public:
	void Spawn(void);
	void Precache(void);
	const char* DisplayName();
	// Think functions
	void SpinUpCall(void);
	void SpinDownCall(void);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	// other functions
	void Shoot(Vector& vecSrc, Vector& vecDirToEnemy);

private:
	int m_iStartSpin;

};

TYPEDESCRIPTION	CTurret::m_SaveData[] =
{
	DEFINE_FIELD(CTurret, m_iStartSpin, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTurret, CBaseTurret)
LINK_ENTITY_TO_CLASS(monster_turret, CTurret)

void CTurret::Spawn()
{
	Precache();
	InitModel();
	m_HackedGunPos = Vector(0, 0, 12.75);
	m_flMaxSpin = TURRET_MAXSPIN;
	pev->view_ofs.z = 12.75;

	CBaseTurret::Spawn();

	m_iRetractHeight = 16;
	m_iDeployHeight = 32;
	m_iMinPitch = -15;
	SetSize(Vector(-32, -32, -m_iRetractHeight), Vector(32, 32, m_iRetractHeight));

	SetThink(&CTurret::Initialize);

	CSprite* eye = CSprite::SpriteCreate(TURRET_GLOW_SPRITE, pev->origin, FALSE);
	m_hEyeGlow = eye;

	// setup friendly glow
	bool oldOn = m_iOn;
	m_iOn = true;
	if (CBaseMonster::IRelationship(Classify(), CLASS_PLAYER) == R_AL) {
		eye->SetTransparency(kRenderGlow, 0, 255, 0, 0, kRenderFxNoDissipation);
	}
	else {
		eye->SetTransparency(kRenderGlow, 255, 0, 0, 0, kRenderFxNoDissipation);
	}
	m_iOn = oldOn;
	
	eye->SetAttachment(edict(), 2);
	m_eyeBrightness = 0;

	pev->nextthink = gpGlobals->time + 0.3;
}

void CTurret::Precache()
{
	CBaseTurret::Precache();
	m_defaultModel = "models/turret.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_MODEL(TURRET_GLOW_SPRITE);
}

const char* CTurret::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Turret";
}

void CTurret::Shoot(Vector& vecSrc, Vector& vecDirToEnemy)
{
	FireBullets(1, vecSrc, vecDirToEnemy, TURRET_SPREAD, TURRET_RANGE, BULLET_MONSTER_12MM, 1);
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "turret/tu_fire1.wav", 1, 0.6);
	PLAY_DISTANT_SOUND(edict(), DISTANT_357);
	pev->effects = pev->effects | EF_MUZZLEFLASH;
}

void CTurret::SpinUpCall(void)
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;

	// Are we already spun up? If not start the two stage process.
	if (!m_iSpin)
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
		// for the first pass, spin up the the barrel
		if (!m_iStartSpin)
		{
			pev->nextthink = gpGlobals->time + 1.0; // spinup delay
			EMIT_SOUND(ENT(pev), CHAN_BODY, "turret/tu_spinup.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
			m_iStartSpin = 1;
			pev->framerate = 0.1;
		}
		// after the barrel is spun up, turn on the hum
		else if (pev->framerate >= 1.0)
		{
			pev->nextthink = gpGlobals->time + 0.1; // retarget delay
			EMIT_SOUND(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
			SetThink(&CTurret::ActiveThink);
			m_iStartSpin = 0;
			m_iSpin = 1;
		}
		else
		{
			pev->framerate += 0.075;
		}
	}

	if (m_iSpin)
	{
		SetThink(&CTurret::ActiveThink);
	}
}

void CTurret::SpinDownCall(void)
{
	if (m_iSpin)
	{
		SetTurretAnim(TURRET_ANIM_SPIN);
		if (pev->framerate == 1.0)
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, "turret/tu_active2.wav", 0, 0, SND_STOP, 100);
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "turret/tu_spindown.wav", TURRET_MACHINE_VOLUME, ATTN_NORM);
		}
		pev->framerate -= 0.02;
		if (pev->framerate <= 0)
		{
			pev->framerate = 0;
			m_iSpin = 0;
		}
	}
}
