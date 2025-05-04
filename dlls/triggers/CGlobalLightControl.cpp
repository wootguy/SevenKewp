#include "extdll.h"
#include "util.h"
#include "CBaseLogic.h"
#include "hlds_hooks.h"
#include "CLight.h"

//
// CGlobalLightControl / global_light_control -- changes light style values/patterns

// apply to all lights that have a custom style
#define SF_GLIT_AFFECT_CUSTOM_STYLES 65536

// don't apply to non-styled lights
#define SF_GLIT_DONT_AFFECT_DEFAULT 131072

class CGlobalLightControl : public CPointEntity
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Spawn();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	string_t m_pattern;
	string_t m_targetEnt;
};

LINK_ENTITY_TO_CLASS(global_light_control, CGlobalLightControl)

void CGlobalLightControl::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pattern"))
	{
		m_pattern = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "target_entity"))
	{
		m_targetEnt = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CPointEntity::KeyValue(pkvd);
	}
}

void CGlobalLightControl::Spawn()
{
	// start on if it can't be toggled
	if (!pev->targetname) {
		Use(this, this, USE_TOGGLE, 0.0f);
	}
}

void CGlobalLightControl::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	const char* pattern = m_pattern ? STRING(m_pattern) : "m";

	if (m_targetEnt) {
		// pretty sure this is impossible in HL. You can't change the lightsytyle for a specific light
		// because the lighting is controlled by style numbers in the BSP faces, not light entity
		// "style" keys. If there's only one light using a specific style, then this will work.

		// future workaround: add new styles to the BSP faces in bspguy, then toggle styles on/off
		// using the entity. Or maybe add a duplicate light with a unique style key and toggle that
		// after regenerating lighting with HLRAD.

		CBaseEntity* target = NULL;
		
		while ((target = UTIL_FindEntityByTargetname(target, STRING(m_targetEnt)))) {
			CLight* light = target->MyLightPointer();
			if (light) {
				LIGHT_STYLE(light->m_iStyle, pattern);
			}
		}
	}
	else {
		if (pev->spawnflags & SF_GLIT_AFFECT_CUSTOM_STYLES) {
			for (int i = 1; i < MAX_LIGHTSTYLE_PATTERNS; i++) {
				LIGHT_STYLE(i, pattern);
			}
		}

		if (!(pev->spawnflags & SF_GLIT_DONT_AFFECT_DEFAULT)) {
			LIGHT_STYLE(0, pattern);
		}
	}
}
