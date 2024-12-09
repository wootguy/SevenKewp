#include "extdll.h"
#include "util.h"
#include "animation.h"
#include "effects.h"
#include "CActAnimating.h"
#include "CBasePlayer.h"

#define TREE_AE_ATTACK		1

class CXenTreeTrigger : public CBaseEntity
{
public:
	void		Touch(CBaseEntity* pOther);
	static CXenTreeTrigger* TriggerCreate(edict_t* pOwner, const Vector& position);
};
LINK_ENTITY_TO_CLASS(xen_ttrigger, CXenTreeTrigger)

CXenTreeTrigger* CXenTreeTrigger::TriggerCreate(edict_t* pOwner, const Vector& position)
{
	CXenTreeTrigger* pTrigger = GetClassPtr((CXenTreeTrigger*)NULL);
	pTrigger->pev->origin = position;
	pTrigger->pev->classname = MAKE_STRING("xen_ttrigger");
	pTrigger->pev->solid = SOLID_TRIGGER;
	pTrigger->pev->movetype = MOVETYPE_NONE;
	pTrigger->pev->owner = pOwner;

	return pTrigger;
}

void CXenTreeTrigger::Touch(CBaseEntity* pOther)
{
	if (pev->owner)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(pev->owner);
		if (pEntity)
			pEntity->Touch(pOther);
	}
}

class CXenTree : public CActAnimating
{
public:
	void		Spawn(void);
	void		Precache(void);
	void		Touch(CBaseEntity* pOther);
	void		Think(void);
	int			TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) { Attack(); return 0; }
	void		HandleAnimEvent(MonsterEvent_t* pEvent);
	void		Attack(void);
	int			Classify(void) { return CLASS_BARNACLE; }

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];

private:
	CXenTreeTrigger* m_pTrigger;
};

LINK_ENTITY_TO_CLASS(xen_tree, CXenTree)

TYPEDESCRIPTION	CXenTree::m_SaveData[] =
{
	DEFINE_FIELD(CXenTree, m_pTrigger, FIELD_CLASSPTR),
};

IMPLEMENT_SAVERESTORE(CXenTree, CActAnimating)

void CXenTree::Spawn(void)
{
	Precache();

	SET_MODEL(ENT(pev), "models/tree.mdl");
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_BBOX;

	pev->takedamage = DAMAGE_YES;

	UTIL_SetSize(pev, Vector(-30, -30, 0), Vector(30, 30, 188));
	SetActivity(ACT_IDLE);
	pev->nextthink = gpGlobals->time + 0.1;
	pev->frame = RANDOM_FLOAT(0, 255);
	pev->framerate = RANDOM_FLOAT(0.7, 1.4);

	if (FBitSet(pev->spawnflags, SF_XEN_PLANT_DROP_TO_FLOOR))
		DropToFloor();

	if (FBitSet(pev->flags, FL_KILLME))
		return;

	Vector triggerPosition;
	UTIL_MakeVectorsPrivate(pev->angles, triggerPosition, NULL, NULL);
	triggerPosition = pev->origin + (triggerPosition * 64);
	// Create the trigger
	m_pTrigger = CXenTreeTrigger::TriggerCreate(edict(), triggerPosition);
	if (m_pTrigger)
		UTIL_SetSize(m_pTrigger->pev, Vector(-24, -24, 0), Vector(24, 24, 128));
}

const char* CXenTree::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CXenTree::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

void CXenTree::Precache(void)
{
	PRECACHE_MODEL("models/tree.mdl");
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
}


void CXenTree::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsMonster() || FClassnameIs(pOther->pev, "monster_bigmomma"))
		return;

	Attack();
}


void CXenTree::Attack(void)
{
	if (GetActivity() == ACT_IDLE)
	{
		SetActivity(ACT_MELEE_ATTACK1);
		pev->framerate = RANDOM_FLOAT(1.0, 1.4);
		EMIT_SOUND_ARRAY_DYN(CHAN_WEAPON, pAttackMissSounds);
	}
}


void CXenTree::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case TREE_AE_ATTACK:
	{
		CBaseEntity* pList[8];
		BOOL sound = FALSE;
		int count = UTIL_EntitiesInBox(pList, 8, m_pTrigger->pev->absmin, m_pTrigger->pev->absmax, FL_MONSTER | FL_CLIENT, false);
		Vector forward;

		UTIL_MakeVectorsPrivate(pev->angles, forward, NULL, NULL);

		for (int i = 0; i < count; i++)
		{
			if( pList[i] == this ||
				(pList[i]->IsPlayer() && ((CBasePlayer*)pList[i])->IsObserver()) ) // Don't hit observers
				continue;

			if (pList[i]->pev->owner != edict())
			{
				sound = TRUE;
				pList[i]->TakeDamage(pev, pev, 25, DMG_CRUSH | DMG_SLASH);
				pList[i]->pev->punchangle.x = 15;
				pList[i]->pev->velocity = pList[i]->pev->velocity + forward * 100;
			}
		}

		if (sound)
		{
			EMIT_SOUND_ARRAY_DYN(CHAN_WEAPON, pAttackHitSounds);
		}
	}
	return;
	}

	CActAnimating::HandleAnimEvent(pEvent);
}

void CXenTree::Think(void)
{
	float flInterval = StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.1;
	DispatchAnimEvents(flInterval);

	switch (GetActivity())
	{
	case ACT_MELEE_ATTACK1:
		if (m_fSequenceFinished)
		{
			SetActivity(ACT_IDLE);
			pev->framerate = RANDOM_FLOAT(0.6, 1.4);
		}
		break;

	default:
	case ACT_IDLE:
		break;

	}
}