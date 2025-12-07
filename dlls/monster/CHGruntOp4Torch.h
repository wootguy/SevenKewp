#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseGruntOp4.h"
#include "monsters.h"

#define	TORCH_DEAGLE_CLIP_SIZE			8 // how many bullets in a clip?
#define TORCH_BEAM_SPRITE "sprites/xbeam3.spr"

namespace TorchAllyBodygroup
{
enum TorchAllyBodygroup
{
	Weapons = 1
};
}

namespace TorchAllyWeapon
{
enum TorchAllyWeapon
{
	DesertEagle = 0,
	Torch,
	None
};
}

namespace TorchAllyWeaponFlag
{
enum TorchAllyWeaponFlag
{
	DesertEagle = 1 << 0,
	Torch = 1 << 1,
	HandGrenade = 1 << 2,
};
}

#define TORCH_AE_HOLSTER_TORCH		17
#define TORCH_AE_HOLSTER_GUN		18
#define TORCH_AE_HOLSTER_BOTH		19
#define TORCH_AE_ACTIVATE_TORCH		20
#define TORCH_AE_DEACTIVATE_TORCH	21

class EXPORT COFTorchAlly : public CBaseGruntOp4
{
public:
	void Spawn( void );
	void Precache( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int GetActivitySequence(Activity NewActivity);
	void GibMonster( void );

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	
	Schedule_t  *GetScheduleOfType ( int Type );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	void Killed( entvars_t* pevAttacker, int iGib ) override;

	void MonsterThink() override;

	MONSTERSTATE GetIdealState()
	{
		return CTalkSquadMonster::GetIdealState();
	}

	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_fGunDrawn;
	float m_painTime;
	float m_checkAttackTime;

	float m_flPlayerDamage;

	BOOL m_fUseTorch;
	EHANDLE m_hNewTargetEnt;
	int m_iBlackOrWhite;
	BOOL m_fGunHolstered;
	BOOL m_fTorchHolstered;
	BOOL m_fTorchActive;

	EHANDLE m_hTorchBeam;

	int m_weaponIndex;
};

class COFTorchAllyRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_human_torch_ally"; };
};
