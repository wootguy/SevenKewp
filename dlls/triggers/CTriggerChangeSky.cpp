#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "CBaseDMStart.h"

//
// CTriggerChangeSky / trigger_changesky -- changes the sky textures and lighting at runtime

#define SF_RPLR_REUSABLE 1

#define SKYBOX_MODEL_PATH "models/skybox"

// minimum distance needed to render the fake skybox (bigass model)
#define SKYBOX_MIN_ZMAX 2097152

class CTriggerChangeSky : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	string_t m_skyname;
	Vector m_color;
};

LINK_ENTITY_TO_CLASS(trigger_changesky, CTriggerChangeSky)

void CTriggerChangeSky::Spawn(void)
{
	Precache();
	UTIL_SetOrigin(pev, pev->origin);

	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;

	// using a bsp because it isn't affected by world lighting and renders everywhere
	if (m_skyname) {
		SET_MODEL(ENT(pev), UTIL_VarArgs(SKYBOX_MODEL_PATH "/%s.bsp", STRING(m_skyname)));
	}

	if (CVAR_GET_FLOAT("sv_zmax") < SKYBOX_MIN_ZMAX) {
		ALERT(at_console, "trigger_changesky: increased sv_zmax to %d for skybox rendering\n", SKYBOX_MIN_ZMAX);
		CVAR_SET_FLOAT("sv_zmax", SKYBOX_MIN_ZMAX);
	}
}

void CTriggerChangeSky::Precache()
{
	if (m_skyname) {
		PRECACHE_MODEL(UTIL_VarArgs(SKYBOX_MODEL_PATH "/%s.bsp", STRING(m_skyname)));
	}
}

void CTriggerChangeSky::KeyValue(KeyValueData* pkvd) {
	if (FStrEq(pkvd->szKeyName, "skyname"))
	{
		if (strlen(pkvd->szValue))
			m_skyname = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "color"))
	{
		UTIL_StringToVector(m_color, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue(pkvd);
	}
}

void CTriggerChangeSky::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// the engine syncs sky lighting to the client in realtime
	CVAR_SET_FLOAT("sv_skycolor_r", m_color.x);
	CVAR_SET_FLOAT("sv_skycolor_g", m_color.y);
	CVAR_SET_FLOAT("sv_skycolor_b", m_color.z);

	if (!m_skyname) {
		// no sky name set, just update light colors
		return;
	}

	// sv_skyname is also synced but clients currently in the server have to rejoin to see it.
	// So, a giant model will be drawn in front of the sky instead.
	// TODO: You can force the client to unload a sky by sending SVC_RESOURCELIST but
	//       I don't see any way to force them to load the new one without a full rejoin.
	//       It is possible to force a rejoin with rehlds but it's not worth it.
	//CVAR_SET_STRING("sv_skyname", STRING(m_skyname));

	// turn off all skyboxes
	CBaseEntity* skyent = NULL;
	while (!FNullEnt(skyent = UTIL_FindEntityByClassname(skyent, "trigger_changesky"))) {
		skyent->pev->effects |= EF_NODRAW;
	}

	// ...except this one, unless using the default sky
	if (!strcmp(STRING(m_skyname), CVAR_GET_STRING("sv_skyname"))) {
		pev->effects |= EF_NODRAW;
		ALERT(at_console, "trigger_changesky: activated sky %s (default)\n", STRING(m_skyname));
	}
	else {
		pev->effects = 0;
		ALERT(at_console, "trigger_changesky: activated sky %s\n", STRING(m_skyname));
	}
}
