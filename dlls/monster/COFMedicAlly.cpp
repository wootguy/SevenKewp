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
#include "CBaseHGrunt.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
#include "explode.h"

int g_fMedicAllyQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int		g_iSkillLevel;

#define	MEDIC_DEAGLE_CLIP_SIZE			9 // how many bullets in a clip?
#define	MEDIC_GLOCK_CLIP_SIZE			9 // how many bullets in a clip?
#define	MEDIC_SENTENCE_VOLUME			(float)0.35 // volume of grunt sentences
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

class COFMedicAlly : public CBaseHGrunt
{
public:
	void Spawn( void );
	void Precache( void );
	int  Classify ( void );
	int ISoundMask ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	void StartTask ( Task_t *pTask );
	void RunTask ( Task_t *pTask );
	void DeathSound( void );
	void PainSound( void );
	void IdleSound ( void );
	void Shoot ( void );
	void GibMonster( void );

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	
	int GetActivitySequence(Activity NewActivity);
	Schedule_t	*GetSchedule( void );
	Schedule_t  *GetScheduleOfType ( int Type );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	int ObjectCaps() override;

	void TalkInit();

	void AlertSound() override;

	void DeclineFollowing() override;

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

	BOOL m_lastAttackCheck;
	float m_flPlayerDamage;

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

	float m_flLastShot;

	static const char *pMedicSentences[];
};

class CHGruntAllyRepel : public CBaseRepel
{
public:
	const char* GetMonsterType() { return "monster_human_medic_ally"; };
};

LINK_ENTITY_TO_CLASS(monster_human_medic_ally, COFMedicAlly);
LINK_ENTITY_TO_CLASS(monster_medic_ally_repel, CHGruntAllyRepel);

TYPEDESCRIPTION	COFMedicAlly::m_SaveData[] = 
{
	DEFINE_FIELD( COFMedicAlly, m_lastAttackCheck, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( COFMedicAlly, m_flNextPainTime, FIELD_TIME ),
//	DEFINE_FIELD( COFMedicAlly, m_flLastEnemySightTime, FIELD_TIME ), // don't save, go to zero
	DEFINE_FIELD( COFMedicAlly, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( COFMedicAlly, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_cClipSize, FIELD_INTEGER ),
//	DEFINE_FIELD( COFMedicAlly, m_voicePitch, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iBrassShell, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iShotgunShell, FIELD_INTEGER ),
	DEFINE_FIELD( COFMedicAlly, m_iSentence, FIELD_INTEGER ),
	DEFINE_FIELD( COFMedicAlly, m_flFollowCheckTime, FIELD_FLOAT ),
	DEFINE_FIELD( COFMedicAlly, m_fFollowChecking, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_fFollowChecked, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_flLastRejectAudio, FIELD_FLOAT ),
	DEFINE_FIELD( COFMedicAlly, m_iBlackOrWhite, FIELD_INTEGER ),
	DEFINE_FIELD( COFMedicAlly, m_iHealCharge, FIELD_INTEGER ),
	DEFINE_FIELD( COFMedicAlly, m_fUseHealing, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_fHealing, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_flLastUseTime, FIELD_TIME ),
	DEFINE_FIELD( COFMedicAlly, m_hNewTargetEnt, FIELD_EHANDLE ),
	DEFINE_FIELD( COFMedicAlly, m_fGunHolstered, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_fHypoHolstered, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_fHealActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( COFMedicAlly, m_iWeaponIdx, FIELD_INTEGER ),
	DEFINE_FIELD( COFMedicAlly, m_flLastShot, FIELD_TIME ),
};

IMPLEMENT_SAVERESTORE( COFMedicAlly, CTalkSquadMonster );

const char *COFMedicAlly::pMedicSentences[] = 
{
	"FG_GREN", // grenade scared grunt
	"FG_ALERT", // sees player
	"FG_MONSTER", // sees monster
	"FG_COVER", // running to cover
	"FG_THROW", // about to throw grenade
	"FG_CHARGE",  // running out to get the enemy
	"FG_TAUNT", // say rude things
};

enum
{
	MEDIC_SENT_NONE = -1,
	MEDIC_SENT_GREN = 0,
	MEDIC_SENT_ALERT,
	MEDIC_SENT_MONSTER,
	MEDIC_SENT_COVER,
	MEDIC_SENT_THROW,
	MEDIC_SENT_CHARGE,
	MEDIC_SENT_TAUNT,
} MEDIC_ALLY_SENTENCE_TYPES;

void COFMedicAlly :: GibMonster ( void )
{
	Vector	vecGunPos;
	Vector	vecGunAngles;

	//TODO: probably the wrong logic, but it was in the original
	if( m_iWeaponIdx != MedicAllyWeapon::None )
	{// throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );

		CBaseEntity *pGun;
		
		if( pev->weapons & MedicAllyWeaponFlag::Glock )
		{
			pGun = DropItem( "weapon_9mmhandgun", vecGunPos, vecGunAngles );
		}
		else if( pev->weapons & MedicAllyWeaponFlag::DesertEagle )
		{
			pGun = DropItem( "weapon_eagle", vecGunPos, vecGunAngles );
		}

		if( pGun )
		{
			pGun->pev->velocity = Vector( RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( -100, 100 ), RANDOM_FLOAT( 200, 300 ) );
			pGun->pev->avelocity = Vector( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}

		m_iWeaponIdx = MedicAllyWeapon::None;

		//Note: this wasn't in the original
		SetBodygroup( MedicAllyBodygroup::Weapons, MedicAllyWeapon::None );
	}

	CBaseMonster :: GibMonster();
}

int COFMedicAlly :: ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			bits_SOUND_DANGER	|
			bits_SOUND_CARCASS	|
			bits_SOUND_MEAT		|
			bits_SOUND_GARBAGE;
}

BOOL COFMedicAlly :: CheckRangeAttack1 ( float flDot, float flDist )
{
	//Only if we have a weapon
	if( pev->weapons )
	{
		//Friendly fire is allowed
		if( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= 1024 && flDot >= 0.5 /*&& NoFriendlyFire()*/ )
		{
			TraceResult	tr;

			CBaseEntity* pEnemy = m_hEnemy;

			//if( !pEnemy->IsPlayer() && flDist <= 64 )
			//{
			//	// kick nonclients, but don't shoot at them.
			//	return FALSE;
			//}

			//TODO: kinda odd that this doesn't use GetGunPosition like the original
			Vector vecSrc = pev->origin + Vector( 0, 0, 55 );

			//Fire at last known position, adjusting for target origin being offset from entity origin
			const auto targetOrigin = pEnemy->BodyTarget( vecSrc );

			const auto targetPosition = targetOrigin - pEnemy->pev->origin + m_vecEnemyLKP;

			// verify that a bullet fired from the gun will hit the enemy before the world.
			UTIL_TraceLine( vecSrc, targetPosition, dont_ignore_monsters, ENT( pev ), &tr );

			m_lastAttackCheck = tr.flFraction == 1.0 ? true : tr.pHit && GET_PRIVATE( tr.pHit ) == pEnemy;

			return m_lastAttackCheck;
		}
	}

	return FALSE;
}

void COFMedicAlly :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	// check for helmet shot
	if (ptr->iHitgroup == 11)
	{
		// make sure we're wearing one
		//TODO: disabled for ally
		if (/*GetBodygroup( HGruntAllyBodygroup::Head ) == HGruntAllyHead::GasMask &&*/ (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_CLUB)))
		{
			// absorb damage
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet( ptr->vecEndPos, 1.0 );
				flDamage = 0.01;
			}
		}
		// it's head shot anyways
		ptr->iHitgroup = HITGROUP_HEAD;
	}
	//PCV absorbs some damage types
	else if( ( ptr->iHitgroup == HITGROUP_CHEST || ptr->iHitgroup == HITGROUP_STOMACH )
		&& ( bitsDamageType & ( DMG_BLAST | DMG_BULLET | DMG_SLASH ) ) )
	{
		flDamage*= 0.5;
	}

	CTalkSquadMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

int COFMedicAlly :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CTalkSquadMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );

	if( !IsAlive() || pev->deadflag == DEAD_DYING )
		return ret;

	if( m_MonsterState != MONSTERSTATE_PRONE && ( pevAttacker->flags & FL_CLIENT ) )
	{
		Forget( bits_MEMORY_INCOVER );

		m_flPlayerDamage += flDamage;

		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if( m_hEnemy == NULL )
		{
			// If the player was facing directly at me, or I'm already suspicious, get mad
			if( gpGlobals->time - m_flLastHitByPlayer < 4.0 && m_iPlayerHits > 2
				&& ( ( m_afMemory & bits_MEMORY_SUSPICIOUS ) || IsFacing( pevAttacker, pev->origin ) ) )
			{
				// Alright, now I'm pissed!
				PlaySentence( "FG_MAD", 4, VOL_NORM, ATTN_NORM );

				Remember( bits_MEMORY_PROVOKED );
				StopFollowing( TRUE );
				ALERT( at_console, "HGrunt Ally is now MAD!\n" );
			}
			else
			{
				// Hey, be careful with that
				PlaySentence( "FG_SHOT", 4, VOL_NORM, ATTN_NORM );
				Remember( bits_MEMORY_SUSPICIOUS );

				if( 4.0 > gpGlobals->time - m_flLastHitByPlayer )
					++m_iPlayerHits;
				else
					m_iPlayerHits = 0;

				m_flLastHitByPlayer = gpGlobals->time;

				ALERT( at_console, "HGrunt Ally is now SUSPICIOUS!\n" );
			}
		}
		else if( !m_hEnemy->IsPlayer() )
		{
			PlaySentence( "FG_SHOT", 4, VOL_NORM, ATTN_NORM );
		}
	}

	return ret;
}

void COFMedicAlly :: IdleSound( void )
{
	if (FOkToSpeak() && (g_fMedicAllyQuestion || RANDOM_LONG(0,1)))
	{
		if (!g_fMedicAllyQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0,2))
			{
			case 0: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CHECK", MEDIC_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fMedicAllyQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "FG_QUEST", MEDIC_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fMedicAllyQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "FG_IDLE", MEDIC_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fMedicAllyQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CLEAR", MEDIC_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "FG_ANSWER", MEDIC_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fMedicAllyQuestion = 0;
		}
		JustSpoke();
	}
}

int	COFMedicAlly :: Classify ( void )
{
	return	CLASS_HUMAN_MILITARY_FRIENDLY;
}

void COFMedicAlly :: Shoot ( void )
{
	//Limit fire rate
	if (m_hEnemy == NULL || gpGlobals->time - m_flLastShot <= 0.11 )
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	const char* pszSoundName;

	if( pev->weapons & MedicAllyWeaponFlag::Glock )
	{
		FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_9MM ); // shoot +-5 degrees
		pszSoundName = "weapons/pl_gun3.wav";
	}
	else if( pev->weapons & MedicAllyWeaponFlag::DesertEagle )
	{
		FireBullets( 1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_PLAYER_357 ); // shoot +-5 degrees
		pszSoundName = "weapons/desert_eagle_fire.wav";
	}

	const auto random = RANDOM_LONG( 0, 20 );

	EMIT_SOUND_DYN( edict(), CHAN_WEAPON, pszSoundName, VOL_NORM, ATTN_NORM, 0, (random <= 10 ? random - 5 : 0) + 100 );

	pev->effects |= EF_MUZZLEFLASH;
	
	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );

	m_flLastShot = gpGlobals->time;
}

void COFMedicAlly :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;
	//TODO: add remaining torch grunt events
	switch( pEvent->event )
	{
		case HGRUNT_AE_DROP_GUN:
			{
				Vector	vecGunPos;
				Vector	vecGunAngles;

				GetAttachment( 0, vecGunPos, vecGunAngles );

				// switch to body group with no gun.
				SetBodygroup( MedicAllyBodygroup::Weapons, MedicAllyWeapon::None );

				// now spawn a gun.
				if( pev->weapons & MedicAllyWeaponFlag::Glock )
				{
					DropItem( "weapon_9mmhandgun", vecGunPos, vecGunAngles );
				}
				else
				{
					DropItem( "weapon_eagle", vecGunPos, vecGunAngles );
				}

				m_iWeaponIdx = MedicAllyWeapon::None;
			}
			break;

		case HGRUNT_AE_RELOAD:

			if( pev->weapons & MedicAllyWeaponFlag::DesertEagle )
			{
				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "weapons/desert_eagle_reload.wav", 1, ATTN_NORM );
			}
			else
			{
				EMIT_SOUND( ENT( pev ), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM );
			}

			m_cAmmoLoaded = m_cClipSize;
			ClearConditions(bits_COND_NO_AMMO_LOADED);
			break;

		case HGRUNT_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
		}
		break;

		case MEDIC_AE_SHOOT:
		{
			Shoot();
		}
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
			CBaseHGrunt::HandleAnimEvent( pEvent );
			break;
	}
}

void COFMedicAlly :: Spawn()
{
	BaseSpawn("models/hgrunt_medic.mdl");

	m_afCapability		= bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP | bits_CAP_HEAR;

	//Note: this code has been rewritten to use SetBodygroup since it relies on hardcoded offsets in the original
	pev->body = 0;

	m_flLastUseTime = 0;
	m_iHealCharge = gSkillData.scientistHeal;
	m_fGunHolstered = false;
	m_fHypoHolstered = true;
	m_fHealActive = false;
	m_fQueueFollow = false;
	m_fUseHealing = false;
	m_fHealing = false;
	m_hNewTargetEnt = nullptr;
	m_fFollowChecked = false;
	m_fFollowChecking = false;


	if( !pev->weapons )
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

	SetBodygroup( MedicAllyBodygroup::Head, m_iBlackOrWhite );
	SetBodygroup( MedicAllyBodygroup::Weapons, m_iWeaponIdx );

	m_cAmmoLoaded = m_cClipSize;

	m_flLastShot = gpGlobals->time;

	pev->skin = 0;

	if( m_iBlackOrWhite == MedicAllyHead::Black )
	{
		m_voicePitch = 95;
	}

	CTalkSquadMonster::g_talkWaitTime = 0;

	SetUse( &COFMedicAlly::HealerUse );

	// get voice pitch
	m_voicePitch = 105;
}

void COFMedicAlly :: Precache()
{
	PRECACHE_MODEL("models/hgrunt_medic.mdl");

	TalkInit();
	
	PRECACHE_SOUND( "fgrunt/death1.wav" );
	PRECACHE_SOUND( "fgrunt/death2.wav" );
	PRECACHE_SOUND( "fgrunt/death3.wav" );
	PRECACHE_SOUND( "fgrunt/death4.wav" );
	PRECACHE_SOUND( "fgrunt/death5.wav" );
	PRECACHE_SOUND( "fgrunt/death6.wav" );

	PRECACHE_SOUND( "fgrunt/pain1.wav" );
	PRECACHE_SOUND( "fgrunt/pain2.wav" );
	PRECACHE_SOUND( "fgrunt/pain3.wav" );
	PRECACHE_SOUND( "fgrunt/pain4.wav" );
	PRECACHE_SOUND( "fgrunt/pain5.wav" );
	PRECACHE_SOUND( "fgrunt/pain6.wav" );

	PRECACHE_SOUND( "weapons/desert_eagle_fire.wav" );
	PRECACHE_SOUND( "weapons/desert_eagle_reload.wav" );

	PRECACHE_SOUND( "fgrunt/medic_give_shot.wav" );
	PRECACHE_SOUND( "fgrunt/medical.wav" );

	PRECACHE_SOUND( "fgrunt/torch_light.wav" );
	PRECACHE_SOUND( "fgrunt/torch_cut_loop.wav" );
	PRECACHE_SOUND( "fgrunt/medic.wav" );

	BasePrecache();
}	

void COFMedicAlly :: StartTask ( Task_t *pTask )
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
		CTalkSquadMonster:: StartTask( pTask );
		break;
	}
}

void COFMedicAlly :: RunTask ( Task_t *pTask )
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
			CBaseHGrunt:: RunTask( pTask );
			break;
		}
	}
}

void COFMedicAlly :: PainSound ( void )
{
	if ( gpGlobals->time > m_flNextPainTime )
	{
#if 0
		if ( RANDOM_LONG(0,99) < 5 )
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "FG_PAIN", MEDIC_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
				JustSpoke();
				return;
			}
		}
#endif 
		switch ( RANDOM_LONG(0,7) )
		{
		case 0:	
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "fgrunt/pain3.wav", 1, ATTN_NORM );	
			break;
		case 1:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "fgrunt/pain4.wav", 1, ATTN_NORM );	
			break;
		case 2:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "fgrunt/pain5.wav", 1, ATTN_NORM );	
			break;
		case 3:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "fgrunt/pain1.wav", 1, ATTN_NORM );	
			break;
		case 4:
			EMIT_SOUND( ENT(pev), CHAN_VOICE, "fgrunt/pain2.wav", 1, ATTN_NORM );	
			break;
		case 5:
			EMIT_SOUND( ENT( pev ), CHAN_VOICE, "fgrunt/pain6.wav", 1, ATTN_NORM );
			break;
		}

		m_flNextPainTime = gpGlobals->time + 1;
	}
}

void COFMedicAlly :: DeathSound ( void )
{
	//TODO: these sounds don't exist, the gr_ prefix is wrong
	switch ( RANDOM_LONG(0,5) )
	{
	case 0:	
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_death1.wav", 1, ATTN_IDLE );	
		break;
	case 1:
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_death2.wav", 1, ATTN_IDLE );	
		break;
	case 2:
		EMIT_SOUND( ENT(pev), CHAN_VOICE, "hgrunt/gr_death3.wav", 1, ATTN_IDLE );	
		break;
	case 3:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_death4.wav", 1, ATTN_IDLE );
		break;
	case 4:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_death5.wav", 1, ATTN_IDLE );
		break;
	case 5:
		EMIT_SOUND( ENT( pev ), CHAN_VOICE, "hgrunt/gr_death6.wav", 1, ATTN_IDLE );
		break;
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
		"Draw Needle"
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
		"Draw Needle"
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
		"Draw Gun"
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
		"Medic Ally Heal Target"
	},
};

DEFINE_CUSTOM_SCHEDULES( COFMedicAlly )
{
	slMedicAllyNewHealTarget,
	slMedicAllyDrawNeedle,
	slMedicAllyDrawGun,
	slMedicAllyHealTarget,
};

IMPLEMENT_CUSTOM_SCHEDULES( COFMedicAlly, CBaseHGrunt );


int COFMedicAlly::GetActivitySequence(Activity NewActivity)
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
		iSequence = CBaseHGrunt::GetActivitySequence(NewActivity);
		break;
	}
	
	return iSequence;
}

Schedule_t *COFMedicAlly :: GetSchedule( void )
{

	// clear old sentence
	m_iSentence = MEDIC_SENT_NONE;

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
					SENTENCEG_PlayRndSz( ENT(pev), "FG_GREN", MEDIC_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
	switch	( m_MonsterState )
	{
	case MONSTERSTATE_COMBAT:
		{
			// dead enemy
			if ( HasConditions( bits_COND_ENEMY_DEAD ) )
			{
				if( FOkToSpeak() )
				{
					PlaySentence( "FG_KILL", 4, VOL_NORM, ATTN_NORM );
				}

				// call base class, all code to handle dead enemies is centralized there.
				return CBaseMonster :: GetSchedule();
			}

			if( m_hWaitMedic )
			{
				CTalkSquadMonster* pMedic = m_hWaitMedic->MyTalkSquadMonsterPointer();

				if( pMedic->pev->deadflag != DEAD_NO )
					m_hWaitMedic = nullptr;
				else
					pMedic->HealMe( nullptr );

				m_flMedicWaitTime = gpGlobals->time + 5.0;
			}

			// new enemy
			//Do not fire until fired upon
			if ( HasAllConditions( bits_COND_NEW_ENEMY | bits_COND_LIGHT_DAMAGE ) )
			{
				if ( InSquad() ) {
					return GetNewSquadEnemySchedule();
				}

				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}

			else if( HasConditions( bits_COND_HEAVY_DAMAGE ) )
				return GetScheduleOfType( SCHED_TAKE_COVER_FROM_ENEMY );
			// no ammo
			//Only if the grunt has a weapon
			else if ( pev->weapons && HasConditions ( bits_COND_NO_AMMO_LOADED ) )
			{
				//!!!KELLY - this individual just realized he's out of bullet ammo. 
				// He's going to try to find cover to run to and reload, but rarely, if 
				// none is available, he'll drop and reload in the open here. 
				return GetScheduleOfType ( SCHED_GRUNT_COVER_AND_RELOAD );
			}
			
			// damaged just a little
			else if ( HasConditions( bits_COND_LIGHT_DAMAGE ) )
			{
				return GetLightDamageSchedule();
			}
			// can kick
			else if ( HasConditions ( bits_COND_CAN_MELEE_ATTACK1 ) )
			{
				return GetScheduleOfType ( SCHED_MELEE_ATTACK1 );
			}
			// can shoot
			else if ( HasConditions ( bits_COND_CAN_RANGE_ATTACK1 ) )
			{
				return GetShootSchedule();
			}
			// can't see enemy
			else if ( HasConditions( bits_COND_ENEMY_OCCLUDED ) )
			{
				return GetEnemyOccludedSchedule();
			}
			
			//Only if not following a player
			if( !m_hTargetEnt || !m_hTargetEnt->IsPlayer() )
			{
				if( HasConditions( bits_COND_SEE_ENEMY ) && !HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) )
				{
					return GetScheduleOfType( SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE );
				}
			}

			//Don't fall through to idle schedules
			break;
		}

		case MONSTERSTATE_ALERT:
		case MONSTERSTATE_IDLE:
			if( HasConditions( bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE ) )
			{
				// flinch if hurt
				return GetScheduleOfType( SCHED_SMALL_FLINCH );
			}

			//if we're not waiting on a medic and we're hurt, call out for a medic
			if( !m_hWaitMedic
				&& gpGlobals->time > m_flMedicWaitTime
				&& pev->health <= 20.0 )
			{
				auto pMedic = MySquadMedic();

				if( !pMedic )
				{
					pMedic = FindSquadMedic( 1024 );
				}

				if( pMedic )
				{
					if( pMedic->pev->deadflag == DEAD_NO )
					{
						ALERT( at_aiconsole, "Injured Grunt found Medic\n" );

						if( pMedic->HealMe( this ) )
						{
							ALERT( at_aiconsole, "Injured Grunt called for Medic\n" );

							EMIT_SOUND_DYN( edict(), CHAN_VOICE, "fgrunt/medic.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM );

							JustSpoke();
							m_flMedicWaitTime = gpGlobals->time + 5.0;
						}
					}
				}
			}

			if( m_hEnemy == NULL && IsFollowing() )
			{
				if( !m_hTargetEnt->IsAlive() )
				{
					// UNDONE: Comment about the recently dead player here?
					StopFollowing( FALSE );
					break;
				}
				else
				{
					if( HasConditions( bits_COND_CLIENT_PUSH ) )
					{
						return GetScheduleOfType( SCHED_MOVE_AWAY_FOLLOW );
					}
					return GetScheduleOfType( SCHED_TARGET_FACE );
				}
			}

			if( HasConditions( bits_COND_CLIENT_PUSH ) )
			{
				return GetScheduleOfType( SCHED_MOVE_AWAY );
			}

			// try to say something about smells
			TrySmellTalk();
			break;
	}
	
	// no special cases here, call the base class
	return CTalkSquadMonster:: GetSchedule();
}

Schedule_t* COFMedicAlly :: GetScheduleOfType ( int Type ) 
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
						SENTENCEG_PlayRndSz( ENT(pev), "FG_THROW", MEDIC_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
			return CBaseHGrunt::GetScheduleOfType ( Type );
		}
	}
}

int COFMedicAlly::ObjectCaps()
{
	//Allow healing the player by continuously using
	return FCAP_ACROSS_TRANSITION | FCAP_CONTINUOUS_USE;
}

void COFMedicAlly::TalkInit()
{
	CTalkSquadMonster::TalkInit();

	m_szGrp[ TLK_ANSWER ] = "FG_ANSWER";
	m_szGrp[ TLK_QUESTION ] = "FG_QUESTION";
	m_szGrp[ TLK_IDLE ] = "FG_IDLE";
	m_szGrp[ TLK_STARE ] = "FG_STARE";
	m_szGrp[ TLK_USE ] = "FG_OK";
	m_szGrp[ TLK_UNUSE ] = "FG_WAIT";
	m_szGrp[ TLK_STOP ] = "FG_STOP";

	m_szGrp[ TLK_NOSHOOT ] = "FG_SCARED";
	m_szGrp[ TLK_HELLO ] = "FG_HELLO";

	m_szGrp[ TLK_PLHURT1 ] = "!FG_CUREA";
	m_szGrp[ TLK_PLHURT2 ] = "!FG_CUREB";
	m_szGrp[ TLK_PLHURT3 ] = "!FG_CUREC";

	m_szGrp[ TLK_PHELLO ] = NULL;	//"BA_PHELLO";		// UNDONE
	m_szGrp[ TLK_PIDLE ] = NULL;	//"BA_PIDLE";			// UNDONE
	m_szGrp[ TLK_PQUESTION ] = "FG_PQUEST";		// UNDONE

	m_szGrp[ TLK_SMELL ] = "FG_SMELL";

	m_szGrp[ TLK_WOUND ] = "FG_WOUND";
	m_szGrp[ TLK_MORTAL ] = "FG_MORTAL";

	// get voice for head - just one barney voice for now
	m_voicePitch = 100;
}

void COFMedicAlly::AlertSound()
{
	if( m_hEnemy && FOkToSpeak() )
	{
		PlaySentence( "FG_ATTACK", RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_NORM );
	}
}

void COFMedicAlly::DeclineFollowing()
{
	PlaySentence( "FG_POK", 2, VOL_NORM, ATTN_NORM );
}

void COFMedicAlly::KeyValue( KeyValueData *pkvd )
{
	if( FStrEq( pkvd->szKeyName, "head" ) )
	{
		m_iBlackOrWhite = atoi( pkvd->szValue );
		pkvd->fHandled = true;
	}
	else
		CTalkSquadMonster::KeyValue( pkvd );
}

void COFMedicAlly::Killed( entvars_t* pevAttacker, int iGib )
{
	if( m_hTargetEnt != nullptr )
	{
		auto pSquadMonster = m_hTargetEnt->MyTalkSquadMonsterPointer();

		if( pSquadMonster )
			pSquadMonster->m_hWaitMedic = nullptr;
	}

	//TODO: missing from medic?
	/*
	if( m_MonsterState != MONSTERSTATE_DEAD )
	{
		if( HasMemory( bits_MEMORY_SUSPICIOUS ) || IsFacing( pevAttacker, pev->origin ) )
		{
			Remember( bits_MEMORY_PROVOKED );

			StopFollowing( true );
		}
	}
	*/

	SetUse( nullptr );

	CTalkSquadMonster::Killed( pevAttacker, iGib );
}

void COFMedicAlly::MonsterThink()
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

BOOL COFMedicAlly::HealMe( CTalkSquadMonster* pTarget )
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

void COFMedicAlly::HealOff()
{
	m_fHealing = false;

	if( m_movementGoal == MOVEGOAL_TARGETENT )
		RouteClear();

	m_hTargetEnt = nullptr;
	ClearSchedule();

	SetThink( nullptr );
	pev->nextthink = 0;
}

void COFMedicAlly::HealerActivate( CBaseMonster* pTarget )
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

void COFMedicAlly::HealerUse( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
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
					SENTENCEG_PlayRndSz( edict(), "MG_HEAL", MEDIC_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
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
				SENTENCEG_PlayRndSz( edict(), "MG_NOTHEAL", MEDIC_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch );
			}
		}

		m_flLastUseTime = gpGlobals->time + 0.2;
		return;
	}

	m_fFollowChecking = true;
	m_flFollowCheckTime = gpGlobals->time;
}
