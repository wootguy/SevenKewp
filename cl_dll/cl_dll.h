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
//  cl_dll.h
//

// 4-23-98  JOHN

//
//  This DLL is linked by the client when they first initialize.
// This DLL is responsible for the following tasks:
//		- Loading the HUD graphics upon initialization
//		- Drawing the HUD graphics every frame
//		- Handling the custum HUD-update packets
//

#include "Platform.h"

typedef unsigned char byte;
typedef unsigned short word;
typedef float vec_t;
typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

#include "vector.h"

#include "../engine/cdll_int.h"
#include "../game_shared/cdll_dll.h"

struct playersim_t {
	vec3_t v_origin;		// camera view origin
	vec3_t v_sim_org;		// player position
	vec3_t v_sim_vel;		// player velocity
	vec3_t v_angles;		// player view angles
	vec3_t v_cl_angles;
	vec3_t v_lastAngles;
	vec3_t ev_punchangle;	// client-side punch angle predicted in weapons code
	vec3_t v_punchangle;	// final combined client and server punch angle
	vec3_t cam_ofs;			// thirdperson camera pitch, yaw, dist
	int cam_thirdperson;	// 1 = third person camera active
	bool b_viewing_cam;		// true if player's view is attached to another entity such as a camera
	float v_frametime;		// view simulation frametime
};

extern cl_enginefunc_t gEngfuncs;
extern playersim_t gPlayerSim;
