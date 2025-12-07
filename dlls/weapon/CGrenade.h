#pragma once
#include "CBaseMonster.h"

// Contact Grenade / Timed grenade / Satchel Charge
class EXPORT CGrenade : public CBaseMonster
{
public:
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }

	void Spawn( void );

	typedef enum { SATCHEL_DETONATE = 0, SATCHEL_RELEASE } SATCHELCODE;

	static CGrenade *ShootTimed( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity, float time, const char* model=NULL);
	static CGrenade *ShootContact( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static CGrenade *ShootSatchelCharge( entvars_t *pevOwner, Vector vecStart, Vector vecVelocity );
	static void UseSatchelCharges( entvars_t *pevOwner, SATCHELCODE code );

	virtual void Explode( Vector vecSrc, Vector vecAim );
	virtual void Explode( TraceResult *pTrace, int bitsDamageType );
	void Smoke( void );

	void BounceTouch( CBaseEntity *pOther );
	void SlideTouch( CBaseEntity *pOther );
	void ExplodeTouch( CBaseEntity *pOther );
	void DangerSoundThink( void );
	void PreDetonate( void );
	void Detonate( void );
	void DetonateUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
	void TumbleThink( void );

	virtual void BounceSound( void );
	virtual int	BloodColor( void ) { return DONT_BLEED; }
	virtual void Killed( entvars_t *pevAttacker, int iGib );
	virtual const char* DisplayName() { return "Grenade"; }
	virtual const char* GetDeathNoticeWeapon() { return "monster_grenade"; }

	const char* GetModel();
	void SetGrenadeModel();
	virtual int MergedModelBody() { return -1; }
	virtual	BOOL IsNormalMonster(void) { return FALSE; }

	BOOL m_fRegisteredSound;// whether or not this grenade has issued its DANGER sound to the world sound list yet.
	Vector m_effectOrigin; // where to play the explosion effects (offset from real origin so sprites look nice)
};
