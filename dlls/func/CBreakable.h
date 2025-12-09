#ifndef FUNC_BREAK_H
#define FUNC_BREAK_H
#include "CBaseDelay.h"

#define MAX_BREAKABLE_OBJECT_SPAWN_TYPES 256

// remaps classnames spawned by func_breakable
EXPORT extern string_t g_breakableSpawnRemap[MAX_BREAKABLE_OBJECT_SPAWN_TYPES];

class CBreakable : public CBaseDelay
{
public:
	// basic functions
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	void EXPORT BreakTouch(CBaseEntity* pOther);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual Vector GetTargetOrigin() override { return Center(); }

	virtual void BreakableDie(CBaseEntity* pActivator) override;
	virtual int		ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static const char* pSpawnObjects[];

	static	TYPEDESCRIPTION m_SaveData[];

	float		m_angle;
	int			m_iszSpawnObject;
};

#endif	// FUNC_BREAK_H
