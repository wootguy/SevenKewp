#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "customentity.h"
#include "effects.h"
#include "weapons.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"

// Screen shake
class CShake : public CPointEntity
{
public:
	void	Spawn(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	KeyValue(KeyValueData* pkvd);

	inline	float	Amplitude(void) { return pev->scale; }
	inline	float	Frequency(void) { return pev->dmg_save; }
	inline	float	Duration(void) { return pev->dmg_take; }
	inline	float	Radius(void) { return pev->dmg; }

	inline	void	SetAmplitude(float amplitude) { pev->scale = amplitude; }
	inline	void	SetFrequency(float frequency) { pev->dmg_save = frequency; }
	inline	void	SetDuration(float duration) { pev->dmg_take = duration; }
	inline	void	SetRadius(float radius) { pev->dmg = radius; }
private:
};

LINK_ENTITY_TO_CLASS(env_shake, CShake);

// pev->scale is amplitude
// pev->dmg_save is frequency
// pev->dmg_take is duration
// pev->dmg is radius
// radius of 0 means all players
// NOTE: UTIL_ScreenShake() will only shake players who are on the ground

#define SF_SHAKE_EVERYONE	0x0001		// Don't check radius
// UNDONE: These don't work yet
#define SF_SHAKE_DISRUPT	0x0002		// Disrupt controls
#define SF_SHAKE_INAIR		0x0004		// Shake players in air

void CShake::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->effects = 0;
	pev->frame = 0;

	if (pev->spawnflags & SF_SHAKE_EVERYONE)
		pev->dmg = 0;
}


void CShake::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "amplitude"))
	{
		SetAmplitude(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "frequency"))
	{
		SetFrequency(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "radius"))
	{
		SetRadius(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}


void CShake::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	UTIL_ScreenShake(pev->origin, Amplitude(), Frequency(), Duration(), Radius());
}
