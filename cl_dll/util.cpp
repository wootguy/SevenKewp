/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// util.cpp
//
// implementation of class-less helper functions
//

#include <cstdio>
#include <cstdlib>
#include <cmath>

#include "hud.h"
#include "cl_util.h"
#include "shared_util.h"
#include <string.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#ifdef _WIN32
vec3_t vec3_origin( 0, 0, 0 ); // linux complains this is doubly defined, yet windows requires it
#endif

void printd(const char* format, ...) // use this to print inside pm_* code
{
	va_list		argptr;
	static char		string[1024];

	va_start(argptr, format);
	vsnprintf(string, 1024, format, argptr);
	va_end(argptr);

	PRINTF(string);
}

bool g_crosshair_active;

HSPRITE LoadSprite(const char *pszName)
{
	char sz[256]; 

	int iRes = gHUD.GetDesiredSpriteRes();

	sprintf(sz, pszName, iRes);

	return SPR_Load(sz);
}

bool fileExists(const char* path) {
	FILE* file = fopen(path, "r");

	if (file) {
		fclose(file);
		return true;
	}

	return false;
}

const char* FindGameFile(const char* path) {
	if (!path || !path[0])
		return NULL;

	const char* gamedir = gEngfuncs.pfnGetGameDirectory();
	const char* search_suffix[4] = {
		"_addon",
		"_hd",
		"",
		"_downloads",
	};

	for (int i = 0; i < 4; i++) {
		const char* searchPath = UTIL_VarArgs("%s%s/%s", gamedir, search_suffix[i], path);
		if (fileExists(searchPath)) {
			return searchPath;
		}
	}

	return NULL;
}

Vector WorldToScreen(const Vector& P, const Vector& viewerOrigin, const Vector& viewerAngles, float fovXDeg) {
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
}

extern Vector v_origin;
extern Vector v_angles;
extern vec3_t cam_ofs;
extern int cam_thirdperson;
extern bool b_viewing_cam;

Vector WorldToScreen(const Vector& P) {
	Vector angles = v_angles;

	if (cam_thirdperson && !b_viewing_cam) {
		angles.x = cam_ofs.x;
		angles.y = cam_ofs.y;
		angles.z = 0; // tilt isn't applied in thirdperson
	}

	return WorldToScreen(P, v_origin, angles, gHUD.m_iFOV);
}