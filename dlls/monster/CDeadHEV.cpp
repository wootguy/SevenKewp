#include "extdll.h"
#include "util.h"
#include "trains.h"
#include "nodes.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "CDeadHEV.h"

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
	m_bloodColor = BloodColorHuman();

	pev->sequence = LookupSequence(m_szPoses[clamp(m_iPose, 0, (int)ARRAY_SZ(m_szPoses) - 1)]);

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

