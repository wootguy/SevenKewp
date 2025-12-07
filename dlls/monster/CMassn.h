#pragma once
#include "extdll.h"
#include "util.h"
#include "CBaseGrunt.h"

#define	MASSASSIN_MP5_CLIP_SIZE			36 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define MASSN_SNIPER_CLIP_SIZE			1
#define MASSN_FOLLOW_SOUND "massn/yeah.wav"
#define MASSN_UNFOLLOW_SOUND "massn/covered.wav"

namespace MAssassinBodygroup
{
enum MAssassinBodygroup
{
	Heads = 1,
	Weapons = 2
};
}

namespace MAssassinHead
{
enum MAssassinHead
{
	Random = -1,
	White = 0,
	Black,
	ThermalVision
};
}

namespace MAssassinWeapon
{
enum MAssassinWeapon
{
	MP5 = 0,
	SniperRifle,
	None
};
}

namespace MAssassinWeaponFlag
{
enum MAssassinWeaponFlag
{
	MP5 = 1 << 0,
	HandGrenade = 1 << 1,
	GrenadeLauncher = 1 << 2,
	SniperRifle = 1 << 3,
};
}

class EXPORT CMassn : public CBaseGrunt
{
public:
	void Spawn( void );
	void Precache( void );
	const char* DisplayName() { return m_displayName ? CBaseMonster::DisplayName() : "Male Assassin"; }
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int GetActivitySequence(Activity NewActivity);
	void DeathSound( void );
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	void KeyValue( KeyValueData* pkvd ) override;

	float m_flStandGroundRange;

	int m_iAssassinHead;
	int m_weaponModel;

private:
	static const char* pDieSounds[];
};

class CMassnRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_male_assassin"; };
};

class CDeadMassn : public CBaseDead
{
public:
	void Spawn(void);
	int	Classify(void) { return	CBaseMonster::Classify(CLASS_HUMAN_MILITARY); }
	int GetPoseSequence() { return LookupSequence(m_szPoses[clamp(m_iPose, 0, (int)ARRAY_SZ(m_szPoses) - 1)]); }

	static const char* m_szPoses[3];
};
