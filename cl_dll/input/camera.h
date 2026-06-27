//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// Camera.h  --  defines and such for a 3rd person camera
// NOTE: must include quakedef.h first

#ifndef _CAMERA_H_
#define _CAMEA_H_

// Using third person camera
extern int g_camAdjustState;
extern int iMouseInUse;

void CAM_Init( void );
void CAM_MouseWheeled(bool mouseWheelUpNotDown);

#endif		// _CAMERA_H_
