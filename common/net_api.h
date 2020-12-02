//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined( NET_APIH )
#define NET_APIH
#ifdef _WIN32
#pragma once
#endif

#if !defined ( NETADRH )
#include "netadr.h"
#endif

#define NETAPI_REQUEST_SERVERLIST	( 0 )  // Doesn't need a remote address
#define NETAPI_REQUEST_PING			( 1 )
#define NETAPI_REQUEST_RULES		( 2 )
#define NETAPI_REQUEST_PLAYERS		( 3 )
#define NETAPI_REQUEST_DETAILS		( 4 )

// Set this flag for things like broadcast requests, etc. where the engine should not
//  kill the request hook after receiving the first response
#define FNETAPI_MULTIPLE_RESPONSE ( 1<<0 )

typedef void ( *net_api_response_func_t ) ( struct net_response_s *response );

#define NET_SUCCESS						( 0 )
#define NET_ERROR_TIMEOUT				( 1<<0 )
#define NET_ERROR_PROTO_UNSUPPORTED		( 1<<1 )
#define NET_ERROR_UNDEFINED				( 1<<2 )

typedef struct net_adrlist_s
{
	struct net_adrlist_s	*next;
	netadr_t				remote_address;
} net_adrlist_t;

typedef struct net_response_s
{
	// NET_SUCCESS or an error code
	int			error;

	// Context ID
	int			context;
	// Type
	int			type;

	// Server that is responding to the request
	netadr_t	remote_address;

	// Response RTT ping time
	double		ping;
	// Key/Value pair string ( separated by backlash \ characters )
	// WARNING:  You must copy this buffer in the callback function, because it is freed
	//  by the engine right after the call!!!!
	// ALSO:  For NETAPI_REQUEST_SERVERLIST requests, this will be a pointer to a linked list of net_adrlist_t's
	void		*response;
} net_response_t;

typedef struct net_status_s
{
		// Connected to remote server?  1 == yes, 0 otherwise
	int			connected; 
	// Client's IP address
	netadr_t	local_address;
	// Address of remote server
	netadr_t	remote_address;
	// Packet Loss ( as a percentage )
	int			packet_loss;
	// Latency, in seconds ( multiply by 1000.0 to get milliseconds )
	double		latency;
	// Connection time, in seconds
	double		connection_time;
	// Rate setting ( for incoming data )
	double		rate;
} net_status_t;

typedef struct net_api_s
{
	// APIs
	/**
	*	Initialize networking.
	*/
	void		( *InitNetworking )( void );

	/**
	*	Query the network's status.
	*	@param[ out ] status Status.
	*/
	void		( *Status ) ( struct net_status_s *status );

	/**
	*	Sends a request.
	*	@param context User defined context ID.
	*	@param request Request type. @see NetRequest
	*	@param flags Flags. @see NetApiFlag
	*	@param timeout When to time out the request.
	*	@param remote_address Address to send the request to.
	*	@param response Callback to invoke when the response has been received.
	*/
	void		( *SendRequest) ( int context, int request, int flags, double timeout, struct netadr_s *remote_address, net_api_response_func_t response );
	
	/**
	*	Cancels the request with the given context ID.
	*	@param context Context ID.
	*/
	void		( *CancelRequest ) ( int context );
	
	/**
	*	Cancels all requests.
	*/
	void		( *CancelAllRequests ) ( void );
	
	/**
	*	Converts an address to a string.
	*	@param a Address.
	*	@return Pointer to a static buffer containing the string representation of the address. Can be an empty string if the address is invalid.
	*/
	char		*( *AdrToString ) ( struct netadr_s *a );
	
	/**
	*	Compares 2 addresses.
	*	@param a First address.
	*	@param b Second address.
	*	@return true if the addresses match, false otherwise.
	*/
	int			( *CompareAdr ) ( struct netadr_s *a, struct netadr_s *b );
	
	/**
	*	Converts a string to an address.
	*	@param pszString String to convert.
	*	@param[ out ] a Address.
	*	@return true on success, false otherwise.
	*/
	int			( *StringToAdr ) ( char *s, struct netadr_s *a );
	
	/**
	*	Finds the value associated with the given key in the given info key buffer.
	*	@param pszBuffer Info key buffer.
	*	@param pszKey Key to search for.
	*	@return Pointer to a static buffer containing the value, or an empty string if it hasn't been found.
	*/
	const char *( *ValueForKey ) ( const char *s, const char *key );
	
	/**
	*	Removes the given key from the given buffer.
	*	@param pszBuffer Info key buffer.
	*	@param pszKey Key to remove.
	*/
	void		( *RemoveKey ) ( char *s, const char *key );
	
	/**
	*	Sets the value for the given key in the given buffer.
	*	@param pszBuffer Info key buffer.
	*	@param pszKey Key whose value to set.
	*	@param pszValue Value to set.
	*	@param iMaxSize Maximum size for the info key buffer.
	*/
	void		( *SetValueForKey ) (char *s, const char *key, const char *value, int maxsize );
} net_api_t;

extern net_api_t netapi;

#endif // NET_APIH