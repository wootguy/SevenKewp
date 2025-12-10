/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
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
#pragma once
#include "archtypes.h"     // DAL

//
// Misc utility code
//
#ifndef ACTIVITY_H
#include "monster/activity.h"
#endif

#ifndef ENGINECALLBACK_H
#include "enginecallback.h"
#endif

#include <vector>
#include <string>
#include "game.h"
#include <unordered_map>
#include <string>
#include "mstream.h"
#include <float.h>
#include "PluginHooks.h"
#include "shared_util.h"
#include "studio.h"
#include "debug.h"
#include "eng_wrappers.h"
#include "wav.h"
#include "ThreadSafeQueue.h"
#include <thread>
#include "HashMap.h"
#include "version.h"
#include "water.h"

class CBasePlayer;

extern EXPORT globalvars_t				*gpGlobals;

extern StringSet g_weaponNames; // names given by weapons (may have a prefix: "hlcoop/weapon_grapple")
extern StringSet g_weaponClassnames; // valid weapon classnames
extern uint64_t g_weaponSlotMasks[MAX_WEAPONS]; // for handling slot conflict. maps an ID to a bitfield representing all IDs that share the weapon slot

// rmaps a half-life weapon to a sevenkewp weapon.
// When a sevenkewp player picks up one of these hl weapons, they'll get the remapped weapon instead.
// This also applies to weapons dropped by NPCs or created with CreateEntity()
EXPORT extern StringMap g_weaponRemapHL;

extern int g_serveractive; // 1 if ServerActivate was called (no longer safe to precache)
extern bool g_can_set_bsp_models; // false if bsp models aren't precached yet (skip SET_MODEL calls)
extern int g_edictsinit; // 1 if all edicts were allocated so that relocations can begin

extern HashMap<int> g_admins;

EXPORT extern std::string g_mp3Command; // current global mp3 command
EXPORT extern bool g_seriesMusic; // true if music should keep playing after level change within the same series

extern TYPEDESCRIPTION	gEntvarsDescription[];
extern const int ENTVARS_COUNT;

struct AlertMsgCall {
	ALERT_TYPE atype;
	std::string msg;
};

EXPORT extern std::thread::id g_main_thread_id;
extern ThreadSafeQueue<AlertMsgCall> g_thread_prints;

EXPORT extern std::string g_lastMapName;

enum AdminLevel {
	ADMIN_NO,
	ADMIN_YES,
	ADMIN_OWNER
};

enum distant_sound_types {
	DISTANT_NONE,
	DISTANT_9MM, // light tapping noise
	DISTANT_357, // deeper tap
	DISTANT_556, // deep tap / small explosion
	DISTANT_BOOM // big explosion
};

enum merged_item_bodies {
	MERGE_MDL_W_9MMAR,
	MERGE_MDL_W_9MMARCLIP,
	MERGE_MDL_W_9MMCLIP,
	MERGE_MDL_W_9MMHANDGUN,
	MERGE_MDL_W_357,
	MERGE_MDL_W_357AMMOBOX,
	MERGE_MDL_W_ARGRENADE,
	MERGE_MDL_W_BATTERY,
	MERGE_MDL_W_BGRAP,
	MERGE_MDL_W_CHAINAMMO,
	MERGE_MDL_W_CROSSBOW,
	MERGE_MDL_W_CROSSBOW_CLIP,
	MERGE_MDL_W_CROWBAR,
	MERGE_MDL_W_DISPLACER,
	MERGE_MDL_W_EGON,
	MERGE_MDL_W_GAUSS,
	MERGE_MDL_W_GAUSSAMMO,
	MERGE_MDL_W_GRENADE,
	MERGE_MDL_W_HGUN,
	MERGE_MDL_W_LONGJUMP,
	MERGE_MDL_W_MEDKIT,
	MERGE_MDL_W_PIPE_WRENCH,
	MERGE_MDL_W_RPG,
	MERGE_MDL_W_RPGAMMO,
	MERGE_MDL_W_SATCHEL,
	MERGE_MDL_W_SECURITY,
	MERGE_MDL_W_SHOTBOX,
	MERGE_MDL_W_SHOTGUN,
	MERGE_MDL_W_SHOTSHELL,
	MERGE_MDL_W_SUIT,
	MERGE_MDL_W_WEAPONBOX,
	MERGE_MDL_GRENADE,
	MERGE_MDL_HVR,
	MERGE_MDL_RPGROCKET,
	MERGE_MDL_SPORE,
	MERGE_MDL_SHOCK_EFFECT,
	MERGE_MDL_W_2UZIS,
	MERGE_MDL_W_UZI,
	MERGE_MDL_W_UZI_CLIP,
	MERGE_MDL_W_SAW,
	MERGE_MDL_W_SAW_CLIP,
	MERGE_MDL_W_DESERT_EAGLE,
	MERGE_MDL_W_M40A1,
	MERGE_MDL_W_M40A1_CLIP,
	MERGE_MDL_W_MINIGUN,
	MERGE_MDL_W_KNIFE,
	MERGE_MDL_W_M16,
	MERGE_MDL_W_PMEDKIT,
	MERGE_MDL_CAMERA,
};

// Use this instead of ALLOC_STRING on constant strings
#define STRING(offset)		((const char *)(gpGlobals->pStringBase + (unsigned int)(offset)))
#define MAKE_STRING(str)	((uint64)(str) - (uint64)(STRING(0))) // NEVER USE THIS IN PLUGINS

// swap 2 values
#define SWAP(a, b, T) { \
	T _temp = (a); \
	(a) = (b); \
	(b) = _temp; \
}

// Prefixes the message with the "targetname (classname): " for ease of debugging
// Only usable inside entity class methods.
#define EALERT(target, format, ...) \
	DEBUG_MSG(target, ("%s (%s): " + std::string(format)).c_str(), \
		STRING(pev->targetname), STRING(pev->classname), ##__VA_ARGS__)

extern std::unordered_map<std::string, StringMap> g_replacementFiles;

struct MessageHistoryItem {
	float startTime;
	float endTime;
};

#define MAX_TEXT_CHANNELS 4
extern MessageHistoryItem g_hudMsgHistory[MAX_TEXT_CHANNELS*33];

// same as the STRING macro but defined as a function for easy calling in the debugger
EXPORT const char* cstr(string_t s);

inline edict_t *FIND_ENTITY_BY_CLASSNAME(edict_t *entStart, const char *pszName) 
{
	return FIND_ENTITY_BY_STRING(entStart, "classname", pszName);
}	

// for doing a reverse lookup. Say you have a door, and want to find its button.
// Only checks the "target" keyvalue, so this skips multi_manager and other types of target keys
inline edict_t *FIND_ENTITY_BY_TARGET(edict_t *entStart, const char *pszName) 
{
	return FIND_ENTITY_BY_STRING(entStart, "target", pszName);
}	

// Keeps clutter down a bit, when writing key-value pairs
#define WRITEKEY_INT(pf, szKeyName, iKeyValue) ENGINE_FPRINTF(pf, "\"%s\" \"%d\"\n", szKeyName, iKeyValue)
#define WRITEKEY_FLOAT(pf, szKeyName, flKeyValue)								\
		ENGINE_FPRINTF(pf, "\"%s\" \"%f\"\n", szKeyName, flKeyValue)
#define WRITEKEY_STRING(pf, szKeyName, szKeyValue)								\
		ENGINE_FPRINTF(pf, "\"%s\" \"%s\"\n", szKeyName, szKeyValue)
#define WRITEKEY_VECTOR(pf, szKeyName, flX, flY, flZ)							\
		ENGINE_FPRINTF(pf, "\"%s\" \"%f %f %f\"\n", szKeyName, flX, flY, flZ)

// Keeps clutter down a bit, when using a float as a bit-vector
#define SetBits(flBitVector, bits)		((flBitVector) = (int)(flBitVector) | (bits))
#define ClearBits(flBitVector, bits)	((flBitVector) = (int)(flBitVector) & ~(bits))
#define FBitSet(flBitVector, bit)		((int)(flBitVector) & (bit))

// Makes these more explicit, and easier to find
#define FILE_GLOBAL static
#define DLL_GLOBAL

// Until we figure out why "const" gives the compiler problems, we'll just have to use
// this bogus "empty" define to mark things as constant.
#define CONSTANT

// More explicit than "int"
typedef int EOFFSET;

// In case it's not alread defined
typedef int BOOL;

// Keeps clutter down a bit, when declaring external entity/global method prototypes
#define DECLARE_GLOBAL_METHOD(MethodName)  extern void DLLEXPORT MethodName( void )
#define GLOBAL_METHOD(funcname)					void DLLEXPORT funcname(void)

typedef void (*SpawnFunc)(entvars_t* pev);

// used by plugins to remap entities to other existing ones
EXPORT extern HashMap<SpawnFunc> g_entityRemap;

#ifdef PLUGIN_NAME
// This is the glue that hooks .MAP entity class names to our CPP classes
// The _declspec forces them to be exported by name so we can do a lookup with GetProcAddress()
// The function is used to intialize / allocate the object for the entity
#define LINK_ENTITY_TO_CLASS(mapClassName,DLLClassName) \
	extern "C" DLLEXPORT void mapClassName( entvars_t *pev ); \
	void mapClassName( entvars_t *pev ) { GetClassPtr( (DLLClassName *)pev ); }
#else
// The non-plugin version allows remapping to classes defined in plugins.
// Plugins can't also use this because it casues infinite loops.
#define LINK_ENTITY_TO_CLASS(mapClassName,DLLClassName) \
	extern "C" DLLEXPORT void mapClassName( entvars_t *pev ); \
	void mapClassName( entvars_t *pev ) { \
		SpawnFunc* remap = g_entityRemap.get(#mapClassName); \
		if (remap) \
			(*remap)(pev); \
		else \
			GetClassPtr( (DLLClassName *)pev ); \
	}
#endif

//
// Conversion among the three types of "entity", including identity-conversions.
//
#if defined(DEBUG) && !defined(PLUGIN_BUILD)
	EXPORT edict_t *DBG_EntOfVars(const entvars_t *pev);
	inline edict_t *ENT(const entvars_t *pev)	{ return DBG_EntOfVars(pev); }
#else
	inline edict_t *ENT(const entvars_t *pev)	{ return pev->pContainingEntity; }
#endif
inline edict_t *ENT(edict_t *pent)		{ return pent; }
inline edict_t *ENT(EOFFSET eoffset)			{ return (*g_engfuncs.pfnPEntityOfEntOffset)(eoffset); }
inline EOFFSET OFFSET(EOFFSET eoffset)			{ return eoffset; }
inline EOFFSET OFFSET(const edict_t *pent)	
{ 
#if _DEBUG
	if ( !pent )
		ALERT( at_error, "Bad ent in OFFSET()\n" );
#endif
	return (*g_engfuncs.pfnEntOffsetOfPEntity)(pent); 
}
inline EOFFSET OFFSET(entvars_t *pev)				
{ 
#if _DEBUG
	if ( !pev )
		ALERT( at_error, "Bad pev in OFFSET()\n" );
#endif
	return OFFSET(ENT(pev)); 
}
inline entvars_t *VARS(entvars_t *pev)					{ return pev; }

inline entvars_t *VARS(edict_t *pent)			
{ 
	if ( !pent )
		return NULL;

	return &pent->v; 
}

inline entvars_t* VARS(EOFFSET eoffset)			{ return VARS(ENT(eoffset)); }
inline int	  ENTINDEX(const edict_t *pEdict)			{ return (*g_engfuncs.pfnIndexOfEdict)(pEdict); }
inline edict_t* INDEXENT( int iEdictNum )		{ return (*g_engfuncs.pfnPEntityOfEntIndex)(iEdictNum); }
inline uint32_t PLRBIT(const edict_t* pEdict)			{ return 1 << (ENTINDEX(pEdict) & 31); }
inline uint32_t PLRBIT(uint32_t idx)			{ return 1 << (idx & 31); }
inline void MESSAGE_BEGIN( int msg_dest, int msg_type, const float *pOrigin, entvars_t *ent ) {
	MESSAGE_BEGIN(msg_dest, msg_type, pOrigin, ENT(ent));
}
EXPORT void WRITE_BYTES(uint8_t* bytes, int count);
EXPORT void WRITE_FLOAT(float val);

// Testing the three types of "entity" for nullity
#define eoNullEntity 0
inline BOOL FNullEnt(EOFFSET eoffset)			{ return eoffset == 0; }
inline BOOL FNullEnt(const edict_t* pent)	{ return pent == NULL || FNullEnt(OFFSET(pent)); }
inline BOOL FNullEnt(entvars_t* pev)				{ return pev == NULL || FNullEnt(OFFSET(pev)); }

// Testing strings for nullity
#define iStringNull 0
inline BOOL FStringNull(int iString)			{ return iString == iStringNull; }

#define cchMapNameMost 32

// Dot products for view cone checking
#define VIEW_FIELD_FULL		(float)-1.0 // +-180 degrees
#define	VIEW_FIELD_WIDE		(float)-0.7 // +-135 degrees 0.1 // +-85 degrees, used for full FOV checks 
#define	VIEW_FIELD_NARROW	(float)0.7 // +-45 degrees, more narrow check used to set up ranged attacks
#define	VIEW_FIELD_ULTRA_NARROW	(float)0.9 // +-25 degrees, more narrow check used to set up ranged attacks

// All monsters need this data
#define		DONT_BLEED			-1
#define		BLOOD_COLOR_RED		(BYTE)247
#define		BLOOD_COLOR_DARK_RED (BYTE)224
#define		BLOOD_COLOR_YELLOW	(BYTE)195
#define		BLOOD_COLOR_GREEN	BLOOD_COLOR_YELLOW

typedef enum 
{

	MONSTERSTATE_NONE = 0,
	MONSTERSTATE_IDLE,
	MONSTERSTATE_COMBAT,
	MONSTERSTATE_ALERT,
	MONSTERSTATE_HUNT,
	MONSTERSTATE_PRONE,
	MONSTERSTATE_SCRIPT,
	MONSTERSTATE_PLAYDEAD,
	MONSTERSTATE_DEAD

} MONSTERSTATE;

// Misc useful
inline BOOL FStrEq(const char*sz1, const char*sz2)
	{ return (strcmp(sz1, sz2) == 0); }
inline BOOL FClassnameIs(edict_t* pent, const char* szClassname)
	{ return FStrEq(STRING(VARS(pent)->classname), szClassname); }
inline BOOL FClassnameIs(entvars_t* pev, const char* szClassname)
	{ return FStrEq(STRING(pev->classname), szClassname); }

class CBaseEntity;

// Misc. Prototypes
EXPORT void			UTIL_SetSize			(entvars_t* pev, const Vector &vecMin, const Vector &vecMax);
EXPORT float		UTIL_VecToYaw			(const Vector &vec);
EXPORT Vector		UTIL_VecToAngles		(const Vector &vec);
EXPORT Vector		UTIL_VecToSpriteAngles	(const Vector &vec); // for oriented sprites
EXPORT float		UTIL_AngleMod			(float a);
EXPORT float		UTIL_AngleDiff			( float destAngle, float srcAngle );

EXPORT CBaseEntity	*UTIL_FindEntityInSphere(CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius);
EXPORT CBaseEntity	*UTIL_FindEntityByString(CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue );
EXPORT CBaseEntity	*UTIL_FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName );
EXPORT CBaseEntity	*UTIL_FindEntityByTargetname(CBaseEntity *pStartEntity, const char *szName );
EXPORT CBaseEntity	*UTIL_FindEntityGeneric(const char *szName, Vector &vecSrc, float flRadius );
EXPORT CBaseEntity	*UTIL_FindEntityClassByTargetname(CBaseEntity* pStartEntity, const char* szClass, const char* szName);

// returns a CBaseEntity pointer to a player by index.  Only returns if the player is spawned and connected
// otherwise returns NULL
// Index is 1 based
EXPORT extern CBasePlayer	*UTIL_PlayerByIndex( int playerIndex );
EXPORT extern CBasePlayer	*UTIL_PlayerByUserId( int userid );
EXPORT extern CBasePlayer	*UTIL_PlayerBySteamId(const char* id);
EXPORT extern CBasePlayer	*UTIL_PlayerBySteamId64(uint64_t id);
EXPORT extern CBasePlayer* UTIL_PlayerBySearchString(const char* search, CBasePlayer* ignorePlayer, bool& multipleMatches);

#define UTIL_EntitiesInPVS(pent)			(*g_engfuncs.pfnEntitiesInPVS)(pent)
EXPORT edict_t*		UTIL_ClientsInPVS(edict_t* edict, int& playerCount);
EXPORT bool			UTIL_IsClientInPVS(edict_t* edict); // faster than UTIL_ClientsInPVS
EXPORT void			UTIL_MakeVectors(const Vector& vecAngles);
EXPORT bool			IsValidPlayer(edict_t* edict); // true if edict is a connected player

// Pass in an array of pointers and an array size, it fills the array and returns the number inserted
EXPORT int UTIL_MonstersInSphere( CBaseEntity **pList, int listMax, const Vector &center, float radius );
EXPORT int UTIL_EntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs,
	int flagMask, bool ignoreDead, bool collisionOnly=false);

inline void UTIL_MakeVectorsPrivate( const Vector &vecAngles, float *p_vForward, float *p_vRight, float *p_vUp )
{
	g_engfuncs.pfnAngleVectors( vecAngles, p_vForward, p_vRight, p_vUp );
}

EXPORT void			UTIL_MakeAimVectors		( const Vector &vecAngles ); // like MakeVectors, but assumes pitch isn't inverted
EXPORT void			UTIL_MakeInvVectors		( const Vector &vec, globalvars_t *pgv );

EXPORT void			UTIL_SetOrigin			( entvars_t* pev, const Vector &vecOrigin ); // Needs to be called any time an entity changes origin, mins, maxs, or solid
EXPORT void			UTIL_EmitAmbientSound	( edict_t *entity, const float* vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch, edict_t* dest=NULL);
EXPORT void			UTIL_ParticleEffect		( const Vector &vecOrigin, const Vector &vecDirection, ULONG ulColor, ULONG ulCount );
EXPORT void			UTIL_ScreenShake		( const Vector &center, float amplitude, float frequency, float duration, float radius );
EXPORT void			UTIL_ScreenShake		( CBasePlayer* plr, float amplitude, float frequency, float duration );
EXPORT void			UTIL_ScreenShakeAll		( const Vector &center, float amplitude, float frequency, float duration );
EXPORT void			UTIL_ShowMessage		( const char *pString, CBaseEntity *pPlayer );
EXPORT void			UTIL_ShowMessageAll		( const char *pString );
EXPORT void			UTIL_ScreenFadeAll		( const Vector &color, float fadeTime, float holdTime, int alpha, int flags );
EXPORT void			UTIL_ScreenFade			( CBaseEntity *pEntity, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags, bool reliable=true );

// duplicate of the engine function with the ability to change the message mode and target entity
EXPORT void ambientsound_msg(edict_t* entity, float* pos, const char* samp, float vol, float attenuation,
	int fFlags, int pitch, int msgDst, edict_t* dest);

// leave target NULL to play music for all players
EXPORT void UTIL_PlayGlobalMp3(const char* path, bool loop, edict_t* target=NULL);

// leave target NULL to stop music for all players
EXPORT void UTIL_StopGlobalMp3(edict_t* target=NULL);

typedef enum { ignore_monsters=1, dont_ignore_monsters=0, missile=2 } IGNORE_MONSTERS;
typedef enum { ignore_glass=1, dont_ignore_glass=0 } IGNORE_GLASS;
EXPORT void			UTIL_TraceLine			(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr);
EXPORT void			UTIL_TraceLine			(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr);
enum { point_hull=0, human_hull=1, large_hull=2, head_hull=3 };
EXPORT void			UTIL_TraceHull			(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr);
EXPORT TraceResult	UTIL_GetGlobalTrace		(void);
EXPORT void			UTIL_TraceModel			(const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr);
EXPORT Vector		UTIL_GetAimVector		(edict_t* pent, float flSpeed);
EXPORT bool			UTIL_PointInBox(const Vector& vec, Vector mins, Vector maxs);

EXPORT int			UTIL_IsMasterTriggered	(string_t sMaster, CBaseEntity *pActivator);
EXPORT Vector		UTIL_RandomBloodVector( void );
EXPORT BOOL			UTIL_ShouldShowBlood( int bloodColor );
EXPORT void			UTIL_StringToVector( float *pVector, const char *pString );
EXPORT bool			UTIL_StringIsVector( const char *pString );
EXPORT const char*	UTIL_VectorToString(const Vector& v);
EXPORT void			UTIL_StringToIntArray( int *pVector, int count, const char *pString );
EXPORT Vector		UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize );
EXPORT float		UTIL_Approach( float target, float value, float speed );
EXPORT float		UTIL_ApproachAngle( float target, float value, float speed );
EXPORT float		UTIL_AngleDistance( float next, float cur );
EXPORT bool			UTIL_IsValidTempEntOrigin( const Vector& v ); // true if the origin can be used with temporary entity effects TE_*

EXPORT void			UTIL_Remove( CBaseEntity *pEntity );
EXPORT BOOL			UTIL_IsValidEntity( edict_t *pent );
EXPORT BOOL			UTIL_TeamsMatch( const char *pTeamName1, const char *pTeamName2 );

// Use for ease-in, ease-out style interpolation (accel/decel)
EXPORT float		UTIL_SplineFraction( float value, float scale );

// Search for water transition along a vertical line
EXPORT float		UTIL_WaterLevel( const Vector &position, float minz, float maxz );

// allows precacheing of other entities
EXPORT void			UTIL_PrecacheOther( const char *szClassname, const StringMap& keys=g_emptyStringMap);

// prints a message to the client.
EXPORT void			UTIL_ClientPrint(CBaseEntity* client, PRINT_TYPE print_type, const char* msg);
EXPORT void			UTIL_ClientPrintAll(PRINT_TYPE print_type, const char *msg);

// handles coloring and name prefix. Don't include the newline
EXPORT void			UTIL_ClientSay(CBasePlayer* plr, const char* text, const char* customPrefix=NULL,
						bool teamMessage=false, edict_t* target=NULL, uint32_t mutes=0, int customColor=-1, const char* customName=NULL);

// print a console font HUD message to the client
EXPORT void UTIL_ClientHudConPrint(CBaseEntity* client, const hudconparms_t& params, const char* msg, bool reliable = true);

// print a console font HUD message to all clients
EXPORT void UTIL_ClientHudConPrintAll(const hudconparms_t& params, const char* msg, bool reliable = true);

// Add a sprite to the player's HUD
// sprite = either an icon set to find in hud.txt, or a file path which excludes "sprites/" and ".spr"
EXPORT void UTIL_HudCustomSprite(CBasePlayer* client, const HUDSpriteParams& params, const char* sprite, bool reliable=true);

// Shows a custom numeric display to a given player or to all players if client is not specified.
EXPORT void UTIL_HudNumDisplay(CBasePlayer* client, const HUDNumDisplayParams& params, bool reliable=true);

// update number for existing HUD numeric display
EXPORT void UTIL_HudUpdateNum(CBasePlayer* client, uint8_t channel, float newVal, bool reliable=true);

// Toggle HUD sprite visibility
EXPORT void UTIL_HudToggleElement(CBasePlayer* plr, uint8_t channel, bool visible);

class CBasePlayerItem;
class CBasePlayer;
EXPORT BOOL UTIL_GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );

typedef struct hudtextparms_s
{
	float		x;
	float		y;
	int			effect;
	byte		r1, g1, b1, a1;
	byte		r2, g2, b2, a2;
	float		fadeinTime;
	float		fadeoutTime;
	float		holdTime;
	float		fxTime;
	int			channel; // -1 = automatic (director message mode)
} hudtextparms_t;

// prints as transparent 'title' to the HUD
EXPORT void			UTIL_HudMessageAll( const hudtextparms_t &textparms, const char *pMessage, int msgMode = MSG_ALL );
EXPORT void			UTIL_HudMessage( CBaseEntity *pEntity, const hudtextparms_t &textparms, const char *pMessage, int msgMode = MSG_ONE);

// for handy use with ClientPrint params
EXPORT char *UTIL_dtos1( int d );
EXPORT char *UTIL_dtos2( int d );
EXPORT char *UTIL_dtos3( int d );
EXPORT char *UTIL_dtos4( int d );

// Writes message to console with player info as a prefix
EXPORT void	UTIL_LogPlayerEvent( const edict_t* plr, const char *fmt, ... );

// Sorta like FInViewCone, but for nonmonsters. 
EXPORT float UTIL_DotPoints ( const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir );

EXPORT void UTIL_StripToken( const char *pKey, char *pDest, int nLen);// for redundant keynames

// Misc functions
EXPORT extern void FireTarget(CBaseEntity* pTarget, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value=0.0f);
EXPORT extern void FireTargets(const char* targetName, CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value=0.0f, float delay=0.0f);
void FireTargetsDelayed(const char* target, string_t killTarget, CBaseEntity* pActivator, USE_TYPE useType, float delay);
EXPORT void SetMovedir(entvars_t* pev);
EXPORT Vector VecBModelOrigin( entvars_t* pevBModel );
EXPORT int BuildChangeList( LEVELLIST *pLevelList, int maxList );

EXPORT extern const Vector g_vecZero;
EXPORT extern string_t g_debug_target;

//
// Constants that were used only by QC (maybe not used at all now)
//
// Un-comment only as needed
//
#define LANGUAGE_ENGLISH				0
#define LANGUAGE_GERMAN					1
#define LANGUAGE_FRENCH					2
#define LANGUAGE_BRITISH				3

extern DLL_GLOBAL int			g_Language;

#define AMBIENT_SOUND_STATIC			0	// medium radius attenuation
#define AMBIENT_SOUND_EVERYWHERE		1
#define AMBIENT_SOUND_SMALLRADIUS		2
#define AMBIENT_SOUND_MEDIUMRADIUS		4
#define AMBIENT_SOUND_LARGERADIUS		8
#define AMBIENT_SOUND_START_SILENT		16
#define AMBIENT_SOUND_NOT_LOOPING		32

#define SPEAKER_START_SILENT			1	// wait for trigger 'on' to start announcements

#define	LFO_SQUARE			1
#define LFO_TRIANGLE		2
#define LFO_RANDOM			3

// func_rotating
#define SF_BRUSH_ROTATE_Y_AXIS		0
#define SF_BRUSH_ROTATE_INSTANT		1
#define SF_BRUSH_ROTATE_BACKWARDS	2
#define SF_BRUSH_ROTATE_Z_AXIS		4
#define SF_BRUSH_ROTATE_X_AXIS		8
#define SF_PENDULUM_AUTO_RETURN		16
#define	SF_PENDULUM_PASSABLE		32


#define SF_BRUSH_ROTATE_SMALLRADIUS	128
#define SF_BRUSH_ROTATE_MEDIUMRADIUS 256
#define SF_BRUSH_ROTATE_LARGERADIUS 512

#define PUSH_BLOCK_ONLY_X	1
#define PUSH_BLOCK_ONLY_Y	2

#define VEC_HULL_MIN		Vector(-16, -16, -36)
#define VEC_HULL_MAX		Vector( 16,  16,  36)
#define VEC_HUMAN_HULL_MIN	Vector( -16, -16, 0 )
#define VEC_HUMAN_HULL_MAX	Vector( 16, 16, 72 )
#define VEC_HUMAN_HULL_DUCK	Vector( 16, 16, 36 )

#define VEC_VIEW			Vector( 0, 0, 28 )

#define VEC_DUCK_HULL_MIN	Vector(-16, -16, -18 )
#define VEC_DUCK_HULL_MAX	Vector( 16,  16,  18)
#define VEC_DUCK_VIEW		Vector( 0, 0, 12 )

enum svc_commands_e
{
	SVC_BAD,
	SVC_NOP,
	SVC_DISCONNECT,
	SVC_EVENT,
	SVC_VERSION,
	SVC_SETVIEW,
	SVC_SOUND,
	SVC_TIME,
	SVC_PRINT,
	SVC_STUFFTEXT,
	SVC_SETANGLE,
	SVC_SERVERINFO,
	SVC_LIGHTSTYLE,
	SVC_UPDATEUSERINFO,
	SVC_DELTADESCRIPTION,
	SVC_CLIENTDATA,
	SVC_STOPSOUND,
	SVC_PINGS,
	SVC_PARTICLE,
	SVC_DAMAGE,
	SVC_SPAWNSTATIC,
	SVC_EVENT_RELIABLE,
	SVC_SPAWNBASELINE,
	SVC_TEMPENTITY,
	SVC_SETPAUSE,
	SVC_SIGNONNUM,
	SVC_CENTERPRINT,
	SVC_KILLEDMONSTER,
	SVC_FOUNDSECRET,
	SVC_SPAWNSTATICSOUND,
	SVC_INTERMISSION,
	SVC_FINALE,
	SVC_CDTRACK,
	SVC_RESTORE,
	SVC_CUTSCENE,
	SVC_WEAPONANIM,
	SVC_DECALNAME,
	SVC_ROOMTYPE,
	SVC_ADDANGLE,
	SVC_NEWUSERMSG,
	SVC_PACKETENTITIES,
	SVC_DELTAPACKETENTITIES,
	SVC_CHOKE,
	SVC_RESOURCELIST,
	SVC_NEWMOVEVARS,
	SVC_RESOURCEREQUEST,
	SVC_CUSTOMIZATION,
	SVC_CROSSHAIRANGLE,
	SVC_SOUNDFADE,
	SVC_FILETXFERFAILED,
	SVC_HLTV,
	SVC_DIRECTOR,
	SVC_VOICEINIT,
	SVC_VOICEDATA,
	SVC_SENDEXTRAINFO,
	SVC_TIMESCALE,
	SVC_RESOURCELOCATION,
	SVC_SENDCVARVALUE,
	SVC_SENDCVARVALUE2,
	SVC_EXEC
};

// triggers
#define	SF_TRIGGER_ALLOWMONSTERS	1// monsters allowed to fire this trigger
#define	SF_TRIGGER_NOCLIENTS		2// players not allowed to fire this trigger
#define SF_TRIGGER_PUSHABLES		4// only pushables can fire this trigger
#define SF_TRIGGER_EVERYTHING_ELSE	8// anything can trigger

// func breakable
#define SF_BREAK_TRIGGER_ONLY	1// may only be broken by trigger
#define	SF_BREAK_TOUCH			2// can be 'crashed through' by running player (plate glass)
#define SF_BREAK_PRESSURE		4// can be broken by a player standing on it
#define SF_BREAK_REPAIRABLE		8// can be broken by a player standing on it
#define SF_BREAK_INSTANT		256// instant break if hit with crowbar or wrench
#define SF_BREAK_EXPLOSIVES_ONLY	512 // only damaged by DMG_BLAST

// func_pushable (it's also func_breakable, so don't collide with those flags)
#define SF_PUSH_BREAKABLE		128
#define SF_PUSH_LIFTABLE		1024

#define SF_LIGHT_START_OFF		1

#define SPAWNFLAG_NOMESSAGE	1
#define SPAWNFLAG_NOTOUCH	1
#define SPAWNFLAG_DROIDONLY	4

#define SPAWNFLAG_USEONLY	1		// can't be touched, must be used (buttons)

#define TELE_PLAYER_ONLY	1
#define TELE_SILENT			2

#define SF_TRIG_PUSH_ONCE		1


// Sound Utilities

// sentence groups
#define CBSENTENCENAME_MAX 16
#define CVOXFILESENTENCEMAX		2048		// max number of sentences in game. NOTE: this must match
											// CVOXFILESENTENCEMAX in engine\sound.h!!!

extern char gszallsentencenames[CVOXFILESENTENCEMAX][CBSENTENCENAME_MAX];
extern int gcallsentences;

EXPORT int USENTENCEG_Pick(int isentenceg, char *szfound);
EXPORT int USENTENCEG_PickSequential(int isentenceg, char *szfound, int ipick, int freset);
EXPORT void USENTENCEG_InitLRU(unsigned char *plru, int count);

EXPORT void SENTENCEG_Init();
EXPORT void SENTENCEG_Stop(edict_t *entity, int isentenceg, int ipick);
EXPORT int SENTENCEG_PlayRndI(edict_t *entity, int isentenceg, float volume, float attenuation, int flags, int pitch);
EXPORT int SENTENCEG_PlayRndSz(edict_t *entity, const char *szrootname, float volume, float attenuation, int flags, int pitch);
EXPORT int SENTENCEG_PlaySequentialSz(edict_t *entity, const char *szrootname, float volume, float attenuation, int flags, int pitch, int ipick, int freset);
EXPORT int SENTENCEG_GetIndex(const char *szrootname);
EXPORT int SENTENCEG_Lookup(const char *sample, char *sentencenum, int bufsz);
EXPORT int SENTENCEG_GroupCount(const char *groupName);

EXPORT char TEXTURETYPE_Find(char *name);

// bulletEmitter = if set, message will be sent to all HL clients besides this player (otherwise all players)
EXPORT float TEXTURETYPE_PlaySound(TraceResult *ptr,  Vector vecSrc, Vector vecEnd, int iBulletType, edict_t* emitter=ENT(0), edict_t* bulletEmitter=NULL);

// NOTE: use EMIT_SOUND_DYN to set the pitch of a sound. Pitch of 100
// is no pitch shift.  Pitch > 100 up to 255 is a higher pitch, pitch < 100
// down to 1 is a lower pitch.   150 to 70 is the realistic range.
// EMIT_SOUND_DYN with pitch != 100 should be used sparingly, as it's not quite as
// fast as EMIT_SOUND (the pitchshift mixer is not native coded).

EXPORT void EMIT_SOUND_DYN(edict_t *entity, int channel, const char *sample, float volume, float attenuation,
						   int flags, int pitch);

// rehlds
#define DEFAULT_SOUND_PACKET_VOLUME 255
#define DEFAULT_SOUND_PACKET_ATTENUATION 1.0f
#define DEFAULT_SOUND_PACKET_PITCH 100
#define MAX_EDICT_BITS 11

// uses a static buffer, returns NULL on failure
EXPORT mstream* BuildStartSoundMessage(edict_t* entity, int channel, const char* sample, float fvolume,
	float attenuation, int fFlags, int pitch, const float* origin);

// play the sound for players with bits contained in messageTargets
// a player bit = 1 << (ENTINDEX(player_edict) % 31)
EXPORT void StartSound(edict_t* entity, int channel, const char* sample, float volume, float attenuation,
	int fFlags, int pitch, const float* origin, uint32_t messageTargets);

EXPORT void StartSound(int eidx, int channel, const char* sample, float volume, float attenuation,
	int fFlags, int pitch, const float* origin, uint32_t messageTargets);

inline void EMIT_SOUND(edict_t *entity, int channel, const char *sample, float volume, float attenuation)
{
	EMIT_SOUND_DYN(entity, channel, sample, volume, attenuation, 0, PITCH_NORM);
}

inline void STOP_SOUND(edict_t *entity, int channel, const char *sample)
{
	EMIT_SOUND_DYN(entity, channel, sample, 0, 0, SND_STOP, PITCH_NORM);
}

// conditionally plays a special distant sound clip for very loud sounds that should be heard everywhere
EXPORT void PLAY_DISTANT_SOUND(edict_t* emitter, int soundType);

EXPORT void EMIT_SOUND_SUIT(edict_t *entity, const char *sample);
EXPORT void EMIT_GROUPID_SUIT(edict_t *entity, int isentenceg);
EXPORT void EMIT_GROUPNAME_SUIT(edict_t *entity, const char *groupname);

// macros for precaching sound arrays and selecting random sounds which are affected my mp_soundvariety
#ifdef CLIENT_DLL
#define PRECACHE_SOUND_ARRAY( a ) \
	{ \
		for (int i = 0; i < ARRAYSIZE( (a) ); i++ ) \
			PRECACHE_SOUND((char *) (a) [i]); \
	}
#define RANDOM_SOUND_ARRAY_IDX( array ) RANDOM_LONG(0,(ARRAYSIZE( (array) ))-1)
#else
#define PRECACHE_SOUND_ARRAY( a ) \
	{ \
		int count = ARRAYSIZE( (a) ); \
		if (soundvariety.value > 0) { \
			count = V_min(soundvariety.value, count); \
		} \
		for (int i = 0; i < count; i++ ) \
			PRECACHE_SOUND((char *) (a) [i]); \
	}
#define PRECACHE_SOUND_ARRAY_WORLD( a ) \
	{ \
		int count = ARRAYSIZE( (a) ); \
		if (soundvariety.value > 0) { \
			count = V_min(soundvariety.value, count); \
		} \
		for (int i = 0; i < count; i++ ) \
			PRECACHE_SOUND_ENT(NULL, (char *) (a) [i]); \
	}
#define RANDOM_SOUND_ARRAY_IDX( array ) RANDOM_LONG(0,(soundvariety.value > 0 ? V_min(ARRAYSIZE( (array) ), soundvariety.value) : ARRAYSIZE( (array) ))-1)
#endif

#define PRECACHE_FOOTSTEP_SOUNDS(array) \
	for (int i = 0; i < (int)ARRAY_SZ(array); i++) { \
		if ((i % 2) && g_footstepVariety < 2) { \
			continue; \
		} \
		PRECACHE_SOUND_ENT(NULL, array[i]); \
	}

#define EMIT_SOUND_ARRAY_DYN( chan, array ) \
	{ \
		EMIT_SOUND_DYN(ENT(pev), chan, (array)[RANDOM_SOUND_ARRAY_IDX(array)], 1.0, ATTN_NORM, 0, RANDOM_LONG(95, 105)); \
	}
#define RANDOM_SOUND_ARRAY( array ) (array) [ RANDOM_SOUND_ARRAY_IDX(array) ]

// randomize sounds in array, so that the same sounds aren't played on every map when mp_soundvariety is low
#define ARRAY_SZ(array) (sizeof(array) / sizeof(array[0]))
#define SHUFFLE_SOUND_ARRAY(array) UTIL_ShuffleSoundArray(array, ARRAY_SZ(array));
EXPORT void UTIL_ShuffleSoundArray(const char** arr, size_t n);

extern int g_footstepVariety;
extern const char* g_stepSoundsConcrete[4];
extern const char* g_stepSoundsMetal[4];
extern const char* g_stepSoundsDirt[4];
extern const char* g_stepSoundsDuct[4];
extern const char* g_stepSoundsDuct[4];
extern const char* g_stepSoundsGrate[4];
extern const char* g_stepSoundsTile[4];
extern const char* g_stepSoundsSlosh[4];
extern const char* g_stepSoundsWade[4];
extern const char* g_stepSoundsLadder[4];
extern const char* g_swimSounds[4];

#define PLAYBACK_EVENT( flags, who, index ) PLAYBACK_EVENT_FULL( flags, who, index, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );
#define PLAYBACK_EVENT_DELAY( flags, who, index, delay ) PLAYBACK_EVENT_FULL( flags, who, index, delay, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, 0, 0, 0, 0 );

#define GROUP_OP_AND	0
#define GROUP_OP_NAND	1

extern int g_groupmask;
extern int g_groupop;

class UTIL_GroupTrace
{
public:
	UTIL_GroupTrace( int groupmask, int op );
	~UTIL_GroupTrace( void );

private:
	int m_oldgroupmask, m_oldgroupop;
};

EXPORT void UTIL_SetGroupTrace( int groupmask, int op );
EXPORT void UTIL_UnsetGroupTrace( void );

EXPORT int UTIL_SharedRandomLong( unsigned int seed, int low, int high );
EXPORT float UTIL_SharedRandomFloat( unsigned int seed, float low, float high );

EXPORT float UTIL_WeaponTimeBase( void );

EXPORT void InitEdictRelocations();
EXPORT void PrintEntindexStats(bool showCounts=false);
EXPORT CBaseEntity* RelocateEntIdx(CBaseEntity* pEntity);

// removes the oldest corpses/gibs/weaponboxes
EXPORT void UTIL_CleanupEntities(int removeCount);

// returns false if the entity index would overflow the client, and prints an error message in that case
EXPORT bool UTIL_isSafeEntIndex(edict_t* plr, int idx, const char* action);

inline void WRITE_COORD_VECTOR(const Vector& vec)
{
	WRITE_COORD(vec.x);
	WRITE_COORD(vec.y);
	WRITE_COORD(vec.z);
}

EXPORT bool boxesIntersect(const Vector& mins1, const Vector& maxs1, const Vector& mins2, const Vector& maxs2);

EXPORT bool fileExists(const char* path);

// loads a global model/sound replacement file. Returns a key that you can use with g_replacementFiles
// format: "file_path" "replacement_file_path"
EXPORT std::string loadReplacementFile(const char* path);

// load replacement file directly into a map. Returns true on success.
EXPORT bool loadReplacementFile(const char* path, StringMap& replacements);

//
// BModelOrigin - calculates origin of a bmodel from absmin/size because all bmodel origins are 0 0 0
//
EXPORT Vector VecBModelOrigin(entvars_t* pevBModel);

EXPORT void PlayCDTrack(int iTrack);

// strips unsafe chars from value to prevent sneaky stuff like "sv_gravity 800;rcon_password lololol"
EXPORT std::string sanitize_cvar_value(std::string val);

EXPORT const char* getActiveWeapon(entvars_t* pev);

EXPORT uint64_t steamid_to_steamid64(const char* steamid);

EXPORT std::string steamid64_to_steamid(uint64_t steam64);

EXPORT void LoadAdminList(bool forceUpdate=false); // call on each map change, so AdminLevel can work

// returns ADMIN_YES for admins, ADMIN_NO for normal players, ADMIN_OWNER for NULL or listen server host
EXPORT int AdminLevel(CBasePlayer* player);

EXPORT std::vector<std::string> getDirFiles(std::string path, std::string extension, std::string startswith, bool onlyOne);

EXPORT short FixedSigned16(float value, float scale);

EXPORT unsigned short FixedUnsigned16(float value, float scale);

EXPORT void KickPlayer(edict_t* ent, const char* reason="");

EXPORT void handleThreadPrints();

EXPORT bool createFolder(const std::string& path);

EXPORT bool folderExists(const std::string& path);

EXPORT uint64_t getFreeSpace(const std::string& path);

EXPORT int count_bits_set(uint64_t v);

EXPORT void UTIL_ForceRetouch(edict_t* ent); // force entity to Touch() all triggers it is in contact with

// return global or per-monster sound replacement, or the same path if not replaced 
EXPORT const char* UTIL_GetReplacementSound(edict_t* ent, const char* sound);

// return replacement model, or same model if not replaced
EXPORT const char* UTIL_GetReplacementModel(const char* model);

// move a player to an active spawnpoint, and optionally respawn them or restore health
// moveLivingPlayers will skip living players if false
// respawnDeadPlayers will also restore hp for living players if true
EXPORT void UTIL_RespawnPlayer(CBasePlayer* plr, bool moveLivingPlayers=true, bool respawnDeadPlayers=true);
EXPORT void UTIL_RespawnAllPlayers(bool moveLivingPlayers=true, bool respawnDeadPlayers=true);

// rotates a point around 0,0,0 using YXZ euler rotation order
EXPORT Vector UTIL_RotatePoint(Vector pos, Vector angles);
EXPORT Vector UTIL_UnwindPoint(Vector pos, Vector angles);

// stop currently playing voice audio for this player to fix stuttering caused by cameras,
// teleports, new VIS areas, etc.
EXPORT void UTIL_ResetVoiceChannel(CBasePlayer* plr);

EXPORT void UTIL_MD5HashData(uint8_t digest[16], uint8_t* data, int dataLen);

EXPORT void UTIL_MD5HashFile(uint8_t digest[16], const char* fpath);

// will call plugin hooks, so don't call this in a userinfo hook or else you'll have an infinite loop
EXPORT void UTIL_SendUserInfo_hooked(edict_t* msgPlayer, edict_t* infoPlayer, char* info);

// helper for sending the userinfo message
EXPORT void UTIL_SendUserInfo(edict_t* msgPlayer, edict_t* infoPlayer, char* info);

// returns: -1 no intersect, 0 = ray starts inside box, >0 = intersection distance
EXPORT float UTIL_RayBoxIntersect(Vector start, Vector rayDir, Vector mins, Vector maxs);

// returns a bit mask representing all players that are using the specified client
EXPORT uint32_t UTIL_ClientBitMask(int clientMod);

// returns true if the map replaces the model (ignores standard mod replacements)
EXPORT bool UTIL_MapReplacesModel(const char* fpath);

// returns true if entity is visible from a given position
EXPORT bool UTIL_TestPVS(Vector viewPos, edict_t* target);

// returns true if entity is audible from a given position
EXPORT bool UTIL_TestPAS(Vector viewPos, edict_t* target);

// send cvar values needed for client prediction (NULL target = all players)
EXPORT void UTIL_SendPredictionCvars(CBasePlayer* target);

// send prediction cvars if any have changed
EXPORT void UTIL_SyncPredictionCvars();

// tell the client which weapon they're using, how much clip it has, and its state
EXPORT void UTIL_UpdateWeaponState(CBasePlayer* plr, int state, int wepId, int clip);