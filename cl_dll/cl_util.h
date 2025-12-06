/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
// cl_util.h
//

#include "cvardef.h"

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#include <stdio.h> // for safe_sprintf()
#include <stdarg.h>  // "
#include <string.h> // for strncpy()
#include "rgb.h"
#include "shared_util.h"
#include "version.h"
#include "com_model.h"
#include "studio.h"
#include "r_studioint.h"
#include "prediction_files.h"

extern bool g_crosshair_active; // true after calling SetCrosshair with a valid crosshair

// Macros to hook function calls into the HUD object
#define HOOK_MESSAGE(x) gEngfuncs.pfnHookUserMsg(#x, __MsgFunc_##x );

#define DECLARE_MESSAGE(y, x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
							{ \
							return gHUD.y.MsgFunc_##x(pszName, iSize, pbuf ); \
							}


#define HOOK_COMMAND(x, y) gEngfuncs.pfnAddCommand( x, __CmdFunc_##y );
#define DECLARE_COMMAND(y, x) void __CmdFunc_##x( void ) \
							{ \
								gHUD.y.UserCmd_##x( ); \
							}

#define PRINTF(msg, ...) gEngfuncs.Con_Printf(msg, ##__VA_ARGS__)

#define RANDOM_LONG(low, high) gEngfuncs.pfnRandomLong(low, high)

inline float CVAR_GET_FLOAT( const char *x ) {	return gEngfuncs.pfnGetCvarFloat( (char*)x ); }
inline struct cvar_s* CVAR_GET_PTR( const char *x ) { return gEngfuncs.pfnGetCvarPointer( (char*)x ); }
inline void CVAR_SET_FLOAT( const char *x, float val ) { gEngfuncs.Cvar_SetValue( (char*)x, val ); }
inline char* CVAR_GET_STRING( const char *x ) {	return gEngfuncs.pfnGetCvarString( (char*)x ); }
inline struct cvar_s *CVAR_CREATE( const char *cv, const char *val, const int flags ) {	return gEngfuncs.pfnRegisterVariable( (char*)cv, (char*)val, flags ); }

extern int g_loadedSprites;
inline HSPRITE SPR_Load(const char* path) {
	HSPRITE spr = gEngfuncs.pfnSPR_Load(path);
	if (spr && spr > g_loadedSprites) {
		g_loadedSprites = spr;
	}
	return spr;
}
#define SPR_Set (*gEngfuncs.pfnSPR_Set)
#define SPR_Frames (*gEngfuncs.pfnSPR_Frames)
#define SPR_GetList (*gEngfuncs.pfnSPR_GetList)

// SPR_Draw  draws a the current sprite as solid
#define SPR_Draw (*gEngfuncs.pfnSPR_Draw)
// SPR_DrawHoles  draws the current sprites,  with color index255 not drawn (transparent)
#define SPR_DrawHoles (*gEngfuncs.pfnSPR_DrawHoles)
// SPR_DrawAdditive  adds the sprites RGB values to the background  (additive transulency)
#define SPR_DrawAdditive (*gEngfuncs.pfnSPR_DrawAdditive)

// SPR_EnableScissor  sets a clipping rect for HUD sprites.  (0,0) is the top-left hand corner of the screen.
#define SPR_EnableScissor (*gEngfuncs.pfnSPR_EnableScissor)
// SPR_DisableScissor  disables the clipping rect
#define SPR_DisableScissor (*gEngfuncs.pfnSPR_DisableScissor)
//
#define FillRGBA (*gEngfuncs.pfnFillRGBA)


// ScreenHeight returns the height of the screen, in pixels
#define ScreenHeight (gHUD.m_scrinfo.iHeight)
// ScreenWidth returns the width of the screen, in pixels
#define ScreenWidth (gHUD.m_scrinfo.iWidth)

#define BASE_XRES 640.f

// use this to project world coordinates to screen coordinates
#define XPROJECT(x)	( (1.0f+(x))*ScreenWidth*0.5f )
#define YPROJECT(y) ( (1.0f-(y))*ScreenHeight*0.5f )

#define XRES(x)					((x)  * ((float)ScreenWidth / 640))
#define YRES(y)					((y)  * ((float)ScreenHeight / 480))
#define XRES_HD(x)				((x)  * max(1.f, (float)ScreenWidth / 1280.f))
#define YRES_HD(y)				((y)  * max(1.f, (float)ScreenHeight / 720.f))

#define GetScreenInfo (*gEngfuncs.pfnGetScreenInfo)
#define ServerCmd (*gEngfuncs.pfnServerCmd)
#define EngineClientCmd (*gEngfuncs.pfnClientCmd)
#define EngineFilteredClientCmd (*gEngfuncs.pfnFilteredClientCmd)
#define AngleVectors (*gEngfuncs.pfnAngleVectors)

#define ARRAYSIZE(p) (sizeof(p)/sizeof(p[0]))
#define RANDOM_SOUND_ARRAY_IDX( array ) RANDOM_LONG(0,(g_soundvariety > 0 ? V_min(ARRAYSIZE( (array) ), g_soundvariety) : ARRAYSIZE( (array) ))-1)
#define RANDOM_SOUND_ARRAY( array ) (array) [ RANDOM_SOUND_ARRAY_IDX(array) ]

inline void SetCrosshair(HSPRITE hspr, wrect_t rc, int r, int g, int b) {
	gEngfuncs.pfnSetCrosshair(hspr, rc, r, g, b);
	g_crosshair_active = rc.bottom != 0 || rc.left != 0 || rc.right != 0 || rc.top != 0;
}

// prevent crashes when map is not loaded
inline cl_entity_t* GetLocalPlayer() {
	static cl_entity_t dummyPlayer;
	return gHUD.m_is_map_loaded ? gEngfuncs.GetLocalPlayer() : &dummyPlayer;
}

extern engine_studio_api_t IEngineStudio;

inline studiohdr_t* GetStudioModel(cl_entity_t* ent) {
	if (ent && ent->model)
		return (studiohdr_t*)IEngineStudio.Mod_Extradata(ent->model);
	return NULL;
}


// Gets the height & width of a sprite,  at the specified frame
inline int SPR_Height( HSPRITE x, int f )	{ return gEngfuncs.pfnSPR_Height(x, f); }
inline int SPR_Width( HSPRITE x, int f )	{ return gEngfuncs.pfnSPR_Width(x, f); }

inline 	client_textmessage_t	*TextMessageGet( const char *pName ) { return gEngfuncs.pfnTextMessageGet( pName ); }
inline 	int						TextMessageDrawChar( int x, int y, int number, int r, int g, int b ) 
{ 
	return gEngfuncs.pfnDrawCharacter( x, y, number, r, g, b ); 
}

inline void SetConsoleTextColor(int r, int g, int b)
{
	gEngfuncs.pfnDrawSetTextColor(r / 255.0f, g / 255.0f, b / 255.0f);
}

inline int DrawConsoleString(int x, int y, const char* string, RGB color)
{
	gEngfuncs.pfnDrawSetTextColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);
	return gEngfuncs.pfnDrawConsoleString(x, y, (char*)string);
}

inline int DrawConsoleString( int x, int y, const char *string )
{
	return gEngfuncs.pfnDrawConsoleString( x, y, (char*) string );
}

inline void GetConsoleStringSize( const char *string, int *width, int *height )
{
	gEngfuncs.pfnDrawConsoleStringLen( string, width, height );
}

inline int ConsoleStringLen( const char *string )
{
	int _width, _height;
	GetConsoleStringSize( string, &_width, &_height );
	return _width;
}

inline void ConsolePrint( const char *string )
{
	gEngfuncs.pfnConsolePrint( string );
}

inline void CenterPrint( const char *string )
{
	gEngfuncs.pfnCenterPrint( string );
}


inline char *safe_strcpy( char *dst, const char *src, int len_dst)
{
	if( len_dst <= 0 )
	{
		return NULL; // this is bad
	}

	strncpy(dst,src,len_dst);
	dst[ len_dst - 1 ] = '\0';

	return dst;
}

inline int safe_sprintf( char *dst, int len_dst, const char *format, ...)
{
	if( len_dst <= 0 )
	{
		return -1; // this is bad
	}

	va_list v;

    va_start(v, format);

	_vsnprintf(dst,len_dst,format,v);

	va_end(v);

	dst[ len_dst - 1 ] = '\0';

	return 0;
}

// sound functions
inline void PlaySound( const char *szSound, float vol ) { gEngfuncs.pfnPlaySoundByName( szSound, vol ); }
inline void PlaySound( int iSound, float vol ) { gEngfuncs.pfnPlaySoundByIndex( iSound, vol ); }

#define fabs(x)	   ((x) > 0 ? (x) : 0 - (x))

void ScaleColors( int &r, int &g, int &b, int a );

#define DotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorSubtract(a,b,c) {(c)[0]=(a)[0]-(b)[0];(c)[1]=(a)[1]-(b)[1];(c)[2]=(a)[2]-(b)[2];}
#define VectorAdd(a,b,c) {(c)[0]=(a)[0]+(b)[0];(c)[1]=(a)[1]+(b)[1];(c)[2]=(a)[2]+(b)[2];}
#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
inline void VectorClear(float *a) { a[0]=0.0;a[1]=0.0;a[2]=0.0;}
float Length(const float *v);
void VectorMA (const float *veca, float scale, const float *vecb, float *vecc);
void VectorScale (const float *in, float scale, float *out);
float VectorNormalize (float *v);
void VectorInverse ( float *v );

extern vec3_t vec3_origin;

// disable 'possible loss of data converting float to int' warning message
#pragma warning( disable: 4244 )
// disable 'truncation from 'const double' to 'float' warning message
#pragma warning( disable: 4305 )

inline void UnpackRGB(int &r, int &g, int &b, unsigned long ulRGB)\
{\
	r = (ulRGB & 0xFF0000) >>16;\
	g = (ulRGB & 0xFF00) >> 8;\
	b = ulRGB & 0xFF;\
}

HSPRITE LoadSprite(const char *pszName);

// not relative to a game dir
bool fileExists(const char* path);

// searches game directories in priority order
// returns NULL if file is not found
const char* FindGameFile(const char* path);

// convert a world space point to screen space
Vector WorldToScreen(const Vector& P, const Vector& viewerOrigin, const Vector& viewerAngles, float fovXDeg);

// convert a world space point to screen space for the local player
Vector WorldToScreen(const Vector& P);
