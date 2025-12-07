#pragma once
#include "extdll.h"
#include "CActAnimating.h"
#include "CPointEntity.h"

// UNDONE:	These need to smoke somehow when they take damage
//			Touch behavior?
//			Cause damage in smoke area

//
// Spores
//
class EXPORT CXenSpore : public CActAnimating
{
public:
	void		Spawn(void);
	void		Precache(void);
	void		Touch(CBaseEntity* pOther);
	void		AnimateThink(void);
	void		DropThink(void);
	int			TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) { Attack(); return 0; }
	//	void		HandleAnimEvent( MonsterEvent_t *pEvent );
	void		Attack(void) {}

	static const char* pModelNames[];
};

class EXPORT CXenSporeSmall : public CXenSpore
{
	void		Spawn(void);
};

class EXPORT CXenSporeMed : public CXenSpore
{
	void		Spawn(void);
};

class EXPORT CXenSporeLarge : public CXenSpore
{
	void		Spawn(void);

	static const Vector m_hullSizes[];
};

// Fake collision box for big spores
class EXPORT CXenHull : public CPointEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	static CXenHull* CreateHull(CBaseEntity* source, const Vector& mins, const Vector& maxs, const Vector& offset);
	int			Classify(void) { return CLASS_BARNACLE; }
};
