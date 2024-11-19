#pragma once
#include "CBaseEntity.h"

//
// generic Delay entity.
//
class EXPORT CBaseDelay : public CBaseEntity
{
public:
	float		m_flDelay;
	int			m_iszKillTarget;

	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	virtual void	KeyValue(KeyValueData* pkvd);
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];
	// common member functions
	void SUB_UseTargets(CBaseEntity* pActivator, USE_TYPE useType, float value);
	void DelayThink(void);
};