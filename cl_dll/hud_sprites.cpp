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
#include "string_deltas.h"

DECLARE_MESSAGE(m_HudSprites, HudSprite);
DECLARE_MESSAGE(m_HudSprites, HudSprTogl);
DECLARE_MESSAGE(m_HudSprites, HudUpdNum);

struct HudSprite {
	hudelementparams_t hud;
	hudspriteparams_t spr;
	hudnumparams_t num;
	dstring_t sprName; // either a literal file path or a name to lookup in hud.txt
	HSPRITE hspr; // for custom sprite
	wrect_t hrect; // region loaded from hud.txt
	HSPRITE hspr_num[10]; // for numeric displays
	wrect_t hspr_rc[10]; // for numeric displays
	float startTime;
	float lastUpdate;
	float lastFrame;
	float frame;
	bool visible;
	bool isNumeric;
};

#define MAX_HUD_SPRITES 16
HudSprite g_hudSprites[MAX_HUD_SPRITES];

const char* GetDeltaString(dstring_t idx);

int CHudSprites::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(HudSprite);
	HOOK_MESSAGE(HudSprTogl);
	HOOK_MESSAGE(HudUpdNum);

	m_iFlags |= HUD_INTERMISSION | HUD_ACTIVE; // is always drawn during an intermission

	return 1;
}

int CHudSprites::VidInit(void)
{
	memset(g_hudSprites, 0, sizeof(g_hudSprites));
	return 1;
}

bool loadHudElementSprites(HudSprite& spr) {
	if (spr.hspr)
		return true;

	hudelementparams_t& hud = spr.hud;
	
	if (spr.isNumeric) {
		for (int i = 0; i < 10; i++) {
			spr.hspr_num[i] = gHUD.GetSprite(gHUD.m_HUD_number_0 + i);
			spr.hspr_rc[i] = gHUD.GetSpriteRect(gHUD.m_HUD_number_0 + i);
		}
		return true;
	}
	
	hudspriteparams_t& params = spr.spr;

	// load sprite if not already
	if (!spr.hspr) {
		const char* sprPath = GetDeltaString(spr.sprName);
		if (!sprPath || !sprPath[0])
			return false; // delta string not received yet

		if (hud.flags & HUD_SPR_USE_CONFIG) {
			// name is an icon set to look up in hud.txt
			int index = gHUD.GetSpriteIndex(sprPath);

			if (index < 0) {
				PRINTD("Failed to find loaded HUD sprite %s\n", sprPath);
				spr.startTime = 0; // don't keep trying
				return false;
			}

			spr.hspr = gHUD.GetSprite(index);
			spr.hrect = gHUD.GetSpriteRect(index);
		}
		else {
			// name is a file path
			const char* loadPath = UTIL_VarArgs("sprites/%s.spr", sprPath);
			spr.hspr = SPR_Load(loadPath);
			spr.hrect.left = params.left;
			spr.hrect.top = params.top;
			spr.hrect.right = params.left + params.width;
			spr.hrect.bottom = params.top + params.height;

			if (!spr.hspr) {
				PRINTD("Failed to load HUD sprite %s\n", loadPath);
				spr.startTime = 0; // don't keep trying
				return false;
			}
		}

		if (!params.numframes) {
			params.numframes = SPR_Frames(spr.hspr);
		}
	}

	return true;
}

void GetHudElementColor(HudSprite& spr, int& r, int& g, int& b) {
	hudelementparams_t& hud = spr.hud;

	r = 255;
	g = 255;
	b = 255;
	float a = 1.0f;

	if (hud.color1.r || hud.color1.g || hud.color1.b) {
		r = hud.color1.r;
		g = hud.color1.g;
		b = hud.color1.b;
	}

	float now = gEngfuncs.GetClientTime();

	if (hud.flags & HUD_ELEM_DEFAULT_ALPHA) {
		a = MIN_ALPHA / 255.0f;
	}
	if (hud.flags & HUD_ELEM_DYNAMIC_ALPHA) {
		// simplified health bar logic
		float t = (now - spr.lastUpdate);
		a = (MIN_ALPHA + V_max(0, FADE_TIME - t * 20)) / 255.0f;
	}

	// color effect
	if (hud.effect) {
		float timeAlive = now - spr.startTime;
		float t = normalizeRangef(timeAlive, 0, hud.fxTime) / hud.fxTime;

		switch (hud.effect) {
		case HUD_EFFECT_TOGGLE:
			t = t > 0.5f ? 1.0f : 0;
			break;
		case HUD_EFFECT_RAMP_UP:
			t = t;
			break;
		case HUD_EFFECT_RAMP_DOWN:
			t = 1.0f - t;
			break;
		case HUD_EFFECT_TRIANGLE:
			t = (t > 0.5f ? 1.0f - t : t) * 2;
			break;
		case HUD_EFFECT_COSINE_UP:
			t = (1.0f - cosf(t * M_PI)) * 0.5f;
			break;
		case HUD_EFFECT_COSINE_DOWN:
			t = (1.0f + cosf(t * M_PI)) * 0.5f;
			break;
		case HUD_EFFECT_COSINE:
			t = 0.5f * (1.0f - cosf(t * 2.0f * M_PI));
			break;
		case HUD_EFFECT_SINE_PULSE:
			t = (sinf(t * 2.0f * M_PI) + 1.0f) * 0.5f;
			break;
		default:
			t = -1; // not a color ramping effect
			break;
		}

		if (t >= 0) {
			r = UTIL_Lerp(hud.color1.r, hud.color2.r, t);
			g = UTIL_Lerp(hud.color1.g, hud.color2.g, t);
			b = UTIL_Lerp(hud.color1.b, hud.color2.b, t);
		}
	}

	// fade
	if (hud.fadeinTime || hud.fadeoutTime) {
		float endTime = spr.startTime + spr.hud.holdTime;
		float timeAlive = now - spr.startTime;
		float timeLeft = endTime - now;
		float fadeIn = clampf(timeAlive / hud.fadeinTime, 0, 1);
		float fadeOut = clampf(timeLeft / hud.fadeoutTime, 0, 1);
		a *= V_min(fadeOut, fadeIn);
	}

	ScaleColors(r, g, b, a * 255);
}

void GetHudElementPosition(HudSprite& spr, int& x, int& y) {
	hudelementparams_t& hud = spr.hud;
	
	int sw = ScreenWidth;
	int sh = ScreenHeight;

	float pixelScale = gHUD.GetHudPixelScale();

	x = hud.x * sw + hud.xPixels * pixelScale;
	y = hud.y * sh + hud.yPixels * pixelScale;
	float alpha = 1.0f;

	if (hud.flags & HUD_ELEM_NO_BORDER) {
		//PRINTD("HUD_ELEM_NO_BORDER not implemented\n");
	}
}

void DrawHudSprite(HudSprite& spr, int x, int y, int r, int g, int b) {
	hudelementparams_t& hud = spr.hud;
	hudspriteparams_t& params = spr.spr;

	// define region
	wrect_t rect = spr.hrect;

	if (hud.flags & HUD_SPR_USE_CONFIG) {
		// selecting a sub-region of the default region
		if (params.left || params.top || params.width || params.height) {
			float scale = gHUD.GetHudPixelScale();
			rect.left += params.left * scale;
			rect.top += params.top * scale;
			rect.right = rect.left + params.width * scale;
			rect.bottom = rect.top + params.height * scale;
		}
	}

	wrect_t* rc = (rect.left || rect.top || rect.right || rect.bottom) ? &rect : NULL;

	SPR_Set(spr.hspr, r, g, b);

	if (hud.flags & HUD_SPR_OPAQUE) {
		if (hud.flags & HUD_SPR_MASKED) {
			SPR_DrawHoles(spr.frame, x, y, rc);
		}
		else {
			SPR_Draw(spr.frame, x, y, rc);
		}
	}
	else {
		SPR_DrawAdditive(spr.frame, x, y, rc);
	}

	float now = gEngfuncs.GetClientTime();

	// animate
	float frametime = now - spr.lastFrame;
	spr.frame += frametime * params.framerate;
	if (spr.frame >= params.numframes) {
		if (hud.flags & HUD_SPR_PLAY_ONCE) {
			spr.frame = params.numframes - 1;
		}
		else {
			spr.frame -= params.numframes;
		}
		if (hud.flags & HUD_SPR_HIDE_WHEN_STOPPED) {
			spr.startTime = 0;
		}
	}
	spr.lastFrame = now;
}

void DrawNumericHud(HudSprite& spr, int x, int y, int r, int g, int b) {

	if ((spr.hud.flags & HUD_NUM_DONT_DRAW_ZERO) && (int)spr.num.value == 0)
		return;

	char formatter[16];

	const char* padding0 = (spr.hud.flags & HUD_NUM_LEADING_ZEROS) ? "0" : "";
	const char* alignment = (spr.hud.flags & HUD_NUM_RIGHT_ALIGN) ? "" : "-";
	const char* plus = (spr.hud.flags & HUD_NUM_PLUS_SIGN) ? "+" : "";
	snprintf(formatter, 16, "%%%s%s%s%dd", alignment, plus, padding0, spr.num.defdigits);

	char* numStr = UTIL_VarArgs(formatter, (int)spr.num.value);
	int digitsLeft = 255;
	if (spr.num.maxdigits)
		digitsLeft = spr.num.maxdigits;

	while (*numStr && digitsLeft) {
		char c = *numStr;
		wrect_t rect = spr.hspr_rc[0];

		if (c == '+' || c == '-') {
			if (!(spr.hud.flags & HUD_NUM_NEGATIVE_NUMBERS)) {

			}
			// TODO: make hud sprites for +/-
		}
		if (c >= '0' && c <= '9') {
			int idx = c - '0';
			rect = spr.hspr_rc[idx];
			SPR_Set(spr.hspr_num[idx], r, g, b);
			SPR_DrawAdditive(0, x, y, &rect);
		}

		x += rect.right - rect.left;
		
		digitsLeft--;
		numStr++;
	}

	if (spr.hud.flags & HUD_NUM_SEPARATOR) {
		int charWidth = spr.hspr_rc[0].right - spr.hspr_rc[0].left;
		int iHeight = gHUD.m_iFontHeight;
		int iWidth = charWidth / 10;
		FillRGBA(x + charWidth / 2, y, iWidth, iHeight, r, g, b, 255);
	}
}

int CHudSprites::Draw(float flTime)
{
	float now = gEngfuncs.GetClientTime();

	for (int i = 0; i < MAX_HUD_SPRITES; i++) {
		HudSprite& spr = g_hudSprites[i];

		if (!spr.startTime || !spr.visible)
			continue;

		float endTime = spr.startTime + spr.hud.holdTime;

		if (endTime < now && spr.hud.holdTime) {
			spr.startTime = 0;
			continue;
		}

		if (!loadHudElementSprites(spr)) {
			continue;
		}

		int x, y, r, g, b;
		GetHudElementPosition(spr, x, y);
		GetHudElementColor(spr, r, g, b);

		if (spr.isNumeric) {
			DrawNumericHud(spr, x, y, r, g, b);
		}
		else {
			DrawHudSprite(spr, x, y, r, g, b);
		}
	}

	return 1;
}

HudSprite& ParseHudElementParams(int channel) {
	HudSprite& spr = g_hudSprites[channel];
	hudelementparams_t& params = spr.hud;
	memset(&spr, 0, sizeof(HudSprite));

	uint8_t msgfl = READ_BYTE();
	params.channel = channel;
	params.flags = READ_SHORT();
	params.flags |= ((uint32_t)READ_BYTE()) << 16;
	params.holdTime = READ_BYTE() * 0.1f;
	spr.sprName = READ_SHORT();
	
	if (msgfl & HUD_ELEM_MSG_SCREEN_POS) {
		params.x = READ_SHORT() / 32767.0f;
		params.y = READ_SHORT() / 32767.0f;
	}
	if (msgfl & HUD_ELEM_MSG_PIXEL_POS) {
		params.xPixels = READ_SHORT();
		params.yPixels = READ_SHORT();
	}
	if (msgfl & HUD_ELEM_MSG_COLOR1) {
		params.color1.r = READ_BYTE();
		params.color1.g = READ_BYTE();
		params.color1.b = READ_BYTE();
	}
	if (msgfl & HUD_ELEM_MSG_COLOR2) {
		params.color2.r = READ_BYTE();
		params.color2.g = READ_BYTE();
		params.color2.b = READ_BYTE();
	}
	if (msgfl & HUD_ELEM_MSG_FADE) {
		params.fadeinTime = READ_BYTE() * 0.1f;
		params.fadeoutTime = READ_BYTE() * 0.1f;
	}
	if (msgfl & HUD_ELEM_MSG_FX) {
		params.effect = READ_BYTE();
		params.fxTime = READ_BYTE() * 0.1f;
	}

	spr.startTime = gEngfuncs.GetClientTime();
	spr.lastFrame = spr.startTime;
	spr.lastUpdate = spr.startTime;
	spr.visible = !(params.flags & HUD_ELEM_HIDDEN);
	spr.isNumeric = params.flags & HUD_ELEM_IS_NUMERIC;

	return spr;
}

void ParseCustomSpriteParams(HudSprite& spr) {
	hudspriteparams_t& params = spr.spr;

	uint8_t msgfl = READ_BYTE();

	if (msgfl & HUD_SPR_MSG_REGION) {
		params.left = READ_BYTE();
		params.top = READ_BYTE();
		params.width = READ_BYTE();
		params.height = READ_BYTE();
	}
	if (msgfl & HUD_SPR_MSG_ANIM) {
		params.frame = READ_BYTE();
		params.framerate = READ_BYTE();
		params.numframes = READ_BYTE();
	}

	spr.frame = params.frame;
}

void ParseNumericDisplayParams(HudSprite& spr) {
	hudnumparams_t& params = spr.num;

	params.value = READ_LONG();
	params.defdigits = READ_BYTE();
	params.maxdigits = READ_BYTE();
}

int CHudSprites::MsgFunc_HudSprite(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int channel = clampi(READ_BYTE(), 0, MAX_HUD_SPRITES-1);

	HudSprite& spr = ParseHudElementParams(channel);
	
	if (spr.isNumeric) {
		ParseNumericDisplayParams(spr);
	}
	else {
		ParseCustomSpriteParams(spr);
	}

	return 1;
}

int CHudSprites::MsgFunc_HudSprTogl(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	uint8_t val = READ_BYTE();
	int channel = val & 0xf;
	bool visible = val >> 4;
	
	g_hudSprites[channel].visible = visible;

	return 1;
}

int CHudSprites::MsgFunc_HudUpdNum(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	uint8_t channel = READ_BYTE();
	g_hudSprites[channel].num.value = READ_LONG();

	return 1;
}