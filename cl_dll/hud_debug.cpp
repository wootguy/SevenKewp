#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h> // _alloca

extern local_state_t g_prediction_debug_state;

static int line_height = 0;
static int var_width = 0;

int CHudDebug::Init(void)
{
	gHUD.AddHudElem(this);

	m_HUD_debug = gEngfuncs.pfnRegisterVariable("cl_debug", "0", FCVAR_ARCHIVE);

	m_iFlags |= HUD_INTERMISSION | HUD_ACTIVE; // is always drawn during an intermission

	GetConsoleStringSize("m_flNextSecondaryAttack  ", &var_width, &line_height);

	return 1;
}

#define PRINT_WEP_STATE_VARF(var) \
	DrawConsoleString(5, yOffset, #var, varColor); \
	DrawConsoleString(5+var_width, yOffset, UTIL_VarArgs(" %f", wep.var), valColor); \
	yOffset += line_height

#define PRINT_WEP_STATE_VARD(var) \
	DrawConsoleString(5, yOffset, #var, varColor); \
	DrawConsoleString(5+var_width, yOffset, UTIL_VarArgs(" %d", wep.var), valColor); \
	yOffset += line_height

int CHudDebug::Draw(float flTime)
{
	if (m_HUD_debug->value <= 0)
		return 0;

	int yOffset = line_height*4;

	local_state_t& state = g_prediction_debug_state;

	if (state.client.m_iId < 0 || state.client.m_iId > MAX_WEAPONS) {
		return 0;
	}

	weapon_data_t& wep = state.weapondata[state.client.m_iId];
	RGB valColor(0, 255, 0);
	RGB varColor(255, 255, 255);

	PRINT_WEP_STATE_VARD(m_iId);
	PRINT_WEP_STATE_VARD(m_iClip);
	yOffset += line_height;
	PRINT_WEP_STATE_VARF(m_flNextPrimaryAttack);
	PRINT_WEP_STATE_VARF(m_flNextSecondaryAttack);
	PRINT_WEP_STATE_VARF(m_flTimeWeaponIdle);
	yOffset += line_height;
	PRINT_WEP_STATE_VARD(m_fInReload);
	PRINT_WEP_STATE_VARD(m_fInSpecialReload);
	PRINT_WEP_STATE_VARF(m_flNextReload);
	PRINT_WEP_STATE_VARF(m_flPumpTime);
	PRINT_WEP_STATE_VARF(m_fReloadTime);
	yOffset += line_height;
	PRINT_WEP_STATE_VARF(m_fReloadTime);
	PRINT_WEP_STATE_VARF(m_fNextAimBonus);
	PRINT_WEP_STATE_VARD(m_fInZoom);
	PRINT_WEP_STATE_VARD(m_iWeaponState);
	yOffset += line_height;
	PRINT_WEP_STATE_VARD(iuser1);
	PRINT_WEP_STATE_VARD(iuser2);
	PRINT_WEP_STATE_VARD(iuser3);
	PRINT_WEP_STATE_VARD(iuser4);
	PRINT_WEP_STATE_VARF(fuser1);
	PRINT_WEP_STATE_VARF(fuser2);
	PRINT_WEP_STATE_VARF(fuser3);
	PRINT_WEP_STATE_VARF(fuser4);
	
	return 1;
}
