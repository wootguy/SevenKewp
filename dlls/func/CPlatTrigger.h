// UNDONE: Need to save this!!! It needs class & linkage
class CPlatTrigger : public CBaseEntity
{
public:
	virtual int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	void SpawnInsideTrigger(CFuncPlat* pPlatform);
	void Touch(CBaseEntity* pOther);
	CFuncPlat* m_pPlatform;
};

void PlatSpawnInsideTrigger(entvars_t* pevPlatform);
