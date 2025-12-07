#pragma once
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "CGrenade.h"

#define SF_WAITFORTRIGGER	(0x04 | 0x40) // UNDONE: Fix!
#define SF_NOWRECKAGE		0x08

class EXPORT CApache : public CBaseMonster
{
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );
	int  Classify( void ) { return CBaseMonster::Classify(CLASS_HUMAN_MILITARY); }
	BOOL IsMachine() { return 1; } // ignore classification overrides
	const char* DisplayName() { return m_displayName ? CBaseMonster::DisplayName() : "Apache"; }
	int  BloodColor( void ) { return DONT_BLEED; }
	void Killed( entvars_t *pevAttacker, int iGib );
	void GibMonster( void );

	// when crashing, show an explosion kill icon
	const char* GetDeathNoticeWeapon() { return IsAlive() ? "weapon_9mmAR" : "grenade"; }

	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector( -300, -300, -172);
		pev->absmax = pev->origin + Vector(300, 300, 8);
	}

	void HuntThink( void );
	void FlyTouch( CBaseEntity *pOther );
	void CrashTouch( CBaseEntity *pOther );
	void DyingThink( void );
	void StartupUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void NullThink( void );

	void ShowDamage( void );
	void Flight( void );
	void FireRocket( void );
	BOOL FireGun( void );
	
	int  TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	int m_iRockets;
	float m_flForce;
	float m_flNextRocket;

	Vector m_vecTarget;
	Vector m_posTarget;

	Vector m_vecDesired;
	Vector m_posDesired;

	Vector m_vecGoal;

	Vector m_angGun;
	float m_flLastSeen;
	float m_flPrevSeen;

	int m_iSoundState; // don't save this

	int m_iSpriteTexture;
	int m_iExplode;
	int m_iBodyGibs;
	int m_iGlassHit;
	int m_iEngineHit;
	int m_iGlassGibs;
	int m_iEngineGibs;

	float m_flGoalSpeed;

	int m_iDoSmokePuff;
};


class EXPORT CApacheHVR : public CGrenade
{
	void Spawn( void );
	void Precache( void );
	void IgniteThink( void );
	void AccelerateThink( void );
	virtual const char* GetDeathNoticeWeapon() { return "rpg_rocket"; };

	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	virtual int MergedModelBody() { return MERGE_MDL_HVR; }

	int m_iTrail;
	Vector m_vecForward;
};
