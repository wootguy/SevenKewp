#ifndef __APIPROXY__
#define __APIPROXY__

#include "archtypes.h"     // DAL
#include "netadr.h"
#include "Sequence.h"

#ifndef _WIN32
#include "enums.h"
#endif

#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s	*next;
	char	name[MAX_ALIAS_NAME];
	char	*value;
} cmdalias_t;

#include "enums.h"


// ********************************************************
// Functions exported by the client .dll
// ********************************************************

// Function type declarations for client exports
typedef int (*INITIALIZE_FUNC)	( struct cl_enginefuncs_s*, int );
typedef void (*HUD_INIT_FUNC)		( void );
typedef int (*HUD_VIDINIT_FUNC)	( void );
typedef int (*HUD_REDRAW_FUNC)	( float, int );
typedef int (*HUD_UPDATECLIENTDATA_FUNC) ( struct client_data_s*, float );
typedef void (*HUD_RESET_FUNC)    ( void );
typedef void (*HUD_CLIENTMOVE_FUNC)( struct playermove_s *ppmove, qboolean server );
typedef void (*HUD_CLIENTMOVEINIT_FUNC)( struct playermove_s *ppmove );
typedef char (*HUD_TEXTURETYPE_FUNC)( char *name );
typedef void (*HUD_IN_ACTIVATEMOUSE_FUNC) ( void );
typedef void (*HUD_IN_DEACTIVATEMOUSE_FUNC)		( void );
typedef void (*HUD_IN_MOUSEEVENT_FUNC)		( int mstate );
typedef void (*HUD_IN_CLEARSTATES_FUNC)		( void );
typedef void (*HUD_IN_ACCUMULATE_FUNC ) ( void );
typedef void (*HUD_CL_CREATEMOVE_FUNC)		( float frametime, struct usercmd_s *cmd, int active );
typedef int (*HUD_CL_ISTHIRDPERSON_FUNC) ( void );
typedef void (*HUD_CL_GETCAMERAOFFSETS_FUNC )( float *ofs );
typedef struct kbutton_s * (*HUD_KB_FIND_FUNC) ( const char *name );
typedef void ( *HUD_CAMTHINK_FUNC )( void );
typedef void ( *HUD_CALCREF_FUNC ) ( struct ref_params_s *pparams );
typedef int	 ( *HUD_ADDENTITY_FUNC ) ( int type, struct cl_entity_s *ent, const char *modelname );
typedef void ( *HUD_CREATEENTITIES_FUNC ) ( void );
typedef void ( *HUD_DRAWNORMALTRIS_FUNC ) ( void );
typedef void ( *HUD_DRAWTRANSTRIS_FUNC ) ( void );
typedef void ( *HUD_STUDIOEVENT_FUNC ) ( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
typedef void ( *HUD_POSTRUNCMD_FUNC ) ( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
typedef void ( *HUD_SHUTDOWN_FUNC ) ( void );
typedef void ( *HUD_TXFERLOCALOVERRIDES_FUNC )( struct entity_state_s *state, const struct clientdata_s *client );
typedef void ( *HUD_PROCESSPLAYERSTATE_FUNC )( struct entity_state_s *dst, const struct entity_state_s *src );
typedef void ( *HUD_TXFERPREDICTIONDATA_FUNC ) ( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
typedef void ( *HUD_DEMOREAD_FUNC ) ( int size, unsigned char *buffer );
typedef int ( *HUD_CONNECTIONLESS_FUNC )( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );
typedef	int	( *HUD_GETHULLBOUNDS_FUNC ) ( int hullnumber, float *mins, float *maxs );
typedef void (*HUD_FRAME_FUNC)		( double );
typedef int (*HUD_KEY_EVENT_FUNC ) ( int eventcode, int keynum, const char *pszCurrentBinding );
typedef void (*HUD_TEMPENTUPDATE_FUNC) ( double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, 	int ( *Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ),	void ( *Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
typedef struct cl_entity_s *(*HUD_GETUSERENTITY_FUNC ) ( int index );
typedef void (*HUD_VOICESTATUS_FUNC)(int entindex, qboolean bTalking);
typedef void (*HUD_DIRECTORMESSAGE_FUNC)( int iSize, void *pbuf );
typedef int ( *HUD_STUDIO_INTERFACE_FUNC )( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio );
typedef void (*HUD_CHATINPUTPOSITION_FUNC)( int *x, int *y );
typedef int (*HUD_GETPLAYERTEAM)(int iplayer);
typedef void *(*CLIENTFACTORY)(); // this should be CreateInterfaceFn but that means including interface.h
									// which is a C++ file and some of the client files a C only... 
									// so we return a void * which we then do a typecast on later.


// Pointers to the exported client functions themselves
typedef struct
{
	/**
	*	Called to initialize the client library.
	*	@param pEngineFuncs Pointer to the engine functions interface;
	*	@param iVersion Interface version. Must match CLDLL_INTERFACE_VERSION.
	*	@return true on success, false otherwise. If iVersion does not match CLDLL_INTERFACE_VERSION, return false.
	*	@see CLDLL_INTERFACE_VERSION
	*/
	INITIALIZE_FUNC						pInitFunc;

	/**
	*	Called to initialize the client library. This occurs after the engine has loaded the client and has initialized all other systems.
	*/
	HUD_INIT_FUNC						pHudInitFunc;

	/**
	*	Called after a connection to a server has been established.
	*/
	HUD_VIDINIT_FUNC					pHudVidInitFunc;

	/**
	*	Called to redraw the HUD.
	*	@param flCurrentTime Current game time.
	*	@param bIsIntermission Whether we're currently in intermission or not.
	*	@return Ignored. true on success, false otherwise.
	*/
	HUD_REDRAW_FUNC						pHudRedrawFunc;

	/**
	*	Called every frame while running a map.
	*	@param pCLData Client data.
	*	@param flCurrentTime Current game time.
	*	@return true if client data was updated, false otherwise.
	*/
	HUD_UPDATECLIENTDATA_FUNC			pHudUpdateClientDataFunc;

	/**
	*	Obsolete. Is never called.
	*/
	HUD_RESET_FUNC						pHudResetFunc;

	/**
	*	Run client side player movement and physics code.
	*	@param ppmove Player movement data.
	*	@param server Whether this is the server or client running movement.
	*/
	HUD_CLIENTMOVE_FUNC					pClientMove;

	/**
	*	Initializes the client side player movement code.
	*	@param ppmove Player movement data.
	*/
	HUD_CLIENTMOVEINIT_FUNC				pClientMoveInit;

	/**
	*	Gets the texture type for a given texture name.
	*	Never called by the engine itself.
	*	@param pszName Texture name.
	*	@return Texture type.
	*/
	HUD_TEXTURETYPE_FUNC				pClientTextureType;

	/**
	*	Called when mouse input is activated.
	*/
	HUD_IN_ACTIVATEMOUSE_FUNC			pIN_ActivateMouse;

	/**
	*	Called when mouse input is deactivated.
	*/
	HUD_IN_DEACTIVATEMOUSE_FUNC			pIN_DeactivateMouse;

	/**
	*	Called when a mouse event has occurred.
	*	@param mstate Bit vector containing new mouse button states.
	*/
	HUD_IN_MOUSEEVENT_FUNC				pIN_MouseEvent;

	/**
	*	Clears all mouse button states.
	*/
	HUD_IN_CLEARSTATES_FUNC				pIN_ClearStates;

	/**
	*	Called to accumulate relative mouse movement.
	*/
	HUD_IN_ACCUMULATE_FUNC				pIN_Accumulate;

	/**
	*	Creates a movement command to send to the server.
	*	@param frametime Delta time between the last and current frame.
	*	@param cmd Command to fill in.
	*	@param bActive if bActive == 1 then we are 1) not playing back demos ( where our commands are ignored ) and
	*					2 ) we have finished signing on to server
	*/
	HUD_CL_CREATEMOVE_FUNC				pCL_CreateMove;

	/**
	*	@return Whether the client is currently in third person mode.
	*/
	HUD_CL_ISTHIRDPERSON_FUNC			pCL_IsThirdPerson;

	/**
	*	Gets the camera offset.
	*	@param[ out ] ofs Offset.
	*/
	HUD_CL_GETCAMERAOFFSETS_FUNC		pCL_GetCameraOffsets;

	/**
	*	Finds a key by name.
	*	@param pszName Key name.
	*	@return Key, or null if it couldn't be found.
	*/
	HUD_KB_FIND_FUNC					pFindKey;

	/**
	*	Runs camera think.
	*/
	HUD_CAMTHINK_FUNC					pCamThink;

	/**
	*	Calculates view data.
	*/
	HUD_CALCREF_FUNC					pCalcRefdef;

	/**
	*	Called when the engine has created a client side copy of an entity.
	*	@param type Entity type. @see EntityType
	*	@param ent Entity.
	*	@param pszModelName Name of the model that the entity is using. Same as ent->model->name. Is an empty string if it has no model.
	*	@return true to add it to the list of visible entities, false to filter it out.
	*/
	HUD_ADDENTITY_FUNC					pAddEntity;

	/**
	*	Gives us a chance to add additional entities to the render this frame.
	*/
	HUD_CREATEENTITIES_FUNC				pCreateEntities;

	/**
	*	Lets the client draw non-transparent geometry.
	*/
	HUD_DRAWNORMALTRIS_FUNC				pDrawNormalTriangles;

	/**
	*	Lets the client draw transparent geometry.
	*/
	HUD_DRAWTRANSTRIS_FUNC				pDrawTransparentTriangles;

	/**
	*	A studiomodel event has occured while advancing an entity's frame.
	*	@param event Event.
	*	@param entity Entity whose frame is being advanced.
	*/
	HUD_STUDIOEVENT_FUNC				pStudioEvent;

	/**
	*	Client calls this during prediction, after it has moved the player and updated any info changed into to->
	*	time is the current client clock based on prediction
	*	cmd is the command that caused the movement, etc
	*	bRunFuncs is true if this is the first time we've predicted this command.  If so, sounds and effects should play, otherwise, they should
	*	be ignored
	*	@param from Old state.
	*	@param to New state.
	*	@param cmd Command that was ran.
	*	@param bRunFuncs Whether to play sounds and effects.
	*	@param time Current time.
	*	@param random_seed Shared random seed.
	*/
	HUD_POSTRUNCMD_FUNC					pPostRunCmd;

	/**
	*	Called when the client shuts down. The library is freed after this call.
	*/
	HUD_SHUTDOWN_FUNC					pShutdown;

	/**
	*	The server sends us our origin with extra precision as part of the clientdata structure, not during the normal
	*	playerstate update in entity_state_t.  In order for these overrides to eventually get to the appropriate playerstate
	*	structure, we need to copy them into the state structure at this point.
	*	@param state Player entity state.
	*	@param client Player client state.
	*/
	HUD_TXFERLOCALOVERRIDES_FUNC		pTxferLocalOverrides;

	/**
	*	We have received entity_state_t for this player over the network.  We need to copy appropriate fields to the
	*	playerstate structure.
	*	@param dst Destination state.
	*	@param src Source state.
	*/
	HUD_PROCESSPLAYERSTATE_FUNC			pProcessPlayerState;

	/**
	*	Because we can predict an arbitrary number of frames before the server responds with an update, we need to be able to copy client side prediction data in
	*	from the state that the server ack'd receiving, which can be anywhere along the predicted frame path ( i.e., we could predict 20 frames into the future and the server ack's
	*	up through 10 of those frames, so we need to copy persistent client-side only state from the 10th predicted frame to the slot the server
	*	update is occupying. )
	*	@param ps Current player entity state.
	*	@param pps Current predicted player entity state.
	*	@param pcd Current client state.
	*	@param ppcd Current predicted player entity state.
	*	@param wd Current weapon data list.
	*	@param pwd Current predicted weapon data list.
	*/
	HUD_TXFERPREDICTIONDATA_FUNC		pTxferPredictionData;

	/**
	*	Called by the engine while playing back a demo. The engine wants us to parse some data from the demo stream.
	*	@param size Buffer size, in bytes.
	*	@param buffer Buffer.
	*/
	HUD_DEMOREAD_FUNC					pReadDemoBuffer;

	/**
	*	Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
	*	size of the response_buffer, so you must zero it out if you choose not to respond.
	*	@param net_from Address of the sender.
	*	@param args Command arguments.
	*	@param response_buffer Buffer to write responses to.
	*	@param response_buffer_size Buffer size.
	*	@return true if the packet is valid, false otherwise.
	*/
	HUD_CONNECTIONLESS_FUNC				pConnectionlessPacket;

	/**
	*	Gets hull bounds for physics.
	*	@param hullnumber Hull number. @see Hull::Hull
	*	@param[ out ] mins Minimum bounds.
	*	@param[ out ] maxs Maximum bounds.
	*	@return true if the bounds were set, false otherwise.
	*/
	HUD_GETHULLBOUNDS_FUNC				pGetHullBounds;

	/**
	*	Called every frame that the client library is loaded.
	*	@param flFrameTime Time between the last and current frame.
	*/
	HUD_FRAME_FUNC						pHudFrame;

	/**
	*	Called when a key has changed state.
	*	@param bDown Whether they key is down or not.
	*	@param keynum Key number. @see KeyNum
	*	@param pszCurrentBinding Command bound to this key.
	*	@return true to allow engine to process the key, otherwise, act on it as needed.
	*/
	HUD_KEY_EVENT_FUNC					pKeyEvent;

	/**
	*	Simulation and cleanup of temporary entities.
	*	@param flFrameTime Time between the last and current frame.
	*	@param flClientTime Current client time.
	*	@param flCLGravity Client side gravity.
	*	@param ppTempEntFree List of free temporary entities.
	*	@param ppTempEntActive List of active temporary entities.
	*	@param pAddVisibleEnt Callback to add visible entities.
	*	@param pTmpPlaySound Callback to play sounds for temporary entities.
	*/
	HUD_TEMPENTUPDATE_FUNC				pTempEntUpdate;

	/**
	*	If you specify negative numbers for beam start and end point entities, then
	*	the engine will call back into this function requesting a pointer to a cl_entity_t
	*	object that describes the entity to attach the beam onto.
	*
	*	Indices must start at 1, not zero.
	*	@param index Entity index.
	*	@return Entity.
	*/
	HUD_GETUSERENTITY_FUNC				pGetUserEntity;

	/**
	*	Called when a player starts or stops talking.
	*	Possibly null on old client dlls.
	*	@param entindex Player index. 1 based. Is -1 when it's the local player, -2 if the server has acknowledged the local player is talking.
	*	@param bTalking Whether the player is currently talking or not.
	*/
	HUD_VOICESTATUS_FUNC				pVoiceStatus;		// Possibly null on old client dlls.

	/**
	*	Called when a director event message was received.
	*	Should be parsed like a user message.
	*	Possibly null on old client dlls.
	*	@param iSize Size of the buffer.
	*	@param pbuf Buffer.
	*/
	HUD_DIRECTORMESSAGE_FUNC			pDirectorMessage;	// Possibly null on old client dlls.

	/**
	*	Export this function for the engine to use the studio renderer class to render objects.
	*	Not used by all clients.
	*	@param version Interface version. Must be STUDIO_INTERFACE_VERSION. Return false if it doesn't match.
	*	@param[ out ] ppinterface Pointer to a pointer that should contain the studio interface.
	*	@param pstudio Engine studio interface.
	*	@return true if the requested interface was available, false otherwise.
	*	@see STUDIO_INTERFACE_VERSION
	*/
	HUD_STUDIO_INTERFACE_FUNC			pStudioInterface;	// Not used by all clients

	/**
	*	Gets the chat input position.
	*	Not used by all clients.
	*	@param x X position.
	*	@param y Y position.
	*/
	HUD_CHATINPUTPOSITION_FUNC			pChatInputPosition;	// Not used by all clients

	/**
	*	Doesn't appear to be called.
	*	Not used by all clients.
	*/
	HUD_GETPLAYERTEAM					pGetPlayerTeam; // Not used by all clients

	/**
	*	This should be CreateInterfaceFn but that means including interface.h
	*	which is a C++ file and some of the client files a C only...
	*	so we return a void * which we then do a typecast on later.
	*
	*	Never actually called, but must be provided in order for CreateInterface to be called.
	*/
	CLIENTFACTORY						pClientFactory;
} cldll_func_t;

// Function type declarations for client destination functions
typedef void (*DST_INITIALIZE_FUNC)	( struct cl_enginefuncs_s**, int *);
typedef void (*DST_HUD_INIT_FUNC)		( void );
typedef void (*DST_HUD_VIDINIT_FUNC)	( void );
typedef void (*DST_HUD_REDRAW_FUNC)	( float*, int* );
typedef void (*DST_HUD_UPDATECLIENTDATA_FUNC) ( struct client_data_s**, float* );
typedef void (*DST_HUD_RESET_FUNC)    ( void );
typedef void (*DST_HUD_CLIENTMOVE_FUNC)( struct playermove_s **, qboolean * );
typedef void (*DST_HUD_CLIENTMOVEINIT_FUNC)( struct playermove_s ** );
typedef void (*DST_HUD_TEXTURETYPE_FUNC)( char ** );
typedef void (*DST_HUD_IN_ACTIVATEMOUSE_FUNC) ( void );
typedef void (*DST_HUD_IN_DEACTIVATEMOUSE_FUNC)		( void );
typedef void (*DST_HUD_IN_MOUSEEVENT_FUNC)		( int * );
typedef void (*DST_HUD_IN_CLEARSTATES_FUNC)		( void );
typedef void (*DST_HUD_IN_ACCUMULATE_FUNC ) ( void );
typedef void (*DST_HUD_CL_CREATEMOVE_FUNC)		( float *, struct usercmd_s **, int * );
typedef void (*DST_HUD_CL_ISTHIRDPERSON_FUNC) ( void );
typedef void (*DST_HUD_CL_GETCAMERAOFFSETS_FUNC )( float ** );
typedef void (*DST_HUD_KB_FIND_FUNC) ( const char ** );
typedef void (*DST_HUD_CAMTHINK_FUNC )( void );
typedef void (*DST_HUD_CALCREF_FUNC ) ( struct ref_params_s ** );
typedef void (*DST_HUD_ADDENTITY_FUNC ) ( int *, struct cl_entity_s **, const char ** );
typedef void (*DST_HUD_CREATEENTITIES_FUNC ) ( void );
typedef void (*DST_HUD_DRAWNORMALTRIS_FUNC ) ( void );
typedef void (*DST_HUD_DRAWTRANSTRIS_FUNC ) ( void );
typedef void (*DST_HUD_STUDIOEVENT_FUNC ) ( const struct mstudioevent_s **, const struct cl_entity_s ** );
typedef void (*DST_HUD_POSTRUNCMD_FUNC ) ( struct local_state_s **, struct local_state_s **, struct usercmd_s **, int *, double *, unsigned int * );
typedef void (*DST_HUD_SHUTDOWN_FUNC ) ( void );
typedef void (*DST_HUD_TXFERLOCALOVERRIDES_FUNC )( struct entity_state_s **, const struct clientdata_s ** );
typedef void (*DST_HUD_PROCESSPLAYERSTATE_FUNC )( struct entity_state_s **, const struct entity_state_s ** );
typedef void (*DST_HUD_TXFERPREDICTIONDATA_FUNC ) ( struct entity_state_s **, const struct entity_state_s **, struct clientdata_s **, const struct clientdata_s **, struct weapon_data_s **, const struct weapon_data_s ** );
typedef void (*DST_HUD_DEMOREAD_FUNC ) ( int *, unsigned char ** );
typedef void (*DST_HUD_CONNECTIONLESS_FUNC )( const struct netadr_s **, const char **, char **, int ** );
typedef void (*DST_HUD_GETHULLBOUNDS_FUNC ) ( int *, float **, float ** );
typedef void (*DST_HUD_FRAME_FUNC)		( double * );
typedef void (*DST_HUD_KEY_EVENT_FUNC ) ( int *, int *, const char ** );
typedef void (*DST_HUD_TEMPENTUPDATE_FUNC) ( double *, double *, double *, struct tempent_s ***, struct tempent_s ***, int ( **Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ),	void ( **Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
typedef void (*DST_HUD_GETUSERENTITY_FUNC ) ( int * );
typedef void (*DST_HUD_VOICESTATUS_FUNC)(int *, qboolean *);
typedef void (*DST_HUD_DIRECTORMESSAGE_FUNC)( int *, void ** );
typedef void (*DST_HUD_STUDIO_INTERFACE_FUNC ) ( int *, struct r_studio_interface_s ***, struct engine_studio_api_s ** );
typedef void (*DST_HUD_CHATINPUTPOSITION_FUNC)( int **, int ** );
typedef void (*DST_HUD_GETPLAYERTEAM)(int);

// Pointers to the client destination functions
typedef struct
{
	DST_INITIALIZE_FUNC						pInitFunc;
	DST_HUD_INIT_FUNC						pHudInitFunc;
	DST_HUD_VIDINIT_FUNC					pHudVidInitFunc;
	DST_HUD_REDRAW_FUNC						pHudRedrawFunc;
	DST_HUD_UPDATECLIENTDATA_FUNC			pHudUpdateClientDataFunc;
	DST_HUD_RESET_FUNC						pHudResetFunc;
	DST_HUD_CLIENTMOVE_FUNC					pClientMove;
	DST_HUD_CLIENTMOVEINIT_FUNC				pClientMoveInit;
	DST_HUD_TEXTURETYPE_FUNC				pClientTextureType;
	DST_HUD_IN_ACTIVATEMOUSE_FUNC			pIN_ActivateMouse;
	DST_HUD_IN_DEACTIVATEMOUSE_FUNC			pIN_DeactivateMouse;
	DST_HUD_IN_MOUSEEVENT_FUNC				pIN_MouseEvent;
	DST_HUD_IN_CLEARSTATES_FUNC				pIN_ClearStates;
	DST_HUD_IN_ACCUMULATE_FUNC				pIN_Accumulate;
	DST_HUD_CL_CREATEMOVE_FUNC				pCL_CreateMove;
	DST_HUD_CL_ISTHIRDPERSON_FUNC			pCL_IsThirdPerson;
	DST_HUD_CL_GETCAMERAOFFSETS_FUNC		pCL_GetCameraOffsets;
	DST_HUD_KB_FIND_FUNC					pFindKey;
	DST_HUD_CAMTHINK_FUNC					pCamThink;
	DST_HUD_CALCREF_FUNC					pCalcRefdef;
	DST_HUD_ADDENTITY_FUNC					pAddEntity;
	DST_HUD_CREATEENTITIES_FUNC				pCreateEntities;
	DST_HUD_DRAWNORMALTRIS_FUNC				pDrawNormalTriangles;
	DST_HUD_DRAWTRANSTRIS_FUNC				pDrawTransparentTriangles;
	DST_HUD_STUDIOEVENT_FUNC				pStudioEvent;
	DST_HUD_POSTRUNCMD_FUNC					pPostRunCmd;
	DST_HUD_SHUTDOWN_FUNC					pShutdown;
	DST_HUD_TXFERLOCALOVERRIDES_FUNC		pTxferLocalOverrides;
	DST_HUD_PROCESSPLAYERSTATE_FUNC			pProcessPlayerState;
	DST_HUD_TXFERPREDICTIONDATA_FUNC		pTxferPredictionData;
	DST_HUD_DEMOREAD_FUNC					pReadDemoBuffer;
	DST_HUD_CONNECTIONLESS_FUNC				pConnectionlessPacket;
	DST_HUD_GETHULLBOUNDS_FUNC				pGetHullBounds;
	DST_HUD_FRAME_FUNC						pHudFrame;
	DST_HUD_KEY_EVENT_FUNC					pKeyEvent;
	DST_HUD_TEMPENTUPDATE_FUNC				pTempEntUpdate;
	DST_HUD_GETUSERENTITY_FUNC				pGetUserEntity;
	DST_HUD_VOICESTATUS_FUNC				pVoiceStatus;	// Possibly null on old client dlls.
	DST_HUD_DIRECTORMESSAGE_FUNC			pDirectorMessage;	// Possibly null on old client dlls.
	DST_HUD_STUDIO_INTERFACE_FUNC			pStudioInterface;  // Not used by all clients
	DST_HUD_CHATINPUTPOSITION_FUNC			pChatInputPosition;  // Not used by all clients
	DST_HUD_GETPLAYERTEAM					pGetPlayerTeam; // Not used by all clients
} cldll_func_dst_t;




// ********************************************************
// Functions exported by the engine
// ********************************************************

// Function type declarations for engine exports
typedef HSPRITE						(*pfnEngSrc_pfnSPR_Load_t )			( const char *szPicName );
typedef int							(*pfnEngSrc_pfnSPR_Frames_t )			( HSPRITE hPic );
typedef int							(*pfnEngSrc_pfnSPR_Height_t )			( HSPRITE hPic, int frame );
typedef int							(*pfnEngSrc_pfnSPR_Width_t )			( HSPRITE hPic, int frame );
typedef void						(*pfnEngSrc_pfnSPR_Set_t )				( HSPRITE hPic, int r, int g, int b );
typedef void						(*pfnEngSrc_pfnSPR_Draw_t )			( int frame, int x, int y, const struct rect_s *prc );
typedef void						(*pfnEngSrc_pfnSPR_DrawHoles_t )		( int frame, int x, int y, const struct rect_s *prc );
typedef void						(*pfnEngSrc_pfnSPR_DrawAdditive_t )	( int frame, int x, int y, const struct rect_s *prc );
typedef void						(*pfnEngSrc_pfnSPR_EnableScissor_t )	( int x, int y, int width, int height );
typedef void						(*pfnEngSrc_pfnSPR_DisableScissor_t )	( void );
typedef struct client_sprite_s	*	(*pfnEngSrc_pfnSPR_GetList_t )			(const char *psz, int *piCount );
typedef void						(*pfnEngSrc_pfnFillRGBA_t )			( int x, int y, int width, int height, int r, int g, int b, int a );
typedef int							(*pfnEngSrc_pfnGetScreenInfo_t ) 		( struct SCREENINFO_s *pscrinfo );
typedef void						(*pfnEngSrc_pfnSetCrosshair_t )		( HSPRITE hspr, wrect_t rc, int r, int g, int b );
typedef struct cvar_s *				(*pfnEngSrc_pfnRegisterVariable_t )	( const char *szName, const char *szValue, int flags );
typedef float						(*pfnEngSrc_pfnGetCvarFloat_t )		( const char *szName );
typedef char*						(*pfnEngSrc_pfnGetCvarString_t )		( const char *szName );
typedef int							(*pfnEngSrc_pfnAddCommand_t )			( const char *cmd_name, void (*pfnEngSrc_function)(void) );
typedef int							(*pfnEngSrc_pfnHookUserMsg_t )			( const char *szMsgName, pfnUserMsgHook pfn );
typedef int							(*pfnEngSrc_pfnServerCmd_t )			( const char *szCmdString );
typedef int							(*pfnEngSrc_pfnClientCmd_t )			( const char *szCmdString );
typedef void						(*pfnEngSrc_pfnPrimeMusicStream_t )	(const char *szFilename, int looping );
typedef void						(*pfnEngSrc_pfnGetPlayerInfo_t )		( int ent_num, struct hud_player_info_s *pinfo );
typedef void						(*pfnEngSrc_pfnPlaySoundByName_t )		( const char *szSound, float volume );
typedef void						(*pfnEngSrc_pfnPlaySoundByNameAtPitch_t )	( const char *szSound, float volume, int pitch );
typedef void						(*pfnEngSrc_pfnPlaySoundVoiceByName_t )		( const char *szSound, float volume, int pitch );
typedef void						(*pfnEngSrc_pfnPlaySoundByIndex_t )	( int iSound, float volume );
typedef void						(*pfnEngSrc_pfnAngleVectors_t )		( const float * vecAngles, float * forward, float * right, float * up );
typedef struct client_textmessage_s*(*pfnEngSrc_pfnTextMessageGet_t )		( const char *pName );
typedef int							(*pfnEngSrc_pfnDrawCharacter_t )		( int x, int y, int number, int r, int g, int b );
typedef int							(*pfnEngSrc_pfnDrawConsoleString_t )	( int x, int y, char *string );
typedef void						(*pfnEngSrc_pfnDrawSetTextColor_t )	( float r, float g, float b );
typedef void						(*pfnEngSrc_pfnDrawConsoleStringLen_t )(  const char *string, int *length, int *height );
typedef void						(*pfnEngSrc_pfnConsolePrint_t )		( const char *string );
typedef void						(*pfnEngSrc_pfnCenterPrint_t )			( const char *string );
typedef int							(*pfnEngSrc_GetWindowCenterX_t )		( void );
typedef int							(*pfnEngSrc_GetWindowCenterY_t )		( void );
typedef void						(*pfnEngSrc_GetViewAngles_t )			( float * );
typedef void						(*pfnEngSrc_SetViewAngles_t )			( float * );
typedef int							(*pfnEngSrc_GetMaxClients_t )			( void );
typedef void						(*pfnEngSrc_Cvar_SetValue_t )			( const char *cvar, float value );
typedef int       					(*pfnEngSrc_Cmd_Argc_t)					(void);	
typedef char *						(*pfnEngSrc_Cmd_Argv_t )				( int arg );
typedef void						(*pfnEngSrc_Con_Printf_t )				( const char *fmt, ... );
typedef void						(*pfnEngSrc_Con_DPrintf_t )			( const char *fmt, ... );
typedef void						(*pfnEngSrc_Con_NPrintf_t )			( int pos, const char *fmt, ... );
typedef void						(*pfnEngSrc_Con_NXPrintf_t )			( struct con_nprint_s *info, const char *fmt, ... );
typedef const char *				(*pfnEngSrc_PhysInfo_ValueForKey_t )	( const char *key );
typedef const char *				(*pfnEngSrc_ServerInfo_ValueForKey_t )( const char *key );
typedef float						(*pfnEngSrc_GetClientMaxspeed_t )		( void );
typedef int							(*pfnEngSrc_CheckParm_t )				( const char *parm, char **ppnext );
typedef void						(*pfnEngSrc_Key_Event_t )				( int key, int down );
typedef void						(*pfnEngSrc_GetMousePosition_t )		( int *mx, int *my );
typedef int							(*pfnEngSrc_IsNoClipping_t )			( void );
typedef struct cl_entity_s *		(*pfnEngSrc_GetLocalPlayer_t )		( void );
typedef struct cl_entity_s *		(*pfnEngSrc_GetViewModel_t )			( void );
typedef struct cl_entity_s *		(*pfnEngSrc_GetEntityByIndex_t )		( int idx );
typedef float						(*pfnEngSrc_GetClientTime_t )			( void );
typedef void						(*pfnEngSrc_V_CalcShake_t )			( void );
typedef void						(*pfnEngSrc_V_ApplyShake_t )			( float *origin, float *angles, float factor );
typedef int							(*pfnEngSrc_PM_PointContents_t )		( float *point, int *truecontents );
typedef int							(*pfnEngSrc_PM_WaterEntity_t )			( float *p );
typedef struct pmtrace_s *			(*pfnEngSrc_PM_TraceLine_t )			( float *start, float *end, int flags, int usehull, int ignore_pe );
typedef struct model_s *			(*pfnEngSrc_CL_LoadModel_t )			( const char *modelname, int *index );
typedef int							(*pfnEngSrc_CL_CreateVisibleEntity_t )	( int type, struct cl_entity_s *ent );
typedef const struct model_s *		(*pfnEngSrc_GetSpritePointer_t )		( HSPRITE hSprite );
typedef void						(*pfnEngSrc_pfnPlaySoundByNameAtLocation_t )	( const char *szSound, float volume, float *origin );
typedef unsigned short				(*pfnEngSrc_pfnPrecacheEvent_t )		( int type, const char* psz );
typedef void						(*pfnEngSrc_pfnPlaybackEvent_t )		( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
typedef void						(*pfnEngSrc_pfnWeaponAnim_t )			( int iAnim, int body );
typedef float						(*pfnEngSrc_pfnRandomFloat_t )			( float flLow, float flHigh );
typedef int32						(*pfnEngSrc_pfnRandomLong_t )			( int32 lLow, int32 lHigh );
typedef void						(*pfnEngSrc_pfnHookEvent_t )			( const char *name, void ( *pfnEvent )( struct event_args_s *args ) );
typedef int							(*pfnEngSrc_Con_IsVisible_t)			();
typedef const char *				(*pfnEngSrc_pfnGetGameDirectory_t )	( void );
typedef struct cvar_s *				(*pfnEngSrc_pfnGetCvarPointer_t )		( const char *szName );
typedef const char *				(*pfnEngSrc_Key_LookupBinding_t )		( const char *pBinding );
typedef const char *				(*pfnEngSrc_pfnGetLevelName_t )		( void );
typedef void						(*pfnEngSrc_pfnGetScreenFade_t )		( struct screenfade_s *fade );
typedef void						(*pfnEngSrc_pfnSetScreenFade_t )		( struct screenfade_s *fade );
typedef void *						(*pfnEngSrc_VGui_GetPanel_t )         ( );
typedef void                        (*pfnEngSrc_VGui_ViewportPaintBackground_t ) (int extents[4]);
typedef byte*						(*pfnEngSrc_COM_LoadFile_t )				( const char *path, int usehunk, int *pLength );
typedef char*						(*pfnEngSrc_COM_ParseFile_t )			( const char *data, char *token );
typedef void						(*pfnEngSrc_COM_FreeFile_t)				( void *buffer );
typedef struct triangleapi_s *		pTriAPI;
typedef struct efx_api_s *			pEfxAPI;
typedef struct event_api_s *		pEventAPI;
typedef struct demo_api_s *			pDemoAPI;
typedef struct net_api_s *			pNetAPI;
typedef struct IVoiceTweak_s *		pVoiceTweak;
typedef int							(*pfnEngSrc_IsSpectateOnly_t ) ( void );
typedef struct model_s *			(*pfnEngSrc_LoadMapSprite_t )			( const char *filename );
typedef void						(*pfnEngSrc_COM_AddAppDirectoryToSearchPath_t ) ( const char *pszBaseDir, const char *appName );
typedef int							(*pfnEngSrc_COM_ExpandFilename_t)				 ( const char *fileName, char *nameOutBuffer, int nameOutBufferSize );
typedef const char *				(*pfnEngSrc_PlayerInfo_ValueForKey_t )( int playerNum, const char *key );
typedef void						(*pfnEngSrc_PlayerInfo_SetValueForKey_t )( const char *key, const char *value );
typedef qboolean					(*pfnEngSrc_GetPlayerUniqueID_t)(int iPlayer, char playerID[16]);
typedef int							(*pfnEngSrc_GetTrackerIDForPlayer_t)(int playerSlot);
typedef int							(*pfnEngSrc_GetPlayerForTrackerID_t)(int trackerID);
typedef int							(*pfnEngSrc_pfnServerCmdUnreliable_t )( char *szCmdString );
typedef void						(*pfnEngSrc_GetMousePos_t )(struct tagPOINT *ppt);
typedef void						(*pfnEngSrc_SetMousePos_t )(int x, int y);
typedef void						(*pfnEngSrc_SetMouseEnable_t)(qboolean fEnable);
typedef struct cvar_s *				(*pfnEngSrc_GetFirstCVarPtr_t)();
typedef unsigned int				(*pfnEngSrc_GetFirstCmdFunctionHandle_t)();
typedef unsigned int				(*pfnEngSrc_GetNextCmdFunctionHandle_t)(unsigned int cmdhandle);
typedef const char *				(*pfnEngSrc_GetCmdFunctionName_t)(unsigned int cmdhandle);
typedef float						(*pfnEngSrc_GetClientOldTime_t)();
typedef float						(*pfnEngSrc_GetServerGravityValue_t)();
typedef struct model_s	*			(*pfnEngSrc_GetModelByIndex_t)( int index );
typedef void						(*pfnEngSrc_pfnSetFilterMode_t )( int mode );
typedef void						(*pfnEngSrc_pfnSetFilterColor_t )( float r, float g, float b );
typedef void						(*pfnEngSrc_pfnSetFilterBrightness_t )( float brightness );
typedef sequenceEntry_s*			(*pfnEngSrc_pfnSequenceGet_t )( const char *fileName, const char* entryName );
typedef void						(*pfnEngSrc_pfnSPR_DrawGeneric_t )( int frame, int x, int y, const struct rect_s *prc, int src, int dest, int w, int h );
typedef sentenceEntry_s*			(*pfnEngSrc_pfnSequencePickSentence_t )( const char *sentenceName, int pickMethod, int* entryPicked );
// draw a complete string
typedef int							(*pfnEngSrc_pfnDrawString_t )		( int x, int y, const char *str, int r, int g, int b );
typedef int							(*pfnEngSrc_pfnDrawStringReverse_t )		( int x, int y, const char *str, int r, int g, int b );
typedef const char *				(*pfnEngSrc_LocalPlayerInfo_ValueForKey_t )( const char *key );
typedef int							(*pfnEngSrc_pfnVGUI2DrawCharacter_t )		( int x, int y, int ch, unsigned int font );
typedef int							(*pfnEngSrc_pfnVGUI2DrawCharacterAdd_t )	( int x, int y, int ch, int r, int g, int b, unsigned int font);
typedef unsigned int		(*pfnEngSrc_COM_GetApproxWavePlayLength ) ( const char * filename);
typedef void *						(*pfnEngSrc_pfnGetCareerUI_t)();
typedef void						(*pfnEngSrc_Cvar_Set_t )			( char *cvar, char *value );
typedef int							(*pfnEngSrc_pfnIsPlayingCareerMatch_t)();
typedef double						(*pfnEngSrc_GetAbsoluteTime_t) ( void );
typedef void						(*pfnEngSrc_pfnProcessTutorMessageDecayBuffer_t)(int *buffer, int bufferLength);
typedef void						(*pfnEngSrc_pfnConstructTutorMessageDecayBuffer_t)(int *buffer, int bufferLength);
typedef void						(*pfnEngSrc_pfnResetTutorMessageDecayData_t)();
typedef void						(*pfnEngSrc_pfnFillRGBABlend_t )			( int x, int y, int width, int height, int r, int g, int b, int a );
typedef int						(*pfnEngSrc_pfnGetAppID_t)			( void );
typedef cmdalias_t*				(*pfnEngSrc_pfnGetAliases_t)		( void );
typedef void					(*pfnEngSrc_pfnVguiWrap2_GetMouseDelta_t) ( int *x, int *y );
typedef int							(*pfnEngSrc_pfnFilteredClientCmd_t) 	( char *szCmdString );

// Pointers to the exported engine functions themselves
typedef struct cl_enginefuncs_s
{
	/**
	*	Loads a sprite by name.
	*	A maximum of 256 HUD sprites can be loaded at the same time.
	*	@param pszPicName Name of the sprite to load. Must include the sprites directory name and the extension.
	*	@return Handle to the sprite.
	*/
	pfnEngSrc_pfnSPR_Load_t					pfnSPR_Load;

	/**
	*	Gets the number of frames in the sprite.
	*	@param hPic Handle to the sprite.
	*	@return Frame count.
	*/
	pfnEngSrc_pfnSPR_Frames_t				pfnSPR_Frames;

	/**
	*	Gets the height of a given sprite frame.
	*	@param hPic Handle to the sprite.
	*	@param frame Frame number.
	*	@return Height in pixels.
	*/
	pfnEngSrc_pfnSPR_Height_t				pfnSPR_Height;

	/**
	*	Gets the width of a given sprite frame.
	*	@param hPic Handle to the sprite.
	*	@param frame Frame number.
	*	@return Width in pixels.
	*/
	pfnEngSrc_pfnSPR_Width_t				pfnSPR_Width;
	
	/**
	*	Sets the sprite to draw, and its color.
	*	@param hPic Handle to the sprite.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*/
	pfnEngSrc_pfnSPR_Set_t					pfnSPR_Set;

	/**
	*	Draws the current sprite as solid.
	*	@param frame Frame to draw.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param prc Optional. Defines the rectangle of the sprite frame to draw.
	*	@see pfnSPR_Set
	*/
	pfnEngSrc_pfnSPR_Draw_t					pfnSPR_Draw;

	/**
	*	Draws the current sprite with color index255 not drawn (transparent).
	*	@param frame Frame to draw.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param prc Optional. Defines the rectangle of the sprite frame to draw.
	*	@see pfnSPR_Set
	*/
	pfnEngSrc_pfnSPR_DrawHoles_t			pfnSPR_DrawHoles;

	/**
	*	Draws the current sprite, adds the sprites RGB values to the background (additive translucency).
	*	@param frame Frame to draw.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param prc Optional. Defines the rectangle of the sprite frame to draw.
	*	@see pfnSPR_Set
	*/
	pfnEngSrc_pfnSPR_DrawAdditive_t			pfnSPR_DrawAdditive;

	/**
	*	Dets a clipping rect for HUD sprites. (0,0) is the top-left hand corner of the screen.
	*	@param x Left coordinate of the box.
	*	@param y Top coordinate of the box.
	*	@param width Width of the box.
	*	@param height Height of the box.
	*/
	pfnEngSrc_pfnSPR_EnableScissor_t		pfnSPR_EnableScissor;

	/**
	*	Disables the scissor box.
	*/
	pfnEngSrc_pfnSPR_DisableScissor_t		pfnSPR_DisableScissor;

	/**
	*	Loads a sprite list. This is a text file defining a list of HUD elements.
	*	Free the returned list with COM_FreeFile.
	*	@param pszName Name of the file to load. Should include the sprites directory and the extension.
	*	@param[ out ] piCount Optional. Pointer to a variable that will contain the number of entries in the list.
	*	@return List of sprites.
	*/
	pfnEngSrc_pfnSPR_GetList_t				pfnSPR_GetList;

	/**
	*	Fills the given rectangle with a given color.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param width Width of the rectangle.
	*	@param height Height of the rectangle.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@param a Alpha value. [ 0, 255 ].
	*/
	pfnEngSrc_pfnFillRGBA_t					pfnFillRGBA;

	/**
	*	Gets screen info.
	*	The SCREENINFO::iSize member must be set to sizeof( SCREENINFO ).
	*	@param psrcinfo Pointer to a SCREENINFO instance that will receive the information.
	*	@return Number of bytes that have been written. 0 if nothing was written.
	*/
	pfnEngSrc_pfnGetScreenInfo_t			pfnGetScreenInfo;

	/**
	*	Sets the crosshair sprite.
	*	@param hPic Handle to the sprite.
	*	@param rc Rectangle that defines the crosshair box to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*/
	pfnEngSrc_pfnSetCrosshair_t				pfnSetCrosshair;

	/**
	*	Registers a new cvar. Avoid calling with the same name more than once.
	*	@param pszName Name of the cvar. Must point to a string that will exist for the rest of the program's lifetime.
	*	@param pszValue Value to set. Can be a temporary string.
	*	@param flags CVar flags.
	*	@return Pointer to the cvar.
	*/
	pfnEngSrc_pfnRegisterVariable_t			pfnRegisterVariable;

	/**
	*	Gets the float value of a cvar.
	*	@param pszName CVar name.
	*	@return Value, or 0 if the cvar doesn't exist.
	*/
	pfnEngSrc_pfnGetCvarFloat_t				pfnGetCvarFloat;

	/**
	*	Gets the string value of a cvar.
	*	@param pszName CVar name.
	*	@return Value, or nullptr if the cvar doesn't exist.
	*/
	pfnEngSrc_pfnGetCvarString_t			pfnGetCvarString;

	/**
	*	Adds a new command.
	*	@param pszCmdName Command name. Must point to a string that will exist for the rest of the program's lifetime.
	*	@param pCallback Callback to invoke when the command is executed. If null, forwards the command to the server.
	*	@return true in all cases.
	*/
	pfnEngSrc_pfnAddCommand_t				pfnAddCommand;

	/**
	*	Hooks a user message.
	*	@param pszMsgName Name of the message. Can be a temporary string.
	*	@param pfn Callback to invoke when the message is received.
	*	@return true if the command was already registered with the same callback, false otherwise.
	*/
	pfnEngSrc_pfnHookUserMsg_t				pfnHookUserMsg;

	/**
	*	Sends a command to the server.
	*	@param pszCmdString Command string.
	*	@return false in all cases.
	*/
	pfnEngSrc_pfnServerCmd_t				pfnServerCmd;

	/**
	*	Enqueues a command for execution for the local client.
	*	@param pszCmdString Command string.
	*	@return true if the command was enqueued, false otherwise.
	*/
	pfnEngSrc_pfnClientCmd_t				pfnClientCmd;

	/**
	*	Gets player info.
	*	@param ent_num 1 based player entity index.
	*	@param pinfo Structure that will contain the player's information.
	*/
	pfnEngSrc_pfnGetPlayerInfo_t			pfnGetPlayerInfo;

	/**
	*	Plays a sound by name.
	*	@param pszSound Name of the sound.
	*	@param volume Volume to play at. [ 0, 1 ].
	*/
	pfnEngSrc_pfnPlaySoundByName_t			pfnPlaySoundByName;

	/**
	*	Plays a sound by index.
	*	@param iSound Index of the sound.
	*	@param volume Volume to play at. [ 0, 1 ].
	*/
	pfnEngSrc_pfnPlaySoundByIndex_t			pfnPlaySoundByIndex;

	/**
	*	Converts angles to directional vectors.
	*	Superseded by AngleVectors.
	*	@param vecAngles Angles.
	*	@param[ out ] forward Forward vector.
	*	@param[ out ] right Right vector.
	*	@param[ out ] up Up vector.
	*	@see AngleVectors
	*/
	pfnEngSrc_pfnAngleVectors_t				pfnAngleVectors;

	/**
	*	Gets a text message, defined in titles.txt.
	*	@param pszName Text message name.
	*					Must be either a message defined in titles.txt, or one of the following:
	*					__DEMOMESSAGE__
	*					__NETMESSAGE__1
	*					__NETMESSAGE__2
	*					__NETMESSAGE__3
	*					__NETMESSAGE__4
	*	@return Text message, or null if no text message could be retrieved.
	*/
	pfnEngSrc_pfnTextMessageGet_t			pfnTextMessageGet;

	/**
	*	Draws a single character.
	*	@param x Left position.
	*	@param y Top position.
	*	@param number Character to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@return Total width of the drawn character.
	*/
	pfnEngSrc_pfnDrawCharacter_t			pfnDrawCharacter;

	/**
	*	Draws a string.
	*	@param x Left position.
	*	@param y Top position.
	*	@param pszString String to draw.
	*	@return Total width of the drawn string.
	*/
	pfnEngSrc_pfnDrawConsoleString_t		pfnDrawConsoleString;

	/**
	*	Sets the text color.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*/
	pfnEngSrc_pfnDrawSetTextColor_t			pfnDrawSetTextColor;

	/**
	*	Gets the length in pixels of a string if it were drawn onscreen.
	*	@param pszString String to check.
	*	@param piLength Pointer to a variable that will contain the total width of the string.
	*	@param piHeight Pointer to a variable that will contain the height of the string.
	*/
	pfnEngSrc_pfnDrawConsoleStringLen_t		pfnDrawConsoleStringLen;

	/**
	*	Prints a string to the console.
	*	@param pszString String to print.
	*/
	pfnEngSrc_pfnConsolePrint_t				pfnConsolePrint;

	/**
	*	Prints a string to the center of the screen.
	*	@param pszString String to print.
	*/
	pfnEngSrc_pfnCenterPrint_t				pfnCenterPrint;

	/**
	*	@return The center of the screen's X axis.
	*/
	pfnEngSrc_GetWindowCenterX_t			GetWindowCenterX;

	/**
	*	@return The center of the screen's Y axis.
	*/
	pfnEngSrc_GetWindowCenterY_t			GetWindowCenterY;

	/**
	*	Gets the view angles.
	*	@param[ out ] vecAngles Will contain the view angles.
	*/
	pfnEngSrc_GetViewAngles_t				GetViewAngles;

	/**
	*	Sets the view angles.
	*	@param vecAngles Angles to set.
	*/
	pfnEngSrc_SetViewAngles_t				SetViewAngles;

	/**
	*	@return The maximum number of clients that can be connected to the current server.
	*/
	pfnEngSrc_GetMaxClients_t				GetMaxClients;

	/**
	*	Sets the value of a cvar.
	*	@param pszCVarName Name of the cvar.
	*	@param value Value to set.
	*/
	pfnEngSrc_Cvar_SetValue_t				Cvar_SetValue;

	/**
	*	@return the number of arguments in the command that is currently being executed.
	*/
	pfnEngSrc_Cmd_Argc_t					Cmd_Argc;

	/**
	*	@param arg Argument index.
	*	@return Argument at the given index.
	*/
	pfnEngSrc_Cmd_Argv_t					Cmd_Argv;

	/**
	*	Prints to the console.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	pfnEngSrc_Con_Printf_t					Con_Printf;

	/**
	*	Prints to the console if developer mode is enabled.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	pfnEngSrc_Con_DPrintf_t					Con_DPrintf;

	/**
	*	Prints to the notify area.
	*	@param pos Position in the notify list to set this message to. [ 0, 32 [.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	pfnEngSrc_Con_NPrintf_t					Con_NPrintf;

	/**
	*	Prints to the notify area.
	*	@param info Notify print info.
	*	@param pszFormat Format string.
	*	@param ... Arguments.
	*/
	pfnEngSrc_Con_NXPrintf_t				Con_NXPrintf;

	/**
	*	Given a key, gets the physics value.
	*	@param pszKey Key.
	*	@return Pointer to the value, or an empty string if the key couldn't be found.
	*/
	pfnEngSrc_PhysInfo_ValueForKey_t		PhysInfo_ValueForKey;

	/**
	*	Given a key, gets the server info value.
	*	@param pszKey Key.
	*	@return Pointer to the value, or an empty string if the key couldn't be found.
	*/
	pfnEngSrc_ServerInfo_ValueForKey_t		ServerInfo_ValueForKey;

	/**
	*	@return The client's maximum speed.
	*/
	pfnEngSrc_GetClientMaxspeed_t			GetClientMaxspeed;

	/**
	*	Checks if the given parameter was provided on the command line.
	*	@param pszParm Parameter to check.
	*	@param[ out ] ppszNext Optional. If the parameter was provided, points to the value for the given parameter.
	*	@return Parameter index in the argument vector. 0 if it wasn't found.
	*/
	pfnEngSrc_CheckParm_t					CheckParm;

	/**
	*	Triggers a key event.
	*	@param key Key number. @see KeyNum.
	*	@param bDown Whether the key is down or up.
	*/
	pfnEngSrc_Key_Event_t					Key_Event;

	/**
	*	Gets the mouse position on-screen.
	*	@param mx X position.
	*	@param my Y position.
	*/
	pfnEngSrc_GetMousePosition_t			GetMousePosition;

	/**
	*	@return Whether the player is currently noclipping.
	*/
	pfnEngSrc_IsNoClipping_t				IsNoClipping;

	/**
	*	Note: do not call until a map has been loaded. Will access invalid memory otherwise, and return a garbage pointer.
	*	Will be valid if called after HUD_VidInit has returned.
	*	@return The entity that represents the local player.
	*/
	pfnEngSrc_GetLocalPlayer_t				GetLocalPlayer;

	/**
	*	@return The entity that represents the player's viewmodel.
	*/
	pfnEngSrc_GetViewModel_t				GetViewModel;

	/**
	*	Gets an entity by index. Note: do not call until a map has been loaded. Will return a null pointer otherwise.
	*	Will be valid if called after HUD_VidInit has returned.
	*	@param idx Index. 0 based.
	*	@return The entity, or null if the index is invalid.
	*/
	pfnEngSrc_GetEntityByIndex_t			GetEntityByIndex;

	/**
	*	@return Current client time.
	*/
	pfnEngSrc_GetClientTime_t				GetClientTime;

	/**
	*	Calculates the current shake settings.
	*/
	pfnEngSrc_V_CalcShake_t					V_CalcShake;

	/**
	*	Applies the shake settings.
	*	@param[ in, out ] vecOrigin Original origin. Will contain the new origin.
	*	@param[ in, out ] vecAngles Original angles. Will contain the new angles.
	*	@param flFactor Factor by which to multiply the shake.
	*/
	pfnEngSrc_V_ApplyShake_t				V_ApplyShake;

	/**
	*	Gets the contents of the given point.
	*	The real contents can contain water current data.
	*	@param vecPoint Point to check.
	*	@param piTruecontents The real contents.
	*	@return Contents.
	*/
	pfnEngSrc_PM_PointContents_t			PM_PointContents;

	/**
	*	Gets the index of the water entity at the given position.
	*	@param vecPosition Position to look for the entity.
	*	@return Entity index. -1 if no water entity was found.
	*/
	pfnEngSrc_PM_WaterEntity_t				PM_WaterEntity;

	/**
	*	Performs a traceline.
	*	@param vecStart Starting point.
	*	@param vecEnd End point.
	*	@param flags Flags.
	*	@param usehull Hull to use. @see Hull::Hull
	*	@param ignore_pe Index of the entity to ignore. -1 if none should be ignored.
	*	@return Pointer to a statically allocated trace result instance.
	*/
	pfnEngSrc_PM_TraceLine_t				PM_TraceLine;

	/**
	*	Loads a model.
	*	@param pszModelName Name of the model to load. Starts in the game directory, must include the extension.
	*	@param[ out ] piIndex Optional. Will contain the index of the model. -1 if loading failed.
	*	@return Pointer to the model.
	*/
	pfnEngSrc_CL_LoadModel_t				CL_LoadModel;

	/**
	*	Creates a new visible entity.
	*	@param type Entity type. @see EntityType
	*	@param ent Entity.
	*	@return true if the entity was successfully added, false otherwise.
	*/
	pfnEngSrc_CL_CreateVisibleEntity_t		CL_CreateVisibleEntity;

	/**
	*	Gets the model that is represented by the given sprite handle.
	*	@param hSprite Handle to the sprite.
	*	@return Pointer to the model, or null if the handle is invalid.
	*/
	pfnEngSrc_GetSpritePointer_t			GetSpritePointer;

	/**
	*	Plays a sound by name at a given location.
	*	@param pszSoundName Name of the sound.
	*	@param volume Sound volume. [ 0, 1 ].
	*	@param vecOrigin Location where the sound should be played.
	*/
	pfnEngSrc_pfnPlaySoundByNameAtLocation_t	pfnPlaySoundByNameAtLocation;

	/**
	*	Precaches an event.
	*	@param type Type. Must be 1.
	*	@param pszName Name of the event.
	*	@return Event index, or 0 if the event couldn't be found.
	*/
	pfnEngSrc_pfnPrecacheEvent_t			pfnPrecacheEvent;

	/**
	*	@param flags Event flags.
	*	@param pInvoker Client that triggered the event.
	*	@param eventindex Event index. @see pfnPrecacheEvent
	*	@param delay Delay before the event should be run.
	*	@param origin If not g_vecZero, this is the origin parameter sent to the clients.
	*	@param angles If not g_vecZero, this is the angles parameter sent to the clients.
	*	@param fparam1 Float parameter 1.
	*	@param fparam2 Float parameter 2.
	*	@param iparam1 Integer parameter 1.
	*	@param iparam2 Integer parameter 2.
	*	@param bparam1 Boolean parameter 1.
	*	@param bparam2 Boolean parameter 2.
	*/
	pfnEngSrc_pfnPlaybackEvent_t			pfnPlaybackEvent;

	/**
	*	Sets the weapon animation and body.
	*	@param iAnim Animation index.
	*	@param body Body to set.
	*/
	pfnEngSrc_pfnWeaponAnim_t				pfnWeaponAnim;

	/**
	*	Generates a random float number in the range [ flLow, flLow ].
	*	@param flLow Lower bound.
	*	@param flHigh Higher bound.
	*	@return Random number.
	*/
	pfnEngSrc_pfnRandomFloat_t				pfnRandomFloat;

	/**
	*	Generates a random long number in the range [ lLow, lHigh ].
	*	@param lLow Lower bound.
	*	@param lHigh Higher bound.
	*	@return Random number, or lLow if lHigh is smaller than or equal to lLow.
	*/
	pfnEngSrc_pfnRandomLong_t				pfnRandomLong;

	/**
	*	Adds a hook for an event.
	*	@param pszName Name of the event.
	*	@param pEventHook Hook to invoke when the event is triggered.
	*/
	pfnEngSrc_pfnHookEvent_t				pfnHookEvent;

	/**
	*	@return Whether the console is currently visible.
	*/
	pfnEngSrc_Con_IsVisible_t				Con_IsVisible;

	/**
	*	@return Name of the game/mod directory.
	*/
	pfnEngSrc_pfnGetGameDirectory_t			pfnGetGameDirectory;

	/**
	*	Gets a cvar by name.
	*	@param pszName Name of the cvar.
	*	@return Pointer to the cvar, or null if it couldn't be found.
	*/
	pfnEngSrc_pfnGetCvarPointer_t			pfnGetCvarPointer;

	/**
	*	Gets the name of the key that is bound to the given command.
	*	@param pszBinding Command.
	*	@return Key name, or "<UNKNOWN KEYNUM>" if it couldn't be found.
	*/
	pfnEngSrc_Key_LookupBinding_t			Key_LookupBinding;

	/**
	*	@return The name of the level that is currently loaded. Has the format "maps/%s.bsp", where %s is the level name.
	*/
	pfnEngSrc_pfnGetLevelName_t				pfnGetLevelName;

	/**
	*	Gets the current screen fade settings.
	*	@param fade Structure that will contain the result.
	*/
	pfnEngSrc_pfnGetScreenFade_t			pfnGetScreenFade;

	/**
	*	Sets the current screen fade settings.
	*	@param fade Structure that contains the new settings.
	*/
	pfnEngSrc_pfnSetScreenFade_t			pfnSetScreenFade;

	/**
	*	@return The root VGUI1 panel to use for the viewport.
	*/
	pfnEngSrc_VGui_GetPanel_t				VGui_GetPanel;

	/**
	*	Paints the VGUI1 viewport background.
	*	Only safe to call from inside subclass of Panel::paintBackground.
	*	@param extents Viewport extents. Contains x1 y1 x2 y2 coordinates.
	*/
	pfnEngSrc_VGui_ViewportPaintBackground_t	VGui_ViewportPaintBackground;

	/**
	*	Loads a file.
	*	@param pszPath Path to the file.
	*	@param usehunk Hunk to use. Must always be 5.
	*	@param[ out ] piLength Optional. Length of the file, in bytes. 0 if the file couldn't be loaded.
	*	@return Pointer to buffer, or null if the file couldn't be loaded.
	*/
	pfnEngSrc_COM_LoadFile_t				COM_LoadFile;

	/**
	*	Parses the given data.
	*	@param pszData Data to parse.
	*	@param[ out ] pszToken Destination buffer for the token. Should be at least 1024 characters large.
	*	@return Pointer to the next character to parse.
	*/
	pfnEngSrc_COM_ParseFile_t				COM_ParseFile;

	/**
	*	Frees the given buffer. Calls free() on it.
	*	@param pBuffer Buffer to free. Can be null, in which case nothing is done.
	*/
	pfnEngSrc_COM_FreeFile_t				COM_FreeFile;
	
	/**
	*	Triangle API. Used to draw 3D geometry.
	*/
	struct triangleapi_s		*pTriAPI;

	/**
	*	Effects API.
	*/
	struct efx_api_s			*pEfxAPI;

	/**
	*	Event API.
	*/
	struct event_api_s			*pEventAPI;

	/**
	*	Demo API.
	*/
	struct demo_api_s			*pDemoAPI;

	/**
	*	Networking API.
	*/
	struct net_api_s			*pNetAPI;

	/**
	*	Voice Tweak API.
	*/
	struct IVoiceTweak_s		*pVoiceTweak;

	/**
	*	@return Whether this client is in spectator only mode (HLTV).
	*/
	pfnEngSrc_IsSpectateOnly_t				IsSpectateOnly;

	/**
	*	Loads a map sprite. Either a TGA or BMP file is required.
	*	@param pszFileName Name of the file.
	*	@return Pointer to the model, or null if the model could not be loaded.
	*/
	pfnEngSrc_LoadMapSprite_t				LoadMapSprite;

	/**
	*	Adds a directory to the PLATFORM search path.
	*	@param pszBaseDir Directory to add.
	*	@param pszAppName Ignored.
	*/
	pfnEngSrc_COM_AddAppDirectoryToSearchPath_t		COM_AddAppDirectoryToSearchPath;

	/**
	*	Converts a relative path to an absolute path.
	*	@param pszFileName Name of the file whose path to make absolute.
	*	@param[ out ] pszNameOutBuffer Destination buffer.
	*	@param nameOutBufferSize Size of the destination buffer, in bytes.
	*/
	pfnEngSrc_COM_ExpandFilename_t			COM_ExpandFilename;

	/**
	*	Given a key, gets the info value.
	*	@param playerNum Player number. 1 based.
	*	@param pszKey Key.
	*	@return Pointer to the value, or an empty string if the key couldn't be found.
	*/
	pfnEngSrc_PlayerInfo_ValueForKey_t		PlayerInfo_ValueForKey;

	/**
	*	Sets the value for a key in the local player's info key buffer.
	*	@param pszKey Key whose value to set.
	*	@param pszValue Value to set.
	*/
	pfnEngSrc_PlayerInfo_SetValueForKey_t	PlayerInfo_SetValueForKey;

	/**
	*	Gets the given player's unique ID.
	*	@param iPlayer 1 based player index.
	*	@param[ out ] playerID Will contain the player's unique ID.
	*	@return true on success, false otherwise.
	*/
	pfnEngSrc_GetPlayerUniqueID_t			GetPlayerUniqueID;

	/**
	*	Obsolete.
	*	@return 0 in all cases.
	*/
	pfnEngSrc_GetTrackerIDForPlayer_t		GetTrackerIDForPlayer;

	/**
	*	Obsolete.
	*	@return 0 in all cases.
	*/
	pfnEngSrc_GetPlayerForTrackerID_t		GetPlayerForTrackerID;

	/**
	*	Sends a command to the server unreliably.
	*	@param pszCmdString Command string.
	*	@return true if the message was written, false otherwise.
	*/
	pfnEngSrc_pfnServerCmdUnreliable_t		pfnServerCmdUnreliable;

	/**
	*	Gets the mouse position.
	*	@param ppt Structure that will contain the mouse position.
	*/
	pfnEngSrc_GetMousePos_t					pfnGetMousePos;

	/**
	*	Sets the mouse position.
	*	@param x X position.
	*	@param y Y position.
	*/
	pfnEngSrc_SetMousePos_t					pfnSetMousePos;

	/**
	*	Obsolete.
	*/
	pfnEngSrc_SetMouseEnable_t				pfnSetMouseEnable;

	/**
	*	@return The first cvar in the list.
	*/
	pfnEngSrc_GetFirstCVarPtr_t				GetFirstCvarPtr;

	/**
	*	@return The first command function handle.
	*/
	pfnEngSrc_GetFirstCmdFunctionHandle_t	GetFirstCmdFunctionHandle;

	/**
	*	Gets the next command function handle.
	*	@param cmdhandle Handle to the command function just before the handle to get.
	*	@return Next handle, or 0 if it was the last handle.
	*/
	pfnEngSrc_GetNextCmdFunctionHandle_t	GetNextCmdFunctionHandle;

	/**
	*	Gets the command function name.
	*	@param cmdhandle Handle to the command.
	*	@return Command name.
	*/
	pfnEngSrc_GetCmdFunctionName_t			GetCmdFunctionName;

	/**
	*	@return The old client time.
	*/
	pfnEngSrc_GetClientOldTime_t			hudGetClientOldTime;

	/**
	*	@return Server gravity value. Only valid if this is a listen server.
	*/
	pfnEngSrc_GetServerGravityValue_t		hudGetServerGravityValue;

	/**
	*	Gets a model by index.
	*	@param index Model index. Must be valid.
	*	@return Model pointer.
	*/
	pfnEngSrc_GetModelByIndex_t				hudGetModelByIndex;

	/**
	*	Sets the filter mode.
	*	Filtering will essentially overlay a single color on top of the entire viewport, with the exception of the HUD.
	*	This works like Counter-Strike's night vision (though that uses a different implementation).
	*	@param bMode Whether to filter or not.
	*/
	pfnEngSrc_pfnSetFilterMode_t			pfnSetFilterMode;

	/**
	*	Sets the filter color.
	*	@param r Red color. [ 0, 1 ].
	*	@param g Green color. [ 0, 1 ].
	*	@param b Blue color. [ 0, 1 ].
	*/
	pfnEngSrc_pfnSetFilterColor_t			pfnSetFilterColor;

	/**
	*	Sets the filter brightness.
	*	@param brightness Brightness.
	*/
	pfnEngSrc_pfnSetFilterBrightness_t		pfnSetFilterBrightness;

	/**
	*	Gets the sequence that has the given entry name.
	*	@param pszFileName Ignored.
	*	@param pszEntryName Entry name.
	*	@return Sequence, or null if no such sequence exists.
	*/
	pfnEngSrc_pfnSequenceGet_t				pfnSequenceGet;

	/**
	*	Draws the current sprite as solid.
	*	@param frame Frame to draw.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param prc Optional. Defines the rectangle of the sprite frame to draw.
	*	@param src glBlendFunc source value.
	*	@param dest glBlendFunc destination value.
	*	@param w Overrides the sprite frame's width.
	*	@param h Overrides the sprite frame's height.
	*	@see pfnSPR_Set
	*/
	pfnEngSrc_pfnSPR_DrawGeneric_t			pfnSPR_DrawGeneric;

	/**
	*	Picks a sentence from the given group.
	*	@param pszGroupName Group from which to select a sentence.
	*	@param pickMethod Ignored.
	*	@param piPicked If not null, this is set to the index of the sentence that was picked.
	*	@return Sentence that was picked, or null if there is no group by that name, or no sentences in the group.
	*/
	pfnEngSrc_pfnSequencePickSentence_t		pfnSequencePickSentence;

	/**
	*	Draws a complete string.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param pszString String to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@return Total width of the string.
	*/
	pfnEngSrc_pfnDrawString_t				pfnDrawString;

	/**
	*	Draws a complete string in reverse.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param pszString String to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@return Total width of the string.
	*/
	pfnEngSrc_pfnDrawStringReverse_t				pfnDrawStringReverse;

	/**
	*	Given a key, gets the info value for the local player.
	*	@param pszKey Key.
	*	@return Pointer to the value, or an empty string if the key couldn't be found.
	*/
	pfnEngSrc_LocalPlayerInfo_ValueForKey_t		LocalPlayerInfo_ValueForKey;

	/**
	*	Draws a single character using the specified font.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param ch Character to draw.
	*	@param font Font to use.
	*	@return Total width of the character.
	*/
	pfnEngSrc_pfnVGUI2DrawCharacter_t		pfnVGUI2DrawCharacter;

	/**
	*	Draws a single character using the specified font, using additive rendering.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param ch Character to draw.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@param font Font to use.
	*	@return Total width of the character.
	*/
	pfnEngSrc_pfnVGUI2DrawCharacterAdd_t	pfnVGUI2DrawCharacterAdd;

	/**
	*	Gets the approximate wave play length of the given file.
	*	@param pszFileName Name of the file to query.
	*	@return Approximate wave play length.
	*/
	pfnEngSrc_COM_GetApproxWavePlayLength	COM_GetApproxWavePlayLength;

	/**
	*	Gets the career UI, if it exists.
	*	@return Career UI. Cast to ICareerUI*. Can be null.
	*/
	pfnEngSrc_pfnGetCareerUI_t				pfnGetCareerUI;

	/**
	*	Sets a cvar's string value.
	*	@param pszCVarName CVar name.
	*	@param pszValue Value to set.
	*/
	pfnEngSrc_Cvar_Set_t					Cvar_Set;

	/**
	*	@return Whether this is a Condition Zero career match.
	*/
	pfnEngSrc_pfnIsPlayingCareerMatch_t		pfnIsCareerMatch;

	/**
	*	Plays a sound by name, with pitch. Uses the bot channel CHAN_BOT.
	*	@param pszSoundName Name of the sound to play.
	*	@param volume Volume. [ 0, 1 ].
	*	@param pitch Pitch. [ 0, 255 ].
	*/
	pfnEngSrc_pfnPlaySoundVoiceByName_t	pfnPlaySoundVoiceByName;

	/**
	*	Sets a music track to play.
	*	@param pszFileName Name of the file to play.
	*	@param bLooping Whether the track should look or not.
	*/
	pfnEngSrc_pfnPrimeMusicStream_t		pfnPrimeMusicStream;

	/**
	*	@return The absolute time since the last call to GetAbsoluteTime.
	*/
	pfnEngSrc_GetAbsoluteTime_t				GetAbsoluteTime;

	/**
	*	Processes the tutor message decay buffer.
	*	@param pBuffer Buffer.
	*	@param bufferLength Size of the buffer, in bytes.
	*/
	pfnEngSrc_pfnProcessTutorMessageDecayBuffer_t		pfnProcessTutorMessageDecayBuffer;

	/**
	*	Constructs the tutor message decay buffer.
	*	@param pBuffer Buffer.
	*	@param bufferLength Size of the buffer, in bytes.
	*/
	pfnEngSrc_pfnConstructTutorMessageDecayBuffer_t		pfnConstructTutorMessageDecayBuffer;

	/**
	*	Resets tutor message decay data.
	*/
	pfnEngSrc_pfnResetTutorMessageDecayData_t		pfnResetTutorMessageDecayData;

	/**
	*	Plays a sound by name, with pitch.
	*	@param pszSoundName Name of the sound to play.
	*	@param volume Volume. [ 0, 1 ].
	*	@param pitch Pitch. [ 0, 255 ].
	*/
	pfnEngSrc_pfnPlaySoundByNameAtPitch_t	pfnPlaySoundByNameAtPitch;

	/**
	*	Fills the given rectangle with a given color.
	*	Blends with existing pixel data.
	*	@param x Left coordinate.
	*	@param y Top coordinate.
	*	@param width Width of the rectangle.
	*	@param height Height of the rectangle.
	*	@param r Red color. [ 0, 255 ].
	*	@param g Green color. [ 0, 255 ].
	*	@param b Blue color. [ 0, 255 ].
	*	@param a Alpha value. [ 0, 255 ].
	*/
	pfnEngSrc_pfnFillRGBABlend_t					pfnFillRGBABlend;

	/**
	*	@return The app ID.
	*/
	pfnEngSrc_pfnGetAppID_t					pfnGetAppID;

	/**
	*	@return The list of command aliases.
	*/
	pfnEngSrc_pfnGetAliases_t				pfnGetAliasList;

	/**
	*	Gets the accumulated mouse delta. The delta is reset in this call only.
	*	@param x X offset.
	*	@param y Y offset.
	*/
	pfnEngSrc_pfnVguiWrap2_GetMouseDelta_t pfnVguiWrap2_GetMouseDelta;
	pfnEngSrc_pfnFilteredClientCmd_t		pfnFilteredClientCmd;
} cl_enginefunc_t;

// Function type declarations for engine destination functions
typedef void	(*pfnEngDst_pfnSPR_Load_t )				( const char ** );
typedef void	(*pfnEngDst_pfnSPR_Frames_t )			( HSPRITE * );
typedef void	(*pfnEngDst_pfnSPR_Height_t )			( HSPRITE *, int * );
typedef void	(*pfnEngDst_pfnSPR_Width_t )			( HSPRITE *, int * );
typedef void	(*pfnEngDst_pfnSPR_Set_t )				( HSPRITE *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnSPR_Draw_t )				( int *, int *, int *, const struct rect_s ** );
typedef void	(*pfnEngDst_pfnSPR_DrawHoles_t )		( int *, int *, int *, const struct rect_s ** );
typedef void	(*pfnEngDst_pfnSPR_DrawAdditive_t )		( int *, int *, int *, const struct rect_s ** );
typedef void	(*pfnEngDst_pfnSPR_EnableScissor_t )	( int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnSPR_DisableScissor_t )	( void );
typedef void	(*pfnEngDst_pfnSPR_GetList_t )			( char **, int ** );
typedef void	(*pfnEngDst_pfnFillRGBA_t )				( int *, int *, int *, int *, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnGetScreenInfo_t ) 		( struct SCREENINFO_s ** );
typedef void	(*pfnEngDst_pfnSetCrosshair_t )			( HSPRITE *, struct rect_s *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnRegisterVariable_t )		( char **, char **, int * );
typedef void	(*pfnEngDst_pfnGetCvarFloat_t )			( char ** );
typedef void	(*pfnEngDst_pfnGetCvarString_t )		( char ** );
typedef void	(*pfnEngDst_pfnAddCommand_t )			( char **, void (**pfnEngDst_function)(void) );
typedef void	(*pfnEngDst_pfnHookUserMsg_t )			( char **, pfnUserMsgHook * );
typedef void	(*pfnEngDst_pfnServerCmd_t )			( char ** );
typedef void	(*pfnEngDst_pfnClientCmd_t )			( char ** );
typedef void	(*pfnEngDst_pfnPrimeMusicStream_t )	( char **, int *);
typedef void	(*pfnEngDst_pfnGetPlayerInfo_t )		( int *, struct hud_player_info_s ** );
typedef void	(*pfnEngDst_pfnPlaySoundByName_t )		( char **, float * );
typedef void	(*pfnEngDst_pfnPlaySoundByNameAtPitch_t )	( char **, float *, int * );
typedef void	(*pfnEngDst_pfnPlaySoundVoiceByName_t )	(char **, float * );
typedef void	(*pfnEngDst_pfnPlaySoundByIndex_t )		( int *, float * );
typedef void	(*pfnEngDst_pfnAngleVectors_t )			( const float * *, float * *, float * *, float * * );
typedef void	(*pfnEngDst_pfnTextMessageGet_t )		( const char ** );
typedef void	(*pfnEngDst_pfnDrawCharacter_t )		( int *, int *, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnDrawConsoleString_t )	( int *, int *, char ** );
typedef void	(*pfnEngDst_pfnDrawSetTextColor_t )		( float *, float *, float * );
typedef void	(*pfnEngDst_pfnDrawConsoleStringLen_t )	(  const char **, int **, int ** );
typedef void	(*pfnEngDst_pfnConsolePrint_t )			( const char ** );
typedef void	(*pfnEngDst_pfnCenterPrint_t )			( const char ** );
typedef void	(*pfnEngDst_GetWindowCenterX_t )		( void );
typedef void	(*pfnEngDst_GetWindowCenterY_t )		( void );
typedef void	(*pfnEngDst_GetViewAngles_t )			( float ** );
typedef void	(*pfnEngDst_SetViewAngles_t )			( float ** );
typedef void	(*pfnEngDst_GetMaxClients_t )			( void );
typedef void	(*pfnEngDst_Cvar_SetValue_t )			( char **, float * );
typedef void    (*pfnEngDst_Cmd_Argc_t)					(void);	
typedef void	(*pfnEngDst_Cmd_Argv_t )				( int * );
typedef void	(*pfnEngDst_Con_Printf_t )				( char **);
typedef void	(*pfnEngDst_Con_DPrintf_t )				( char **);
typedef void	(*pfnEngDst_Con_NPrintf_t )				( int *, char ** );
typedef void	(*pfnEngDst_Con_NXPrintf_t )			( struct con_nprint_s **, char **);
typedef void	(*pfnEngDst_PhysInfo_ValueForKey_t )	( const char ** );
typedef void	(*pfnEngDst_ServerInfo_ValueForKey_t )	( const char ** );
typedef void	(*pfnEngDst_GetClientMaxspeed_t )		( void );
typedef void	(*pfnEngDst_CheckParm_t )				( char **, char *** );
typedef void	(*pfnEngDst_Key_Event_t )				( int *, int * );
typedef void	(*pfnEngDst_GetMousePosition_t )		( int **, int ** );
typedef void	(*pfnEngDst_IsNoClipping_t )			( void );
typedef void	(*pfnEngDst_GetLocalPlayer_t )			( void );
typedef void	(*pfnEngDst_GetViewModel_t )			( void );
typedef void	(*pfnEngDst_GetEntityByIndex_t )		( int * );
typedef void	(*pfnEngDst_GetClientTime_t )			( void );
typedef void	(*pfnEngDst_V_CalcShake_t )				( void );
typedef void	(*pfnEngDst_V_ApplyShake_t )			( float **, float **, float * );
typedef void	(*pfnEngDst_PM_PointContents_t )		( float **, int ** );
typedef void	(*pfnEngDst_PM_WaterEntity_t )			( float ** );
typedef void	(*pfnEngDst_PM_TraceLine_t )			( float **, float **, int *, int *, int * );
typedef void	(*pfnEngDst_CL_LoadModel_t )			( const char **, int ** );
typedef void	(*pfnEngDst_CL_CreateVisibleEntity_t )	( int *, struct cl_entity_s ** );
typedef void	(*pfnEngDst_GetSpritePointer_t )		( HSPRITE * );
typedef void	(*pfnEngDst_pfnPlaySoundByNameAtLocation_t )	( char **, float *, float ** );
typedef void	(*pfnEngDst_pfnPrecacheEvent_t )		( int *, const char* * );
typedef void	(*pfnEngDst_pfnPlaybackEvent_t )		( int *, const struct edict_s **, unsigned short *, float *, float **, float **, float *, float *, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnWeaponAnim_t )			( int *, int * );
typedef void	(*pfnEngDst_pfnRandomFloat_t )			( float *, float * );
typedef void	(*pfnEngDst_pfnRandomLong_t )			( int32 *, int32 * );
typedef void	(*pfnEngDst_pfnHookEvent_t )			( char **, void ( **pfnEvent )( struct event_args_s *args ) );
typedef void	(*pfnEngDst_Con_IsVisible_t)			();
typedef void	(*pfnEngDst_pfnGetGameDirectory_t )		( void );
typedef void	(*pfnEngDst_pfnGetCvarPointer_t )		( const char ** );
typedef void	(*pfnEngDst_Key_LookupBinding_t )		( const char ** );
typedef void	(*pfnEngDst_pfnGetLevelName_t )			( void );
typedef void	(*pfnEngDst_pfnGetScreenFade_t )		( struct screenfade_s ** );
typedef void	(*pfnEngDst_pfnSetScreenFade_t )		( struct screenfade_s ** );
typedef void	(*pfnEngDst_VGui_GetPanel_t )			( );
typedef void	(*pfnEngDst_VGui_ViewportPaintBackground_t ) (int **);
typedef void	(*pfnEngDst_COM_LoadFile_t )			( char **, int *, int ** );
typedef void	(*pfnEngDst_COM_ParseFile_t )			( char **, char ** );
typedef void	(*pfnEngDst_COM_FreeFile_t)				( void ** );
typedef void	(*pfnEngDst_IsSpectateOnly_t )			( void );
typedef void	(*pfnEngDst_LoadMapSprite_t )			( const char ** );
typedef void	(*pfnEngDst_COM_AddAppDirectoryToSearchPath_t ) ( const char **, const char ** );
typedef void	(*pfnEngDst_COM_ExpandFilename_t)		( const char **, char **, int * );
typedef void	(*pfnEngDst_PlayerInfo_ValueForKey_t )	( int *, const char ** );
typedef void	(*pfnEngDst_PlayerInfo_SetValueForKey_t )( const char **, const char ** );
typedef void	(*pfnEngDst_GetPlayerUniqueID_t)		(int *, char **);
typedef void	(*pfnEngDst_GetTrackerIDForPlayer_t)	(int *);
typedef void	(*pfnEngDst_GetPlayerForTrackerID_t)	(int *);
typedef void	(*pfnEngDst_pfnServerCmdUnreliable_t )	( char ** );
typedef void	(*pfnEngDst_GetMousePos_t )				(struct tagPOINT **);
typedef void	(*pfnEngDst_SetMousePos_t )				(int *, int *);
typedef void	(*pfnEngDst_SetMouseEnable_t )			(qboolean *);
typedef void	(*pfnEngDst_pfnSetFilterMode_t)			( int * );
typedef void	(*pfnEngDst_pfnSetFilterColor_t)		( float *, float *, float * );
typedef void	(*pfnEngDst_pfnSetFilterBrightness_t)	( float * );
typedef void	(*pfnEngDst_pfnSequenceGet_t )			( const char**, const char** );
typedef void	(*pfnEngDst_pfnSPR_DrawGeneric_t )		( int *, int *, int *, const struct rect_s **, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnSequencePickSentence_t )	( const char**, int *, int ** );
typedef void	(*pfnEngDst_pfnDrawString_t )			( int *, int *, const char *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnDrawStringReverse_t )			( int *, int *, const char *, int *, int *, int * );
typedef void	(*pfnEngDst_LocalPlayerInfo_ValueForKey_t )( const char **);
typedef void	(*pfnEngDst_pfnVGUI2DrawCharacter_t )		( int *, int *, int *, unsigned int * );
typedef void	(*pfnEngDst_pfnVGUI2DrawCharacterAdd_t )	( int *, int *, int *, int *, int *, int *, unsigned int *);
typedef void	(*pfnEngDst_pfnProcessTutorMessageDecayBuffer_t )(int **, int *);
typedef void	(*pfnEngDst_pfnConstructTutorMessageDecayBuffer_t )(int **, int *);
typedef void	(*pfnEngDst_pfnResetTutorMessageDecayData_t)();
typedef void	(*pfnEngDst_pfnFillRGBABlend_t )				( int *, int *, int *, int *, int *, int *, int *, int * );
typedef void	(*pfnEngDst_pfnGetAppID_t )				( void );
typedef void	(*pfnEngDst_pfnGetAliases_t )				( void );
typedef void	(*pfnEngDst_pfnVguiWrap2_GetMouseDelta_t) ( int *x, int *y );
typedef void	(*pfnEngDst_pfnFilteredClientCmd_t )	( char ** );


// Pointers to the engine destination functions
typedef struct
{
	pfnEngDst_pfnSPR_Load_t					pfnSPR_Load;
	pfnEngDst_pfnSPR_Frames_t				pfnSPR_Frames;
	pfnEngDst_pfnSPR_Height_t				pfnSPR_Height;
	pfnEngDst_pfnSPR_Width_t				pfnSPR_Width;
	pfnEngDst_pfnSPR_Set_t					pfnSPR_Set;
	pfnEngDst_pfnSPR_Draw_t					pfnSPR_Draw;
	pfnEngDst_pfnSPR_DrawHoles_t			pfnSPR_DrawHoles;
	pfnEngDst_pfnSPR_DrawAdditive_t			pfnSPR_DrawAdditive;
	pfnEngDst_pfnSPR_EnableScissor_t		pfnSPR_EnableScissor;
	pfnEngDst_pfnSPR_DisableScissor_t		pfnSPR_DisableScissor;
	pfnEngDst_pfnSPR_GetList_t				pfnSPR_GetList;
	pfnEngDst_pfnFillRGBA_t					pfnFillRGBA;
	pfnEngDst_pfnGetScreenInfo_t			pfnGetScreenInfo;
	pfnEngDst_pfnSetCrosshair_t				pfnSetCrosshair;
	pfnEngDst_pfnRegisterVariable_t			pfnRegisterVariable;
	pfnEngDst_pfnGetCvarFloat_t				pfnGetCvarFloat;
	pfnEngDst_pfnGetCvarString_t			pfnGetCvarString;
	pfnEngDst_pfnAddCommand_t				pfnAddCommand;
	pfnEngDst_pfnHookUserMsg_t				pfnHookUserMsg;
	pfnEngDst_pfnServerCmd_t				pfnServerCmd;
	pfnEngDst_pfnClientCmd_t				pfnClientCmd;
	pfnEngDst_pfnGetPlayerInfo_t			pfnGetPlayerInfo;
	pfnEngDst_pfnPlaySoundByName_t			pfnPlaySoundByName;
	pfnEngDst_pfnPlaySoundByIndex_t			pfnPlaySoundByIndex;
	pfnEngDst_pfnAngleVectors_t				pfnAngleVectors;
	pfnEngDst_pfnTextMessageGet_t			pfnTextMessageGet;
	pfnEngDst_pfnDrawCharacter_t			pfnDrawCharacter;
	pfnEngDst_pfnDrawConsoleString_t		pfnDrawConsoleString;
	pfnEngDst_pfnDrawSetTextColor_t			pfnDrawSetTextColor;
	pfnEngDst_pfnDrawConsoleStringLen_t		pfnDrawConsoleStringLen;
	pfnEngDst_pfnConsolePrint_t				pfnConsolePrint;
	pfnEngDst_pfnCenterPrint_t				pfnCenterPrint;
	pfnEngDst_GetWindowCenterX_t			GetWindowCenterX;
	pfnEngDst_GetWindowCenterY_t			GetWindowCenterY;
	pfnEngDst_GetViewAngles_t				GetViewAngles;
	pfnEngDst_SetViewAngles_t				SetViewAngles;
	pfnEngDst_GetMaxClients_t				GetMaxClients;
	pfnEngDst_Cvar_SetValue_t				Cvar_SetValue;
	pfnEngDst_Cmd_Argc_t					Cmd_Argc;
	pfnEngDst_Cmd_Argv_t					Cmd_Argv;
	pfnEngDst_Con_Printf_t					Con_Printf;
	pfnEngDst_Con_DPrintf_t					Con_DPrintf;
	pfnEngDst_Con_NPrintf_t					Con_NPrintf;
	pfnEngDst_Con_NXPrintf_t				Con_NXPrintf;
	pfnEngDst_PhysInfo_ValueForKey_t		PhysInfo_ValueForKey;
	pfnEngDst_ServerInfo_ValueForKey_t		ServerInfo_ValueForKey;
	pfnEngDst_GetClientMaxspeed_t			GetClientMaxspeed;
	pfnEngDst_CheckParm_t					CheckParm;
	pfnEngDst_Key_Event_t					Key_Event;
	pfnEngDst_GetMousePosition_t			GetMousePosition;
	pfnEngDst_IsNoClipping_t				IsNoClipping;
	pfnEngDst_GetLocalPlayer_t				GetLocalPlayer;
	pfnEngDst_GetViewModel_t				GetViewModel;
	pfnEngDst_GetEntityByIndex_t			GetEntityByIndex;
	pfnEngDst_GetClientTime_t				GetClientTime;
	pfnEngDst_V_CalcShake_t					V_CalcShake;
	pfnEngDst_V_ApplyShake_t				V_ApplyShake;
	pfnEngDst_PM_PointContents_t			PM_PointContents;
	pfnEngDst_PM_WaterEntity_t				PM_WaterEntity;
	pfnEngDst_PM_TraceLine_t				PM_TraceLine;
	pfnEngDst_CL_LoadModel_t				CL_LoadModel;
	pfnEngDst_CL_CreateVisibleEntity_t		CL_CreateVisibleEntity;
	pfnEngDst_GetSpritePointer_t			GetSpritePointer;
	pfnEngDst_pfnPlaySoundByNameAtLocation_t	pfnPlaySoundByNameAtLocation;
	pfnEngDst_pfnPrecacheEvent_t			pfnPrecacheEvent;
	pfnEngDst_pfnPlaybackEvent_t			pfnPlaybackEvent;
	pfnEngDst_pfnWeaponAnim_t				pfnWeaponAnim;
	pfnEngDst_pfnRandomFloat_t				pfnRandomFloat;
	pfnEngDst_pfnRandomLong_t				pfnRandomLong;
	pfnEngDst_pfnHookEvent_t				pfnHookEvent;
	pfnEngDst_Con_IsVisible_t				Con_IsVisible;
	pfnEngDst_pfnGetGameDirectory_t			pfnGetGameDirectory;
	pfnEngDst_pfnGetCvarPointer_t			pfnGetCvarPointer;
	pfnEngDst_Key_LookupBinding_t			Key_LookupBinding;
	pfnEngDst_pfnGetLevelName_t				pfnGetLevelName;
	pfnEngDst_pfnGetScreenFade_t			pfnGetScreenFade;
	pfnEngDst_pfnSetScreenFade_t			pfnSetScreenFade;
	pfnEngDst_VGui_GetPanel_t				VGui_GetPanel;
	pfnEngDst_VGui_ViewportPaintBackground_t	VGui_ViewportPaintBackground;
	pfnEngDst_COM_LoadFile_t				COM_LoadFile;
	pfnEngDst_COM_ParseFile_t				COM_ParseFile;
	pfnEngDst_COM_FreeFile_t				COM_FreeFile;
	struct triangleapi_s		*pTriAPI;
	struct efx_api_s			*pEfxAPI;
	struct event_api_s			*pEventAPI;
	struct demo_api_s			*pDemoAPI;
	struct net_api_s			*pNetAPI;
	struct IVoiceTweak_s		*pVoiceTweak;
	pfnEngDst_IsSpectateOnly_t				IsSpectateOnly;
	pfnEngDst_LoadMapSprite_t				LoadMapSprite;
	pfnEngDst_COM_AddAppDirectoryToSearchPath_t		COM_AddAppDirectoryToSearchPath;
	pfnEngDst_COM_ExpandFilename_t			COM_ExpandFilename;
	pfnEngDst_PlayerInfo_ValueForKey_t		PlayerInfo_ValueForKey;
	pfnEngDst_PlayerInfo_SetValueForKey_t	PlayerInfo_SetValueForKey;
	pfnEngDst_GetPlayerUniqueID_t			GetPlayerUniqueID;
	pfnEngDst_GetTrackerIDForPlayer_t		GetTrackerIDForPlayer;
	pfnEngDst_GetPlayerForTrackerID_t		GetPlayerForTrackerID;
	pfnEngDst_pfnServerCmdUnreliable_t		pfnServerCmdUnreliable;
	pfnEngDst_GetMousePos_t					pfnGetMousePos;
	pfnEngDst_SetMousePos_t					pfnSetMousePos;
	pfnEngDst_SetMouseEnable_t				pfnSetMouseEnable;
	pfnEngDst_pfnSetFilterMode_t			pfnSetFilterMode ;
	pfnEngDst_pfnSetFilterColor_t			pfnSetFilterColor ;
	pfnEngDst_pfnSetFilterBrightness_t		pfnSetFilterBrightness ;
	pfnEngDst_pfnSequenceGet_t				pfnSequenceGet;
	pfnEngDst_pfnSPR_DrawGeneric_t			pfnSPR_DrawGeneric;
	pfnEngDst_pfnSequencePickSentence_t		pfnSequencePickSentence;
	pfnEngDst_pfnDrawString_t				pfnDrawString;
	pfnEngDst_pfnDrawString_t				pfnDrawStringReverse;
	pfnEngDst_LocalPlayerInfo_ValueForKey_t	LocalPlayerInfo_ValueForKey;
	pfnEngDst_pfnVGUI2DrawCharacter_t		pfnVGUI2DrawCharacter;
	pfnEngDst_pfnVGUI2DrawCharacterAdd_t	pfnVGUI2DrawCharacterAdd;
	pfnEngDst_pfnPlaySoundVoiceByName_t	pfnPlaySoundVoiceByName;
	pfnEngDst_pfnPrimeMusicStream_t			pfnPrimeMusicStream;
	pfnEngDst_pfnProcessTutorMessageDecayBuffer_t		pfnProcessTutorMessageDecayBuffer;
	pfnEngDst_pfnConstructTutorMessageDecayBuffer_t		pfnConstructTutorMessageDecayBuffer;
	pfnEngDst_pfnResetTutorMessageDecayData_t		pfnResetTutorMessageDecayData;
	pfnEngDst_pfnPlaySoundByNameAtPitch_t	pfnPlaySoundByNameAtPitch;
	pfnEngDst_pfnFillRGBABlend_t					pfnFillRGBABlend;
	pfnEngDst_pfnGetAppID_t							pfnGetAppID;
	pfnEngDst_pfnGetAliases_t				pfnGetAliasList;
	pfnEngDst_pfnVguiWrap2_GetMouseDelta_t	pfnVguiWrap2_GetMouseDelta;
	pfnEngDst_pfnFilteredClientCmd_t		pfnFilteredClientCmd;
} cl_enginefunc_dst_t;


// ********************************************************
// Functions exposed by the engine to the module
// ********************************************************

// Functions for ModuleS
typedef void (*PFN_KICKPLAYER)(int nPlayerSlot, int nReason);

typedef struct modshelpers_s
{
	PFN_KICKPLAYER m_pfnKickPlayer;

	// reserved for future expansion
	int m_nVoid1;
	int m_nVoid2;
	int m_nVoid3;
	int m_nVoid4;
	int m_nVoid5;
	int m_nVoid6;
	int m_nVoid7;
	int m_nVoid8;
	int m_nVoid9;
} modshelpers_t;

// Functions for moduleC
typedef struct modchelpers_s
{
	// reserved for future expansion
	int m_nVoid0;
	int m_nVoid1;
	int m_nVoid2;
	int m_nVoid3;
	int m_nVoid4;
	int m_nVoid5;
	int m_nVoid6;
	int m_nVoid7;
	int m_nVoid8;
	int m_nVoid9;
} modchelpers_t;


// ********************************************************
// Information about the engine
// ********************************************************
typedef struct engdata_s
{
	cl_enginefunc_t	*pcl_enginefuncs;		// functions exported by the engine
	cl_enginefunc_dst_t *pg_engdstAddrs;	// destination handlers for engine exports
	cldll_func_t *pcl_funcs;				// client exports
	cldll_func_dst_t *pg_cldstAddrs;		// client export destination handlers
	struct modfuncs_s *pg_modfuncs;			// engine's pointer to module functions
	struct cmd_function_s **pcmd_functions;	// list of all registered commands
	void *pkeybindings;						// all key bindings (not really a void *, but easier this way)
	void (*pfnConPrintf)(char *, ...);		// dump to console
	struct cvar_s **pcvar_vars;				// pointer to head of cvar list
	struct glwstate_t *pglwstate;			// OpenGl information
	void *(*pfnSZ_GetSpace)(struct sizebuf_s *, int); // pointer to SZ_GetSpace
	struct modfuncs_s *pmodfuncs;			// &g_modfuncs
	void *pfnGetProcAddress;				// &GetProcAddress
	void *pfnGetModuleHandle;				// &GetModuleHandle
	struct server_static_s *psvs;			// &svs
	struct client_static_s *pcls;			// &cls
	void (*pfnSV_DropClient)(struct client_s *, qboolean, char *, ...);	// pointer to SV_DropClient
	void (*pfnNetchan_Transmit)(struct netchan_s *, int, byte *);		// pointer to Netchan_Transmit
	void (*pfnNET_SendPacket)(enum netsrc_s sock, int length, void *data, netadr_t to); // &NET_SendPacket
	struct cvar_s *(*pfnCvarFindVar)(const char *pchName);				// pointer to Cvar_FindVar
	int *phinstOpenGlEarly;					// &g_hinstOpenGlEarly

	// Reserved for future expansion
	void *pVoid0;							// reserved for future expan
	void *pVoid1;							// reserved for future expan
	void *pVoid2;							// reserved for future expan
	void *pVoid3;							// reserved for future expan
	void *pVoid4;							// reserved for future expan
	void *pVoid5;							// reserved for future expan
	void *pVoid6;							// reserved for future expan
	void *pVoid7;							// reserved for future expan
	void *pVoid8;							// reserved for future expan
	void *pVoid9;							// reserved for future expan
} engdata_t;


// ********************************************************
// Functions exposed by the security module
// ********************************************************
typedef void (*PFN_LOADMOD)(char *pchModule);
typedef void (*PFN_CLOSEMOD)(void);
typedef int (*PFN_NCALL)(int ijump, int cnArg, ...);

typedef void (*PFN_GETCLDSTADDRS)(cldll_func_dst_t *pcldstAddrs);
typedef void (*PFN_GETENGDSTADDRS)(cl_enginefunc_dst_t *pengdstAddrs);
typedef void (*PFN_MODULELOADED)(void);

typedef void (*PFN_PROCESSOUTGOINGNET)(struct netchan_s *pchan, struct sizebuf_s *psizebuf);
typedef qboolean (*PFN_PROCESSINCOMINGNET)(struct netchan_s *pchan, struct sizebuf_s *psizebuf);

typedef void (*PFN_TEXTURELOAD)(char *pszName, int dxWidth, int dyHeight, char *pbData);
typedef void (*PFN_MODELLOAD)(struct model_s *pmodel, void *pvBuf);

typedef void (*PFN_FRAMEBEGIN)(void);
typedef void (*PFN_FRAMERENDER1)(void);
typedef void (*PFN_FRAMERENDER2)(void);

typedef void (*PFN_SETMODSHELPERS)(modshelpers_t *pmodshelpers);
typedef void (*PFN_SETMODCHELPERS)(modchelpers_t *pmodchelpers);
typedef void (*PFN_SETENGDATA)(engdata_t *pengdata);

typedef void (*PFN_CONNECTCLIENT)(int iPlayer);
typedef void (*PFN_RECORDIP)(unsigned int pnIP);
typedef void (*PFN_PLAYERSTATUS)(unsigned char *pbData, int cbData);

typedef void (*PFN_SETENGINEVERSION)(int nVersion);

// typedef class CMachine *(*PFN_PCMACHINE)(void);
typedef int (*PFN_PCMACHINE)(void);
typedef void (*PFN_SETIP)(int ijump);
typedef void (*PFN_EXECUTE)(void);

typedef struct modfuncs_s
{
	// Functions for the pcode interpreter
	PFN_LOADMOD m_pfnLoadMod;
	PFN_CLOSEMOD m_pfnCloseMod;
	PFN_NCALL m_pfnNCall;

	// API destination functions
	PFN_GETCLDSTADDRS m_pfnGetClDstAddrs;
	PFN_GETENGDSTADDRS m_pfnGetEngDstAddrs;

	// Miscellaneous functions
	PFN_MODULELOADED m_pfnModuleLoaded;     // Called right after the module is loaded

	// Functions for processing network traffic
	PFN_PROCESSOUTGOINGNET m_pfnProcessOutgoingNet;   // Every outgoing packet gets run through this
	PFN_PROCESSINCOMINGNET m_pfnProcessIncomingNet;   // Every incoming packet gets run through this

	// Resource functions
	PFN_TEXTURELOAD m_pfnTextureLoad;     // Called as each texture is loaded
	PFN_MODELLOAD m_pfnModelLoad;         // Called as each model is loaded

	// Functions called every frame
	PFN_FRAMEBEGIN m_pfnFrameBegin;       // Called at the beginning of each frame cycle
	PFN_FRAMERENDER1 m_pfnFrameRender1;   // Called at the beginning of the render loop
	PFN_FRAMERENDER2 m_pfnFrameRender2;   // Called at the end of the render loop

	// Module helper transfer
	PFN_SETMODSHELPERS m_pfnSetModSHelpers;
	PFN_SETMODCHELPERS m_pfnSetModCHelpers;
	PFN_SETENGDATA m_pfnSetEngData;

	// Which version of the module is this?
	int m_nVersion;

	// Miscellaneous game stuff
	PFN_CONNECTCLIENT m_pfnConnectClient;	// Called whenever a new client connects
	PFN_RECORDIP m_pfnRecordIP;				// Secure master has reported a new IP for us
	PFN_PLAYERSTATUS m_pfnPlayerStatus;		// Called whenever we receive a PlayerStatus packet

	// Recent additions
	PFN_SETENGINEVERSION m_pfnSetEngineVersion;	// 1 = patched engine

	// reserved for future expansion
	int m_nVoid2;
	int m_nVoid3;
	int m_nVoid4;
	int m_nVoid5;
	int m_nVoid6;
	int m_nVoid7;
	int m_nVoid8;
	int m_nVoid9;
} modfuncs_t;


#define k_nEngineVersion15Base		0
#define k_nEngineVersion15Patch		1
#define k_nEngineVersion16Base		2
#define k_nEngineVersion16Validated	3		// 1.6 engine with built-in validation


typedef struct validator_s
{
	int m_nRandomizer;			// Random number to be XOR'd into all subsequent fields
	int m_nSignature1;			// First signature that identifies this structure
	int m_nSignature2;			// Second signature
	int m_pbCode;				// Beginning of the code block
	int m_cbCode;				// Size of the code block
	int m_nChecksum;			// Checksum of the code block
	int m_nSpecial;				// For engine, 1 if hw.dll, 0 if sw.dll.  For client, pclfuncs checksum
	int m_nCompensator;			// Keeps the checksum correct
} validator_t;


#define k_nChecksumCompensator 0x36a8f09c	// Don't change this value: it's hardcorded in cdll_int.cpp, 

#define k_nModuleVersionCur 0x43210004


#endif // __APIPROXY__
