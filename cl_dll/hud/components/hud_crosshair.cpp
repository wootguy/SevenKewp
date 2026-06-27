#include "hud.h"
#include "cl_util.h"
#include "com_weapons.h"
#include "event_api.h"
#include "triangleapi.h"
#include "ammohistory.h"
#include "wc_params.h"

// for updating in prediction code
void UpdateZoomCrosshair(int id, bool zoom) {
	gHUD.m_Crosshair.UpdateZoomCrosshair(id, zoom, true);
}

int CHudCrosshair::Init(void)
{
	gHUD.AddHudElem(this);

	Reset();

	m_hud_crosshair_mode = gEngfuncs.pfnRegisterVariable("hud_crosshair_mode", "1", FCVAR_ARCHIVE);
	m_hud_crosshair_length = gEngfuncs.pfnRegisterVariable("hud_crosshair_length", "-1", FCVAR_ARCHIVE);
	m_hud_crosshair_width = gEngfuncs.pfnRegisterVariable("hud_crosshair_width", "-1", FCVAR_ARCHIVE);
	m_hud_crosshair_border = gEngfuncs.pfnRegisterVariable("hud_crosshair_border", "1", FCVAR_ARCHIVE);
	m_hud_crosshair_color = gEngfuncs.pfnRegisterVariable("hud_crosshair_color", "0", FCVAR_ARCHIVE);
	m_hud_crosshair_tee = gEngfuncs.pfnRegisterVariable("hud_crosshair_tee", "0", FCVAR_ARCHIVE);
	m_hud_crosshair_dot = gEngfuncs.pfnRegisterVariable("hud_crosshair_dot", "1", FCVAR_ARCHIVE);

	m_iFlags |= HUD_ACTIVE; //!!!

	return 1;
};

void CHudCrosshair::Reset(void)
{
	m_iFlags |= HUD_ACTIVE;
}

bool CHudCrosshair::IsWeaponZoomed() {
	return gHUD.m_iFOV < 90 || (gHUD.m_is_map_loaded && IsPredictionWeaponZoomed());
}

void CHudCrosshair::UpdateZoomCrosshair(int id, bool zoom, bool autoaimOnTarget) {
	if (id < 1)
		return;

	WEAPON* pWeapon = gWR.GetWeapon(id);

	if (!pWeapon)
		return;

	if (!zoom)
	{ // normal crosshairs
		if (autoaimOnTarget && pWeapon->hAutoaim)
			SetCrosshair(pWeapon->hAutoaim, pWeapon->rcAutoaim, 255, 255, 255);
		else
			SetCrosshair(pWeapon->hCrosshair, pWeapon->rcCrosshair, 255, 255, 255);
	}
	else
	{ // zoomed crosshairs
		if (autoaimOnTarget && pWeapon->hZoomedAutoaim)
			SetCrosshair(pWeapon->hZoomedAutoaim, pWeapon->rcZoomedAutoaim, 255, 255, 255);
		else
			SetCrosshair(pWeapon->hZoomedCrosshair, pWeapon->rcZoomedCrosshair, 255, 255, 255);
	}
}

int CHudCrosshair::Draw(float flTime)
{
	DrawDynamicCrosshair();

	WEAPON* pw = g_pActiveWeapon;

	if (!pw)
		return 0;

	// re-enable static crosshair without having to switch weapons
	static int oldCvar = 0;
	int newCvar = m_hud_crosshair_mode->value;
	if (oldCvar != newCvar) {
		if (!(gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)) && pw)
			SetCrosshair(pw->hCrosshair, pw->rcCrosshair, 255, 255, 255);
		oldCvar = newCvar;
	}

	if (!(gHUD.m_iWeaponBits & (1ULL << (WEAPON_SUIT))))
		return 1;

	if ((gHUD.m_iHideHUDDisplay & (HIDEHUD_WEAPONS | HIDEHUD_ALL)))
		return 1;

	return 1;
}

void CHudCrosshair::DrawStretchedZoomCrosshair(HSPRITE spr, wrect_t rect, bool aspectCorrection) {
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	int sw = ScreenWidth;
	int sh = ScreenHeight;

	int minX = 0;
	int minY = 0;
	int maxX = sw;
	int maxY = sh;

	if (aspectCorrection) {
		if (sw - w < sh - h) {
			float scale = sw / (float)w;
			w *= scale;
			h *= scale;

			int border = (sh - h) / 2;
			minY = border;
			maxY = sh - border;

			gEngfuncs.pfnFillRGBABlend(0, 0, sw, border, 0, 0, 0, 255);
			gEngfuncs.pfnFillRGBABlend(0, sh - border, sw, border, 0, 0, 0, 255);
		}
		else {
			float scale = sh / (float)h;
			w *= scale;
			h *= scale;

			int border = (sw - w) / 2;
			minX = border;
			maxX = sw - border;

			gEngfuncs.pfnFillRGBABlend(0, 0, border, sh, 0, 0, 0, 255);
			gEngfuncs.pfnFillRGBABlend(sw - border, 0, border, sh, 0, 0, 0, 255);
		}
	}

	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);
	gEngfuncs.pTriAPI->SpriteTexture((model_t*)gEngfuncs.GetSpritePointer(spr), 0);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);

	gEngfuncs.pTriAPI->TexCoord2f(0.0f, 1.0f);
	gEngfuncs.pTriAPI->Vertex3f(minX, minY, 0);

	gEngfuncs.pTriAPI->TexCoord2f(0.0f, 0.0f);
	gEngfuncs.pTriAPI->Vertex3f(minX, maxY, 0);

	gEngfuncs.pTriAPI->TexCoord2f(1.0f, 0.0f);
	gEngfuncs.pTriAPI->Vertex3f(maxX, maxY, 0);

	gEngfuncs.pTriAPI->TexCoord2f(1.0f, 1.0f);
	gEngfuncs.pTriAPI->Vertex3f(maxX, minY, 0);

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
}

// converts an accuracy in degrees to a pixel gap for the crosshair
int CHudCrosshair::CrosshairGapPixels(float accuracyDeg, bool isVertical) {
	int screenW = ScreenWidth;
	int screenH = ScreenHeight;

	float aspect = (float)screenW / (float)screenH;
	float baseAspect = 4.0f / 3.0f; // GoldSrc aspect used in FOV calculation

	// convert GoldSrc fov to real FOV
	float fovXRad43 = gHUD.m_iFOV * (M_PI / 180.0f);
	float fovXRad = 2.0f * atan(tan(fovXRad43 * 0.5f) * (aspect / baseAspect));
	float fovYRad = 2.0f * atan(tan(fovXRad * 0.5f) / aspect);

	// accuracy angle in radians
	float accRad = accuracyDeg * 0.5f * (M_PI / 180.0f);
	float spread = tan(accRad); // for VECTOR_CONE_* math

	if (isVertical) {
		return (screenW * 0.5f) * (tan(accRad) / tan(fovXRad * 0.5f));
	}
	else {
		return (screenH * 0.5f) * (tan(accRad) / tan(fovYRad * 0.5f));
	}
}

void CHudCrosshair::DrawCrossHair(float accuracyX, float accuracyY, int len, int thick, int border,
	int r, int g, int b, bool drawDot, bool drawTee) {
	int centerX = ScreenWidth / 2;
	int centerY = ScreenHeight / 2;

	int gapX = 10;
	int gapY = 10;
	int hthick = thick / 2;

	int minGap = thick + border * 4;

	if (!drawDot) {
		minGap = thick + border * 2;
	}

	gapX = V_max(CrosshairGapPixels(accuracyX, false), minGap);
	gapY = V_max(CrosshairGapPixels(accuracyY, true), minGap);

	if (gapX == minGap && gapY == minGap) {
		//len *= 0.75f;
	}
	int a = 200;


	if (border > 0) {
		int blen = len + border * 2;
		int bthick = thick + border * 2;
		int bhthick = bthick / 2;
		int rb = 0;
		int gb = 0;
		int bb = 0;

		// horizontal
		gEngfuncs.pfnFillRGBABlend(centerX - (gapX + blen - border), centerY - bhthick, blen, bthick, rb, gb, bb, a);
		gEngfuncs.pfnFillRGBABlend(centerX + gapX - border, centerY - bhthick, blen, bthick, rb, gb, bb, a);

		// vertical
		if (!drawTee)
			gEngfuncs.pfnFillRGBABlend(centerX - bhthick, centerY - (gapY + blen - border), bthick, blen, rb, gb, bb, a);
		gEngfuncs.pfnFillRGBABlend(centerX - bhthick, centerY + gapY - border, bthick, blen, rb, gb, bb, a);

		if (drawDot) {
			// center dot
			gEngfuncs.pfnFillRGBABlend(centerX - bhthick, centerY - bhthick, bthick, bthick, rb, gb, bb, a);
			gEngfuncs.pfnFillRGBABlend(centerX - bhthick, centerY - bhthick, bthick, bthick, rb, gb, bb, a);
		}
	}

	// horizontal
	gEngfuncs.pfnFillRGBABlend(centerX - (gapX + len), centerY - hthick, len, thick, r, g, b, a);
	gEngfuncs.pfnFillRGBABlend(centerX + gapX, centerY - hthick, len, thick, r, g, b, a);

	// vertical
	if (!drawTee)
		gEngfuncs.pfnFillRGBABlend(centerX - hthick, centerY - (gapY + len), thick, len, r, g, b, a);
	gEngfuncs.pfnFillRGBABlend(centerX - hthick, centerY + gapY, thick, len, r, g, b, a);

	if (drawDot) {
		// center dot
		gEngfuncs.pfnFillRGBABlend(centerX - hthick, centerY - hthick, thick, thick, r, g, b, a);
		gEngfuncs.pfnFillRGBABlend(centerX - hthick, centerY - hthick, thick, thick, r, g, b, a);
	}
}

void CHudCrosshair::DrawDynamicCrosshair() {
	WEAPON* pw = g_pActiveWeapon;

	if (pw) {
		CustomWeaponParams* wcparams = GetCustomWeaponParams(pw->iId, WC_PARAMS_AUTO);
		bool shouldDrawZoomCrosshair = pw->hZoomedCrosshair && IsWeaponZoomed() && (pw->iFlagsEx & WEP_FLAG_USE_ZOOM_CROSSHAIR);
		bool shouldStretchZoom = wcparams && (wcparams->flags & FL_WC_WEP_ZOOM_SPR_STRETCH);
		bool useIronSights = (gHUD.m_is_map_loaded && IsPredictionWeaponZoomed()) && (pw->iFlagsEx & WEP_FLAG_NO_ZOOM_CROSSHAIR);

		if (useIronSights) {
			if (g_crosshair_active) {
				static wrect_t nullrc;
				SetCrosshair(0, nullrc, 0, 0, 0);
			}

			return;
		}

		if (shouldDrawZoomCrosshair && shouldStretchZoom && !is_software_renderer) {
			static wrect_t nullrc;
			SetCrosshair(0, nullrc, 0, 0, 0);

			bool aspectCorrection = wcparams && wcparams->flags & FL_WC_WEP_ZOOM_SPR_ASPECT;
			DrawStretchedZoomCrosshair(pw->hZoomedCrosshair, pw->rcZoomedCrosshair, aspectCorrection);
			return;
		}

		if (shouldDrawZoomCrosshair) {
			return; // draw the sprite version of the crosshair for zooming
		}
	}

	if (m_hud_crosshair_mode->value <= 0 || !gHUD.IsCompatibleSevenKewpServer())
		return; // drawing sprite crosshair

	int len = clamp(m_hud_crosshair_length->value, 1, 1000);
	int width = clamp(m_hud_crosshair_width->value, 1, 1000);
	int border = clamp(m_hud_crosshair_border->value, 0, 1000);
	bool drawDot = m_hud_crosshair_dot->value > 0;
	bool drawTee = m_hud_crosshair_tee->value > 0;

	if (m_hud_crosshair_length->value == -1) {
		len = 8;
		if (ScreenHeight > 1440) {
			len = 15;
		}
		else if (ScreenHeight > 768) {
			len = 12;
		}
	}

	if (m_hud_crosshair_width->value == -1) {
		// auto size
		width = 2;
		if (ScreenHeight <= 768) {
			width = 1;
		}
	}

	int r, g, b;
	if (!UTIL_ParseHexColor(m_hud_crosshair_color->string, r, g, b)) {
		UnpackRGB(r, g, b, gHUD.GetHudColor());
	}

	// crosshair for not holding anything
	if (!pw) {
		if (m_hud_crosshair_mode->value >= 2) {
			DrawCrossHair(0, 0, len, width, border, r, g, b, drawDot, drawTee);
		}
		return;
	}

	// weapon doesn't have a crosshair
	if (!pw->hCrosshair && m_hud_crosshair_mode->value != 2) {
		return;
	}

	// disable sprite crosshair so it doesn't overlap the dynamic one
	if (g_crosshair_active) {
		static wrect_t nullrc;
		SetCrosshair(0, nullrc, 0, 0, 0);
	}

	float accuracyX = pw->accuracyX;
	float accuracyY = pw->accuracyY;
	float accuracyX2 = pw->accuracyX2;
	float accuracyY2 = pw->accuracyY2;
	bool dynamicAccuracy = pw->iFlagsEx & WEP_FLAG_DYNAMIC_ACCURACY;

	GetCurrentCustomWeaponAccuracy(pw->iId, accuracyX, accuracyY, accuracyX2, accuracyY2, dynamicAccuracy);

	float now = gEngfuncs.GetClientTime();
	if (g_last_attack_mode == 2 && (now - g_last_attack_time) < 2.0f) {
		accuracyX = accuracyX2;
		accuracyY = accuracyY2;
	}

	if (g_last_attack_time > now) {
		g_last_attack_time = 0; // level changed
	}

	// lerp between accuracy changes
	{
		static int lastWeaponId;
		static float lastAccuracyX;
		static float lastAccuracyY;
		static float nextAccuracyX;
		static float nextAccuracyY;
		static float lerpStart;

		if (lastWeaponId != pw->iId) {
			lastWeaponId = pw->iId;
			lastAccuracyX = accuracyX;
			lastAccuracyY = accuracyY;
			nextAccuracyX = accuracyX;
			nextAccuracyY = accuracyY;
			lerpStart = now;
		}

		if (accuracyX != nextAccuracyX || accuracyY != nextAccuracyY) {
			lastAccuracyX = nextAccuracyX;
			lastAccuracyY = nextAccuracyY;
			nextAccuracyX = accuracyX;
			nextAccuracyY = accuracyY;
			lerpStart = now;
		}

		if (lerpStart > now) {
			lerpStart = now; // level changed
		}

		float t = V_min(0.1f, (now - lerpStart)) / 0.1f;
		accuracyX = lastAccuracyX + (nextAccuracyX - lastAccuracyX) * t;
		accuracyY = lastAccuracyY + (nextAccuracyY - lastAccuracyY) * t;
		accuracyX2 = accuracyX;
		accuracyY2 = accuracyY;
	}


	float punch = V_max(fabs(gPlayerSim.v_punchangle[0]), fabs(gPlayerSim.v_punchangle[1]));
	if (punch > 0) {
		punch = powf(punch, 0.5f);
		accuracyX += punch;
		accuracyY += punch;
		accuracyX2 += punch;
		accuracyY2 += punch;
	}

	DrawCrossHair(accuracyX, accuracyY, len, width, border, r, g, b, drawDot, drawTee);
}
