#include "extdll.h"
#include "util.h"
#include "customentity.h"
#include "effects.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"

LINK_ENTITY_TO_CLASS(env_laser, CLaser)

TYPEDESCRIPTION	CLaser::m_SaveData[] =
{
	DEFINE_FIELD(CLaser, m_hSprite, FIELD_EHANDLE),
	DEFINE_FIELD(CLaser, m_iszSpriteName, FIELD_STRING),
	DEFINE_FIELD(CLaser, m_firePosition, FIELD_POSITION_VECTOR),
};

IMPLEMENT_SAVERESTORE(CLaser, CBeam)

void CLaser::Spawn(void)
{
	if (FStringNull(pev->model))
	{
		SetThink(&CLaser::SUB_Remove);
		return;
	}
	pev->solid = SOLID_NOT;							// Remove model & collisions
	Precache();

	SetThink(&CLaser::StrikeThink);
	pev->flags |= FL_CUSTOMENTITY;

	PointsInit(pev->origin, pev->origin);

	if (!m_hSprite && m_iszSpriteName)
		m_hSprite = CSprite::SpriteCreate(STRING(m_iszSpriteName), pev->origin, TRUE);
	else
		m_hSprite = NULL;

	CSprite* m_pSprite = (CSprite*)m_hSprite.GetEntity();
	if (m_pSprite)
		m_pSprite->SetTransparency(kRenderGlow, pev->rendercolor.x, pev->rendercolor.y, pev->rendercolor.z, pev->renderamt, pev->renderfx);

	if (pev->targetname && !(pev->spawnflags & SF_BEAM_STARTON))
		TurnOff();
	else
		TurnOn();
}

void CLaser::Precache(void)
{
	pev->modelindex = PRECACHE_MODEL((char*)STRING(pev->model));
	if (pev->modelindex == g_notPrecachedModelIdx) {
		pev->modelindex = g_sModelIndexLaser;
	}
	if (m_iszSpriteName)
		PRECACHE_MODEL((char*)STRING(m_iszSpriteName));
}


void CLaser::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "LaserTarget"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "width"))
	{
		SetWidth((int)atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "NoiseAmplitude"))
	{
		SetNoise(atoi(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "TextureScroll"))
	{
		SetScrollRate(atoi(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "texture"))
	{
		pev->model = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "EndSprite"))
	{
		m_iszSpriteName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "framestart"))
	{
		pev->frame = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBeam::KeyValue(pkvd);
}


int CLaser::IsOn(void)
{
	if (pev->effects & EF_NODRAW)
		return 0;
	return 1;
}


void CLaser::TurnOff(void)
{
	pev->effects |= EF_NODRAW;
	pev->nextthink = 0;

	CSprite* m_pSprite = (CSprite*)m_hSprite.GetEntity();
	if (m_pSprite)
		m_pSprite->TurnOff();
}


void CLaser::TurnOn(void)
{
	pev->effects &= ~EF_NODRAW;
	pev->dmgtime = gpGlobals->time;
	pev->nextthink = gpGlobals->time;

	CSprite* m_pSprite = (CSprite*)m_hSprite.GetEntity();
	if (m_pSprite)
		m_pSprite->TurnOn();
}


void CLaser::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int active = IsOn();

	if (!ShouldToggle(useType, active))
		return;
	if (active)
	{
		TurnOff();
	}
	else
	{
		TurnOn();
	}
}


void CLaser::FireAtPoint(TraceResult& tr)
{
	SetEndPos(tr.vecEndPos);

	CSprite* m_pSprite = (CSprite*)m_hSprite.GetEntity();
	if (m_pSprite)
		UTIL_SetOrigin(m_pSprite->pev, tr.vecEndPos);

	BeamDamage(&tr);
	DoSparks(GetStartPos(), tr.vecEndPos);
}

void CLaser::StrikeThink(void)
{
	CBaseEntity* pEnd = RandomTargetname(STRING(pev->message));

	if (pEnd)
		m_firePosition = pEnd->pev->origin;

	TraceResult tr;

	UTIL_TraceLine(pev->origin, m_firePosition, dont_ignore_monsters, NULL, &tr);
	FireAtPoint(tr);
	pev->nextthink = gpGlobals->time + 0.1;
}

