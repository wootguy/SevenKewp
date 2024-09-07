#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "customentity.h"
#include "effects.h"
#include "weapons.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"
#include "CBeam.h"

class CLightning : public CBeam
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	KeyValue(KeyValueData* pkvd);
	void	Activate(void);

	void	EXPORT StrikeThink(void);
	void	EXPORT DamageThink(void);
	void	RandomArea(void);
	void	RandomPoint(Vector& vecSrc);
	void	Zap(const Vector& vecSrc, const Vector& vecDest);
	void	EXPORT StrikeUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void	EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	inline BOOL ServerSide(void)
	{
		if (m_life == 0 && !(pev->spawnflags & SF_BEAM_RING))
			return TRUE;
		return FALSE;
	}

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	void	BeamUpdateVars(void);

	int		m_active;
	int		m_iszStartEntity;
	int		m_iszEndEntity;
	float	m_life;
	int		m_boltWidth;
	int		m_noiseAmplitude;
	int		m_brightness;
	int		m_speed;
	float	m_restrike;
	int		m_spriteTexture;
	int		m_iszSpriteName;
	int		m_frameStart;

	float	m_radius;
};

LINK_ENTITY_TO_CLASS(env_lightning, CLightning)
LINK_ENTITY_TO_CLASS(env_beam, CLightning)

// UNDONE: Jay -- This is only a test
#if _DEBUG
class CTripBeam : public CLightning
{
	void Spawn(void);
};
LINK_ENTITY_TO_CLASS(trip_beam, CTripBeam)

void CTripBeam::Spawn(void)
{
	CLightning::Spawn();
	SetTouch(&CTripBeam::TriggerTouch);
	pev->solid = SOLID_TRIGGER;
	RelinkBeam();
}
#endif



TYPEDESCRIPTION	CLightning::m_SaveData[] =
{
	DEFINE_FIELD(CLightning, m_active, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_iszStartEntity, FIELD_STRING),
	DEFINE_FIELD(CLightning, m_iszEndEntity, FIELD_STRING),
	DEFINE_FIELD(CLightning, m_life, FIELD_FLOAT),
	DEFINE_FIELD(CLightning, m_boltWidth, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_noiseAmplitude, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_brightness, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_speed, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_restrike, FIELD_FLOAT),
	DEFINE_FIELD(CLightning, m_spriteTexture, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_iszSpriteName, FIELD_STRING),
	DEFINE_FIELD(CLightning, m_frameStart, FIELD_INTEGER),
	DEFINE_FIELD(CLightning, m_radius, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CLightning, CBeam)


void CLightning::Spawn(void)
{
	if (FStringNull(m_iszSpriteName))
	{
		SetThink(&CLightning::SUB_Remove);
		return;
	}
	pev->solid = SOLID_NOT;							// Remove model & collisions
	Precache();

	pev->dmgtime = gpGlobals->time;

	if (ServerSide())
	{
		SetThink(NULL);
		if (pev->dmg > 0)
		{
			SetThink(&CLightning::DamageThink);
			pev->nextthink = gpGlobals->time + 0.1;
		}
		if (pev->targetname)
		{
			if (!(pev->spawnflags & SF_BEAM_STARTON))
			{
				pev->effects = EF_NODRAW;
				m_active = 0;
				pev->nextthink = 0;
			}
			else
				m_active = 1;

			SetUse(&CLightning::ToggleUse);
		}
	}
	else
	{
		m_active = 0;
		if (!FStringNull(pev->targetname))
		{
			SetUse(&CLightning::StrikeUse);
		}
		if (FStringNull(pev->targetname) || FBitSet(pev->spawnflags, SF_BEAM_STARTON))
		{
			SetThink(&CLightning::StrikeThink);
			pev->nextthink = gpGlobals->time + 1.0;
		}
	}

	// beam life is used as (int)(m_life*10) in the network messages, so this must be at least 0.1
	// or else it becomes 0, meaining infinite life, leading to beam overflows.
	// 0 is still a valid value though for mappers wanting infinite life.
	if (m_life > 0 && m_life < 0.1f) {
		m_life = 0.1f;
	}
}

void CLightning::Precache(void)
{
	m_spriteTexture = PRECACHE_MODEL((char*)STRING(m_iszSpriteName));
	CBeam::Precache();
}


void CLightning::Activate(void)
{
	if (ServerSide())
		BeamUpdateVars();
}


void CLightning::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "LightningStart"))
	{
		m_iszStartEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "LightningEnd"))
	{
		m_iszEndEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "life"))
	{
		m_life = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "BoltWidth"))
	{
		m_boltWidth = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "NoiseAmplitude"))
	{
		m_noiseAmplitude = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "TextureScroll"))
	{
		m_speed = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "StrikeTime"))
	{
		m_restrike = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "texture"))
	{
		m_iszSpriteName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "framestart"))
	{
		m_frameStart = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "Radius"))
	{
		m_radius = atof(pkvd->szValue);
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


void CLightning::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_active))
		return;
	if (m_active)
	{
		m_active = 0;
		pev->effects |= EF_NODRAW;
		pev->nextthink = 0;
	}
	else
	{
		m_active = 1;
		pev->effects &= ~EF_NODRAW;
		DoSparks(GetStartPos(), GetEndPos());
		if (pev->dmg > 0)
		{
			pev->nextthink = gpGlobals->time;
			pev->dmgtime = gpGlobals->time;
		}
	}
}


void CLightning::StrikeUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_active))
		return;

	if (m_active)
	{
		m_active = 0;
		SetThink(NULL);
	}
	else
	{
		SetThink(&CLightning::StrikeThink);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	if (!FBitSet(pev->spawnflags, SF_BEAM_TOGGLE))
		SetUse(NULL);
}


int IsPointEntity(CBaseEntity* pEnt)
{
	if (!pEnt->pev->modelindex)
		return 1;
	if (FClassnameIs(pEnt->pev, "info_target") || FClassnameIs(pEnt->pev, "info_landmark") || FClassnameIs(pEnt->pev, "path_corner"))
		return 1;

	return 0;
}


void CLightning::StrikeThink(void)
{
	if (m_life != 0)
	{
		if (pev->spawnflags & SF_BEAM_RANDOM)
			pev->nextthink = gpGlobals->time + m_life + RANDOM_FLOAT(0, m_restrike);
		else
			pev->nextthink = gpGlobals->time + m_life + m_restrike;
	}
	m_active = 1;

	if (FStringNull(m_iszEndEntity))
	{
		if (FStringNull(m_iszStartEntity))
		{
			RandomArea();
		}
		else
		{
			CBaseEntity* pStart = RandomTargetname(STRING(m_iszStartEntity));
			if (pStart != NULL)
				RandomPoint(pStart->pev->origin);
			else
				ALERT(at_console, "env_beam: unknown entity \"%s\"\n", STRING(m_iszStartEntity));
		}
		return;
	}

	CBaseEntity* pStart = RandomTargetname(STRING(m_iszStartEntity));
	CBaseEntity* pEnd = RandomTargetname(STRING(m_iszEndEntity));

	if (pStart != NULL && pEnd != NULL)
	{
		if (IsPointEntity(pStart) || IsPointEntity(pEnd))
		{
			if (pev->spawnflags & SF_BEAM_RING)
			{
				// don't work
				return;
			}
		}

		MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
		if (IsPointEntity(pStart) || IsPointEntity(pEnd))
		{
			if (!IsPointEntity(pEnd))	// One point entity must be in pEnd
			{
				CBaseEntity* pTemp;
				pTemp = pStart;
				pStart = pEnd;
				pEnd = pTemp;
			}
			if (!IsPointEntity(pStart))	// One sided
			{
				if (!UTIL_isSafeEntIndex(pStart->entindex(), "create lightning")) {
					return;
				}
				WRITE_BYTE(TE_BEAMENTPOINT);
				WRITE_SHORT(pStart->entindex());
				WRITE_COORD(pEnd->pev->origin.x);
				WRITE_COORD(pEnd->pev->origin.y);
				WRITE_COORD(pEnd->pev->origin.z);
			}
			else
			{
				WRITE_BYTE(TE_BEAMPOINTS);
				WRITE_COORD(pStart->pev->origin.x);
				WRITE_COORD(pStart->pev->origin.y);
				WRITE_COORD(pStart->pev->origin.z);
				WRITE_COORD(pEnd->pev->origin.x);
				WRITE_COORD(pEnd->pev->origin.y);
				WRITE_COORD(pEnd->pev->origin.z);
			}


		}
		else
		{
			if (!UTIL_isSafeEntIndex(pStart->entindex(), "create lightning")) {
				return;
			}
			if (!UTIL_isSafeEntIndex(pEnd->entindex(), "create lightning")) {
				return;
			}

			if (pev->spawnflags & SF_BEAM_RING)
				WRITE_BYTE(TE_BEAMRING);
			else
				WRITE_BYTE(TE_BEAMENTS);
			WRITE_SHORT(pStart->entindex());
			WRITE_SHORT(pEnd->entindex());
		}

		WRITE_SHORT(m_spriteTexture);
		WRITE_BYTE(m_frameStart); // framestart
		WRITE_BYTE((int)pev->framerate); // framerate
		WRITE_BYTE((int)(m_life * 10.0)); // life
		WRITE_BYTE(m_boltWidth);  // width
		WRITE_BYTE(m_noiseAmplitude);   // noise
		WRITE_BYTE((int)pev->rendercolor.x);   // r, g, b
		WRITE_BYTE((int)pev->rendercolor.y);   // r, g, b
		WRITE_BYTE((int)pev->rendercolor.z);   // r, g, b
		WRITE_BYTE(pev->renderamt);	// brightness
		WRITE_BYTE(m_speed);		// speed
		MESSAGE_END();
		DoSparks(pStart->pev->origin, pEnd->pev->origin);
		if (pev->dmg > 0)
		{
			TraceResult tr;
			UTIL_TraceLine(pStart->pev->origin, pEnd->pev->origin, dont_ignore_monsters, NULL, &tr);
			BeamDamageInstant(&tr, pev->dmg);
		}
	}
}


void CBeam::BeamDamage(TraceResult* ptr)
{
	RelinkBeam();
	if (ptr->flFraction != 1.0 && ptr->pHit != NULL)
	{
		CBaseEntity* pHit = CBaseEntity::Instance(ptr->pHit);
		if (pHit)
		{
			ClearMultiDamage();
			pHit->TraceAttack(pev, pev->dmg * (gpGlobals->time - pev->dmgtime), (ptr->vecEndPos - pev->origin).Normalize(), ptr, DMG_ENERGYBEAM);
			ApplyMultiDamage(pev, pev);
			if (pev->spawnflags & SF_BEAM_DECALS)
			{
				if (pHit->IsBSPModel())
					UTIL_DecalTrace(ptr, DECAL_BIGSHOT1 + RANDOM_LONG(0, 4));
			}
		}
	}
	pev->dmgtime = gpGlobals->time;
}


void CLightning::DamageThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1;
	TraceResult tr;
	UTIL_TraceLine(GetStartPos(), GetEndPos(), dont_ignore_monsters, NULL, &tr);
	BeamDamage(&tr);
}



void CLightning::Zap(const Vector& vecSrc, const Vector& vecDest)
{
#if 1
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(vecSrc.x);
	WRITE_COORD(vecSrc.y);
	WRITE_COORD(vecSrc.z);
	WRITE_COORD(vecDest.x);
	WRITE_COORD(vecDest.y);
	WRITE_COORD(vecDest.z);
	WRITE_SHORT(m_spriteTexture);
	WRITE_BYTE(m_frameStart); // framestart
	WRITE_BYTE((int)pev->framerate); // framerate
	WRITE_BYTE((int)(m_life * 10.0)); // life
	WRITE_BYTE(m_boltWidth);  // width
	WRITE_BYTE(m_noiseAmplitude);   // noise
	WRITE_BYTE((int)pev->rendercolor.x);   // r, g, b
	WRITE_BYTE((int)pev->rendercolor.y);   // r, g, b
	WRITE_BYTE((int)pev->rendercolor.z);   // r, g, b
	WRITE_BYTE(pev->renderamt);	// brightness
	WRITE_BYTE(m_speed);		// speed
	MESSAGE_END();
#else
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_LIGHTNING);
	WRITE_COORD(vecSrc.x);
	WRITE_COORD(vecSrc.y);
	WRITE_COORD(vecSrc.z);
	WRITE_COORD(vecDest.x);
	WRITE_COORD(vecDest.y);
	WRITE_COORD(vecDest.z);
	WRITE_BYTE(10);
	WRITE_BYTE(50);
	WRITE_BYTE(40);
	WRITE_SHORT(m_spriteTexture);
	MESSAGE_END();
#endif
	DoSparks(vecSrc, vecDest);
}

void CLightning::RandomArea(void)
{
	int iLoops = 0;

	for (iLoops = 0; iLoops < 10; iLoops++)
	{
		Vector vecSrc = pev->origin;

		Vector vecDir1 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		vecDir1 = vecDir1.Normalize();
		TraceResult		tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir1 * m_radius, ignore_monsters, ENT(pev), &tr1);

		if (tr1.flFraction == 1.0)
			continue;

		Vector vecDir2;
		do {
			vecDir2 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		} while (DotProduct(vecDir1, vecDir2) > 0);
		vecDir2 = vecDir2.Normalize();
		TraceResult		tr2;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir2 * m_radius, ignore_monsters, ENT(pev), &tr2);

		if (tr2.flFraction == 1.0)
			continue;

		if ((tr1.vecEndPos - tr2.vecEndPos).Length() < m_radius * 0.1)
			continue;

		UTIL_TraceLine(tr1.vecEndPos, tr2.vecEndPos, ignore_monsters, ENT(pev), &tr2);

		if (tr2.flFraction != 1.0)
			continue;

		Zap(tr1.vecEndPos, tr2.vecEndPos);

		break;
	}
}


void CLightning::RandomPoint(Vector& vecSrc)
{
	int iLoops = 0;

	for (iLoops = 0; iLoops < 10; iLoops++)
	{
		Vector vecDir1 = Vector(RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0), RANDOM_FLOAT(-1.0, 1.0));
		vecDir1 = vecDir1.Normalize();
		TraceResult		tr1;
		UTIL_TraceLine(vecSrc, vecSrc + vecDir1 * m_radius, ignore_monsters, ENT(pev), &tr1);

		if ((tr1.vecEndPos - vecSrc).Length() < m_radius * 0.1)
			continue;

		if (tr1.flFraction == 1.0)
			continue;

		Zap(vecSrc, tr1.vecEndPos);
		break;
	}
}



void CLightning::BeamUpdateVars(void)
{
	int beamType;
	int pointStart, pointEnd;

	edict_t* pStart = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_iszStartEntity));
	edict_t* pEnd = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_iszEndEntity));
	pointStart = IsPointEntity(CBaseEntity::Instance(pStart));
	pointEnd = IsPointEntity(CBaseEntity::Instance(pEnd));

	pev->skin = 0;
	pev->sequence = 0;
	pev->rendermode = 0;
	pev->flags |= FL_CUSTOMENTITY;
	pev->model = m_iszSpriteName;
	SetTexture(m_spriteTexture);

	beamType = BEAM_ENTS;
	if (pointStart || pointEnd)
	{
		if (!pointStart)	// One point entity must be in pStart
		{
			edict_t* pTemp;
			// Swap start & end
			pTemp = pStart;
			pStart = pEnd;
			pEnd = pTemp;
			int swap = pointStart;
			pointStart = pointEnd;
			pointEnd = swap;
		}
		if (!pointEnd)
			beamType = BEAM_ENTPOINT;
		else
			beamType = BEAM_POINTS;
	}

	SetType(beamType);
	if (beamType == BEAM_POINTS || beamType == BEAM_ENTPOINT || beamType == BEAM_HOSE)
	{
		SetStartPos(pStart->v.origin);
		if (beamType == BEAM_POINTS || beamType == BEAM_HOSE)
			SetEndPos(pEnd->v.origin);
		else
			SetEndEntity(ENTINDEX(pEnd));
	}
	else
	{
		SetStartEntity(ENTINDEX(pStart));
		SetEndEntity(ENTINDEX(pEnd));
	}

	RelinkBeam();

	SetWidth(m_boltWidth);
	SetNoise(m_noiseAmplitude);
	SetFrame(m_frameStart);
	SetScrollRate(m_speed);
	if (pev->spawnflags & SF_BEAM_SHADEIN)
		SetFlags(BEAM_FSHADEIN);
	else if (pev->spawnflags & SF_BEAM_SHADEOUT)
		SetFlags(BEAM_FSHADEOUT);
}
