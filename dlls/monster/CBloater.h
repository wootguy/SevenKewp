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
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	void PainSound( void );
	void AlertSound( void );
	void IdleSound( void );
	void AttackSnd( void );

	// No range attacks
	BOOL CheckRangeAttack1 ( float flDot, float flDist ) { return FALSE; }
	BOOL CheckRangeAttack2 ( float flDot, float flDist ) { return FALSE; }
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
};
