#include "CRuleEntity.h"

enum EffectMode {
	TEF_NO_CHANGE,
	TEF_ADD,
	TEF_SUBTRACT
};

class EXPORT CTriggerEffect : public CBaseLogic
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	int m_glow_mode;
	Vector m_glow;		// glow color. (0,0,0) = no glow

	int m_block_weapons_mode;
	int m_invulnerable_mode;
	int m_invisible_mode;
	int m_nonsolid_mode;

	int m_respiration_mode;
	float m_respiration;	// additional/removed time for drowning

	int m_friction_mode;
	float m_friction;	// percent friction changed

	int m_gravity_mode;
	float m_gravity;		// percent gravity changed
	
	int m_speed_mode;
	float m_speed;		// percent speed changed

	int m_damage_mode;
	float m_damage;		// player weapon damage percent changed
};

LINK_ENTITY_TO_CLASS(trigger_effect, CTriggerEffect)

void CTriggerEffect::Spawn(void) {
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
}

void CTriggerEffect::KeyValue(KeyValueData* pkvd) {
	if (FStrEq(pkvd->szKeyName, "effect_glow"))
	{
		UTIL_StringToVector(m_glow, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_glow_mode"))
	{
		m_glow_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_block_weapons_mode"))
	{
		m_block_weapons_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_invulnerable_mode"))
	{
		m_invulnerable_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_invisible_mode"))
	{
		m_invisible_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_nonsolid_mode"))
	{
		m_nonsolid_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_respiration"))
	{
		m_respiration = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_respiration_mode"))
	{
		m_respiration_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_friction"))
	{
		m_friction = atof(pkvd->szValue) / 100.0f;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_friction_mode"))
	{
		m_friction_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_gravity"))
	{
		m_gravity = atof(pkvd->szValue) / 100.0f;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_gravity_mode"))
	{
		m_gravity_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_speed"))
	{
		m_speed = atof(pkvd->szValue) / 100.0f;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_speed_mode"))
	{
		m_speed_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_damage"))
	{
		m_damage = atof(pkvd->szValue) / 100.0f;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "effect_damage_mode"))
	{
		m_damage_mode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue(pkvd);
	}
}


void CTriggerEffect::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	std::vector<CBaseEntity*> targets = FindLogicEntities(STRING(pev->target));

	for (CBaseEntity* ent : targets) {
		CBaseMonster* mon = ent->MyMonsterPointer();
		if (!mon)
			continue;

		if (m_glow_mode == TEF_ADD) {
			mon->m_tef_glow = mon->m_tef_glow + m_glow;
		}
		else if (m_glow_mode == TEF_SUBTRACT) {
			mon->m_tef_glow = mon->m_tef_glow - m_glow;
		}

		if (m_block_weapons_mode) {
			mon->m_tef_block_weapons = m_block_weapons_mode == TEF_ADD;
		}

		if (m_invulnerable_mode) {
			mon->m_tef_invulnerable = m_invulnerable_mode == TEF_ADD;
		}

		if (m_invisible_mode) {
			mon->m_tef_invisible = m_invisible_mode == TEF_ADD;
		}

		if (m_nonsolid_mode) {
			mon->m_tef_nonsolid = m_nonsolid_mode == TEF_ADD;
		}

		if (m_respiration_mode == TEF_ADD) {
			mon->m_tef_respiration += m_respiration;
		}
		else if (m_respiration_mode == TEF_SUBTRACT) {
			mon->m_tef_respiration -= m_respiration;
		}

		if (m_friction_mode == TEF_ADD) {
			mon->m_tef_friction += m_friction;
		}
		else if (m_friction_mode == TEF_SUBTRACT) {
			mon->m_tef_friction -= m_friction;
		}

		if (m_gravity_mode == TEF_ADD) {
			mon->m_tef_gravity += m_gravity;
		}
		else if (m_gravity_mode == TEF_SUBTRACT) {
			mon->m_tef_gravity -= m_gravity;
		}

		if (m_speed_mode == TEF_ADD) {
			mon->m_tef_speed += m_speed;
		}
		else if (m_speed_mode == TEF_SUBTRACT) {
			mon->m_tef_speed -= m_speed;
		}

		if (m_damage_mode == TEF_ADD) {
			mon->m_tef_damage += m_damage;
		}
		else if (m_damage_mode == TEF_SUBTRACT) {
			mon->m_tef_damage -= m_damage;
		}

		mon->ApplyEffects();
	}
}