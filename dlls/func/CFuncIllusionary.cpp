#include "extdll.h"
#include "util.h"
#include "CBaseToggle.h"

/*QUAKED func_illusionary (0 .5 .8) ?
A simple entity that looks solid but lets you walk through it.
*/
class CFuncIllusionary : public CBaseToggle
{
public:
	void Spawn(void);
	void EXPORT SloshTouch(CBaseEntity* pOther);
	void KeyValue(KeyValueData* pkvd);
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

LINK_ENTITY_TO_CLASS(func_illusionary, CFuncIllusionary)

void CFuncIllusionary::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "skin"))//skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CFuncIllusionary::Spawn(void)
{
	pev->angles = (pev->spawnflags & SF_WALL_USE_ANGLES) ? pev->angles : g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;// always solid_not 
	SET_MODEL(ENT(pev), STRING(pev->model));

	// I'd rather eat the network bandwidth of this than figure out how to save/restore
	// these entities after they have been moved to the client, or respawn them ala Quake
	// Perhaps we can do this in deathmatch only.
	//	MAKE_STATIC(ENT(pev));
}
