#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h> // _alloca

#include "shared_effects.h"
#include "pmtrace.h"
#include "pm_defs.h"
#include "event_api.h"
#include "StringPool.h"

DECLARE_MESSAGE(m_HudEntStatus, StringIdx);

static int em_width;
static int em_height;

extern vec3_t v_origin;
extern vec3_t v_angles;

typedef uint16_t dstring_t;

int g_plr_look_index; // player being looked at

dstring_t g_knownStrings[65536];
StringPool g_knownStringData;

float* GetClientColor(int clientIndex);

int CHudEntStatus::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(StringIdx);

	m_iFlags |= HUD_INTERMISSION | HUD_ACTIVE; // is always drawn during an intermission

	GetConsoleStringSize("A", &em_width, &em_height);

	return 1;
}

int CHudEntStatus::VidInit(void)
{
	g_knownStringData.clear();
	memset(g_knownStrings, 0, sizeof(g_knownStrings));
	return 1;
}

void unpackStatus(cl_entity_t* ent, const char*& name, uint16_t& ihint, RGB& color, bool& invincible) {
	if (!ent)
		return;
	
	if (ent->index < 32) {
		extra_player_info_t& info = g_PlayerExtraInfo[ent->index];
		name = g_PlayerInfoList[ent->index].name;
		ihint = 0;
		invincible = false;

		float* fcolor = GetClientColor(ent->index);
		color = RGB(fcolor[0] * 255, fcolor[1] * 255, fcolor[2] * 255);
	}
	else {
		uint32_t status = ent->curstate.iuser3;
		invincible = status & (1U << 31);
		uint16_t iname = (status >> 12) & 0x3FF;
		ihint = (status >> 22) & 0x1FF;
		uint32_t icolor = status & 0xFFF;

		if (icolor) {
			color.r = ((icolor >> 8) & 0xF) * 17;
			color.g = ((icolor >> 4) & 0xF) * 17;
			color.b = ((icolor >> 0) & 0xF) * 17;
		}

		name = g_knownStringData.str(g_knownStrings[iname]);
	}
}

uint32_t GetEntHealth(cl_entity_t* ent) {
	return ent->index < 32 ? g_PlayerExtraInfo[ent->index].health
		: UTIL_DecompressUint(ent->curstate.health);
}

uint32_t GetEntArmor(cl_entity_t* ent) {
	return ent->index < 32 ? g_PlayerExtraInfo[ent->index].armor : 0;
}

std::string FormatLargeNumber(uint32_t num) {
	if (num >= 1000 * 1000 * 10) {
		return UTIL_VarArgs("%um", num / (1000U * 1000U));
	}
	else if (num >= 1000 * 10) {
		return UTIL_VarArgs("%uk", num / 1000U);
	}
	else {
		return UTIL_VarArgs("%u", num);
	}
}

const char* GetStatusString(uint32_t hp, uint32_t ap, bool invincible) {
	std::string hpStr = FormatLargeNumber(hp);

	if (invincible) {
		return "Invincible";
	}
	if (ap) {
		std::string armorStr = FormatLargeNumber(ap);
		return UTIL_VarArgs("%s HP | %s AP", hpStr.c_str(), armorStr.c_str());
	}

	return UTIL_VarArgs("%s HP", hpStr.c_str());
}

int CHudEntStatus::Draw(float flTime)
{
	if (!gHUD.m_is_map_loaded || !gHUD.IsSevenKewpServer())
		return 1;

	Vector forward, right, up;
	AngleVectors(v_angles, forward, right, up);

	cl_entity_t* localPlayer = GetLocalPlayer();

	pmtrace_t tr;
	gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
	gEngfuncs.pEventAPI->EV_PushPMStates();
	gEngfuncs.pEventAPI->EV_SetSolidPlayers(localPlayer->index - 1);
	gEngfuncs.pEventAPI->EV_SetTraceHull(2);
	gEngfuncs.pEventAPI->EV_PlayerTrace(v_origin, v_origin + forward*8192, 0, -1, &tr);
	physent_t* pPhys = gEngfuncs.pEventAPI->EV_GetPhysent(tr.ent);
	cl_entity_t* pEnt = pPhys && pPhys->info ? gEngfuncs.GetEntityByIndex(pPhys->info) : NULL;
	gEngfuncs.pEventAPI->EV_PopPMStates();

	static int viewEnt = 0;
	static float lastUpdate = 0;

	float now = gEngfuncs.GetClientTime();

	const char* name = NULL;
	RGB color;
	uint16_t ihint = 0;
	uint32_t icolor = 0;
	bool invincible;

	unpackStatus(pEnt, name, ihint, color, invincible);

	if (lastUpdate > now)
		lastUpdate = 0;

	bool mergeTags = gHUD.m_Nametags.m_HUD_nametags->value > 0
		&& gHUD.m_Nametags.m_HUD_nametag_info->value > 0;

	if (mergeTags) {
		if (pPhys->info < 32 && pPhys->info) {
			lastUpdate = now;
			g_plr_look_index = viewEnt = pPhys->info;
			return 1;
		}
	}

	if (pEnt && name) {
		lastUpdate = now;
		viewEnt = pPhys->info;
		g_plr_look_index = 0;
	}
	else if (now - lastUpdate > 1.0f || !viewEnt) {
		viewEnt = 0;
		g_plr_look_index = 0;
		return 1;
	}

	if (viewEnt && (viewEnt > 32 || !mergeTags)) {
		pEnt = gEngfuncs.GetEntityByIndex(viewEnt);

		const char* name = NULL;
		const char* hint = NULL;
		const char* info = NULL;

		if (pEnt) {
			unpackStatus(pEnt, name, ihint, color, invincible);

			hint = g_knownStringData.str(g_knownStrings[ihint]);
			uint32_t health = GetEntHealth(pEnt);
			uint32_t armor = GetEntArmor(pEnt);

			if (pEnt->curstate.messagenum < GetLocalPlayer()->curstate.messagenum) {
				// left VIS area or gibbed
			}
			else if (health) {
				info = GetStatusString(health, armor, invincible);
			}
		}

		if (!name)
			return 1;

		int x, y, w, h;
		x = gHUD.m_iFontHeight;
		y = ScreenHeight / 2;
		int centerX = ScreenWidth / 2;
		int yOffset = 0;
		bool shouldCenter = gHUD.m_StatusBar.m_HUD_centerid->value;

		if (shouldCenter) {
			y += (gHUD.m_StatusBar.m_HUD_centerid->value - 1) * em_height;
		}
		else {
			y = ScreenHeight - (gHUD.m_iFontHeight + em_height*4);
		}

		DrawConsoleString(0, 0, ""); // fix first console string being colored by HUD text

		if (shouldCenter) {
			GetConsoleStringSize(name, &w, &h);
			x = centerX - (w / 2);
		}
		if (color.r || color.g || color.b)
			DrawConsoleString(x, y, name, color);
		else
			DrawConsoleString(x, y, name);

		if (info) {
			if (shouldCenter) {
				GetConsoleStringSize(info, &w, &h);
				x = centerX - (w / 2);
			}
			
			DrawConsoleString(x, y + em_height, info);

			if (hint) {
				if (shouldCenter) {
					GetConsoleStringSize(hint, &w, &h);
					x = centerX - (w / 2);
				}
				
				DrawConsoleString(x, y + em_height * 2, hint, RGB(200, 200, 200));
			}
		}
	}

	return 1;
}

int CHudEntStatus::MsgFunc_StringIdx(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	uint16_t idx = READ_SHORT();
	const char* str = READ_STRING();

	dstring_t ofs = g_knownStringData.alloc(str).offset;
	g_knownStrings[idx] = ofs;

	return 1;
}
