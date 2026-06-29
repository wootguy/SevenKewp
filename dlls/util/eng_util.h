#pragma once
#include "com_model.h"

//
// Utils for reverse engineered engine functions and data
//

struct msprite_sv_t;

// get engine sprite data
EXPORT msprite_sv_t* GET_SPRITE_PTR(int modelindex);