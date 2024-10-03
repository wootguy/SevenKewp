#pragma once

const float GARG_ATTACKDIST = 80.0;

// Garg animation events
#define GARG_AE_SLASH_LEFT			1
//#define GARG_AE_BEAM_ATTACK_RIGHT	2		// No longer used
#define GARG_AE_LEFT_FOOT			3
#define GARG_AE_RIGHT_FOOT			4
#define GARG_AE_STOMP				5
#define GARG_AE_BREATHE				6
#define GARG_AE_KICK				7


// Gargantua is immune to any damage but this
#define GARG_DAMAGE					(DMG_ENERGYBEAM|DMG_CRUSH|DMG_MORTAR|DMG_BLAST)
#define GARG_EYE_SPRITE_NAME		"sprites/gargeye1.spr"
#define GARG_BEAM_SPRITE_NAME		"sprites/xbeam3.spr"
#define GARG_BEAM_SPRITE2			"sprites/xbeam3.spr"
#define GARG_STOMP_SPRITE_NAME		"sprites/gargeye1.spr"
#define GARG_STOMP_BUZZ_SOUND		"weapons/mine_charge.wav"
#define GARG_GIB_MODEL				"models/metalplategibs.mdl"

#define ATTN_GARG					(ATTN_NORM)

#define	STOMP_INTERVAL		0.025
#define STOMP_SPRITE_COUNT			10

#define SPIRAL_INTERVAL		0.1 //025

extern int gStompSprite;

void StreakSplash(const Vector& origin, const Vector& direction, int color, int count, int speed, int velocityRange);
void SpawnExplosion(Vector center, float randomRange, float time, int magnitude);


// Spiral Effect
class CSpiral : public CBaseEntity
{
public:
	void Spawn(void);
	void Think(void);
	int ObjectCaps(void) { return FCAP_DONT_SAVE; }
	static CSpiral* Create(const Vector& origin, float height, float radius, float duration);
};


class CStomp : public CBaseEntity
{
public:
	void Spawn(void);
	void Think(void);
	static CStomp* StompCreate(const Vector& origin, const Vector& end, edict_t* owner, float speed, float damage);

private:
	// UNDONE: re-use this sprite list instead of creating new ones all the time
	//	CSprite		*m_pSprites[ STOMP_SPRITE_COUNT ];
};


class CSmoker : public CBaseEntity
{
public:
	void Spawn(void);
	void Think(void);
};


class CGargantua : public CBaseMonster
{
public:
	virtual void Spawn(void);
	virtual void Precache(void);
	void PrecacheCommon(void);
	void SetYawSpeed(void);
	virtual int  Classify(void);
	virtual const char* DisplayName();
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	void HandleAnimEvent(MonsterEvent_t* pEvent);

	BOOL CheckMeleeAttack1(float flDot, float flDist);		// Swipe
	BOOL CheckMeleeAttack2(float flDot, float flDist);		// Flames
	BOOL CheckRangeAttack1(float flDot, float flDist);		// Stomp attack
	virtual void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-80, -80, 0);
		pev->absmax = pev->origin + Vector(80, 80, 214);
	}

	Schedule_t* GetScheduleOfType(int Type);
	void StartTask(Task_t* pTask);
	void RunTask(Task_t* pTask);

	void PrescheduleThink(void);

	void Killed(entvars_t* pevAttacker, int iGib);
	void DeathEffect(void);

	void EyeOff(void);
	void EyeOn(int level);
	void EyeUpdate(void);
	void Leap(void);
	void StompAttack(void);
	void FlameCreate(void);
	void FlameUpdate(void);
	void FlameControls(float angleX, float angleY);
	void FlameDestroy(void);
	inline BOOL FlameIsOn(void) { return m_hFlame[0] != 0; }

	void FlameDamage(Vector vecStart, Vector vecEnd, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType);

	//virtual void IdleSound();
	//virtual void AlertSound();
	virtual void PainSound();
	virtual void AttackSound();
	virtual void BeamSound(int idx);
	virtual void FootSound();
	virtual void StompSound();
	virtual void BreatheSound();
	virtual void StartFollowingSound();
	virtual void StopFollowingSound();
	virtual void CantFollowSound();

	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }
	

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	CUSTOM_SCHEDULES;

protected:
	CBaseEntity* GargantuaCheckTraceHullAttack(float flDist, int iDamage, int iDmgType);

	EHANDLE m_hEyeGlow;		// Glow around the eyes
	EHANDLE m_hFlame[4];		// Flame beams

	int			m_eyeBrightness;	// Brightness target
	float		m_seeTime;			// Time to attack (when I see the enemy, I set this)
	float		m_flameTime;		// Time of next flame attack
	float		m_painSoundTime;	// Time of next pain sound
	float		m_streakTime;		// streak timer (don't send too many)
	float		m_flameX;			// Flame thrower aim
	float		m_flameY;

	bool explodeOnDeath;
	bool shakeOnStep;
	float flameLength;
	float flameWidth;
	float flameWidth2;
	float meleeAttackHeight;
	float sparkSpeed;

	float slashDamage;
	float fireDamage;
	float stompDamage;
	float m_lastPainSound;

	static const char* pRicSounds[];
	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

private:
	static const char* pBeamAttackSounds[];
	static const char* pFootSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pStompSounds[];
	static const char* pBreatheSounds[];
};
