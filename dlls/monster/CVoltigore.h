#pragma once
#include "extdll.h"
#include "CBaseMonster.h"
#include "CSprite.h"

#define VOLTI_EVENT_SHOOT_ORB 1
#define VOLTI_EVENT_SLASH_BOTH 12
#define VOLTI_EVENT_SLASH_RIGHT 13

#define VOLTI_MELEE_ATTACK1_DISTANCE 128
#define VOLTI_SHOCK_CHARGE_BEAMS 3
#define VOLTI_SHOCK_FLY_BEAMS 7
#define VOLTI_SHOCK_DEATH_BEAMS 12

#define VOLTI_SHOCK_SPRITE "sprites/blueflare2.spr"
#define VOLTI_BEAM_SPRITE "sprites/lgtning.spr"
#define VOLTI_SPORE_EXPLODE_SPRITE "sprites/spore_exp_01.spr"
#define VOLTI_SPORE_EXPLODE_SPRITE2 "sprites/tinyspit.spr"
#define VOLTI_SPORE_EXPLODE_SOUND "weapons/splauncher_impact.wav"
#define VOLTI_SHOCK_SOUND "debris/beamstart1.wav"

#define VOLTI_HEAD_ATTACHEMENT 1
#define VOLTI_ARM_LEFT_ATTACHEMENT 2
#define VOLTI_ARM_RIGHT_ATTACHEMENT 3
#define VOLTI_SHOCK_ATTACHMENT 4

int sporeExplodeSprIdx = 0;

class EXPORT CVoltigore : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	void SetYawSpeed(void);
	int Classify(void);
	const char* DisplayName();
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	void ExplodeThink(void);
	void Killed(entvars_t* pevAttacker, int iGib);
	int IgnoreConditions(void);
	Schedule_t* GetScheduleOfType(int Type);
	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-80, -80, 0);
		pev->absmax = pev->origin + Vector(80, 80, 90);
	}
	void ShowChargeBeam();
	void HideChargeBeam();

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();
	const char* GetDeathNoticeWeapon() {
		return IsAlive() ? "weapon_crowbar" : "grenade";
	}
	void UpdateOnRemove(void);
	void RemoveBeams();

	virtual void Revive();

private:
	float m_rangeAttackCooldown; // next time a range attack can be considered
	EHANDLE m_handShock;
	EHANDLE m_pBeam[VOLTI_SHOCK_CHARGE_BEAMS];
	EHANDLE m_pDeathBeam[VOLTI_SHOCK_DEATH_BEAMS];
	float explodeTime;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pDieSounds[];
	static const char* pEventSounds[];
};

class CVoltigoreShock : public CBaseEntity
{
public:
	void Spawn(void);

	static void Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	void Touch(CBaseEntity* pOther);
	void EXPORT Fly(void);
	void EXPORT Shock(void);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

private:
	EHANDLE m_pBeam[VOLTI_SHOCK_FLY_BEAMS];
	int beamUpdateIdx;
	int shocksLeft;
};
