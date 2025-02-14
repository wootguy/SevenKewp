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
#ifndef CLIENT_H
#define CLIENT_H

extern void respawn( entvars_t* pev, BOOL fCopyCorpse );
extern BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
extern void ClientDisconnect( edict_t *pEntity );
extern void ClientKill( edict_t *pEntity );
extern void ClientPutInServer( edict_t *pEntity );
extern void ClientCommand( edict_t *pEntity );
extern void ClientUserInfoChanged( edict_t *pEntity, char *infobuffer );
extern void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
extern void ServerDeactivate( void );
extern void StartFrame( void );
extern void PlayerPostThink( edict_t *pEntity );
extern void PlayerPreThink( edict_t *pEntity );
extern void ParmsNewLevel( void );
extern void ParmsChangeLevel( void );

extern void ClientPrecache( void );

extern const char *GetGameDescription( void );
extern void PlayerCustomization( edict_t *pEntity, customization_t *pCust );

extern void SpectatorConnect ( edict_t *pEntity );
extern void SpectatorDisconnect ( edict_t *pEntity );
extern void SpectatorThink ( edict_t *pEntity );

extern void Sys_Error( const char *error_string );

extern void SetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas );
extern void	UpdateClientData ( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd );
extern int AddToFullPack( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet );
extern void CreateBaseline( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs );
extern void RegisterEncoders( void );

extern int GetWeaponData( struct edict_s *player, struct weapon_data_s *info );

extern void	CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed );
extern void	CmdEnd ( const edict_t *player );

extern int	ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );

extern int GetHullBounds( int hullnumber, float *mins, float *maxs );

extern void	CreateInstancedBaselines ( void );

extern int	InconsistentFile( const edict_t *player, const char *filename, char *disconnect_message );

extern int AllowLagCompensation( void );

// new dll funcs
void OnFreeEntPrivateData(edict_t* pEnt);
void GameShutdown(void);
int	ShouldCollide(edict_t* pentTouched, edict_t* pentOther);
void CvarValue(const edict_t* pEnt, const char* pszValue);
void CvarValue2(const edict_t* pEnt, int requestID, const char* pszCvarName, const char* pszValue);

typedef struct {
	DLL_FUNCTIONS* dllapi_table;
	NEW_DLL_FUNCTIONS* newapi_table;
} gamedll_funcs_t;

EXPORT extern gamedll_funcs_t* gpGamedllFuncs; // for ease of porting to/from metamod

// C functions for external declarations that call the appropriate C++ methods

extern "C" DLLEXPORT int GetEntityAPI(DLL_FUNCTIONS * pFunctionTable, int interfaceVersion);
extern "C" DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS * pFunctionTable, int* interfaceVersion);
extern "C" DLLEXPORT int GetNewDLLFunctions(NEW_DLL_FUNCTIONS * pNewFunctionTable, int* interfaceVersion);

EXPORT extern int DispatchSpawn(edict_t* pent);
EXPORT extern void DispatchKeyValue(edict_t* pentKeyvalue, KeyValueData* pkvd);
EXPORT extern void DispatchTouch(edict_t* pentTouched, edict_t* pentOther);
EXPORT extern void DispatchUse(edict_t* pentUsed, edict_t* pentOther);
EXPORT extern void DispatchThink(edict_t* pent);
EXPORT extern void DispatchBlocked(edict_t* pentBlocked, edict_t* pentOther);
EXPORT extern void DispatchSave(edict_t* pent, SAVERESTOREDATA* pSaveData);
EXPORT extern int DispatchRestore(edict_t* pent, SAVERESTOREDATA* pSaveData, int globalEntity);
EXPORT extern void DispatchObjectCollsionBox(edict_t* pent);
EXPORT extern void SaveWriteFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);
EXPORT extern void SaveReadFields(SAVERESTOREDATA* pSaveData, const char* pname, void* pBaseData, TYPEDESCRIPTION* pFields, int fieldCount);
EXPORT extern void SaveGlobalState(SAVERESTOREDATA* pSaveData);
EXPORT extern void RestoreGlobalState(SAVERESTOREDATA* pSaveData);
EXPORT extern void ResetGlobalState(void);
EXPORT extern void SetObjectCollisionBox(entvars_t* pev);

// for convenience
EXPORT extern void DispatchKeyValue(edict_t* pentKeyvalue, const char* key, const char* value);
EXPORT extern void DispatchKeyValue(edict_t* pentKeyvalue, const char* key, float value);
EXPORT extern void DispatchKeyValue(edict_t* pentKeyvalue, const char* key, int value);
EXPORT extern void DispatchKeyValue(edict_t* pentKeyvalue, const char* key, Vector value);

#endif		// CLIENT_H
