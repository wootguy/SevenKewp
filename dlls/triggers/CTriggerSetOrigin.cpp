#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "cbase.h"

//
// CTriggerSetOrigin / trigger_setorigin -- copies position and orientation of one entity to another

#define SF_CONSTANT					1	// Always update the target's position and orientation.
#define SF_SET_ONCE					4	// Only update the target once then remove the trigger_setorigin.

// Use the vector between the target and the copypointer plus the "Offset"-keyvalue as the offset vector.
// Also implies copy of X/Y/Z axes. TODO: X/Y/Z angles as well?
#define SF_LOCK_OFFSETS				8

#define SF_COPY_X_ANGLE				16
#define SF_COPY_Y_ANGLE				32
#define SF_COPY_Z_ANGLE				64
#define SF_COPY_X_AXIS				128
#define SF_COPY_Y_AXIS				256
#define SF_COPY_Z_AXIS				512

// If set, the target entity will not be moved to the copypointer's origin before doing
// the offset difference calculation (set this unless you want the target entity stuck
// to the center of the copypointer).
#define SF_SKIP_INITIAL_SET			1024

class CTriggerSetOrigin : public CPointEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT ConstantModeThink();

	void UpdateEntity();

	string_t m_copyPointer;
	Vector m_offset;
	Vector m_angleoffset;
	bool m_invertAngleX;
	bool m_invertAngleY;
	bool m_invertAngleZ;

	EHANDLE m_hCopyEnt;
	EHANDLE m_hTarget;
	bool m_isActive;
	Vector m_lockOffset;
	Vector m_lockOffsetAngles;
};

LINK_ENTITY_TO_CLASS(trigger_setorigin, CTriggerSetOrigin);

void CTriggerSetOrigin::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "copypointer"))
	{
		m_copyPointer = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "offset"))
	{
		UTIL_StringToVector(m_offset, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "angleoffset"))
	{
		UTIL_StringToVector(m_angleoffset, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "invert_x"))
	{
		m_invertAngleX = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "invert_y"))
	{
		m_invertAngleY = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "invert_z"))
	{
		m_invertAngleZ = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CPointEntity::KeyValue(pkvd);
	}
}

void CTriggerSetOrigin::Spawn(void)
{
	CPointEntity::Spawn();
}

void CTriggerSetOrigin::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	m_isActive = useType == USE_TOGGLE ? !m_isActive : useType == USE_ON; // TODO: test that it respects useType

	m_hCopyEnt = m_copyPointer ? UTIL_FindEntityByTargetname(NULL, STRING(m_copyPointer)) : NULL;
	m_hTarget = pev->target ? UTIL_FindEntityByTargetname(NULL, STRING(pev->target)) : NULL;
	
	if (FNullEnt(m_hCopyEnt.GetEdict()) || FNullEnt(m_hTarget.GetEdict())) {
		if (pev->spawnflags & SF_SET_ONCE) {
			UTIL_Remove(this);
		}
		return;
	}

	if (pev->spawnflags & SF_LOCK_OFFSETS) {

		if (pev->spawnflags & SF_SKIP_INITIAL_SET) {
			m_lockOffset = m_hTarget->pev->origin - m_hCopyEnt->pev->origin;
		}
		else {
			m_lockOffset = Vector(0, 0, 0); // TODO: just guessing after reading docs
		}
		
		m_lockOffsetAngles = m_hTarget->pev->angles - m_hCopyEnt->pev->angles;
	}

	UpdateEntity();

	if (pev->spawnflags & SF_SET_ONCE) {
		UTIL_Remove(this);
		return;
	}

	if (pev->spawnflags & SF_CONSTANT) {
		if (m_isActive) {
			SetThink(&CTriggerSetOrigin::ConstantModeThink);
			pev->nextthink = gpGlobals->time;
		}
		else {
			SetThink(NULL);
			pev->nextthink = 0;
		}
	}
}

void CTriggerSetOrigin::ConstantModeThink() {
	UpdateEntity();
	pev->nextthink = gpGlobals->time;
}

void CTriggerSetOrigin::UpdateEntity() {
	if (FNullEnt(m_hCopyEnt.GetEdict()) || FNullEnt(m_hTarget.GetEdict())) {
		return;
	}

	if (pev->spawnflags & (SF_COPY_X_AXIS | SF_LOCK_OFFSETS)) {
		m_hTarget->pev->origin.x = m_hCopyEnt->pev->origin.x + m_offset.x + m_lockOffset.x;
	}
	if (pev->spawnflags & (SF_COPY_Y_AXIS | SF_LOCK_OFFSETS)) {
		m_hTarget->pev->origin.y = m_hCopyEnt->pev->origin.y + m_offset.y + m_lockOffset.y;
	}
	if (pev->spawnflags & (SF_COPY_Z_AXIS | SF_LOCK_OFFSETS)) {
		m_hTarget->pev->origin.z = m_hCopyEnt->pev->origin.z + m_offset.z + m_lockOffset.z;
	}

	if (pev->spawnflags & (SF_COPY_X_ANGLE | SF_LOCK_OFFSETS)) {
		float v = m_hCopyEnt->pev->angles.x;
		m_hTarget->pev->angles.x = (m_invertAngleX ? -v : v) + m_angleoffset.x + m_lockOffsetAngles.x;
	}
	if (pev->spawnflags & (SF_COPY_Y_ANGLE | SF_LOCK_OFFSETS)) {
		float v = m_hCopyEnt->pev->angles.y;
		m_hTarget->pev->angles.y = (m_invertAngleY ? -v : v) + m_angleoffset.y + m_lockOffsetAngles.y;
	}
	if (pev->spawnflags & (SF_COPY_Z_ANGLE | SF_LOCK_OFFSETS)) {
		float v = m_hCopyEnt->pev->angles.z;
		m_hTarget->pev->angles.z = (m_invertAngleZ ? -v : v) + m_angleoffset.z + m_lockOffsetAngles.z;
	}

	// TODO: might not be needed but I know this has fixed bugs in the past
	UTIL_SetOrigin(m_hTarget->pev, m_hTarget->pev->origin);
}
