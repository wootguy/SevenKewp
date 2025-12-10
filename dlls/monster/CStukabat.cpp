#include	"extdll.h"
#include	"util.h"
#include	"monsters.h"
#include	"effects.h"
#include	"schedule.h"
#include	"weapons.h"
#include	"CStukabat.h"
#include	"hlds_hooks.h"

// TODO:
// - land on ground to feed on corpse
// - perch on ceiling if can't fly around, or as the default AI when idle

LINK_ENTITY_TO_CLASS( monster_stukabat, CStukabat)

TYPEDESCRIPTION	CStukabat::m_SaveData[] =
{
	DEFINE_FIELD(CStukabat, m_isDiving, FIELD_INTEGER ),
};
IMPLEMENT_SAVERESTORE(CStukabat, CBaseMonster )


const char * CStukabat::pAttackSounds[] =
{
	"controller/con_attack1.wav",
	"controller/con_attack2.wav",
	"controller/con_attack3.wav",
};

const char * CStukabat::pIdleSounds[] =
{
	"controller/con_idle1.wav",
	"controller/con_idle2.wav",
	"controller/con_idle3.wav",
	"controller/con_idle4.wav",
	"controller/con_idle5.wav",
};

const char * CStukabat::pAlertSounds[] =
{
	"controller/con_alert1.wav",
	"controller/con_alert2.wav",
	"controller/con_alert3.wav",
};

const char * CStukabat::pPainSounds[] =
{
	"controller/con_pain1.wav",
	"controller/con_pain2.wav",
	"controller/con_pain3.wav",
};

const char * CStukabat::pDeathSounds[] =
{
	"controller/con_die1.wav",
	"controller/con_die2.wav",
};

const char* CStukabat::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

enum
{
	TASK_GET_PATH_ABOVE_ENEMY = LAST_COMMON_TASK + 1,	// Move up and away from enemy for a dive attack
};

// Chase enemy schedule
Task_t tlStukabatRetreat[] =
{
	{ TASK_GET_PATH_ABOVE_ENEMY,(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_SET_ACTIVITY,		ACT_FLY			},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0		},
};

Schedule_t slStukabatRetreat[] =
{
	{
		tlStukabatRetreat,
		ARRAYSIZE(tlStukabatRetreat),
		bits_COND_HEAVY_DAMAGE |
		bits_COND_TASK_FAILED,
		0,
		"STUKABAT_RETREAT"
	},
};

Task_t	tlStukabatDiveAttack[] =
{
	{ TASK_SET_FAIL_SCHEDULE,		SCHED_CHASE_ENEMY			},
	{ TASK_GET_PATH_TO_ENEMY,		(float)128					},
	{ TASK_RANGE_ATTACK1,			0							},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_SET_ACTIVITY,			ACT_HOVER					},
	{ TASK_WAIT_FACE_ENEMY,			0.5							},
};

Schedule_t	slStukabatDiveAttack[] =
{
	{
		tlStukabatDiveAttack,
		ARRAYSIZE(tlStukabatDiveAttack),
		bits_COND_HEAVY_DAMAGE,
		0,
		"STUKABAT_DIVE_ATTACK"
	},
};

Task_t	tlStukabatFlinch[] =
{
	{ TASK_REMEMBER,			(float)bits_MEMORY_FLINCHED },
	{ TASK_SMALL_FLINCH,		0							},
};

Schedule_t	slStukabatFlinch[] =
{
	{
		tlStukabatFlinch,
		ARRAYSIZE(tlStukabatFlinch),
		bits_COND_HEAVY_DAMAGE,
		0,
		"STUKABAT_FLINCH"
	},
};

Task_t	tlStukabatFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_HOVER },
	{ TASK_WAIT,				(float)0.5f		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slStukabatFail[] =
{
	{
		tlStukabatFail,
		ARRAYSIZE(tlStukabatFail),
		0,
		0,
		"STUKABAT_FAIL"
	},
};



DEFINE_CUSTOM_SCHEDULES(CStukabat)
{
	slStukabatRetreat,
	slStukabatDiveAttack,
	slStukabatFlinch,
	slStukabatFail,
};

IMPLEMENT_CUSTOM_SCHEDULES(CStukabat, CBaseMonster)

const char* CStukabat::GetTaskName(int taskIdx) {
	switch (taskIdx) {
	case TASK_GET_PATH_ABOVE_ENEMY: return "TASK_GET_PATH_ABOVE_ENEMY";
	default:
		return CBaseMonster::GetTaskName(taskIdx);
	}
}

int	CStukabat::Classify ( void )
{
	return CBaseMonster::Classify(CLASS_ALIEN_MONSTER);
}

const char* CStukabat::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Stukabat";
}

void CStukabat:: SetYawSpeed ( void )
{
	pev->yaw_speed = 120 * gSkillData.sk_yawspeed_mult;
}

void CStukabat:: PainSound( void )
{
	if (RANDOM_LONG(0, 3) < 2) {
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_SOUND_ARRAY_IDX(pPainSounds)], 1.0, ATTN_NORM, 0, RANDOM_LONG(150, 160));
	}
}	

void CStukabat:: AlertSound( void )
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAlertSounds[RANDOM_SOUND_ARRAY_IDX(pAlertSounds)], 1.0, ATTN_NORM, 0, RANDOM_LONG(150, 160));
}

void CStukabat:: IdleSound( void )
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pIdleSounds[RANDOM_SOUND_ARRAY_IDX(pIdleSounds)], 1.0, ATTN_NORM, 0, RANDOM_LONG(150, 160));
}

void CStukabat:: AttackSound( void )
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_SOUND_ARRAY_IDX(pAttackSounds)], 1.0, ATTN_NORM, 0, RANDOM_LONG(150, 160));
	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_GUN_VOLUME, 0.3);
}

void CStukabat:: DeathSound( void )
{
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pDeathSounds[RANDOM_SOUND_ARRAY_IDX(pDeathSounds)], 1.0, ATTN_NORM, 0, RANDOM_LONG(150, 160));
}

void CStukabat::StartFollowingSound() {
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pAttackSounds[RANDOM_SOUND_ARRAY_IDX(pAttackSounds)], 1.0, ATTN_NORM, 0, RANDOM_LONG(150, 160));
}

void CStukabat::StopFollowingSound() {
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_SOUND_ARRAY_IDX(pPainSounds)], 1.0, ATTN_NORM, 0, RANDOM_LONG(150, 160));

}

void CStukabat::CantFollowSound() {
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_SOUND_ARRAY_IDX(pPainSounds)], 1.0, ATTN_NORM, 0, RANDOM_LONG(150, 160));
}

void CStukabat::HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case STUKABAT_AE_FLAP:
		EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, STUKABAT_FLAP_SOUND, 1.0, ATTN_STATIC, 0, RANDOM_LONG(120, 130));
		break;
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

void CStukabat::Spawn()
{
	Precache( );

	InitModel();
	SetSize(Vector( -32, -32, 0 ), Vector( 32, 32, 32 ));

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_FLY;
	pev->flags			|= FL_FLY;
	m_bloodColor		= BloodColorAlien();
	pev->view_ofs		= Vector( 0, 0, -2 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= VIEW_FIELD_WIDE;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();
}

void CStukabat::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/stukabat.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_SOUND(STUKABAT_FLAP_SOUND);

	PRECACHE_SOUND_ARRAY(pAttackSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDeathSounds);
	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
}	

void CStukabat::DiveTouch(CBaseEntity* pOther)
{
	Vector start = BodyTarget(pev->origin);
	Vector end = pOther->BodyTarget(pev->origin);
	Vector dir = (end - start).Normalize();
	TraceResult tr;
	UTIL_TraceLine(start, end, dont_ignore_monsters, edict(), &tr);

	ClearMultiDamage();
	pOther->TraceAttack(pev, gSkillData.sk_stukabat_dmg_bite, dir, &tr, DMG_CLUB);
	ApplyMultiDamage(pev, pev);
	pOther->pev->punchangle.x = 18;

	EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));

	m_isDiving = 0;
	m_velocity = g_vecZero;
	m_flGroundSpeed = 1; // not 0 to prevent nan origin
	TaskComplete();

	SetTouch(NULL);
}

void CStukabat::StartTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_DIE:
		CBaseMonster::StartTask(pTask);
		SetActivity(ACT_FALL);
		pev->angles.x = 0;
		pev->movetype = MOVETYPE_TOSS;
		pev->velocity = m_velocity;
		break;
	case TASK_RANGE_ATTACK1:
		m_isDiving = 2;
		SetTouch(&CStukabat::DiveTouch);
		TaskComplete();
		AttackSound();
		break;
	case TASK_SMALL_FLINCH:
		CBaseMonster::StartTask(pTask);
		m_movementActivity = ACT_SMALL_FLINCH;
		SetTouch(NULL);
		
		// always play sound when flinched
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_SOUND_ARRAY_IDX(pPainSounds)], 1.0, ATTN_NORM, 0, RANDOM_LONG(150, 160));
		break;
	case TASK_GET_PATH_ABOVE_ENEMY:
	{
		if (!m_hEnemy) {
			TaskFail();
			break;
		}

		int oldSolid = pev->solid;
		pev->solid = SOLID_NOT; // ignore self for traces

		float bestDist = 0;
		Vector bestPos = g_vecZero;
		bool foundPos = false;
		float step = 20;
		float angle = RANDOM_FLOAT(0,360);

		float vertAngles[4] = { -45, 0, 10, 45 };

		for (int k = 0; k < 4 && !foundPos; k++) {
			angle = RANDOM_FLOAT(0, 360);

			for (int i = 0; i < 360.0f / step; i++) {
				angle += step;
				Vector angles = Vector(vertAngles[k], angle, 0);
				MAKE_VECTORS(angles);

				TraceResult tr;
				TRACE_MONSTER_HULL(edict(), pev->origin, pev->origin + gpGlobals->v_forward * 1024, dont_ignore_monsters, m_hEnemy.GetEdict(), &tr);

				float dist = (tr.vecEndPos - pev->origin).Length();

				if (dist < 256) {
					//te_debug_beam(enemyOri, tr.vecEndPos, 10, RGBA(255, 0, 0));
					continue; // too close to current position
				}

				Vector flyPos = tr.vecEndPos - gpGlobals->v_forward * 32; // stay a bit away from walls

				if (dist > bestDist) {
					bestDist = dist;
					bestPos = flyPos;
					foundPos = true;
				}
				//te_debug_beam(pev->origin, flyPos, 10, RGBA(0, 255, 0));
			}
		}

		pev->solid = oldSolid;
		m_retreatPos = bestPos;

		if (!foundPos) {
			TaskFail();
			break;
		}

		AttackSound();

		if (BuildRoute(bestPos, bits_MF_TO_LOCATION, NULL, true))
		{
			TaskComplete();
		}
		else if (BuildNearestRoute(bestPos, pev->view_ofs, 0, (bestPos - pev->origin).Length()))
		{
			TaskComplete();
		}
		else
		{
			// no way to get there =(
			ALERT(at_aiconsole, "TASK_GET_PATH_ABOVE_ENEMY failed!!\n");
			TaskFail();
		}
		break;
	}
	default:
		CBaseMonster :: StartTask ( pTask );
		break;
	}
}

void CStukabat::RunTask ( Task_t *pTask )
{
	const int dive_max_speed = 1000; // terminal velocity
	const int dive_fly_speed = 400; // speed when flapping towards target
	const int circle_fly_speed = 300; // speed when retreating or roaming

	switch ( pTask->iTask )
	{
	case TASK_DIE:
		if (pev->flags & FL_ONGROUND) {
			pev->angles.x = 0;
			if (m_Activity != ACT_DIESIMPLE && m_Activity != ACT_DIEVIOLENT) {
				if (RANDOM_LONG(0, 2) == 0) {
					SetActivity(ACT_DIEVIOLENT);
				}
				else {
					SetActivity(ACT_DIESIMPLE);
				}
				pev->framerate = 1.5f;
			}
			CBaseMonster::RunTask(pTask);
		}
		else {
			Vector angles = UTIL_VecToAngles(pev->velocity);
			pev->angles.x = angles.x + 60;
			pev->velocity = pev->velocity * 0.95f; // wings slow descent
		}
		break;
	case TASK_WAIT_FACE_ENEMY:
		m_isDiving = 0;
		m_IdealActivity = ACT_HOVER;
		m_movementActivity = ACT_HOVER;
		m_flGroundSpeed = 0;
		CBaseMonster::RunTask(pTask);
		break;
	case TASK_SMALL_FLINCH:
		// big flinch animation cut short so it doesn't freeze waiting for the schedule to change
		if (m_fSequenceFinished || (pev->sequence == 8 && pev->frame > 200)) {
			TaskComplete();
			m_flGroundSpeed = 100;
			m_velocity = g_vecZero;
			m_isDiving = 0;
		}
		break;
	case TASK_WAIT_FOR_MOVEMENT:
	case TASK_WAIT:
	case TASK_MOVE_TO_TARGET_RANGE:
	case TASK_WAIT_PVS:
	case TASK_WALK_PATH_FOR_UNITS:
	{
		if (m_hTargetEnt && IsFollowing() && (m_MonsterState == MONSTERSTATE_IDLE || m_MonsterState == MONSTERSTATE_ALERT)) {
			MakeIdealYaw(m_hTargetEnt->pev->origin);
		}
		else if (m_velocity.Length()) {
			MakeIdealYaw(pev->origin + m_velocity);
		}

		ChangeYaw(pev->yaw_speed);

		if (m_isDiving) {
			MAKE_VECTORS(pev->angles);

			if (gpGlobals->v_forward.z > 0.7f || m_velocity.Length() > dive_max_speed-100) {
				m_isDiving = 1;
			}
			else {
				m_isDiving = 2;
			}
		}

		if (m_fSequenceFinished)
		{
			pev->framerate = 1.0f;
		}

		CBaseMonster::RunTask(pTask);

		if (m_Activity != m_IdealActivity)
		{
			SetActivity(m_IdealActivity);
			if (m_Activity == ACT_SMALL_FLINCH) {
				m_isDiving = 0;
			}
		}

		pev->framerate = 1.0f;

		if (m_isDiving) {
			m_flGroundSpeed = V_min(dive_max_speed, V_max(dive_fly_speed, m_flGroundSpeed + 100));

			if (m_isDiving == 1) {
				m_movementActivity = ACT_RANGE_ATTACK2;
			}
			else if (HasConditions(bits_COND_CAN_MELEE_ATTACK1)) {
				if (m_Activity == ACT_MELEE_ATTACK1 && m_fSequenceFinished) {
					TaskFail();
				}
				m_movementActivity = ACT_MELEE_ATTACK1;
			}
			else {
				m_movementActivity = ACT_FLY;
			}
		}
		else {
			if (m_velocity.Length() > 50) {
				m_movementActivity = ACT_FLY;
				pev->framerate = 1.0f;
			}
			else {
				m_movementActivity = ACT_HOVER;
			}
			m_flGroundSpeed = V_min(m_flGroundSpeed + 100, circle_fly_speed);
		}

		if (m_Activity == ACT_FLY) {
			pev->framerate = m_isDiving ? 2.0f : 1.5f;
		}

		break;
	}
	default: 
		CBaseMonster :: RunTask ( pTask );
		break;
	}
}

Schedule_t * CStukabat::GetSchedule ( void )
{
	switch	( m_MonsterState )
	{
	case MONSTERSTATE_IDLE:
		break;
	case MONSTERSTATE_ALERT:
		break;
	case MONSTERSTATE_COMBAT:
		break;
	default:
		break;
	}

	if (HasConditions(bits_COND_HEAVY_DAMAGE)) {
		return GetScheduleOfType(SCHED_SMALL_FLINCH);
	}

	return CBaseMonster :: GetSchedule();
}

Schedule_t* CStukabat::GetScheduleOfType ( int Type )
{
	// ALERT( at_console, "%d\n", m_iFrustration );
	switch	( Type )
	{
	case SCHED_CHASE_ENEMY:
	case SCHED_MELEE_ATTACK1:
		return slStukabatRetreat;
	case SCHED_COMBAT_FACE:
	case SCHED_RANGE_ATTACK1:
		return slStukabatDiveAttack;
	case SCHED_SMALL_FLINCH:
		return slStukabatFlinch;
	case SCHED_FAIL:
		return slStukabatFail;
	}

	return CBaseMonster :: GetScheduleOfType( Type );
}

BOOL CStukabat::CheckRangeAttack1 ( float flDot, float flDist )
{
	if (flDist > 256 && flDist <= 4096 )
	{
		return TRUE;
	}
	return FALSE;
}

BOOL CStukabat::CheckRangeAttack2 ( float flDot, float flDist )
{
	return FALSE;
}

BOOL CStukabat:: CheckMeleeAttack1 ( float flDot, float flDist )
{
	return m_isDiving == 2 && flDist < 350;
}

void CStukabat:: SetActivity ( Activity NewActivity )
{
	CBaseMonster::SetActivity( NewActivity );


}

int CStukabat::LookupActivity(int activity)
{
	ASSERT(activity != 0);
	void* pmodel = GET_MODEL_PTR(ENT(pev));

	switch (activity) {
	case ACT_MELEE_ATTACK1:
		return ::LookupActivityWithOffset(pmodel, pev, ACT_RANGE_ATTACK2, 0);
	case ACT_RANGE_ATTACK2:
		return ::LookupActivityWithOffset(pmodel, pev, ACT_RANGE_ATTACK2, 1);
	case ACT_DIESIMPLE:
		return ::LookupActivityWithOffset(pmodel, pev, ACT_DIESIMPLE, 0);
	case ACT_IDLE:
	case ACT_WALK:
		return ::LookupActivity(pmodel, pev, ACT_HOVER);
	case ACT_SMALL_FLINCH:
		return ::LookupActivity(pmodel, pev, m_isDiving ? ACT_BIG_FLINCH : ACT_SMALL_FLINCH);
	default:
		return ::LookupActivity(pmodel, pev, activity);
	}
}

void CStukabat::RunAI( void )
{
	CBaseMonster :: RunAI();
	Vector vecStart, angleGun;

	if (IsAlive()) {
		if (m_Activity != ACT_HOVER && m_Activity != ACT_IDLE) {
			Vector angles = UTIL_VecToAngles(m_velocity.Normalize());
			pev->angles.x = angles.x;
		}
		else {
			pev->angles.x = 0;
		}
	}

	if ( HasMemory( bits_MEMORY_KILLED ) )
		return;
}

extern void DrawRoute( entvars_t *pev, WayPoint_t *m_Route, int m_iRouteIndex, int r, int g, int b );

void CStukabat::Stop( void )
{ 
	m_IdealActivity = GetStoppedActivity(); 
}

#define DIST_TO_CHECK	200
void CStukabat:: Move ( float flInterval )
{
	float		flWaypointDist;
	float		flCheckDist;
	float		flDist;// how far the lookahead check got before hitting an object.
	float		flMoveDist;
	Vector		vecDir;
	Vector		vecApex;
	CBaseEntity	*pTargetEnt;

	// Don't move if no valid route
	if ( FRouteClear() )
	{
		ALERT( at_aiconsole, "Tried to move with no route!\n" );
		TaskFail();
		return;
	}
	
	if ( m_flMoveWaitFinished > gpGlobals->time )
		return;

	// if the monster is moving directly towards an entity (enemy for instance), we'll set this pointer
	// to that entity for the CheckLocalMove and Triangulate functions.
	pTargetEnt = NULL;

	if (m_flGroundSpeed == 0)
	{
		m_flGroundSpeed = 200;
		// TaskFail( );
		// return;
	}

	flMoveDist = m_flGroundSpeed * flInterval;

	do 
	{
		if ((m_Route[m_iRouteIndex].iType & (~bits_MF_NOT_TO_MASK)) == bits_MF_TO_ENEMY)
		{
			// only on a PURE move to enemy ( i.e., ONLY MF_TO_ENEMY set, not MF_TO_ENEMY and DETOUR )
			pTargetEnt = m_hEnemy;
			
			// aim for enemy's future position since divebomb attack cannot instantly change direction
			if (m_hEnemy) {
				m_Route[m_iRouteIndex].vecLocation = m_hEnemy->Center()
					+ m_hEnemy->pev->velocity * 0.25f;
			}
		}
		else if ((m_Route[m_iRouteIndex].iType & ~bits_MF_NOT_TO_MASK) == bits_MF_TO_TARGETENT)
		{
			pTargetEnt = m_hTargetEnt;
		}

		// local move to waypoint.
		vecDir = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Normalize();
		flWaypointDist = ( m_Route[ m_iRouteIndex ].vecLocation - pev->origin ).Length();
		
		// MakeIdealYaw ( m_Route[ m_iRouteIndex ].vecLocation );
		// ChangeYaw ( pev->yaw_speed );

		// if the waypoint is closer than CheckDist, CheckDist is the dist to waypoint
		if ( flWaypointDist < DIST_TO_CHECK )
		{
			flCheckDist = flWaypointDist;
		}
		else
		{
			flCheckDist = DIST_TO_CHECK;
		}

		// !!!BUGBUG - CheckDist should be derived from ground speed.
		// If this fails, it should be because of some dynamic entity blocking this guy.
		// We've already checked this path, so we should wait and time out if the entity doesn't move
		flDist = 0;
		if ( CheckLocalMove ( pev->origin, pev->origin + vecDir * flCheckDist, pTargetEnt, &flDist ) != LOCALMOVE_VALID )
		{
			CBaseEntity *pBlocker;

			// Blocking entity is in global trace_ent
			pBlocker = CBaseEntity::Instance( gpGlobals->trace_ent );
			bool isExpectedBlock = m_isDiving && pBlocker->IsMonster();

			if (!isExpectedBlock) {
				if (pBlocker) {
					DispatchBlocked(edict(), pBlocker->edict());
				}

				// Can't move, stop
				Stop();

				if (!m_isDiving && pBlocker && m_moveWaitTime > 0 && pBlocker->IsMoving() && !pBlocker->IsPlayer() && (gpGlobals->time - m_flMoveWaitFinished) > 3.0)
				{
					// Can we still move toward our target?
					if (flDist < m_flGroundSpeed)
					{
						// Wait for a second
						m_flMoveWaitFinished = gpGlobals->time + m_moveWaitTime;
						//				ALERT( at_aiconsole, "Move %s!!!\n", STRING( pBlocker->pev->classname ) );
						return;
					}
				}
				else
				{
					// try to triangulate around whatever is in the way.
					if (FTriangulate(pev->origin, m_Route[m_iRouteIndex].vecLocation, flDist, pTargetEnt, &vecApex))
					{
						InsertWaypoint(vecApex, bits_MF_TO_DETOUR);
						RouteSimplify(pTargetEnt);
					}
					else if (!m_isDiving)
					{
						ALERT(at_aiconsole, "Couldn't Triangulate\n");
						Stop();
						if (m_moveWaitTime > 0)
						{
							FRefreshRoute();
							m_flMoveWaitFinished = gpGlobals->time + m_moveWaitTime * 0.5;
						}
						else
						{
							TaskFail();
							ALERT(at_aiconsole, "Failed to move!\n");
							//ALERT( at_aiconsole, "%f, %f, %f\n", pev->origin.z, (pev->origin + (vecDir * flCheckDist)).z, m_Route[m_iRouteIndex].vecLocation.z );
						}
						return;
					}
				}
			}
		}

		// Debug, test movement code
		//DrawRoute(pev, m_Route, m_iRouteIndex, 0, 0, 255);

		// UNDONE: this is a hack to quit moving farther than it has looked ahead.
		if (flCheckDist < flMoveDist)
		{
			MoveExecute( pTargetEnt, vecDir, flCheckDist / m_flGroundSpeed );

			// ALERT( at_console, "%.02f\n", flInterval );
			AdvanceRoute( flWaypointDist );
			flMoveDist -= flCheckDist;
		}
		else
		{
			MoveExecute( pTargetEnt, vecDir, flMoveDist / m_flGroundSpeed );

			if ( ShouldAdvanceRoute( flWaypointDist - flMoveDist ) )
			{
				AdvanceRoute( flWaypointDist );
			}

			flMoveDist = 0;
		}

		if ( MovementIsComplete() )
		{
			Stop();
			RouteClear();
		}
	} while (flMoveDist > 0 && flCheckDist > 0);

	if (m_movementGoal == MOVEGOAL_LOCATION) {
		// reached retreat position
		if (flWaypointDist < 32)
			RouteClear();
	}
	else if (flWaypointDist < 128) // cut corner?
	{
		if ( m_movementGoal == MOVEGOAL_ENEMY )
			RouteSimplify( m_hEnemy );
		else
			RouteSimplify( m_hTargetEnt );
		FRefreshRoute();

		if (m_flGroundSpeed > 200)
			m_flGroundSpeed -= 40;
	}
	else
	{
		if (m_flGroundSpeed < 400)
			m_flGroundSpeed += 10;
	}
}

BOOL CStukabat::ShouldAdvanceRoute( float flWaypointDist )
{
	if ( flWaypointDist <= 32  )
	{
		return TRUE;
	}

	return FALSE;
}

int CStukabat::CheckLocalMove ( const Vector &vecStart, const Vector &vecEnd, CBaseEntity *pTarget, float *pflDist )
{
	TraceResult tr;

	UTIL_TraceHull( vecStart + Vector( 0, 0, 32), vecEnd + Vector( 0, 0, 32), dont_ignore_monsters, large_hull, edict(), &tr );

	// ALERT( at_console, "%.0f %.0f %.0f : ", vecStart.x, vecStart.y, vecStart.z );
	// ALERT( at_console, "%.0f %.0f %.0f\n", vecEnd.x, vecEnd.y, vecEnd.z );

	if (pflDist)
	{
		*pflDist = ( (tr.vecEndPos - Vector( 0, 0, 32 )) - vecStart ).Length();// get the distance.
	}

	// ALERT( at_console, "check %d %d %f\n", tr.fStartSolid, tr.fAllSolid, tr.flFraction );
	if (tr.fStartSolid || tr.flFraction < 1.0)
	{
		if ( pTarget && pTarget->edict() == gpGlobals->trace_ent )
			return LOCALMOVE_VALID;
		return LOCALMOVE_INVALID;
	}

	return LOCALMOVE_VALID;
}

void CStukabat::MoveExecute( CBaseEntity *pTargetEnt, const Vector &vecDir, float flInterval )
{
	if ( m_IdealActivity != m_movementActivity )
		m_IdealActivity = m_movementActivity;

	//ALERT( at_console, "move %.4f %.4f %.4f : %f\n", m_velocity.x, m_velocity.y, m_velocity.z, flInterval );

	if (m_isDiving) {
		m_velocity = m_velocity * 0.4 + m_flGroundSpeed * vecDir * 0.5;
	}
	else {
		m_velocity = m_velocity * 0.8 + m_flGroundSpeed * vecDir * 0.2;
	}
	
	Vector expectedOri = pev->origin + (m_velocity * flInterval);

	UTIL_MoveToOrigin ( ENT(pev), pev->origin + m_velocity, m_velocity.Length() * flInterval, MOVE_STRAFE );

	// if the move failed then it ran into something
	if (m_isDiving && (pev->origin - expectedOri).Length() > 1) {
		TraceResult tr;
		TRACE_MONSTER_HULL(edict(), pev->origin, expectedOri, dont_ignore_monsters, edict(), &tr);

		// move as far close to the target position as possible
		Vector delta = tr.vecEndPos - pev->origin;
		UTIL_MoveToOrigin(ENT(pev), tr.vecEndPos, delta.Length() * 0.99f, MOVE_STRAFE);

		// manually touch the blocking entity because MoveToOrigin doesn't trigger touch events
		DispatchTouch(edict(), tr.pHit);
	}
}

int CStukabat::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) {
	if (flDamage >= 20) {
		// stukabats are delicate. Any caliber heavier than 9mm will flinch it.
		// Without this you'll never see them flinch because their health is so low.
		SetConditions(bits_COND_HEAVY_DAMAGE);
	}

	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}