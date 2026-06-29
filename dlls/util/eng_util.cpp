#include "rehlds.h"
#include "eng_util.h"
#include "util.h"

msprite_sv_t* GET_SPRITE_PTR(int modelindex) {
	if (!UTIL_ModelIsSprite(modelindex)) {
		return NULL;
	}
	
	model_t* model = g_RehldsData->GetModel(modelindex);
	
	return model ? (msprite_sv_t*)model->cache.data : NULL;
}