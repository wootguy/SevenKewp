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
	void Spawn( void ) override;
	void Precache( void ) override;
	int  Classify( void ) override;
	int IRelationship(CBaseEntity* pTarget) override;
	void SuperBounceTouch( CBaseEntity *pOther );
	void HuntThink( void );
	int  BloodColor( void ) override { return BloodColorAlien(); }
	void Killed( entvars_t *pevAttacker, int iGib ) override;
	void GibMonster( void ) override;
	const char* DisplayName() override { return m_displayName ? CBaseMonster::DisplayName() : "Snark"; }
	virtual const char* GetDeathNoticeWeapon() override { return "snark"; };

	virtual BOOL IsBarnacleFood(void) override { return TRUE; }
	void BarnacleVictimBitten(entvars_t* pevBarnacle) override;
	BOOL BarnacleVictimCaught() override;

	virtual int		Save( CSave &save ) override;
	virtual int		Restore( CRestore &restore ) override;
	
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
