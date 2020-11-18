#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"
#include "player_util.h"

//=====================================
//
// trigger_cdaudio - starts/stops cd audio tracks
//
class CTriggerCDAudio : public CBaseTrigger
{
public:
	void Spawn(void);

	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void PlayTrack(void);
	void Touch(CBaseEntity* pOther);
};

LINK_ENTITY_TO_CLASS(trigger_cdaudio, CTriggerCDAudio);

//
// Changes tracks or stops CD when player touches
//
// !!!HACK - overloaded HEALTH to avoid adding new field
void CTriggerCDAudio::Touch(CBaseEntity* pOther)
{
	if (!pOther->IsPlayer())
	{// only clients may trigger these events
		return;
	}

	PlayTrack();
}

void CTriggerCDAudio::Spawn(void)
{
	InitTrigger();
}

void CTriggerCDAudio::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	PlayTrack();
}

// only plays for ONE client, so only use in single play!
void CTriggerCDAudio::PlayTrack(void)
{
	PlayCDTrack((int)pev->health);

	SetTouch(NULL);
	UTIL_Remove(this);
}

