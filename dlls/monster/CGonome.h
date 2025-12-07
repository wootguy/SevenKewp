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
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int Classify ( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	int LookupActivity(int activity);
	void Killed(entvars_t* pevAttacker, int iGib);
	Schedule_t* GetScheduleOfType(int Type);
	void MonsterThink(void);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-24, -24, 0);
		pev->absmax = pev->origin + Vector(24, 24, 88);
	}

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();

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
	void Spawn(void);

	static void Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	void Touch(CBaseEntity* pOther);
	void Animate(void);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	int  m_maxFrame;
};
