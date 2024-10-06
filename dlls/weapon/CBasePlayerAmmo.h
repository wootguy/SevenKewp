#pragma once
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "ammo.h"

class EXPORT CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );
	void KeyValue(KeyValueData* pkvd);
	void DefaultTouch( CBaseEntity *pOther ); // default weapon touch
	void DefaultUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };
	virtual int	ObjectCaps(void);

	CBaseEntity* Respawn( void );
	void Materialize( void );

	float m_flCustomRespawnTime;
};
