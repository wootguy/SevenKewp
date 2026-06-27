#include "vector.h"
#include "const.h"

void HookEffectMessages();

void EF_WaterSplash(Vector origin, int splashSprIdx, int wakeSprIdx, const char* sample, float scale, int fps, float volume, int pitch);

void PredictBodySplash();

void FlashlightEffect();