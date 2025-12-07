#pragma once
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "CBaseGruntOp4.h"

#define	MEDIC_DEAGLE_CLIP_SIZE			9 // how many bullets in a clip?
#define	MEDIC_GLOCK_CLIP_SIZE			9 // how many bullets in a clip?
#define TORCH_BEAM_SPRITE "sprites/xbeam3.spr"

namespace MedicAllyBodygroup
{
enum MedicAllyBodygroup
{
	Head = 2,
	Weapons = 3
};
}

namespace MedicAllyHead
{
enum MedicAllyHead
{
	Default = -1,
	White = 0,
	Black
};
}

namespace MedicAllyWeapon
{
enum MedicAllyWeapon
{
	DesertEagle = 0,
	Glock,
	Needle,
	None
};
}

namespace MedicAllyWeaponFlag
{
enum MedicAllyWeaponFlag
{
	DesertEagle = 1 << 0,
	Glock = 1 << 1,
	Needle = 1 << 2,
	HandGrenade = 1 << 3,
};
}

#define	MEDIC_AE_SHOOT			4
#define MEDIC_AE_HOLSTER_GUN	15
#define MEDIC_AE_EQUIP_NEEDLE	16
#define MEDIC_AE_HOLSTER_NEEDLE	17
#define MEDIC_AE_EQUIP_GUN		18

enum
{
	SCHED_MEDIC_ALLY_HEAL_ALLY = LAST_BASE_GRUNT_SCHEDULE + 1,
};

class EXPORT CHGruntOp4Medic : public CBaseGruntOp4
{
public:
	void Spawn( void );
	void Precache( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	void GibMonster( void );

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	
	int GetActivitySequence(Activity NewActivity);
	Schedule_t	*GetSchedule( void );
	Schedule_t  *GetScheduleOfType ( int Type );
	
	int ObjectCaps() override;

	void KeyValue( KeyValueData *pkvd ) override;

	void Killed( entvars_t* pevAttacker, int iGib ) override;

	void MonsterThink() override;

	BOOL HealMe( CTalkSquadMonster* pTarget );

	void HealOff();

	void HealerUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );

	void HealerActivate( CBaseMonster* pTarget );

	MONSTERSTATE GetIdealState()
	{
		return CTalkSquadMonster::GetIdealState();
	}

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	int m_iHealCharge;
	BOOL m_fUseHealing;
	BOOL m_fHealing;

	float m_flLastUseTime;

	EHANDLE m_hNewTargetEnt;

	BOOL m_fQueueFollow;
	BOOL m_fHealAudioPlaying;

	float m_flFollowCheckTime;
	BOOL m_fFollowChecking;
	BOOL m_fFollowChecked;

	float m_flLastRejectAudio;

	int m_iBlackOrWhite;

	BOOL m_fGunHolstered;
	BOOL m_fHypoHolstered;
	BOOL m_fHealActive;

	int m_iWeaponIdx;
};

class CHGruntOp4MedicRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_human_medic_ally"; };
};
