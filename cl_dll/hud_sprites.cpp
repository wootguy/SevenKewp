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

struct HudSprite {
	hudspriteparams_t params;
	dstring_t sprFile;
	HSPRITE hspr;
	float startTime;
	float lastUpdate;
	float lastFrame;
	float frame;
	bool visible;
};

#define MAX_HUD_SPRITES 16
HudSprite g_hudSprites[MAX_HUD_SPRITES];

const char* GetDeltaString(dstring_t idx);

int CHudSprites::Init(void)
{
	gHUD.AddHudElem(this);

	HOOK_MESSAGE(HudSprite);
	HOOK_MESSAGE(HudSprTogl);

	m_iFlags |= HUD_INTERMISSION | HUD_ACTIVE; // is always drawn during an intermission

	return 1;
}

int CHudSprites::VidInit(void)
{
	memset(g_hudSprites, 0, sizeof(g_hudSprites));
	return 1;
}

int CHudSprites::Draw(float flTime)
{
	float now = gEngfuncs.GetClientTime();

	int sw = ScreenWidth;
	int sh = ScreenHeight;

	for (int i = 0; i < MAX_HUD_SPRITES; i++) {
		HudSprite& spr = g_hudSprites[i];

		if (!spr.startTime || !spr.visible)
			continue;

		float endTime = spr.startTime + spr.params.holdtime;

		if (endTime < now && spr.params.holdtime) {
			spr.startTime = 0;
			continue;
		}

		hudspriteparams_t& params = spr.params;

		// load sprite if not already
		if (!spr.hspr) {
			const char* sprPath = GetDeltaString(spr.sprFile);
			if (!sprPath || !sprPath[0])
				continue; // delta string not received yet

			const char* loadPath = UTIL_VarArgs("sprites/%s.spr", sprPath);
			spr.hspr = SPR_Load(loadPath);

			if (!spr.hspr) {
				PRINTD("Failed to load HUD sprite %s\n", loadPath);
				spr.startTime = 0; // don't keep trying
				continue;
			}

			if (!params.numframes) {
				params.numframes = SPR_Frames(spr.hspr);
			}
		}

		float timeAlive = now - spr.startTime;

		int x = params.x * sw;
		int y = params.y * sh;
		float alpha = 1.0f;

		if (params.flags & HUD_ELEM_ABSOLUTE_X) {
			x = params.x;
		}
		if (params.flags & HUD_ELEM_ABSOLUTE_Y) {
			y = params.y;
		}
		if (params.flags & HUD_ELEM_SCR_CENTER_X) {
			x += sw / 2;
		}
		if (params.flags & HUD_ELEM_SCR_CENTER_Y) {
			y += sh / 2;
		}
		if (params.flags & HUD_ELEM_NO_BORDER) {
			//PRINTD("HUD_ELEM_NO_BORDER not implemented\n");
		}
		if (params.flags & HUD_ELEM_EFFECT_ONCE) {
			// redundant?
		}
		if (params.flags & HUD_ELEM_DEFAULT_ALPHA) {
			alpha = MIN_ALPHA / 255.0f;
		}
		if (params.flags & HUD_ELEM_DYNAMIC_ALPHA) {
			// simplified health bar logic
			float t = (now - spr.lastUpdate);
			alpha = (MIN_ALPHA + V_max(0, FADE_TIME - t * 20)) / 255.0f;
		}
		
		int r = 255;
		int g = 255;
		int b = 255;

		if (params.color1.r || params.color1.g || params.color1.b ) {
			r = params.color1.r;
			g = params.color1.g;
			b = params.color1.b;
		}

		// color effect
		if (params.effect) {
			float t = normalizeRangef(timeAlive, 0, params.fxTime) / params.fxTime;

			switch (params.effect) {
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
				r = UTIL_Lerp(params.color1.r, params.color2.r, t);
				g = UTIL_Lerp(params.color1.g, params.color2.g, t);
				b = UTIL_Lerp(params.color1.b, params.color2.b, t);
			}
		}

		// fade
		if (params.fadeinTime || params.fadeoutTime) {
			float timeLeft = endTime - now;
			float fadeIn = clampf(timeAlive / params.fadeinTime, 0, 1);
			float fadeOut = clampf(timeLeft / params.fadeoutTime, 0, 1);
			alpha *= V_min(fadeOut, fadeIn);
		}

		ScaleColors(r, g, b, alpha * 255);

		// define region
		wrect_t rect;
		rect.left = params.left;
		rect.top = params.top;
		rect.right = params.left + params.width;
		rect.bottom = params.top + params.height;
		wrect_t* rc = (rect.left || rect.top || rect.right || rect.bottom) ? &rect : NULL;

		SPR_Set(spr.hspr, r, g, b);

		if (params.flags & HUD_SPR_OPAQUE) {
			if (params.flags & HUD_SPR_MASKED) {
				SPR_DrawHoles(spr.frame, x, y, rc);
			}
			else {
				SPR_Draw(spr.frame, x, y, rc);
			}
		}
		else {
			SPR_DrawAdditive(spr.frame, x, y, rc);
		}

		// animate
		float frametime = now - spr.lastFrame;
		spr.frame += frametime * params.framerate;
		if (spr.frame >= params.numframes) {
			if (params.flags & HUD_SPR_PLAY_ONCE) {
				spr.frame = params.numframes - 1;
			}
			else {
				spr.frame -= params.numframes;
			}
			if (params.flags & HUD_SPR_HIDE_WHEN_STOPPED) {
				spr.startTime = 0;
			}
		}
		spr.lastFrame = now;
	}

	return 1;
}

int CHudSprites::MsgFunc_HudSprite(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int msgfl = READ_BYTE();
	int channel = clampi(READ_BYTE(), 0, MAX_HUD_SPRITES-1);

	HudSprite& spr = g_hudSprites[channel];
	hudspriteparams_t& params = spr.params;
	memset(&spr, 0, sizeof(HudSprite));

	params.channel = channel;
	params.flags = READ_SHORT();
	params.holdtime = READ_BYTE() * 0.1f;
	params.x = READ_SHORT() / 32767.0f;
	params.y = READ_SHORT() / 32767.0f;
	spr.sprFile = READ_SHORT();

	if (msgfl & HUD_SPR_MSG_REGION) {
		params.left = READ_BYTE();
		params.top = READ_BYTE();
		params.width = READ_BYTE();
		params.height = READ_BYTE();
	}
	if (msgfl & HUD_SPR_MSG_COLOR1) {
		params.color1.r = READ_BYTE();
		params.color1.g = READ_BYTE();
		params.color1.b = READ_BYTE();
	}
	if (msgfl & HUD_SPR_MSG_COLOR2) {
		params.color2.r = READ_BYTE();
		params.color2.g = READ_BYTE();
		params.color2.b = READ_BYTE();
	}
	if (msgfl & HUD_SPR_MSG_ANIM) {
		params.frame = READ_BYTE();
		params.framerate = READ_BYTE();
		params.numframes = READ_BYTE();
	}
	if (msgfl & HUD_SPR_MSG_FADE) {
		params.fadeinTime = READ_BYTE() * 0.1f;
		params.fadeoutTime = READ_BYTE() * 0.1f;
	}
	if (msgfl & HUD_SPR_MSG_FX) {
		params.effect = READ_BYTE();
		params.fxTime = READ_BYTE() * 0.1f;
	}

	spr.startTime = gEngfuncs.GetClientTime();
	spr.lastFrame = spr.startTime;
	spr.lastUpdate = spr.startTime;
	spr.frame = params.frame;
	spr.visible = !(params.flags & HUD_ELEM_HIDDEN);

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