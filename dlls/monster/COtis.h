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
// UNDONE: Holster weapon?

#pragma once
#include	"extdll.h"
#include	"CTalkSquadMonster.h"

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
// first flag is otis dying for scripted sequences?
#define		OTIS_AE_DRAW		( 2 )
#define		OTIS_AE_SHOOT		( 3 )
#define		OTIS_AE_HOLSTER	( 4 )

#define	OTIS_BODY_GUNHOLSTERED	0
#define	OTIS_BODY_GUNDRAWN		1
#define OTIS_BODY_GUNGONE			2

namespace OtisBodyGroup
{
enum OtisBodyGroup
{
	Weapons = 1,
	Heads = 2
};
}

namespace OtisWeapon
{
enum OtisWeapon
{
	Random = -1,
	None = 0,
	DesertEagle,
	Donut
};
}

namespace OtisHead
{
enum OtisHead
{
	Random = -1,
	Hair = 0,
	Balding
};
}

class EXPORT COtis : public CTalkSquadMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int  ISoundMask( void );
	void OtisFirePistol( void );
	void AlertSound( void );
	int  Classify ( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	
	void RunTask( Task_t *pTask );
	void StartTask( Task_t *pTask );
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	
	void DeclineFollowing( void );

	// Override these to set behavior
	Schedule_t *GetScheduleOfType ( int Type );
	Schedule_t *GetSchedule ( void );
	MONSTERSTATE GetIdealState ( void );

	void DeathSound( void );
	void PainSound( void );
	
	void TalkInit( void );

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	void Killed( entvars_t *pevAttacker, int iGib );

	void KeyValue( KeyValueData* pkvd ) override;

	const char* GetDeathNoticeWeapon() { return "weapon_357"; }

	void ShuffleSoundArrays();
	void PlaySentence(const char* pszSentence, float duration, float volume, float attenuation);

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	static	TYPEDESCRIPTION m_SaveData[];

	BOOL	m_fGunDrawn;
	float	m_painTime;
	float	m_checkAttackTime;
	BOOL	m_lastAttackCheck;

	//These were originally used to store off the setting AND track state,
	//but state is now tracked by calling GetBodygroup
	int m_iOtisBody;
	int m_iOtisHead;

	// UNDONE: What is this for?  It isn't used?
	float	m_flPlayerDamage;// how much pain has the player inflicted on me?

	CUSTOM_SCHEDULES;

private:
	static const char* pPainSounds[];
	static const char* pDieSounds[];
	static const char* pAnswerSounds[];
	static const char* pQuestionSounds[];
	static const char* pIdleSounds[];
	static const char* pOkSounds[];
	static const char* pWaitSounds[];
	static const char* pScaredSounds[];
	static const char* pHelloSounds[];
	static const char* pSmellSounds[];
	static const char* pWoundSounds[];
	static const char* pMortalSounds[];
	static const char* pMadSounds[];
	static const char* pShotSounds[];
	static const char* pKillSounds[];
};

