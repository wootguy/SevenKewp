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
// barnacle - stationary ceiling mounted 'fishing' monster
//=========================================================

#include	"extdll.h"
#include	"util.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"CGib.h"
#include	"CBarnacle.h"
#include	"CBasePlayer.h"

LINK_ENTITY_TO_CLASS( monster_barnacle, CBarnacle )

const char* CBarnacle::pChewSounds[] =
{
	"barnacle/bcl_chew2.wav", // used for gonome effect
	"barnacle/bcl_chew1.wav",
	"barnacle/bcl_chew3.wav",
};

const char* CBarnacle::pDieSounds[] =
{
	"barnacle/bcl_die1.wav",
	"barnacle/bcl_die3.wav",
};

TYPEDESCRIPTION	CBarnacle::m_SaveData[] = 
{
	DEFINE_FIELD( CBarnacle, m_flAltitude, FIELD_FLOAT ),
	DEFINE_FIELD( CBarnacle, m_flKillVictimTime, FIELD_TIME ),
	DEFINE_FIELD( CBarnacle, m_cGibs, FIELD_INTEGER ),// barnacle loads up on gibs each time it kills something.
	DEFINE_FIELD( CBarnacle, m_fTongueExtended, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarnacle, m_fLiftingPrey, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBarnacle, m_flTongueAdj, FIELD_FLOAT ),
};

IMPLEMENT_SAVERESTORE( CBarnacle, CBaseMonster )


//=========================================================
// Classify - indicates this monster's place in the 
// relationship table.
//=========================================================
int	CBarnacle :: Classify ( void )
{
	return	CBaseMonster::Classify(CLASS_BARNACLE);
}

const char* CBarnacle::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Barnacle";
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//
// Returns number of events handled, 0 if none.
//=========================================================
void CBarnacle :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case BARNACLE_AE_PUKEGIB:
		CGib::SpawnMonsterGibs( pev, 1, 1 );
		break;
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CBarnacle :: Spawn()
{
	Precache( );

	InitModel();
	SetSize( Vector(-16, -16, -32), Vector(16, 16, 0) );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_NONE;
	pev->takedamage		= DAMAGE_AIM;
	SetBloodColor(BloodColorHuman());
	pev->effects		= EF_INVLIGHT; // take light from the ceiling 
	SetHealth();
	m_flFieldOfView		= 0.5;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;
	m_flKillVictimTime	= 0;
	m_cGibs				= 0;
	m_fLiftingPrey		= FALSE;
	m_flTongueAdj		= -100;

	InitBoneControllers();

	SetActivity ( ACT_IDLE );

	SetThink ( &CBarnacle::BarnacleThink );
	pev->nextthink = gpGlobals->time + 0.5;

	UTIL_SetOrigin ( pev, pev->origin );

	pev->flags |= FL_MONSTER;
}

int CBarnacle::TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if ( bitsDamageType & DMG_CLUB )
	{
		flDamage = pev->health;
	}

	return CBaseMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
}

//=========================================================
//=========================================================
void CBarnacle :: BarnacleThink ( void )
{
	CBaseEntity *pTouchEnt;
	float flLength = 0;

	UpdateShockEffect();

	pev->nextthink = gpGlobals->time + 0.1;

	if ( m_hEnemy != NULL )
	{
// barnacle has prey.
		CBasePlayer* plr = m_hEnemy->MyPlayerPointer();
		if ( (m_hEnemy->IsMonster() && !m_hEnemy->IsAlive() && !m_hEnemy->IsGrenade()) || (plr && plr->m_noclip))
		{
			// someone (maybe even the barnacle) killed the prey. Reset barnacle.
			m_fLiftingPrey = FALSE;// indicate that we're not lifting prey.
			
			CBaseMonster* pVictim = m_hEnemy->MyMonsterPointer();
			if (pVictim)
				pVictim->BarnacleVictimReleased();

			m_hEnemy = NULL;
			return;
		}

		if ( m_fLiftingPrey )
		{
			if ( m_hEnemy != NULL && m_hEnemy->pev->deadflag != DEAD_NO )
			{
				// crap, someone killed the prey on the way up.
				m_hEnemy = NULL;
				m_fLiftingPrey = FALSE;
				return;
			}

	// still pulling prey.
			Vector vecNewEnemyOrigin = m_hEnemy->pev->origin;
			vecNewEnemyOrigin.x = pev->origin.x;
			vecNewEnemyOrigin.y = pev->origin.y;

			Vector ofs = m_hEnemy->m_barnacleOffset;

			// guess as to where their neck is
			// this offsets you to prevent seeing from the inside of the tongue
			if (m_hEnemy->IsMonster() || ofs != g_vecZero) {
				vecNewEnemyOrigin.x -= (6 + ofs.x) * cos(m_hEnemy->pev->angles.y * M_PI / 180.0);
				vecNewEnemyOrigin.y -= (6 + ofs.x) * sin(m_hEnemy->pev->angles.y * M_PI / 180.0);
			}

			//m_flAltitude -= gSkillData.sk_barnacle_pullspeed;
			//vecNewEnemyOrigin.z += gSkillData.sk_barnacle_pullspeed;

			m_flAltitude = (pev->origin.z - m_hEnemy->EyePosition().z);

			float distLeft = fabs(pev->origin.z - (vecNewEnemyOrigin.z + m_hEnemy->pev->view_ofs.z + ofs.y - 8));

			// interpolate movement
			if (!m_hEnemy->IsPlayer()) {
				m_hEnemy->pev->movetype = MOVETYPE_BOUNCE;
				m_hEnemy->pev->gravity = FLT_MIN;
			}
				
			m_hEnemy->pev->velocity = Vector(0, 0, gSkillData.sk_barnacle_pullspeed * 10);
			if (m_hEnemy->pev->waterlevel >= WATERLEVEL_WAIST) {
				// PM_WaterMove() won't add velocity unless the player is trying to move with a high enough maxspeed
				m_hEnemy->pev->origin.z += gSkillData.sk_barnacle_pullspeed;
			}

			if (distLeft < BARNACLE_BODY_HEIGHT || m_touchedEnemy)
			{
		// prey has just been lifted into position ( if the victim origin + eye height + 8 is higher than the bottom of the barnacle, it is assumed that the head is within barnacle's body )
				m_fLiftingPrey = FALSE;
				m_flAltitude = 0;

				EMIT_SOUND( ENT(pev), CHAN_WEAPON, "barnacle/bcl_bite3.wav", 1, ATTN_NORM );	

				// now that the victim is in place, the killing bite will be administered in 10 seconds.
				m_flKillVictimTime = gpGlobals->time + (m_hEnemy->IsMonster() && !m_hEnemy->IsGrenade() ? 10 : 3);

				m_nextBite = gpGlobals->time + RANDOM_FLOAT(1.5f, 4.0f);
				m_hEnemy->BarnacleVictimBitten( pev );
				SetActivity ( ACT_EAT );
			}

			vecNewEnemyOrigin.z = m_hEnemy->pev->origin.z;
			UTIL_SetOrigin ( m_hEnemy->pev, vecNewEnemyOrigin );
		}
		else
		{
	// prey is lifted fully into feeding position and is dangling there.

			float viewOfs = m_hEnemy->IsPlayer() ? VEC_VIEW.z : m_hEnemy->pev->view_ofs.z;
			Vector ofs = m_hEnemy->m_barnacleOffset;
			Vector biteOri = pev->origin - Vector(0, 0, viewOfs + 32 + ofs.y);

			if (ofs != g_vecZero) {
				biteOri.x -= (6 + ofs.x) * cos(m_hEnemy->pev->angles.y * M_PI / 180.0);
				biteOri.y -= (6 + ofs.x) * sin(m_hEnemy->pev->angles.y * M_PI / 180.0);
			}

			// now center inside of the barnacle
			m_hEnemy->pev->velocity = g_vecZero;
			UTIL_SetOrigin(m_hEnemy->pev, biteOri);

			if ( m_flKillVictimTime != -1 && gpGlobals->time > m_flKillVictimTime )
			{
				// kill!
				if (m_hEnemy->IsMonster() && !m_hEnemy->IsGrenade()) {
					m_hEnemy->TakeDamage(pev, pev, m_hEnemy->pev->health, DMG_SLASH | DMG_ALWAYSGIB);
					m_cGibs = 3;
				}
				else {
					// don't keep catching the same weapon
					UTIL_MakeVectors(Vector(0, RANDOM_FLOAT(-180, 180), 0));
					m_hEnemy->pev->velocity = gpGlobals->v_forward * 50;

					m_hEnemy->BarnacleVictimReleased();
					
					m_hEnemy = NULL;
				}

				m_nextBite = gpGlobals->time + 2;

				return;
			}

			// bite prey every once in a while
			if ( m_nextBite < gpGlobals->time)
			{
				m_nextBite = gpGlobals->time + RANDOM_FLOAT(1.5f, 4.0f);

				EMIT_SOUND(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pChewSounds), 1, ATTN_NORM);

				m_hEnemy->BarnacleVictimBitten( pev );
			}

		}
	}
	else
	{
		// barnacle has no prey right now, so just idle and check to see if anything is touching the tongue.

		// If idle and no nearby client, don't think so often
		if (!UTIL_IsClientInPVS(edict()))
			pev->nextthink = gpGlobals->time + RANDOM_FLOAT(1,1.5);	// Stagger a bit to keep barnacles from thinking on the same frame

		if ( m_fSequenceFinished )
		{// this is done so barnacle will fidget.
			SetActivity ( ACT_IDLE );
			m_flTongueAdj = -100;
		}

		if ( m_cGibs && RANDOM_LONG(0,99) == 1 )
		{
			// cough up a gib.
			CGib::SpawnMonsterGibs( pev, 1, 1 );
			m_cGibs--;

			EMIT_SOUND(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pChewSounds), 1, ATTN_NORM);
		}

		pTouchEnt = TongueTouchEnt( &flLength );

		if ( pTouchEnt != NULL && m_fTongueExtended )
		{
			// tongue is fully extended, and is touching someone.
			if ( pTouchEnt->BarnacleVictimCaught() )
			{
				EMIT_SOUND( ENT(pev), CHAN_WEAPON, "barnacle/bcl_alert2.wav", 1, ATTN_NORM );	

				SetSequenceByName ( "attack1" );
				m_flTongueAdj = -20;

				m_hEnemy = pTouchEnt;

				// TODO: use BOUNCE for interpolation. Save previous gravity and friction values somewhere
				pTouchEnt->pev->movetype = MOVETYPE_FLY;
				pTouchEnt->pev->velocity = g_vecZero;
				pTouchEnt->pev->basevelocity = g_vecZero;

				TraceResult tr;
				Vector vecStart = pev->origin + Vector(0, 0, 32); // start from a bit higher in case of slope
				Vector vecEnd(pev->origin.x, pev->origin.y, pTouchEnt->pev->origin.z);
				TRACE_MONSTER_HULL(pTouchEnt->edict(), vecStart, vecEnd, ignore_monsters, pTouchEnt->edict(), &tr);

				pTouchEnt->pev->origin.x = tr.vecEndPos.x;
				pTouchEnt->pev->origin.y = tr.vecEndPos.y;

				m_fLiftingPrey = TRUE;// indicate that we should be lifting prey.
				m_flKillVictimTime = -1;// set this to a bogus time while the victim is lifted.

				m_flAltitude = (pev->origin.z - pTouchEnt->EyePosition().z);
			}
		}
		else
		{
			// think faster to prevent grenades passing thru
			pev->nextthink = gpGlobals->time + 0.02f;

			// calculate a new length for the tongue to be clear of anything else that moves under it. 
			if ( m_flAltitude < flLength )
			{
				// if tongue is higher than is should be, lower it kind of slowly.
				m_flAltitude += gSkillData.sk_barnacle_pullspeed * 0.2f;
				m_fTongueExtended = FALSE;
			}
			else
			{
				m_flAltitude = flLength;
				m_fTongueExtended = TRUE;
			}

		}

	}

	//ALERT( at_console, "tounge %f %f LEN %f\n", m_flAltitude, m_flTongueAdj, flLength);
	SetBoneController( 0, -(m_flAltitude + m_flTongueAdj) );
	StudioFrameAdvance();
}

//=========================================================
// Killed.
//=========================================================
void CBarnacle :: Killed( entvars_t *pevAttacker, int iGib )
{
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;

	if ( m_hEnemy != NULL )
	{
		m_hEnemy->BarnacleVictimReleased();
	}

//	CGib::SpawnRandomGibs( pev, 4, 1 );

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pDieSounds), 1, ATTN_NORM);
	
	SetActivity ( ACT_DIESIMPLE );
	SetBoneController( 0, 0 );

	StudioFrameAdvance();

	pev->nextthink = gpGlobals->time + 0.1;
	SetThink ( &CBarnacle::WaitTillDead );
}

//=========================================================
//=========================================================
void CBarnacle :: WaitTillDead ( void )
{
	pev->nextthink = gpGlobals->time + 0.1;

	float flInterval = StudioFrameAdvance();
	DispatchAnimEvents ( flInterval );
	UpdateShockEffect();

	if ( m_fSequenceFinished )
	{
		// death anim finished. 
		StopAnimation();
		SetThink ( NULL );
	}
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CBarnacle :: Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/barnacle.mdl";
	PRECACHE_MODEL(GetModel());

	PRECACHE_SOUND_ARRAY(pChewSounds);
	PRECACHE_SOUND_ARRAY(pDieSounds);

	PRECACHE_SOUND("barnacle/bcl_alert2.wav");//happy, lifting food up
	PRECACHE_SOUND("barnacle/bcl_bite3.wav");//just got food to mouth
}	

void CBarnacle::Touch(CBaseEntity* pOther) {
	if (pOther == m_hEnemy.GetEntity()) {
		m_touchedEnemy = true;
	}
}

//=========================================================
// TongueTouchEnt - does a trace along the barnacle's tongue
// to see if any entity is touching it. Also stores the length
// of the trace in the int pointer provided.
//=========================================================
CBaseEntity *CBarnacle :: TongueTouchEnt ( float *pflLength )
{
	TraceResult	tr;
	float		length;

	// trace once to hit architecture and see if the tongue needs to change position.
	UTIL_TraceLine ( pev->origin, pev->origin - Vector ( 0 , 0 , 2048 ), ignore_monsters, ENT(pev), &tr );
	length = fabs( pev->origin.z - tr.vecEndPos.z );
	if ( pflLength )
	{
		*pflLength = length;
	}

	if (gpGlobals->time < m_nextBite)
		return NULL;

	Vector delta = Vector( BARNACLE_CHECK_SPACING, BARNACLE_CHECK_SPACING, 0 );
	Vector mins = pev->origin - delta;
	Vector maxs = pev->origin + delta;
	maxs.z = pev->origin.z;
	mins.z -= length;

	CBaseEntity *pList[10];
	int count = UTIL_EntitiesInBox( pList, 10, mins, maxs, 0, true);
	if ( count )
	{
		for ( int i = 0; i < count; i++ )
		{
			if (pList[i] == this || pList[i]->pev->movetype == MOVETYPE_NOCLIP)
				continue;

			if (pList[i]->IsMonster() && pList[i]->pev->deadflag != DEAD_NO)
				continue;

			if (pList[i]->IsBarnacleFood() )
			{
				m_touchedEnemy = false;
				return pList[i];
			}
		}
	}

	return NULL;
}
