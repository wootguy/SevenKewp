#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "CBaseLogic.h"

//
// CTriggerHurtRemote / trigger_hurt_remote	-- Damages or kills any entity with custom damage types

#define SF_RHURT_INSTANT_KILL 1
#define SF_RHURT_CONSTANT 2
#define SF_RHURT_START_ON 4
#define SF_RHURT_DO_ARMOR 8

class CTriggerHurtRemote : public CBaseLogic
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Spawn();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void ConstantThink();
	void HurtAllTargets();
	void HurtTarget(CBaseEntity* target);
	virtual const char* DisplayName() { return "Hazard"; }

	string_t m_targetClass;
	float m_armorDmg;
	float m_delay;
	int m_damageType;
	bool m_isActive;
};

LINK_ENTITY_TO_CLASS(trigger_hurt_remote, CTriggerHurtRemote)

void CTriggerHurtRemote::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "targetclass"))
	{
		m_targetClass = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "armordmg"))
	{
		m_armorDmg = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "delay"))
	{
		m_delay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damagetype"))
	{
		m_damageType = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CBaseLogic::KeyValue(pkvd);
	}
}

void CTriggerHurtRemote::Spawn() {
	if ((pev->spawnflags & SF_RHURT_START_ON) && (pev->spawnflags & SF_RHURT_CONSTANT)) {
		SetThink(&CTriggerHurtRemote::ConstantThink);
		pev->nextthink = gpGlobals->time;
		m_isActive = true;
	}
}

void CTriggerHurtRemote::HurtTarget(CBaseEntity* loser) {
	if (pev->spawnflags & SF_RHURT_INSTANT_KILL) {
		if (pev->dmg >= 0) {
			loser->Killed(pev, GIB_NORMAL);
		}
		else {
			// a bug that became a feature?
			loser->pev->health = loser->pev->max_health;
		}
	}
	else {
		if (pev->spawnflags & SF_RHURT_DO_ARMOR) {
			float maxArmor = mp_startarmor.value > 100 ? mp_startarmor.value : 100;
			loser->pev->armorvalue = V_max(loser->pev->armorvalue - m_armorDmg, 0);
			loser->pev->armorvalue = V_min(maxArmor, loser->pev->armorvalue);
		}

		if (pev->dmg > 0) {
			loser->TakeDamage(pev, pev, pev->dmg, m_damageType);
		}
		else {
			loser->TakeHealth(-pev->dmg, m_damageType);
		}
		
	}
}

void CTriggerHurtRemote::HurtAllTargets() {
	if (pev->target) {
		std::vector<CBaseEntity*> targets = FindLogicEntities(STRING(pev->target));

		const char* classFilter = m_targetClass ? STRING(m_targetClass) : NULL;

		for (CBaseEntity* target : targets) {
			if (classFilter && strcmp(STRING(target->pev->classname), classFilter)) {
				continue;
			}

			HurtTarget(target);
		}
	}
	else if (m_targetClass) {
		CBaseEntity* ent = NULL;
		while ((ent = UTIL_FindEntityByClassname(ent, STRING(m_targetClass)))) {
			HurtTarget(ent);
		}
	}
}

void CTriggerHurtRemote::ConstantThink() {
	HurtAllTargets();
	pev->nextthink = gpGlobals->time + m_delay;
}

void CTriggerHurtRemote::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	if (pev->spawnflags & SF_RHURT_CONSTANT) {
		m_isActive = useType == USE_TOGGLE ? !m_isActive : useType == USE_ON;

		if (m_isActive) {
			SetThink(&CTriggerHurtRemote::ConstantThink);
			pev->nextthink = gpGlobals->time;
		}
		else {
			SetThink(NULL);
			pev->nextthink = 0;
		}
	}
	else if (useType != USE_OFF) {
		HurtAllTargets();
	}
}

