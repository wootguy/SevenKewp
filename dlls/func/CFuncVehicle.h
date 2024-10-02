class CFuncVehicle : public CBaseEntity
{
public:
	void Spawn( void );
	void Precache( void );

	void Blocked( CBaseEntity *pOther );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void KeyValue( KeyValueData* pkvd );
	virtual const char* DisplayName() { return "Vehicle"; }

	void EXPORT Next( void );
	void EXPORT Find( void );
	void EXPORT NearestPath( void );
	void EXPORT DeadEnd( void );

	void		NextThink( float thinkTime, BOOL alwaysThink );
	int			Classify (void);
	void		CollisionDetection(void);
	void		TerrainFollowing(void);
	void		CheckTurning(void);

	void SetTrack( CPathTrack *track ) { m_ppath = track->Nearest(pev->origin); }
	void SetControls( entvars_t *pevControls );
	BOOL OnControls( entvars_t *pev );

	void StopSound ( void );
	void UpdateSound ( void );

	static CFuncVehicle *Instance( edict_t *pent );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	static	TYPEDESCRIPTION m_SaveData[];
	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DIRECTIONAL_USE; }

	virtual void	OverrideReset( void );

	CPathTrack	*m_ppath;
	float		m_length;
	float		m_width;
	float		m_height;
	float		m_speed;
	float		m_dir;
	float		m_startSpeed;
	Vector		m_controlMins;
	Vector		m_controlMaxs;
	int			m_soundPlaying;
	int			m_sounds;
	int			m_acceleration;
	float		m_flVolume;
	float		m_flBank;
	float		m_oldSpeed;
	int			m_iTurnAngle;
	float		m_flSteeringWheelDecay;
	float		m_flAcceleratorDecay;
	float		m_flTurnStartTime;			
	float		m_flLaunchTime;				//Time at which the vehicle has become airborne
	float		m_flLastNormalZ;
	float		m_flCanTurnNow;
	float		m_flUpdateSound;
	Vector		m_vFrontLeft;
	Vector		m_vFront;
	Vector		m_vFrontRight;
	Vector		m_vBackLeft;
	Vector		m_vBack;
	Vector		m_vBackRight;
	Vector		m_vSurfaceNormal;
	Vector		m_vVehicleDirection;
	CBasePlayer *m_pDriver;

	// GOOSEMAN
	void Restart();

private:
	unsigned short m_usAdjustPitch;
};