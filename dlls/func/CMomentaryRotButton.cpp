#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "CBaseButton.h"

// Make this button behave like a door (HACKHACK)
// This will disable use and make the button solid
// rotating buttons were made SOLID_NOT by default since their were some
// collision problems with them...
#define SF_MOMENTARY_DOOR		0x0001

class CMomentaryRotButton : public CBaseToggle
{
public:
	void	Spawn(void);
	void	KeyValue(KeyValueData* pkvd);
	virtual int	ObjectCaps(void)
	{
		int flags = CBaseToggle::ObjectCaps() & (~FCAP_ACROSS_TRANSITION);
		if (pev->spawnflags & SF_MOMENTARY_DOOR)
			return flags;
		return flags | FCAP_CONTINUOUS_USE;
	}
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	EXPORT Off(void);
	void	EXPORT Return(void);
	void	UpdateSelf(float value);
	void	UpdateSelfReturn(float value);
	void	UpdateAllButtons(float value, int start);

	void	PlaySound(void);
	void	UpdateTarget(float value);

	float	GetRotProgress(Vector angles); // how close is the button to its destination angle? (0-1)

	static CMomentaryRotButton* Instance(edict_t* pent) { return (CMomentaryRotButton*)GET_PRIVATE(pent); };
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	int		m_lastUsed;
	int		m_direction;
	float	m_returnSpeed;
	vec3_t	m_start;
	vec3_t	m_end;
	int		m_sounds;
	int		m_overflows; // number of times the rotation has overflowd the 3600 engine limit
	float	m_lastProgress;
};
TYPEDESCRIPTION CMomentaryRotButton::m_SaveData[] =
{
	DEFINE_FIELD(CMomentaryRotButton, m_lastUsed, FIELD_INTEGER),
	DEFINE_FIELD(CMomentaryRotButton, m_direction, FIELD_INTEGER),
	DEFINE_FIELD(CMomentaryRotButton, m_returnSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CMomentaryRotButton, m_start, FIELD_VECTOR),
	DEFINE_FIELD(CMomentaryRotButton, m_end, FIELD_VECTOR),
	DEFINE_FIELD(CMomentaryRotButton, m_sounds, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CMomentaryRotButton, CBaseToggle)

LINK_ENTITY_TO_CLASS(momentary_rot_button, CMomentaryRotButton)

void CMomentaryRotButton::Spawn(void)
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	CBaseToggle::AxisDir(pev);

	if (pev->speed == 0)
		pev->speed = 100;

	if (m_flMoveDistance < 0)
	{
		m_start = pev->angles + pev->movedir * m_flMoveDistance;
		m_end = pev->angles;
		m_direction = 1;		// This will toggle to -1 on the first use()
		m_flMoveDistance = -m_flMoveDistance;
	}
	else
	{
		m_start = pev->angles;
		m_end = pev->angles + pev->movedir * m_flMoveDistance;
		m_direction = -1;		// This will toggle to +1 on the first use()
	}

	if (pev->spawnflags & SF_MOMENTARY_DOOR)
		pev->solid = SOLID_BSP;
	else
		pev->solid = SOLID_NOT;

	pev->movetype = MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	const char* pszSound = ButtonSound(m_sounds);
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);
	m_lastUsed = 0;
}

void CMomentaryRotButton::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "returnspeed"))
	{
		m_returnSpeed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CMomentaryRotButton::PlaySound(void)
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);
}

float CMomentaryRotButton::GetRotProgress(Vector angles) {
	return (CBaseToggle::AxisDelta(pev->spawnflags, angles, m_start) + (m_overflows * 3600.0f)) / m_flMoveDistance;
}

// BUGBUG: This design causes a latentcy.  When the button is retriggered, the first impulse
// will send the target in the wrong direction because the parameter is calculated based on the
// current, not future position.
void CMomentaryRotButton::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!RunInventoryRules(pCaller)) {
		return;
	}

	pev->ideal_yaw = GetRotProgress(pev->angles);

	UpdateAllButtons(pev->ideal_yaw, 1);

	// Calculate destination angle and use it to predict value, this prevents sending target in wrong direction on retriggering
	Vector dest = pev->angles + pev->avelocity * (pev->nextthink - pev->ltime);
	float value1 = GetRotProgress(dest);	

	UpdateTarget(value1);

}

void CMomentaryRotButton::UpdateAllButtons(float value, int start)
{
	// Update all rot buttons attached to the same target
	CBaseEntity* pentTarget = NULL;
	for (;;)
	{

		pentTarget = UTIL_FindEntityByString(pentTarget, "target", STRING(pev->target));
		if (!pentTarget)
			break;

		if (FClassnameIs(pentTarget->pev, "momentary_rot_button"))
		{
			CMomentaryRotButton* pEntity = CMomentaryRotButton::Instance(pentTarget->edict());
			if (pEntity)
			{
				if (start)
					pEntity->UpdateSelf(value);
				else
					pEntity->UpdateSelfReturn(value);
			}
		}
	}

	// detect when engine modulos the angle value, so it can be undone to calculate rotation progress
	if (value < 1.0f && m_direction > 0 && value < m_lastProgress) {
		m_overflows++;
	}
	else if (value > 0.0f && m_direction < 0 && value > m_lastProgress) {
		m_overflows--;
	}
	m_lastProgress = value;

	//ALERT(at_console, "ROT: %f %d %d %f\n", pev->angles.z, m_direction, m_overflows, value);
}

void CMomentaryRotButton::UpdateSelf(float value)
{
	BOOL fplaysound = FALSE;

	if (!m_lastUsed)
	{
		m_lastProgress = value; // don't immediately trigger overflow
		fplaysound = TRUE;
		m_direction = -m_direction;
	}
	m_lastUsed = 1;

	pev->nextthink = pev->ltime + 0.1;
	if (m_direction > 0 && value >= 1.0)
	{
		pev->avelocity = g_vecZero;
		pev->angles = m_end;

		for (int i = 0; i < 3; i++)
			if (pev->angles[i] <= -3600.0f || pev->angles[i] >= 3600.0f)
				pev->angles[i] = fmod(pev->angles[i], 3600.0f);

		m_overflows = m_flMoveDistance / 3600.0f;

		return;
	}
	else if (m_direction < 0 && value <= 0)
	{
		pev->avelocity = g_vecZero;
		pev->angles = m_start;
		m_overflows = 0;

		for (int i = 0; i < 3; i++)
			if (pev->angles[i] <= -3600.0f || pev->angles[i] >= 3600.0f)
				pev->angles[i] = fmod(pev->angles[i], 3600.0f);

		return;
	}

	if (fplaysound)
		PlaySound();

	// HACKHACK -- If we're going slow, we'll get multiple player packets per frame, bump nexthink on each one to avoid stalling
	if (pev->nextthink < pev->ltime)
		pev->nextthink = pev->ltime + 0.1;
	else
		pev->nextthink += 0.1;

	pev->avelocity = (m_direction * pev->speed) * pev->movedir;
	SetThink(&CMomentaryRotButton::Off);
}

void CMomentaryRotButton::UpdateTarget(float value)
{
	if (!FStringNull(pev->target))
	{
		CBaseEntity* pentTarget = NULL;
		for (;;)
		{
			pentTarget = UTIL_FindEntityByTargetname(pentTarget, STRING(pev->target));
			if (!pentTarget)
				break;
			CBaseEntity* pEntity = CBaseEntity::Instance(pentTarget->edict());
			if (pEntity)
			{
				pEntity->Use(this, this, USE_SET, value);
			}
		}
	}
}

void CMomentaryRotButton::Off(void)
{
	pev->avelocity = g_vecZero;
	m_lastUsed = 0;

	// don't immediately trigger an underflow
	m_lastProgress = GetRotProgress(pev->angles);

	if (FBitSet(pev->spawnflags, SF_PENDULUM_AUTO_RETURN) && m_returnSpeed > 0)
	{
		SetThink(&CMomentaryRotButton::Return);
		pev->nextthink = pev->ltime + 0.1;
		m_direction = -1;
	}
	else
		SetThink(NULL);
}

void CMomentaryRotButton::Return(void)
{
	m_direction = -1;
	float value = GetRotProgress(pev->angles);

	UpdateAllButtons(value, 0);	// This will end up calling UpdateSelfReturn() n times, but it still works right

	// recalculate in case of underflow
	value = GetRotProgress(pev->angles);

	if (value > 0)
		UpdateTarget(value);
}


void CMomentaryRotButton::UpdateSelfReturn(float value)
{
	m_lastUsed = 0;

	if (value <= 0)
	{
		pev->avelocity = g_vecZero;
		pev->angles = m_start;
		pev->nextthink = -1;
		SetThink(NULL);
	}
	else
	{
		pev->avelocity = -m_returnSpeed * pev->movedir;
		pev->nextthink = pev->ltime + 0.1;
	}
}