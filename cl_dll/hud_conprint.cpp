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

DECLARE_MESSAGE(m_HudConPrint, HudConPrint);

struct HudConText {
	hudconparms_t params;
	char message[256];
	float startTime;
};

#define USER_HUD_CON_TEXTS 64
#define SYSTEM_CON_TEXTS 4
#define TOTAL_HUD_CON_TEXTS (USER_HUD_CON_TEXTS + SYSTEM_CON_TEXTS)
HudConText g_hudConText[TOTAL_HUD_CON_TEXTS];
int g_hudConTextIdx = 0;

int em_width;
int em_height;

extern vec3_t v_origin;

HudConText& AllocHudConText(int id, bool userRequest) {
	int idx = id;

	if (userRequest) {
		int half = USER_HUD_CON_TEXTS / 2;

		if (id == 0) {
			idx = ((g_hudConTextIdx++) % half) + half;
		}
		else {
			idx = clampi(idx, 0, half);
		}
	}
	else {
		id = (id + USER_HUD_CON_TEXTS) % SYSTEM_CON_TEXTS;
	}
	
	memset(&g_hudConText[idx], 0, sizeof(HudConText));
	return g_hudConText[idx];
}

HudConText& GetHudConText(int id, bool userRequest) {
	int idx = id;

	if (userRequest) {
		int half = USER_HUD_CON_TEXTS / 2;
		idx = clampi(idx, 0, half);
	}
	else {
		id = (id + USER_HUD_CON_TEXTS) % SYSTEM_CON_TEXTS;
	}

	memset(&g_hudConText[idx], 0, sizeof(HudConText));
	return g_hudConText[idx];
}

int CHudConPrint::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(HudConPrint);

	m_iFlags |= HUD_INTERMISSION | HUD_ACTIVE; // is always drawn during an intermission

	GetConsoleStringSize("A", &em_width, &em_height);

	return 1;
}

int CHudConPrint::VidInit(void)
{
	memset(g_hudConText, 0, sizeof(g_hudConText));
	return 1;
}

int CHudConPrint::Draw(float flTime)
{
	float now = gEngfuncs.GetClientTime();

	int sw = ScreenWidth;
	int sh = ScreenHeight;

	cl_entity_t* localPlayer = GetLocalPlayer();

	int rDefault, gDefault, bDefault;
	UnpackRGB(rDefault, gDefault, bDefault, gHUD.GetHudColor());

	for (int i = 0; i < TOTAL_HUD_CON_TEXTS; i++) {
		HudConText& txt = g_hudConText[i];

		if (!txt.startTime)
			continue;

		if (txt.startTime + txt.params.holdTime * 0.001f < now) {
			txt.startTime = 0;
			continue;
		}

		hudconparms_t& params = txt.params;

		int x = params.xPixels + params.xPercent*sw + params.xEm*em_width;
		int y = params.yPixels + params.yPercent*sh + params.yEm*em_height;

		if ((params.flags & FL_HUDCON_WORLD) || params.attachEnt) {
			Vector worldPos(params.xWorld, params.yWorld, params.zWorld);

			if (params.attachEnt) {
				cl_entity_s* pEnt = gEngfuncs.GetEntityByIndex(params.attachEnt);
				
				if (!pEnt || pEnt->curstate.messagenum < localPlayer->curstate.messagenum)
					continue;

				worldPos = pEnt->origin + worldPos;
			}

			Vector screenPos = WorldToScreen(worldPos);

			if (screenPos.z < 0) {
				continue;
			}

			if (!(params.flags & FL_HUDCON_XRAY)) {
				pmtrace_t tr;
				gEngfuncs.pEventAPI->EV_SetTraceHull(2);
				gEngfuncs.pEventAPI->EV_PlayerTrace(v_origin, worldPos, PM_STUDIO_IGNORE | PM_GLASS_IGNORE, -1, &tr);

				if (tr.fraction < 1.0f) {
					continue;
				}
			}

			x += screenPos.x;
			y += screenPos.y;
		}

		int r, g, b;
		
		if (params.r || params.g || params.b) {
			r = params.r;
			g = params.g;
			b = params.b;
		}
		else {
			r = rDefault;
			g = gDefault;
			b = bDefault;
		}

		RGB color(r, g, b);
		const char* msg = txt.message;

		static char buf[256];
		int lastLineStart = 0;

		int k = 0;
		int lineNum = 0;
		while (1) {
			if (msg[k] == '\n' || !msg[k]) {
				strcpy_safe(buf, msg, k+1);
				buf[k] = 0;

				int dx = x;
				int dy = y + lineNum * em_height;
				if (params.alignment) {
					int w, h;
					GetConsoleStringSize(buf, &w, &h);

					if (params.alignment == HUDCON_ALIGN_CENTER) {
						dx -= w / 2;
					}
					else if (params.alignment == HUDCON_ALIGN_RIGHT) {
						dx -= w;
					}
				}

				DrawConsoleString(dx, dy, buf, color);

				if (!msg[k])
					break;

				msg += k+1;
				k = 0;
				lineNum++;
				continue;
			}

			k++;
		}
	}

	return 1;
}

int CHudConPrint::MsgFunc_HudConPrint(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int msgfl = READ_BYTE();

	hudconparms_t params;
	memset(&params, 0, sizeof(hudconparms_t));

	if (msgfl & FL_HUDCON_MSG_PERCENT) {
		params.xPercent = (uint16_t)READ_SHORT() / 65535.0f;
		params.yPercent = (uint16_t)READ_SHORT() / 65535.0f;
	}
	if (msgfl & FL_HUDCON_MSG_EM) {
		params.xEm = READ_SHORT() / 100.0f;
		params.yEm = READ_SHORT() / 100.0f;
	}
	if (msgfl & FL_HUDCON_MSG_PIXELS) {
		params.xPixels = READ_SHORT();
		params.yPixels = READ_SHORT();
	}
	if (msgfl & FL_HUDCON_MSG_WORLD) {
		params.xWorld = READ_SHORT();
		params.yWorld = READ_SHORT();
		params.zWorld = READ_SHORT();
	}
	if (msgfl & FL_HUDCON_MSG_ENT) {
		params.attachEnt = READ_SHORT();
	}

	params.r = READ_BYTE();
	params.g = READ_BYTE();
	params.b = READ_BYTE();
	params.holdTime = READ_SHORT();
	params.id = READ_BYTE();

	uint8_t packedFlags = READ_BYTE();
	params.alignment = packedFlags & 0x3;
	params.flags = packedFlags >> 2;

	HudConText& txt = AllocHudConText(params.id, true);
	txt.params = params;
	txt.startTime = gEngfuncs.GetClientTime();

	strcpy_safe(txt.message, READ_STRING(), sizeof(txt.message));

	return 1;
}
