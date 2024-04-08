#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "trains.h"
#include "saverestore.h"
#include "CBasePlatTrain.h"
#include "CFuncPlat.h"

TYPEDESCRIPTION	CBasePlatTrain::m_SaveData[] =
{
	DEFINE_FIELD(CBasePlatTrain, m_bMoveSnd, FIELD_CHARACTER),
	DEFINE_FIELD(CBasePlatTrain, m_bStopSnd, FIELD_CHARACTER),
	DEFINE_FIELD(CBasePlatTrain, m_volume, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CBasePlatTrain, CBaseToggle);

void CBasePlatTrain::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "height"))
	{
		m_flHeight = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "rotation"))
	{
		m_vecFinalAngle.x = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "stopsnd"))
	{
		m_bStopSnd = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_volume = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

void CBasePlatTrain::Precache(void)
{
	// set the plat's "in-motion" sound
	switch (m_bMoveSnd)
	{
	case	0:
		pev->plat_noiseMoving = MAKE_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("plats/bigmove1.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/bigmove1.wav");
		break;
	case	2:
		PRECACHE_SOUND("plats/bigmove2.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/bigmove2.wav");
		break;
	case	3:
		PRECACHE_SOUND("plats/elevmove1.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/elevmove1.wav");
		break;
	case	4:
		PRECACHE_SOUND("plats/elevmove2.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/elevmove2.wav");
		break;
	case	5:
		PRECACHE_SOUND("plats/elevmove3.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/elevmove3.wav");
		break;
	case	6:
		PRECACHE_SOUND("plats/freightmove1.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/freightmove1.wav");
		break;
	case	7:
		PRECACHE_SOUND("plats/freightmove2.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/freightmove2.wav");
		break;
	case	8:
		PRECACHE_SOUND("plats/heavymove1.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/heavymove1.wav");
		break;
	case	9:
		PRECACHE_SOUND("plats/rackmove1.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/rackmove1.wav");
		break;
	case	10:
		PRECACHE_SOUND("plats/railmove1.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/railmove1.wav");
		break;
	case	11:
		PRECACHE_SOUND("plats/squeekmove1.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/squeekmove1.wav");
		break;
	case	12:
		PRECACHE_SOUND("plats/talkmove1.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/talkmove1.wav");
		break;
	case	13:
		PRECACHE_SOUND("plats/talkmove2.wav");
		pev->plat_noiseMoving = MAKE_STRING("plats/talkmove2.wav");
		break;
	default:
		pev->plat_noiseMoving = MAKE_STRING("common/null.wav");
		break;
	}

	// set the plat's 'reached destination' stop sound
	switch (m_bStopSnd)
	{
	case	0:
		pev->plat_noiseArrived = MAKE_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("plats/bigstop1.wav");
		pev->plat_noiseArrived = MAKE_STRING("plats/bigstop1.wav");
		break;
	case	2:
		PRECACHE_SOUND("plats/bigstop2.wav");
		pev->plat_noiseArrived = MAKE_STRING("plats/bigstop2.wav");
		break;
	case	3:
		PRECACHE_SOUND("plats/freightstop1.wav");
		pev->plat_noiseArrived = MAKE_STRING("plats/freightstop1.wav");
		break;
	case	4:
		PRECACHE_SOUND("plats/heavystop2.wav");
		pev->plat_noiseArrived = MAKE_STRING("plats/heavystop2.wav");
		break;
	case	5:
		PRECACHE_SOUND("plats/rackstop1.wav");
		pev->plat_noiseArrived = MAKE_STRING("plats/rackstop1.wav");
		break;
	case	6:
		PRECACHE_SOUND("plats/railstop1.wav");
		pev->plat_noiseArrived = MAKE_STRING("plats/railstop1.wav");
		break;
	case	7:
		PRECACHE_SOUND("plats/squeekstop1.wav");
		pev->plat_noiseArrived = MAKE_STRING("plats/squeekstop1.wav");
		break;
	case	8:
		PRECACHE_SOUND("plats/talkstop1.wav");
		pev->plat_noiseArrived = MAKE_STRING("plats/talkstop1.wav");
		break;

	default:
		pev->plat_noiseArrived = MAKE_STRING("common/null.wav");
		break;
	}
}
