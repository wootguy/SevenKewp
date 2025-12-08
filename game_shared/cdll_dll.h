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
//
//  cdll_dll.h

// this file is included by both the game-dll and the client-dll,

#ifndef CDLL_DLL_H
#define CDLL_DLL_H

#define MAX_WEAPONS		64		// ???

#define MAX_WEAPON_SLOTS		6	// hud item selection slots
#define MAX_ITEM_TYPES			6	// hud item selection slots

#define MAX_ITEMS				5	// hard coded item types

#define	HIDEHUD_WEAPONS					( 1<<0 )
#define	HIDEHUD_FLASHLIGHT				( 1<<1 )
#define	HIDEHUD_ALL						( 1<<2 )
#define HIDEHUD_HEALTH_AND_ARMOR		( 1<<3 ) // keeping this redundant flag for HL players
#define HIDEHUD_HEALTH					( 1<<4 )
#define HIDEHUD_ARMOR					( 1<<5 )

#define	MAX_AMMO_TYPES	32		// ???
#define MAX_AMMO_SLOTS  32		// not really slots

#define HUD_PRINTCONSOLE	0
#define HUD_PRINTCENTER		1
#define HUD_PRINTTALK		2

#ifdef CLIENT_DLL
#define HUD_PRINTNOTIFY		3
#else
#define HUD_PRINTNOTIFY		2
#endif

#define WEAPON_SUIT			31

#endif
