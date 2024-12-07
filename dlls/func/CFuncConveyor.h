#include "CFuncWall.h"

#define SF_CONVEYOR_VISUAL		0x0001
#define SF_CONVEYOR_NOTSOLID	0x0002

class CFuncConveyor : public CFuncWall
{
public:
	void	Spawn(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	UpdateSpeed(float speed);
};