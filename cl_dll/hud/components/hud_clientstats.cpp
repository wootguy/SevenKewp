#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h> // _alloca

#include "engine_pv.h"

#define MAX_FPS_HIST 8

extern float* GetClientColor(int clientIndex);

int CHudClientStats::Init(void)
{
	gHUD.AddHudElem(this);
	
	m_HUD_rspeeds = gEngfuncs.pfnRegisterVariable("cl_rspeeds", "0", FCVAR_ARCHIVE);

	int height;
	gEngfuncs.pfnDrawConsoleStringLen("000.0 ms", &m_msMaxLen, &height);
	gEngfuncs.pfnDrawConsoleStringLen("00000000 wpoly", &m_wpolyMaxLen, &height);
	gEngfuncs.pfnDrawConsoleStringLen("00000000 epoly", &m_epolyMaxLen, &height);
	m_textColor = RGB(229, 229, 178);

	m_iFlags |= HUD_INTERMISSION | HUD_ACTIVE; // is always drawn during an intermission

	return 1;
}

int CHudClientStats::Draw(float flTime)
{
	if (m_HUD_rspeeds->value <= 0)
		return 0;

	static double lastCall;
	static double msHistory[MAX_FPS_HIST];
	static int msHistoryIdx;

	double now = gEngfuncs.GetAbsoluteTime();
	double delta = now - lastCall;
	lastCall = now;

	msHistory[msHistoryIdx++ % MAX_FPS_HIST] = delta;

	double avgMs = 0;
	for (int i = 0; i < MAX_FPS_HIST; i++) {
		avgMs += msHistory[i];
	}
	int fps = 1.0f / (avgMs / MAX_FPS_HIST) + 0.5f;
	float ms = (avgMs / MAX_FPS_HIST) * 1000.0f;

	static int height;
	static char szFps[32]; int fpsLen;
	static char szMs[32]; int msLen;
	static char szWpoly[32]; int wpolyLen;
	static char szEpoly[32]; int epolyLen;
	safe_sprintf(szFps, 32, "%4d fps", fps);
	safe_sprintf(szMs, 32, "%2.1f ms", ms);
	safe_sprintf(szWpoly, 32, "%d wpoly", *g_enginepv.r_wpoly);
	safe_sprintf(szEpoly, 32, "%d epoly", *g_enginepv.r_epoly);
	
	gEngfuncs.pfnDrawConsoleStringLen(szMs, &msLen, &height);
	gEngfuncs.pfnDrawConsoleStringLen(szWpoly, &wpolyLen, &height);
	gEngfuncs.pfnDrawConsoleStringLen(szEpoly, &epolyLen, &height);
	gEngfuncs.pfnDrawConsoleStringLen(szFps, &fpsLen, &height);
	
	const int steamFpsOverlayWidth = 50;
	int offset = steamFpsOverlayWidth + 2;
	DrawConsoleString(ScreenWidth - (offset + epolyLen), 2, szEpoly, m_textColor);
	offset += m_epolyMaxLen;
	DrawConsoleString(ScreenWidth - (offset + wpolyLen), 2, szWpoly, m_textColor);
	offset += m_wpolyMaxLen;
	DrawConsoleString(ScreenWidth - (offset + msLen), 2, szMs, m_textColor);
	offset += m_msMaxLen;
	DrawConsoleString(ScreenWidth - (offset + fpsLen), 2, szFps, m_textColor);
	return 1;
}
