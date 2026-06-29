#include "effects.h"
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include "com_weapons.h"
#include "shared_util.h"
#include "event_api.h"
#include "engine_pv.h"
#include "pm_defs.h"
#include "eventscripts.h"
#include "ev_hldm.h"
#include "mstream.h"
#include "sprites.h"

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
	origin[0] = args.x = READ_BITS(16);
	origin[1] = args.y = READ_BITS(16);
	origin[2] = args.z = READ_BITS(16);
	int modelIdx = READ_BITS(9);

	args.renderMode = kRenderTransAdd;
	args.sprMode = SPR_MODE_PARALLEL;
	args.color.r = args.color.g = args.color.b = 255;
	args.scale = 10;
	args.loopAnim = READ_BIT();

	if (READ_BIT()) {
		args.scale = READ_BITS(8);
	}
	if (READ_BIT()) {
		args.framerate = READ_BITS(8) * 0.1f;
	}
	if (READ_BIT()) {
		args.maxLife = READ_BITS(8);
	}
	if (READ_BIT()) {
		args.startFrame = READ_BITS(8);
	}
	if (READ_BIT()) {
		args.endFrame = READ_BITS(8);
	}
	if (READ_BIT()) {
		args.fadeMode = READ_BITS(8);

		if (READ_BIT()) {
			args.fadeTime = READ_BITS(8);
		}
	}
	if (READ_BIT()) {
		args.useLightmap = READ_BIT();
		args.renderMode = READ_BITS(3);

		if (args.renderMode != kRenderNormal) {
			args.renderamt = READ_BITS(8);
		}
	}
	if (READ_BIT()) {
		args.sprMode = READ_BITS(3);

		if (args.sprMode == SPR_MODE_ORIENTED || args.sprMode == SPR_MODE_PARALLEL_ORIENTED) {
			args.rx = READ_BITS(12) * 0.1f;
			args.ry = READ_BITS(12) * 0.1f;
			args.rz = READ_BITS(12) * 0.1f;
		}
	}
	if (READ_BIT()) {
		args.expandSpeed = READ_BITS(16);

		if (READ_BIT()) {
			args.expandMax = READ_BITS(10);
		}
	}
	if (READ_BIT()) {
		args.color.r = READ_BITS(8);
		args.color.g = READ_BITS(8);
		args.color.b = READ_BITS(8);
	}
	if (READ_BIT()) {
		args.spinX = (int8_t)READ_BITS(8);
		args.spinY = (int8_t)READ_BITS(8);
		args.spinZ = (int8_t)READ_BITS(8);
	}
	if (READ_BIT()) {
		args.vel = READ_VECTOR_LOWP(10);
		args.acc = READ_VECTOR_LOWP(10);

		args.collideBsp = READ_BIT();
		args.collideEnt = READ_BIT();

		if (READ_BIT()) {
			args.elasticity = READ_BITS(7);
		}
	}

	model_s* model = gEngfuncs.hudGetModelByIndex(modelIdx);
	tempent_s* temp = gEngfuncs.pEfxAPI->CL_TentEntAllocCustom(origin, model, 1, SpriteAdvCallback);

	if (!temp)
		return 1;

	LinkSpriteAdv(temp, spriteAdv);

	return 1;
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

int __MsgFunc_Tracer2(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	float start[3];
	float end[3];
	start[0] = READ_SHORT();
	start[1] = READ_SHORT();
	start[2] = READ_SHORT();
	end[0] = READ_SHORT();
	end[1] = READ_SHORT();
	end[2] = READ_SHORT();
	int color = READ_BYTE();

	EV_CreateTracer(start, end, color);

	return 1;
}

int __MsgFunc_BloodSpr2(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	float pos[3];
	pos[0] = READ_SHORT();
	pos[1] = READ_SHORT();
	pos[2] = READ_SHORT();
	short spraySpriteIdx = READ_SHORT();
	short dripSpriteIdx = READ_SHORT();
	int color = READ_BYTE();
	int scale = READ_BYTE();

	gEngfuncs.pEfxAPI->R_BloodSprite(pos, color, spraySpriteIdx, dripSpriteIdx, scale);

	return 1;
}

int __MsgFunc_TempFx(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	float pos[3];
	int type = READ_BYTE();

	switch (type) {
	case TE_DECAL:
	case TE_GUNSHOTDECAL: {
		pos[0] = READ_SHORT();
		pos[1] = READ_SHORT();
		pos[2] = READ_SHORT();
		int decalIdx = READ_SHORT();
		int entityIdx = READ_SHORT();

		gEngfuncs.pEfxAPI->R_DecalShoot(gEngfuncs.pEfxAPI->Draw_DecalIndex(decalIdx), entityIdx, 0, pos, 0);

		if (type == TE_GUNSHOTDECAL) {
			EV_HLDM_GunshotDecalEffects(pos, true, true);
		}
		break;
	}
	case TE_SPARKS:
		pos[0] = READ_SHORT();
		pos[1] = READ_SHORT();
		pos[2] = READ_SHORT();
		gEngfuncs.pEfxAPI->R_SparkShower(pos);
		break;
	case TE_EXPLOSION: {
		pos[0] = READ_SHORT();
		pos[1] = READ_SHORT();
		pos[2] = READ_SHORT();
		int sprIndex = READ_SHORT();
		int scale = READ_BYTE();
		int framerate = READ_BYTE();
		int eflags = READ_BYTE();
		gEngfuncs.pEfxAPI->R_Explosion(pos, sprIndex, scale * 0.1f, framerate, eflags);
		break;
	}
	case TE_SMOKE: {
		pos[0] = READ_SHORT();
		pos[1] = READ_SHORT();
		pos[2] = READ_SHORT();
		int sprIndex = READ_SHORT();
		int scale = READ_BYTE();
		int framerate = READ_BYTE();
		TEMPENTITY* pTemp = gEngfuncs.pEfxAPI->R_DefaultSprite(pos, sprIndex, framerate);
		gEngfuncs.pEfxAPI->R_Sprite_Smoke(pTemp, scale * 0.1f);
		break;
	}
	case TE_STREAK_SPLASH: {
		float dir[3];
		pos[0] = READ_SHORT();
		pos[1] = READ_SHORT();
		pos[2] = READ_SHORT();
		dir[0] = READ_COORD();
		dir[1] = READ_COORD();
		dir[2] = READ_COORD();
		int color = READ_BYTE();
		int count = READ_SHORT();
		int speed = READ_SHORT();
		int speedNoise = READ_SHORT();
		gEngfuncs.pEfxAPI->R_StreakSplash(pos, dir, color, count, speed, -speedNoise, speedNoise);
		break;
	}
	case TE_BREAKMODEL: {
		float size[3];
		float dir[3];
		pos[0] = READ_SHORT();
		pos[1] = READ_SHORT();
		pos[2] = READ_SHORT();
		size[0] = READ_COORD();
		size[1] = READ_COORD();
		size[2] = READ_COORD();
		dir[0] = READ_COORD();
		dir[1] = READ_COORD();
		dir[2] = READ_COORD();
		int random = READ_BYTE();
		int modelIdx = READ_SHORT();
		int shards = READ_BYTE();
		int duration = READ_BYTE();
		int flags = READ_BYTE();
		gEngfuncs.pEfxAPI->R_BreakModel(pos, size, dir, random * 10, duration * 0.1f, shards, modelIdx, flags);
		break;
	}
	case TE_DLIGHT: {
		pos[0] = READ_SHORT();
		pos[1] = READ_SHORT();
		pos[2] = READ_SHORT();
		int radius = READ_BYTE();
		int r = READ_BYTE();
		int g = READ_BYTE();
		int b = READ_BYTE();
		int time = READ_BYTE();
		int decay = READ_BYTE();

		dlight_t* dl = gEngfuncs.pEfxAPI->CL_AllocDlight(0);
		VectorCopy(pos, dl->origin);
		dl->radius = radius * 10;
		dl->decay = decay * 10;
		dl->color.r = r;
		dl->color.g = g;
		dl->color.b = b;
		dl->die = gEngfuncs.GetClientTime() + time * 0.1f;
		break;
	}
	}


	return 1;
}

int __MsgFunc_Fog(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);
	g_fog.enabled = READ_BYTE();
	g_fog.color.x = READ_BYTE() / 255.0f;
	g_fog.color.y = READ_BYTE() / 255.0f;
	g_fog.color.z = READ_BYTE() / 255.0f;
	g_fog.startDist = (uint16_t)READ_SHORT();
	g_fog.endDist = (uint16_t)READ_SHORT();
	return 1;
}

void HookEffectMessages() {
	HOOK_MESSAGE(Fog);
	HOOK_MESSAGE(ToxicCloud);
	HOOK_MESSAGE(SpriteAdv);
	HOOK_MESSAGE(WaterSplash);
	HOOK_MESSAGE(Tracer2);
	HOOK_MESSAGE(BloodSpr2);
	HOOK_MESSAGE(TempFx);
}