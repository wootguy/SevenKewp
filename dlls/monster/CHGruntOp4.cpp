#include "extdll.h"
#include "plane.h"
#include "util.h"
#include "monsters.h"
#include "schedule.h"
#include "defaultai.h"
#include "animation.h"
#include "weapons.h"
#include "CHGruntOp4.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"

const char* CDeadHGruntAlly::m_szPoses[] = { "deadstomach", "deadside", "deadsitting", "dead_on_back", "hgrunt_dead_stomach", "dead_headcrabed", "dead_canyon" };

LINK_ENTITY_TO_CLASS(monster_human_grunt_ally, CHGruntOp4)
LINK_ENTITY_TO_CLASS(monster_grunt_ally_repel, CHGruntOp4Repel)
LINK_ENTITY_TO_CLASS(monster_human_grunt_ally_dead, CDeadHGruntAlly)

TYPEDESCRIPTION	CHGruntOp4::m_SaveData[] = 
{
	DEFINE_FIELD( CHGruntOp4, m_flPlayerDamage, FIELD_FLOAT ),
	DEFINE_FIELD( CHGruntOp4, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CHGruntOp4, m_flNextPainTime, FIELD_TIME ),
//	DEFINE_FIELD( CHGruntOp4, m_flLastEnemySightTime, FIELD_TIME ), // don't save, go to zero
	DEFINE_FIELD( CHGruntOp4, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CHGruntOp4, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4, m_cClipSize, FIELD_INTEGER ),
//	DEFINE_FIELD( CHGruntOp4, m_voicePitch, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iBrassShell, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iShotgunShell, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntOp4, m_iSentence, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntOp4, m_iWeaponIdx, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntOp4, m_iGruntHead, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntOp4, m_iGruntTorso, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CHGruntOp4, CTalkSquadMonster )

void CHGruntOp4 :: GibMonster ( void )
{
	if( m_hWaitMedic )
	{
		CTalkSquadMonster* pMedic = m_hWaitMedic->MyTalkSquadMonsterPointer();

		if( pMedic->pev->deadflag != DEAD_NO )
			m_hWaitMedic = nullptr;
		else
			pMedic->HealMe( nullptr );
	}

	m_iWeaponIdx = HGruntAllyWeapon::None;
	if (DropEquipment(0, true))
		SetBodygroup(HGruntAllyBodygroup::Weapons, HGruntAllyWeapon::None);

	CBaseMonster :: GibMonster();
}

void CHGruntOp4 :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case HGRUNT_AE_DROP_GUN:
			if (DropEquipment(0, false))
				SetBodygroup( HGruntAllyBodygroup::Weapons, HGruntAllyWeapon::None );
			m_iWeaponIdx = HGruntAllyWeapon::None;
			break;

		default:
			CBaseGrunt::HandleAnimEvent( pEvent );
			break;
	}
}

void CHGruntOp4 :: Spawn()
{
	m_skinFrames = 2;
	BaseSpawn();

	CTalkSquadMonster::g_talkWaitTime = 0;
	SetUse(&CHGruntOp4::FollowerUse);

	SetBodygroup(HGruntAllyBodygroup::Head, m_iGruntHead);
	SetBodygroup(HGruntAllyBodygroup::Torso, m_iGruntTorso);
	SetBodygroup(HGruntAllyBodygroup::Weapons, m_iWeaponIdx);
}

void CHGruntOp4 :: Precache()
{
	m_afCapability = bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_HEAR;

	//Note: this code has been rewritten to use SetBodygroup since it relies on hardcoded offsets in the original
	pev->body = 0;
	m_iGruntTorso = HGruntAllyTorso::Normal;

	if (pev->weapons == 0) { // default equipment
		pev->weapons = HGruntAllyWeaponFlag::MP5 | HGruntAllyWeaponFlag::HandGrenade;
	}

	if (pev->weapons & HGruntAllyWeaponFlag::MP5)
	{
		m_iWeaponIdx = HGruntAllyWeapon::MP5;
		m_cClipSize = GRUNT_MP5_CLIP_SIZE;
	}
	else if (pev->weapons & HGruntAllyWeaponFlag::Shotgun)
	{
		m_cClipSize = GRUNT_SHOTGUN_CLIP_SIZE;
		m_iWeaponIdx = HGruntAllyWeapon::Shotgun;
		m_iGruntTorso = HGruntAllyTorso::Shotgun;
	}
	else if (pev->weapons & HGruntAllyWeaponFlag::Saw)
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

	if (m_iGruntHead == HGruntAllyHead::Default)
	{
		if (pev->spawnflags & SF_SQUADMONSTER_LEADER)
		{
			m_iGruntHead = HGruntAllyHead::BeretWhite;
		}
		else if (m_iWeaponIdx == HGruntAllyWeapon::Shotgun)
		{
			m_iGruntHead = HGruntAllyHead::OpsMask;
		}
		else if (m_iWeaponIdx == HGruntAllyWeapon::Saw)
		{
			m_iGruntHead = RANDOM_LONG(0, 1) + HGruntAllyHead::BandanaWhite;
		}
		else if (m_iWeaponIdx == HGruntAllyWeapon::MP5)
		{
			m_iGruntHead = HGruntAllyHead::MilitaryPolice;
		}
		else
		{
			m_iGruntHead = HGruntAllyHead::GasMask;
		}
	}

	//TODO: probably also needs this for head HGruntAllyHead::BeretBlack
	if (m_iGruntHead == HGruntAllyHead::OpsMask || m_iGruntHead == HGruntAllyHead::BandanaBlack)
		m_voicePitch = 90;

	m_flMedicWaitTime = gpGlobals->time;

	// get voice pitch
	m_voicePitch = 100;

	// set base equipment flags
	if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::MP5)) {
		m_iEquipment |= MEQUIP_MP5;
	}
	if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Shotgun)) {
		m_iEquipment |= MEQUIP_SHOTGUN;
	}
	if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::Saw)) {
		m_iEquipment |= MEQUIP_SAW;
	}
	if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::HandGrenade)) {
		m_iEquipment |= MEQUIP_HAND_GRENADE;
	}
	if (FBitSet(pev->weapons, HGruntAllyWeaponFlag::GrenadeLauncher)) {
		m_iEquipment |= MEQUIP_GRENADE_LAUNCHER;
	}
	if (m_iGruntHead == HGruntAllyHead::GasMask || m_iGruntHead == HGruntAllyHead::MilitaryPolice) {
		m_iEquipment |= MEQUIP_HELMET;
	}

	CBaseGruntOp4::Precache();
	m_defaultModel = "models/hgrunt_opfor.mdl";
	PRECACHE_MODEL(GetModel());
}	

const char* CHGruntOp4::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Grunt";
}

int CHGruntOp4::GetActivitySequence(Activity NewActivity) {
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

Schedule_t* CHGruntOp4 :: GetScheduleOfType ( int Type ) 
{
	switch	( Type )
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			if ( InSquad() )
			{
				if (gSkillData.sk_hgrunt_gcover && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
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

void CHGruntOp4::KeyValue( KeyValueData *pkvd )
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
	m_bloodColor = BloodColorHuman();

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
}
