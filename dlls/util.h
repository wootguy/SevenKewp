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
#include <map>
#include <set>
#include <string>
#include "Bsp.h"
#include "mstream.h"
#include <float.h>
#include "mod_api.h"

inline void MESSAGE_BEGIN( int msg_dest, int msg_type, const float *pOrigin, entvars_t *ent );  // implementation later in this file

struct WavInfo {
	int durationMillis;
	bool isLooped; // sound is looped with cue points
};

extern globalvars_t				*gpGlobals;

// resources that were successfully precached
extern std::map<std::string, std::string> g_precachedModels; // storing values so GET_MODEL can be used with MAKE_STRING
extern std::set<std::string> g_missingModels; // storing values so GET_MODEL can be used with MAKE_STRING
extern std::set<std::string> g_precachedSounds;
extern std::set<std::string> g_precachedGeneric;
extern std::map<std::string, int> g_precachedEvents;

// resources that attempted to precache but may have been replaced with a failure model
extern std::set<std::string> g_tryPrecacheModels;
extern std::set<std::string> g_tryPrecacheSounds;
extern std::set<std::string> g_tryPrecacheGeneric;
extern std::set<std::string> g_tryPrecacheEvents;

extern std::map<std::string, WavInfo> g_wavInfos; // cached wav info, cleared on map change

extern int g_serveractive; // 1 if ServerActivate was called (no longer safe to precache)
extern int g_edictsinit; // 1 if all edicts were allocated so that relocations can begin

#define NOT_PRECACHED_MODEL "models/" MOD_MODEL_FOLDER "not_precached.mdl"
#define NOT_PRECACHED_SOUND "common/null.wav"
#define MAX_PRECACHE 512
#define MAX_PRECACHE_SOUND 511
#define MAX_PRECACHE_MODEL 510
#define MAX_PRECACHE_EVENT 256

extern Bsp g_bsp;

extern std::string g_mp3Command; // current global mp3 command

struct RGBA {
	uint8_t r, g, b, a;

	RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}
	RGBA(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b), a(255) {}
};

// Use this instead of ALLOC_STRING on constant strings
#define STRING(offset)		((const char *)(gpGlobals->pStringBase + (unsigned int)(offset)))
#define MAKE_STRING(str)	((uint64)(str) - (uint64)(STRING(0)))

inline edict_t *FIND_ENTITY_BY_CLASSNAME(edict_t *entStart, const char *pszName) 
{
	return FIND_ENTITY_BY_STRING(entStart, "classname", pszName);
}	

inline edict_t *FIND_ENTITY_BY_TARGETNAME(edict_t *entStart, const char *pszName) 
{
	return FIND_ENTITY_BY_STRING(entStart, "targetname", pszName);
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

// In case this ever changes
#define M_PI			3.14159265358979323846

// Keeps clutter down a bit, when declaring external entity/global method prototypes
#define DECLARE_GLOBAL_METHOD(MethodName)  extern void DLLEXPORT MethodName( void )
#define GLOBAL_METHOD(funcname)					void DLLEXPORT funcname(void)

// This is the glue that hooks .MAP entity class names to our CPP classes
// The _declspec forces them to be exported by name so we can do a lookup with GetProcAddress()
// The function is used to intialize / allocate the object for the entity
#define LINK_ENTITY_TO_CLASS(mapClassName,DLLClassName) \
	extern "C" DLLEXPORT void mapClassName( entvars_t *pev ); \
	void mapClassName( entvars_t *pev ) { GetClassPtr( (DLLClassName *)pev ); }


//
// Conversion among the three types of "entity", including identity-conversions.
//
#ifdef DEBUG
	extern edict_t *DBG_EntOfVars(const entvars_t *pev);
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
inline int	  ENTINDEX(edict_t *pEdict)			{ return (*g_engfuncs.pfnIndexOfEdict)(pEdict); }
inline edict_t* INDEXENT( int iEdictNum )		{ return (*g_engfuncs.pfnPEntityOfEntIndex)(iEdictNum); }
inline uint32_t PLRBIT(edict_t* pEdict)			{ return 1 << (ENTINDEX(pEdict) & 31); }
inline void MESSAGE_BEGIN( int msg_dest, int msg_type, const float *pOrigin, entvars_t *ent ) {
	(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ENT(ent));
}
void WRITE_BYTES(uint8_t* bytes, int count);

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



// Things that toggle (buttons/triggers/doors) need this
typedef enum
	{
	TS_AT_TOP,
	TS_AT_BOTTOM,
	TS_GOING_UP,
	TS_GOING_DOWN
	} TOGGLE_STATE;

// Misc useful
inline BOOL FStrEq(const char*sz1, const char*sz2)
	{ return (strcmp(sz1, sz2) == 0); }
inline BOOL FClassnameIs(edict_t* pent, const char* szClassname)
	{ return FStrEq(STRING(VARS(pent)->classname), szClassname); }
inline BOOL FClassnameIs(entvars_t* pev, const char* szClassname)
	{ return FStrEq(STRING(pev->classname), szClassname); }

class CBaseEntity;

// Misc. Prototypes
extern void			UTIL_SetSize			(entvars_t* pev, const Vector &vecMin, const Vector &vecMax);
extern float		UTIL_VecToYaw			(const Vector &vec);
extern Vector		UTIL_VecToAngles		(const Vector &vec);
extern float		UTIL_AngleMod			(float a);
extern float		UTIL_AngleDiff			( float destAngle, float srcAngle );

extern CBaseEntity	*UTIL_FindEntityInSphere(CBaseEntity *pStartEntity, const Vector &vecCenter, float flRadius);
extern CBaseEntity	*UTIL_FindEntityByString(CBaseEntity *pStartEntity, const char *szKeyword, const char *szValue );
extern CBaseEntity	*UTIL_FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName );
extern CBaseEntity	*UTIL_FindEntityByTargetname(CBaseEntity *pStartEntity, const char *szName );
extern CBaseEntity	*UTIL_FindEntityGeneric(const char *szName, Vector &vecSrc, float flRadius );

// returns a CBaseEntity pointer to a player by index.  Only returns if the player is spawned and connected
// otherwise returns NULL
// Index is 1 based
extern CBaseEntity	*UTIL_PlayerByIndex( int playerIndex );

#define UTIL_EntitiesInPVS(pent)			(*g_engfuncs.pfnEntitiesInPVS)(pent)
extern edict_t*		UTIL_ClientsInPVS(edict_t* edict, int& playerCount);
extern bool			UTIL_IsClientInPVS(edict_t* edict); // faster than UTIL_ClientsInPVS
extern void			UTIL_MakeVectors(const Vector& vecAngles);
extern bool			IsValidPlayer(edict_t* edict); // true if edict is a connected player

// Pass in an array of pointers and an array size, it fills the array and returns the number inserted
extern int			UTIL_MonstersInSphere( CBaseEntity **pList, int listMax, const Vector &center, float radius );
extern int			UTIL_EntitiesInBox( CBaseEntity **pList, int listMax, const Vector &mins, const Vector &maxs, int flagMask, bool ignoreDead);

inline void UTIL_MakeVectorsPrivate( const Vector &vecAngles, float *p_vForward, float *p_vRight, float *p_vUp )
{
	g_engfuncs.pfnAngleVectors( vecAngles, p_vForward, p_vRight, p_vUp );
}

extern void			UTIL_MakeAimVectors		( const Vector &vecAngles ); // like MakeVectors, but assumes pitch isn't inverted
extern void			UTIL_MakeInvVectors		( const Vector &vec, globalvars_t *pgv );

extern void			UTIL_SetOrigin			( entvars_t* pev, const Vector &vecOrigin ); // Needs to be called any time an entity changes origin, mins, maxs, or solid
extern void			UTIL_EmitAmbientSound	( edict_t *entity, const float* vecOrigin, const char *samp, float vol, float attenuation, int fFlags, int pitch, edict_t* dest=NULL);
extern void			UTIL_ParticleEffect		( const Vector &vecOrigin, const Vector &vecDirection, ULONG ulColor, ULONG ulCount );
extern void			UTIL_ScreenShake		( const Vector &center, float amplitude, float frequency, float duration, float radius );
extern void			UTIL_ScreenShakeAll		( const Vector &center, float amplitude, float frequency, float duration );
extern void			UTIL_ShowMessage		( const char *pString, CBaseEntity *pPlayer );
extern void			UTIL_ShowMessageAll		( const char *pString );
extern void			UTIL_ScreenFadeAll		( const Vector &color, float fadeTime, float holdTime, int alpha, int flags );
extern void			UTIL_ScreenFade			( CBaseEntity *pEntity, const Vector &color, float fadeTime, float fadeHold, int alpha, int flags );

// leave target NULL to play music for all players
extern void UTIL_PlayGlobalMp3(const char* path, bool loop, edict_t* target=NULL);

// leave target NULL to stop music for all players
extern void UTIL_StopGlobalMp3(edict_t* target=NULL);

typedef enum { ignore_monsters=1, dont_ignore_monsters=0, missile=2 } IGNORE_MONSTERS;
typedef enum { ignore_glass=1, dont_ignore_glass=0 } IGNORE_GLASS;
extern void			UTIL_TraceLine			(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr);
extern void			UTIL_TraceLine			(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr);
enum { point_hull=0, human_hull=1, large_hull=2, head_hull=3 };
extern void			UTIL_TraceHull			(const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, int hullNumber, edict_t *pentIgnore, TraceResult *ptr);
extern TraceResult	UTIL_GetGlobalTrace		(void);
extern void			UTIL_TraceModel			(const Vector &vecStart, const Vector &vecEnd, int hullNumber, edict_t *pentModel, TraceResult *ptr);
extern Vector		UTIL_GetAimVector		(edict_t* pent, float flSpeed);
extern int			UTIL_PointContents		(const Vector &vec);

extern int			UTIL_IsMasterTriggered	(string_t sMaster, CBaseEntity *pActivator);
extern void			UTIL_BloodStream( const Vector &origin, const Vector &direction, int color, int amount );
extern void			UTIL_BloodDrips( const Vector &origin, const Vector &direction, int color, int amount );
extern Vector		UTIL_RandomBloodVector( void );
extern BOOL			UTIL_ShouldShowBlood( int bloodColor );
extern void			UTIL_BloodDecalTrace( TraceResult *pTrace, int bloodColor );
extern void			UTIL_DecalTrace( TraceResult *pTrace, int decalNumber );
extern void			UTIL_PlayerDecalTrace( TraceResult *pTrace, int playernum, int decalNumber, BOOL bIsCustom );
extern void			UTIL_GunshotDecalTrace( TraceResult *pTrace, int decalNumber );
extern void			UTIL_Sparks( const Vector &position );
extern void			UTIL_Ricochet( const Vector &position, float scale );
extern void			UTIL_Shrapnel(Vector pos, Vector dir, float flDamage, int bitsDamageType);
extern void			UTIL_StringToVector( float *pVector, const char *pString );
extern bool			UTIL_StringIsVector( const char *pString );
extern void			UTIL_StringToIntArray( int *pVector, int count, const char *pString );
extern Vector		UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize );
extern float		UTIL_Approach( float target, float value, float speed );
extern float		UTIL_ApproachAngle( float target, float value, float speed );
extern float		UTIL_AngleDistance( float next, float cur );

extern char			*UTIL_VarArgs( const char *format, ... );
extern void			UTIL_Remove( CBaseEntity *pEntity );
extern BOOL			UTIL_IsValidEntity( edict_t *pent );
extern BOOL			UTIL_TeamsMatch( const char *pTeamName1, const char *pTeamName2 );

// Use for ease-in, ease-out style interpolation (accel/decel)
extern float		UTIL_SplineFraction( float value, float scale );

// Search for water transition along a vertical line
extern float		UTIL_WaterLevel( const Vector &position, float minz, float maxz );
extern void			UTIL_Bubbles( Vector mins, Vector maxs, int count );
extern void			UTIL_BubbleTrail( Vector from, Vector to, int count );

// allows precacheing of other entities
extern void			UTIL_PrecacheOther( const char *szClassname, std::map<std::string, std::string> keys=std::map<std::string, std::string>() );

// prints a message to each client
extern void			UTIL_ClientPrintAll( int msg_dest, const char *msg);
inline void			UTIL_CenterPrintAll( const char *msg_name, const char *param1 = NULL, const char *param2 = NULL, const char *param3 = NULL, const char *param4 = NULL ) 
{
	UTIL_ClientPrintAll( print_center, msg_name );
}

class CBasePlayerItem;
class CBasePlayer;
extern BOOL UTIL_GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon );

// prints messages through the HUD
extern void UTIL_ClientPrint(edict_t* client, int msg_dest, const char *msg );

// prints a message to the HUD say (chat)
extern void			UTIL_SayText( const char *pText, CBaseEntity *pEntity );
extern void			UTIL_SayTextAll( const char *pText, CBaseEntity *pEntity );


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
	int			channel;
} hudtextparms_t;

// prints as transparent 'title' to the HUD
extern void			UTIL_HudMessageAll( const hudtextparms_t &textparms, const char *pMessage );
extern void			UTIL_HudMessage( CBaseEntity *pEntity, const hudtextparms_t &textparms, const char *pMessage );

// for handy use with ClientPrint params
extern char *UTIL_dtos1( int d );
extern char *UTIL_dtos2( int d );
extern char *UTIL_dtos3( int d );
extern char *UTIL_dtos4( int d );

// Writes message to console with timestamp and FragLog header.
extern void			UTIL_LogPrintf( const char *fmt, ... );

// Sorta like FInViewCone, but for nonmonsters. 
extern float UTIL_DotPoints ( const Vector &vecSrc, const Vector &vecCheck, const Vector &vecDir );

extern void UTIL_StripToken( const char *pKey, char *pDest );// for redundant keynames

// Misc functions
extern void SetMovedir(entvars_t* pev);
extern Vector VecBModelOrigin( entvars_t* pevBModel );
extern int BuildChangeList( LEVELLIST *pLevelList, int maxList );

//
// How did I ever live without ASSERT?
//
#ifdef	DEBUG
void DBG_AssertFunction(BOOL fExpr, const char* szExpr, const char* szFile, int szLine, const char* szMessage);
#define ASSERT(f)		DBG_AssertFunction(f, #f, __FILE__, __LINE__, NULL)
#define ASSERTSZ(f, sz)	DBG_AssertFunction(f, #f, __FILE__, __LINE__, sz)
#else	// !DEBUG
#define ASSERT(f)
#define ASSERTSZ(f, sz)
#endif	// !DEBUG


extern DLL_GLOBAL const Vector g_vecZero;

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

#define SND_FL_VOLUME		(1<<0)		// send volume (set automatically)
#define SND_FL_ATTENUATION	(1<<1)		// send attenuation (set automatically)
#define SND_FL_LARGE_INDEX	(1<<2)		// send large entity index (set automatically)
#define SND_FL_PITCH		(1<<3)		// send pitch (set automatically)
#define SND_SPAWNING		(1<<8)		// duplicated in protocol.h we're spawing, used in some cases for ambients 
#define SND_SENTENCE		(1<<4)
#define SND_STOP			(1<<5)		// duplicated in protocol.h stop sound
#define SND_CHANGE_VOL		(1<<6)		// duplicated in protocol.h change sound vol
#define SND_CHANGE_PITCH	(1<<7)		// duplicated in protocol.h change sound pitch

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

typedef enum svc_commands_e
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

// func breakable
#define SF_BREAK_TRIGGER_ONLY	1// may only be broken by trigger
#define	SF_BREAK_TOUCH			2// can be 'crashed through' by running player (plate glass)
#define SF_BREAK_PRESSURE		4// can be broken by a player standing on it
#define SF_BREAK_CROWBAR		256// instant break if hit with crowbar
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
#define CVOXFILESENTENCEMAX		1536		// max number of sentences in game. NOTE: this must match
											// CVOXFILESENTENCEMAX in engine\sound.h!!!

extern char gszallsentencenames[CVOXFILESENTENCEMAX][CBSENTENCENAME_MAX];
extern int gcallsentences;

int USENTENCEG_Pick(int isentenceg, char *szfound);
int USENTENCEG_PickSequential(int isentenceg, char *szfound, int ipick, int freset);
void USENTENCEG_InitLRU(unsigned char *plru, int count);

void SENTENCEG_Init();
void SENTENCEG_Stop(edict_t *entity, int isentenceg, int ipick);
int SENTENCEG_PlayRndI(edict_t *entity, int isentenceg, float volume, float attenuation, int flags, int pitch);
int SENTENCEG_PlayRndSz(edict_t *entity, const char *szrootname, float volume, float attenuation, int flags, int pitch);
int SENTENCEG_PlaySequentialSz(edict_t *entity, const char *szrootname, float volume, float attenuation, int flags, int pitch, int ipick, int freset);
int SENTENCEG_GetIndex(const char *szrootname);
int SENTENCEG_Lookup(const char *sample, char *sentencenum);

void TEXTURETYPE_Init();
char TEXTURETYPE_Find(char *name);
float TEXTURETYPE_PlaySound(TraceResult *ptr,  Vector vecSrc, Vector vecEnd, int iBulletType);

// NOTE: use EMIT_SOUND_DYN to set the pitch of a sound. Pitch of 100
// is no pitch shift.  Pitch > 100 up to 255 is a higher pitch, pitch < 100
// down to 1 is a lower pitch.   150 to 70 is the realistic range.
// EMIT_SOUND_DYN with pitch != 100 should be used sparingly, as it's not quite as
// fast as EMIT_SOUND (the pitchshift mixer is not native coded).

void EMIT_SOUND_DYN(edict_t *entity, int channel, const char *sample, float volume, float attenuation,
						   int flags, int pitch);

// play the sound for players with bits contained in messageTargets
// a player bit = 1 << (ENTINDEX(player_edict) % 31)
void StartSound(edict_t* entity, int channel, const char* sample, float volume, float attenuation,
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
void PLAY_DISTANT_SOUND(edict_t* emitter, int soundType);

void EMIT_SOUND_SUIT(edict_t *entity, const char *sample);
void EMIT_GROUPID_SUIT(edict_t *entity, int isentenceg);
void EMIT_GROUPNAME_SUIT(edict_t *entity, const char *groupname);

// macros for precaching sound arrays and selecting random sounds which are affected my mp_soundvariety
#define RANDOM_SOUND_ARRAY_IDX( array ) RANDOM_LONG(0,(soundvariety.value > 0 ? V_min(ARRAYSIZE( (array) ), soundvariety.value) : ARRAYSIZE( (array) ))-1)
#define PRECACHE_SOUND_ARRAY( a ) \
	{ \
		int count = ARRAYSIZE( (a) ); \
		if (soundvariety.value > 0) { \
			count = V_min(soundvariety.value, count); \
		} \
		for (int i = 0; i < count; i++ ) \
			PRECACHE_SOUND((char *) (a) [i]); \
	}

#define EMIT_SOUND_ARRAY_DYN( chan, array ) \
	{ \
		EMIT_SOUND_DYN(ENT(pev), chan, (array)[RANDOM_SOUND_ARRAY_IDX(array)], 1.0, ATTN_NORM, 0, RANDOM_LONG(95, 105)); \
	}
#define RANDOM_SOUND_ARRAY( array ) (array) [ RANDOM_SOUND_ARRAY_IDX(array) ]

// randomize sounds in array, so that the same sounds aren't played on every map when mp_soundvariety is low
#define SOUND_ARRAY_SZ(array) (sizeof(array) / sizeof(const char*))
#define SHUFFLE_SOUND_ARRAY(array) UTIL_ShuffleSoundArray(array, SOUND_ARRAY_SZ(array));
void UTIL_ShuffleSoundArray(const char** arr, size_t n);

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

void UTIL_SetGroupTrace( int groupmask, int op );
void UTIL_UnsetGroupTrace( void );

int UTIL_SharedRandomLong( unsigned int seed, int low, int high );
float UTIL_SharedRandomFloat( unsigned int seed, float low, float high );

float UTIL_WeaponTimeBase( void );

#ifdef CLIENT_DLL
#define PRECACHE_MODEL	(*g_engfuncs.pfnPrecacheModel)
#define SET_MODEL		(*g_engfuncs.pfnSetModel)
#define PRECACHE_SOUND	(*g_engfuncs.pfnPrecacheSound)
#define PRECACHE_EVENT	(*g_engfuncs.pfnPrecacheEvent)
#define MODEL_INDEX		(*g_engfuncs.pfnModelIndex)
#define GET_MODEL(model) model
inline void MESSAGE_BEGIN(int msg_dest, int msg_type, const float* pOrigin = NULL, edict_t* ed = NULL) {
	(*g_engfuncs.pfnMessageBegin)(msg_dest, msg_type, pOrigin, ed);
}
#define MESSAGE_END		(*g_engfuncs.pfnMessageEnd)
#define WRITE_BYTE		(*g_engfuncs.pfnWriteByte)
#define WRITE_CHAR		(*g_engfuncs.pfnWriteChar)
#define WRITE_SHORT		(*g_engfuncs.pfnWriteShort)
#define WRITE_LONG		(*g_engfuncs.pfnWriteLong)
#define WRITE_ANGLE		(*g_engfuncs.pfnWriteAngle)
#define WRITE_COORD		(*g_engfuncs.pfnWriteCoord)
#define WRITE_STRING	(*g_engfuncs.pfnWriteString)
#define WRITE_ENTITY	(*g_engfuncs.pfnWriteEntity)
#define GET_MODEL_PTR	(*g_engfuncs.pfnGetModelPtr)
#define CREATE_NAMED_ENTITY		(*g_engfuncs.pfnCreateNamedEntity)
#else
// engine wrappers which handle model/sound replacement logic
int PRECACHE_GENERIC(const char* path);
int PRECACHE_SOUND_ENT(CBaseEntity* ent, const char* path);
int PRECACHE_SOUND_NULLENT(const char* path);
int PRECACHE_MODEL(const char* model);
int PRECACHE_EVENT(int id, const char* path);
void SET_MODEL(edict_t* edict, const char* model);
const char* GET_MODEL(const char* model); // return replacement model, if one exists, or the given model
int MODEL_INDEX(const char* model);
void* GET_MODEL_PTR(edict_t* edict);
edict_t* CREATE_NAMED_ENTITY(string_t cname);
#define PRECACHE_SOUND(path) PRECACHE_SOUND_ENT(this, path)

void MESSAGE_BEGIN(int msg_dest, int msg_type, const float* pOrigin = NULL, edict_t* ed = NULL);
void MESSAGE_END();
void WRITE_BYTE(int iValue);
void WRITE_CHAR(int iValue);
void WRITE_SHORT(int iValue);
void WRITE_LONG(int iValue);
void WRITE_ANGLE(float fValue);
void WRITE_COORD(float iValue);
void WRITE_STRING(const char* sValue);
void WRITE_ENTITY(int iValue);
#endif

void InitEdictRelocations();
void PrintEntindexStats();
CBaseEntity* RelocateEntIdx(CBaseEntity* pEntity);

// returns false if the entity index would overflow the client, and prints an error message in that case
inline bool UTIL_isSafeEntIndex(int idx, const char* action);

inline void WRITE_COORD_VECTOR(const Vector& vec)
{
	WRITE_COORD(vec.x);
	WRITE_COORD(vec.y);
	WRITE_COORD(vec.z);
}

// write the most recent X seconds of message history for debugging client disconnects 
// due to malformed network messages.
// reason = reason for writing the message history
void writeNetworkMessageHistory(std::string reason);
void clearNetworkMessageHistory();

std::vector<std::string> splitString(std::string str, const char* delimitters);

std::string toLowerCase(std::string str);

std::string toUpperCase(std::string str);

std::string trimSpaces(std::string s);

std::string replaceString(std::string subject, std::string search, std::string replace);

bool boxesIntersect(const Vector& mins1, const Vector& maxs1, const Vector& mins2, const Vector& maxs2);

float clampf(float val, float min, float max);

int clampi(int val, int min, int max);

// returns 0 if the file doesn't exist
uint64_t getFileModifiedTime(const char* path);

bool fileExists(const char* path);

// searches game directories in order (e.g. valve/path, valve_downloads/path)
// returns an empty string if the file can't be found
std::string getGameFilePath(const char* path);

// loads a global model/sound replacement file
// format: "file_path" "replacement_file_path"
std::map<std::string, std::string> loadReplacementFile(const char* path);

void te_debug_beam(Vector start, Vector end, uint8_t life, RGBA c, int msgType=MSG_BROADCAST, edict_t* dest=NULL);

WavInfo getWaveFileInfo(const char* path);