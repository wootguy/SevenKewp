#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "event_api.h"
#include "pm_defs.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h> // _alloca
#include "com_weapons.h"
#include "wc_params.h"
#include "ammohistory.h"

extern local_state_t g_prediction_debug_state;

static int line_height = 0;
static int var_width = 0;
static int num_width = 0;

bool VerboseDebugEnabled() {
	return gHUD.m_Debug.m_HUD_debug->value >= 2;
}

int CHudDebug::Init(void)
{
	gHUD.AddHudElem(this);

	m_HUD_debug = gEngfuncs.pfnRegisterVariable("cl_debug", "0", FCVAR_ARCHIVE);

	m_iFlags |= HUD_INTERMISSION | HUD_ACTIVE; // is always drawn during an intermission

	GetConsoleStringSize("m_fInSpecialReload  ", &var_width, &line_height);
	GetConsoleStringSize("-32768  ", &num_width, &line_height);

	return 1;
}

#define PRINT_VARF(name, var) \
	DrawConsoleString(5, yOffset, name, varColor); \
	DrawConsoleString(5+var_width, yOffset, UTIL_VarArgs(" % .2f", var), valColor); \
	yOffset += line_height

#define PRINT_VARD(name, var) \
	DrawConsoleString(5, yOffset, name, varColor); \
	DrawConsoleString(5+var_width, yOffset, UTIL_VarArgs(" %d", var), valColor); \
	yOffset += line_height

#define PRINT_VARSTR(name, var) \
	DrawConsoleString(5, yOffset, name, varColor); \
	DrawConsoleString(5+var_width, yOffset, UTIL_VarArgs(" %s", var), valColor); \
	yOffset += line_height

#define PRINT_VARDSTR(name, var, num) \
	DrawConsoleString(5, yOffset, name, varColor); \
	DrawConsoleString(5+var_width, yOffset, UTIL_VarArgs(" %s (%d)", var, num), valColor); \
	yOffset += line_height

#define PRINT_VEC_VARF(name, vec) \
	DrawConsoleString(5, yOffset, name, varColor); \
	DrawConsoleString(5+var_width, yOffset, UTIL_VarArgs(" % .2f % .2f % .2f", vec.x, vec.y, vec.z), valColor); \
	yOffset += line_height

#define PRINT_VEC_VARD(name, vec) \
	DrawConsoleString(5, yOffset, name, varColor); \
	DrawConsoleString(5+var_width, yOffset, UTIL_VarArgs("% d", (int)vec.x), valColor); \
	DrawConsoleString(5+var_width+num_width, yOffset, UTIL_VarArgs("% d", (int)vec.y), valColor); \
	DrawConsoleString(5+var_width+num_width*2, yOffset, UTIL_VarArgs("% d", (int)vec.z), valColor); \
	yOffset += line_height

int CHudDebug::Draw(float flTime)
{
	if (m_HUD_debug->value <= 0 || !gHUD.IsSevenKewpServer())
		return 0;

	int yOffset = line_height * 2;

	local_state_t& state = g_prediction_debug_state;

	if (state.client.m_iId < 0 || state.client.m_iId > MAX_WEAPONS) {
		return 0;
	}

	weapon_data_t& wep = state.weapondata[state.client.m_iId];
	RGB valColor(0, 255, 0);
	RGB varColor(255, 255, 255);

	CustomWeaponParams* params = GetCustomWeaponParams(state.client.m_iId, WC_PARAMS_AUTO);
	bool isCustomWeapon = IsCustomWeapon(state.client.m_iId);

	int ammoType1 = (int)state.client.vuser4[0];
	int ammoAmt1 = (int)state.client.vuser4[1];
	int ammoType2 = (int)state.client.vuser3[2];
	int ammoAmt2 = (int)state.client.vuser4[2];

	std::string ammo1 = UTIL_VarArgs("%-4d| %-4d (Type %d)", wep.m_iClip, ammoAmt1, ammoType1);
	std::string ammo2 = UTIL_VarArgs("%-4d| %-4d (Type %d)", wep.iuser4, ammoAmt2, ammoType2);
	Vector nextAttacks(wep.m_flNextPrimaryAttack, wep.m_flNextSecondaryAttack, wep.fuser4);

	WEAPON* pw = gWR.GetWeapon(wep.m_iId);
	std::string hudWepInfo = UTIL_VarArgs("%s (ID %d)", pw->szName, wep.m_iId);

	float accuracyX = pw->accuracyX;
	float accuracyY = pw->accuracyY;
	float accuracyX2 = pw->accuracyX2;
	float accuracyY2 = pw->accuracyY2;
	bool dynamicAccuracy = pw->iFlagsEx & WEP_FLAG_DYNAMIC_ACCURACY;
	GetCurrentCustomWeaponAccuracy(pw->iId, accuracyX, accuracyY, accuracyX2, accuracyY2, dynamicAccuracy);
	std::string accuracy1 = UTIL_VarArgs("% .2f  % .2f", accuracyX, accuracyY);
	std::string accuracy2 = UTIL_VarArgs("% .2f  % .2f", accuracyX2, accuracyY2);

	std::string flagStr;
	if (pw->iFlags & ITEM_FLAG_SELECTONEMPTY) flagStr += "Empty select + ";
	if (pw->iFlags & ITEM_FLAG_NOAUTORELOAD) flagStr += "No auto reload + ";
	if (pw->iFlags & ITEM_FLAG_NOAUTOSWITCHEMPTY) flagStr += "No switch empty + ";
	if (pw->iFlags & ITEM_FLAG_LIMITINWORLD) flagStr += "Limit in world + ";
	if (pw->iFlags & ITEM_FLAG_EXHAUSTIBLE) flagStr += "Exhaustible + ";
	if (pw->iFlags & ITEM_FLAG_NOAUTOSWITCHTO) flagStr += "No switch to + ";
	if (pw->iFlagsEx & WEP_FLAG_DYNAMIC_ACCURACY) flagStr += "Dyn. acc + ";
	if (pw->iFlagsEx & WEP_FLAG_SECONDARY_ACCURACY) flagStr += "2nd acc + ";
	if (pw->iFlagsEx & WEP_FLAG_VERTICAL_ACCURACY) flagStr += "Vert. acc + ";
	if (pw->iFlagsEx & WEP_FLAG_USE_ZOOM_CROSSHAIR) flagStr += "Zoom xhair + ";
	if (pw->iFlagsEx & WEP_FLAG_NO_ZOOM_CROSSHAIR) flagStr += "No zoom xhair + ";
	flagStr = flagStr.substr(0, flagStr.size() - 3);

	PRINT_VARSTR("Weapon", hudWepInfo.c_str());
	PRINT_VARSTR("Weapon Flags", flagStr.c_str());
	yOffset += line_height;
	PRINT_VARSTR("Primary Accuracy", accuracy1.c_str());
	PRINT_VARSTR("Secondary Accuracy", accuracy2.c_str());
	yOffset += line_height;
	PRINT_VARSTR("Primary Ammo", ammo1.c_str());
	PRINT_VARSTR("Secondary Ammo", ammo2.c_str());
	if (isCustomWeapon) {
		PRINT_VARD("Akimbo Clip", wep.iuser1);
	}
	yOffset += line_height;
	PRINT_VEC_VARF("Attack Cooldowns", nextAttacks);
	PRINT_VARF("Idle Cooldown", wep.m_flTimeWeaponIdle);

	yOffset += line_height;
	
	if (isCustomWeapon) {
		const char* m_fInSpecialReload = "???";
		switch (wep.m_fInSpecialReload) {
		case WC_RELOAD_STAGE_START: m_fInSpecialReload = "Start"; break;
		case WC_RELOAD_STAGE_START_EMPTY: m_fInSpecialReload = "Start (empty)"; break;
		case WC_RELOAD_STAGE_SHELL: m_fInSpecialReload = "Shell"; break;
		case WC_RELOAD_STAGE_PUMP: m_fInSpecialReload = "Pump"; break;
		case WC_RELOAD_STAGE_SECONDARY: m_fInSpecialReload = "Secondary"; break;
		case WC_RELOAD_STAGE_AKIMBO: m_fInSpecialReload = "Akimbo"; break;
		}

		PRINT_VARDSTR("Reloading", wep.m_fInReload ? "Yes" : "No", wep.m_fInReload);
		PRINT_VARDSTR("Reload Stage", m_fInSpecialReload, wep.m_fInSpecialReload);
	}
	else {
		PRINT_VARD("m_fInReload", wep.m_fInReload);
		PRINT_VARD("m_fInSpecialReload", wep.m_fInSpecialReload);
		PRINT_VARF("m_flNextReload", wep.m_flNextReload);
		PRINT_VARF("m_flPumpTime", wep.m_flPumpTime);
		PRINT_VARF("m_fReloadTime", wep.m_fReloadTime);
	}

	yOffset += line_height;
	if (isCustomWeapon) {
		const char* iuser3 = GetCustomWeaponStateString();
		PRINT_VARDSTR("Weapon State", iuser3, wep.iuser3);
		PRINT_VARSTR("Charge States", GetCustomWeaponChargeStatesString());

		int shotsFired = wep.m_fInZoom & 0xf;
		int iDirection = (wep.m_fInZoom >> 4) & 1;
		int delayFire = (wep.m_fInZoom >> 5) & 1;
		float nextDec = wep.m_fNextAimBonus;
		std::string str = UTIL_VarArgs("%d %.2f %s %s (%d)", shotsFired, nextDec,
			iDirection ? "<" : ">", delayFire ? "D" : "", wep.m_fInZoom);
		PRINT_VARSTR("Shots Fired", str.c_str());
	}
	else {
		PRINT_VARF("m_fNextAimBonus", wep.m_fNextAimBonus);
		PRINT_VARD("m_fInZoom", wep.m_fInZoom);
		PRINT_VARD("m_iWeaponState", wep.m_iWeaponState);
		PRINT_VARD("m_fireState", wep.iuser3);
		PRINT_VARF("m_flStartThrow", wep.fuser2);
		PRINT_VARF("m_flReleaseThrow", wep.fuser3);
		PRINT_VARD("m_fInAttack", wep.iuser2);
		PRINT_VARD("m_chargeReady", wep.iuser1);
		PRINT_VARF("fuser1", wep.fuser1);
	}
	
	yOffset += line_height;
	Vector camAngles = Vector(gPlayerSim.cam_ofs[0], gPlayerSim.cam_ofs[1], 0);
	camAngles = gPlayerSim.cam_thirdperson ? camAngles : gPlayerSim.v_angles;
	PRINT_VEC_VARD("Player Angles", gPlayerSim.v_angles);
	PRINT_VEC_VARD("Camera Angles", camAngles);
	PRINT_VEC_VARF("Client Punch", gPlayerSim.ev_punchangle);
	PRINT_VEC_VARF("Server Punch", gPlayerSim.sv_punchangle);

	yOffset += line_height;
	Vector speeds = Vector(gPlayerSim.v_sim_vel.Length(), gPlayerSim.v_sim_vel.Length2D(), gPlayerSim.v_sim_vel.z);
	PRINT_VEC_VARD("Player Origin", gPlayerSim.v_sim_org);
	PRINT_VEC_VARD("Camera Origin", gPlayerSim.v_origin);
	PRINT_VEC_VARD("Speed (3D/2D/UP)", speeds);	

	yOffset += line_height;
	int real_contents;
	int contents = gEngfuncs.PM_PointContents(gPlayerSim.v_sim_org, &real_contents);

	const char* contentsName = "???";
	switch (contents) {
	case CONTENTS_EMPTY: contentsName = "EMPTY"; break;
	case CONTENTS_SOLID: contentsName = "SOLID"; break;
	case CONTENTS_WATER: contentsName = "WATER"; break;
	case CONTENTS_SLIME: contentsName = "SLIME"; break;
	case CONTENTS_LAVA: contentsName = "LAVA"; break;
	case CONTENTS_SKY: contentsName = "SKY"; break;
	case CONTENTS_ORIGIN: contentsName = "ORIGIN"; break;
	case CONTENTS_CLIP: contentsName = "CLIP"; break;
	case CONTENTS_CURRENT_0: contentsName = "CURRENT_0"; break;
	case CONTENTS_CURRENT_90: contentsName = "CURRENT_90"; break;
	case CONTENTS_CURRENT_180: contentsName = "CURRENT_180"; break;
	case CONTENTS_CURRENT_270: contentsName = "CURRENT_270"; break;
	case CONTENTS_CURRENT_UP: contentsName = "CURRENT_UP"; break;
	case CONTENTS_CURRENT_DOWN: contentsName = "CURRENT_DOWN"; break;
	case CONTENTS_TRANSLUCENT: contentsName = "TRANSLUCENT"; break;
	case CONTENTS_LADDER: contentsName = "LADDER"; break;
	case CONTENTS_FLYFIELD: contentsName = "FLYFIELD"; break;
	case CONTENTS_GRAVITY_FLYFIELD: contentsName = "GRAVITY_FLYFIELD"; break;
	case CONTENTS_FOG: contentsName = "FOG"; break;
	}
	PRINT_VARDSTR("Contents", contentsName, contents);

	if (m_HUD_debug->value >= 2) {
		static int numPhysEnts = 0;
		static int drawCount = 0;
		static float lastDraw = 0;
		if (gEngfuncs.GetClientTime() - lastDraw > 0.055f || lastDraw > gEngfuncs.GetClientTime()) {
			lastDraw = gEngfuncs.GetClientTime();
			numPhysEnts = 0;

			const int maxBoxes = 15; // more than this many boxes per frame and boxes won't be drawn
			int drawMin = (drawCount % 8) * maxBoxes;
			int drawMax = (drawCount % 8) * maxBoxes + maxBoxes;

			for (int i = 1; i < MAX_PHYSENTS; i++) {
				physent_t* test = gEngfuncs.pEventAPI->EV_GetPhysent(i);
				if (!test)
					continue;

				Vector min = test->mins + test->origin;
				Vector max = test->maxs + test->origin;
				Vector ori = test->origin + min + (max - min) * 0.5f;

				if (numPhysEnts >= drawMin && numPhysEnts < drawMax) {
					RGB color;

					switch (test->solid) {
					default:
					case SOLID_NOT:
						color = RGB(255, 0, 255); // should never happen?
						break;
					case SOLID_TRIGGER:
						color = RGB(128, 0, 255);
						break;
					case SOLID_BBOX:
						color = RGB(255, 128, 0);
						break;
					case SOLID_SLIDEBOX:
						color = RGB(0, 255, 0);
						break;
					case SOLID_BSP:
						color = RGB(255, 0, 0);
						break;
					}

					te_debug_box(min, max, 0.5f, color);
					//te_debug_beam(gPlayerSim.v_sim_org, ori, 1, RGB(255, 255, 0));
				}

				numPhysEnts++;
			}

			drawCount++;
		}
		PRINT_VARD("Phys Ents", numPhysEnts);
	}


	return 1;
}
