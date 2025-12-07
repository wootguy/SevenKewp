//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: New version of the slider bar
//
// $NoKeywords: $
//=============================================================================

//=========================================================
// playermonster - for scripted sequence use.
//=========================================================
#pragma once
#include	"extdll.h"
#include	"CBaseMonster.h"

// For holograms, make them not solid so the player can walk through them
#define	SF_MONSTERPLAYER_NOTSOLID					4 

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

class EXPORT CPlayerMonster : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int ISoundMask ( void );
};
