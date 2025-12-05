#include "extdll.h"
#include "util.h"
#include "CBaseLogic.h"

CBaseEntity* CBaseLogic::FindLogicEntity(string_t targetname) {
	if (!strcmp(STRING(targetname), "!activator")) {
		if (!m_hActivator) {
			EALERT(at_console, "Failed to find !activator. Using worldspawn instead.\n");
			return CBaseEntity::Instance(ENT(0));
		}

		return m_hActivator;
	}

	if (!strcmp(STRING(targetname), "!caller")) {
		if (!m_hCaller) {
			EALERT(at_console, "Failed to find !caller. Using worldspawn instead.\n");
			return CBaseEntity::Instance(ENT(0));
		}

		return m_hCaller;
	}

	return UTIL_FindEntityByTargetname(NULL, STRING(targetname));
}

std::vector<CBaseEntity*> CBaseLogic::FindLogicEntities(const char* targetName) {
	std::vector<CBaseEntity*> foundEnts;

	if (!targetName)
		return foundEnts;

	if (!strcmp(targetName, "!activator")) {
		if (m_hActivator) {
			foundEnts.push_back(m_hActivator);
		}
	}
	else if (!strcmp(targetName, "!caller")) {
		if (m_hCaller) {
			foundEnts.push_back(m_hCaller);
		}
	}
	else {
		CBaseEntity* ent = NULL;
		while ((ent = UTIL_FindEntityByTargetname(ent, targetName))) {
			if (!(ent->pev->flags & FL_KILLME)) {
				foundEnts.push_back(ent);
			}
		}
	}
	
	return foundEnts;
}

void CBaseLogic::FireLogicTargets(const char* targetName, USE_TYPE useType, float value)
{
	if (!targetName)
		return;

	ALERT(at_aiconsole, "Firing: (%s)\n", targetName);

	if (!strcmp(targetName, "!activator")) {
		if (m_hActivator) {
			FireTarget(m_hActivator, m_hActivator, this, useType, value);
		}
	}
	else if (!strcmp(targetName, "!caller")) {
		if (m_hCaller) {
			FireTarget(m_hCaller, m_hActivator, this, useType, value);
		}
	}
	else {
		CBaseEntity* ent = NULL;
		while ((ent = UTIL_FindEntityByTargetname(ent, targetName))) {
			if (ent && ent != this) {
				FireTarget(ent, m_hActivator, this, useType, value);
			}
		}
	}
}

int CBaseLogic::FloatToInteger(float f, int flt2str_mode) {
	switch (flt2str_mode) {
	case FLT2STR_ROUND: return f + 0.5f;
	case FLT2STR_CEIL: return ceilf(f);
	case FLT2STR_FLOOR:
	default:
		return f;
	}
}

float CBaseLogic::VectorToFloat(Vector v) {
	int dont_use_coords = GetVectorDontUseFlags();

	if (dont_use_coords == (SF_LOGIC_DONT_USE_Y | SF_LOGIC_DONT_USE_Z)) {
		return v.x;
	}
	if (dont_use_coords == (SF_LOGIC_DONT_USE_X | SF_LOGIC_DONT_USE_Z)) {
		return v.y;
	}
	if (dont_use_coords == (SF_LOGIC_DONT_USE_X | SF_LOGIC_DONT_USE_Y)) {
		return v.z;
	}
	
	if (dont_use_coords & SF_LOGIC_DONT_USE_X) {
		v.x = 0;
	}
	if (dont_use_coords & SF_LOGIC_DONT_USE_Y) {
		v.y = 0;
	}
	if (dont_use_coords & SF_LOGIC_DONT_USE_Z) {
		v.z = 0;
	}

	return v.Length();
}

std::string CBaseLogic::FloatToString(float f, int flt2str_mode) {
	switch (flt2str_mode) {
	case FLT2STR_5DP: return UTIL_VarArgs("%.5f", f);
	case FLT2STR_4DP: return UTIL_VarArgs("%.4f", f);
	case FLT2STR_3DP: return UTIL_VarArgs("%.3f", f);
	case FLT2STR_2DP: return UTIL_VarArgs("%.2f", f);
	case FLT2STR_1DP: return UTIL_VarArgs("%.1f", f);
	case FLT2STR_ROUND: return UTIL_VarArgs("%d", (int)(f + 0.5f));
	case FLT2STR_CEIL: return UTIL_VarArgs("%d", (int)ceilf(f));
	case FLT2STR_FLOOR: return UTIL_VarArgs("%d", (int)f);
	case FLT2STR_6DP:
	default:
		return UTIL_VarArgs("%f", f);
	}
}

std::string CBaseLogic::VectorToString(Vector v, int flt2str_mode) {
	int dont_use_coords = GetVectorDontUseFlags();

	std::string s;

	if (!(dont_use_coords & SF_LOGIC_DONT_USE_X)) {
		s += FloatToString(v.x, flt2str_mode);
	}
	if (!(dont_use_coords & SF_LOGIC_DONT_USE_Y)) {
		if (s.length()) {
			s += " ";
		}
		s += FloatToString(v.y, flt2str_mode);
	}
	if (!(dont_use_coords & SF_LOGIC_DONT_USE_Z)) {
		if (s.length()) {
			s += " ";
		}
		s += FloatToString(v.z, flt2str_mode);
	}

	return s;
}