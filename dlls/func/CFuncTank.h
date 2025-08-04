#pragma once

#define SF_TANK_ACTIVE			(1<<0)
#define SF_TANK_PLAYER			(1<<1)
#define SF_TANK_HUMANS			(1<<2)
#define SF_TANK_ALIENS			(1<<3)
#define SF_TANK_LINEOFSIGHT		(1<<4)
#define SF_TANK_CANCONTROL		(1<<5)
#define SF_TANK_USE_RELATIONS	(1<<9)
#define SF_TANK_SOUNDON			(1<<15)

enum TANKBULLET
{
	TANK_BULLET_NONE = 0,
	TANK_BULLET_9MM = 1,
	TANK_BULLET_MP5 = 2,
	TANK_BULLET_12MM = 3,
};

extern Vector gTankSpread[];

//			Custom damage
//			env_laser (duration is 0.5 rate of fire)
//			rockets
//			explosion?

class CFuncTank : public CBaseEntity
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	KeyValue(KeyValueData* pkvd);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	Think(void);
	void	TrackTarget(void);
	BOOL	IsTank() { return TRUE; }
	virtual CFuncTank* MyTankPointer(void) { return this; }
	virtual const char* DisplayName() { return "Mounted Gun"; }
	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmAR"; };
	virtual int IRelationship(CBaseEntity* pTarget);

	virtual void Fire(const Vector& barrelEnd, const Vector& forward, entvars_t* pevAttacker);
	virtual Vector UpdateTargetPosition(CBaseEntity* pTarget)
	{
		return pTarget->BodyTarget(pev->origin);
	}

	void	StartRotSound(void);
	void	StopRotSound(void);

	// Bmodels don't go across transitions
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	inline BOOL IsActive(void) { return (pev->spawnflags & SF_TANK_ACTIVE) ? TRUE : FALSE; }
	inline void TankActivate(void) { pev->spawnflags |= SF_TANK_ACTIVE; pev->nextthink = pev->ltime + 0.1; m_fireLast = 0; }
	inline void TankDeactivate(void) { pev->spawnflags &= ~SF_TANK_ACTIVE; m_fireLast = 0; StopRotSound(); }
	inline BOOL CanFire(void) { return (gpGlobals->time - m_lastSightTime) < m_persist; }
	BOOL		InRange(float range);

	// Acquire a target.
	CBaseEntity* FindTarget(Vector forward);

	void		TankTrace(const Vector& vecStart, const Vector& vecForward, const Vector& vecSpread, TraceResult& tr);

	// tipNotBase = true to get the tip of the barrell rather than the base of it
	Vector		BarrelPosition(bool tipNotBase)
	{
		Vector forward, right, up;
		UTIL_MakeVectorsPrivate(pev->angles, forward, right, up);
		
		if (tipNotBase) {
			return pev->origin + (forward * m_barrelPos.x) + (right * m_barrelPos.y) + (up * m_barrelPos.z);
		}
		else {
			return pev->origin + (right * m_barrelPos.y) + (up * m_barrelPos.z);
		}
	}

	void		AdjustAnglesForBarrel(Vector& angles, float distance);

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL OnControls(entvars_t* pevTest);
	BOOL StartControl(CBasePlayer* pController);
	void StopControl(void);
	void ControllerPostFrame(void);

	EHANDLE		m_hController;

protected:
	float		m_flNextAttack;
	Vector		m_vecControllerUsePos;

	float		m_yawCenter;	// "Center" yaw
	float		m_yawRate;		// Max turn rate to track targets
	float		m_yawRange;		// Range of turning motion (one-sided: 30 is +/- 30 degress from center)
								// Zero is full rotation
	float		m_yawTolerance;	// Tolerance angle

	float		m_pitchCenter;	// "Center" pitch
	float		m_pitchRate;	// Max turn rate on pitch
	float		m_pitchRange;	// Range of pitch motion as above
	float		m_pitchTolerance;	// Tolerance angle

	float		m_fireLast;		// Last time I fired
	float		m_fireRate;		// How many rounds/second
	float		m_lastSightTime;// Last time I saw target
	float		m_persist;		// Persistence of firing (how long do I shoot when I can't see)
	float		m_minRange;		// Minimum range to aim/track
	float		m_maxRange;		// Max range to aim/track

	Vector		m_barrelPos;	// Length of the freakin barrel
	float		m_spriteScale;	// Scale of any sprites we shoot
	int			m_iszSpriteSmoke;
	int			m_iszSpriteFlash;
	TANKBULLET	m_bulletType;	// Bullet type
	int			m_iBulletDamage; // 0 means use Bullet type's default damage

	Vector		m_sightOrigin;	// Last sight of target
	int			m_spread;		// firing spread
	int			m_iszMaster;	// Master entity (game_team_master or multisource)

	int8_t		m_iRelation[16]; // manual relationship settings
};