#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "doors.h"

#define SF_BTARGET_USE		0x0001
#define SF_BTARGET_ON		0x0002

class CButtonTarget : public CBaseEntity
{
public:
	void Spawn(void);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	int	ObjectCaps(void);

};

LINK_ENTITY_TO_CLASS(button_target, CButtonTarget);

void CButtonTarget::Spawn(void)
{
	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->takedamage = DAMAGE_YES;

	if (FBitSet(pev->spawnflags, SF_BTARGET_ON))
		pev->frame = 1;
}

void CButtonTarget::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, (int)pev->frame))
		return;
	pev->frame = 1 - pev->frame;
	if (pev->frame)
		SUB_UseTargets(pActivator, USE_ON, 0);
	else
		SUB_UseTargets(pActivator, USE_OFF, 0);
}


int	CButtonTarget::ObjectCaps(void)
{
	int caps = CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;

	if (FBitSet(pev->spawnflags, SF_BTARGET_USE))
		return caps | FCAP_IMPULSE_USE;
	else
		return caps;
}


int CButtonTarget::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	Use(Instance(pevAttacker), this, USE_TOGGLE, 0);

	return 1;
}
