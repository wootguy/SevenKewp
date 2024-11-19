#include "extdll.h"
#include "util.h"
#include "effects.h"
#include "explode.h"
#include "CFuncTank.h"

class CFuncTankControls : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	virtual int	ObjectCaps(void);
	void Spawn(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void Think(void);

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hTank;
};
LINK_ENTITY_TO_CLASS(func_tankcontrols, CFuncTankControls)

TYPEDESCRIPTION	CFuncTankControls::m_SaveData[] =
{
	DEFINE_FIELD(CFuncTankControls, m_hTank, FIELD_EHANDLE),
};

IMPLEMENT_SAVERESTORE(CFuncTankControls, CBaseEntity)

int	CFuncTankControls::ObjectCaps(void)
{
	return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE;
}


void CFuncTankControls::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{ // pass the Use command onto the controls
	if (m_hTank)
		m_hTank->Use(pActivator, pCaller, useType, value);

	ASSERT(m_hTank != NULL);	// if this fails,  most likely means save/restore hasn't worked properly
}


void CFuncTankControls::Think(void)
{
	edict_t* pTarget = NULL;

	do
	{
		pTarget = FIND_ENTITY_BY_TARGETNAME(pTarget, STRING(pev->target));
	} while (!FNullEnt(pTarget) && strncmp(STRING(pTarget->v.classname), "func_tank", 9));

	if (FNullEnt(pTarget))
	{
		ALERT(at_console, "No tank %s\n", STRING(pev->target));
		return;
	}

	m_hTank = (CFuncTank*)Instance(pTarget);
}

void CFuncTankControls::Spawn(void)
{
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;
	SET_MODEL(ENT(pev), STRING(pev->model));

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	pev->nextthink = gpGlobals->time + 0.3;	// After all the func_tank's have spawned

	CBaseEntity::Spawn();
}
