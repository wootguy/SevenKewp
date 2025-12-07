#pragma once
#include "extdll.h"
#include "CActAnimating.h"

#define TREE_AE_ATTACK		1

class EXPORT CXenTreeTrigger : public CBaseEntity
{
public:
	void		Touch(CBaseEntity* pOther);
	static CXenTreeTrigger* TriggerCreate(edict_t* pOwner, const Vector& position);
};

class EXPORT CXenTree : public CActAnimating
{
public:
	void		Spawn(void);
	void		Precache(void);
	void		Touch(CBaseEntity* pOther);
	void		AnimateThink(void);
	void		DropThink(void);
	int			TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) { Attack(); return 0; }
	void		HandleAnimEvent(MonsterEvent_t* pEvent);
	void		Attack(void);
	int			Classify(void) { return CLASS_BARNACLE; }
	const char* DisplayName() { return "Xen Tree"; }

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

private:
	CXenTreeTrigger* m_pTrigger;
};
