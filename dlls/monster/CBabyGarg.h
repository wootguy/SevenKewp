#pragma once
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "CGargantua.h"

#define BABYGARG_FLAME_LENGTH 130
#define BABYGARG_FLAME_WIDTH 120
#define BABYGARG_FLAME_WIDTH2 70
#define BABYGARG_PITCH 180 // babygarg sounds are garg sounds pitched up

class EXPORT CBabyGarg : public CGargantua
{
public:
	void Spawn(void);
	void Precache(void);
	int  Classify(void) { return CBaseMonster::Classify(CLASS_ALIEN_MONSTER); }
	const char* DisplayName() { return m_displayName ? CBaseMonster::DisplayName() : "Baby Gargantua"; }
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-32, -32, 0);
		pev->absmax = pev->origin + Vector(32, 32, 96);
	}
	void Killed(entvars_t* pevAttacker, int iGib);
	void HandleAnimEvent(MonsterEvent_t* pEvent);

	void PainSound();
	void AttackSound();
	void BeamSound(int idx);
	void FootSound();
	void StompSound();
	void BreatheSound();
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();

private:
	static const char* pBeamAttackSounds[];
	static const char* pFootSounds[];
	static const char* pIdleSounds[];
	static const char* pAlertSounds[];
	static const char* pPainSounds[];
	static const char* pAttackSounds[];
	static const char* pStompSounds[];
	static const char* pBreatheSounds[];
	static const char* pDeathSounds[];
};
