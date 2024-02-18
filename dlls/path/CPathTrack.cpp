#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "trains.h"
#include "saverestore.h"
#include "CPathTrack.h"

TYPEDESCRIPTION	CPathTrack::m_SaveData[] =
{
	DEFINE_FIELD(CPathTrack, m_length, FIELD_FLOAT),
	DEFINE_FIELD(CPathTrack, m_hNext, FIELD_EHANDLE),
	DEFINE_FIELD(CPathTrack, m_hAltpath, FIELD_CLASSPTR),
	DEFINE_FIELD(CPathTrack, m_hPrevious, FIELD_EHANDLE),
	DEFINE_FIELD(CPathTrack, m_altName, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CPathTrack, CBaseEntity);
LINK_ENTITY_TO_CLASS(path_track, CPathTrack);

//
// Cache user-entity-field values until spawn is called.
//
void CPathTrack::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "altpath"))
	{
		m_altName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CPathTrack::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	int on;

	// Use toggles between two paths
	if (m_hAltpath)
	{
		on = !FBitSet(pev->spawnflags, SF_PATH_ALTERNATE);
		if (ShouldToggle(useType, on))
		{
			if (on)
				SetBits(pev->spawnflags, SF_PATH_ALTERNATE);
			else
				ClearBits(pev->spawnflags, SF_PATH_ALTERNATE);
		}
	}
	else	// Use toggles between enabled/disabled
	{
		on = !FBitSet(pev->spawnflags, SF_PATH_DISABLED);

		if (ShouldToggle(useType, on))
		{
			if (on)
				SetBits(pev->spawnflags, SF_PATH_DISABLED);
			else
				ClearBits(pev->spawnflags, SF_PATH_DISABLED);
		}
	}
}


void CPathTrack::Link(void)
{
	edict_t* pentTarget;

	if (!FStringNull(pev->target))
	{
		pentTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target));
		if (!FNullEnt(pentTarget))
		{
			CPathTrack* next = CPathTrack::Instance(pentTarget);
			m_hNext = next;

			if (next)		// If no next pointer, this is the end of a path
			{
				next->SetPrevious(this);
			}
		}
		else
			ALERT(at_console, "Dead end link %s\n", STRING(pev->target));
	}

	// Find "alternate" path
	if (m_altName)
	{
		pentTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_altName));
		if (!FNullEnt(pentTarget))
		{
			CPathTrack* alt = CPathTrack::Instance(pentTarget);
			m_hAltpath = alt;

			if (alt)		// If no next pointer, this is the end of a path
			{
				alt->SetPrevious(this);
			}
		}
	}
}


void CPathTrack::Spawn(void)
{
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8));

	m_hNext = NULL;
	m_hPrevious = NULL;
	// DEBUGGING CODE
#if PATH_SPARKLE_DEBUG
	SetThink(Sparkle);
	pev->nextthink = gpGlobals->time + 0.5;
#endif
}


void CPathTrack::Activate(void)
{
	if (!FStringNull(pev->targetname))		// Link to next, and back-link
		Link();
}

CPathTrack* CPathTrack::ValidPath(CPathTrack* ppath, int testFlag)
{
	if (!ppath)
		return NULL;

	if (testFlag && FBitSet(ppath->pev->spawnflags, SF_PATH_DISABLED))
		return NULL;

	return ppath;
}


void CPathTrack::Project(CPathTrack* pstart, CPathTrack* pend, Vector* origin, float dist)
{
	if (pstart && pend)
	{
		Vector dir = (pend->pev->origin - pstart->pev->origin);
		dir = dir.Normalize();
		*origin = pend->pev->origin + dir * dist;
	}
}

CPathTrack* CPathTrack::GetNext(void)
{
	if (m_hAltpath && FBitSet(pev->spawnflags, SF_PATH_ALTERNATE) && !FBitSet(pev->spawnflags, SF_PATH_ALTREVERSE))
		return (CPathTrack*)m_hAltpath.GetEntity();

	return (CPathTrack*)m_hNext.GetEntity();
}



CPathTrack* CPathTrack::GetPrevious(void)
{
	if (m_hAltpath && FBitSet(pev->spawnflags, SF_PATH_ALTERNATE) && FBitSet(pev->spawnflags, SF_PATH_ALTREVERSE))
		return (CPathTrack*)m_hAltpath.GetEntity();

	return (CPathTrack*)m_hPrevious.GetEntity();
}



void CPathTrack::SetPrevious(CPathTrack* pprev)
{
	// Only set previous if this isn't my alternate path
	if (pprev && !FStrEq(STRING(pprev->pev->targetname), STRING(m_altName)))
		m_hPrevious = pprev;
}


// Assumes this is ALWAYS enabled
CPathTrack* CPathTrack::LookAhead(Vector* origin, float dist, int move)
{
	CPathTrack* pcurrent;
	float originalDist = dist;

	pcurrent = this;
	Vector currentPos = *origin;

	if (dist < 0)		// Travelling backwards through path
	{
		dist = -dist;
		while (dist > 0)
		{
			Vector dir = pcurrent->pev->origin - currentPos;
			float length = dir.Length();
			if (!length)
			{
				if (!ValidPath(pcurrent->GetPrevious(), move)) 	// If there is no previous node, or it's disabled, return now.
				{
					if (!move)
						Project(pcurrent->GetNext(), pcurrent, origin, dist);
					return NULL;
				}
				pcurrent = pcurrent->GetPrevious();
			}
			else if (length > dist)	// enough left in this path to move
			{
				*origin = currentPos + (dir * (dist / length));
				return pcurrent;
			}
			else
			{
				dist -= length;
				currentPos = pcurrent->pev->origin;
				*origin = currentPos;
				if (!ValidPath(pcurrent->GetPrevious(), move))	// If there is no previous node, or it's disabled, return now.
					return NULL;

				pcurrent = pcurrent->GetPrevious();
			}
		}
		*origin = currentPos;
		return pcurrent;
	}
	else
	{
		while (dist > 0)
		{
			if (!ValidPath(pcurrent->GetNext(), move))	// If there is no next node, or it's disabled, return now.
			{
				if (!move)
					Project(pcurrent->GetPrevious(), pcurrent, origin, dist);
				return NULL;
			}
			Vector dir = pcurrent->GetNext()->pev->origin - currentPos;
			float length = dir.Length();
			if (!length && !ValidPath(pcurrent->GetNext()->GetNext(), move))
			{
				if (dist == originalDist) // HACK -- up against a dead end
					return NULL;
				return pcurrent;
			}
			if (length > dist)	// enough left in this path to move
			{
				*origin = currentPos + (dir * (dist / length));
				return pcurrent;
			}
			else
			{
				dist -= length;
				currentPos = pcurrent->GetNext()->pev->origin;
				pcurrent = pcurrent->GetNext();
				*origin = currentPos;
			}
		}
		*origin = currentPos;
	}

	return pcurrent;
}


// Assumes this is ALWAYS enabled
CPathTrack* CPathTrack::Nearest(Vector origin)
{
	int			deadCount;
	float		minDist, dist;
	Vector		delta;
	CPathTrack* ppath, * pnearest;


	delta = origin - pev->origin;
	delta.z = 0;
	minDist = delta.Length();
	pnearest = this;
	ppath = GetNext();

	// Hey, I could use the old 2 racing pointers solution to this, but I'm lazy :)
	deadCount = 0;
	while (ppath && ppath != this)
	{
		deadCount++;
		if (deadCount > 9999)
		{
			ALERT(at_error, "Bad sequence of path_tracks from %s", STRING(pev->targetname));
			return NULL;
		}
		delta = origin - ppath->pev->origin;
		delta.z = 0;
		dist = delta.Length();
		if (dist < minDist)
		{
			minDist = dist;
			pnearest = ppath;
		}
		ppath = ppath->GetNext();
	}
	return pnearest;
}


CPathTrack* CPathTrack::Instance(edict_t* pent)
{
	if (FClassnameIs(pent, "path_track"))
		return (CPathTrack*)GET_PRIVATE(pent);
	return NULL;
}


// DEBUGGING CODE
#if PATH_SPARKLE_DEBUG
void CPathTrack::Sparkle(void)
{

	pev->nextthink = gpGlobals->time + 0.2;
	if (FBitSet(pev->spawnflags, SF_PATH_DISABLED))
		UTIL_ParticleEffect(pev->origin, Vector(0, 0, 100), 210, 10);
	else
		UTIL_ParticleEffect(pev->origin, Vector(0, 0, 100), 84, 10);
}
#endif
