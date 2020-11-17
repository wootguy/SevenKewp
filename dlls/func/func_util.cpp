#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "func_util.h"

Vector VecBModelOrigin(entvars_t* pevBModel)
{
	return pevBModel->absmin + (pevBModel->size * 0.5);
}