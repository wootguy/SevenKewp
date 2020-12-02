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
#if !defined ( EVENT_APIH )
#define EVENT_APIH
#ifdef _WIN32
#pragma once
#endif

#define EVENT_API_VERSION 1

typedef struct event_api_s
{
	/**
	*	Interface version.
	*	Must be EVENT_API_VERSION.
	*	@see EVENT_API_VERSION
	*/
	int		version;

	/**
	*	Plays a sound for the given entity at the given location.
	*	@param ent Entity index.
	*	@param vecOrigin Origin.
	*	@param channel Channel to play on. @see SoundChannel
	*	@param pszSample Sound to play.
	*	@param volume Volume. [ 0, 1 ].
	*	@param attenuation.
	*	@param fFlags Sound flags.
	*	@param pitch Pitch. [ 0, 255 ].
	*/
	void	( *EV_PlaySound ) ( int ent, float *origin, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch );
	
	/**
	*	Stops the sound on the given entity's given channel.
	*	@param ent Entity index.
	*	@param channel Channel to stop. @see SoundChannel.
	*	@param pszSample Name of the sound being played.
	*/
	void	( *EV_StopSound ) ( int ent, int channel, const char *sample );
	
	/**
	*	Finds the index of the given model.
	*	@param pszModelName Name of the model.
	*	@return Model index. 0 if the model isn't loaded.
	*/
	int		( *EV_FindModelIndex )( const char *pmodel );

	/**
	*	Checks if the given player number is the local player.
	*	@param playernum Player number.
	*	@return true if the given player number is the local player, false otherwise.
	*/
	int		( *EV_IsLocal ) ( int playernum );

	/**
	*	@return Whether the local player is ducking.
	*/
	int		( *EV_LocalPlayerDucking ) ( void );

	/**
	*	Gets the view height of the local player.
	*	@param[ out ] vecViewHeight View height.
	*/
	void	( *EV_LocalPlayerViewheight ) ( float * );

	/**
	*	Gets the local player's bounds for the given hull.
	*	@param hull Hull number. @see Hull::Hull
	*	@param[ out ] vecMins Minimum bounds. Can be null.
	*	@param[ out ] vecMaxs Maximum bounds. Can be null.
	*/
	void	( *EV_LocalPlayerBounds ) ( int hull, float *mins, float *maxs );

	/**
	*	Gets the entity index of the entity that was hit by the given trace.
	*	@param pTrace Trace result.
	*	@return Entity index.
	*/
	int		( *EV_IndexFromTrace) ( struct pmtrace_s *pTrace );

	/**
	*	Gets the physics entity at the given position.
	*/
	struct physent_s *( *EV_GetPhysent ) ( int idx );

	/**
	*	Sets up player prediction.
	*	@param bDoPred Whether to do prediction. Seems to do nothing.
	*	@param bIncludeLocalClient Whether to include the local client.
	*/
	void	( *EV_SetUpPlayerPrediction ) ( int dopred, int bIncludeLocalClient );

	/**
	*	Stores off player physics states. Can only push one set of states.
	*/
	void	( *EV_PushPMStates ) ( void );

	/**
	*	Restores stored player physics states.
	*/
	void	( *EV_PopPMStates ) ( void );

	/**
	*	Makes all players solid, except for the given player.
	*	@param playernum Player number. 0 based. If -1, all players are made solid.
	*/
	void	( *EV_SetSolidPlayers ) (int playernum);

	/**
	*	Sets which hull should be used for traces.
	*	@param hull Hull number. @see Hull::Hull
	*/
	void	( *EV_SetTraceHull ) ( int hull );

	/**
	*	Performs a player trace.
	*	@param vecStart Starting point.
	*	@param vecEnd End point.
	*	@param traceFlags Flags. @see PMTraceFlag
	*	@param ignore_pe Index of the entity to ignore. -1 for none.
	*	@param tr Trace result.
	*/
	void	( *EV_PlayerTrace ) ( float *start, float *end, int traceFlags, int ignore_pe, struct pmtrace_s *tr );
	
	/**
	*	Sets the weapon animation and body.
	*	@param sequence Sequence index to set.
	*	@param body Body to set.
	*/
	void	( *EV_WeaponAnimation ) ( int sequence, int body );
	
	/**
	*	Precaches an event.
	*	@param type Type. Must be 1.
	*	@param pszEvent Event name.
	*	@return Event index, or 0 if the event couldn't be found.
	*/
	unsigned short ( *EV_PrecacheEvent ) ( int type, const char* psz );
	
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
	void	( *EV_PlaybackEvent ) ( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
	
	/**
	*	Traces a texture.
	*	@param ground Index of the ground entity to trace.
	*	@param vecStart Starting point.
	*	@param vecEnd End point.
	*	@return Name of the texture, or null if it couldn't find any ground surfaces.
	*/
	const char *( *EV_TraceTexture ) ( int ground, float *vstart, float *vend );
	
	/**
	*	Stops all sounds on the given entity's given channel.
	*	@param entnum Entity index.
	*	@param entchannel Sound channel. @see SoundChannel
	*/
	void	( *EV_StopAllSounds ) ( int entnum, int entchannel );
	
	/**
	*	Kills all events for the given entity and the given event name.
	*	@param entnum Entity number.
	*	@param pszEventName Event name.
	*/
	void    ( *EV_KillEvents ) ( int entnum, const char *eventname );
} event_api_t;

extern event_api_t eventapi;

#endif
