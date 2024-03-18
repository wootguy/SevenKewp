#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "user_messages.h"
#include "weapons.h"

// TODO:
// - blinking
// - vanilla HL smoke clouds

#define TOXIC_SOUND "ambience/disgusting.wav"
#define TOXIC_SOUND2 "doors/aliendoor1.wav"
#define TOXIC_SPRITE "sprites/puff1.spr"

#define TOXIC_START_DISTANCE 200

#define SMOKE_TIME 3.0f

enum
{
	TASK_START_CLOUD = LAST_COMMON_TASK + 1,
	TASK_STOP_CLOUD,
};

class CChumtoad : public CBaseMonster
{
public:
	void Spawn(void);
	void Precache(void);
	int Classify(void);
	const char* DisplayName();
	void SetYawSpeed(void);
	void HandleAnimEvent(MonsterEvent_t* pEvent);
	void PrescheduleThink();
	void StartTask(Task_t* pTask);
	Schedule_t* GetScheduleOfType(int Type);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	BOOL CheckRangeAttack1(float flDot, float flDist) { return FALSE; }
	BOOL CheckRangeAttack2(float flDot, float flDist) { return FALSE; }
	BOOL CheckMeleeAttack1(float flDot, float flDist);

	CUSTOM_SCHEDULES;

	float nextCloudEmit;
	int smokeColor;
	bool stopSmoking;
	int m_iSmokeSpr;
};

LINK_ENTITY_TO_CLASS(monster_chumtoad, CChumtoad);

void CChumtoad::Spawn()
{
	Precache();

	m_skinFrames = 3;
	InitModel();
	SetSize(Vector(-16, -16, 0), Vector(16, 12, 36));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->view_ofs = Vector(0, 0, 0);// position of the eyes relative to monster's origin.
	m_MonsterState = MONSTERSTATE_NONE;
	m_afCapability = bits_CAP_MELEE_ATTACK1;

	MonsterInit();

	stopSmoking = false;
}

void CChumtoad::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/chumtoad.mdl";
	PRECACHE_MODEL(GetModel());
	m_iSmokeSpr = PRECACHE_MODEL(TOXIC_SPRITE);
	PRECACHE_SOUND(TOXIC_SOUND);
	PRECACHE_SOUND(TOXIC_SOUND2);
}

int	CChumtoad::Classify(void)
{
	return CBaseMonster::Classify(CLASS_ALIEN_MONSTER);
}

const char* CChumtoad::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Chumtoad";
}

void CChumtoad::SetYawSpeed(void)
{
	int ys;

	switch (m_Activity)
	{
	case ACT_IDLE:
	default:
		ys = 90;
	}

	pev->yaw_speed = ys * gSkillData.sk_yawspeed_mult;
}

void CChumtoad::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	switch (pEvent->event)
	{
	case 0:
	default:
		CBaseMonster::HandleAnimEvent(pEvent);
		break;
	}
}

Task_t	tlToxicCloud[] =
{
	{ TASK_STOP_MOVING,		(float)0	},
	{ TASK_PLAY_SEQUENCE,	(float)ACT_FEAR_DISPLAY	},
	{ TASK_START_CLOUD,		(float)0	},
	{ TASK_PLAY_SEQUENCE,	(float)ACT_TWITCH	},
	{ TASK_WAIT,			(float)SMOKE_TIME},
	{ TASK_START_CLOUD,		(float)0	},
	{ TASK_WAIT,			(float)SMOKE_TIME	},
	{ TASK_START_CLOUD,		(float)0	},
	{ TASK_WAIT,			(float)SMOKE_TIME	},
	{ TASK_START_CLOUD,		(float)0	},
	{ TASK_WAIT,			(float)SMOKE_TIME	},
	{ TASK_STOP_CLOUD,		(float)0	},
	{ TASK_WAIT,			(float)SMOKE_TIME	},
	{ TASK_PLAY_SEQUENCE,	(float)ACT_STAND	},
};

Schedule_t	slToxicCloud[] =
{
	{
		tlToxicCloud,
		ARRAYSIZE(tlToxicCloud),
		0,

		0,
		"Toxic cloud"
	},
};

DEFINE_CUSTOM_SCHEDULES(CChumtoad)
{
	slToxicCloud
};

IMPLEMENT_CUSTOM_SCHEDULES(CChumtoad, CBaseMonster);

void CChumtoad::PrescheduleThink() {
	if (m_Activity == ACT_TWITCH && !stopSmoking) {
		m_fSequenceLoops = true;

		if (gpGlobals->time >= nextCloudEmit) {
			nextCloudEmit = gpGlobals->time + 0.1f;
			pev->nextthink = nextCloudEmit;

			::RadiusDamage(pev->origin, pev, pev, 15.0f, 256, CLASS_ALIEN_MONSTER, DMG_NERVEGAS);

			// in sven, it might be message per X seconds of smoke, rather than one message per particle. 
			// Since the message is so small I don't think it matters. Need to test if this works similarly
			// when monster is on the border of the PVS. Maybe sven sends these messages globally to prevent
			// glitches.
			/*
			MESSAGE_BEGIN(MSG_PVS, gmsgToxicCloud, pev->origin);
				WRITE_SHORT(entindex()); // entity to spawn the effect from
				WRITE_BYTE(RANDOM_LONG(0, 255)); // seed for random velocity calculated on the client
				WRITE_BYTE(smokeColor); // color brightness
			MESSAGE_END();
			*/

			// using vanilla HL effects until this mod is standalone
			Vector pos = pev->origin + Vector(RANDOM_FLOAT(pev->mins.x, pev->maxs.x)*8, RANDOM_FLOAT(pev->mins.y, pev->maxs.y)*8, 1.0f);
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(pos.x);
			WRITE_COORD(pos.y);
			WRITE_COORD(pos.z);
			WRITE_SHORT(m_iSmokeSpr);
			WRITE_BYTE(RANDOM_LONG(80, 120));
			WRITE_BYTE(RANDOM_LONG(15, 25));
			MESSAGE_END();
		}
	}
}

void CChumtoad::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (pTask->iTask)
	{
	case TASK_START_CLOUD:
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, TOXIC_SOUND, 1.0, ATTN_NORM, 0, 100);
		EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, TOXIC_SOUND2, 1.0, ATTN_NORM, 0, 100);
		smokeColor = RANDOM_LONG(60, 100);
		stopSmoking = false;
		TaskComplete();
		break;
	case TASK_STOP_CLOUD:
		stopSmoking = true;
		TaskComplete();
		break;
	case TASK_PLAY_SEQUENCE:
		CBaseMonster::StartTask(pTask);
		if ((int)pTask->flData == ACT_TWITCH) {
			TaskComplete(); // keeps timing consistent between smoke tasks
		}
		break;
	default:
		CBaseMonster::StartTask(pTask);
		break;
	}
}

Schedule_t* CChumtoad::GetScheduleOfType(int Type) {
	switch(Type) {
	case SCHED_MELEE_ATTACK1:
		m_fSequenceLoops = true;
		nextCloudEmit = 0;
		return &slToxicCloud[0];
		
	default:
		return CBaseMonster::GetScheduleOfType(Type);
	}
}

BOOL CChumtoad::CheckMeleeAttack1(float flDot, float flDist) {
	if (flDist < TOXIC_START_DISTANCE) {
		return TRUE;
	}
	return FALSE;
}
