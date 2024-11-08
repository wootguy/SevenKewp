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

