#pragma once
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "CSprite.h"

#define GONOME_EVENT_ATTACK1_LEFT 1
#define GONOME_EVENT_ATTACK1_RIGHT 2
#define GONOME_EVENT_GRAB_BLOOD 3
#define GONOME_EVENT_THROW_BLOOD 4
#define GONOME_EVENT_ATTACK2_SWING0 19
#define GONOME_EVENT_ATTACK2_SWING1 20
#define GONOME_EVENT_ATTACK2_SWING2 21
#define GONOME_EVENT_ATTACK2_SWING3 22
#define GONOME_EVENT_PLAY_SOUND 1011

#define GONOME_MELEE_ATTACK1_DISTANCE 80 
#define GONOME_MELEE_ATTACK2_DISTANCE 56 // the one where it tries to eat you
#define GONOME_MELEE_CHASE_DISTANCE 600 // don't waste time with ranged attack within this distance

#define GONOME_MELEE_ATTACK1_SEQUENCE_OFFSET 0
#define GONOME_MELEE_ATTACK2_SEQUENCE_OFFSET 1 // second ATTACK1 sequence in the model should be gonna-eat-you one

#define GONOME_SPIT_SPRITE "sprites/blood_chnk.spr" 

#define GRAB_BLOOD_SOUND "barnacle/bcl_chew2.wav" // this is new in sven co-op

class EXPORT CGonome : public CBaseMonster
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void SetYawSpeed( void ) override;
	int Classify ( void ) override;
	const char* DisplayName() override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;
	BOOL CheckMeleeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	int LookupActivity(int activity) override;
	void Killed(entvars_t* pevAttacker, int iGib) override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void StartTask(Task_t* pTask) override;
	void MonsterThink(void) override;
	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }
	virtual BOOL IsBarnacleFood(void) override { return TRUE; }

	void SetObjectCollisionBox(void) override
	{
		pev->absmin = pev->origin + Vector(-24, -24, 0);
		pev->absmax = pev->origin + Vector(24, 24, 88);
	}

	void PainSound(void) override;
	void AlertSound(void) override;
	void IdleSound(void) override;
	void StartFollowingSound() override;
	void StopFollowingSound() override;
	void CantFollowSound() override;

	CUSTOM_SCHEDULES;

	static const char* pSpitSounds[];

private:
	float m_rangeAttackCooldown; // next time a range attack can be considered
	float m_nextBloodSound; // next time the grabbing blood sound should be played (should really be an animation event)
	EHANDLE m_hHandBlood;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pIdleSounds[];
	static const char* pPainSounds[];
	static const char* pDieSounds[];
	static const char* pEventSounds[];
};

class EXPORT CGonomeSpit : public CBaseEntity
{
public:
	void Spawn(void) override;

	static void Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	void Touch(CBaseEntity* pOther) override;
	void Animate(void);
	const char* GetDeathNoticeWeapon() override { return "weapon_crowbar"; }

	int  m_maxFrame;
};
