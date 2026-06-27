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

bool IsPredictionWeaponZoomed();
CustomWeaponParams* GetCustomWeaponParams(int id, int which); // which: 0 = active, 1 = default, 2 = alternate
CustomWeaponParams* GetCurrentCustomWeaponParams();
bool IsCustomWeapon(int id);
void GetCurrentCustomWeaponAccuracy(int id, float& accuracyX, float& accuracyY,
	float& accuracyX2, float& accuracyY2, bool& dynamicAccuracy);
void GetAkimboViewModelState(studiohdr_t* header, int& seq, float& animtime, float** m_lastEventFrame);
bool CanWeaponAkimbo(int id);
bool GetPredictedAmmoCount(int id, int& clip, int& clip2, int& priamaryAmmo, int& secondaryAmmo, int& akimboClip); // false if weapon isn't predicted
CBasePlayerWeapon* GetPredictedWeapon(int id);
bool IsViewModelAkimbo();
bool IsWeaponIronSightsActive();
int GetCustomWeaponStateIconIdx(); // -1 for no icon
const char* GetCustomWeaponStateString();
const char* GetCustomWeaponChargeStatesString();
int GetCustomWeaponBody(int id);
int GetActiveCustomWeaponViewModel(); // -1 if not a custom weapon
bool IsExclusiveWeapon(int id);
void InitCustomWeapon(int id);
void ResetCustomWeaponStates();
void SetItemInfo(WEAPON* wep);
ViewModelSprite* GetSpriteWeaponState(); // return NULL if not a sprite or not loaded yet

extern cvar_t *cl_lw;

extern int g_runfuncs;
extern float g_lastFOV;
extern struct local_state_s *g_finalstate;
extern int g_runningKickbackPred;
extern int g_last_attack_mode;
extern int g_irunninggausspred;
extern float g_last_attack_time;
extern int g_last_attack_mode;
extern Vector g_vApplyVel;

#endif