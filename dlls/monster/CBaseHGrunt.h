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

#define HGRUNT_9MMAR				( 1 << 0)
#define HGRUNT_HANDGRENADE			( 1 << 1)
#define HGRUNT_GRENADELAUNCHER		( 1 << 2)
#define HGRUNT_SHOTGUN				( 1 << 3)

#define HEAD_GROUP					1
#define HEAD_GRUNT					0
#define HEAD_COMMANDER				1
#define HEAD_SHOTGUN				2
#define HEAD_M203					3
#define GUN_GROUP					2
#define GUN_MP5						0
#define GUN_SHOTGUN					1
#define GUN_NONE					2

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
};

//=========================================================
// monster-specific tasks
//=========================================================
enum
{
	TASK_GRUNT_FACE_TOSS_DIR = LAST_COMMON_TASK + 1,
	TASK_GRUNT_SPEAK_SENTENCE,
	TASK_GRUNT_CHECK_FIRE,
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
};

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
extern Schedule_t	slGruntRangeAttack2[];
extern Schedule_t	slGruntRepel[];
extern Schedule_t	slGruntRepelAttack[];
extern Schedule_t	slGruntRepelLand[];

//=========================================================
// monster-specific conditions
//=========================================================
#define bits_COND_GRUNT_NOFIRE	( bits_COND_SPECIAL2 )

extern int g_fGruntQuestion; // true if an idle grunt asked a question. Cleared when someone answers.

class CBaseHGrunt : public CTalkSquadMonster
{
public:
	void BaseSpawn(const char* model);
	void BasePrecache();
	void SetYawSpeed(void);
	int  Classify(void);
	int ISoundMask(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	BOOL FCanCheckAttacks(void);
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack2(float flDot, float flDist);
	void CheckAmmo(void);
	void SetActivity(Activity NewActivity);
	virtual int GetActivitySequence(Activity NewActivity);
	void StartTask(Task_t* pTask);
	void RunTask(Task_t* pTask);
	Vector GetGunPosition(void);
	void Shoot(void);
	void Shotgun(void);
	void PrescheduleThink(void);
	virtual void GibMonster(void);
	virtual CBaseEntity* DropGun(Vector pos, Vector angles);
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
	Schedule_t* GetSchedule(void);
	Schedule_t* GetScheduleOfType(int Type);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);

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

	int m_voicePitch;

	int		m_iBrassShell;
	int		m_iShotgunShell;

	int		m_iSentence;

	int m_iGruntHead;
};

class CBaseRepel : public CBaseMonster {
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	void EXPORT RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual const char* GetMonsterType() { return "monster_human_grunt"; }

	const char* monsterType;
	int m_iSpriteTexture;	// Don't save, precache
	int m_iGruntHead;
	int m_iszUse;
	int m_iszUnUse;
};

class CBaseDead : public CBaseMonster {
public:
	void BaseSpawn(const char* model);
	virtual int	Classify(void) { return	CLASS_HUMAN_MILITARY; }
	virtual int GetPoseSequence() { return -1; }

	void KeyValue(KeyValueData* pkvd);

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	int m_iGruntHead;
};