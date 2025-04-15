#pragma once

// UNDONE: Need to save this!!! It needs class & linkage
class CPlatTrigger : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	virtual int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	void SpawnInsideTrigger(CFuncPlat* pPlatform);
	void Touch(CBaseEntity* pOther);
	EHANDLE m_hPlatform;
	float m_lastTouch;
	float m_nextMove;
};

void PlatSpawnInsideTrigger(entvars_t* pevPlatform);
