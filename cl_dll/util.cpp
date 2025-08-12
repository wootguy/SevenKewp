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
#include <string.h>

#ifndef M_PI
#define M_PI		3.14159265358979323846	// matches value in gcc v2 math.h
#endif

#ifdef _WIN32
vec3_t vec3_origin( 0, 0, 0 ); // linux complains this is doubly defined, yet windows requires it
#endif

HSPRITE LoadSprite(const char *pszName)
{
	int iRes;
	char sz[256]; 

#if !defined( _TFC )
	if (ScreenWidth > 2560 && ScreenHeight > 1600)
		iRes = 2560;
	else if (ScreenWidth >= 1280 && ScreenHeight > 720)
		iRes = 1280;
	else 
#endif
	if (ScreenWidth >= 640)
		iRes = 640;
	else
		iRes = 320;

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

char* UTIL_VarArgs(const char* format, ...) {
	va_list		argptr;
	static char	string[1024];

	va_start(argptr, format);
	vsnprintf(string, 1024, format, argptr);
	va_end(argptr);

	string[1023] = 0;

	return string;
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