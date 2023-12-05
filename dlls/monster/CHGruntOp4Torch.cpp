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
#include "CBaseGruntOp4.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
#include "explode.h"

#define	TORCH_DEAGLE_CLIP_SIZE			8 // how many bullets in a clip?
#define TORCH_BEAM_SPRITE "sprites/xbeam3.spr"

namespace TorchAllyBodygroup
{
enum TorchAllyBodygroup
{
	Head = 1,
	Weapons = 2
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

class COFTorchAlly : public CBaseGruntOp4
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

	CBeam* m_pTorchBeam;
};

class COFTorchAllyRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_human_torch_ally"; };
};

LINK_ENTITY_TO_CLASS(monster_human_torch_ally, COFTorchAlly );
LINK_ENTITY_TO_CLASS(monster_torch_ally_repel, COFTorchAllyRepel);

TYPEDESCRIPTION	COFTorchAlly::m_SaveData[] = 
{
	DEFINE_FIELD( COFTorchAlly, m_painTime, FIELD_TIME ),
	DEFINE_FIELD( COFTorchAlly, m_checkAttackTime, FIELD_TIME ),
	DEFINE_FIELD( COFTorchAlly, m_lastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFTorchAlly, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( COFTorchAlly, m_flNextPainTime, FIELD_TIME ),
//	DEFINE_FIELD( COFTorchAlly, m_flLastEnemySightTime, FIELD_TIME ), // don't save, go to zero
	DEFINE_FIELD( COFTorchAlly, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( COFTorchAlly, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFTorchAlly, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFTorchAlly, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFTorchAlly, m_cClipSize, FIELD_INTEGER ),
//	DEFINE_FIELD( COFTorchAlly, m_voicePitch, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iBrassShell, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iShotgunShell, FIELD_INTEGER ),
	DEFINE_FIELD( COFTorchAlly, m_iSentence, FIELD_INTEGER ),
	DEFINE_FIELD( COFTorchAlly, m_iBlackOrWhite, FIELD_INTEGER ),
	DEFINE_FIELD( COFTorchAlly, m_fUseTorch, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFTorchAlly, m_fGunHolstered, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFTorchAlly, m_fTorchHolstered, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFTorchAlly, m_fTorchActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFTorchAlly, m_flLastShot, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( COFTorchAlly, CTalkSquadMonster );

void COFTorchAlly :: GibMonster ( void )
{
	if( m_hWaitMedic )
	{
		CTalkSquadMonster* pMedic = m_hWaitMedic->MyTalkSquadMonsterPointer();

		if( pMedic->pev->deadflag != DEAD_NO )
			m_hWaitMedic = nullptr;
		else
			pMedic->HealMe( nullptr );
	}

	DropEquipment(0, true);
	SetBodygroup(TorchAllyBodygroup::Weapons, TorchAllyWeapon::None);

	if( m_fTorchActive )
	{
		m_fTorchActive = false;
		UTIL_Remove( m_pTorchBeam );
		m_pTorchBeam = nullptr;
	}

	CBaseMonster :: GibMonster();
}

void COFTorchAlly :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	// check for Torch fuel tank hit
	if( ptr->iHitgroup == 8 )
	{
		//Make sure it kills this grunt
		bitsDamageType = DMG_ALWAYSGIB | DMG_BLAST;
		flDamage = pev->health;
		ExplosionCreate( ptr->vecEndPos, pev->angles, edict(), 100, true );
	}

	CBaseGruntOp4::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

void COFTorchAlly :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	//TODO: add remaining torch grunt events
	switch( pEvent->event )
	{
		case HGRUNT_AE_DROP_GUN:
			DropEquipment(0, false);
			SetBodygroup( TorchAllyBodygroup::Weapons, TorchAllyWeapon::None );
			break;

		case HGRUNT_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
		}
		break;

		case TORCH_AE_HOLSTER_TORCH:
			{
				SetBodygroup( TorchAllyBodygroup::Weapons, TorchAllyWeapon::DesertEagle );
				m_fGunHolstered = false;
				m_fTorchHolstered = true;
				break;
			}

		case TORCH_AE_HOLSTER_GUN:
			{
				SetBodygroup( TorchAllyBodygroup::Weapons, TorchAllyWeapon::Torch );
				m_fGunHolstered = true;
				m_fTorchHolstered = false;
				break;
			}

		case TORCH_AE_HOLSTER_BOTH:
			{
				SetBodygroup( TorchAllyBodygroup::Weapons, TorchAllyWeapon::None );
				m_fGunHolstered = true;
				m_fTorchHolstered = true;
				break;
			}

		case TORCH_AE_ACTIVATE_TORCH:
			{
				m_fTorchActive = true;
				m_pTorchBeam = CBeam::BeamCreate( TORCH_BEAM_SPRITE, 5 );

				if( m_pTorchBeam )
				{
					Vector vecTorchPos, vecTorchAng;
					CBaseAnimating::GetAttachment( 2, vecTorchPos, vecTorchAng );

					m_pTorchBeam->EntsInit( entindex(), entindex() );

					m_pTorchBeam->SetStartAttachment( 4 );
					m_pTorchBeam->SetEndAttachment( 3 );

					m_pTorchBeam->SetColor( 0, 0, 255 );
					m_pTorchBeam->SetBrightness( 255 );
					m_pTorchBeam->SetWidth( 5 );
					m_pTorchBeam->SetFlags( BEAM_FSHADEIN );
					m_pTorchBeam->SetScrollRate( 20 );

					m_pTorchBeam->pev->spawnflags |= SF_BEAM_SPARKEND;
					m_pTorchBeam->DoSparks( vecTorchPos, vecTorchPos );
				}
				break;
			}

		case TORCH_AE_DEACTIVATE_TORCH:
			{
				if( m_pTorchBeam )
				{
					m_fTorchActive = false;
					UTIL_Remove( m_pTorchBeam );
					m_pTorchBeam = nullptr;
				}
				break;
			}

		default:
			CBaseGrunt::HandleAnimEvent( pEvent );
			break;
	}
}

void COFTorchAlly :: Spawn()
{
	BaseSpawn();

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_HEAR;

	//Note: this code has been rewritten to use SetBodygroup since it relies on hardcoded offsets in the original
	pev->body = 0;

	m_fGunDrawn = false;
	m_fGunHolstered = false;
	m_fTorchHolstered = true;
	m_fTorchActive = false;

	if( !pev->weapons )
	{
		pev->weapons |= TorchAllyWeaponFlag::DesertEagle;
	}

	int weaponIndex = TorchAllyWeapon::None;

	if( pev->weapons & TorchAllyWeaponFlag::DesertEagle )
	{
		weaponIndex = TorchAllyWeapon::DesertEagle;
		m_cClipSize = TORCH_DEAGLE_CLIP_SIZE;
	}
	else
	{
		weaponIndex = TorchAllyWeapon::Torch;
		m_cClipSize = TORCH_DEAGLE_CLIP_SIZE;
		m_fGunHolstered = true;
		m_fTorchHolstered = false;
	}

	SetBodygroup( TorchAllyBodygroup::Weapons, weaponIndex );

	m_cAmmoLoaded = m_cClipSize;

	m_flLastShot = gpGlobals->time;

	pev->skin = 0;

	CTalkSquadMonster::g_talkWaitTime = 0;

	m_flMedicWaitTime = gpGlobals->time;

	// get voice pitch
	m_voicePitch = 95;

	SetUse( &COFTorchAlly::FollowerUse );

	// set base equipment flags
	if (FBitSet(pev->weapons, TorchAllyWeaponFlag::DesertEagle)) {
		m_iEquipment |= MEQUIP_DEAGLE;
	}
	m_iEquipment |= MEQUIP_HELMET;
}

void COFTorchAlly :: Precache()
{
	m_defaultModel = MOD_MDL_FOLDER "hgrunt_torch.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_MODEL( TORCH_BEAM_SPRITE );

	PRECACHE_SOUND(MOD_SND_FOLDER "fgrunt/torch_light.wav" );
	PRECACHE_SOUND(MOD_SND_FOLDER "fgrunt/torch_cut_loop.wav" );

	CBaseGruntOp4::Precache();
}

const char* COFTorchAlly::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Torch Grunt";
}

int COFTorchAlly::GetActivitySequence(Activity NewActivity)
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;

	switch ( NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if ( m_fStanding )
		{
			// get aimable sequence
			iSequence = LookupSequence( "standing_mp5" );
		}
		else
		{
			// get crouching shoot
			iSequence = LookupSequence( "crouching_mp5" );
		}
		break;
	default:
		iSequence = CBaseGrunt::GetActivitySequence( NewActivity );
		break;
	}
	
	return iSequence;
}

Schedule_t* COFTorchAlly :: GetScheduleOfType ( int Type ) 
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
	default:
		{
			return CBaseGrunt::GetScheduleOfType ( Type );
		}
	}
}

void COFTorchAlly::Killed( entvars_t* pevAttacker, int iGib )
{
	if( m_fTorchActive )
	{
		m_fTorchActive = false;
		UTIL_Remove( m_pTorchBeam );
		m_pTorchBeam = nullptr;
	}

	CBaseGruntOp4::Killed( pevAttacker, iGib );
}

void COFTorchAlly::MonsterThink()
{
	if( m_fTorchActive && m_pTorchBeam )
	{
		Vector vecTorchPos;
		Vector vecTorchAng;
		Vector vecEndPos;
		Vector vecEndAng;

		GetAttachment( 2, vecTorchPos, vecTorchAng );
		GetAttachment( 3, vecEndPos, vecEndAng );

		TraceResult tr;
		UTIL_TraceLine( vecTorchPos, ( vecEndPos - vecTorchPos ).Normalize() * 4 + vecTorchPos, ignore_monsters, edict(), &tr );
		
		if( tr.flFraction != 1.0 )
		{
			m_pTorchBeam->pev->spawnflags &= ~SF_BEAM_SPARKSTART;
			//TODO: looks like a bug to me, shouldn't be bitwise inverting
			m_pTorchBeam->pev->spawnflags |= ~SF_BEAM_SPARKEND;

			UTIL_DecalTrace( &tr, RANDOM_LONG( 0, 4 ) );
			m_pTorchBeam->DoSparks( tr.vecEndPos, tr.vecEndPos );
		}

		m_pTorchBeam->SetBrightness( RANDOM_LONG( 192, 255 ) );
	}

	CTalkSquadMonster::MonsterThink();
}
