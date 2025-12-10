#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseMonster.h"

#define N_SCALE		15
#define N_SPHERES	20

class CNihilanth : public CBaseMonster
{
public:
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }
	void Spawn( void );
	void Precache( void );
	int  Classify( void ) { return CBaseMonster::Classify(CLASS_ALIEN_MILITARY); };
	const char* DisplayName() { return m_displayName ? CBaseMonster::DisplayName() : "Nihilanth"; }
	int  BloodColor( void ) { return BloodColorAlien(); }
	void Killed( entvars_t *pevAttacker, int iGib );
	void GibMonster( void );

	void SetObjectCollisionBox( void )
	{
		pev->absmin = pev->origin + Vector( -16 * N_SCALE, -16 * N_SCALE, -48 * N_SCALE );
		pev->absmax = pev->origin + Vector( 16 * N_SCALE, 16 * N_SCALE, 28 * N_SCALE );
	}

	void HandleAnimEvent( MonsterEvent_t *pEvent );

	void StartupThink( void );
	void HuntThink( void );
	void CrashTouch( CBaseEntity *pOther );
	void DyingThink( void );
	void StartupUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void NullThink( void );
	void CommandUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void FloatSequence( void );
	void NextActivity( void );

	void Flight( void );

	BOOL AbsorbSphere( void );
	BOOL EmitSphere( void );
	void TargetSphere( USE_TYPE useType, float value );
	CBaseEntity *RandomTargetname( const char *szName );
	void ShootBalls( void );
	void MakeFriend( Vector vecPos );
	
	int  TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	void PainSound( void );
	void DeathSound( void );
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	static const char *pAttackSounds[];	// vocalization: play sometimes when he launches an attack
	static const char *pBallSounds[];	// the sound of the lightening ball launch
	static const char *pShootSounds[];	// grunting vocalization: play sometimes when he launches an attack
	static const char *pRechargeSounds[];	// vocalization: play when he recharges
	static const char *pLaughSounds[];	// vocalization: play sometimes when hit and still has lots of health
	static const char *pPainSounds[];	// vocalization: play sometimes when hit and has much less health and no more chargers
	static const char *pDeathSounds[];	// vocalization: play as he dies
	
	// x_teleattack1.wav	the looping sound of the teleport attack ball.

	float m_flForce;

	float m_flNextPainSound;

	Vector m_velocity;
	Vector m_avelocity;

	Vector m_vecTarget;
	Vector m_posTarget;

	Vector m_vecDesired;
	Vector m_posDesired;

	float  m_flMinZ;
	float  m_flMaxZ;

	Vector m_vecGoal;

	float m_flLastSeen;
	float m_flPrevSeen;

	int m_irritation;

	int m_iLevel;
	int m_iTeleport;

	EHANDLE m_hRecharger;

	EHANDLE m_hSphere[N_SPHERES];
	int	m_iActiveSpheres;

	float m_flAdj;

	EHANDLE m_hBall;

	char m_szRechargerTarget[64];
	char m_szDrawUse[64];
	char m_szTeleportUse[64];
	char m_szTeleportTouch[64];
	char m_szDeadUse[64];
	char m_szDeadTouch[64];

	float m_flShootEnd;
	float m_flShootTime;

	float m_irritationTime; // for automatically starting combat even if not triggered

	EHANDLE m_hFriend[3];
};


class EXPORT CNihilanthHVR : public CBaseMonster
{
public:
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );
	const char* DisplayName();

	void CircleInit( CBaseEntity *pTarget );
	void AbsorbInit( void );
	void TeleportInit( CNihilanth *pOwner, CBaseEntity *pEnemy, CBaseEntity *pTarget, CBaseEntity *pTouch );
	void GreenBallInit( void );
	void ZapInit( CBaseEntity *pEnemy );

	void HoverThink( void );
	BOOL CircleTarget( Vector vecTarget );
	void DissipateThink( void );

	void ZapThink( void );
	void TeleportThink( void );
	void TeleportTouch( CBaseEntity *pOther );
	
	void RemoveTouch( CBaseEntity *pOther );
	void BounceTouch( CBaseEntity *pOther );
	void ZapTouch( CBaseEntity *pOther );

	CBaseEntity *RandomClassname( const char *szName );

	// void EXPORT SphereUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void MovetoTarget( Vector vecTarget );
	virtual void Crawl( void );

	void Zap( void );
	void Teleport( void );

	float m_flIdealVel;
	Vector m_vecIdeal;
	CNihilanth *m_pNihilanth;
	EHANDLE m_hTouch;
	int m_nFrames;
};
