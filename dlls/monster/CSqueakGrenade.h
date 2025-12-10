#pragma once
#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "weapon/CSqueak.h"
#include "weapon/CGrenade.h"
#include "monsters.h"

#define SQUEEK_DETONATE_DELAY	15.0

enum w_squeak_e {
	WSQUEAK_IDLE1 = 0,
	WSQUEAK_FIDGET,
	WSQUEAK_JUMP,
	WSQUEAK_RUN,
};

class EXPORT CSqueakGrenade : public CGrenade
{
	void Spawn( void );
	void Precache( void );
	int  Classify( void );
	int IRelationship(CBaseEntity* pTarget);
	void SuperBounceTouch( CBaseEntity *pOther );
	void HuntThink( void );
	int  BloodColor( void ) { return BloodColorAlien(); }
	void Killed( entvars_t *pevAttacker, int iGib );
	void GibMonster( void );
	const char* DisplayName() { return m_displayName ? CBaseMonster::DisplayName() : "Snark"; }
	virtual const char* GetDeathNoticeWeapon() { return "snark"; };

	virtual int		Save( CSave &save ); 
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	static float m_flNextBounceSoundTime;

	// CBaseEntity *m_pTarget;
	float m_flDie;
	Vector m_vecTarget;
	float m_flNextHunt;
	float m_flNextHit;
	Vector m_posPrev;
	EHANDLE m_hOwner;
	int  m_iMyClass;

private:
	static const char* pHuntSounds[];
};
