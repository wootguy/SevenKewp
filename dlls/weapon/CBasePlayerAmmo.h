#pragma once
#include "extdll.h"
#include "util.h"
#include "ammo.h"
#include "CBaseEntity.h"

class EXPORT CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn( void );
	void KeyValue(KeyValueData* pkvd);
	void DefaultTouch( CBaseEntity *pOther ); // default weapon touch
	void DefaultUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual BOOL AddAmmo( CBaseEntity *pOther ) { return TRUE; };
	virtual CBasePlayerAmmo* MyAmmoPtr(void) { return this; };
	virtual int	ObjectCaps(void);

	CBaseEntity* Respawn( void );
	void Materialize( void );
	virtual int AddToFullPack(struct entity_state_s* state, CBasePlayer* player) override;

	float m_flCustomRespawnTime;
	uint32_t m_pickupPlayers; // bitfield flagging which players picked up this item in one-pickup-per-player mode
};
