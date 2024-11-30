#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "CBaseTrigger.h"

#define SF_LADDER_START_OFF 1

class CLadder : public CBaseTrigger
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	void KeyValue(KeyValueData* pkvd);
	void Spawn(void);
	void Precache(void);
	void Toggle(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};
LINK_ENTITY_TO_CLASS(func_ladder, CLadder)


void CLadder::KeyValue(KeyValueData* pkvd)
{
	CBaseTrigger::KeyValue(pkvd);
}

void CLadder::Toggle(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {

	pev->skin = pev->skin ? 0 : CONTENTS_LADDER;
	UTIL_SetOrigin(pev, pev->origin);
}


//=========================================================
// func_ladder - makes an area vertically negotiable
//=========================================================
void CLadder::Precache(void)
{
	// Do all of this in here because we need to 'convert' old saved games
	pev->solid = SOLID_NOT;
	pev->skin = (pev->spawnflags & SF_LADDER_START_OFF) ? 0 : CONTENTS_LADDER;

	if (CVAR_GET_FLOAT("showtriggers") == 0)
	{
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 0;
	}
	pev->effects &= ~EF_NODRAW;

	SetUse(&CLadder::Toggle);

	PRECACHE_SOUND("player/pl_ladder1.wav");
	PRECACHE_SOUND("player/pl_ladder2.wav");
	PRECACHE_SOUND("player/pl_ladder3.wav");
	PRECACHE_SOUND("player/pl_ladder4.wav");
}


void CLadder::Spawn(void)
{
	Precache();

	SET_MODEL(ENT(pev), STRING(pev->model));    // set size and link into world
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);
}

