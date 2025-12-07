#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseMonster.h"

typedef struct 
{
	int isValid;
	EHANDLE hGrunt;
	Vector	vecOrigin;
	Vector  vecAngles;
} t_ospreygrunt;

#define SF_OSPREY_WAITFORTRIGGER	0x40
#define OSPREY_MAX_CARRY	24

enum OverrideGruntType {
	OSPREY_GRUNT_DEFAULT,
	OSPREY_GRUNT_OP4,
	OSPREY_GRUNT_RANDOM,				// randomly choose from: op4, hecu
	OSPREY_GRUNT_RANDOM_HWGRUNT,		// randomly choose from: op4, hecu, hw
	OSPREY_GRUNT_RANDOM_HWGRUNT_SNIPER	// randomly choose from: op4, hecu, hw, sniper
};

class EXPORT COsprey : public CBaseMonster
{
public:
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
	int		ObjectCaps( void ) { return CBaseMonster :: ObjectCaps() & ~(FCAP_IMPULSE_USE | FCAP_ACROSS_TRANSITION); }
	
	void Spawn( void );
	void KeyValue(KeyValueData* pkvd);
	void Precache( void );
	int  Classify( void ) { return CBaseMonster::Classify(CLASS_MACHINE); };
	BOOL IsMachine() { return 1; } // ignore classification overrides
	const char* DisplayName() { return m_displayName ? CBaseMonster::DisplayName() : "Osprey"; }
	int  BloodColor( void ) { return DONT_BLEED; }
	void Killed( entvars_t *pevAttacker, int iGib );

	void UpdateGoal( void );
	BOOL HasDead( void );
	void FlyThink( void );
	void DeployThink( void );
	void Flight( void );
	void HitTouch( CBaseEntity *pOther );
	void FindAllThink( void );
	void HoverThink( void );
	CBaseMonster *MakeGrunt( Vector vecSrc );
	void CrashTouch( CBaseEntity *pOther );
	void DyingThink( void );
	void CommandUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	int  TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	void ShowDamage( void );
	void Update();

	EHANDLE m_hGoalEnt;
	Vector m_vel1;
	Vector m_vel2;
	Vector m_pos1;
	Vector m_pos2;
	Vector m_ang1;
	Vector m_ang2;
	float m_startTime;
	float m_dTime;

	Vector m_velocity;

	float m_flRotortilt;

	float m_flRightHealth;
	float m_flLeftHealth;

	int	m_iUnits;
	EHANDLE m_hGrunt[OSPREY_MAX_CARRY];
	Vector m_vecOrigin[OSPREY_MAX_CARRY];
	EHANDLE m_hRepel[4];

	int m_iSoundState;
	int m_iSpriteTexture;

	int m_iPitch;

	int m_iExplode;
	int	m_iTailGibs;
	int	m_iBodyGibs;
	int	m_iEngineGibs;
	int m_iGlassHit;
	int m_iEngineHit;
	int m_iGlassGibs;
	int m_iMechGibs;

	int m_overrideGruntCount;
	int m_overrideGruntType;
	const char* replenishMonster;
};
