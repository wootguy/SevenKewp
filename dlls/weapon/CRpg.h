#pragma once
#include "CBasePlayerWeapon.h"
#include "CGrenade.h"

#define RPG_MAX_CLIP			1
#define RPG_DEFAULT_GIVE		1

extern int g_laserBeamIdx;

class CLaserSpot : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );

	int	ObjectCaps( void ) { return FCAP_DONT_SAVE; }

public:
	void Suspend( float flSuspendTime );
	void EXPORT Revive( void );

	void EXPORT MonsterAimThink();

	// updates faster than monster AI so that laser movement is smooth and accurate
	void ActivateMonsterControl();
	
	static CLaserSpot *CreateSpot( edict_t* owner );
};

class CRpg : public CBasePlayerWeapon
{
public:

#ifndef CLIENT_DLL
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
#endif

	void Spawn( void );
	void Precache( void );
	void PrecacheEvents( void );
	void Reload( void );
	int iItemSlot( void ) { return 4; }
	int GetItemInfo(ItemInfo *p);

	BOOL Deploy( void );
	BOOL CanHolster( void );
	void Holster( int skiplocal = 0 );

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void WeaponIdle( void );

	void UpdateSpot( void );
	BOOL ShouldWeaponIdle( void ) { return TRUE; };

	EHANDLE m_hSpot;
	int m_fSpotActive;
	int m_cActiveRockets;// how many missiles in flight from this launcher right now?
	float m_lastBeamUpdate;

	virtual int MergedModelBody() { return MERGE_MDL_W_RPG; }

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	unsigned short m_usRpg;

};

class CRpgRocket : public CGrenade
{
public:
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];
	void Spawn( void );
	void Precache( void );
	void EXPORT FollowThink( void );
	void EXPORT IgniteThink( void );
	void EXPORT RocketTouch( CBaseEntity *pOther );
	virtual void Explode(TraceResult* pTrace, int bitsDamageType);
	virtual const char* GetDeathNoticeWeapon() { return "rpg_rocket"; };

	static CRpgRocket *CreateRpgRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner, CRpg *pLauncher );

	virtual int MergedModelBody() { return MERGE_MDL_RPGROCKET; }

	int m_iTrail;
	float m_flIgniteTime;
	EHANDLE m_hLauncher;// pointer back to the launcher that fired me. 
};