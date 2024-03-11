
#define TURRET_SHOTS	2
#define TURRET_RANGE	(100 * 12)
#define TURRET_SPREAD	Vector( 0, 0, 0 )
#define TURRET_TURNRATE	30		//angles per 0.1 second
#define TURRET_MAXWAIT	15		// seconds turret will stay active w/o a target
#define TURRET_MAXSPIN	5		// seconds turret barrel will spin w/o a target
#define TURRET_MACHINE_VOLUME	0.5

typedef enum
{
	TURRET_ANIM_NONE = 0,
	TURRET_ANIM_FIRE,
	TURRET_ANIM_SPIN,
	TURRET_ANIM_DEPLOY,
	TURRET_ANIM_RETIRE,
	TURRET_ANIM_DIE,
} TURRET_ANIM;

class CBaseTurret : public CBaseMonster
{
public:
	void Spawn(void);
	virtual void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	void EXPORT TurretUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	virtual int	 TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual int	 Classify(void);
	const char* GetDeathNoticeWeapon() { return "weapon_9mmAR"; }
	BOOL IsMachine() { return 1; } // ignore classification overrides
	BOOL IsTurret() { return 1; }

	int BloodColor(void) { return DONT_BLEED; }
	void GibMonster(void) {}	// UNDONE: Throw turret gibs?

	// Think functions

	void EXPORT ActiveThink(void);
	void EXPORT SearchThink(void);
	void EXPORT AutoSearchThink(void);
	void EXPORT TurretDeath(void);

	virtual void EXPORT SpinDownCall(void) { m_iSpin = 0; }
	virtual void EXPORT SpinUpCall(void) { m_iSpin = 1; }

	// void SpinDown(void);
	// float EXPORT SpinDownCall( void ) { return SpinDown(); }

	// virtual float SpinDown(void) { return 0;}
	// virtual float Retire(void) { return 0;}

	virtual void EXPORT Deploy(void);
	void EXPORT Retire(void);

	void EXPORT Initialize(void);

	virtual void Ping(void);
	virtual void EyeOn(void);
	virtual void EyeOff(void);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	// other functions
	void SetTurretAnim(TURRET_ANIM anim);
	int MoveTurret(void);
	virtual void Shoot(Vector& vecSrc, Vector& vecDirToEnemy) { };

	float m_flMaxSpin;		// Max time to spin the barrel w/o a target
	int m_iSpin;

	EHANDLE m_hEyeGlow;
	int		m_eyeBrightness;

	int	m_iDeployHeight;
	int	m_iRetractHeight;
	int m_iMinPitch;

	int m_iBaseTurnRate;	// angles per second
	float m_fTurnRate;		// actual turn rate
	int m_iOrientation;		// 0 = floor, 1 = Ceiling
	int	m_iOn;
	int m_fBeserk;			// Sometimes this bitch will just freak out
	int m_iAutoStart;		// true if the turret auto deploys when a target
							// enters its range

	Vector m_vecLastSight;
	float m_flLastSight;	// Last time we saw a target
	float m_flMaxWait;		// Max time to seach w/o a target
	int m_iSearchSpeed;		// Not Used!

	// movement
	float	m_flStartYaw;
	Vector	m_vecCurAngles;
	Vector	m_vecGoalAngles;


	float	m_flPingTime;	// Time until the next ping, used when searching
	float	m_flSpinUpTime;	// Amount of time until the barrel should spin down when searching

private:
	static const char* pDieSounds[];
};