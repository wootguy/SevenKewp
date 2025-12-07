#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseMonster.h"
#include "CBasePlayerWeapon.h"

#define TEMP_FOR_SCREEN_SHOTS
#ifdef TEMP_FOR_SCREEN_SHOTS //===================================================

class EXPORT CCycler : public CBaseMonster
{
public:
	void GenericCyclerSpawn(const char *szModel, Vector vecMin, Vector vecMax);
	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() | FCAP_IMPULSE_USE); }
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void Spawn( void );
	void Think( void );
	//void Pain( float flDamage );
	void Use ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	// Don't treat as a live target
	virtual BOOL IsAlive( void ) { return FALSE; }
	virtual BOOL IsAllowedToSpeak() { return TRUE; }
	virtual BOOL IsNormalMonster() { return FALSE; }

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	int			m_animate;
};

//
// we should get rid of all the other cyclers and replace them with this.
//
class EXPORT CGenericCycler : public CCycler
{
public:
	void Spawn( void ) { GenericCyclerSpawn( STRING(pev->model), Vector(-16, -16, 0), Vector(16, 16, 72) ); }
};

// Probe droid imported for tech demo compatibility
//
// PROBE DROID
//
class EXPORT CCyclerProbe : public CCycler
{
public:	
	void Spawn( void );
};

#endif

class EXPORT CCyclerSprite : public CBaseEntity
{
public:
	void Spawn( void );
	void Think( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	virtual int	ObjectCaps( void ) { return (CBaseEntity :: ObjectCaps() | FCAP_DONT_SAVE | FCAP_IMPULSE_USE); }
	virtual int	TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType );
	void	Animate( float frames );

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	inline int		ShouldAnimate( void ) { return m_animate && m_maxFrame > 1.0; }
	int			m_animate;
	float		m_lastTime;
	float		m_maxFrame;
};


class EXPORT CWeaponCycler : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	int iItemSlot( void ) { return 1; }
	int GetItemInfo(ItemInfo *p) {return 0; }

	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iszModel;
	int m_iModel;
};


// Flaming Wreakage
class EXPORT CWreckage : public CBaseMonster
{
	int		Save( CSave &save );
	int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	void Spawn( void );
	void Precache( void );
	void Think( void );

	int m_flStartTime;
};
