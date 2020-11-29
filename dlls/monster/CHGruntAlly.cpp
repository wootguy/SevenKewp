#include "extdll.h"
#include "plane.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "defaultai.h"
#include "animation.h"
#include "weapons.h"
#include "CTalkSquadMonster.h"
#include "CBaseGruntAlly.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"

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

class CHGruntAlly : public CBaseGruntAlly
{
public:
	void Spawn( void );
	void Precache( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	void GibMonster( void );

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	
	int GetActivitySequence(Activity NewActivity);
	Schedule_t  *GetScheduleOfType ( int Type );

	void ShootSaw();

	void KeyValue( KeyValueData* pkvd ) override;

	MONSTERSTATE GetIdealState()
	{
		return CTalkSquadMonster::GetIdealState();
	}

	static TYPEDESCRIPTION m_SaveData[];

	int m_iSawShell;
	int m_iSawLink;

	int m_iWeaponIdx;
	int m_iGruntHead;
	int m_iGruntTorso;
};

class CHGruntAllyRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_human_grunt_ally"; };
};

class CDeadHGruntAlly : public CBaseDead
{
public:
	void Spawn(void);
	int	Classify(void) { return	CLASS_HUMAN_MILITARY_FRIENDLY; }
	int GetPoseSequence() { return LookupSequence(m_szPoses[m_iPose]); }

	static char* m_szPoses[7];
};

char* CDeadHGruntAlly::m_szPoses[] = { "deadstomach", "deadside", "deadsitting", "dead_on_back", "hgrunt_dead_stomach", "dead_headcrabed", "dead_canyon" };

LINK_ENTITY_TO_CLASS(monster_human_grunt_ally, CHGruntAlly);
LINK_ENTITY_TO_CLASS(monster_grunt_ally_repel, CHGruntAllyRepel);
LINK_ENTITY_TO_CLASS(monster_human_grunt_ally_dead, CDeadHGruntAlly);

TYPEDESCRIPTION	CHGruntAlly::m_SaveData[] = 
{
	DEFINE_FIELD( CHGruntAlly, m_flPlayerDamage, FIELD_FLOAT ),
	DEFINE_FIELD( CHGruntAlly, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CHGruntAlly, m_flNextPainTime, FIELD_TIME ),
//	DEFINE_FIELD( CHGruntAlly, m_flLastEnemySightTime, FIELD_TIME ), // don't save, go to zero
	DEFINE_FIELD( CHGruntAlly, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CHGruntAlly, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntAlly, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntAlly, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntAlly, m_cClipSize, FIELD_INTEGER ),
//	DEFINE_FIELD( CHGruntAlly, m_voicePitch, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iBrassShell, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iShotgunShell, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntAlly, m_iSentence, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntAlly, m_iWeaponIdx, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntAlly, m_iGruntHead, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntAlly, m_iGruntTorso, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CHGruntAlly, CTalkSquadMonster );

void CHGruntAlly :: GibMonster ( void )
{
	if( m_hWaitMedic )
	{
		CTalkSquadMonster* pMedic = m_hWaitMedic->MyTalkSquadMonsterPointer();

		if( pMedic->pev->deadflag != DEAD_NO )
			m_hWaitMedic = nullptr;
		else
			pMedic->HealMe( nullptr );
	}

	Vector	vecGunPos;
	Vector	vecGunAngles;

	if ( m_iWeaponIdx != HGruntAllyWeapon::None )
	{// throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun;
		if (FBitSet( pev->weapons, HGruntAllyWeaponFlag::Shotgun ))
		{
			pGun = DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
		}
		else if( FBitSet( pev->weapons, HGruntAllyWeaponFlag::Saw ) )
		{
			pGun = DropItem( "weapon_m249", vecGunPos, vecGunAngles );
		}
		else
		{
			pGun = DropItem( "weapon_9mmAR", vecGunPos, vecGunAngles );
		}
		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}
	
		if (FBitSet( pev->weapons, HGruntAllyWeaponFlag::GrenadeLauncher ))
		{
			pGun = DropItem( "ammo_ARgrenades", vecGunPos, vecGunAngles );
			if ( pGun )
			{
				pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
				pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
			}
		}

		m_iWeaponIdx = HGruntAllyWeapon::None;
	}

	CBaseMonster :: GibMonster();
}

void CHGruntAlly :: HandleAnimEvent( MonsterEvent_t *pEvent )
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
			SetBodygroup( HGruntAllyBodygroup::Weapons, HGruntAllyWeapon::None );

			// now spawn a gun.
			if (FBitSet( pev->weapons, HGruntAllyWeaponFlag::Shotgun ))
			{
				 DropItem( "weapon_shotgun", vecGunPos, vecGunAngles );
			}
			else if( FBitSet( pev->weapons, HGruntAllyWeaponFlag::Saw ) )
			{
				DropItem( "weapon_m249", vecGunPos, vecGunAngles );
			}
			else
			{
				 DropItem( "weapon_9mmAR", vecGunPos, vecGunAngles );
			}

			if (FBitSet( pev->weapons, HGruntAllyWeaponFlag::GrenadeLauncher ))
			{
				DropItem( "ammo_ARgrenades", BodyTarget( pev->origin ), vecGunAngles );
			}

			m_iWeaponIdx = HGruntAllyWeapon::None;
			}
			break;

		case HGRUNT_AE_RELOAD:
			if( FBitSet( pev->weapons, HGruntAllyWeaponFlag::Saw ) )
			{
				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/saw_reload.wav", 1, ATTN_NORM );
			}
			else
				EMIT_SOUND( ENT(pev), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM );

			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
			break;

		case HGRUNT_AE_BURST1:
		{
			if ( FBitSet( pev->weapons, HGruntAllyWeaponFlag::MP5 ))
			{
				Shoot();

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
			else if( FBitSet( pev->weapons, HGruntAllyWeaponFlag::Saw ) )
			{
				ShootSaw();
			}
			else
			{
				Shotgun( );

				EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sbarrel1.wav", 1, ATTN_NORM );
			}
		
			CSoundEnt::InsertSound ( bits_SOUND_COMBAT, pev->origin, 384, 0.3 );
		}
		break;

		case HGRUNT_AE_BURST2:
		case HGRUNT_AE_BURST3:
			if( FBitSet( pev->weapons, HGruntAllyWeaponFlag::MP5 ) )
			{
				Shoot();
			}
			else if( FBitSet( pev->weapons, HGruntAllyWeaponFlag::Saw ) )
			{
				ShootSaw();
			}
			break;

		default:
			CBaseGrunt::HandleAnimEvent( pEvent );
			break;
	}
}

void CHGruntAlly :: Spawn()
{
	BaseSpawn("models/hgrunt_opfor.mdl");

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_HEAR;

	//Note: this code has been rewritten to use SetBodygroup since it relies on hardcoded offsets in the original
	pev->body = 0;
	m_iGruntTorso = HGruntAllyTorso::Normal;

	if( pev->weapons & HGruntAllyWeaponFlag::MP5 )
	{
		m_iWeaponIdx = HGruntAllyWeapon::MP5;
		m_cClipSize = GRUNT_MP5_CLIP_SIZE;
	}
	else if( pev->weapons & HGruntAllyWeaponFlag::Shotgun )
	{
		m_cClipSize = GRUNT_SHOTGUN_CLIP_SIZE;
		m_iWeaponIdx = HGruntAllyWeapon::Shotgun;
		m_iGruntTorso = HGruntAllyTorso::Shotgun;
	}
	else if( pev->weapons & HGruntAllyWeaponFlag::Saw )
	{
		m_iWeaponIdx = HGruntAllyWeapon::Saw;
		m_cClipSize = GRUNT_SAW_CLIP_SIZE;
		m_iGruntTorso = HGruntAllyTorso::Saw;
	}
	else
	{
		m_iWeaponIdx = HGruntAllyWeapon::None;
		m_cClipSize = GRUNT_MP5_CLIP_SIZE;
	}

	m_cAmmoLoaded = m_cClipSize;

	if( m_iGruntHead == HGruntAllyHead::Default )
	{
		if( pev->spawnflags & SF_SQUADMONSTER_LEADER )
		{
			m_iGruntHead = HGruntAllyHead::BeretWhite;
		}
		else if( m_iWeaponIdx == HGruntAllyWeapon::Shotgun )
		{
			m_iGruntHead = HGruntAllyHead::OpsMask;
		}
		else if( m_iWeaponIdx == HGruntAllyWeapon::Saw )
		{
			m_iGruntHead = RANDOM_LONG( 0, 1 ) + HGruntAllyHead::BandanaWhite;
		}
		else if( m_iWeaponIdx == HGruntAllyWeapon::MP5 )
		{
			m_iGruntHead = HGruntAllyHead::MilitaryPolice;
		}
		else
		{
			m_iGruntHead = HGruntAllyHead::GasMask;
		}
	}

	SetBodygroup( HGruntAllyBodygroup::Head, m_iGruntHead );
	SetBodygroup( HGruntAllyBodygroup::Torso, m_iGruntTorso );
	SetBodygroup( HGruntAllyBodygroup::Weapons, m_iWeaponIdx );

	//TODO: probably also needs this for head HGruntAllyHead::BeretBlack
	if( m_iGruntHead == HGruntAllyHead::OpsMask || m_iGruntHead == HGruntAllyHead::BandanaBlack )
		m_voicePitch = 90;

	pev->skin = 0;	

	CTalkSquadMonster::g_talkWaitTime = 0;

	m_flMedicWaitTime = gpGlobals->time;

	// get voice pitch
	m_voicePitch = 100;

	SetUse( &CHGruntAlly::FollowerUse );

	canHaveGrenadeLauncher = true;
}

void CHGruntAlly :: Precache()
{
	PRECACHE_MODEL("models/hgrunt_opfor.mdl");

	PRECACHE_SOUND( "weapons/saw_fire1.wav" );
	PRECACHE_SOUND( "weapons/saw_fire2.wav" );
	PRECACHE_SOUND( "weapons/saw_fire3.wav" );
	PRECACHE_SOUND( "weapons/saw_reload.wav" );

	m_iSawShell = PRECACHE_MODEL("models/saw_shell.mdl");
	m_iSawLink = PRECACHE_MODEL("models/saw_link.mdl");

	CBaseGruntAlly::Precache();
}	

int CHGruntAlly::GetActivitySequence(Activity NewActivity) {
	int iSequence = ACTIVITY_NOT_AVAILABLE;

	switch (NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::MP5))
		{
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
		}
		else if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Saw))
		{
			if (m_fStanding)
			{
				// get aimable sequence
				iSequence = LookupSequence("standing_saw");
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence("crouching_saw");
			}
		}
		else
		{
			if (m_fStanding)
			{
				// get aimable sequence
				iSequence = LookupSequence("standing_shotgun");
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence("crouching_shotgun");
			}
		}
		break;
	default:
		iSequence = CBaseGrunt::GetActivitySequence(NewActivity);
		break;
	}

	return iSequence;
}

Schedule_t* CHGruntAlly :: GetScheduleOfType ( int Type ) 
{
	switch	( Type )
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			if ( InSquad() )
			{
				if ( g_iSkillLevel == SKILL_HARD && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
				{
					if (FOkToSpeak())
					{
						SENTENCEG_PlayRndSz( ENT(pev), "FG_THROW", ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						JustSpoke();
					}
					return slGruntTossGrenadeCover;
				}
				else
				{
					return &slGruntTakeCover[ 0 ];
				}
			}
			else
			{
					return &slGruntTakeCover[ 0 ];
			}
		}
	case SCHED_RANGE_ATTACK1:
		{
			//Always stand when using Saw
			if( pev->weapons & HGruntAllyWeaponFlag::Saw )
			{
				m_fStanding = true;
				return &slGruntRangeAttack1B[ 0 ];
			}

			// randomly stand or crouch
			if (RANDOM_LONG(0,9) == 0)
				m_fStanding = RANDOM_LONG(0,1);
		 
			if (m_fStanding)
				return &slGruntRangeAttack1B[ 0 ];
			else
				return &slGruntRangeAttack1A[ 0 ];
		}

	default:
		{
			return CBaseGrunt:: GetScheduleOfType ( Type );
		}
	}
}

void CHGruntAlly::ShootSaw()
{
	if( m_hEnemy == NULL )
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors( pev->angles );

	switch( RANDOM_LONG( 0, 1 ) )
	{
	case 0:
		{
			auto vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT( 75, 200 ) + gpGlobals->v_up * RANDOM_FLOAT( 150, 200 ) + gpGlobals->v_forward * 25.0;
			EjectBrass( vecShootOrigin - vecShootDir * 6, vecShellVelocity, pev->angles.y, m_iSawLink, TE_BOUNCE_SHELL );
			break;
		}

	case 1:
		{
			auto vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT( 100, 250 ) + gpGlobals->v_up * RANDOM_FLOAT( 100, 150 ) + gpGlobals->v_forward * 25.0;
			EjectBrass( vecShootOrigin - vecShootDir * 6, vecShellVelocity, pev->angles.y, m_iSawShell, TE_BOUNCE_SHELL );
			break;
		}
	}

	FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_5DEGREES, 8192, BULLET_PLAYER_556, 2 ); // shoot +-5 degrees

	switch( RANDOM_LONG( 0, 2 ) )
	{
	case 0: EMIT_SOUND_DYN( edict(), CHAN_WEAPON, "weapons/saw_fire1.wav", VOL_NORM, ATTN_NORM, 0, RANDOM_LONG( 0, 15 ) + 94 ); break;
	case 1: EMIT_SOUND_DYN( edict(), CHAN_WEAPON, "weapons/saw_fire2.wav", VOL_NORM, ATTN_NORM, 0, RANDOM_LONG( 0, 15 ) + 94 ); break;
	case 2: EMIT_SOUND_DYN( edict(), CHAN_WEAPON, "weapons/saw_fire3.wav", VOL_NORM, ATTN_NORM, 0, RANDOM_LONG( 0, 15 ) + 94 ); break;
	}

	pev->effects |= EF_MUZZLEFLASH;

	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );
}

void CHGruntAlly::KeyValue( KeyValueData *pkvd )
{
	if( FStrEq( pkvd->szKeyName, "head" ) )
	{
		m_iGruntHead = atoi( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else
		CTalkSquadMonster::KeyValue( pkvd );
}


//
// repel and dead monsters
//


void CDeadHGruntAlly:: Spawn( void )
{
	CBaseDead::BaseSpawn("models/hgrunt_opfor.mdl");
	m_bloodColor = BLOOD_COLOR_RED;

	if( pev->weapons & HGruntAllyWeaponFlag::MP5 )
	{
		SetBodygroup( HGruntAllyBodygroup::Torso, HGruntAllyTorso::Normal );
		SetBodygroup( HGruntAllyBodygroup::Weapons, HGruntAllyWeapon::MP5 );
	}
	else if( pev->weapons & HGruntAllyWeaponFlag::Shotgun )
	{
		SetBodygroup( HGruntAllyBodygroup::Torso, HGruntAllyTorso::Shotgun );
		SetBodygroup( HGruntAllyBodygroup::Weapons, HGruntAllyWeapon::Shotgun );
	}
	else if( pev->weapons & HGruntAllyWeaponFlag::Saw )
	{
		SetBodygroup( HGruntAllyBodygroup::Torso, HGruntAllyTorso::Saw );
		SetBodygroup( HGruntAllyBodygroup::Weapons, HGruntAllyWeapon::Saw );
	}
	else
	{
		SetBodygroup( HGruntAllyBodygroup::Torso, HGruntAllyTorso::Normal );
		SetBodygroup( HGruntAllyBodygroup::Weapons, HGruntAllyWeapon::None );
	}

	SetBodygroup( HGruntAllyBodygroup::Head, m_iGruntHead );

	pev->skin = 0;
}
