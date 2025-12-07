#pragma once
#include "extdll.h"
#include "CBaseMonster.h"

//=========================================================
// Furniture - this is the cool comment I cut-and-pasted
//=========================================================
class EXPORT CFurniture : public CBaseMonster
{
public:
	void Spawn(void);
	void Die(void);
	int	 Classify(void);
	virtual int	ObjectCaps(void) { return (CBaseMonster::ObjectCaps() & ~(FCAP_IMPULSE_USE | FCAP_ACROSS_TRANSITION)); }
};
