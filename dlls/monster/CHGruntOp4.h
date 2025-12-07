#pragma once
#include "extdll.h"
#include "util.h"
#include "monsters.h"
#include "CBaseGruntOp4.h"

#define	GRUNT_MP5_CLIP_SIZE				36 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define GRUNT_SHOTGUN_CLIP_SIZE			8
#define GRUNT_SAW_CLIP_SIZE				36

namespace HGruntAllyWeaponFlag
{
enum HGruntAllyWeaponFlag
{
	MP5 = 1 << 0,
	HandGrenade = 1 << 1,
	GrenadeLauncher = 1 << 2,
	Shotgun = 1 << 3,
	Saw = 1 << 4
};
}

namespace HGruntAllyBodygroup
{
enum HGruntAllyBodygroup
{
	Head = 1,
	Torso = 2,
	Weapons = 3
};
}

namespace HGruntAllyHead
{
enum HGruntAllyHead
{
	Default = -1,
	GasMask = 0,
	BeretWhite,
	OpsMask,
	BandanaWhite,
	BandanaBlack,
	MilitaryPolice,
	Commander,
	BeretBlack,
};
}

namespace HGruntAllyTorso
{
enum HGruntAllyTorso
{
	Normal = 0,
	Saw,
	Nothing,
	Shotgun
};
}

namespace HGruntAllyWeapon
{
enum HGruntAllyWeapon
{
	MP5 = 0,
	Shotgun,
	Saw,
	None
};
}

class EXPORT CHGruntOp4 : public CBaseGruntOp4
{
public:
	void Spawn( void );
	virtual void Precache( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void GibMonster( void );

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	
	int GetActivitySequence(Activity NewActivity);
	Schedule_t  *GetScheduleOfType ( int Type );

	void KeyValue( KeyValueData* pkvd ) override;

	MONSTERSTATE GetIdealState()
	{
		return CTalkSquadMonster::GetIdealState();
	}

	static TYPEDESCRIPTION m_SaveData[];

	int m_iWeaponIdx;
	int m_iGruntHead;
	int m_iGruntTorso;
};

class CHGruntOp4Repel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_human_grunt_ally"; };
};

class CDeadHGruntAlly : public CBaseDead
{
public:
	void Spawn(void);
	int	Classify(void) { return	CBaseMonster::Classify(CLASS_PLAYER_ALLY); }
	int GetPoseSequence() { return LookupSequence(m_szPoses[clamp(m_iPose, 0, (int)ARRAY_SZ(m_szPoses) - 1)]); }

	static const char* m_szPoses[7];
};
