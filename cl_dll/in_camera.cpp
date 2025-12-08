//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "hud.h"
#include "cl_util.h"
#include "camera.h"
#include "kbutton.h"
#include "cvardef.h"
#include "usercmd.h"
#include "const.h"
#include "camera.h"
#include "in_defs.h"
#include "Exports.h"

#include "SDL2/SDL_mouse.h"
#include "port.h"
#include "pm_defs.h"
#include "event_api.h"

extern cl_enginefunc_t gEngfuncs;
extern SDL_Window* g_sdl_window;

extern cvar_t* cl_pitchdown;
extern cvar_t* cl_pitchup;

//-------------------------------------------------- Constants

#define CAM_MAX_DIST 1024
#define CAM_MIN_DIST 0

//-------------------------------------------------- Global Variables

cvar_t	*cam_snapback;
cvar_t	*cam_idealyaw;
cvar_t	*cam_idealdist;
cvar_t	*cam_contain;

int iMouseInUse=0;

float g_camPressedTime;
float g_camReleasedTime;
Vector g_camPressViewAngles; // view angles when camera movement began
Vector g_camPressAngles; // allow rotating the camera while holding the thirdperson key down
int g_camAdjustState;
bool g_camPressedWhileInFirst;
float g_camWheelAdjust; // mouse wheel distance adjustment, added to offset gradually

void CAM_ToThirdPerson(void);
void CAM_ToFirstPerson(void);
bool IN_UseRawInput();
bool IN_InvertMouse();

void SDL_GetCursorPos( POINT *p )
{
//	gEngfuncs.GetMousePosition( (int *)&p->x, (int *)&p->y );
	SDL_GetMouseState( (int *)&p->x, (int *)&p->y );
}

void CL_DLLEXPORT CAM_Think( void )
{
	if( !gPlayerSim.cam_thirdperson || !gHUD.m_is_map_loaded)
		return;

	cl_entity_t* player = GetLocalPlayer();
	float now = gEngfuncs.GetClientTime();

	Vector viewangles, view_ofs;
	gEngfuncs.GetViewAngles((float*)viewangles);
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(view_ofs);

	if (g_camPressedTime && now - g_camPressedTime > 0.2f)
	{
		//iMouseInUse = 1;
		bool firstAdjust = g_camAdjustState == 0;
		if (g_camAdjustState == 0) {
			g_camAdjustState = 1;
			gEngfuncs.GetViewAngles((float*)g_camPressViewAngles);
		}
		else {
			gEngfuncs.SetViewAngles((float*)g_camPressViewAngles);
		}

		//get windows cursor position
		int mx, my, winX, winY;
		SDL_GetMouseState(&mx, &my);
		SDL_GetWindowPosition(g_sdl_window, &winX, &winY);
		int centerX = gEngfuncs.GetWindowCenterX() - winX;
		int centerY = gEngfuncs.GetWindowCenterY() - winY;
		
		int camDeltaX = centerX - mx;
		int camDeltaY = centerY - my;

#ifndef WIN32
		SDL_WarpMouseInWindow(g_sdl_window, centerX, centerY);
#else
		if (IN_UseRawInput()) {
			SDL_WarpMouseInWindow(g_sdl_window, centerX, centerY);

			if (firstAdjust) { // view jumps otherwise
				camDeltaX = camDeltaY = 0;
			}
		}
#endif

		if (IN_InvertMouse())
			camDeltaY *= -1;

		float sensitivity = gHUD.GetSensitivity();
		if (!sensitivity)
			sensitivity = 0.2f;

		g_camPressAngles.x += camDeltaY * -sensitivity;
		g_camPressAngles.y += camDeltaX * sensitivity;

		if (viewangles.x + g_camPressAngles.x > cl_pitchdown->value) {
			g_camPressAngles.x = cl_pitchdown->value - viewangles.x;
		}
		if (viewangles.x + g_camPressAngles.x < -cl_pitchup->value) {
			g_camPressAngles.x = -cl_pitchup->value - viewangles.x;
		}

		g_camPressAngles.y = normalizeRangef(g_camPressAngles.y, -180, 180);

		// zoom in/out
		if (g_camWheelAdjust) {
			float step = 500.0f * gHUD.m_flTimeDelta;
			if (g_camWheelAdjust < 0)
				step *= -step;

			if (fabs(step) > fabs(g_camWheelAdjust)) {
				step = g_camWheelAdjust;
			}

			g_camPressAngles.z += step;
			g_camWheelAdjust -= step;

			if (fabs(g_camWheelAdjust) < 0.1f) {
				g_camPressAngles.z += g_camWheelAdjust;
				g_camWheelAdjust = 0;
			}

			if (cam_idealdist->value + g_camPressAngles.z < 0) {
				g_camPressAngles.z = -cam_idealdist->value;
			}
		}
	}
	else if (g_camReleasedTime && now - g_camReleasedTime > 0.4f) {
		//iMouseInUse = 0;
		g_camAdjustState = 0;

		if (cam_snapback->value) {
			g_camPressAngles = g_camPressAngles * 0.95f;
			if (g_camPressAngles.Length() < 0.1f) {
				g_camPressAngles = Vector();
			}
		}
	}

	Vector origin = player->origin + view_ofs;
	viewangles = viewangles + g_camPressAngles + Vector(0.0f, cam_idealyaw->value, 0.0f);

	Vector camForward, camRight, camUp;
	AngleVectors(viewangles, camForward, camRight, camUp);

	float idealDist = V_min(CAM_MAX_DIST, cam_idealdist->value + g_camPressAngles.z);

	if (cam_contain->value) {
		Vector pnt = origin + camForward * -idealDist;
		pmtrace_s* trace = gEngfuncs.PM_TraceLine(origin, pnt, PM_TRACELINE_PHYSENTSONLY, 1, -1);
		idealDist *= trace->fraction;
	}

	gPlayerSim.cam_ofs[0] = viewangles.x;
	gPlayerSim.cam_ofs[1] = viewangles.y;
	gPlayerSim.cam_ofs[2] = idealDist;
}

void CAM_MouseWheeled(bool mouseWheelUpNotDown) {
	float dist = g_camPressAngles.z + cam_idealdist->value;

	float step = 16;
	if (dist < 128) {
		step = V_max(0.5f, dist * 0.05f);
	}

	g_camWheelAdjust += mouseWheelUpNotDown ? -step : step;

	if (cam_idealdist->value + g_camPressAngles.z + g_camWheelAdjust > CAM_MAX_DIST) {
		g_camWheelAdjust = 0;
	}
}

void CAM_CamButtonPress(void) {
	g_camPressedTime = gEngfuncs.GetClientTime();

	if (!gPlayerSim.cam_thirdperson) {
		CAM_ToThirdPerson();
		g_camPressedWhileInFirst = true;
	}

	g_camReleasedTime = 0;
}

void CAM_CamButtonRelease(void)
{
	g_camReleasedTime = gEngfuncs.GetClientTime();
	float timePressed = g_camReleasedTime - g_camPressedTime;

	if (timePressed < 0.2f && !g_camPressedWhileInFirst) {
		CAM_ToThirdPerson();
		g_camAdjustState = 0;
	}

	g_camPressedTime = 0;
	g_camPressedWhileInFirst = false;
}

void CAM_ToThirdPerson(void) { 
	vec3_t viewangles;

	if ( gEngfuncs.GetMaxClients() > 1 && !gHUD.IsSevenKewpServer())
	{
		// no thirdperson in multiplayer.
		return;
	}

	if (!gPlayerSim.cam_thirdperson) {
		gPlayerSim.cam_thirdperson = 1;

		gEngfuncs.GetViewAngles((float*)viewangles);

		gPlayerSim.cam_ofs[YAW] = viewangles[YAW];
		gPlayerSim.cam_ofs[PITCH] = viewangles[PITCH];
		gPlayerSim.cam_ofs[2] = CAM_MIN_DIST;
	}
	else {
		gPlayerSim.cam_thirdperson = 0;
	}
}

void CAM_ToFirstPerson(void) { 
	gPlayerSim.cam_thirdperson = 0;
	g_camAdjustState = 0;
	g_camPressAngles = Vector(0, 0, 0);
}

void CAM_Init( void )
{
	gEngfuncs.pfnAddCommand( "+thirdperson", CAM_CamButtonPress );
	gEngfuncs.pfnAddCommand( "-thirdperson", CAM_CamButtonRelease );
	gEngfuncs.pfnAddCommand( "thirdperson", CAM_ToThirdPerson );
	gEngfuncs.pfnAddCommand( "firstperson", CAM_ToFirstPerson );

	cam_snapback			= gEngfuncs.pfnRegisterVariable ( "cam_snapback", "1", 0 );	 // reset to default view after adjusting cam
	cam_idealyaw			= gEngfuncs.pfnRegisterVariable ( "cam_idealyaw", "0", 0 );	 // thirdperson yaw
	cam_idealdist			= gEngfuncs.pfnRegisterVariable ( "cam_idealdist", "90", 0 );	 // thirdperson distance
	cam_contain				= gEngfuncs.pfnRegisterVariable ( "cam_contain", "1", 0 );	// contain camera to world
}

int CL_DLLEXPORT CL_IsThirdPerson( void ) {
//	RecClCL_IsThirdPerson();

	return (gPlayerSim.cam_thirdperson ? 1 : 0) || (g_iUser1 && (g_iUser2 == GetLocalPlayer()->index) );
}

void CL_DLLEXPORT CL_CameraOffset( float *ofs ) {
//	RecClCL_GetCameraOffsets(ofs);

	VectorCopy( gPlayerSim.cam_ofs, ofs );
}
