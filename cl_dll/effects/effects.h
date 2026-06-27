#pragma once
#include "vector.h"
#include "const.h"
#include "r_efx.h"
#include "shared_effects.h"

// advanced sprite state for various effects
struct SpriteAdv {
	SpriteAdvArgs args;
	float spawnTime;
	int index;
};

struct Fog {
	bool enabled;
	Vector color;
	float startDist;
	float endDist;
};

extern Fog g_fog;

void HookEffectMessages();

void EffectsInit();

SpriteAdv& AllocSpriteAdv();
void LinkSpriteAdv(tempent_s* temp, SpriteAdv& state);
void SpriteAdvCallback(tempent_s* tempent, float frametime, float curtime);

void EF_WaterSplash(Vector origin, int splashSprIdx, int wakeSprIdx, const char* sample, float scale, int fps, float volume, int pitch);
void PredictBodySplash();

void FlashlightEffect();

void SetupFog();