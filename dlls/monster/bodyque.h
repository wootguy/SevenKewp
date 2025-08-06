#include "extdll.h"

// Body queue class here.... It's really just CBaseEntity
class EXPORT CCorpse : public CBaseMonster
{
	virtual int ObjectCaps(void) { return FCAP_DONT_SAVE; }
	int Classify() { return CLASS_NONE; }
	BOOL IsPlayerCorpse(void) { return TRUE; }
	BOOL IsNormalMonster(void) { return FALSE; }
};

EXPORT void CreatePlayerCorpse(CBasePlayer* plr);