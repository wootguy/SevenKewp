#include "extdll.h"
#include "util.h"
#include "skill.h"

//
// CTriggerSetCVar / trigger_setcvar -- set a server cvar

class CTriggerSetCVar : public CPointEntity
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	string_t m_iszCVarToChange;
};

LINK_ENTITY_TO_CLASS(trigger_setcvar, CTriggerSetCVar)

void CTriggerSetCVar::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszCVarToChange"))
	{
		m_iszCVarToChange = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CTriggerSetCVar::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	static StringSet valid_cvars = {
		"mp_falldamage",
		"mp_flashlight",
		"mp_forcerespawn",
		"mp_fraglimit",
		"mp_npckill",
		"mp_respawndelay",
		"mp_timelimit",
		"mp_weaponstay",
		"sv_accelerate",
		"sv_airaccelerate",
		"sv_friction",
		"sv_gravity",
		"sv_maxspeed",
		"sv_wateraccelerate",
		"sv_waterfriction",
	};

	static StringSet unimpl_cvars = {
		"mp_allowmonsterinfo",
		"mp_banana",
		"mp_barnacle_paralyze",
		"mp_disablegaussjump",
		"mp_disable_autoclimb",
		"mp_disable_pcbalancing",
		"mp_disable_player_rappel",
		"mp_dropweapons",
		"mp_grapple_mode",
		"mp_monsterpoints",
		"mp_noblastgibs",
		"mp_nomedkit",
		"mp_no_akimbo_uzis",
		"mp_pcbalancing_factorlist",
		"mp_weapon_respawndelay",
		"mp_ammo_respawndelay",
		"npc_dropweapons",
		"skill",
		"sv_ai_enemy_detection_mode",
	};

	if (pev->netname) {
		FireTargets(STRING(pev->netname), pActivator, this, USE_TOGGLE, 0.0f);
	}

	if (!m_iszCVarToChange) {
		ALERT(at_console, "%s (trigger_setcvar): no cvar selected\n", STRING(pev->targetname));
		return;
	}

	const char* cvarname = STRING(m_iszCVarToChange);
	cvar_t* skillCvar = GetSkillCvar(cvarname);

	if (!skillCvar) {
		if (unimpl_cvars.hasKey(cvarname)) {
			ALERT(at_error, "%s (trigger_setcvar): unimplemented cvar '%s'\n",
				STRING(pev->targetname), cvarname);
			return;
		}

		if (!valid_cvars.hasKey(cvarname)) {
			ALERT(at_error, "%s (trigger_setcvar): invalid cvar '%s'\n",
				STRING(pev->targetname), cvarname);
			return;
		}
	}

	std::string cvarvalue = sanitize_cvar_value(pev->message ? STRING(pev->message) : "");

	ALERT(at_logged, "trigger_setcvar: '%s' set to '%s'\n", cvarname, cvarvalue.c_str());

	if (skillCvar) {
		// cvar must be set now so that skill data can be refreshed this same frame
		// SERVER_COMMAND takes at least a frame to complete
		skillCvar->value = atof(cvarvalue.c_str());
		RefreshSkillData(true);
	}
	else {
		SERVER_COMMAND(UTIL_VarArgs("%s %s\n", cvarname, cvarvalue.c_str()));
	}
}
