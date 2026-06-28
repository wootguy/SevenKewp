//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

// com_weapons.h
// Shared weapons common function prototypes
#if !defined( COM_WEAPONSH )
#define COM_WEAPONSH
#ifdef _WIN32
#pragma once
#endif

#include "hud_iface.h"
#include "Exports.h"

extern uint32_t g_cmd_id; // id of the currently executed command. Doubles as a random seed for the shared rand funcs.

void			COM_Log( const char *pszFile, const char *fmt, ...);
int				CL_IsDead( void );

float			UTIL_SharedRandomFloat( unsigned int seed, float low, float high );
int				UTIL_SharedRandomLong( unsigned int seed, int low, int high );

int				HUD_GetWeaponAnim( void );
void			HUD_SendWeaponAnim( int iAnim, int body, int force );
void			HUD_PlaySound( const char *sound, float volume );
void			HUD_PlaybackEvent( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
void			HUD_SetMaxSpeed( const struct edict_s *ed, float speed );
int				stub_PrecacheModel( const char* s );
int				stub_PrecacheSound( const char* s );
unsigned short	stub_PrecacheEvent( int type, const char *s );
const char		*stub_NameForFunction	( uint32 function );
void			stub_SetModel			( struct edict_s *e, const char *m );

// toggle zoom crosshair for the given weapon
void UpdateZoomCrosshair(int weapon_id, bool zoom);

// get the global cmd timer value at the given cmd ID
uint32_t GetTimeAtCmd(uint32_t cmdId);

struct CustomWeaponParams;
struct WEAPON;
struct ViewModelSprite;
class CBasePlayerWeapon;

struct PredictionWeaponState {
	bool isCustom;
	bool ironSights;			// true if iron sights are active
	int body;
	float accuracyX[2];		// 0 = primary, 1 = secondary
	float accuracyY[2];		// 0 = primary, 1 = secondary
	bool dynamicAccuracy;	// true if the current weapon accuracy is dynamic

	bool canAkimbo;			// can this weapon activate akimbo mode?
	int akimboSeq;
	float akimboAnimTime;
	float* akimboLastEventFrame;
	bool isAkimbo;

	int clip1;					// ammo in primary clip
	int clip2;					// ammo in secondary clip
	int akimboClip;				// ammo in left hand clip in akimbo mode
	int ammo1;					// ammo in primary reserve
	int ammo2;					// ammo in secondary reserve

	CustomWeaponParams* params;	// current custom weapon params
	ViewModelSprite* v_sprite;	// non-null if current weapon is using a sprite for the view model, and that sprite is loaded
	int v_model;				// active view model index
	int stateSpriteIdx;			// frame of the state sprite to display. -1 = don't display.
};

struct PredictionState {
	local_state_t local;			// raw prediction data
	PredictionWeaponState weapon;	// active weapon
	float fov;
	int last_attack_mode;			// was the last attack a primary or secondary fire? (0 = primary, 1 = secondary)
	float last_attack_time;			// time of the last attack

	inline bool IsZoomed() { return fov != 0 && fov < 90; };
	const char* GetCustomWeaponStateString();
	const char* GetCustomWeaponChargeStatesString();
};

extern PredictionState g_prediction;

CustomWeaponParams* GetCustomWeaponParams(int id, int which); // which: 0 = active, 1 = default, 2 = alternate
bool IsExclusiveWeapon(int id);
void InitCustomWeapon(int id);
void ResetCustomWeaponStates();
void SetItemInfo(WEAPON* wep);

extern cvar_t *cl_lw;

extern int g_runfuncs;
extern struct local_state_s *g_finalstate;
extern int g_runningKickbackPred;
extern int g_irunninggausspred;
extern Vector g_vApplyVel;

#endif