#include "effects.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include "shared_util.h"
#include "event_api.h"
#include <cfloat>
#include "pm_defs.h"
#include "gfx_util.h"
#include "triangleapi.h"
#include "GL/gl.h"

#define MAX_ADV_SPRITES 512

SpriteAdv g_sprAdvArgs[MAX_ADV_SPRITES];
int g_advSpriteArgIdx;
Fog g_fog;

SpriteAdv& AllocSpriteAdv() {
	int idx = g_advSpriteArgIdx++ % MAX_ADV_SPRITES;

	memset(&g_sprAdvArgs[idx], 0, sizeof(SpriteAdv));
	g_sprAdvArgs[idx].index = idx;
	g_sprAdvArgs[idx].spawnTime = gEngfuncs.GetClientTime();

	return g_sprAdvArgs[idx];
}

// links a sprite state to temp ent and initializes the temp ent
void LinkSpriteAdv(tempent_s* temp, SpriteAdv& state) {
	SpriteAdvArgs& args = state.args;

	if (!args.scale) {
		args.scale = 10;
	}
	if (!args.renderamt) {
		args.renderamt = 200;
	}
	if (!args.color.r && !args.color.g && !args.color.b) {
		args.color.r = args.color.g = args.color.b = 255;
	}

	temp->entity.curstate.scale = args.scale * 0.1f;
	temp->entity.curstate.renderamt = args.renderamt;
	temp->entity.curstate.rendermode = args.renderMode;
	temp->entity.curstate.rendercolor.r = args.color.r;
	temp->entity.curstate.rendercolor.g = args.color.g;
	temp->entity.curstate.rendercolor.b = args.color.b;
	temp->entity.angles.x = args.rx;
	temp->entity.angles.y = args.ry;
	temp->entity.angles.z = args.rz;
	temp->entity.curstate.frame = args.startFrame;
	temp->entity.curstate.framerate = args.framerate;

	temp->entity.curstate.velocity = args.vel;

	float lifetime = args.maxLife * 0.1f;

	if (!args.endFrame) {
		args.endFrame = temp->entity.model->numframes;
	}

	if (!args.maxLife) {
		lifetime = args.endFrame / (float)args.framerate;
	}

	temp->die = state.spawnTime + lifetime;

	temp->entity.curstate.iuser1 = state.index;
}

// generic sprite effect routine
void SpriteAdvCallback(tempent_s* tempent, float frametime, float curtime) {
	SpriteAdv& advSpr = g_sprAdvArgs[tempent->entity.curstate.iuser1];
	SpriteAdvArgs& args = advSpr.args;

	tempent->entity.angles.x += args.spinX * 10.0f * frametime;
	tempent->entity.angles.y += args.spinY * 10.0f * frametime;
	tempent->entity.angles.z += args.spinZ * 10.0f * frametime;

	tempent->entity.curstate.velocity = tempent->entity.curstate.velocity + args.acc * frametime;

	Vector newOri = tempent->entity.origin + tempent->entity.curstate.velocity * frametime;

	if (args.collideBsp || args.collideEnt) {
		int flags = args.collideEnt ? PM_NORMAL : PM_STUDIO_IGNORE;

		pmtrace_t pmtrace;
		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(tempent->entity.origin, newOri, flags, -1, &pmtrace);

		// ignore players when not flagged to collide with entities.
		if (pmtrace.fraction != 1 && !args.collideEnt) {
			physent_t* pe = gEngfuncs.pEventAPI->EV_GetPhysent(pmtrace.ent);
			if (pe && pe->info >= 1 && pe->info <= gEngfuncs.GetMaxClients()) {
				// no easy way to set all players as non-solid(?), so running the trace again here
				// with the collided player ignored.
				gEngfuncs.pEventAPI->EV_PlayerTrace(tempent->entity.origin, newOri, flags, pmtrace.ent, &pmtrace);
			}
		}

		if (pmtrace.fraction != 1)
		{
			newOri = pmtrace.endpos;
			tempent->entity.curstate.velocity = -tempent->entity.curstate.velocity * args.elasticity * 0.01f;
		}
	}

	tempent->entity.origin = newOri;

	if (args.useLightmap) {
		RGB light = GetEntityLighting(&tempent->entity).ApplyGamma();
		tempent->entity.curstate.rendercolor.r = light.r;
		tempent->entity.curstate.rendercolor.g = light.g;
		tempent->entity.curstate.rendercolor.b = light.b;
	}

	tempent->entity.curstate.frame += tempent->entity.curstate.framerate * frametime;
	if (!args.loopAnim && tempent->entity.curstate.frame >= tempent->entity.model->numframes) {
		tempent->entity.curstate.frame = tempent->entity.model->numframes - 1;
	}

	if (args.expandSpeed) {
		float& scale = tempent->entity.curstate.scale;
		scale += args.expandSpeed * 0.01f * frametime;

		if (args.expandMax) {
			float expandMax = args.expandMax * 0.1f;

			if (args.expandSpeed > 0 && scale > expandMax) {
				scale = expandMax;
			}
			else if (args.expandSpeed < 0 && scale < expandMax) {
				scale = expandMax;
			}
		}

		if (scale <= 0)
			scale = FLT_MIN;
	}

	if (args.fadeMode) {
		float timeAlive = curtime - advSpr.spawnTime;
		float timeLeft = tempent->die - curtime;
		float fadeTime = args.fadeTime * 0.1f;
		float fadeIn = clampf(timeAlive / fadeTime, 0, 1);
		float fadeOut = clampf(timeLeft / fadeTime, 0, 1);

		switch (args.fadeMode) {
		case SPRADV_FADE_OUT:
			tempent->entity.curstate.renderamt = args.renderamt * fadeOut;
			break;
		case SPRADV_FADE_IN:
			tempent->entity.curstate.renderamt = args.renderamt * fadeIn;
			break;
		case SPRADV_FADE_BOTH:
			tempent->entity.curstate.renderamt = args.renderamt * V_min(fadeOut, fadeIn);
			break;
		default:
			break;
		}
	}
}

void EF_WaterSplash(Vector origin, int splashSprIdx, int wakeSprIdx, const char* sample, float scale, int fps, float volume, int pitch) {
	model_s* splashModel = gEngfuncs.hudGetModelByIndex(splashSprIdx);
	model_s* wakeModel = gEngfuncs.hudGetModelByIndex(wakeSprIdx);
	tempent_s* splashEnt = gEngfuncs.pEfxAPI->CL_TentEntAllocCustom(origin, splashModel, 1, SpriteAdvCallback);
	tempent_s* wakeEnt = gEngfuncs.pEfxAPI->CL_TentEntAllocCustom(origin, wakeModel, 1, SpriteAdvCallback);
	tempent_s* wakeEnt2 = gEngfuncs.pEfxAPI->CL_TentEntAllocCustom(origin, wakeModel, 1, SpriteAdvCallback);

	if (!splashEnt || !wakeEnt)
		return;

	SpriteAdv& splashAdv = AllocSpriteAdv();
	splashAdv.args.scale = scale * 34;
	splashAdv.args.renderMode = kRenderTransAlpha;
	splashAdv.args.renderamt = 200;
	splashAdv.args.framerate = fps;
	LinkSpriteAdv(splashEnt, splashAdv);
	splashEnt->entity.origin.z += splashModel->maxs.z * splashEnt->entity.curstate.scale;

	SpriteAdv& wakeAdv = AllocSpriteAdv();
	wakeAdv.args.maxLife = 10;
	wakeAdv.args.sprMode = SPR_MODE_ORIENTED;
	wakeAdv.args.rx = 90;
	wakeAdv.args.rz = gEngfuncs.pfnRandomLong(0, 359);
	wakeAdv.args.spinZ = gEngfuncs.pfnRandomLong(0, 1) ? 1 : -1;
	wakeAdv.args.renderMode = kRenderTransAlpha;
	wakeAdv.args.renderamt = 200;
	wakeAdv.args.fadeMode = SPRADV_FADE_OUT;
	wakeAdv.args.fadeTime = 8;
	wakeAdv.args.expandSpeed = 15;
	wakeAdv.args.scale = scale * 10;
	LinkSpriteAdv(wakeEnt, wakeAdv);

	SpriteAdv& wakeAdv2 = AllocSpriteAdv();
	wakeAdv2.args = wakeAdv.args;
	wakeAdv2.args.rx = -90; // underside of the wake
	LinkSpriteAdv(wakeEnt2, wakeAdv2);

	if (volume > 0) {
		gEngfuncs.pEventAPI->EV_PlaySound(0, origin, CHAN_STATIC, sample, volume, ATTN_NORM, 0, pitch);
	}
}

extern playermove_t* pmove;

void UTIL_WaterSplashFootstep(int player_index) {
	// can only predict local player so index is unused
	Vector origin = gPlayerSim.v_sim_org - Vector(0, 0, 36);
	float scale = 0.3f;
	int fps = 20;

	Vector surface;
	if (UTIL_WaterTrace(origin, origin + Vector(0, 0, 36), surface)) {
		origin = surface + Vector(0, 0, 1);
	}

	int splashSprIdx = gEngfuncs.pEventAPI->EV_FindModelIndex(RemapFile("sprites/splash2.spr"));
	int wakeSprIdx = gEngfuncs.pEventAPI->EV_FindModelIndex(RemapFile("sprites/splashwake.spr"));

	EF_WaterSplash(origin, splashSprIdx, wakeSprIdx, 0, scale, fps, 0, 0);
}

void PredictBodySplash() {
	static int oldContents = CONTENTS_EMPTY;
	static Vector oldPos;

	if (!gHUD.m_is_map_loaded) {
		oldContents = CONTENTS_EMPTY;
		return;
	}

	Vector origin = gPlayerSim.v_sim_org;
	origin.z -= (pmove->flags & FL_DUCKING) ? 18 : 36;

	int contents = UTIL_PointContents(origin);
	bool inLiquid = UTIL_IsLiquidContents(contents);

	bool wasInLiquid = UTIL_IsLiquidContents(oldContents);
	bool solidTransition = oldContents == CONTENTS_SOLID || contents == CONTENTS_SOLID;
	bool playerJumpingOut = !inLiquid;
	const float thickness = 32;

	// water transiation
	if (wasInLiquid != inLiquid && !solidTransition && !playerJumpingOut) {
		if (fabs(gPlayerSim.v_sim_vel.Length()) > 100 && gPlayerSim.v_sim_vel.z < -50) {
			Vector splashPos = inLiquid ? origin : oldPos;
			float scale = 0.63f;

			float fps, vol, ratio, sz;
			int pitch;
			const char* sample;
			UTIL_WaterSplashParams(scale, 1, ratio, sz, fps, vol, pitch, sample);
			sample = RemapFile(sample);

			int splashSprIdx = gEngfuncs.pEventAPI->EV_FindModelIndex(RemapFile("sprites/splash2.spr"));
			int wakeSprIdx = gEngfuncs.pEventAPI->EV_FindModelIndex(RemapFile("sprites/splashwake.spr"));

			EF_WaterSplash(splashPos, splashSprIdx, wakeSprIdx, sample, scale, fps, vol, pitch);
		}
	}

	oldContents = contents;
	oldPos = origin;
}

void FlashlightEffect() {
	if (g_flashlight_size == 8 || !gHUD.m_is_map_loaded)
		return; // let the engine handle the default size

	static bool wasOn;
	bool isOn = gHUD.m_Flash.m_fOn;

	if (isOn) {
		// override the default flashlight effect with a custom dlight
		cl_entity_t* player = GetLocalPlayer();
		int idx = player->index;
		pmtrace_t tr;

		Vector origin = gPlayerSim.v_sim_org;
		Vector view_ofs;
		gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);

		Vector vecSrc = origin + view_ofs;
		Vector forward, right, up;
		Vector angles = gPlayerSim.v_angles + gPlayerSim.ev_punchangle;
		AngleVectors(angles, forward, right, up);
		Vector vecEnd = vecSrc + forward * 4096.0f;

		gEngfuncs.pEventAPI->EV_SetUpPlayerPrediction(false, true);
		gEngfuncs.pEventAPI->EV_PushPMStates();
		gEngfuncs.pEventAPI->EV_SetSolidPlayers(idx - 1);
		gEngfuncs.pEventAPI->EV_SetTraceHull(2);
		gEngfuncs.pEventAPI->EV_PlayerTrace(vecSrc, vecEnd, PM_NORMAL, -1, &tr);
		gEngfuncs.pEventAPI->EV_PopPMStates();

		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(1337);
		dl->origin = tr.endpos;
		dl->radius = g_flashlight_size * 10;
		dl->color.r = 255;
		dl->color.g = 255;
		dl->color.b = 255;
		dl->key = 1337;
		dl->die = gEngfuncs.GetClientTime() + 0.5;
	}
	else if (wasOn) {
		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(1337);
		dl->die = 0; // turn off the dlight now
	}

	wasOn = isOn;
}

void SetupFog() {
	if (!g_fog.enabled)
	{
		gEngfuncs.pTriAPI->Fog(Vector(0, 0, 0), 0, 0, FALSE);
		glDisable(GL_FOG);
		return;
	}

	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_DENSITY, 0.5);

	glFogfv(GL_FOG_COLOR, g_fog.color);
	glFogf(GL_FOG_START, g_fog.startDist);
	glFogf(GL_FOG_END, g_fog.endDist);

	// Tell the engine too
	gEngfuncs.pTriAPI->Fog(g_fog.color, g_fog.startDist, g_fog.endDist, TRUE);
}

void EffectsInit() {
	g_fog.enabled = false;
}