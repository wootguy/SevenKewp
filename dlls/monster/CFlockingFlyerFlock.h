#pragma once
#include	"extdll.h"
#include	"CTalkSquadMonster.h"

#define		AFLOCK_MAX_RECRUIT_RADIUS	1024
#define		AFLOCK_FLY_SPEED			125
#define		AFLOCK_TURN_RATE			75
#define		AFLOCK_ACCELERATE			10
#define		AFLOCK_CHECK_DIST			192
#define		AFLOCK_TOO_CLOSE			100
#define		AFLOCK_TOO_FAR				256

class EXPORT CFlockingFlyerFlock : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void KeyValue( KeyValueData *pkvd );
	void SpawnFlock( void );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int		m_cFlockSize;
	float	m_flFlockRadius;

	static const char* pAlertSounds[];
	static const char* pIdleSounds[];
};


class EXPORT CFlockingFlyer : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SpawnCommonCode( void );
	void IdleThink( void );
	void BoidAdvanceFrame( void );
	void FormFlock( void );
	void Start( void );
	void FlockLeaderThink( void );
	void FlockFollowerThink( void );
	void FallHack( void );
	void MakeSound( void );
	void AlertFlock( void );
	void SpreadFlock( void );
	void SpreadFlock2( void );
	void Killed( entvars_t *pevAttacker, int iGib );
	void Poop ( void );
	BOOL FPathBlocked( void );
	//void KeyValue( KeyValueData *pkvd );
	const char* DisplayName() {
		return m_displayName ? CBaseMonster::DisplayName() : "Flyer Flock";
	}

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int IsLeader( void ) { return m_pSquadLeader == this; }
	int	InSquad( void ) { return m_pSquadLeader != NULL; }
	int	SquadCount( void );
	void SquadRemove( CFlockingFlyer *pRemove );
	void SquadUnlink( void );
	void SquadAdd( CFlockingFlyer *pAdd );
	void SquadDisband( void );

	CFlockingFlyer *m_pSquadLeader;
	CFlockingFlyer *m_pSquadNext;
	BOOL	m_fTurning;// is this boid turning?
	BOOL	m_fCourseAdjust;// followers set this flag TRUE to override flocking while they avoid something
	BOOL	m_fPathBlocked;// TRUE if there is an obstacle ahead
	Vector	m_vecReferencePoint;// last place we saw leader
	Vector	m_vecAdjustedVelocity;// adjusted velocity (used when fCourseAdjust is TRUE)
	float	m_flGoalSpeed;
	float	m_flLastBlockedTime;
	float	m_flFakeBlockedTime;
	float	m_flAlertTime;
	float	m_flFlockNextSoundTime;
};
