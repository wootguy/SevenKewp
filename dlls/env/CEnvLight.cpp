#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CLight.h"

class CEnvLight : public CLight
{
public:
	void	KeyValue(KeyValueData* pkvd);
	void	Spawn(void);
};

LINK_ENTITY_TO_CLASS(light_environment, CEnvLight)

void CEnvLight::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "_light"))
	{
		int r, g, b, v, j;
		char szColor[64];
		j = sscanf(pkvd->szValue, "%d %d %d %d\n", &r, &g, &b, &v);
		if (j == 1)
		{
			g = b = r;
		}
		else if (j == 4)
		{
			r = r * (v / 255.0);
			g = g * (v / 255.0);
			b = b * (v / 255.0);
		}

		// simulate qrad direct, ambient,and gamma adjustments, as well as engine scaling
		r = pow(r / 114.0, 0.6) * 264;
		g = pow(g / 114.0, 0.6) * 264;
		b = pow(b / 114.0, 0.6) * 264;

		pkvd->fHandled = TRUE;
		snprintf(szColor, 64, "%d", r);
		CVAR_SET_STRING("sv_skycolor_r", szColor);
		snprintf(szColor, 64, "%d", g);
		CVAR_SET_STRING("sv_skycolor_g", szColor);
		snprintf(szColor, 64, "%d", b);
		CVAR_SET_STRING("sv_skycolor_b", szColor);
	}
	else
	{
		CLight::KeyValue(pkvd);
	}
}


void CEnvLight::Spawn(void)
{
	char szVector[64];
	UTIL_MakeAimVectors(pev->angles);

	snprintf(szVector, 64, "%f", gpGlobals->v_forward.x);
	CVAR_SET_STRING("sv_skyvec_x", szVector);
	snprintf(szVector, 64, "%f", gpGlobals->v_forward.y);
	CVAR_SET_STRING("sv_skyvec_y", szVector);
	snprintf(szVector, 64, "%f", gpGlobals->v_forward.z);
	CVAR_SET_STRING("sv_skyvec_z", szVector);

	CLight::Spawn();
}
