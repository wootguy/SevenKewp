#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"
#include "path/CPathCorner.h"

#define SF_CAMERA_PLAYER_POSITION	1 // camera position starts at the activator
#define SF_CAMERA_PLAYER_TARGET		2 // camera follows the activator
#define SF_CAMERA_PLAYER_TAKECONTROL 4 // freeze viewers
#define SF_CAMERA_ALL_PLAYERS 8 // force everyone to view, dead or alive
#define SF_CAMERA_FORCE_VIEW 16 // activator is forced to view even if dead ("all players" also does this)
#define SF_CAMERA_INSTANT_TURN 32 // if enabled, don't smoothly rotate to face target
#define SF_CAMERA_PLAYER_INVULNERABLE 256 // disable viewer damage while camera is active

class CTriggerCamera : public CBaseDelay
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT FollowTarget(void);
	void Move(void);
	void TogglePlayerViews(bool enabled);
	void TogglePlayerView(CBasePlayer* plr, bool enabled);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	virtual int	ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hPlayer;
	EHANDLE m_hTarget;
	CBaseEntity* m_pentPath;
	int	  m_sPath;
	float m_flWait;
	float m_flReturnTime;
	float m_flStopTime;
	float m_moveDistance;
	float m_targetSpeed;
	float m_initialSpeed;
	float m_acceleration;
	float m_deceleration;
	float m_turnspeed;
	int	  m_state;

};
LINK_ENTITY_TO_CLASS(trigger_camera, CTriggerCamera)

// Global Savedata for changelevel friction modifier
TYPEDESCRIPTION	CTriggerCamera::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerCamera, m_hPlayer, FIELD_EHANDLE),
	DEFINE_FIELD(CTriggerCamera, m_hTarget, FIELD_EHANDLE),
	DEFINE_FIELD(CTriggerCamera, m_pentPath, FIELD_CLASSPTR),
	DEFINE_FIELD(CTriggerCamera, m_sPath, FIELD_STRING),
	DEFINE_FIELD(CTriggerCamera, m_flWait, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_flReturnTime, FIELD_TIME),
	DEFINE_FIELD(CTriggerCamera, m_flStopTime, FIELD_TIME),
	DEFINE_FIELD(CTriggerCamera, m_moveDistance, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_targetSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_initialSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_acceleration, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_deceleration, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_state, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTriggerCamera, CBaseDelay)

void CTriggerCamera::Spawn(void)
{
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;							// Remove model & collisions
	pev->renderamt = 0;								// The engine won't draw this model if this is set to 0 and blending is on
	pev->rendermode = kRenderTransTexture;

	// views won't update if camera doesn't have a model, even if invisible
	SET_MODEL(ENT(pev), "models/player.mdl");

	m_initialSpeed = pev->speed;
	if (m_acceleration == 0)
		m_acceleration = 500;
	if (m_deceleration == 0)
		m_deceleration = 500;
	if (m_turnspeed == 0)
		m_turnspeed = 40;
}


void CTriggerCamera::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "moveto"))
	{
		m_sPath = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "deceleration"))
	{
		m_deceleration = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "turnspeed"))
	{
		m_turnspeed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerCamera::TogglePlayerView(CBasePlayer* plr, bool enabled) {
	if (pev->spawnflags & SF_CAMERA_PLAYER_TAKECONTROL) {
		plr->EnableControl(enabled ? FALSE : TRUE);
	}

	if (pev->spawnflags & SF_CAMERA_PLAYER_INVULNERABLE) {
		plr->pev->takedamage = enabled ? DAMAGE_NO : DAMAGE_YES;
	}

	if (!enabled && plr->m_hActiveCamera.GetEntity() != this) {
		return; // don't reset player's view if another camera interrupted this one
	}

	SET_VIEW(plr->edict(), enabled ? edict() : plr->edict());
	plr->m_hActiveCamera = enabled ? this : NULL;
}

void CTriggerCamera::TogglePlayerViews(bool enabled) {
	bool forceView = pev->spawnflags & SF_CAMERA_FORCE_VIEW;

	if (pev->spawnflags & SF_CAMERA_ALL_PLAYERS) {
		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			edict_t* ent = INDEXENT(i);
			CBasePlayer* plr = (CBasePlayer*)GET_PRIVATE(ent);

			if (!IsValidPlayer(ent) || !plr) {
				continue;
			}

			TogglePlayerView(plr, enabled);
		}
	}
	else if (IsValidPlayer(m_hPlayer.GetEdict())) {
		CBasePlayer* plr = (CBasePlayer*)m_hPlayer.GetEntity();

		if (enabled && !forceView && !plr->IsAlive()) {
			return;
		}

		TogglePlayerView(plr, enabled);
	}
}

void CTriggerCamera::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_state))
		return;

	// Toggle state
	m_state = !m_state;
	if (m_state == 0)
	{
		m_flReturnTime = gpGlobals->time;
		return;
	}

	m_hPlayer = pActivator;

	m_flReturnTime = gpGlobals->time + m_flWait;
	pev->speed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;

	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TARGET))
	{
		m_hTarget = m_hPlayer;
	}
	else
	{
		m_hTarget = GetNextTarget();
	}

	if (m_sPath)
	{
		m_pentPath = Instance(FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_sPath)));
	}
	else
	{
		m_pentPath = NULL;
	}

	m_flStopTime = gpGlobals->time;
	if (m_pentPath)
	{
		if (m_pentPath->pev->speed != 0)
			m_targetSpeed = m_pentPath->pev->speed;

		m_flStopTime += m_pentPath->GetDelay();
	}

	// copy over player information
	if (m_hPlayer && FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_POSITION))
	{
		UTIL_SetOrigin(pev, pActivator->pev->origin + pActivator->pev->view_ofs);
		pev->angles.x = -pActivator->pev->angles.x;
		pev->angles.y = pActivator->pev->angles.y;
		pev->angles.z = 0;
		pev->velocity = pActivator->pev->velocity;
	}
	else
	{
		pev->velocity = Vector(0, 0, 0);
	}

	TogglePlayerViews(true);

	// follow the player down
	SetThink(&CTriggerCamera::FollowTarget);
	pev->nextthink = gpGlobals->time;

	m_moveDistance = 0;
	Move();
}


void CTriggerCamera::FollowTarget()
{
	if (m_flReturnTime < gpGlobals->time)
	{
		TogglePlayerViews(false);

		// TODO: do sven cameras exclude certain ents or just never trigger their targets?
		//SUB_UseTargets(this, USE_TOGGLE, 0);
		
		pev->avelocity = Vector(0, 0, 0);
		m_state = 0;
		return;
	}

	if (m_hTarget == NULL) {
		pev->nextthink = gpGlobals->time;
		return;
	}

	Vector vecGoal = UTIL_VecToAngles(m_hTarget->pev->origin - pev->origin);
	vecGoal.x = -vecGoal.x;

	if (!(pev->spawnflags & SF_CAMERA_INSTANT_TURN)) {
		if (pev->angles.y > 360)
			pev->angles.y -= 360;

		if (pev->angles.y < 0)
			pev->angles.y += 360;

		float dx = vecGoal.x - pev->angles.x;
		float dy = vecGoal.y - pev->angles.y;

		if (dx < -180)
			dx += 360;
		if (dx > 180)
			dx = dx - 360;

		if (dy < -180)
			dy += 360;
		if (dy > 180)
			dy = dy - 360;

		pev->avelocity.x = dx * m_turnspeed * gpGlobals->frametime;
		pev->avelocity.y = dy * m_turnspeed * gpGlobals->frametime;
	}
	else {
		pev->angles = vecGoal;
	}

	pev->nextthink = gpGlobals->time;

	Move();
}

void CTriggerCamera::Move()
{
	// Not moving on a path, return
	if (!m_pentPath) {
		if (!(FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL)))
		{
			pev->velocity = pev->velocity * 0.8;
			if (pev->velocity.Length() < 10.0)
				pev->velocity = g_vecZero;
		}
		return;
	}

	// Subtract movement from the previous frame
	m_moveDistance -= pev->speed * gpGlobals->frametime;

	// Have we moved enough to reach the target?
	if (m_moveDistance <= 0)
	{
		// Fire the passtarget if there is one
		if (m_pentPath->pev->message)
		{
			FireTargets(STRING(m_pentPath->pev->message), this, this, USE_TOGGLE, 0);
			if (FBitSet(m_pentPath->pev->spawnflags, SF_CORNER_FIREONCE))
				m_pentPath->pev->message = 0;
		}
		// Time to go to the next target
		m_pentPath = m_pentPath->GetNextTarget();

		// Set up next corner
		if (!m_pentPath)
		{
			pev->velocity = g_vecZero;
		}
		else
		{
			if (m_pentPath->pev->speed != 0)
				m_targetSpeed = m_pentPath->pev->speed;

			Vector delta = m_pentPath->pev->origin - pev->origin;
			m_moveDistance = delta.Length();
			pev->movedir = delta.Normalize();
			m_flStopTime = gpGlobals->time + m_pentPath->GetDelay();
		}
	}

	if (m_flStopTime > gpGlobals->time)
		pev->speed = UTIL_Approach(0, pev->speed, m_deceleration * gpGlobals->frametime);
	else
		pev->speed = UTIL_Approach(m_targetSpeed, pev->speed, m_acceleration * gpGlobals->frametime);

	float fraction = 2 * gpGlobals->frametime;

	// the last bit carries over velocity from the previous frame, for spline-like movement.
	// This causes the camera to overshoot/undershoot its target because m_moveDistance
	// is intialized with a linear distance while the actual path travelled is curved.
	pev->velocity = ((pev->movedir * pev->speed) * fraction) + (pev->velocity * (1 - fraction));
}
