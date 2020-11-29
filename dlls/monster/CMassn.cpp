#include"extdll.h"
#include "plane.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "CSquadMonster.h"
#include "weapons.h"
#include "CTalkMonster.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
#include "CBaseHGrunt.h"


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

class CMassn : public CBaseHGrunt
{
public:
	void Spawn( void );
	void Precache( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	void CheckAmmo ( void );
	int GetActivitySequence(Activity NewActivity);
	void DeathSound( void );
	void Shoot ( void );
	CBaseEntity* DropGun(Vector pos, Vector angles);

	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);

	void KeyValue( KeyValueData* pkvd ) override;

	float m_flLastShot;
	float m_flStandGroundRange;

	int m_iAssassinHead;
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
	int	Classify(void) { return	CLASS_HUMAN_MILITARY; }
	int GetPoseSequence() { return LookupSequence(m_szPoses[m_iPose]); }

	static const char* m_szPoses[3];
};

LINK_ENTITY_TO_CLASS(monster_male_assassin, CMassn);
LINK_ENTITY_TO_CLASS(monster_assassin_dead, CMassnRepel);
LINK_ENTITY_TO_CLASS(monster_massassin_dead, CDeadMassn);

const char* CDeadMassn::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

CBaseEntity* CMassn::DropGun(Vector pos, Vector angles) {
	if (FBitSet(pev->weapons, MAssassinWeaponFlag::MP5))
	{
		return DropItem("weapon_9mmAR", pos, angles);
	}
	else
	{
		return DropItem("weapon_sniperrifle", pos, angles);
	}
}

BOOL CMassn :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if( pev->weapons )
	{
		return CBaseHGrunt::CheckRangeAttack1(flDot, flDist);
	}

	return FALSE;
}

void CMassn :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	// check for helmet shot
	if (ptr->iHitgroup == 11)
	{
		// it's head shot anyways
		ptr->iHitgroup = HITGROUP_HEAD;
	}

	CSquadMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

void CMassn :: CheckAmmo ( void )
{
	if ( pev->weapons && m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

void CMassn :: Shoot ( void )
{
	if (m_hEnemy == NULL)
	{
		return;
	}

	if( FBitSet( pev->weapons, MAssassinWeaponFlag::SniperRifle ) && gpGlobals->time - m_flLastShot <= 0.11 )
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	if( FBitSet( pev->weapons, MAssassinWeaponFlag::MP5 ) )
	{
		Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT( 40, 90 ) + gpGlobals->v_up * RANDOM_FLOAT( 75, 200 ) + gpGlobals->v_forward * RANDOM_FLOAT( -40, 40 );
		EjectBrass( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL );
		FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, BULLET_MONSTER_MP5 ); // shoot +-5 degrees
	}
	else
	{
		//TODO: why is this 556? is 762 too damaging?
		FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 2048, BULLET_PLAYER_556 );
	}

	pev->effects |= EF_MUZZLEFLASH;
	
	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
}

void CMassn :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;

	switch( pEvent->event )
	{
		case HGRUNT_AE_DROP_GUN:
			{
			Vector	vecGunPos;
			Vector	vecGunAngles;

			GetAttachment( 0, vecGunPos, vecGunAngles );

			// switch to body group with no gun.
			SetBodygroup( MAssassinBodygroup::Weapons, MAssassinWeapon::None );

			// now spawn a gun.
			if (FBitSet( pev->weapons, MAssassinWeaponFlag::MP5 ))
			{
				 DropItem( "weapon_9mmAR", vecGunPos, vecGunAngles );
			}
			else
			{
				 DropItem( "weapon_sniperrifle", vecGunPos, vecGunAngles );
			}
			if (FBitSet( pev->weapons, MAssassinWeaponFlag::GrenadeLauncher ))
			{
				DropItem( "ammo_ARgrenades", BodyTarget( pev->origin ), vecGunAngles );
			}

			}
			break;

		case HGRUNT_AE_BURST1:
		{
			Shoot();

			if ( FBitSet( pev->weapons, MAssassinWeaponFlag::MP5 ))
			{
				// the first round of the three round burst plays the sound and puts a sound in the world sound list.
				if ( RANDOM_LONG(0,1) )
				{
					EMIT_SOUND( ENT(pev), CHAN_WEAPON, "hgrunt/gr_mgun1.wav", 1, ATTN_NORM );
				}
				else
				{
					EMIT_SOUND( ENT(pev), CHAN_WEAPON, "hgrunt/gr_mgun2.wav", 1, ATTN_NORM );
				}
			}
			else
			{
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sniper_fire.wav", 1, ATTN_NORM );
			}
		
			CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
		}
		break;

		default:
			CBaseHGrunt::HandleAnimEvent( pEvent );
			break;
	}
}

void CMassn :: Spawn()
{
	BaseSpawn("models/massn.mdl");

	pev->health			= gSkillData.massassinHealth;

	if( m_iAssassinHead == MAssassinHead::Random )
	{
		m_iAssassinHead = RANDOM_LONG( MAssassinHead::White, MAssassinHead::ThermalVision );
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
	}
	else
	{
		weaponModel = MAssassinWeapon::None;
		m_cClipSize = 0;
	}

	SetBodygroup( MAssassinBodygroup::Heads, m_iAssassinHead );
	SetBodygroup( MAssassinBodygroup::Weapons, weaponModel );

	m_cAmmoLoaded		= m_cClipSize;

	m_flLastShot = gpGlobals->time;

	pev->skin = 0;

	MonsterInit();

	// get voice pitch
	if (RANDOM_LONG(0, 1))
		m_voicePitch = 109 + RANDOM_LONG(0, 7);
	else
		m_voicePitch = 100;
}

void CMassn :: Precache()
{
	PRECACHE_MODEL("models/massn.mdl");
	
	PRECACHE_SOUND( "hgrunt/gr_die1.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die2.wav" );
	PRECACHE_SOUND( "hgrunt/gr_die3.wav" );

	BasePrecache();
}	

void CMassn :: DeathSound ( void )
{
	switch ( RANDOM_LONG(0,2) )
	{
	case 0:	
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_die1.wav", 1, ATTN_IDLE );	
		break;
	case 1:
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_die2.wav", 1, ATTN_IDLE );	
		break;
	case 2:
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_die3.wav", 1, ATTN_IDLE );	
		break;
	}
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
		iSequence = CBaseHGrunt::GetActivitySequence(NewActivity);
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
		CSquadMonster::KeyValue( pkvd );
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