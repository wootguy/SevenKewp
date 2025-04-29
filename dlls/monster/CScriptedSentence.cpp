#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "schedule.h"
#include "CCineMonster.h"
#include "defaultai.h"
#include "sentences.h"

class CScriptedSentence : public CBaseToggle
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void Spawn(void);
	void Precache();
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT FindThink(void);
	void EXPORT DelayThink(void);
	int	 ObjectCaps(void) { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	CBaseMonster* FindEntity(void);
	BOOL AcceptableSpeaker(CBaseMonster* pMonster);
	BOOL StartSentence(CBaseMonster* pTarget);


private:
	int		m_iszSentence;		// string index for idle animation
	int		m_iszEntity;	// entity that is wanted for this sentence
	float	m_flRadius;		// range to search
	float	m_flDuration;	// How long the sentence lasts
	float	m_flRepeat;	// repeat rate
	float	m_flAttenuation;
	float	m_flVolume;
	BOOL	m_active;
	int		m_iszListener;	// name of entity to look at while talking
};

#define SF_SENTENCE_ONCE		0x0001
#define SF_SENTENCE_FOLLOWERS	0x0002	// only say if following player
#define SF_SENTENCE_INTERRUPT	0x0004	// force talking except when dead
#define SF_SENTENCE_CONCURRENT	0x0008	// allow other people to keep talking

TYPEDESCRIPTION	CScriptedSentence::m_SaveData[] =
{
	DEFINE_FIELD(CScriptedSentence, m_iszSentence, FIELD_STRING),
	DEFINE_FIELD(CScriptedSentence, m_iszEntity, FIELD_STRING),
	DEFINE_FIELD(CScriptedSentence, m_flRadius, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_flDuration, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_flRepeat, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_flAttenuation, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_flVolume, FIELD_FLOAT),
	DEFINE_FIELD(CScriptedSentence, m_active, FIELD_BOOLEAN),
	DEFINE_FIELD(CScriptedSentence, m_iszListener, FIELD_STRING),
};


IMPLEMENT_SAVERESTORE(CScriptedSentence, CBaseToggle)

LINK_ENTITY_TO_CLASS(scripted_sentence, CScriptedSentence)

void CScriptedSentence::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "sentence"))
	{
		m_iszSentence = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "entity"))
	{
		m_iszEntity = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "duration"))
	{
		m_flDuration = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "refire"))
	{
		m_flRepeat = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "attenuation"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_flVolume = atof(pkvd->szValue) * 0.1;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "listener"))
	{
		m_iszListener = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}


void CScriptedSentence::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (!m_active)
		return;
	//	ALERT( at_console, "Firing sentence: %s\n", STRING(m_iszSentence) );
	SetThink(&CScriptedSentence::FindThink);
	pev->nextthink = gpGlobals->time;
}


void CScriptedSentence::Spawn(void)
{
	Precache();

	pev->solid = SOLID_NOT;

	m_active = TRUE;
	// if no targetname, start now
	if (!pev->targetname)
	{
		SetThink(&CScriptedSentence::FindThink);
		pev->nextthink = gpGlobals->time + 1.0;
	}

	switch (pev->impulse)
	{
	case 1: // Medium radius
		m_flAttenuation = ATTN_STATIC;
		break;

	case 2:	// Large radius
		m_flAttenuation = ATTN_NORM;
		break;

	case 3:	//EVERYWHERE
		m_flAttenuation = ATTN_NONE;
		break;

	default:
	case 0: // Small radius
		m_flAttenuation = ATTN_IDLE;
		break;
	}
	pev->impulse = 0;

	// No volume, use normal
	if (m_flVolume <= 0)
		m_flVolume = 1.0;
}

void CScriptedSentence::Precache(void) {
	if (m_iszSentence) {
		if (STRING(m_iszSentence)[0] == '+')
			PRECACHE_SOUND(STRING(m_iszSentence) + 1);
		else
			PrecacheCustomSentence(this, STRING(m_iszSentence));
	}
}

void CScriptedSentence::FindThink(void)
{
	CBaseMonster* pMonster = FindEntity();
	if (pMonster)
	{
		StartSentence(pMonster);
		if (pev->spawnflags & SF_SENTENCE_ONCE)
			UTIL_Remove(this);
		SetThink(&CScriptedSentence::DelayThink);
		pev->nextthink = gpGlobals->time + m_flDuration + m_flRepeat;
		m_active = FALSE;
		//		ALERT( at_console, "%s: found monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
	}
	else
	{
		//		ALERT( at_console, "%s: can't find monster %s\n", STRING(m_iszSentence), STRING(m_iszEntity) );
		pev->nextthink = gpGlobals->time + m_flRepeat + 0.5;
	}
}


void CScriptedSentence::DelayThink(void)
{
	m_active = TRUE;
	if (!pev->targetname)
		pev->nextthink = gpGlobals->time + 0.1;
	SetThink(&CScriptedSentence::FindThink);
}


BOOL CScriptedSentence::AcceptableSpeaker(CBaseMonster* pMonster)
{
	if (pMonster)
	{
		if (pev->spawnflags & SF_SENTENCE_FOLLOWERS)
		{
			if (pMonster->m_hTargetEnt == NULL || !FClassnameIs(pMonster->m_hTargetEnt->pev, "player"))
				return FALSE;
		}
		BOOL override;
		if (pev->spawnflags & SF_SENTENCE_INTERRUPT)
			override = TRUE;
		else
			override = FALSE;
		if (pMonster->CanPlaySentence(override))
			return TRUE;
	}
	return FALSE;
}


CBaseMonster* CScriptedSentence::FindEntity(void)
{
	CBaseEntity* pentTarget;
	CBaseMonster* pMonster;


	pentTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_iszEntity));
	pMonster = NULL;

	while (pentTarget)
	{
		pMonster = pentTarget->MyMonsterPointer();
		if (pMonster != NULL)
		{
			if (AcceptableSpeaker(pMonster))
				return pMonster;
			//			ALERT( at_console, "%s (%s), not acceptable\n", STRING(pMonster->pev->classname), STRING(pMonster->pev->targetname) );
		}
		pentTarget = UTIL_FindEntityByTargetname(pentTarget, STRING(m_iszEntity));
	}

	CBaseEntity* pEntity = NULL;
	while ((pEntity = UTIL_FindEntityInSphere(pEntity, pev->origin, m_flRadius)) != NULL)
	{
		if (FClassnameIs(pEntity->pev, STRING(m_iszEntity)))
		{
			if (FBitSet(pEntity->pev->flags, FL_MONSTER))
			{
				pMonster = pEntity->MyMonsterPointer();
				if (AcceptableSpeaker(pMonster))
					return pMonster;
			}
		}
	}

	return NULL;
}


BOOL CScriptedSentence::StartSentence(CBaseMonster* pTarget)
{
	if (!pTarget)
	{
		ALERT(at_aiconsole, "Not Playing sentence %s\n", STRING(m_iszSentence));
		return FALSE;
	}

	BOOL bConcurrent = FALSE;
	if (!(pev->spawnflags & SF_SENTENCE_CONCURRENT))
		bConcurrent = TRUE;

	CBaseEntity* pListener = NULL;
	if (!FStringNull(m_iszListener))
	{
		float radius = m_flRadius;

		if (FStrEq(STRING(m_iszListener), "player"))
			radius = 4096;	// Always find the player

		pListener = UTIL_FindEntityGeneric(STRING(m_iszListener), pTarget->pev->origin, radius);
	}

	pTarget->PlayScriptedSentence(STRING(m_iszSentence), m_flDuration, m_flVolume, m_flAttenuation, bConcurrent, pListener);
	ALERT(at_aiconsole, "Playing sentence %s (%.1f)\n", STRING(m_iszSentence), m_flDuration);
	SUB_UseTargets(NULL, USE_TOGGLE, 0);
	return TRUE;
}
