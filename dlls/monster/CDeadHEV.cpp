#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "trains.h"
#include "nodes.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"

//=========================================================
// Dead HEV suit prop
//=========================================================
class CDeadHEV : public CBaseMonster
{
public:
	void Spawn(void);
	int	Classify(void) { return	CBaseMonster::Classify(CLASS_HUMAN_MILITARY); }
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }

	void KeyValue(KeyValueData* pkvd);

	int	m_iPose;// which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[4];
};

const char* CDeadHEV::m_szPoses[] = { "deadback", "deadsitting", "deadstomach", "deadtable" };

void CDeadHEV::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(monster_hevsuit_dead, CDeadHEV)

//=========================================================
// ********** DeadHEV SPAWN **********
//=========================================================
void CDeadHEV::Spawn(void)
{
	m_defaultModel = "models/player.mdl";
	PRECACHE_MODEL(GetModel());
	InitModel();

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	pev->body = 1;
	m_bloodColor = BLOOD_COLOR_RED;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead hevsuit with bad pose\n");
		pev->sequence = 0;
		//pev->effects = EF_BRIGHTFIELD;
	}

	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();
}

