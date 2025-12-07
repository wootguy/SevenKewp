//=========================================================
// Generic Monster - purely for scripted sequence work.
//=========================================================
#pragma once
#include "extdll.h"
#include "CBaseMonster.h"

// For holograms, make them not solid so the player can walk through them
#define	SF_GENERICMONSTER_NOTSOLID					4 

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

class EXPORT CGenericMonster : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int ISoundMask ( void );
};
