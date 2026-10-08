#include "extdll.h"
#include "eng_util.h"
#include "util.h"
#include "rehlds_util.h"
#include "com_model.h"

msprite_sv_t* GET_SPRITE_PTR(int modelindex) {
	if (!UTIL_ModelIsSprite(modelindex)) {
		return NULL;
	}
	
	model_t* model = rehlds_get_model(modelindex);
	
	return model ? (msprite_sv_t*)model->cache.data : NULL;
}