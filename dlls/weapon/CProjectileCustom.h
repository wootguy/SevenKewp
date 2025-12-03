#pragma once
#include "CBaseEntity.h"

class EXPORT CProjectileCustom : public CBaseEntity {
public:
	void Spawn(void);
	void Precache(void);
	int  Classify(void);
	void DefaultThink();
	void DefaultTouch(CBaseEntity* pOther);
	virtual bool CustomThink() { return false; } // return true to override the default think logic
	virtual bool CustomTouch(CBaseEntity* pOther) { return false; } // return true to override the default touch logic
};