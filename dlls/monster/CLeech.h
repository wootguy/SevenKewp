#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"CBaseMonster.h"

// Animation events
#define LEECH_AE_ATTACK		1
#define LEECH_AE_FLOP		2

// Movement constants
#define		LEECH_ACCELERATE		10
#define		LEECH_CHECK_DIST		45
#define		LEECH_SWIM_SPEED		50
#define		LEECH_SWIM_ACCEL		80
#define		LEECH_SWIM_DECEL		10
#define		LEECH_TURN_RATE			90
#define		LEECH_SIZEX				10
#define		LEECH_FRAMETIME			0.1

#define DEBUG_BEAMS		0

#if DEBUG_BEAMS
#include "effects.h"
#endif

class EXPORT CLeech : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	virtual	BOOL IsNormalMonster(void) { return FALSE; }

	void SwimThink( void );
	void DeadThink( void );
	void Touch( CBaseEntity *pOther )
	{
		if ( pOther->IsPlayer() )
		{
			// If the client is pushing me, give me some base velocity
			if ( gpGlobals->trace_ent && gpGlobals->trace_ent == edict() )
			{
				pev->basevelocity = pOther->pev->velocity;
				pev->flags |= FL_BASEVELOCITY;
			}
		}
	}

	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector(-8,-8,0);
		pev->absmax = pev->origin + Vector(8,8,2);
	}

	void AttackSound( void );
	void AlertSound( void );
	void UpdateMotion( void );
	float ObstacleDistance( CBaseEntity *pTarget );
	void MakeVectors( void );
	void RecalculateWaterlevel( void );
	void SwitchLeechState( void );
	
	// Base entity functions
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int	BloodColor( void ) { return DONT_BLEED; }
	void Killed( entvars_t *pevAttacker, int iGib );
	void Activate( void );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	int	Classify( void ) { return CBaseMonster::Classify(CLASS_INSECT); }
	const char* DisplayName() { return m_displayName ? CBaseMonster::DisplayName() : "Leech"; }
	int IRelationship( CBaseEntity *pTarget );
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	static const char *pAttackSounds[];
	static const char *pAlertSounds[];

private:
	// UNDONE: Remove unused boid vars, do group behavior
	float	m_flTurning;// is this boid turning?
	BOOL	m_fPathBlocked;// TRUE if there is an obstacle ahead
	float	m_flAccelerate;
	float	m_obstacle;
	float	m_top;
	float	m_bottom;
	float	m_height;
	float	m_waterTime;
	float	m_sideTime;		// Timer to randomly check clearance on sides
	float	m_zTime;
	float	m_stateTime;
	float	m_attackSoundTime;

#if DEBUG_BEAMS
	CBeam	*m_pb;
	CBeam	*m_pt;
#endif
};
