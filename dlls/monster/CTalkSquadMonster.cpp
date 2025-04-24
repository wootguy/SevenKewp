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
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "schedule.h"
#include "CTalkSquadMonster.h"
#include "defaultai.h"
#include "CCineMonster.h"
#include "env/CSoundEnt.h"
#include "animation.h"
#include "plane.h"
#include "sentences.h"

extern Schedule_t	slChaseEnemyFailed[];

float	CTalkSquadMonster::g_talkWaitTime = 0;		// time delay until it's ok to speak: used so that two NPCs don't talk at once

// NOTE: m_voicePitch & m_szGrp should be fixed up by precache each save/restore

TYPEDESCRIPTION	CTalkSquadMonster::m_SaveData[] = 
{
	DEFINE_FIELD( CTalkSquadMonster, m_bitsSaid, FIELD_INTEGER ),
	DEFINE_FIELD( CTalkSquadMonster, m_nSpeak, FIELD_INTEGER ),

	// Recalc'ed in Precache()
	//	DEFINE_FIELD( CTalkSquadMonster, m_voicePitch, FIELD_INTEGER ),
	//	DEFINE_FIELD( CTalkSquadMonster, m_szGrp, FIELD_??? ),
	DEFINE_FIELD( CTalkSquadMonster, m_useTime, FIELD_TIME ),
	DEFINE_FIELD( CTalkSquadMonster, m_iszUse, FIELD_STRING ),
	DEFINE_FIELD( CTalkSquadMonster, m_iszUnUse, FIELD_STRING ),
	DEFINE_FIELD( CTalkSquadMonster, m_flLastSaidSmelled, FIELD_TIME ),
	DEFINE_FIELD( CTalkSquadMonster, m_flStopTalkTime, FIELD_TIME ),
	DEFINE_FIELD( CTalkSquadMonster, m_hTalkTarget, FIELD_EHANDLE ),

	DEFINE_FIELD(CTalkSquadMonster, m_hSquadLeader, FIELD_EHANDLE),
	DEFINE_ARRAY(CTalkSquadMonster, m_hSquadMember, FIELD_EHANDLE, MAX_SQUAD_MEMBERS - 1),

	// DEFINE_FIELD( CTalkSquadMonster, m_afSquadSlots, FIELD_INTEGER ), // these need to be reset after transitions!
	DEFINE_FIELD(CTalkSquadMonster, m_fEnemyEluded, FIELD_BOOLEAN),
	DEFINE_FIELD(CTalkSquadMonster, m_flLastEnemySightTime, FIELD_TIME),

	DEFINE_FIELD(CTalkSquadMonster, m_iMySlot, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE( CTalkSquadMonster, CBaseMonster )

// array of friend names
const char *CTalkSquadMonster::m_szFriends[TLK_CFRIENDS] = 
{
	"monster_barney",
	"monster_otis",
	"monster_scientist",
	"monster_sitting_scientist",
	"monster_human_grunt_ally",
	"monster_human_medic_ally",
	"monster_human_torch_ally",
	"monster_bodyguard",
};


//=========================================================
// AI Schedules Specific to talking monsters
//=========================================================

Task_t	tlIdleResponse[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},// Stop and listen
	{ TASK_WAIT,			(float)0.5		},// Wait until sure it's me they are talking to
	{ TASK_TLK_EYECONTACT,	(float)0		},// Wait until speaker is done
	{ TASK_TLK_RESPOND,		(float)0		},// Wait and then say my response
	{ TASK_TLK_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		}, 
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},
	{ TASK_TLK_EYECONTACT,	(float)0		},// Wait until speaker is done
};

Schedule_t	slIdleResponse[] =
{
	{ 
		tlIdleResponse,
		ARRAYSIZE ( tlIdleResponse ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"TALK_IDLE_RESPONSE"

	},
};

Task_t	tlIdleSpeak[] =
{
	{ TASK_TLK_SPEAK,		(float)0		},// question or remark
	{ TASK_TLK_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		}, 
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT_RANDOM,		(float)0.5		},
};

Schedule_t	slIdleSpeak[] =
{
	{ 
		tlIdleSpeak,
		ARRAYSIZE ( tlIdleSpeak ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_CLIENT_PUSH	|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"TALK_IDLE_SPEAK"
	},
};

Task_t	tlIdleSpeakWait[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},// Stop and talk
	{ TASK_TLK_SPEAK,		(float)0		},// question or remark
	{ TASK_TLK_EYECONTACT,	(float)0		},// 
	{ TASK_WAIT,			(float)2		},// wait - used when sci is in 'use' mode to keep head turned
};

Schedule_t	slIdleSpeakWait[] =
{
	{ 
		tlIdleSpeakWait,
		ARRAYSIZE ( tlIdleSpeakWait ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_CLIENT_PUSH	|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"TALK_IDLE_SPEAK_WAIT"
	},
};

Task_t	tlIdleHello[] =
{
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},// Stop and talk
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit
	{ TASK_TLK_HELLO,		(float)0		},// Try to say hello to player
	{ TASK_TLK_EYECONTACT,	(float)0		},
	{ TASK_WAIT,			(float)0.5		},// wait a bit

};

Schedule_t	slIdleHello[] =
{
	{ 
		tlIdleHello,
		ARRAYSIZE ( tlIdleHello ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_CLIENT_PUSH	|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT,
		"TALK_IDLE_HELLO"
	},
};

Task_t	tlIdleStopShooting[] =
{
	{ TASK_TLK_STOPSHOOTING,	(float)0		},// tell player to stop shooting friend
	// { TASK_TLK_EYECONTACT,		(float)0		},// look at the player
};

Schedule_t	slIdleStopShooting[] =
{
	{ 
		tlIdleStopShooting,
		ARRAYSIZE ( tlIdleStopShooting ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,
		0,
		"TALK_IDLE_STOP_SHOOTING"
	},
};

Task_t	tlTlkIdleWatchClient[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_TLK_LOOK_AT_CLIENT,	(float)6		},
};

Task_t	tlTlkIdleWatchClientStare[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_TLK_CLIENT_STARE,	(float)6		},
	{ TASK_TLK_STARE,			(float)0		},
	{ TASK_TLK_IDEALYAW,		(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,			(float)0		}, 
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE	},
	{ TASK_TLK_EYECONTACT,		(float)0		},
};

Schedule_t	slTlkIdleWatchClient[] =
{
	{
		tlTlkIdleWatchClient,
		ARRAYSIZE ( tlTlkIdleWatchClient ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_CLIENT_PUSH	|
		bits_COND_CLIENT_UNSEEN	|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"TALK_IDLE_WATCH_CLIENT1"
	},

	{ 
		tlTlkIdleWatchClientStare,
		ARRAYSIZE ( tlTlkIdleWatchClientStare ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND	|
		bits_COND_SMELL			|
		bits_COND_CLIENT_PUSH	|
		bits_COND_CLIENT_UNSEEN	|
		bits_COND_PROVOKED,

		bits_SOUND_COMBAT		|// sound flags - change these, and you'll break the talking code.
		//bits_SOUND_PLAYER		|
		//bits_SOUND_WORLD		|
		
		bits_SOUND_DANGER		|
		bits_SOUND_MEAT			|// scents
		bits_SOUND_CARCASS		|
		bits_SOUND_GARBAGE,
		"TALK_IDLE_WATCH_CLIENT2"
	},
};


Task_t	tlTlkIdleEyecontact[] =
{
	{ TASK_TLK_IDEALYAW,	(float)0		},// look at who I'm talking to
	{ TASK_FACE_IDEAL,		(float)0		}, 
	{ TASK_SET_ACTIVITY,	(float)ACT_IDLE	},
	{ TASK_TLK_EYECONTACT,	(float)0		},// Wait until speaker is done
};

Schedule_t	slTlkIdleEyecontact[] =
{
	{ 
		tlTlkIdleEyecontact,
		ARRAYSIZE ( tlTlkIdleEyecontact ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_CLIENT_PUSH	|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"TALK_EYE_CONTACT"
	},
};


DEFINE_CUSTOM_SCHEDULES( CTalkSquadMonster )
{
	slIdleResponse,
	slIdleSpeak,
	slIdleHello,
	slIdleSpeakWait,
	slIdleStopShooting,
	slTlkIdleWatchClient,
	&slTlkIdleWatchClient[ 1 ],
	slTlkIdleEyecontact,
};

IMPLEMENT_CUSTOM_SCHEDULES( CTalkSquadMonster, CBaseMonster )


void CTalkSquadMonster :: SetActivity ( Activity newActivity )
{
	CBaseMonster::SetActivity( newActivity );
}


void CTalkSquadMonster :: StartTask( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_TLK_SPEAK:
		// ask question or make statement
		FIdleSpeak();
		TaskComplete();
		break;

	case TASK_TLK_RESPOND:
		// respond to question
		IdleRespond();
		TaskComplete();
		break;

	case TASK_TLK_HELLO:
		// greet player
		FIdleHello();
		TaskComplete();
		break;
	

	case TASK_TLK_STARE:
		// let the player know I know he's staring at me.
		FIdleStare();
		TaskComplete();
		break;

	case TASK_FACE_PLAYER:
	case TASK_TLK_LOOK_AT_CLIENT:
	case TASK_TLK_CLIENT_STARE:
		// track head to the client for a while.
		m_flWaitFinished = gpGlobals->time + pTask->flData;
		break;

	case TASK_TLK_EYECONTACT:
		break;

	case TASK_TLK_IDEALYAW:
		if (m_hTalkTarget != NULL)
		{
			pev->yaw_speed = 60;
			float yaw = VecToYaw(m_hTalkTarget->pev->origin - pev->origin) - pev->angles.y;

			if (yaw > 180) yaw -= 360;
			if (yaw < -180) yaw += 360;

			if (yaw < 0)
			{
				pev->ideal_yaw = V_min( yaw + 45, 0 ) + pev->angles.y;
			}
			else
			{
				pev->ideal_yaw = V_max( yaw - 45, 0 ) + pev->angles.y;
			}
		}
		TaskComplete();
		break;

	case TASK_TLK_HEADRESET:
		// reset head position after looking at something
		m_hTalkTarget = NULL;
		TaskComplete();
		break;

	case TASK_TLK_STOPSHOOTING:
		// tell player to stop shooting
		PlaySentence( m_szGrp[TLK_NOSHOOT], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_NORM );
		TaskComplete();
		break;

	case TASK_CANT_FOLLOW:
		StopFollowing(FALSE);
		PlaySentence(m_szGrp[TLK_STOP], RANDOM_FLOAT(2, 2.5), VOL_NORM, ATTN_NORM);
		TaskComplete();
		break;

	case TASK_PLAY_SCRIPT:
		m_hTalkTarget = NULL;
		CBaseMonster::StartTask( pTask );
		break;

	default:
		CBaseMonster::StartTask( pTask );
	}
}


void CTalkSquadMonster :: RunTask( Task_t *pTask )
{
	switch( pTask->iTask )
	{
	case TASK_TLK_CLIENT_STARE:
	case TASK_TLK_LOOK_AT_CLIENT:
	{
		edict_t* pPlayer = NULL;

		// track head to the client for a while.
		if (m_MonsterState == MONSTERSTATE_IDLE &&
			!IsMoving() &&
			!IsTalking())
		{
			// Get edict for one player
			pPlayer = g_engfuncs.pfnPEntityOfEntIndex(1);

			if (pPlayer)
			{
				IdleHeadTurn(pPlayer->v.origin);
			}
		}
		else
		{
			// started moving or talking
			TaskFail();
			return;
		}

		if (pTask->iTask == TASK_TLK_CLIENT_STARE)
		{
			// fail out if the player looks away or moves away.
			if ((pPlayer->v.origin - pev->origin).Length2D() > TLK_STARE_DIST)
			{
				// player moved away.
				TaskFail();
			}

			UTIL_MakeVectors(pPlayer->v.angles);
			if (UTIL_DotPoints(pPlayer->v.origin, pev->origin, gpGlobals->v_forward) < m_flFieldOfView)
			{
				// player looked away
				TaskFail();
			}
		}

		if (gpGlobals->time > m_flWaitFinished)
		{
			TaskComplete();
		}
		break;
	}
	case TASK_FACE_PLAYER:
		{
			// Get edict for one player
			edict_t *pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );

			if ( pPlayer )
			{
				MakeIdealYaw ( pPlayer->v.origin );
				ChangeYaw ( pev->yaw_speed );
				IdleHeadTurn( pPlayer->v.origin );
				if ( gpGlobals->time > m_flWaitFinished && FlYawDiff() < 10 )
				{
					TaskComplete();
				}
			}
			else
			{
				TaskFail();
			}
		}
		break;

	case TASK_TLK_EYECONTACT:
		if (!IsMoving() && IsTalking() && m_hTalkTarget != NULL && !IsFollowing())
		{
			// ALERT( at_console, "waiting %f\n", m_flStopTalkTime - gpGlobals->time );
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			TaskComplete();
		}
		break;

	case TASK_WAIT_FOR_MOVEMENT:
		if (IsTalking() && m_hTalkTarget != NULL)
		{
			// ALERT(at_console, "walking, talking\n");
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			IdleHeadTurn( pev->origin );
			// override so that during walk, a scientist may talk and greet player
			FIdleHello();
			if (RANDOM_LONG(0,m_nSpeak * 20) == 0)
			{
				FIdleSpeak();
			}
		}

		CBaseMonster::RunTask( pTask );
		if (TaskIsComplete())
			IdleHeadTurn( pev->origin );
		break;

	default:
		if (IsTalking() && m_hTalkTarget != NULL)
		{
			IdleHeadTurn( m_hTalkTarget->pev->origin );
		}
		else
		{
			SetBoneController( 0, 0 );
		}
		CBaseMonster::RunTask( pTask );
	}
}

const char* CTalkSquadMonster::GetTaskName(int taskIdx) {
	switch (taskIdx) {
		case TASK_TLK_RESPOND: return "TASK_TLK_RESPOND";
		case TASK_TLK_SPEAK: return "TASK_TLK_SPEAK";
		case TASK_TLK_HELLO: return "TASK_TLK_HELLO";
		case TASK_TLK_HEADRESET: return "TASK_TLK_HEADRESET";
		case TASK_TLK_STOPSHOOTING: return "TASK_TLK_STOPSHOOTING";
		case TASK_TLK_STARE: return "TASK_TLK_STARE";
		case TASK_TLK_LOOK_AT_CLIENT: return "TASK_TLK_LOOK_AT_CLIENT";
		case TASK_TLK_CLIENT_STARE: return "TASK_TLK_CLIENT_STARE";
		case TASK_TLK_EYECONTACT: return "TASK_TLK_EYECONTACT";
		case TASK_TLK_IDEALYAW: return "TASK_TLK_IDEALYAW";
		default:
			return CBaseMonster::GetTaskName(taskIdx);
	}
}

void CTalkSquadMonster :: Killed( entvars_t *pevAttacker, int iGib )
{
	// If a client killed me (unless I was already Barnacle'd), make everyone else mad/afraid of him
	if ((pevAttacker->flags & FL_CLIENT) && m_MonsterState != MONSTERSTATE_PRONE )
	{
		AlertFriends((CBaseEntity*)GET_PRIVATE(ENT(pevAttacker)));
		//LimitFollowers( CBaseEntity::Instance(pevAttacker), 0 );
	}

	m_hTargetEnt = NULL;
	// Don't finish that sentence
	StopTalking();
	SetUse( NULL );

	VacateSlot();
	if (InSquad())
	{
		MySquadLeader()->SquadRemove(this);
	}

	CBaseMonster::Killed( pevAttacker, iGib );
}



CBaseEntity	*CTalkSquadMonster::EnumFriends( CBaseEntity *pPrevious, int listNumber, BOOL bTrace )
{
	CBaseEntity *pFriend = pPrevious;
	const char *pszFriend;
	TraceResult tr;
	Vector vecCheck;

	pszFriend = m_szFriends[ FriendNumber(listNumber) ];
	while ((pFriend = UTIL_FindEntityByClassname( pFriend, pszFriend )) != 0)
	{
		if (pFriend == this || !pFriend->IsAlive())
			// don't talk to self or dead people
			continue;

		if (IRelationship(pFriend) > R_NO) {
			continue; // don't talk to enemies
		}

		if ( bTrace )
		{
			vecCheck = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			UTIL_TraceLine( pev->origin, vecCheck, ignore_monsters, ENT(pev), &tr);
		}
		else
			tr.flFraction = 1.0;

		if (tr.flFraction == 1.0)
		{
			return pFriend;
		}
	}

	return NULL;
}


void CTalkSquadMonster::AlertFriends(CBaseEntity* attacker)
{
	CBaseEntity *pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for ( i = 0; i < TLK_CFRIENDS; i++ )
	{
		while ((pFriend = EnumFriends( pFriend, i, TRUE )) != 0)
		{
			CTalkSquadMonster* pMonster = pFriend->MyTalkSquadMonsterPointer();
			if (pMonster && pMonster->IsAlive() && pMonster->canBeMadAtPlayer) {
				// don't provoke a friend that's playing a death animation. They're a goner
				pMonster->m_afMemory |= bits_MEMORY_PROVOKED;

				if (attacker && attacker->IsPlayer())
					pMonster->m_bMadPlayer[attacker->entindex()-1] = true;
			}
		}
	}
}

void CTalkSquadMonster::UnprovokeFriends(void) {
	CBaseEntity* pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for (i = 0; i < TLK_CFRIENDS; i++)
	{
		while ((pFriend = EnumFriends(pFriend, i, TRUE)) != 0)
		{
			CTalkSquadMonster* pMonster = pFriend->MyTalkSquadMonsterPointer();
			if (pMonster && pMonster->IsAlive()) {
				pMonster->Unprovoke(false);
			}
		}
	}
}

void CTalkSquadMonster::ShutUpFriends( void )
{
	CBaseEntity *pFriend = NULL;
	int i;

	// for each friend in this bsp...
	for ( i = 0; i < TLK_CFRIENDS; i++ )
	{
		while ((pFriend = EnumFriends( pFriend, i, TRUE )) != 0)
		{
			CBaseMonster *pMonster = pFriend->MyMonsterPointer();
			if ( pMonster )
			{
				pMonster->SentenceStop();
			}
		}
	}
}


float CTalkSquadMonster::TargetDistance( void )
{
	// If we lose the player, or he dies, return a really large distance
	if ( m_hTargetEnt == NULL || !m_hTargetEnt->IsAlive() )
		return 1e6;

	return (m_hTargetEnt->pev->origin - pev->origin).Length();
}


//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CTalkSquadMonster :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{		
	case SCRIPT_EVENT_SENTENCE_RND1:		// Play a named sentence group 25% of the time
		if (RANDOM_LONG(0,99) < 75)
			break;
		// fall through...
	case SCRIPT_EVENT_SENTENCE:				// Play a named sentence group
		ShutUpFriends();
		PlaySentence( pEvent->options, RANDOM_FLOAT(2.8, 3.4), VOL_NORM, ATTN_IDLE );
		//ALERT(at_console, "script event speak\n");
		break;

	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

// monsters derived from CTalkSquadMonster should call this in precache()

void CTalkSquadMonster :: TalkInit( void )
{
	// every new talking monster must reset this global, otherwise
	// when a level is loaded, nobody will talk (time is reset to 0)

	CTalkSquadMonster::g_talkWaitTime = 0;

	m_voicePitch = 100;
}	
//=========================================================
// FindNearestFriend
// Scan for nearest, visible friend. If fPlayer is true, look for
// nearest player
//=========================================================
CBaseEntity *CTalkSquadMonster :: FindNearestFriend(BOOL fPlayer)
{
	CBaseEntity *pFriend = NULL;
	CBaseEntity *pNearest = NULL;
	float range = 10000000.0;
	TraceResult tr;
	Vector vecStart = pev->origin;
	Vector vecCheck;
	int i;
	const char *pszFriend;
	int cfriends;

	vecStart.z = pev->absmax.z;
	
	if (fPlayer)
		cfriends = 1;
	else
		cfriends = TLK_CFRIENDS;

	// for each type of friend...

	for (i = cfriends-1; i > -1; i--)
	{
		if (fPlayer)
			pszFriend = "player";
		else
			pszFriend = m_szFriends[FriendNumber(i)];

		if (!pszFriend)
			continue;

		// for each friend in this bsp...
		while ((pFriend = UTIL_FindEntityByClassname( pFriend, pszFriend )) != 0)
		{
			if (pFriend == this || !pFriend->IsAlive())
				// don't talk to self or dead people
				continue;

			if (IRelationship(pFriend) != R_AL)
				continue; // don't talk to enemies

			CBaseMonster *pMonster = pFriend->MyMonsterPointer();

			// If not a monster for some reason, or in a script, or prone
			if ( !pMonster || pMonster->m_MonsterState == MONSTERSTATE_SCRIPT || pMonster->m_MonsterState == MONSTERSTATE_PRONE )
				continue;

			vecCheck = pFriend->pev->origin;
			vecCheck.z = pFriend->pev->absmax.z;

			// if closer than previous friend, and in range, see if he's visible

			if (range > (vecStart - vecCheck).Length())
			{
				UTIL_TraceLine(vecStart, vecCheck, ignore_monsters, ENT(pev), &tr);

				if (tr.flFraction == 1.0)
				{
					// visible and in range, this is the new nearest scientist
					if ((vecStart - vecCheck).Length() < TALKRANGE_MIN)
					{
						pNearest = pFriend;
						range = (vecStart - vecCheck).Length();
					}
				}
			}
		}
	}
	return pNearest;
}

int CTalkSquadMonster :: GetVoicePitch( void )
{
	return m_voicePitch + RANDOM_LONG(0,3);
}

//=========================================================
// IdleRespond
// Respond to a previous question
//=========================================================
void CTalkSquadMonster :: IdleRespond( void )
{	
	// play response
	PlaySentence( m_szGrp[TLK_ANSWER], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
}

int CTalkSquadMonster :: FOkToSpeak( void )
{
	// if in the grip of a barnacle, don't speak
	if ( m_MonsterState == MONSTERSTATE_PRONE || m_IdealMonsterState == MONSTERSTATE_PRONE )
	{
		return FALSE;
	}

	// if not alive, certainly don't speak
	if ( pev->deadflag != DEAD_NO )
	{
		return FALSE;
	}

	// if someone else is talking, don't speak
	if (gpGlobals->time <= CTalkSquadMonster::g_talkWaitTime)
		return FALSE;

	// don't get stuck in an infinite loop if there's no one to talk to
	if (gpGlobals->time < m_lastTalkFail + 2.0f) {
		return FALSE;
	}

	if ( pev->spawnflags & SF_MONSTER_GAG )
		return FALSE;

	if ( m_MonsterState == MONSTERSTATE_PRONE )
		return FALSE;

	// if player is not in pvs, don't speak
	if (!IsAlive() || !UTIL_IsClientInPVS(edict()))
		return FALSE;

	// don't talk if you're in combat
	if (m_hEnemy != NULL && FVisible( m_hEnemy ))
		return FALSE;

	return TRUE;
}


int CTalkSquadMonster::CanPlaySentence( BOOL fDisregardState ) 
{ 
	if ( fDisregardState )
		return CBaseMonster::CanPlaySentence( fDisregardState );
	return FOkToSpeak(); 
}

//=========================================================
// FIdleStare
//=========================================================
int CTalkSquadMonster :: FIdleStare( void )
{
	if (!FOkToSpeak())
		return FALSE;

	PlaySentence( m_szGrp[TLK_STARE], RANDOM_FLOAT(5, 7.5), VOL_NORM, ATTN_IDLE );

	m_hTalkTarget = FindNearestFriend( TRUE );
	return TRUE;
}

//=========================================================
// IdleHello
// Try to greet player first time he's seen
//=========================================================
int CTalkSquadMonster :: FIdleHello( void )
{
	// if this is first time scientist has seen player, greet him
	if (!FBitSet(m_bitsSaid, bit_saidHelloPlayer))
	{
		// get a player
		CBaseEntity *pPlayer = FindNearestFriend(TRUE);

		if (pPlayer)
		{
			if (FInViewCone(pPlayer) && FVisible(pPlayer))
			{
				m_hTalkTarget = pPlayer;

				if (FBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER))
					PlaySentence( m_szGrp[TLK_PHELLO], RANDOM_FLOAT(3, 3.5), VOL_NORM,  ATTN_IDLE );
				else
					PlaySentence( m_szGrp[TLK_HELLO], RANDOM_FLOAT(3, 3.5), VOL_NORM,  ATTN_IDLE );

				SetBits(m_bitsSaid, bit_saidHelloPlayer);
				
				return TRUE;
			}
		}
	}

	Talk(0); // don't keep trying
	return FALSE;
}


// turn head towards supplied origin
void CTalkSquadMonster :: IdleHeadTurn( Vector &vecFriend )
{
	 // turn head in desired direction only if ent has a turnable head
	if (m_afCapability & bits_CAP_TURN_HEAD)
	{
		float yaw = VecToYaw(vecFriend - pev->origin) - pev->angles.y;

		if (yaw > 180) yaw -= 360;
		if (yaw < -180) yaw += 360;

		// turn towards vector
		SetBoneController( 0, yaw );
	}
}

//=========================================================
// FIdleSpeak
// ask question of nearby friend, or make statement
//=========================================================
int CTalkSquadMonster :: FIdleSpeak ( void )
{ 
	// try to start a conversation, or make statement
	const char *szIdleGroup;
	const char *szQuestionGroup;
	float duration;

	if (!FOkToSpeak())
		return FALSE;

	// set idle groups based on pre/post disaster
	if (FBitSet(pev->spawnflags, SF_MONSTER_PREDISASTER))
	{
		szIdleGroup = m_szGrp[TLK_PIDLE];
		szQuestionGroup = m_szGrp[TLK_PQUESTION];
		// set global min delay for next conversation
		duration = RANDOM_FLOAT(4.8, 5.2);
	}
	else
	{
		szIdleGroup = m_szGrp[TLK_IDLE];
		szQuestionGroup = m_szGrp[TLK_QUESTION];
		// set global min delay for next conversation
		duration = RANDOM_FLOAT(2.8, 3.2);

	}
		
	// player using this entity is alive and wounded?
	CBaseEntity *pTarget = m_hTargetEnt;

	if ( pTarget != NULL )
	{
		if ( pTarget->IsPlayer() )
		{
			if ( pTarget->IsAlive() )
			{
				m_hTalkTarget = m_hTargetEnt;
				if (!FBitSet(m_bitsSaid, bit_saidDamageHeavy) && 
					(m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 8))
				{
					//EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, m_szGrp[TLK_PLHURT3], 1.0, ATTN_IDLE, 0, pitch);
					PlaySentence( m_szGrp[TLK_PLHURT3], duration, VOL_NORM, ATTN_IDLE );
					SetBits(m_bitsSaid, bit_saidDamageHeavy);
					return TRUE;
				}
				else if (!FBitSet(m_bitsSaid, bit_saidDamageMedium) && 
					(m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 4))
				{
					//EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, m_szGrp[TLK_PLHURT2], 1.0, ATTN_IDLE, 0, pitch);
					PlaySentence( m_szGrp[TLK_PLHURT2], duration, VOL_NORM, ATTN_IDLE );
					SetBits(m_bitsSaid, bit_saidDamageMedium);
					return TRUE;
				}
				else if (!FBitSet(m_bitsSaid, bit_saidDamageLight) &&
					(m_hTargetEnt->pev->health <= m_hTargetEnt->pev->max_health / 2))
				{
					//EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, m_szGrp[TLK_PLHURT1], 1.0, ATTN_IDLE, 0, pitch);
					PlaySentence( m_szGrp[TLK_PLHURT1], duration, VOL_NORM, ATTN_IDLE );
					SetBits(m_bitsSaid, bit_saidDamageLight);
					return TRUE;
				}
			}
			else
			{
				//!!!KELLY - here's a cool spot to have the talkmonster talk about the dead player if we want.
				// "Oh dear, Gordon Freeman is dead!" -Scientist
				// "Damn, I can't do this without you." -Barney
			}
		}
	}

	// if there is a friend nearby to speak to, play sentence, set friend's response time, return
	CBaseEntity *pFriend = FindNearestFriend(FALSE);

	if (pFriend && !(pFriend->IsMoving()) && (RANDOM_LONG(0,99) < 75))
	{
		PlaySentence( szQuestionGroup, duration, VOL_NORM, ATTN_IDLE );
		//SENTENCEG_PlayRndSz( ENT(pev), szQuestionGroup, 1.0, ATTN_IDLE, 0, pitch );

		// force friend to answer
		CTalkSquadMonster *pTalkMonster = (CTalkSquadMonster *)pFriend;
		m_hTalkTarget = pFriend;
		pTalkMonster->SetAnswerQuestion( this ); // UNDONE: This is EVIL!!!
		pTalkMonster->m_flStopTalkTime = m_flStopTalkTime;

		m_nSpeak++;
		return TRUE;
	}

	// otherwise, play an idle statement, try to face client when making a statement.
	if ( RANDOM_LONG(0,1) )
	{
		//SENTENCEG_PlayRndSz( ENT(pev), szIdleGroup, 1.0, ATTN_IDLE, 0, pitch );
		pFriend = FindNearestFriend(TRUE);

		if ( pFriend )
		{
			m_hTalkTarget = pFriend;
			PlaySentence( szIdleGroup, duration, VOL_NORM, ATTN_IDLE );
			m_nSpeak++;
			return TRUE;
		}
	}

	// didn't speak
	Talk( 0 );
	CTalkSquadMonster::g_talkWaitTime = 0;
	return FALSE;
}

void CTalkSquadMonster::PlayScriptedSentence( const char *pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity *pListener )
{
	if ( !bConcurrent )
		ShutUpFriends();

	ClearConditions( bits_COND_CLIENT_PUSH );	// Forget about moving!  I've got something to say!
	m_useTime = gpGlobals->time + duration;
	PlaySentence( pszSentence, duration, volume, attenuation );

	m_hTalkTarget = pListener;
}

void CTalkSquadMonster::PlaySentence( const char *pszSentence, float duration, float volume, float attenuation )
{
	if ( !pszSentence )
		return;

	Talk ( duration );

	CTalkSquadMonster::g_talkWaitTime = gpGlobals->time + duration + 2.0;
	
	CBaseMonster::PlaySentence(pszSentence, duration, volume, attenuation);

	// If you say anything, don't greet the player - you may have already spoken to them
	SetBits(m_bitsSaid, bit_saidHelloPlayer);
}

//=========================================================
// Talk - set a timer that tells us when the monster is done
// talking.
//=========================================================
void CTalkSquadMonster :: Talk( float flDuration )
{
	if (flDuration <= 0)
	{
		// no duration :( 
		//m_flStopTalkTime = gpGlobals->time + 3;
		m_lastTalkFail = gpGlobals->time;
	}
	else
	{
		m_flStopTalkTime = gpGlobals->time + flDuration;
	}
}

// Prepare this talking monster to answer question
void CTalkSquadMonster :: SetAnswerQuestion( CTalkSquadMonster *pSpeaker )
{
	if ( !m_hCine )
		ChangeSchedule( slIdleResponse );
	m_hTalkTarget = (CBaseMonster *)pSpeaker;
}

int CTalkSquadMonster :: TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (IsImmune(pevAttacker, flDamage))
		return 0;

	if ( IsAlive() && flDamage > 0 )
	{
		CBaseEntity* attacker = (CBaseEntity*)GET_PRIVATE(ENT(pevAttacker));

		// if player damaged this entity, have other friends talk about it
		if (pevAttacker && m_MonsterState != MONSTERSTATE_PRONE && FBitSet(pevAttacker->flags, FL_CLIENT) && attacker && IRelationship(attacker) == R_AL)
		{
			CBaseEntity *pFriend = FindNearestFriend(FALSE);

			if (pFriend && pFriend->IsAlive())
			{
				// only if not dead or dying!
				CTalkSquadMonster *pTalkMonster = (CTalkSquadMonster *)pFriend;
				pTalkMonster->ChangeSchedule( slIdleStopShooting );
			}
		}
	}
	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}


Schedule_t* CTalkSquadMonster :: GetScheduleOfType ( int Type )
{
	switch( Type )
	{
	case SCHED_CHASE_ENEMY_FAILED:
	{
		return &slChaseEnemyFailed[0];
	}
	case SCHED_TARGET_FACE:
		// speak during 'use'
		if (RANDOM_LONG(0,99) < 2)
			//ALERT ( at_console, "target chase speak\n" );
			return IsFollowing() ? slIdleSpeak : slIdleSpeakWait;
		else
			return slFaceTarget;
		
	case SCHED_IDLE_STAND:
		{	
			// if never seen player, try to greet him
			if (!FBitSet(m_bitsSaid, bit_saidHelloPlayer) && HasConditions(bits_COND_SEE_CLIENT))
			{
				return slIdleHello;
			}

			// sustained light wounds?
			if (!FBitSet(m_bitsSaid, bit_saidWoundLight) && (pev->health <= (pev->max_health * 0.75)))
			{
				//SENTENCEG_PlayRndSz( ENT(pev), m_szGrp[TLK_WOUND], 1.0, ATTN_IDLE, 0, GetVoicePitch() );
				//CTalkSquadMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(2.8, 3.2);
				PlaySentence( m_szGrp[TLK_WOUND], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
				SetBits(m_bitsSaid, bit_saidWoundLight);
				return slIdleStand;
			}
			// sustained heavy wounds?
			else if (!FBitSet(m_bitsSaid, bit_saidWoundHeavy) && (pev->health <= (pev->max_health * 0.5)))
			{
				//SENTENCEG_PlayRndSz( ENT(pev), m_szGrp[TLK_MORTAL], 1.0, ATTN_IDLE, 0, GetVoicePitch() );
				//CTalkSquadMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(2.8, 3.2);
				PlaySentence( m_szGrp[TLK_MORTAL], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
				SetBits(m_bitsSaid, bit_saidWoundHeavy);
				return slIdleStand;
			}

			// talk about world
			if (FOkToSpeak() && RANDOM_LONG(0,m_nSpeak * 2) == 0)
			{
				//ALERT ( at_console, "standing idle speak\n" );
				return slIdleSpeak;
			}
			
			if ( !IsTalking() && HasConditions ( bits_COND_SEE_CLIENT ) && RANDOM_LONG( 0, 6 ) == 0 )
			{
				edict_t *pPlayer = g_engfuncs.pfnPEntityOfEntIndex( 1 );

				if ( pPlayer )
				{
					// watch the client.
					UTIL_MakeVectors ( pPlayer->v.angles );
					if ( ( pPlayer->v.origin - pev->origin ).Length2D() < TLK_STARE_DIST	&& 
						 UTIL_DotPoints( pPlayer->v.origin, pev->origin, gpGlobals->v_forward ) >= m_flFieldOfView )
					{
						// go into the special STARE schedule if the player is close, and looking at me too.
						return &slTlkIdleWatchClient[ 1 ];
					}

					return slTlkIdleWatchClient;
				}
			}
			else
			{
				if (IsTalking())
					// look at who we're talking to
					return slTlkIdleEyecontact;
				else
					// regular standing idle
					return slIdleStand;
			}


			// NOTE - caller must first CTalkSquadMonster::GetScheduleOfType, 
			// then check result and decide what to return ie: if sci gets back
			// slIdleStand, return slIdleSciStand
		}
		break;
	}

	return CBaseMonster::GetScheduleOfType( Type );
}

//=========================================================
// IsTalking - am I saying a sentence right now?
//=========================================================
BOOL CTalkSquadMonster :: IsTalking( void )
{
	if ( m_flStopTalkTime > gpGlobals->time )
	{
		return TRUE;
	}

	return FALSE;
}

//=========================================================
// If there's a player around, watch him.
//=========================================================
void CTalkSquadMonster :: PrescheduleThink ( void )
{
	if ( !HasConditions ( bits_COND_SEE_CLIENT ) )
	{
		SetConditions ( bits_COND_CLIENT_UNSEEN );
	}
}

// try to smell something
void CTalkSquadMonster :: TrySmellTalk( void )
{
	if ( !FOkToSpeak() )
		return;

	// clear smell bits periodically
	if ( gpGlobals->time > m_flLastSaidSmelled  )
	{
//		ALERT ( at_aiconsole, "Clear smell bits\n" );
		ClearBits(m_bitsSaid, bit_saidSmelled);
	}
	// smelled something?
	if (!FBitSet(m_bitsSaid, bit_saidSmelled) && HasConditions ( bits_COND_SMELL ))
	{
		PlaySentence( m_szGrp[TLK_SMELL], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE );
		m_flLastSaidSmelled = gpGlobals->time + 60;// don't talk about the stinky for a while.
		SetBits(m_bitsSaid, bit_saidSmelled);
	}
}


void CTalkSquadMonster::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "UseSentence"))
	{
		m_iszUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "UnUseSentence"))
	{
		m_iszUnUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else 
		CBaseMonster::KeyValue( pkvd );
}


void CTalkSquadMonster::Precache( void )
{
	CBaseMonster::Precache();

	if ( m_iszUse )
		m_szGrp[TLK_USE] = STRING( m_iszUse );
	if ( m_iszUnUse )
		m_szGrp[TLK_UNUSE] = STRING( m_iszUnUse );

	for (int i = 0; i < TLK_CGROUPS; i++) {
		if (m_szGrp[i] && g_customSentencesMap.groups.get(m_szGrp[i])) {
			PrecacheCustomSentence(this, m_szGrp[i]);
		}
	}
}


void CTalkSquadMonster::StopFollowing(BOOL clearSchedule)
{
	if (IsFollowing() && !(m_afMemory & bits_MEMORY_PROVOKED))
	{
		PlaySentence(m_szGrp[TLK_UNUSE], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
		m_hTalkTarget = m_hTargetEnt;
	}

	CBaseMonster::StopFollowing(clearSchedule);
}


void CTalkSquadMonster::StartFollowing(CBaseEntity* pLeader)
{
	PlaySentence(m_szGrp[TLK_USE], RANDOM_FLOAT(2.8, 3.2), VOL_NORM, ATTN_IDLE);
	m_hTalkTarget = m_hTargetEnt;
	SetBits(m_bitsSaid, bit_saidHelloPlayer);	// Don't say hi after you've started following

	CBaseMonster::StartFollowing(pLeader);
}

bool CTalkSquadMonster::CanBePushed()
{
	return !IsTalking();
}





//=========================================================
// OccupySlot - if any slots of the passed slots are 
// available, the monster will be assigned to one.
//=========================================================
BOOL CTalkSquadMonster::OccupySlot(int iDesiredSlots)
{
	int i;
	int iMask;
	int iSquadSlots;

	if (!InSquad())
	{
		return TRUE;
	}

	if (SquadEnemySplit())
	{
		// if the squad members aren't all fighting the same enemy, slots are disabled
		// so that a squad member doesn't get stranded unable to engage his enemy because
		// all of the attack slots are taken by squad members fighting other enemies.
		m_iMySlot = bits_SLOT_SQUAD_SPLIT;
		return TRUE;
	}

	CTalkSquadMonster* pSquadLeader = MySquadLeader();

	if (!(iDesiredSlots ^ pSquadLeader->m_afSquadSlots))
	{
		// none of the desired slots are available. 
		return FALSE;
	}

	iSquadSlots = pSquadLeader->m_afSquadSlots;

	for (i = 0; i < NUM_SLOTS; i++)
	{
		iMask = 1 << i;
		if (iDesiredSlots & iMask) // am I looking for this bit?
		{
			if (!(iSquadSlots & iMask))	// Is it already taken?
			{
				// No, use this bit
				pSquadLeader->m_afSquadSlots |= iMask;
				m_iMySlot = iMask;
				//				ALERT ( at_aiconsole, "Took slot %d - %d\n", i, m_hSquadLeader->m_afSquadSlots );
				return TRUE;
			}
		}
	}

	return FALSE;
}

//=========================================================
// VacateSlot 
//=========================================================
void CTalkSquadMonster::VacateSlot()
{
	if (m_iMySlot != bits_NO_SLOT && InSquad())
	{
		//		ALERT ( at_aiconsole, "Vacated Slot %d - %d\n", m_iMySlot, m_hSquadLeader->m_afSquadSlots );
		MySquadLeader()->m_afSquadSlots &= ~m_iMySlot;
		m_iMySlot = bits_NO_SLOT;
	}
}

//=========================================================
// ScheduleChange
//=========================================================
void CTalkSquadMonster::ScheduleChange(void)
{
	VacateSlot();
	CBaseMonster::ScheduleChange();
}

// These functions are still awaiting conversion to CTalkSquadMonster 
//=========================================================
//
// SquadRemove(), remove pRemove from my squad.
// If I am pRemove, promote m_pSquadNext to leader
//
//=========================================================
void CTalkSquadMonster::SquadRemove(CTalkSquadMonster* pRemove)
{
	ASSERT(pRemove != NULL);
	ASSERT(this->IsLeader());
	ASSERT(pRemove->m_hSquadLeader == this);

	// If I'm the leader, get rid of my squad
	if (pRemove == MySquadLeader())
	{
		for (int i = 0; i < MAX_SQUAD_MEMBERS - 1; i++)
		{
			CTalkSquadMonster* pMember = MySquadMember(i);
			if (pMember)
			{
				pMember->m_hSquadLeader = NULL;
				m_hSquadMember[i] = NULL;
			}
		}
	}
	else
	{
		CTalkSquadMonster* pSquadLeader = MySquadLeader();
		if (pSquadLeader)
		{
			for (int i = 0; i < MAX_SQUAD_MEMBERS - 1; i++)
			{
				if (pSquadLeader->m_hSquadMember[i] == this)
				{
					pSquadLeader->m_hSquadMember[i] = NULL;
					break;
				}
			}
		}
	}

	pRemove->m_hSquadLeader = NULL;
}

//=========================================================
//
// SquadAdd(), add pAdd to my squad
//
//=========================================================
BOOL CTalkSquadMonster::SquadAdd(CTalkSquadMonster* pAdd)
{
	ASSERT(pAdd != NULL);
	ASSERT(!pAdd->InSquad());
	ASSERT(this->IsLeader());

	for (int i = 0; i < MAX_SQUAD_MEMBERS - 1; i++)
	{
		if (m_hSquadMember[i] == NULL)
		{
			m_hSquadMember[i] = pAdd;
			pAdd->m_hSquadLeader = this;
			return TRUE;
		}
	}
	return FALSE;
	// should complain here
}


//=========================================================
// 
// SquadPasteEnemyInfo - called by squad members that have
// current info on the enemy so that it can be stored for 
// members who don't have current info.
//
//=========================================================
void CTalkSquadMonster::SquadPasteEnemyInfo(void)
{
	CTalkSquadMonster* pSquadLeader = MySquadLeader();
	if (pSquadLeader)
		pSquadLeader->m_vecEnemyLKP = m_vecEnemyLKP;
}

//=========================================================
//
// SquadCopyEnemyInfo - called by squad members who don't
// have current info on the enemy. Reads from the same fields
// in the leader's data that other squad members write to,
// so the most recent data is always available here.
//
//=========================================================
void CTalkSquadMonster::SquadCopyEnemyInfo(void)
{
	CTalkSquadMonster* pSquadLeader = MySquadLeader();
	if (pSquadLeader)
		m_vecEnemyLKP = pSquadLeader->m_vecEnemyLKP;
}

//=========================================================
// 
// SquadMakeEnemy - makes everyone in the squad angry at
// the same entity.
//
//=========================================================
void CTalkSquadMonster::SquadMakeEnemy(CBaseEntity* pEnemy)
{
	if (!InSquad())
		return;

	if (!pEnemy)
	{
		ALERT(at_console, "ERROR: SquadMakeEnemy() - pEnemy is NULL!\n");
		return;
	}

	CTalkSquadMonster* pSquadLeader = MySquadLeader();
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CTalkSquadMonster* pMember = pSquadLeader->MySquadMember(i);
		if (pMember)
		{
			// reset members who aren't activly engaged in fighting
			if (pMember->m_hEnemy != pEnemy && !pMember->HasConditions(bits_COND_SEE_ENEMY))
			{
				if (pMember->m_hEnemy != NULL)
				{
					// remember their current enemy
					pMember->PushEnemy(pMember->m_hEnemy, pMember->m_vecEnemyLKP);
				}
				// give them a new enemy
				pMember->m_hEnemy = pEnemy;
				pMember->m_vecEnemyLKP = pEnemy->pev->origin;
				pMember->SetConditions(bits_COND_NEW_ENEMY);
			}
		}
	}
}


//=========================================================
//
// SquadCount(), return the number of members of this squad
// callable from leaders & followers
//
//=========================================================
int CTalkSquadMonster::SquadCount(void)
{
	if (!InSquad())
		return 0;

	CTalkSquadMonster* pSquadLeader = MySquadLeader();
	int squadCount = 0;
	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		if (pSquadLeader->MySquadMember(i) != NULL)
			squadCount++;
	}

	return squadCount;
}


//=========================================================
//
// SquadRecruit(), get some monsters of my classification and
// link them as a group.  returns the group size
//
//=========================================================
int CTalkSquadMonster::SquadRecruit(int searchRadius, int maxMembers)
{
	int squadCount;
	int iMyClass = Classify();// cache this monster's class


	// Don't recruit if I'm already in a group
	if (InSquad())
		return 0;

	if (maxMembers < 2)
		return 0;

	// I am my own leader
	m_hSquadLeader = this;
	squadCount = 1;

	CBaseEntity* pEntity = NULL;

	if (!FStringNull(pev->netname))
	{
		// I have a netname, so unconditionally recruit everyone else with that name.
		pEntity = UTIL_FindEntityByString(pEntity, "netname", STRING(pev->netname));
		while (pEntity)
		{
			CTalkSquadMonster* pRecruit = pEntity->MyTalkSquadMonsterPointer();

			if (pRecruit)
			{
				if (!pRecruit->InSquad() && pRecruit->Classify() == iMyClass && pRecruit != this)
				{
					// minimum protection here against user error.in worldcraft. 
					if (!SquadAdd(pRecruit))
						break;
					squadCount++;
				}
			}

			pEntity = UTIL_FindEntityByString(pEntity, "netname", STRING(pev->netname));
		}
	}
	else
	{
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, searchRadius)) != NULL)
		{
			CTalkSquadMonster* pRecruit = pEntity->MyTalkSquadMonsterPointer();
			//if (pRecruit) {
			//	ALERT(at_console, "Potential squad member!\n");
			//}

			if (pRecruit && pRecruit != this && pRecruit->IsAlive() && !pRecruit->m_hCine)
			{
				// Can we recruit this guy?
				if (!pRecruit->InSquad() && pRecruit->Classify() == iMyClass &&
					((iMyClass != CLASS_ALIEN_MONSTER) || FStrEq(STRING(pev->classname), STRING(pRecruit->pev->classname))) &&
					FStringNull(pRecruit->pev->netname))
				{
					TraceResult tr;
					UTIL_TraceLine(pev->origin + pev->view_ofs, pRecruit->pev->origin + pev->view_ofs, ignore_monsters, pRecruit->edict(), &tr);// try to hit recruit with a traceline.
					if (tr.flFraction == 1.0)
					{
						if (!SquadAdd(pRecruit))
							break;

						squadCount++;
					}
				}
			}
		}
	}

	// no single member squads
	if (squadCount == 1)
	{
		m_hSquadLeader = NULL;
	}

	return squadCount;
}

//=========================================================
// CheckEnemy
//=========================================================
int CTalkSquadMonster::CheckEnemy(CBaseEntity* pEnemy)
{
	int iUpdatedLKP;

	iUpdatedLKP = CBaseMonster::CheckEnemy(m_hEnemy);

	// communicate with squad members about the enemy IF this individual has the same enemy as the squad leader.
	if (InSquad() && (CBaseEntity*)m_hEnemy == MySquadLeader()->m_hEnemy)
	{
		if (iUpdatedLKP)
		{
			// have new enemy information, so paste to the squad.
			SquadPasteEnemyInfo();
		}
		else
		{
			// enemy unseen, copy from the squad knowledge.
			SquadCopyEnemyInfo();
		}
	}

	return iUpdatedLKP;
}

//=========================================================
// StartMonster
//=========================================================
void CTalkSquadMonster::StartMonster(void)
{
	CBaseMonster::StartMonster();

	if ((m_afCapability & bits_CAP_SQUAD) && !InSquad())
	{
		if (!FStringNull(pev->netname))
		{
			// if I have a groupname, I can only recruit if I'm flagged as leader
			if (!(pev->spawnflags & SF_SQUADMONSTER_LEADER))
			{
				return;
			}
		}

		// try to form squads now.
		int iSquadSize = SquadRecruit(1024, 4);

		if (iSquadSize)
		{
			ALERT(at_aiconsole, "Squad of %d %s formed\n", iSquadSize, STRING(pev->classname));
		}
	}
}

//=========================================================
// NoFriendlyFire - checks for possibility of friendly fire
//
// Builds a large box in front of the grunt and checks to see 
// if any squad members are in that box. 
//=========================================================
BOOL CTalkSquadMonster::NoFriendlyFire(void)
{
	if (!InSquad())
	{
		return TRUE;
	}

	return CBaseMonster::NoFriendlyFire();
}

//=========================================================
// GetIdealState - surveys the Conditions information available
// and finds the best new state for a monster.
//=========================================================
MONSTERSTATE CTalkSquadMonster::GetIdealState(void)
{
	// If no schedule conditions, the new ideal state is probably the reason we're in here.
	switch (m_MonsterState)
	{
	case MONSTERSTATE_IDLE:
	case MONSTERSTATE_ALERT:
		if (HasConditions(bits_COND_NEW_ENEMY) && InSquad())
		{
			SquadMakeEnemy(m_hEnemy);
		}
		break;
	default:
		break;
	}

	return CBaseMonster::GetIdealState();
}

//=========================================================
// FValidateCover - determines whether or not the chosen
// cover location is a good one to move to. (currently based
// on proximity to others in the squad)
//=========================================================
BOOL CTalkSquadMonster::FValidateCover(const Vector& vecCoverLocation)
{
	if (!InSquad())
	{
		return TRUE;
	}

	if (SquadMemberInRange(vecCoverLocation, 128))
	{
		// another squad member is too close to this piece of cover.
		return FALSE;
	}

	return TRUE;
}

//=========================================================
// SquadEnemySplit- returns TRUE if not all squad members
// are fighting the same enemy. 
//=========================================================
BOOL CTalkSquadMonster::SquadEnemySplit(void)
{
	if (!InSquad())
		return FALSE;

	CTalkSquadMonster* pSquadLeader = MySquadLeader();
	CBaseEntity* pEnemy = pSquadLeader->m_hEnemy;

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CTalkSquadMonster* pMember = pSquadLeader->MySquadMember(i);
		if (pMember != NULL && pMember->m_hEnemy != NULL && pMember->m_hEnemy != pEnemy)
		{
			return TRUE;
		}
	}
	return FALSE;
}

//=========================================================
// FValidateCover - determines whether or not the chosen
// cover location is a good one to move to. (currently based
// on proximity to others in the squad)
//=========================================================
BOOL CTalkSquadMonster::SquadMemberInRange(const Vector& vecLocation, float flDist)
{
	if (!InSquad())
		return FALSE;

	CTalkSquadMonster* pSquadLeader = MySquadLeader();

	for (int i = 0; i < MAX_SQUAD_MEMBERS; i++)
	{
		CTalkSquadMonster* pSquadMember = pSquadLeader->MySquadMember(i);
		if (pSquadMember && (vecLocation - pSquadMember->pev->origin).Length2D() <= flDist)
			return TRUE;
	}
	return FALSE;
}




CTalkSquadMonster* CTalkSquadMonster::MySquadMedic()
{
	for (CBaseEntity* member : m_hSquadMember)
	{
		if (!member) {
			continue;
		}
		CTalkSquadMonster* pMember = member->MyTalkSquadMonsterPointer();

		if (pMember && FClassnameIs(pMember->pev, "monster_human_medic_ally"))
		{
			return pMember;
		}
	}

	return nullptr;
}

CTalkSquadMonster* CTalkSquadMonster::FindSquadMedic(int searchRadius)
{
	for (CBaseEntity* pEntity = nullptr; (pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, searchRadius)) != 0; )
	{
		CTalkSquadMonster* pMonster = pEntity->MyTalkSquadMonsterPointer();

		if (pMonster
			&& pMonster != this
			&& pMonster->IsAlive()
			&& !pMonster->m_hCine
			&& FClassnameIs(pMonster->pev, "monster_human_medic_ally"))
		{
			return pMonster;
		}
	}

	return nullptr;
}

BOOL CTalkSquadMonster::HealMe(CTalkSquadMonster* pTarget)
{
	return false;
}