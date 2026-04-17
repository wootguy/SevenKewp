#include "extdll.h"
#include "util.h"
#include "CCineMonster.h"
#include "CPathWaypoint.h"
#include "defaultai.h"

LINK_ENTITY_TO_CLASS(path_waypoint, CPathWaypoint)

void CPathWaypoint::KeyValue(KeyValueData* pkvd) {
	if (FStrEq(pkvd->szKeyName, "movementtype"))
	{
		m_fMoveTo = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flMoveToRadius = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "useangles"))
	{
		m_useAngles = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "trigger_on_arrival"))
	{
		m_triggerOnArrival = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "arrival_animation"))
	{
		m_arrivalAnim = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "trigger_after_arrival"))
	{
		m_triggerAfterArrivalAnim = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait_activity"))
	{
		m_waitActivity = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait_animation"))
	{
		m_waitAnim = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait_time"))
	{
		m_waitTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait_master"))
	{
		m_waitMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "departure_animation"))
	{
		m_departureAnim = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "trigger_on_departure"))
	{
		m_triggerOnDeparture = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "occupant_radius_check"))
	{
		m_occupantRadius = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "occupant_limit"))
	{
		m_occupantLimit = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	/*
	else if (FStrEq(pkvd->szKeyName, "waituntilfull"))
	{
		m_waitUntilFull = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "overflow_waypoint"))
	{
		m_overflowWaypoint = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "force_complete"))
	{
		m_forceCompletion = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "stop_trigger"))
	{
		m_triggerOnStop = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "restart_delay"))
	{
		m_restartDelay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	*/
	else
	{
		CBaseEntity::KeyValue(pkvd);
	}
}

void CPathWaypoint::Spawn() {
	pev->solid = SOLID_NOT;
	m_interruptable = false;
	pev->spawnflags |= SF_SCRIPT_REPEATABLE;
}

void CPathWaypoint::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	//EALERT(at_console, "Use waypoint\n", STRING(pev->targetname));
	
	CBaseMonster* pTarget = pCaller ? pCaller->MyMonsterPointer() : NULL;
	if (pTarget) {
		m_hTargetEnt = pCaller;
		CCineMonster::PossessEntity();
		pTarget->ChangeSchedule(GetScriptSchedule());
	}
}

Schedule_t* CPathWaypoint::GetScriptSchedule() {
	switch (m_fMoveTo)
	{
	case PWPT_WALK: return slWalkToPathWaypoint;
	case PWPT_RUN: return slRunToPathWaypoint;
	case PWPT_TELEPORT: return slWaitPathWaypoint;
	case PWPT_TURN_TO_FACE: return slWaitPathWaypoint;
	default:
	case PWPT_DONT_MOVE: return slWaitPathWaypoint;
	}
}

void CPathWaypoint::DoScript(CBaseMonster* pTarget) {
	if (pTarget->m_pathWaypointWaitForSeq && !pTarget->m_fSequenceFinished) {
		//ALERT(at_console, "Wait for sequence..\n");
		return;
	}
	pTarget->m_pathWaypointWaitForSeq = false;

	if (pTarget->m_pathWaypointState == PWPT_STATE_ARRIVE) {
		if (m_fMoveTo == PWPT_TELEPORT) {
			UTIL_SetOrigin(pTarget->pev, pev->origin);
			DROP_TO_FLOOR(ENT(pTarget->pev));
		}

		if (m_triggerOnArrival)
			FireTargets(STRING(m_triggerOnArrival), pTarget, pTarget, USE_TOGGLE);
		
		pTarget->m_pathWaypointState = PWPT_STATE_FACE_IDEAL;

		if (m_arrivalAnim) {
			//ALERT(at_console, "Play arrival anim!\n");
			pTarget->m_pathWaypointWaitForSeq = true;
			CCineMonster::StartSequence(pTarget, m_arrivalAnim, TRUE);
			return;
		}
	}

	if (pTarget->m_pathWaypointState == PWPT_STATE_FACE_IDEAL) {
		if (m_useAngles) {
			pTarget->pev->ideal_yaw = pev->angles.y;
			pTarget->ChangeYaw(pTarget->pev->yaw_speed);
			//ALERT(at_console, "Face ideal...\n");
			if (!pTarget->FacingIdeal()) {
				return;
			}
		}

		pTarget->m_pathWaypointState = PWPT_STATE_WAIT;

		if (m_arrivalAnim && m_triggerAfterArrivalAnim)
			FireTargets(STRING(m_triggerAfterArrivalAnim), pTarget, pTarget, USE_TOGGLE);
	}	

	if (pTarget->m_pathWaypointState == PWPT_STATE_WAIT) {
		pTarget->m_pathWaypointState = PWPT_STATE_DEPART;

		if (m_waitTime) {
			m_startTime = gpGlobals->time + m_waitTime;
		}

		if (m_waitAnim && m_waitActivity == PWPT_WAIT_PLAY_ANIM) {
			//ALERT(at_console, "Play wait anim!\n");
			pTarget->m_pathWaypointWaitForSeq = true;
			CCineMonster::StartSequence(pTarget, m_waitAnim, TRUE);
			return;
		}
	}

	bool shouldWait = gpGlobals->time < m_startTime ||
		(m_waitMaster && !UTIL_IsMasterTriggered(m_waitMaster, this));

	if (shouldWait) {
		if (m_waitActivity == PWPT_LOOK_AROUND) {
			if (m_nextLookAround < gpGlobals->time) {
				m_nextLookAround = gpGlobals->time + 0.4f;
				pTarget->pev->ideal_yaw = RANDOM_FLOAT(-180, 180);
			}
			pTarget->ChangeYaw(pTarget->pev->yaw_speed);
		}

		//ALERT(at_console, "Waiting...\n");
		return;
	}

	if (pTarget->m_pathWaypointState == PWPT_STATE_DEPART) {
		pTarget->m_pathWaypointState = PWPT_STATE_NEXT_WAYPOINT;

		if (m_departureAnim) {
			//ALERT(at_console, "Play depart anim!\n");
			pTarget->m_pathWaypointWaitForSeq = true;
			CCineMonster::StartSequence(pTarget, m_departureAnim, TRUE);
			return;
		}
	}

	if (m_triggerOnDeparture)
		FireTargets(STRING(m_triggerOnDeparture), pTarget, pTarget, USE_TOGGLE);

	FinishRoute(pTarget);

	if (pev->target) {
		CBaseEntity* ent = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
		CPathWaypoint* waypoint = ent ? ent->MyPathWaypointPointer() : NULL;

		if (waypoint && !waypoint->IsFull(pTarget)) {
			pTarget->m_pathWaypoint = pev->target;
			waypoint->Use(pTarget, pTarget, USE_TOGGLE, 0);
		}
	}
}

void CPathWaypoint::FinishRoute(CBaseMonster* pMonster) {
	pMonster->m_pathWaypoint = 0;
	pMonster->m_pathWaypointState = PWPT_STATE_ARRIVE;
	pMonster->CineCleanup();
	FixScriptMonsterSchedule(pMonster);
}

bool CPathWaypoint::IsFull(CBaseMonster* checker) {
	int count = 0;

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_occupantRadius)) != NULL)
	{
		if (pEntity->IsNormalMonster() && pEntity != checker) {
			count++;
			if (count >= m_occupantLimit)
				return true;
		}
	}

	return false;
}