/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// rat - environmental monster
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"monsters.h"
#include	"schedule.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================

class CRat : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  Classify ( void );
	const char* DisplayName();
};
LINK_ENTITY_TO_CLASS( monster_rat, CRat )

//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CRat :: Classify ( void )
{
	return	CBaseMonster::Classify(CLASS_INSECT);
}

const char* CRat::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Rat";
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CRat :: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_IDLE:
	default:
		ys = 45;
		break;
	}

	pev->yaw_speed = ys * gSkillData.sk_yawspeed_mult;
}

//=========================================================
// Spawn
//=========================================================
void CRat :: Spawn()
{
	Precache( );

	InitModel();
	SetSize(Vector( 0, 0, 0 ), Vector( 0, 0, 0 ) );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_RED;
	pev->view_ofs		= Vector ( 0, 0, 6 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CRat :: Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/bigrat.mdl";
	PRECACHE_MODEL(GetModel());
}	

//=========================================================
// AI Schedules Specific to this monster
//=========================================================
