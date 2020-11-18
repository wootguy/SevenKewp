#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "player_util.h"

// This plays a CD track when fired or when the player enters it's radius
class CTargetCDAudio : public CPointEntity
{
public:
	void			Spawn(void);
	void			KeyValue(KeyValueData* pkvd);

	virtual void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void			Think(void);
	void			Play(void);
};

LINK_ENTITY_TO_CLASS(target_cdaudio, CTargetCDAudio);

void CTargetCDAudio::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		pev->scale = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CTargetCDAudio::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	if (pev->scale > 0)
		pev->nextthink = gpGlobals->time + 1.0;
}

void CTargetCDAudio::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	Play();
}

// only plays for ONE client, so only use in single play!
void CTargetCDAudio::Think(void)
{
	edict_t* pClient;

	// manually find the single player. 
	pClient = g_engfuncs.pfnPEntityOfEntIndex(1);

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	pev->nextthink = gpGlobals->time + 0.5;

	if ((pClient->v.origin - pev->origin).Length() <= pev->scale)
		Play();

}

void CTargetCDAudio::Play(void)
{
	PlayCDTrack((int)pev->health);
	UTIL_Remove(this);
}