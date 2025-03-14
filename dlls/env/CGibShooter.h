#pragma once
#include "CGib.h"

#define	SF_GIBSHOOTER_REPEATABLE	1 // allows a gibshooter to be refired

class CGibShooter : public CBaseDelay
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void	Spawn(void);
	void	Precache(void);
	void	KeyValue(KeyValueData* pkvd);
	void EXPORT ShootThink(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	virtual CGib *CreateGib( float lifeTime );

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	int	m_iGibs;
	int m_iGibCapacity;
	int m_iGibMaterial;
	int m_iGibModelIndex;
	float m_flGibVelocity;
	float m_flVariance;
	float m_flGibLife;
};