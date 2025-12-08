#include "effects.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include "com_weapons.h"
#include "shared_util.h"
#include "event_api.h"
#include "engine_pv.h"
#include <cfloat>
#include "pm_defs.h"

#define MAX_ADV_SPRITES 512

// advanced sprite state for various effects
struct SpriteAdv {
	SpriteAdvArgs args;
	float spawnTime;
	int index;
};

SpriteAdv g_sprAdvArgs[MAX_ADV_SPRITES];
int g_advSpriteArgIdx;

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
	if (!args.r && !args.g && !args.b) {
		args.r = args.g = args.b = 255;
	}

	temp->entity.curstate.scale = args.scale * 0.1f;
	temp->entity.curstate.renderamt = args.renderamt;
	temp->entity.curstate.rendermode = args.renderMode;
	temp->entity.curstate.rendercolor.r = args.r;
	temp->entity.curstate.rendercolor.g = args.g;
	temp->entity.curstate.rendercolor.b = args.b;
	temp->entity.angles.x = args.rx;
	temp->entity.angles.y = args.ry;
	temp->entity.angles.z = args.rz;
	temp->entity.curstate.frame = args.startFrame;
	temp->entity.curstate.framerate = args.framerate;

	temp->entity.curstate.velocity.x = args.velX;
	temp->entity.curstate.velocity.y = args.velY;
	temp->entity.curstate.velocity.z = args.velZ;

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

	tempent->entity.curstate.velocity.x += args.accelX * frametime;
	tempent->entity.curstate.velocity.y += args.accelY * frametime;
	tempent->entity.curstate.velocity.z += args.accelZ * frametime;

	tempent->entity.origin = tempent->entity.origin + tempent->entity.curstate.velocity * frametime;

	tempent->entity.curstate.frame += tempent->entity.curstate.framerate * frametime;
	if (!args.maxLife && tempent->entity.curstate.frame >= tempent->entity.model->numframes) {
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

int __MsgFunc_ToxicCloud(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	int idx = READ_SHORT();
	int seed = READ_BYTE();
	float colorRand = READ_BYTE() * 0.01f;

	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(idx);

	if (ent) {
		int modelindex;
		model_s* model = gEngfuncs.CL_LoadModel(RemapFile("sprites/hlcoop/puff1.spr"), &modelindex);
		
		if (!model)
			return 1;
		
		Vector pos = ent->origin;

		tempent_s* temp = gEngfuncs.pEfxAPI->CL_TentEntAllocCustom(&pos.x, model, 1, SpriteAdvCallback);
		temp->die = gEngfuncs.GetClientTime() + 3.0f;

		float x = UTIL_SharedRandomLong(seed, -100, 100) * 0.01f;
		seed = UTIL_SharedRandomLong(seed, 0, 255);
		float y = UTIL_SharedRandomLong(seed, -100, 100) * 0.01f;
		seed = UTIL_SharedRandomLong(seed, 0, 255);
		float speed = UTIL_SharedRandomLong(seed, 60, 90);

		// TODO: lighting is calculated for each particle in sven, every frame, but maybe that's too expensive?
		float redScale = ent->cvFloorColor.r / 255.0;
		float blueScale = ent->cvFloorColor.b / 255.0f;

		temp->entity.curstate.velocity = Vector(x, y, 1.0f) * speed;
		temp->entity.curstate.scale = 0.01;
		temp->entity.curstate.renderamt = 255;
		temp->entity.curstate.rendermode = kRenderTransAlpha;
		temp->entity.curstate.rendercolor.r = 240 * colorRand * redScale;
		temp->entity.curstate.rendercolor.b = 255 * colorRand * blueScale;

		SpriteAdv& spr = AllocSpriteAdv();
		spr.args.expandSpeed = 1000;
		spr.args.expandMax = 10 * 10;
		spr.args.spinZ = 10;
		spr.args.fadeMode = SPRADV_FADE_OUT;
		spr.args.fadeTime = 30;
		spr.args.renderamt = 255;
		LinkSpriteAdv(temp, spr);
	}

	return 1;
}

int __MsgFunc_SpriteAdv(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	SpriteAdv& spriteAdv = AllocSpriteAdv();
	SpriteAdvArgs& args = spriteAdv.args;

	float origin[3];
	origin[0] = args.x = READ_SHORT();
	origin[1] = args.y = READ_SHORT();
	origin[2] = args.z = READ_SHORT();
	int modelIdx = READ_SHORT();
	args.scale = READ_BYTE();
	args.framerate = READ_BYTE() * 0.1f;
	args.renderamt = READ_BYTE();
	uint8_t flags = READ_BYTE();

	args.renderMode = kRenderTransAdd;
	args.sprMode = SPR_VP_PARALLEL;
	args.r = args.g = args.b = 255;

	if (flags & FL_SPRITEADV_LOOP) {
		args.maxLife = READ_BYTE();
	}
	if (flags & FL_SPRITEADV_FRAME) {
		args.startFrame = READ_BYTE();
		args.endFrame = READ_BYTE();
	}
	if (flags & FL_SPRITEADV_FADE) {
		args.fadeMode = READ_BYTE();
		args.fadeTime = READ_BYTE();
	}
	if (flags & FL_SPRITEADV_MODE) {
		uint8_t mode = READ_BYTE();
		args.renderMode = (mode >> 4) & 0xf;
		args.sprMode = mode & 0xf;

		if (args.sprMode == SPR_ORIENTED || args.sprMode == SPR_VP_PARALLEL_ORIENTED) {
			args.rx = READ_ANGLE();
			args.ry = READ_ANGLE();
			args.rz = READ_ANGLE();
		}
	}
	if (flags & FL_SPRITEADV_EXPAND) {
		args.expandSpeed = READ_SHORT();
		args.expandMax = READ_BYTE();
	}
	if (flags & FL_SPRITEADV_COLOR) {
		args.r = READ_BYTE();
		args.g = READ_BYTE();
		args.b = READ_BYTE();
	}
	if (flags & FL_SPRITEADV_SPIN) {
		args.spinX = (int8_t)READ_BYTE();
		args.spinY = (int8_t)READ_BYTE();
		args.spinZ = (int8_t)READ_BYTE();
	}
	if (flags & FL_SPRITEADV_MOVE) {
		uint8_t moveFlags = READ_BYTE();

		if (moveFlags & FL_SPRITEADV_MOVE_VEL_X) {
			args.velX = (int16_t)READ_SHORT();
		}
		if (moveFlags & FL_SPRITEADV_MOVE_VEL_Y) {
			args.velY = (int16_t)READ_SHORT();
		}
		if (moveFlags & FL_SPRITEADV_MOVE_VEL_Z) {
			args.velZ = (int16_t)READ_SHORT();
		}
		if (moveFlags & FL_SPRITEADV_MOVE_ACCEL_X) {
			args.accelX = (int16_t)READ_SHORT();
		}
		if (moveFlags & FL_SPRITEADV_MOVE_ACCEL_Y) {
			args.accelY = (int16_t)READ_SHORT();
		}
		if (moveFlags & FL_SPRITEADV_MOVE_ACCEL_Z) {
			args.accelZ = (int16_t)READ_SHORT();
		}
	}

	model_s* model = gEngfuncs.hudGetModelByIndex(modelIdx);
	tempent_s* temp = gEngfuncs.pEfxAPI->CL_TentEntAllocCustom(origin, model, 1, SpriteAdvCallback);

	if (!temp)
		return 1;

	LinkSpriteAdv(temp, spriteAdv);

	return 1;
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
	wakeAdv.args.sprMode = SPR_ORIENTED;
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


int __MsgFunc_WaterSplash(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	float origin[3];
	origin[0] = READ_SHORT();
	origin[1] = READ_SHORT();
	origin[2] = READ_SHORT();
	int splashSprIdx = READ_SHORT();
	int wakeSprIdx = READ_SHORT();
	int soundIdx = READ_SHORT();
	float scale = READ_BYTE() * 0.1f;
	int fps = READ_BYTE();
	float volume = READ_BYTE() * 0.01f;
	int pitch = READ_BYTE();
	
	EF_WaterSplash(origin, splashSprIdx, wakeSprIdx, GetSoundByIndex(soundIdx), scale, fps, volume, pitch);

	return 1;
}

extern playermove_t* pmove;

void UTIL_WaterSplashFootstep(int player_index) {
	// can only predict local player so index is unused
	Vector origin = gPlayerSim.v_sim_org - Vector(0, 0, 36);
	float scale = 0.3f;
	int fps = 20;

	Vector surface;
	if (UTIL_WaterTrace(origin, origin + Vector(0, 0, 36), surface)) {
		origin = surface + Vector(0,0,1);
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

void HookEffectMessages() {
	HOOK_MESSAGE(ToxicCloud);
	HOOK_MESSAGE(SpriteAdv);
	HOOK_MESSAGE(WaterSplash);
}