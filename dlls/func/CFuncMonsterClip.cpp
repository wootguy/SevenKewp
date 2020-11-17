#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "doors.h"
#include "CFuncWall.h"

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
	void	Spawn(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {}		// Clear out func_wall's use function
};

LINK_ENTITY_TO_CLASS(func_monsterclip, CFuncMonsterClip);

void CFuncMonsterClip::Spawn(void)
{
	CFuncWall::Spawn();
	if (CVAR_GET_FLOAT("showtriggers") == 0)
		pev->effects = EF_NODRAW;
	pev->flags |= FL_MONSTERCLIP;
}
