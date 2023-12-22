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
// monster template
//=========================================================
#if 0

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"

class CMyMonster : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	int Classify(void);
	void SetYawSpeed( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );

	BOOL CheckRangeAttack1(float flDot, float flDist) { return FALSE; }
	BOOL CheckRangeAttack2(float flDot, float flDist) { return FALSE; }
	BOOL CheckMeleeAttack1(float flDot, float flDist);
};

LINK_ENTITY_TO_CLASS( my_monster, CMyMonster );

void CMyMonster :: Spawn()
{
	Precache( );

	SET_MODEL(ENT(pev), "models/mymodel.mdl");
	SetSize(Vector( -16, -16, 0 ), Vector( 16, 16, 32 ) );

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->health = 100;
	pev->view_ofs = Vector ( 0, 0, 0 );
	m_flFieldOfView = 0.0;
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_MELEE_ATTACK1;

	MonsterInit();
}

void CMyMonster :: Precache()
{
	CBaseMonster::Precache();

	PRECACHE_SOUND("mysound.wav");

	PRECACHE_MODEL("models/mymodel.mdl");
}

int	CMyMonster::Classify(void)
{
	return CBaseMonster::Classify(CLASS_ALIEN_MONSTER);
}

void CMyMonster::SetYawSpeed(void)
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
	default:
		ys = 90;
	}

	pev->yaw_speed = ys;
}

void CMyMonster::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case 0:
	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

BOOL CMyMonster::CheckMeleeAttack1(float flDot, float flDist) {
	return flDist < 128 ? TRUE : FALSE;
}

#endif
