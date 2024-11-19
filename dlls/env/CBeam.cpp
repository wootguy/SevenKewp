#include "extdll.h"
#include "util.h"
#include "customentity.h"
#include "effects.h"
#include "decals.h"
#include "CBreakable.h"
#include "shake.h"
#include "CBeam.h"

LINK_ENTITY_TO_CLASS(beam, CBeam)

void CBeam::Spawn(void)
{
	pev->solid = SOLID_NOT;							// Remove model & collisions
	Precache();
}

void CBeam::Precache(void)
{
	if (pev->owner)
		SetStartEntity(ENTINDEX(pev->owner));
	if (pev->aiment)
		SetEndEntity(ENTINDEX(pev->aiment));
}

void CBeam::SetStartEntity(int entityIndex)
{
	pev->sequence = (entityIndex & 0x0FFF) | ((pev->sequence & 0xF000) << 12);
	pev->owner = g_engfuncs.pfnPEntityOfEntIndex(entityIndex);
}

void CBeam::SetEndEntity(int entityIndex)
{
	pev->skin = (entityIndex & 0x0FFF) | ((pev->skin & 0xF000) << 12);
	pev->aiment = g_engfuncs.pfnPEntityOfEntIndex(entityIndex);
}


// These don't take attachments into account
const Vector& CBeam::GetStartPos(void)
{
	if (GetType() == BEAM_ENTS)
	{
		edict_t* pent = g_engfuncs.pfnPEntityOfEntIndex(GetStartEntity());
		return pent->v.origin;
	}
	return pev->origin;
}


const Vector& CBeam::GetEndPos(void)
{
	int type = GetType();
	if (type == BEAM_POINTS || type == BEAM_HOSE)
	{
		return pev->angles;
	}

	edict_t* pent = g_engfuncs.pfnPEntityOfEntIndex(GetEndEntity());
	if (pent)
		return pent->v.origin;
	return pev->angles;
}


CBeam* CBeam::BeamCreate(const char* pSpriteName, int width)
{
	// Create a new entity with CBeam private data
	CBeam* pBeam = GetClassPtr((CBeam*)NULL);
	pBeam->pev->classname = MAKE_STRING("beam");

	pBeam->BeamInit(pSpriteName, width);

	return pBeam;
}


void CBeam::BeamInit(const char* pSpriteName, int width)
{
	pev->flags |= FL_CUSTOMENTITY;
	SetColor(255, 255, 255);
	SetBrightness(255);
	SetNoise(0);
	SetFrame(0);
	SetScrollRate(0);
	pev->model = MAKE_STRING(pSpriteName);
	SetTexture(PRECACHE_MODEL((char*)pSpriteName));
	SetWidth(width);
	pev->skin = 0;
	pev->sequence = 0;
	pev->rendermode = 0;
}


void CBeam::PointsInit(const Vector& start, const Vector& end)
{
	SetType(BEAM_POINTS);
	SetStartPos(start);
	SetEndPos(end);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}


void CBeam::HoseInit(const Vector& start, const Vector& direction)
{
	SetType(BEAM_HOSE);
	SetStartPos(start);
	SetEndPos(direction);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}


void CBeam::PointEntInit(const Vector& start, int endIndex)
{
	SetType(BEAM_ENTPOINT);
	SetStartPos(start);
	SetEndEntity(endIndex);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}

void CBeam::EntsInit(int startIndex, int endIndex)
{
	SetType(BEAM_ENTS);
	SetStartEntity(startIndex);
	SetEndEntity(endIndex);
	SetStartAttachment(0);
	SetEndAttachment(0);
	RelinkBeam();
}


void CBeam::RelinkBeam(void)
{
	const Vector& startPos = GetStartPos(), & endPos = GetEndPos();

	pev->mins.x = V_min(startPos.x, endPos.x);
	pev->mins.y = V_min(startPos.y, endPos.y);
	pev->mins.z = V_min(startPos.z, endPos.z);
	pev->maxs.x = V_max(startPos.x, endPos.x);
	pev->maxs.y = V_max(startPos.y, endPos.y);
	pev->maxs.z = V_max(startPos.z, endPos.z);
	pev->mins = pev->mins - pev->origin;
	pev->maxs = pev->maxs - pev->origin;

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);
}

#if 0
void CBeam::SetObjectCollisionBox(void)
{
	const Vector& startPos = GetStartPos(), & endPos = GetEndPos();

	pev->absmin.x = V_min(startPos.x, endPos.x);
	pev->absmin.y = V_min(startPos.y, endPos.y);
	pev->absmin.z = V_min(startPos.z, endPos.z);
	pev->absmax.x = V_max(startPos.x, endPos.x);
	pev->absmax.y = V_max(startPos.y, endPos.y);
	pev->absmax.z = V_max(startPos.z, endPos.z);
}
#endif


void CBeam::TriggerTouch(CBaseEntity* pOther)
{
	if (pOther->pev->flags & (FL_CLIENT | FL_MONSTER))
	{
		if (pev->owner)
		{
			CBaseEntity* pOwner = CBaseEntity::Instance(pev->owner);
			pOwner->Use(pOther, this, USE_TOGGLE, 0);
		}
		ALERT(at_console, "Firing targets!!!\n");
	}
}


CBaseEntity* CBeam::RandomTargetname(const char* szName)
{
	int total = 0;

	CBaseEntity* pEntity = NULL;
	CBaseEntity* pNewEntity = NULL;
	while ((pNewEntity = UTIL_FindEntityByTargetname(pNewEntity, szName)) != NULL)
	{
		total++;
		if (RANDOM_LONG(0, total - 1) < 1)
			pEntity = pNewEntity;
	}
	return pEntity;
}


void CBeam::DoSparks(const Vector& start, const Vector& end)
{
	if (pev->spawnflags & (SF_BEAM_SPARKSTART | SF_BEAM_SPARKEND))
	{
		if (pev->spawnflags & SF_BEAM_SPARKSTART)
		{
			UTIL_Sparks(start);
		}
		if (pev->spawnflags & SF_BEAM_SPARKEND)
		{
			UTIL_Sparks(end);
		}
	}
}

