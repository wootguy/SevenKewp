#include "extdll.h"
#include "eiface.h"
#include "util.h"
#include "gamerules.h"
#include "cbase.h"
#include "CRuleEntity.h"

//
// CTriggerRandom / trigger_random -- fires a random target

#define SF_START_ON				1
#define SF_TRIGGER_ONCE			2
#define SF_REUSABLE				4
#define SF_TIMED				8
#define SF_UNIQUE				16

#define MAX_RANDOM_TARGETS 16

class CTriggerRandom : public CPointEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	string_t targets[MAX_RANDOM_TARGETS];
	bool firedTargets[MAX_RANDOM_TARGETS];

	int target_count;
	float min_delay;
	float max_delay;
	bool isActive;
};

LINK_ENTITY_TO_CLASS(trigger_random, CTriggerRandom);
LINK_ENTITY_TO_CLASS(trigger_random_unique, CTriggerRandom);

void CTriggerRandom::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "target_count"))
	{
		target_count = clampi(atoi(pkvd->szValue), 0, MAX_RANDOM_TARGETS);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "min_delay"))
	{
		ALERT(at_console, "trigger_random: Timed mode not implemented\n");
		min_delay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "max_delay"))
	{
		
		max_delay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		static const char* targetKeys[MAX_RANDOM_TARGETS] = {
			"target1", "target2", "target3", "target4",
			"target5", "target6", "target7", "target8",
			"target9", "target10", "target11", "target12",
			"target13", "target14", "target15", "target16"
		};

		for (int i = 0; i < MAX_RANDOM_TARGETS; i++) {
			if (FStrEq(pkvd->szKeyName, targetKeys[i]))
			{
				targets[i] = ALLOC_STRING(pkvd->szValue);
				pkvd->fHandled = TRUE;
				return;
			}
		}

		CPointEntity::KeyValue(pkvd);
	}
}

void CTriggerRandom::Spawn(void)
{
	CPointEntity::Spawn();

	if (!strcmp(STRING(pev->classname), "trigger_random_unique")) {
		if (pev->spawnflags & 1) {
			// re-usable. Convert to trigger_random's reusable flag
			pev->spawnflags &= ~1;
			pev->spawnflags |= SF_REUSABLE;
		}
		pev->spawnflags |= SF_UNIQUE;
	}
	
	if (pev->spawnflags & (SF_START_ON | SF_TRIGGER_ONCE | SF_TIMED)) {
		ALERT(at_error, "trigger_random: timed mode not implemented\n");
	}
}


void CTriggerRandom::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	isActive = useType == USE_TOGGLE ? !isActive : useType;

	if ((pev->spawnflags & SF_TIMED) == 0) {
		int randomIdx;

		if (pev->spawnflags & SF_UNIQUE) {
			int remainingTargetCount = 0;
			int remainingTargets[16];
			for (int i = 0; i < target_count; i++) {
				if (!firedTargets[i]) {
					remainingTargets[remainingTargetCount++] = i;
				}
			}

			if (remainingTargetCount == 0) {
				if (pev->spawnflags & SF_REUSABLE) {
					memset(firedTargets, 0, sizeof(bool) * MAX_RANDOM_TARGETS);
					randomIdx = RANDOM_LONG(0, target_count - 1);
				}
				else {
					return; // fired all targets and is not flagged as reusable
				}
			}
			else {
				randomIdx = remainingTargets[RANDOM_LONG(0, remainingTargetCount - 1)];
			}
		}
		else {
			randomIdx = RANDOM_LONG(0, target_count - 1);
		}

		string_t target = targets[randomIdx];
		firedTargets[randomIdx] = true;

		if (target) {
			FireTargets(STRING(target), pActivator, this, useType, value);
		}
	}
}
