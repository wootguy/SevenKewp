#include"extdll.h"
#include "plane.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "CTalkSquadMonster.h"
#include "weapons.h"
#include "CTalkSquadMonster.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
#include "CBaseGrunt.h"


#define	MASSASSIN_MP5_CLIP_SIZE			36 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define MASSN_SNIPER_CLIP_SIZE			1

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

class CMassn : public CBaseGrunt
{
public:
	void Spawn( void );
	void Precache( void );
	const char* DisplayName() { return m_displayName ? CBaseMonster::DisplayName() : "Male Assassin"; }
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	int GetActivitySequence(Activity NewActivity);
	void DeathSound( void );

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	void KeyValue( KeyValueData* pkvd ) override;

	float m_flStandGroundRange;

	int m_iAssassinHead;

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
	int GetPoseSequence() { return LookupSequence(m_szPoses[m_iPose]); }

	static const char* m_szPoses[3];
};

LINK_ENTITY_TO_CLASS(monster_male_assassin, CMassn);
LINK_ENTITY_TO_CLASS(monster_assassin_repel, CMassnRepel);
LINK_ENTITY_TO_CLASS(monster_massassin_dead, CDeadMassn);

const char* CMassn::pDieSounds[] =
{
	"hgrunt/gr_die1.wav",
	"hgrunt/gr_die2.wav",
	"hgrunt/gr_die3.wav",
};

const char* CDeadMassn::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

void CMassn :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	// check for helmet shot
	if (ptr->iHitgroup == 11)
	{
		// it's head shot anyways
		ptr->iHitgroup = HITGROUP_HEAD;
	}

	CTalkSquadMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

void CMassn :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case HGRUNT_AE_DROP_GUN:
			if (DropEquipment(0, false))
				SetBodygroup( MAssassinBodygroup::Weapons, MAssassinWeapon::None );
			break;

		default:
			CBaseGrunt::HandleAnimEvent( pEvent );
			break;
	}
}

void CMassn :: Spawn()
{
	if (!pev->health) pev->health = gSkillData.massassinHealth;
	BaseSpawn();

	if( m_iAssassinHead == MAssassinHead::Random )
	{
		m_iAssassinHead = RANDOM_LONG( MAssassinHead::White, MAssassinHead::ThermalVision );
	}

	if (pev->weapons == 0) { // default equipment
		pev->weapons = MAssassinWeaponFlag::MP5 | MAssassinWeaponFlag::HandGrenade;
	}

	auto weaponModel = MAssassinWeapon::None;

	if (FBitSet( pev->weapons, MAssassinWeaponFlag::MP5 ))
	{
		weaponModel = MAssassinWeapon::MP5;
		m_cClipSize = MASSASSIN_MP5_CLIP_SIZE;
	}
	else if( FBitSet( pev->weapons, MAssassinWeaponFlag::SniperRifle ) )
	{
		weaponModel = MAssassinWeapon::SniperRifle;
		m_cClipSize = MASSN_SNIPER_CLIP_SIZE;
		m_flDistTooFar = 4096.0;
		m_flDistLook = 4096.0;
		maxShootDist = 4096;
	}
	else
	{
		weaponModel = MAssassinWeapon::None;
		m_cClipSize = 0;
		runFromHeavyDamage = false;
	}

	SetBodygroup( MAssassinBodygroup::Heads, m_iAssassinHead );
	SetBodygroup( MAssassinBodygroup::Weapons, weaponModel );

	m_cAmmoLoaded		= m_cClipSize;

	m_flLastShot = gpGlobals->time;

	pev->skin = 0;

	// get voice pitch
	if (RANDOM_LONG(0, 1))
		m_voicePitch = 109 + RANDOM_LONG(0, 7);
	else
		m_voicePitch = 100;

	// set base equipment flags
	if (FBitSet(pev->weapons, MAssassinWeaponFlag::MP5)) {
		m_iEquipment |= MEQUIP_MP5;
	}
	if (FBitSet(pev->weapons, MAssassinWeaponFlag::SniperRifle)) {
		m_iEquipment |= MEQUIP_SNIPER;
	}
	if (FBitSet(pev->weapons, MAssassinWeaponFlag::HandGrenade)) {
		m_iEquipment |= MEQUIP_HAND_GRENADE;
	}
	if (FBitSet(pev->weapons, MAssassinWeaponFlag::GrenadeLauncher)) {
		m_iEquipment |= MEQUIP_GRENADE_LAUNCHER;
	}
}

void CMassn :: Precache()
{
	BasePrecache();
	m_defaultModel = "models/massn.mdl";
	PRECACHE_MODEL(GetModel());
	
	PRECACHE_SOUND_ARRAY(pDieSounds);
}	

void CMassn :: DeathSound ( void )
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDieSounds), 1, ATTN_IDLE);
}

int CMassn::GetActivitySequence(Activity NewActivity) {
	int iSequence = ACTIVITY_NOT_AVAILABLE;

	switch (NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// Sniper uses the same animations for mp5 and sniper
		if (m_fStanding)
		{
			// get aimable sequence
			iSequence = LookupSequence("standing_mp5");
		}
		else
		{
			// get crouching shoot
			iSequence = LookupSequence("crouching_mp5");
		}
		break;
	default:
		iSequence = CBaseGrunt::GetActivitySequence(NewActivity);
		break;
	}

	return iSequence;
}

void CMassn::KeyValue( KeyValueData* pkvd )
{
	if( FStrEq( "head", pkvd->szKeyName ) )
	{
		m_iAssassinHead = atoi( pkvd->szValue );
		pkvd->fHandled = true;
	}
	else if( FStrEq( "standgroundrange", pkvd->szKeyName ) )
	{
		m_flStandGroundRange = atof( pkvd->szValue );
		pkvd->fHandled = true;
	}
	else
		CTalkSquadMonster::KeyValue( pkvd );
}


void CDeadMassn::Spawn(void)
{
	CBaseDead::BaseSpawn("models/hgrunt.mdl");
	m_bloodColor = BLOOD_COLOR_RED;

	// map old bodies onto new bodies
	//TODO: verify these
	switch( pev->body )
	{
	case 0: // Grunt with Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup( MAssassinBodygroup::Heads, MAssassinHead::White );
		SetBodygroup( MAssassinBodygroup::Weapons, MAssassinWeapon::MP5 );
		break;
	case 1: // Commander with Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup( MAssassinBodygroup::Heads, MAssassinHead::Black );
		SetBodygroup( MAssassinBodygroup::Weapons, MAssassinWeapon::MP5 );
		break;
	case 2: // Grunt no Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup( MAssassinBodygroup::Heads, MAssassinHead::White );
		SetBodygroup( MAssassinBodygroup::Weapons, MAssassinWeapon::SniperRifle );
		break;
	case 3: // Commander no Gun
		pev->body = 0;
		pev->skin = 0;
		SetBodygroup( MAssassinBodygroup::Heads, MAssassinHead::White );
		SetBodygroup( MAssassinBodygroup::Weapons, MAssassinWeapon::None );
		break;
	}
}