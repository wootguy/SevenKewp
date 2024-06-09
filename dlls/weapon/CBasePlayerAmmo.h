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
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };

	CBaseEntity* Respawn( void );
	void Materialize( void );

	float m_flCustomRespawnTime;
};
