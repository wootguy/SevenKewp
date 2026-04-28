#include "extdll.h"
#include "util.h"
#include "CFuncWall.h"

#define SF_MONSTERCLIP_START_OFF 1

// -------------------------------------------------------------------------------
//
// Monster only clip brush
// 
// This brush will be solid for any entity who has the FL_MONSTERCLIP flag set
// in pev->flags
//
// otherwise it will be invisible and not solid.  This can be used to keep 
// specific monsters out of certain areas
//
// -------------------------------------------------------------------------------
class CFuncMonsterClip : public CFuncWall
{
public:
	int		GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void	Spawn(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	bool m_isActive;
};

LINK_ENTITY_TO_CLASS(func_monsterclip, CFuncMonsterClip)

void CFuncMonsterClip::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	CFuncWall::Spawn();
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		pev->effects = EF_NODRAW;
	pev->flags |= FL_MONSTERCLIP;

	m_isActive = true;
	if (pev->spawnflags & SF_MONSTERCLIP_START_OFF) {
		Use(this, this, USE_OFF, 0);
	}
}

void CFuncMonsterClip::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	m_isActive = useType == USE_TOGGLE ? !m_isActive : useType == USE_ON;
	pev->solid = m_isActive ? SOLID_BSP : SOLID_NOT;
	UTIL_SetOrigin(pev, pev->origin);
}
