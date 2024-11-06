#include "extdll.h"
#include "plane.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "defaultai.h"
#include "animation.h"
#include "CTalkSquadMonster.h"
#include "weapons.h"
#include "CTalkSquadMonster.h"
#include "CBaseGruntOp4.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
#include "explode.h"
#include "CGrenade.h"

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

class CHGruntOp4Medic : public CBaseGruntOp4
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

	void EXPORT HealerUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value );

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

LINK_ENTITY_TO_CLASS(monster_human_medic_ally, CHGruntOp4Medic)
LINK_ENTITY_TO_CLASS(monster_medic_ally_repel, CHGruntOp4MedicRepel)

TYPEDESCRIPTION	CHGruntOp4Medic::m_SaveData[] = 
{
	DEFINE_FIELD( CHGruntOp4Medic, m_lastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CHGruntOp4Medic, m_flNextPainTime, FIELD_TIME ),
//	DEFINE_FIELD( CHGruntOp4Medic, m_flLastEnemySightTime, FIELD_TIME ), // don't save, go to zero
	DEFINE_FIELD( CHGruntOp4Medic, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_cClipSize, FIELD_INTEGER ),
//	DEFINE_FIELD( CHGruntOp4Medic, m_voicePitch, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iBrassShell, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iShotgunShell, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntOp4Medic, m_iSentence, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntOp4Medic, m_flFollowCheckTime, FIELD_FLOAT ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fFollowChecking, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fFollowChecked, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_flLastRejectAudio, FIELD_FLOAT ),
	DEFINE_FIELD( CHGruntOp4Medic, m_iBlackOrWhite, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntOp4Medic, m_iHealCharge, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fUseHealing, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fHealing, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_flLastUseTime, FIELD_TIME ),
	DEFINE_FIELD( CHGruntOp4Medic, m_hNewTargetEnt, FIELD_EHANDLE ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fGunHolstered, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fHypoHolstered, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_fHealActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( CHGruntOp4Medic, m_iWeaponIdx, FIELD_INTEGER ),
	DEFINE_FIELD( CHGruntOp4Medic, m_flLastShot, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( CHGruntOp4Medic, CTalkSquadMonster )

void CHGruntOp4Medic :: GibMonster ( void )
{
	
	m_iWeaponIdx = MedicAllyWeapon::None;
	if (DropEquipment(0, true))
		SetBodygroup( MedicAllyBodygroup::Weapons, MedicAllyWeapon::None );

	CBaseMonster :: GibMonster();
}

void CHGruntOp4Medic :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case HGRUNT_AE_DROP_GUN:
			if (DropEquipment(0, false))
				SetBodygroup( MedicAllyBodygroup::Weapons, MedicAllyWeapon::None );
			m_iWeaponIdx = MedicAllyWeapon::None;
			break;

		case HGRUNT_AE_GREN_DROP:
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
			break;

		case MEDIC_AE_HOLSTER_GUN:
			SetBodygroup( MedicAllyBodygroup::Weapons, MedicAllyWeapon::None );
			m_fGunHolstered = true;
			break;

		case MEDIC_AE_EQUIP_NEEDLE:
			SetBodygroup( MedicAllyBodygroup::Weapons, MedicAllyWeapon::Needle ); 
			m_fHypoHolstered = false;
			break;

		case MEDIC_AE_HOLSTER_NEEDLE:
			SetBodygroup( MedicAllyBodygroup::Weapons, MedicAllyWeapon::None );
			m_fHypoHolstered = true;
			break;

		case MEDIC_AE_EQUIP_GUN:
			SetBodygroup( MedicAllyBodygroup::Weapons, pev->weapons & MedicAllyWeaponFlag::Glock ? MedicAllyWeapon::Glock : MedicAllyWeapon::DesertEagle );
			m_fGunHolstered = false;
			break;

		default:
			CBaseGrunt::HandleAnimEvent( pEvent );
			break;
	}
}

void CHGruntOp4Medic :: Spawn()
{
	BaseSpawn();

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_HEAR;

	//Note: this code has been rewritten to use SetBodygroup since it relies on hardcoded offsets in the original
	pev->body = 0;

	m_flLastUseTime = 0;
	m_iHealCharge = gSkillData.sk_scientist_heal;
	m_fGunHolstered = false;
	m_fHypoHolstered = true;
	m_fHealActive = false;
	m_fQueueFollow = false;
	m_fUseHealing = false;
	m_fHealing = false;
	m_hNewTargetEnt = nullptr;
	m_fFollowChecked = false;
	m_fFollowChecking = false;

	CTalkSquadMonster::g_talkWaitTime = 0;

	SetUse(&CHGruntOp4Medic::HealerUse);

	// get voice pitch
	m_voicePitch = 105;

	SetBodygroup(MedicAllyBodygroup::Head, m_iBlackOrWhite);
	SetBodygroup(MedicAllyBodygroup::Weapons, m_iWeaponIdx);
}

void CHGruntOp4Medic :: Precache()
{
	if (!pev->weapons)
	{
		pev->weapons |= MedicAllyWeaponFlag::Glock;
	}

	if( m_iBlackOrWhite == MedicAllyHead::Default )
	{
		m_iBlackOrWhite = RANDOM_LONG( 0, 99 ) % 2 == 0 ? MedicAllyHead::White : MedicAllyHead::Black;
	}

	if( pev->weapons & MedicAllyWeaponFlag::Glock )
	{
		m_iWeaponIdx = MedicAllyWeapon::Glock;
		m_cClipSize = MEDIC_GLOCK_CLIP_SIZE;
	}
	else if( pev->weapons & MedicAllyWeaponFlag::DesertEagle )
	{
		m_iWeaponIdx = MedicAllyWeapon::DesertEagle;
		m_cClipSize = MEDIC_DEAGLE_CLIP_SIZE;
	}
	else if( pev->weapons & MedicAllyWeaponFlag::Needle )
	{
		m_iWeaponIdx = MedicAllyWeapon::Needle;
		m_cClipSize = 1;
		m_fGunHolstered = true;
		m_fHypoHolstered = false;
	}
	else
	{
		m_iWeaponIdx = MedicAllyWeapon::None;
		m_cClipSize = 0;
	}

	m_cAmmoLoaded = m_cClipSize;

	m_flLastShot = gpGlobals->time;

	if (m_iBlackOrWhite == MedicAllyHead::Black)
	{
		m_voicePitch = 95;
	}

	// set base equipment flags
	if (FBitSet(pev->weapons, MedicAllyWeaponFlag::Glock)) {
		m_iEquipment |= MEQUIP_GLOCK;
	}
	if (FBitSet(pev->weapons, MedicAllyWeaponFlag::DesertEagle)) {
		m_iEquipment |= MEQUIP_DEAGLE;
	}
	if (FBitSet(pev->weapons, MedicAllyWeaponFlag::Needle)) {
		m_iEquipment |= MEQUIP_NEEDLE;
	}
	m_iEquipment |= MEQUIP_HELMET;

	CBaseGruntOp4::Precache();

	m_defaultModel = "models/hgrunt_medic.mdl";
	PRECACHE_MODEL(GetModel());

	PRECACHE_SOUND("fgrunt/medic_give_shot.wav" );
	PRECACHE_SOUND("fgrunt/medical.wav" );

	PRECACHE_SOUND("fgrunt/medic.wav" );
}	

const char* CHGruntOp4Medic::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Medic Grunt";
}

void CHGruntOp4Medic :: StartTask ( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch ( pTask->iTask )
	{
	case TASK_MELEE_ATTACK2:
		{
			m_IdealActivity = ACT_MELEE_ATTACK2;

			if( !m_fHealAudioPlaying )
			{
				EMIT_SOUND( edict(), CHAN_WEAPON, "fgrunt/medic_give_shot.wav", VOL_NORM, ATTN_NORM);
				m_fHealAudioPlaying = true;
			}
			break;
		}

	case TASK_WAIT_FOR_MOVEMENT:
		{
			if( !m_fHealing )
				return CTalkSquadMonster::StartTask( pTask );

			if( m_hTargetEnt )
			{
				CBaseEntity* pTarget = m_hTargetEnt;
				CTalkSquadMonster* pTargetMonster = pTarget->MyTalkSquadMonsterPointer();

				if( pTargetMonster )
					pTargetMonster->m_hWaitMedic = nullptr;

				m_fHealing = false;
				m_fUseHealing = false;

				STOP_SOUND( edict(), CHAN_WEAPON, "fgrunt/medic_give_shot.wav" );

				m_fFollowChecked = false;
				m_fFollowChecking = false;

				if( m_movementGoal == MOVEGOAL_TARGETENT )
					RouteClear();

				m_hTargetEnt = nullptr;

				m_fHealActive = false;

				return CTalkSquadMonster::StartTask( pTask );
			}

			m_fHealing = false;
			m_fUseHealing = false;

			STOP_SOUND( edict(), CHAN_WEAPON, "fgrunt/medic_give_shot.wav" );

			m_fFollowChecked = false;
			m_fFollowChecking = false;

			if( m_movementGoal == MOVEGOAL_TARGETENT )
				RouteClear();

			m_IdealActivity = ACT_DISARM;
			m_fHealActive = false;
			break;
		}

	default: 
		CBaseGrunt::StartTask( pTask );
		break;
	}
}

void CHGruntOp4Medic :: RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_MELEE_ATTACK2:
		{
			if( m_fSequenceFinished )
			{
				if( m_fUseHealing )
				{
					if( gpGlobals->time - m_flLastUseTime > 0.3 )
						m_Activity = ACT_RESET;
				}
				else
				{
					m_fHealActive = true;

					if( m_hTargetEnt )
					{
						CBaseEntity* pHealTarget = m_hTargetEnt;

						const auto toHeal = V_min( 5, pHealTarget->pev->max_health - pHealTarget->pev->health );

						if( toHeal != 0 && pHealTarget->TakeHealth( toHeal, DMG_GENERIC ) )
						{
							m_iHealCharge -= toHeal;
						}
						else
						{
							m_Activity = ACT_RESET;
						}
					}
					else
					{
						m_Activity = m_fHealing ? ACT_MELEE_ATTACK2 : ACT_RESET;
					}
				}

				TaskComplete();
			}

			break;
		}

	default:
		{
			CBaseGrunt::RunTask( pTask );
			break;
		}
	}
}

Task_t	tlMedicAllyNewHealTarget[] =
{
	{ TASK_SET_FAIL_SCHEDULE, SCHED_TARGET_CHASE },
	{ TASK_MOVE_TO_TARGET_RANGE, 50 },
	{ TASK_FACE_IDEAL, 0 },
	{ TASK_GRUNT_SPEAK_SENTENCE, 0 },
};

Schedule_t	slMedicAllyNewHealTarget[] =
{
	{
		tlMedicAllyNewHealTarget,
		ARRAYSIZE( tlMedicAllyNewHealTarget ),
		0,
		0,
		"MEDIC_NEW_HEAL_TARGET"
	},
};

Task_t	tlMedicAllyDrawNeedle[] =
{
	{ TASK_STOP_MOVING, 0 },
	{ TASK_PLAY_SEQUENCE_FACE_TARGET, ACT_ARM },
	{ TASK_SET_FAIL_SCHEDULE, SCHED_TARGET_CHASE },
	{ TASK_MOVE_TO_TARGET_RANGE, 50 },
	{ TASK_FACE_IDEAL, 0, },
	{ TASK_GRUNT_SPEAK_SENTENCE, 0 }
};

Schedule_t	slMedicAllyDrawNeedle[] =
{
	{
		tlMedicAllyDrawNeedle,
		ARRAYSIZE( tlMedicAllyDrawNeedle ),
		0,
		0,
		"MEDIC_DRAW_NEEDLE"
	},
};

Task_t	tlMedicAllyDrawGun[] =
{
	{ TASK_PLAY_SEQUENCE, ACT_DISARM },
	{ TASK_WAIT_FOR_MOVEMENT, 0 },
};

Schedule_t	slMedicAllyDrawGun[] =
{
	{
		tlMedicAllyDrawGun,
		ARRAYSIZE( tlMedicAllyDrawGun ),
		0,
		0,
		"MEDIC_DRAW_GUN"
	},
};

Task_t	tlMedicAllyHealTarget[] =
{
	{ TASK_MELEE_ATTACK2, 0 },
	{ TASK_WAIT, 0.2f },
	{ TASK_TLK_HEADRESET, 0 },
};

Schedule_t	slMedicAllyHealTarget[] =
{
	{
		tlMedicAllyHealTarget,
		ARRAYSIZE( tlMedicAllyHealTarget ),
		0,
		0,
		"MEDIC_HEAL_TARGET"
	},
};

DEFINE_CUSTOM_SCHEDULES( CHGruntOp4Medic )
{
	slMedicAllyNewHealTarget,
	slMedicAllyDrawNeedle,
	slMedicAllyDrawGun,
	slMedicAllyHealTarget,
};

IMPLEMENT_CUSTOM_SCHEDULES( CHGruntOp4Medic, CBaseGrunt )

int CHGruntOp4Medic::GetActivitySequence(Activity NewActivity)
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
		iSequence = CBaseGrunt::GetActivitySequence(NewActivity);
		break;
	}
	
	return iSequence;
}

Schedule_t *CHGruntOp4Medic :: GetSchedule( void )
{

	// clear old sentence
	m_iSentence = HGRUNT_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if ( pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE )
	{
		if (pev->flags & FL_ONGROUND)
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType ( SCHED_GRUNT_REPEL_LAND );
		}
		else
		{
			// repel down a rope, 
			if ( m_MonsterState == MONSTERSTATE_COMBAT )
				return GetScheduleOfType ( SCHED_GRUNT_REPEL_ATTACK );
			else
				return GetScheduleOfType ( SCHED_GRUNT_REPEL );
		}
	}

	if( m_fHealing )
	{
		if( m_hTargetEnt )
		{
			CBaseEntity* pHealTarget = m_hTargetEnt;

			if( ( pHealTarget->pev->origin - pev->origin ).Make2D().Length() <= 50.0
				&& ( !m_fUseHealing || gpGlobals->time - m_flLastUseTime <= 0.25 )
				&& m_iHealCharge
				&& pHealTarget->IsAlive()
				&& pHealTarget->pev->health != pHealTarget->pev->max_health )
			{
				return slMedicAllyHealTarget;
			}
		}

		return slMedicAllyDrawGun;
	}

	// grunts place HIGH priority on running away from danger sounds.
	if ( HasConditions(bits_COND_HEAR_SOUND) )
	{
		CSound *pSound;
		pSound = PBestSound();

		ASSERT( pSound != NULL );
		if ( pSound)
		{
			if (pSound->m_iType & bits_SOUND_DANGER)
			{
				// dangerous sound nearby!
				
				//!!!KELLY - currently, this is the grunt's signal that a grenade has landed nearby,
				// and the grunt should find cover from the blast
				// good place for "SHIT!" or some other colorful verbal indicator of dismay.
				// It's not safe to play a verbal order here "Scatter", etc cause 
				// this may only affect a single individual in a squad. 
				
				if (FOkToSpeak())
				{
					SENTENCEG_PlayRndSz( ENT(pev), "FG_GREN", ALLY_GRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_BEST_SOUND );
			}
			/*
			if (!HasConditions( bits_COND_SEE_ENEMY ) && ( pSound->m_iType & (bits_SOUND_PLAYER | bits_SOUND_COMBAT) ))
			{
				MakeIdealYaw( pSound->m_vecOrigin );
			}
			*/
		}
	}
	
	return CBaseGruntOp4::GetMonsterStateSchedule();
}

Schedule_t* CHGruntOp4Medic :: GetScheduleOfType ( int Type ) 
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

	case SCHED_MEDIC_ALLY_HEAL_ALLY:
		return slMedicAllyHealTarget;

	default:
		{
			return CBaseGrunt::GetScheduleOfType ( Type );
		}
	}
}

int CHGruntOp4Medic::ObjectCaps()
{
	//Allow healing the player by continuously using
	return FCAP_ACROSS_TRANSITION | FCAP_CONTINUOUS_USE;
}

void CHGruntOp4Medic::KeyValue( KeyValueData *pkvd )
{
	if( FStrEq( pkvd->szKeyName, "head" ) )
	{
		m_iBlackOrWhite = atoi( pkvd->szValue );
		pkvd->fHandled = true;
	}
	else
		CTalkSquadMonster::KeyValue( pkvd );
}

void CHGruntOp4Medic::Killed( entvars_t* pevAttacker, int iGib )
{
	if (m_hTargetEnt != NULL)
	{
		CTalkSquadMonster* pSquadMonster = m_hTargetEnt->MyTalkSquadMonsterPointer();

		if (pSquadMonster && pSquadMonster->m_hWaitMedic == this)
			pSquadMonster->m_hWaitMedic = NULL;
	}

	CBaseGruntOp4::Killed( pevAttacker, iGib );
}

void CHGruntOp4Medic::MonsterThink()
{
	if( m_fFollowChecking && !m_fFollowChecked && gpGlobals->time - m_flFollowCheckTime > 0.5 )
	{
		m_fFollowChecking = false;

		//TODO: not suited for multiplayer
		auto pPlayer = UTIL_FindEntityByClassname( nullptr, "player" );

		FollowerUse( pPlayer, pPlayer, USE_TOGGLE, 0 );
	}

	CTalkSquadMonster::MonsterThink();
}

BOOL CHGruntOp4Medic::HealMe( CTalkSquadMonster* pTarget )
{
	if( pTarget )
	{
		if( m_hTargetEnt && !m_hTargetEnt->IsPlayer() )
		{
			auto pCurrentTarget = m_hTargetEnt->MyTalkSquadMonsterPointer();

			if( pCurrentTarget && pCurrentTarget->MySquadLeader() == MySquadLeader() )
			{
				return false;
			}

			if( pTarget->MySquadLeader() != MySquadLeader() )
			{
				return false;
			}
		}

		if( m_MonsterState != MONSTERSTATE_COMBAT && m_iHealCharge )
		{
			HealerActivate( pTarget );
			return true;
		}
	}
	else
	{
		if( m_hTargetEnt )
		{
			auto v14 = m_hTargetEnt->MyTalkSquadMonsterPointer();
			if( v14 )
				v14->m_hWaitMedic = nullptr;
		}
		
		m_hTargetEnt = nullptr;

		if( m_movementGoal == MOVEGOAL_TARGETENT )
			RouteClear();

		ClearSchedule();
		ChangeSchedule( slMedicAllyDrawGun );
	}

	return false;
}

void CHGruntOp4Medic::HealOff()
{
	m_fHealing = false;

	if( m_movementGoal == MOVEGOAL_TARGETENT )
		RouteClear();

	m_hTargetEnt = nullptr;
	ClearSchedule();

	SetThink( nullptr );
	pev->nextthink = 0;
}

void CHGruntOp4Medic::HealerActivate( CBaseMonster* pTarget )
{
	if( m_hTargetEnt )
	{
		auto pMonster = m_hTargetEnt->MyTalkSquadMonsterPointer();

		if( pMonster )
			pMonster->m_hWaitMedic = nullptr;

		//TODO: this makes little sense given the null check above
		pMonster->m_hWaitMedic = this;

		m_hTargetEnt = pTarget;

		m_fHealing = false;

		ClearSchedule();

		ChangeSchedule( slMedicAllyNewHealTarget );
	}
	else if( m_iHealCharge > 0
		&& pTarget->IsAlive()
		&& pTarget->pev->max_health > pTarget->pev->health
		&& !m_fHealing )
	{
		if( m_hTargetEnt && m_hTargetEnt->IsPlayer() )
		{
			StopFollowing( false );
		}

		m_hTargetEnt = pTarget;

		auto pMonster = pTarget->MyTalkSquadMonsterPointer();

		if( pMonster )
			pMonster->m_hWaitMedic = this;

		m_fHealing = true;

		ClearSchedule();

		EMIT_SOUND( edict(), CHAN_VOICE, "fgrunt/medical.wav", VOL_NORM, ATTN_NORM );

		ChangeSchedule( slMedicAllyDrawNeedle );
	}
}

void CHGruntOp4Medic::HealerUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	if( m_useTime > gpGlobals->time
		|| m_flLastUseTime > gpGlobals->time
		|| pActivator == m_hEnemy )
	{
		return;
	}

	if( m_fFollowChecked || m_fFollowChecking )
	{
		if( !m_fFollowChecked && m_fFollowChecking )
		{
			if( gpGlobals->time - m_flFollowCheckTime < 0.3 )
				return;

			m_fFollowChecked = true;
			m_fFollowChecking = false;
		}

		const auto newTarget = !m_fUseHealing && m_hTargetEnt && m_fHealing;

		if( newTarget )
		{
			if( pActivator->pev->health >= pActivator->pev->max_health )
				return;
			
			m_fHealing = false;

			auto pMonster = m_hTargetEnt->MyTalkSquadMonsterPointer();

			if( pMonster )
				pMonster->m_hWaitMedic = nullptr;
		}

		if( m_iHealCharge > 0
			&& pActivator->IsAlive()
			&& pActivator->pev->max_health > pActivator->pev->health )
		{
			if( !m_fHealing )
			{
				if( m_hTargetEnt && m_hTargetEnt->IsPlayer() )
				{
					StopFollowing( false );
				}

				m_hTargetEnt = pActivator;

				m_fHealing = true;
				m_fUseHealing = true;

				ClearSchedule();

				m_fHealAudioPlaying = false;

				if( newTarget )
				{
					ChangeSchedule( slMedicAllyNewHealTarget );
				}
				else
				{
					SENTENCEG_PlayRndSz( edict(), "MG_HEAL", ALLY_GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
					ChangeSchedule( slMedicAllyDrawNeedle );
				}
			}

			if( m_fHealActive )
			{
				if( pActivator->TakeHealth( 2, DMG_GENERIC ) )
				{
					m_iHealCharge -= 2;
				}
			}
			else if( pActivator->TakeHealth( 1, DMG_GENERIC ) )
			{
				--m_iHealCharge;
			}
		}
		else
		{
			m_fFollowChecked = false;
			m_fFollowChecking = false;

			if( gpGlobals->time - m_flLastRejectAudio > 4.0 && m_iHealCharge <= 0 && !m_fHealing )
			{
				m_flLastRejectAudio = gpGlobals->time;
				SENTENCEG_PlayRndSz( edict(), "MG_NOTHEAL", ALLY_GRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
			}
		}

		m_flLastUseTime = gpGlobals->time + 0.2;
		return;
	}

	m_fFollowChecking = true;
	m_flFollowCheckTime = gpGlobals->time;
}
