#pragma once
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "CGib.h"

#define	BARNACLE_BODY_HEIGHT	44 // how 'tall' the barnacle's model is.
#define BARNACLE_KILL_VICTIM_DELAY	5 // how many seconds after pulling prey in to gib them. 

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define	BARNACLE_AE_PUKEGIB	2

#define BARNACLE_CHECK_SPACING	8

class EXPORT CBarnacle : public CBaseMonster
{
public:
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }
	void Spawn( void );
	void Precache( void );
	CBaseEntity *TongueTouchEnt ( float *pflLength );
	int  Classify ( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void BarnacleThink ( void );
	void WaitTillDead ( void );
	void Killed( entvars_t *pevAttacker, int iGib );
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flAltitude;
	float m_flKillVictimTime;
	int	  m_cGibs;// barnacle loads up on gibs each time it kills something.
	BOOL  m_fTongueExtended;
	BOOL  m_fLiftingPrey;
	float m_flTongueAdj;

private:
	static const char* pChewSounds[];
	static const char* pDieSounds[];
};
