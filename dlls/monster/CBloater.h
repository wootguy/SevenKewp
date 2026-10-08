#pragma once
#include	"extdll.h"
#include	"util.h"
#include	"monsters.h"


//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	BLOATER_AE_ATTACK_MELEE1		0x01


class EXPORT CBloater : public CBaseMonster
{
public:
	void Spawn( void ) override;
	void Precache( void ) override;
	void SetYawSpeed( void ) override;
	int  Classify ( void ) override;
	const char* DisplayName() override;
	void HandleAnimEvent( MonsterEvent_t *pEvent ) override;

	void PainSound( void ) override;
	void AlertSound( void ) override;
	void IdleSound( void ) override;
	void AttackSnd( void );

	virtual BOOL IsBarnacleFood(void) override { return TRUE; }

	// No range attacks
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) override { return FALSE; }
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) override { return FALSE; }
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType ) override;
};
