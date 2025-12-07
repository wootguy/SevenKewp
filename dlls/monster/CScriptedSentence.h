#pragma once
#include "extdll.h"
#include "util.h"
#include "CCineMonster.h"

#define SF_SENTENCE_ONCE		0x0001
#define SF_SENTENCE_FOLLOWERS	0x0002	// only say if following player
#define SF_SENTENCE_INTERRUPT	0x0004	// force talking except when dead
#define SF_SENTENCE_CONCURRENT	0x0008	// allow other people to keep talking

class EXPORT CScriptedSentence : public CBaseToggle
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void Spawn(void);
	void Precache();
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void FindThink(void);
	void DelayThink(void);
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
