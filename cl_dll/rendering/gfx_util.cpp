#include "hud.h"
#include "cl_util.h"
#include "gfx_util.h"
#include "com_model.h"
#include "triangleapi.h"
#include "com_weapons.h"
#include "wc_params.h"
#include "GL/gl.h"

void gfx_start_colored_rendering(int polyMode, const RGBA& color) {
	gEngfuncs.pTriAPI->RenderMode(kRenderTransTexture);

	if (gHUD.m_hWhiteSprite) {
		gEngfuncs.pTriAPI->SpriteTexture((model_t*)gEngfuncs.GetSpritePointer(gHUD.m_hWhiteSprite), 0);
	}

	gEngfuncs.pTriAPI->Begin(polyMode);
	gEngfuncs.pTriAPI->Color4f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
}

void gfx_draw_cube_quads(const Vector& mins, const Vector& maxs) {
	gEngfuncs.pTriAPI->Vertex3f(mins.x, maxs.y, maxs.z);
	gEngfuncs.pTriAPI->Vertex3f(mins.x, maxs.y, mins.z);
	gEngfuncs.pTriAPI->Vertex3f(mins.x, mins.y, mins.z);
	gEngfuncs.pTriAPI->Vertex3f(mins.x, mins.y, maxs.z);

	gEngfuncs.pTriAPI->Vertex3f(maxs.x, maxs.y, mins.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, maxs.y, maxs.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, mins.y, maxs.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, mins.y, mins.z);

	gEngfuncs.pTriAPI->Vertex3f(mins.x, mins.y, mins.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, mins.y, mins.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, mins.y, maxs.z);
	gEngfuncs.pTriAPI->Vertex3f(mins.x, mins.y, maxs.z);

	gEngfuncs.pTriAPI->Vertex3f(mins.x, maxs.y, maxs.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, maxs.y, maxs.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, maxs.y, mins.z);
	gEngfuncs.pTriAPI->Vertex3f(mins.x, maxs.y, mins.z);

	gEngfuncs.pTriAPI->Vertex3f(mins.x, maxs.y, mins.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, maxs.y, mins.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, mins.y, mins.z);
	gEngfuncs.pTriAPI->Vertex3f(mins.x, mins.y, mins.z);

	gEngfuncs.pTriAPI->Vertex3f(maxs.x, maxs.y, maxs.z);
	gEngfuncs.pTriAPI->Vertex3f(mins.x, maxs.y, maxs.z);
	gEngfuncs.pTriAPI->Vertex3f(mins.x, mins.y, maxs.z);
	gEngfuncs.pTriAPI->Vertex3f(maxs.x, mins.y, maxs.z);
}

void gfx_draw_cube(const Vector& mins, const Vector& maxs, const RGBA& color) {
	gfx_start_colored_rendering(TRI_QUADS, color);
	gfx_draw_cube_quads(mins, maxs);
	gEngfuncs.pTriAPI->End();
}

void gfx_draw_line_quad(const Vector& a, const Vector& b, float width) {
	Vector edge = b - a;
	edge.Normalize();

	Vector view = gPlayerSim.v_origin - (a + b) * 0.5f;
	view = view.Normalize();

	Vector right = CrossProduct(edge, view);
	right = right.Normalize();
	right = right * width * 0.5f;

	gEngfuncs.pTriAPI->Vertex3fv((b - right));
	gEngfuncs.pTriAPI->Vertex3fv((b + right));
	gEngfuncs.pTriAPI->Vertex3fv((a + right));
	gEngfuncs.pTriAPI->Vertex3fv((a - right));
}

void gfx_draw_line(const Vector& a, const Vector& b, const RGBA& color, float width) {
	gfx_start_colored_rendering(TRI_QUADS, color);
	gfx_draw_line_quad(a, b, width);
	gEngfuncs.pTriAPI->End();
}

void gfx_draw_cube_outline(const Vector& mins, const Vector& maxs, const RGBA& color, float thickness) {
	gfx_start_colored_rendering(TRI_QUADS, color);

	Vector v000(mins.x, mins.y, mins.z);
	Vector v100(maxs.x, mins.y, mins.z);
	Vector v110(maxs.x, maxs.y, mins.z);
	Vector v010(mins.x, maxs.y, mins.z);

	Vector v001(mins.x, mins.y, maxs.z);
	Vector v101(maxs.x, mins.y, maxs.z);
	Vector v111(maxs.x, maxs.y, maxs.z);
	Vector v011(mins.x, maxs.y, maxs.z);

	gfx_draw_line_quad(v000, v100, thickness);
	gfx_draw_line_quad(v100, v110, thickness);
	gfx_draw_line_quad(v110, v010, thickness);
	gfx_draw_line_quad(v010, v000, thickness);

	gfx_draw_line_quad(v001, v101, thickness);
	gfx_draw_line_quad(v101, v111, thickness);
	gfx_draw_line_quad(v111, v011, thickness);
	gfx_draw_line_quad(v011, v001, thickness);

	gfx_draw_line_quad(v000, v001, thickness);
	gfx_draw_line_quad(v100, v101, thickness);
	gfx_draw_line_quad(v110, v111, thickness);
	gfx_draw_line_quad(v010, v011, thickness);

	gEngfuncs.pTriAPI->End();
}

Vector Unproject(float sx, float sy, Vector& forward, Vector& right, Vector& up, float fovXDeg) {
	int sw = ScreenWidth;
	int sh = ScreenHeight;

	const float dist = 12.0f; // closest a quad can be before getting clipped
	float aspect = sw / (float)sh;

	float fovXRad = fovXDeg * (M_PI / 180.0f);
	float fovX = 2.0f * atanf(tanf(fovXRad * 0.5f));
	float tanHalfFov = tanf(fovX * 0.5f);

	float ndcX = (sx / sw) * 2.0f - 1.0f;
	float ndcY = 1.0f - (sy / sh) * 2.0f;

	Vector dir = forward +
		right * (ndcX * aspect * tanHalfFov) +
		up * (ndcY * tanHalfFov);

	return gPlayerSim.v_origin + dir.Normalize() * dist;
}

void gfx_draw_fake_hud_quad(float minX, float minY, float maxX, float maxY) {
	{
		// No idea why this happens, but the sprite needs shifting vertically depending on resolution.
		// There doesn't seem to be a formula to calculate this either. It's some combination of 
		// screen height and aspect ratio that affects this.
		// TODO: the tri api has GetMatrix and ScreenToWorld which might help unprojecting
		int sw = ScreenWidth;
		int sh = ScreenHeight + SOFTWARE_MODE_BLACK_BAR_HEIGHT;
		int shift = 80;
		if (sw == 720 && sh == 480)	shift = 56;
		else if (sw == 720 && sh == 576)	shift = 64;
		else if (sw == 1176 && sh == 576)	shift = 79;
		else if (sw == 1280 && sh == 720)	shift = 86;
		else if (sw == 1280 && sh == 768)	shift = 92;
		else if (sw == 1360 && sh == 768)	shift = 92;
		else if (sw == 1366 && sh == 768)	shift = 92;
		else if (sw == 1280 && sh == 800)	shift = 96;
		else if (sw == 1280 && sh == 1024)	shift = 108;
		else if (sw == 1440 && sh == 900)	shift = 109;
		else if (sw == 640 && sh == 480)	shift = 59;
		else if (sw == 800 && sh == 600)	shift = 80;
		else if (sw == 1024 && sh == 768)	shift = 101;
		else if (sw == 1152 && sh == 864)	shift = 113;
		else if (sw == 1280 && sh == 960)	shift = 125;
		else if (sw == 1440 && sh == 1080)	shift = 140;

		float fadeEnd = V_max(gPlayerSim.fade.fadeReset, gPlayerSim.fade.fadeEnd) - gEngfuncs.GetClientTime();
		if (gPlayerSim.waterlevel == WATERLEVEL_HEAD || fadeEnd > 0) {
			// the shift here should be different per resolution too, but idc
			shift -= 4;
		}

		minY -= shift;
		maxY -= shift;
	}

	Vector forward, right, up;
	AngleVectors(gPlayerSim.v_angles, forward, right, up);

	float fovXDeg = g_prediction.fov ? g_prediction.fov : gHUD.default_fov->value;

	Vector ul = Unproject(minX, minY, forward, right, up, fovXDeg);
	Vector ur = Unproject(maxX, minY, forward, right, up, fovXDeg);
	Vector ll = Unproject(minX, maxY, forward, right, up, fovXDeg);
	Vector lr = Unproject(maxX, maxY, forward, right, up, fovXDeg);

	gEngfuncs.pTriAPI->TexCoord2f(0, 0); gEngfuncs.pTriAPI->Vertex3fv(ul);
	gEngfuncs.pTriAPI->TexCoord2f(1, 0); gEngfuncs.pTriAPI->Vertex3fv(ur);
	gEngfuncs.pTriAPI->TexCoord2f(1, 1); gEngfuncs.pTriAPI->Vertex3fv(lr);
	gEngfuncs.pTriAPI->TexCoord2f(0, 1); gEngfuncs.pTriAPI->Vertex3fv(ll);
}

void gfx_draw_sprite_weapon() {
	if (gPlayerSim.cam_thirdperson)
		return;

	ViewModelSprite* pSpr = g_prediction.weapon.v_sprite;
	if (!pSpr || pSpr->hSprite <= 0)
		return;
	ViewModelSprite& spr = *pSpr;

	wrect_t rc;
	rc.top = 0;
	rc.left = 0;
	rc.right = spr.w;
	rc.bottom = spr.h;

	float scale = (ScreenHeight * 0.001f) * spr.scale;
	if (is_software_renderer)
		scale *= 0.70f;

	rc.right *= scale;
	rc.bottom *= scale;
	spr.x *= scale;
	spr.y *= scale;

	int minX = spr.x + (ScreenWidth - rc.right) / 2;
	int minY = spr.y + (ScreenHeight - rc.bottom);
	int maxX = minX + rc.right;
	int maxY = minY + rc.bottom;

	int triRenderMode = spr.rendermode;
	RGBA c = spr.color;

	switch (spr.rendermode) {
	case kRenderNormal:
	case kRenderTransTexture:
	case kRenderTransAlpha:
	case kRenderTransColor:
		triRenderMode = kRenderTransAlpha;
		break;
	case kRenderGlow:
	case kRenderTransAdd:
		triRenderMode = kRenderTransAdd;
		break;
	}

	gEngfuncs.pTriAPI->RenderMode(triRenderMode);
	gEngfuncs.pTriAPI->SpriteTexture((model_t*)gEngfuncs.GetSpritePointer(spr.hSprite), spr.frame);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	gEngfuncs.pTriAPI->Color4f(
		c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f
	);

	if (is_software_renderer) {
		gfx_draw_fake_hud_quad(minX, minY, maxX, maxY);
	}
	else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		gEngfuncs.pTriAPI->TexCoord2f(0.0f, 0.0f);
		gEngfuncs.pTriAPI->Vertex3f(minX, minY, 0);

		gEngfuncs.pTriAPI->TexCoord2f(0.0f, 1.0f);
		gEngfuncs.pTriAPI->Vertex3f(minX, maxY, 0);

		gEngfuncs.pTriAPI->TexCoord2f(1.0f, 1.0f);
		gEngfuncs.pTriAPI->Vertex3f(maxX, maxY, 0);

		gEngfuncs.pTriAPI->TexCoord2f(1.0f, 0.0f);
		gEngfuncs.pTriAPI->Vertex3f(maxX, minY, 0);
	}

	gEngfuncs.pTriAPI->End();
	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);

	if (spr.renderfx == kRenderFxGlowShell) {
		// Poor man's glow effect. Looks more like motion blur, but it's cheap.
		c = spr.glowShellColor;
		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		gEngfuncs.pTriAPI->Begin(TRI_QUADS);

		gEngfuncs.pTriAPI->Color4f(
			c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, 0.5f
		);

		float t = gHUD.m_flTime * 16;
		for (int i = 0; i < 2; i++) {
			float a = i == 0 ? t : -t * 0.5f;
			float r = i == 0 ? 1.0f : 1.5f;
			float dx = cosf(a) * scale * r;
			float dy = sinf(a) * scale * r;

			if (is_software_renderer) {
				gfx_draw_fake_hud_quad(minX + dx, minY + dy, maxX + dx, maxY + dy);
			}
			else {
				gEngfuncs.pTriAPI->TexCoord2f(0.0f, 0.0f);
				gEngfuncs.pTriAPI->Vertex3f(minX + dx, minY + dy, 0);

				gEngfuncs.pTriAPI->TexCoord2f(0.0f, 1.0f);
				gEngfuncs.pTriAPI->Vertex3f(minX + dx, maxY + dy, 0);

				gEngfuncs.pTriAPI->TexCoord2f(1.0f, 1.0f);
				gEngfuncs.pTriAPI->Vertex3f(maxX + dx, maxY + dy, 0);

				gEngfuncs.pTriAPI->TexCoord2f(1.0f, 0.0f);
				gEngfuncs.pTriAPI->Vertex3f(maxX + dx, minY + dy, 0);
			}
		}

		gEngfuncs.pTriAPI->End();
		gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
	}
}

Vector WorldToScreen(const Vector& P) {
	Vector screen;
	gEngfuncs.pTriAPI->WorldToScreen((float*)&P[0], screen);

	int w = ScreenWidth / 2;
	int h = ScreenHeight / 2;

	Vector delta = P - gPlayerSim.v_origin;
	float depth = DotProduct(delta, gPlayerSim.cam_forward);

	return Vector(screen.x * w + w, screen.y * -h + h, depth);

	// below also works and was used before discovering the API.
	/*
	Vector forward, right, up;
	AngleVectors(viewerAngles, forward, right, up);

	Vector rel = P - viewerOrigin;

	float x = DotProduct(rel, right);
	float y = DotProduct(rel, up);
	float z = DotProduct(rel, forward);

	int screenW = ScreenWidth;
	int screenH = ScreenHeight;
	float aspect = (float)screenW / (float)screenH;

	// convert to actual horizontal FOV for this aspect
	float baseAspect = 4.0f / 3.0f;
	float fovXRad43 = fovXDeg * (M_PI / 180.0f);
	float fovXRad = 2.0f * atan(tan(fovXRad43 * 0.5f) * (aspect / baseAspect));
	float fovYRad = 2.0f * atan(tan(fovXRad * 0.5f) / aspect);
	float f = 1.0f / tan(fovYRad * 0.5f);

	float ndcX = (x * f / aspect) / z;
	float ndcY = (y * f) / z;

	float screenX = (ndcX + 1.0f) * 0.5f * screenW;
	float screenY = (1.0f - ndcY) * 0.5f * screenH;

	return { screenX, screenY, z };
	*/
}

Vector ScreenToWorld(const Vector& screen)
{
	int w = ScreenWidth / 2;
	int h = ScreenHeight / 2;

	Vector s;
	s.x = (screen.x - w) / w;
	s.y = -(screen.y - h) / h;
	s.z = screen.z; // depth

	Vector world;
	gEngfuncs.pTriAPI->ScreenToWorld((float*)&s[0], world);
	return world;
}

RGB GetEntityLighting(cl_entity_t* ent) {
	float colors[3];
	gEngfuncs.pTriAPI->LightAtPoint(ent->origin, colors);
	return RGB(colors[0], colors[1], colors[2]);
}