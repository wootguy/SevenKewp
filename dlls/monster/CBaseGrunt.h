//=========================================================
// monster-specific DEFINE's
//=========================================================
#define	GRUNT_CLIP_SIZE					36 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define GRUNT_VOL						0.35		// volume of grunt sounds
#define GRUNT_ATTN						ATTN_NORM	// attenutation of grunt sentences
#define HGRUNT_LIMP_HEALTH				20
#define HGRUNT_DMG_HEADSHOT				( DMG_BULLET | DMG_CLUB )	// damage types that can kill a grunt with a single headshot.
#define HGRUNT_NUM_HEADS				2 // how many grunt heads are there? 
#define HGRUNT_MINIMUM_HEADSHOT_DAMAGE	15 // must do at least this much damage in one shot to head to score a headshot kill

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define		HGRUNT_AE_RELOAD		( 2 )
#define		HGRUNT_AE_KICK			( 3 )
#define		HGRUNT_AE_BURST1		( 4 )
#define		HGRUNT_AE_BURST2		( 5 ) 
#define		HGRUNT_AE_BURST3		( 6 ) 
#define		HGRUNT_AE_GREN_TOSS		( 7 )
#define		HGRUNT_AE_GREN_LAUNCH	( 8 )
#define		HGRUNT_AE_GREN_DROP		( 9 )
#define		HGRUNT_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		HGRUNT_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

//=========================================================
// monster-specific schedule types
//=========================================================
enum
{
	SCHED_GRUNT_SUPPRESS = LAST_COMMON_SCHEDULE + 1,
	SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE,// move to a location to set up an attack against the enemy. (usually when a friendly is in the way).
	SCHED_GRUNT_COVER_AND_RELOAD,
	SCHED_GRUNT_SWEEP,
	SCHED_GRUNT_FOUND_ENEMY,
	SCHED_GRUNT_REPEL,
	SCHED_GRUNT_REPEL_ATTACK,
	SCHED_GRUNT_REPEL_LAND,
	SCHED_GRUNT_WAIT_FACE_ENEMY,
	SCHED_GRUNT_TAKECOVER_FAILED,// special schedule type that forces analysis of conditions and picks the best possible schedule to recover from this type of failure.
	SCHED_GRUNT_ELOF_FAIL,
	LAST_BASE_GRUNT_SCHEDULE
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_GRUNT_FACE_TOSS_DIR = LAST_TALKMONSTER_TASK + 1,
	TASK_GRUNT_SPEAK_SENTENCE,
	TASK_GRUNT_CHECK_FIRE,
	LAST_BASE_GRUNT_TASK
};

enum HGRUNT_SENTENCE_TYPES
{
	HGRUNT_SENT_NONE = -1,
	HGRUNT_SENT_GREN = 0,
	HGRUNT_SENT_ALERT,
	HGRUNT_SENT_MONSTER,
	HGRUNT_SENT_COVER,
	HGRUNT_SENT_THROW,
	HGRUNT_SENT_CHARGE,
	HGRUNT_SENT_TAUNT,
	HGRUNT_SENT_SHOT,
	HGRUNT_SENT_MAD,
	HGRUNT_SENT_KILL,
};

// each monster handles weapon flags differently. This unites those flags into a bitfield
// that all monsters can interpret the same way
enum MONSTER_EQUIPMENT
{
	MEQUIP_MP5				= 1 << 0,
	MEQUIP_HAND_GRENADE		= 1 << 1,
	MEQUIP_GRENADE_LAUNCHER = 1 << 2,
	MEQUIP_SHOTGUN			= 1 << 3,
	MEQUIP_SAW				= 1 << 4,
	MEQUIP_SNIPER			= 1 << 5,
	MEQUIP_GLOCK			= 1 << 6,
	MEQUIP_DEAGLE			= 1 << 7,
	MEQUIP_357				= 1 << 8,
	MEQUIP_MINIGUN			= 1 << 9,
	MEQUIP_AKIMBO_UZIS		= 1 << 10,
	MEQUIP_NEEDLE			= 1 << 11, // for healing
	MEQUIP_HELMET			= 1 << 12,
};

#define MEQUIP_EVERYTHING 0xffffffff

#define ANY_RANGED_WEAPON (MEQUIP_MP5 | MEQUIP_SHOTGUN | MEQUIP_SAW | MEQUIP_SNIPER \
						  | MEQUIP_GLOCK | MEQUIP_DEAGLE | MEQUIP_357 | MEQUIP_MINIGUN \
						  | MEQUIP_AKIMBO_UZIS)

extern Schedule_t	slGruntFail[];
extern Schedule_t	slGruntCombatFail[];
extern Schedule_t	slGruntVictoryDance[];
extern Schedule_t slGruntEstablishLineOfFire[];
extern Schedule_t	slGruntFoundEnemy[];
extern Schedule_t	slGruntCombatFace[];
extern Schedule_t	slGruntSignalSuppress[];
extern Schedule_t	slGruntSuppress[];
extern Schedule_t	slGruntWaitInCover[];
extern Schedule_t	slGruntTakeCover[];
extern Schedule_t	slGruntGrenadeCover[];
extern Schedule_t	slGruntTossGrenadeCover[];
extern Schedule_t	slGruntTakeCoverFromBestSound[];
extern Schedule_t slGruntHideReload[];
extern Schedule_t	slGruntSweep[];
extern Schedule_t	slGruntRangeAttack1A[];
extern Schedule_t	slGruntRangeAttack1B[];
extern Schedule_t	slGruntRangeAttack1C[];
extern Schedule_t	slGruntRangeAttack2[];
extern Schedule_t	slGruntRepel[];
extern Schedule_t	slGruntRepelAttack[];
extern Schedule_t	slGruntRepelLand[];
extern Schedule_t	slMinigunSpinup[];
extern Schedule_t	slMinigunSpindown[];

//=========================================================
// monster-specific conditions
//=========================================================
#define bits_COND_GRUNT_NOFIRE	( bits_COND_SPECIAL2 )

extern int g_fGruntQuestion; // true if an idle grunt asked a question. Cleared when someone answers.

class CBaseGrunt : public CTalkSquadMonster
{
public:
	void BaseSpawn();
	void BasePrecache();
	void PrecacheEquipment(int equipment);
	void SetYawSpeed(void);
	int  Classify(void);
	virtual int ISoundMask(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	BOOL FCanCheckAttacks(void);
	virtual BOOL CheckMeleeAttack1(float flDot, float flDist);
	virtual BOOL CheckRangeAttack1(float flDot, float flDist);
	virtual BOOL CheckRangeAttack2(float flDot, float flDist);
	virtual void CheckAmmo(void);
	virtual void SetActivity(Activity NewActivity);
	virtual int GetActivitySequence(Activity NewActivity);
	virtual void StartTask(Task_t* pTask);
	void RunTask(Task_t* pTask);
	virtual const char* GetTaskName(int taskIdx);
	Vector GetGunPosition(void);
	bool HasEquipment(int equipItems);
	void Shoot(bool firstRound);
	void ShootMp5(Vector& vecShootOrigin, Vector& vecShootDir);
	void ShootUzis(Vector& vecShootOrigin, Vector& vecShootDir);
	void ShootSniper(Vector& vecShootOrigin, Vector& vecShootDir);
	void ShootMinigun(Vector& vecShootOrigin, Vector& vecShootDir);
	void ShootShotgun(Vector& vecShootOrigin, Vector& vecShootDir);
	void ShootSaw(Vector& vecShootOrigin, Vector& vecShootDir);
	void ShootGlock(Vector& vecShootOrigin, Vector& vecShootDir);
	void ShootDeagle(Vector& vecShootOrigin, Vector& vecShootDir);
	void Shoot357(Vector& vecShootOrigin, Vector& vecShootDir);
	void Reload();
	void PointAtEnemy();
	virtual void PrescheduleThink(void);
	virtual void GibMonster(void);
	virtual void Killed(entvars_t* pevAttacker, int iGib);
	virtual bool DropEquipment(int attachmentIdx, bool randomToss);
	virtual bool DropEquipment(int attachmentIdx, int equipMask, Vector velocity, Vector aVelocity);
	virtual const char* GetDeathNoticeWeapon(); // for player death notice icons
	void SpeakSentence(void);
	virtual void PlaySentenceSound(int sentenceType) {}

	int	Save(CSave& save);
	int Restore(CRestore& restore);

	CBaseEntity* Kick(void);
	virtual Schedule_t* GetMonsterStateSchedule(void);
	Schedule_t* GetNewSquadEnemySchedule(void);
	Schedule_t* GetShootSchedule(void);
	Schedule_t* GetLightDamageSchedule(void);
	Schedule_t* GetEnemyOccludedSchedule(void);
	virtual Schedule_t* GetSchedule(void);
	virtual Schedule_t* GetScheduleOfType(int Type);
	virtual void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

	int IRelationship(CBaseEntity* pTarget);

	BOOL FOkToSpeak(void);
	void JustSpoke(void);

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;

	Vector	m_vecTossVelocity;

	BOOL	m_fThrowGrenade;
	BOOL	m_fStanding;
	BOOL	m_fFirstEncounter;// only put on the handsign show in the squad's first encounter.
	int		m_cClipSize;

	float m_flLastShot;
	BOOL m_lastAttackCheck;

	int	m_iBrassShell;
	int m_iShotgunShell;
	int m_iSawShell;
	int m_iSawLink;

	int		m_iSentence;

	int m_iGruntHead;

	int m_iEquipment; // bitfield of MONSTER_EQUIPMENT

	virtual void InitAiFlags();
	// AI behavior flags (TODO: make this a bitfield)
	bool waitForEnemyFire; // wait for the enemy to attack before shooting
	bool runFromHeavyDamage; // take cover if taking heavy damage
	bool canCallMedic; // call out for a medic when injured
	bool suppressOccludedTarget; // keep firing at the last known target position if the target is behind cover

	float maxSuppressTime; // max time to shoot at a wall the target took cover behind

	int shellEjectAttachment;

	float maxShootDist; // max range for primary weapon (not grenades)

private:
	void DropEquipmentToss(const char* cname, Vector vecGunPos, Vector vecGunAngles, Vector velocity, Vector aVelocity);
};

class CBaseRepel : public CBaseMonster {
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	void EXPORT RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual const char* GetMonsterType() { return "monster_human_grunt"; }
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }

	const char* monsterType;
	int m_iSpriteTexture;	// Don't save, precache
	int m_iGruntHead;
	int m_iszUse;
	int m_iszUnUse;
};

class CBaseDead : public CBaseMonster {
public:
	void BaseSpawn(const char* model);
	virtual int	Classify(void) { return	CBaseMonster::Classify(CLASS_HUMAN_MILITARY); }
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }
	virtual int GetPoseSequence() { return -1; }

	void KeyValue(KeyValueData* pkvd);

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	int m_iGruntHead;
};