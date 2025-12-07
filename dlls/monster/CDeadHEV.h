#pragma once
#include "extdll.h"
#include "CBaseMonster.h"

//=========================================================
// Dead HEV suit prop
//=========================================================
class EXPORT CDeadHEV : public CBaseMonster
{
public:
	void Spawn(void);
	int	Classify(void) { return	CBaseMonster::Classify(CLASS_HUMAN_MILITARY); }
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }

	void KeyValue(KeyValueData* pkvd);

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[4];
};
