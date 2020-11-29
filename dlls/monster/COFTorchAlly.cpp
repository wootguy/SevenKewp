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
#include "CBaseGrunt.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"
#include "explode.h"

int g_fTorchAllyQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int		g_iSkillLevel;

#define	TORCH_DEAGLE_CLIP_SIZE			8 // how many bullets in a clip?
#define	TORCH_SENTENCE_VOLUME			(float)0.35 // volume of grunt sentences
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

#define		TORCH_AE_RELOAD		( 2 )
#define		TORCH_AE_KICK			( 3 )
#define		TORCH_AE_SHOOT			( 4 )
#define		TORCH_AE_GREN_TOSS		( 7 )
#define		TORCH_AE_GREN_DROP		( 9 )
#define		TORCH_AE_CAUGHT_ENEMY	( 10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define		TORCH_AE_DROP_GUN		( 11) // grunt (probably dead) is dropping his mp5.

#define TORCH_AE_HOLSTER_TORCH		17
#define TORCH_AE_HOLSTER_GUN		18
#define TORCH_AE_HOLSTER_BOTH		19
#define TORCH_AE_ACTIVATE_TORCH		20
#define TORCH_AE_DEACTIVATE_TORCH	21

class COFTorchAlly : public CBaseGrunt
{
public:
	void Spawn( void );
	void Precache( void );
	int ISoundMask ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	int GetActivitySequence(Activity NewActivity);
	void DeathSound( void );
	void PainSound( void );
	void IdleSound ( void );
	void Shoot ( void );
	int	Classify(void);
	void GibMonster( void );

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	
	Schedule_t	*GetMonsterStateSchedule( void );
	Schedule_t  *GetScheduleOfType ( int Type );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	int ObjectCaps() override;

	void TalkInit();

	void AlertSound() override;

	void DeclineFollowing() override;

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

	BOOL m_lastAttackCheck;
	float m_flPlayerDamage;

	BOOL m_fUseTorch;
	EHANDLE m_hNewTargetEnt;
	int m_iBlackOrWhite;
	BOOL m_fGunHolstered;
	BOOL m_fTorchHolstered;
	BOOL m_fTorchActive;

	CBeam* m_pTorchBeam;

	float m_flLastShot;

	static const char *pTorchSentences[];
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

const char *COFTorchAlly::pTorchSentences[] = 
{
	"FG_GREN", // grenade scared grunt
	"FG_ALERT", // sees player
	"FG_MONSTER", // sees monster
	"FG_COVER", // running to cover
	"FG_THROW", // about to throw grenade
	"FG_CHARGE",  // running out to get the enemy
	"FG_TAUNT", // say rude things
};

int	COFTorchAlly::Classify(void)
{
	return	CLASS_HUMAN_MILITARY_FRIENDLY;
}

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

	Vector	vecGunPos;
	Vector	vecGunAngles;

	//TODO: probably the wrong logic, but it was in the original
	if ( GetBodygroup( TorchAllyBodygroup::Weapons ) != TorchAllyWeapon::DesertEagle )
	{// throw a gun if the grunt has one
		GetAttachment( 0, vecGunPos, vecGunAngles );
		
		CBaseEntity *pGun = DropItem( "weapon_eagle", vecGunPos, vecGunAngles );

		if ( pGun )
		{
			pGun->pev->velocity = Vector (RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));
			pGun->pev->avelocity = Vector ( 0, RANDOM_FLOAT( 200, 400 ), 0 );
		}

		//Note: this wasn't in the original
		SetBodygroup( TorchAllyBodygroup::Weapons, TorchAllyWeapon::None );
	}

	if( m_fTorchActive )
	{
		m_fTorchActive = false;
		UTIL_Remove( m_pTorchBeam );
		m_pTorchBeam = nullptr;
	}

	CBaseMonster :: GibMonster();
}

int COFTorchAlly :: ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			bits_SOUND_DANGER	|
			bits_SOUND_CARCASS	|
			bits_SOUND_MEAT		|
			bits_SOUND_GARBAGE;
}

BOOL COFTorchAlly :: CheckRangeAttack1 ( float flDot, float flDist )
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
	// check for helmet shot
	else if (ptr->iHitgroup == 11)
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

int COFTorchAlly :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
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

void COFTorchAlly :: IdleSound( void )
{
	if (FOkToSpeak() && (g_fTorchAllyQuestion || RANDOM_LONG(0,1)))
	{
		if (!g_fTorchAllyQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0,2))
			{
			case 0: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CHECK", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fTorchAllyQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "FG_QUEST", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fTorchAllyQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "FG_IDLE", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fTorchAllyQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CLEAR", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "FG_ANSWER", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fTorchAllyQuestion = 0;
		}
		JustSpoke();
	}
}

void COFTorchAlly :: Shoot ( void )
{
	//Limit fire rate
	if (m_hEnemy == NULL || gpGlobals->time - m_flLastShot <= 0.11 )
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy( vecShootOrigin );

	UTIL_MakeVectors ( pev->angles );

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_PLAYER_357, 0 ); // shoot +-5 degrees

	const auto random = RANDOM_LONG( 0, 20 );

	const auto pitch = random <= 10 ? random + 95 : 100;

	EMIT_SOUND_DYN( edict(), CHAN_WEAPON, "weapons/desert_eagle_fire.wav", VOL_NORM, ATTN_NORM, 0, pitch );

	pev->effects |= EF_MUZZLEFLASH;
	
	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );

	m_flLastShot = gpGlobals->time;
}

void COFTorchAlly :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	Vector	vecShootDir;
	Vector	vecShootOrigin;
	//TODO: add remaining torch grunt events
	switch( pEvent->event )
	{
		case TORCH_AE_DROP_GUN:
			{
				//If we don't have a gun equipped
				//TODO: why is it checking like this?
				if( GetBodygroup( TorchAllyBodygroup::Weapons ) != TorchAllyWeapon::DesertEagle )
				{
					Vector	vecGunPos;
					Vector	vecGunAngles;

					GetAttachment( 0, vecGunPos, vecGunAngles );

					// switch to body group with no gun.
					SetBodygroup( TorchAllyBodygroup::Weapons, TorchAllyWeapon::None );

					// now spawn a gun.
					DropItem( "weapon_eagle", vecGunPos, vecGunAngles );
				}
			}
			break;

		case TORCH_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
		}
		break;

		case TORCH_AE_SHOOT:
		{
			Shoot();
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
	BaseSpawn("models/hgrunt_torch.mdl");

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
}

void COFTorchAlly :: Precache()
{
	PRECACHE_MODEL("models/hgrunt_torch.mdl");
	PRECACHE_MODEL( TORCH_BEAM_SPRITE );

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

	PRECACHE_SOUND( "fgrunt/torch_light.wav" );
	PRECACHE_SOUND( "fgrunt/torch_cut_loop.wav" );
	PRECACHE_SOUND( "fgrunt/medic.wav" );

	BasePrecache();
}	

void COFTorchAlly :: PainSound ( void )
{
	if ( gpGlobals->time > m_flNextPainTime )
	{
#if 0
		if ( RANDOM_LONG(0,99) < 5 )
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "FG_PAIN", TORCH_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
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

void COFTorchAlly :: DeathSound ( void )
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

Schedule_t *COFTorchAlly :: GetMonsterStateSchedule( void )
{
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
				if ( InSquad() )
				{
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
						SENTENCEG_PlayRndSz( ENT(pev), "FG_THROW", TORCH_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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

int COFTorchAlly::ObjectCaps()
{
	return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
}

void COFTorchAlly::TalkInit()
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

void COFTorchAlly::AlertSound()
{
	if( m_hEnemy && FOkToSpeak() )
	{
		PlaySentence( "FG_ATTACK", RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_NORM );
	}
}

void COFTorchAlly::DeclineFollowing()
{
	PlaySentence( "FG_POK", 2, VOL_NORM, ATTN_NORM );
}

void COFTorchAlly::Killed( entvars_t* pevAttacker, int iGib )
{
	if( m_hTargetEnt != nullptr )
	{
		m_hTargetEnt->MyTalkSquadMonsterPointer()->m_hWaitMedic = nullptr;
	}

	if( m_MonsterState != MONSTERSTATE_DEAD )
	{
		if( HasMemory( bits_MEMORY_SUSPICIOUS ) || IsFacing( pevAttacker, pev->origin ) )
		{
			Remember( bits_MEMORY_PROVOKED );

			StopFollowing( true );
		}
	}

	if( m_hWaitMedic )
	{
		CTalkSquadMonster* v4 = m_hWaitMedic->MyTalkSquadMonsterPointer();
		if( v4->pev->deadflag )
			m_hWaitMedic = nullptr;
		else
			v4->HealMe( nullptr );
	}

	SetUse( nullptr );

	if( m_fTorchActive )
	{
		m_fTorchActive = false;
		UTIL_Remove( m_pTorchBeam );
		m_pTorchBeam = nullptr;
	}

	CTalkSquadMonster::Killed( pevAttacker, iGib );
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
