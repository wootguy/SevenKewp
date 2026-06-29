#include "hud.h"
#include "cl_util.h"
#include "gfx_util.h"
#include "com_model.h"
#include "triangleapi.h"
#include "com_weapons.h"
#include "wc_params.h"
#include "shared_effects.h"
#include "GL/gl.h"
#include "sprites.h"

// entities that should have sprites rendered in their place are added here
// then drawn at transparent triangle render time
#define MAX_SPRITE_RENDER_QUEUE_SZ 512
uint16_t g_spriteRenderQueue[MAX_SPRITE_RENDER_QUEUE_SZ];
int g_spriteRenderQueueSz;

void gfx_start_colored_rendering(int polyMode, const RGBA& color) {
	gEngfuncs.pTriAPI->RenderMode(is_software_renderer ? kRenderTransTexture : kRenderTransAlpha);

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

	if (is_software_renderer && c.a < 255)
		triRenderMode = kRenderTransAdd; // the only working transparency mode

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

int GetSpriteAngleFrame(Vector spritePos, Vector spriteForward, Vector spriteRight, Vector lookPos)
{
	Vector delta = spritePos - lookPos;
	delta.z = 0;
	delta = delta.Normalize();

	float dotR = DotProduct(delta, spriteRight);
	float dot = DotProduct(delta, spriteForward);

	const float COS22 = 0.92387953f;
	const float COS67 = 0.38268343f;

	if (dot < -COS22)
		return 0;
	else if (dot < -COS67)
		return dotR > 0 ? 1 : 7;
	else if (dot < COS67)
		return dotR > 0 ? 2 : 6;
	else if (dot < COS22)
		return dotR > 0 ? 3 : 5;
	else
		return 4;
}

void gfx_draw_sprite(Vector origin, Vector angles, int modelIdx, int frame, SpriteMode sprMode, float scale, RGBA color, AngleSpriteMode angleMode) {
	model_t* model = gEngfuncs.hudGetModelByIndex(modelIdx);

	if (!g_studio_init)
		return;

	if (!model || model->type != mod_sprite) {
		PRINTF("Tried to draw invalid sprite index %d\n", modelIdx);
		return;
	}

	msprite_cl_t* header = (msprite_cl_t*)IEngineStudio.Mod_Extradata(model);
	
	Vector right;
	Vector up;

	if (sprMode == SPR_MODE_AUTO) {
		sprMode = (SpriteMode)header->type;
	}

	switch (sprMode) {
	default:
	case SPR_MODE_PARALLEL:
	case SPR_MODE_PARALLEL_ORIENTED:
		up = gPlayerSim.cam_up;
		right = gPlayerSim.cam_right;
		break;
	case SPR_MODE_PARALLEL_UPRIGHT:
	case SPR_MODE_FACING_UPRIGHT: // it's broken in-game, but it sort of looks like parallel_upright
		up = Vector(0, 0, 1);
		right = gPlayerSim.cam_right;
		break;
	case SPR_MODE_ORIENTED:
		break; // not implemented - needs lots of testing
	}

	up = up * scale;
	right = right * scale;

	if (angleMode == ANGLE_SPRITE_8WAY) {
		Vector sprForward, sprRight, sprUp;
		AngleVectors(angles, sprForward, sprRight, sprUp);
		int angle = GetSpriteAngleFrame(origin, sprForward, sprRight, gPlayerSim.v_origin);
		frame = frame * 8 + angle;
	}

	frame = clamp(frame, 0, header->numframes-1);

	float fleft, fright, fup, fdown;
	if (is_software_renderer) {
		mspriteframe_sw_t* fr = header->frames[frame].frameptr_sw;
		fleft = fr->left;
		fright = fr->right;
		fup = fr->up;
		fdown = fr->down;
	}
	else {
		mspriteframe_hw_t* fr = header->frames[frame].frameptr_hw;
		fleft = fr->left;
		fright = fr->right;
		fup = fr->up;
		fdown = fr->down;
	}

	Vector tl = origin + fleft * right + fup * up;
	Vector tr = origin + fright * right + fup * up;
	Vector br = origin + fright * right + fdown * up;
	Vector bl = origin + fleft * right + fdown * up;

	int rmode = kRenderTransTexture;
	if (is_software_renderer) {
		rmode = color.a < 255 ? kRenderTransAdd : kRenderTransAlpha;
	}

	gEngfuncs.pTriAPI->RenderMode(rmode);
	gEngfuncs.pTriAPI->SpriteTexture(model, frame);
	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	gEngfuncs.pTriAPI->Color4f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);

	gEngfuncs.pTriAPI->TexCoord2f(0, 0); gEngfuncs.pTriAPI->Vertex3fv(tl);
	gEngfuncs.pTriAPI->TexCoord2f(1, 0); gEngfuncs.pTriAPI->Vertex3fv(tr);
	gEngfuncs.pTriAPI->TexCoord2f(1, 1); gEngfuncs.pTriAPI->Vertex3fv(br);
	gEngfuncs.pTriAPI->TexCoord2f(0, 1); gEngfuncs.pTriAPI->Vertex3fv(bl);

	gEngfuncs.pTriAPI->End();
}

void queue_sprite_render_ent(int entindex) {
	if (g_spriteRenderQueueSz < MAX_SPRITE_RENDER_QUEUE_SZ) {
		g_spriteRenderQueue[g_spriteRenderQueueSz++] = entindex;
	}
	else {
		PRINTF("Sprite queue overflowed\n");
	}
}

void render_sprite_ent(cl_entity_t* ent) {
	entity_state_t& state = ent->curstate;
	int sprIdx = state.weaponmodel & 0x1ff;
	AngleSpriteMode sprMode = (AngleSpriteMode)(state.weaponmodel >> 9);
	RGBA color(state.rendercolor.r, state.rendercolor.g, state.rendercolor.b, state.renderamt);
	if (state.rendermode == kRenderNormal)
		color.a = 255;

	gfx_draw_sprite(state.origin, state.angles, sprIdx, state.frame, SPR_MODE_AUTO, state.scale, color, sprMode);
}

void render_sprite_queue() {
	if (!is_software_renderer) {
		for (int i = 0; i < g_spriteRenderQueueSz; i++) {
			cl_entity_t* ent = gEngfuncs.GetEntityByIndex(g_spriteRenderQueue[i]);
			if (!ent || !ent->curstate.weaponmodel) {
				continue;
			}

			render_sprite_ent(ent);
		}
	}
	else {
		// software mode doesn't sort our custom triangles, which means transparent pixels sometimes
		// overwrite sprites in the background. Sort sprites back to front to fix that.
		struct SortEnt {
			float dist;
			cl_entity_t* ent;
		};
		static SortEnt sortTemp[MAX_SPRITE_RENDER_QUEUE_SZ];
		int numSort = 0;

		for (int i = 0; i < g_spriteRenderQueueSz; i++) {
			cl_entity_t* ent = gEngfuncs.GetEntityByIndex(g_spriteRenderQueue[i]);
			if (!ent || !ent->curstate.weaponmodel) {
				continue;
			}

			SortEnt& sortItem = sortTemp[numSort++];
			sortItem.dist = (ent->curstate.origin - gPlayerSim.v_origin).Length();
			sortItem.ent = ent;
		}

		std::sort(std::begin(sortTemp), std::begin(sortTemp) + numSort,
			[](const SortEnt& a, const SortEnt& b) {
				return a.dist > b.dist;
			});

		for (int i = 0; i < numSort; i++) {
			render_sprite_ent(sortTemp[i].ent);
		}
	}
	
	g_spriteRenderQueueSz = 0;
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