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
#include "ModPlayerState.h"

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

HSPRITE SPR_Load(const char* path) {
	HSPRITE spr = gEngfuncs.pfnSPR_Load(path);
	if (spr && spr > g_loadedSprites) {
		g_loadedSprites = spr;
	}
	return spr;
}

cl_entity_t* GetLocalPlayer() {
	static cl_entity_t dummyPlayer; // prevent crashes when map is not loaded
	return gHUD.m_is_map_loaded ? gEngfuncs.GetLocalPlayer() : &dummyPlayer;
}

ModPlayerState& GetLocalPlayerState() {
	int idx = GetLocalPlayer()->index;
	return g_modPlayerStates[idx];
}

void COM_FileBase(const char* in, char* out)
{
	int len, start, end;

	len = strlen(in);

	// scan backward for '.'
	end = len - 1;
	while (end && in[end] != '.' && in[end] != '/' && in[end] != '\\')
		end--;

	if (in[end] != '.')		// no '.', copy to end
		end = len - 1;
	else
		end--;					// Found ',', copy to left of '.'


	// Scan backward for '/'
	start = len - 1;
	while (start >= 0 && in[start] != '/' && in[start] != '\\')
		start--;

	if (in[start] != '/' && in[start] != '\\')
		start = 0;
	else
		start++;

	// Length of new sting
	len = end - start + 1;

	// Copy partial string
	strncpy(out, &in[start], len);
	// Terminate it
	out[len] = 0;
}

int IsGame(const char* game)
{
	const char* gamedir;
	char gd[1024];

	gamedir = gEngfuncs.pfnGetGameDirectory();
	if (gamedir && gamedir[0])
	{
		COM_FileBase(gamedir, gd);
		if (!stricmp(gd, game))
			return 1;
	}
	return 0;
}