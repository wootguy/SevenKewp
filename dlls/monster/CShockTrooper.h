#pragma once
#include "extdll.h"
#include "util.h"
#include "CTalkSquadMonster.h"
#include "CBaseGrunt.h"

//=========================================================
// monster-specific DEFINE's
//=========================================================
#define ShockTrooper_SENTENCE_VOLUME (float)0.5	// volume of grunt sentences

namespace STrooperBodyGroup
{
	enum STrooperBodyGroup
	{
		Weapons = 1,
	};
}

namespace STrooperWeapon
{
	enum STrooperWeapon
	{
		Roach = 0,
		None = 1
	};
}

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define STROOPER_AE_RELOAD (2)
#define STROOPER_AE_KICK (3)
#define STROOPER_AE_SHOOT (4)
#define STROOPER_AE_GREN_TOSS (7)
#define STROOPER_AE_CAUGHT_ENEMY (10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define STROOPER_AE_DROP_GUN (11)	  // grunt (probably dead) is dropping his mp5.

enum ShockTrooper_SENTENCE_TYPES
{
	ShockTrooper_SENT_NONE = -1,
	ShockTrooper_SENT_GREN = 0,
	ShockTrooper_SENT_ALERT,
	ShockTrooper_SENT_MONSTER,
	ShockTrooper_SENT_COVER,
	ShockTrooper_SENT_THROW,
	ShockTrooper_SENT_CHARGE,
	ShockTrooper_SENT_TAUNT,
};

class EXPORT CShockTrooper : public CTalkSquadMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	const char* DisplayName();
	int ISoundMask() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	BOOL FCanCheckAttacks() override;
	BOOL CheckMeleeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack2(float flDot, float flDist) override;
	void CheckAmmo() override;
	void SetActivity(Activity NewActivity) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void PainSound() override;
	void IdleSound() override;
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();
	Vector GetGunPosition() override;
	void Shoot();
	void PrescheduleThink() override;
	void GibMonster() override;
	void SpeakSentence();

	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-24, -24, 0);
		pev->absmax = pev->origin + Vector(24, 24, 88);
	}

	BOOL Save(CSave& save) override;
	BOOL Restore(CRestore& restore) override;

	CBaseEntity* Kick();
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	int IRelationship(CBaseEntity* pTarget) override;

	bool FOkToSpeak();
	void JustSpoke();

	void MonsterThink() override;

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;

	Vector m_vecTossVelocity;

	bool m_fThrowGrenade;
	bool m_fStanding;
	bool m_fFirstEncounter; // only put on the handsign show in the squad's first encounter.
	int m_cClipSize;

	float m_flLastShot;

	int m_voicePitch;

	int m_iBrassShell;
	int m_iShotgunShell;

	int m_iSentence;

	float m_flLastChargeTime;
	float m_flLastBlinkTime;
	float m_flLastBlinkInterval;

	static const char* pGruntSentences[];
	static const char* pPainSounds[];
	static const char* pDieSounds[]; // TODO: unused?
	static const char* pAlertSounds[];
};


class CShockTrooperRepel : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int m_iSpriteTexture; // Don't save, precache
};


//=========================================================
// DEAD SHOCKTROOPER PROP
//=========================================================
class CDeadShockTrooper : public CBaseMonster
{
public:
	void Spawn() override;
	int Classify() override { return CBaseMonster::Classify(CLASS_HUMAN_MILITARY); }

	void KeyValue(KeyValueData* pkvd) override;

	int m_iPose; // which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[3];
};
