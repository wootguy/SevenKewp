#include "extdll.h"
#include "util.h"
#include "trains.h"
#include "saverestore.h"
#include "CBasePlatTrain.h"
#include "CFuncPlat.h"
#include "CFuncPlatRot.h"
#include "CFuncTrackTrain.h"


// This class defines the volume of space that the player must stand in to control the train
class CFuncTrainControls : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void Spawn(void);
	void EXPORT Find(void);
};
LINK_ENTITY_TO_CLASS(func_traincontrols, CFuncTrainControls)


void CFuncTrainControls::Find(void)
{
	CBaseEntity* pTarget = NULL;

	do
	{
		pTarget = UTIL_FindEntityByTargetname(pTarget, STRING(pev->target));
	} while (pTarget && !FClassnameIs(pTarget->pev, "func_tracktrain"));

	if (!pTarget)
	{
		ALERT(at_console, "No train %s\n", STRING(pev->target));
		return;
	}

	CFuncTrackTrain* ptrain = CFuncTrackTrain::Instance(pTarget->edict());
	ptrain->SetControls(pev);
	UTIL_Remove(this);
}


void CFuncTrainControls::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model));

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	SetThink(&CFuncTrainControls::Find);
	pev->nextthink = gpGlobals->time;
}