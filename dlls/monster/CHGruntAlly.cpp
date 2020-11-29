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
#include "CTalkSquadMonster.h"
#include "CBaseHGrunt.h"
#include "CSoundEnt.h"
#include "effects.h"
#include "customentity.h"

int g_fGruntAllyQuestion;				// true if an idle grunt asked a question. Cleared when someone answers.

extern DLL_GLOBAL int		g_iSkillLevel;

#define	GRUNT_MP5_CLIP_SIZE				36 // how many bullets in a clip? - NOTE: 3 round burst sound, so keep as 3 * x!
#define GRUNT_SHOTGUN_CLIP_SIZE			8
#define GRUNT_SAW_CLIP_SIZE				36
#define	HGRUNT_SENTENCE_VOLUME			(float)0.35 // volume of grunt sentences

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

class CHGruntAlly : public CBaseHGrunt
{
public:
	void Spawn( void );
	void Precache( void );
	int  Classify ( void );
	int ISoundMask ( void );
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL CheckRangeAttack1 ( float flDot, float flDist );
	void DeathSound( void );
	void PainSound( void );
	void IdleSound ( void );
	void GibMonster( void );
	void PlaySentenceSound(int sentenceType);

	int	Save( CSave &save ); 
	int Restore( CRestore &restore );
	
	int GetActivitySequence(Activity NewActivity);
	Schedule_t  * GetMonsterStateSchedule( );
	Schedule_t  *GetScheduleOfType ( int Type );
	void TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	int TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType );

	int ObjectCaps() override;

	void TalkInit();

	void AlertSound() override;

	void DeclineFollowing() override;

	void ShootSaw();

	void KeyValue( KeyValueData* pkvd ) override;

	void Killed( entvars_t* pevAttacker, int iGib ) override;

	MONSTERSTATE GetIdealState()
	{
		return CTalkSquadMonster::GetIdealState();
	}

	static TYPEDESCRIPTION m_SaveData[];

	BOOL m_lastAttackCheck;
	float m_flPlayerDamage;

	int m_iSawShell;
	int m_iSawLink;

	int m_iWeaponIdx;
	int m_iGruntHead;
	int m_iGruntTorso;

	static const char *pGruntSentences[];
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

const char *CHGruntAlly::pGruntSentences[] = 
{
	"FG_GREN", // grenade scared grunt
	"FG_ALERT", // sees player
	"FG_MONSTER", // sees monster
	"FG_COVER", // running to cover
	"FG_THROW", // about to throw grenade
	"FG_CHARGE",  // running out to get the enemy
	"FG_TAUNT", // say rude things
};

void CHGruntAlly::PlaySentenceSound(int sentenceType) {
	SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[m_iSentence], HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
}

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

int CHGruntAlly :: ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			bits_SOUND_DANGER	|
			bits_SOUND_CARCASS	|
			bits_SOUND_MEAT		|
			bits_SOUND_GARBAGE;
}

BOOL CHGruntAlly :: CheckRangeAttack1 ( float flDot, float flDist )
{
	//Only if we have a weapon
	if( pev->weapons )
	{
		const auto maxDistance = pev->weapons & HGruntAllyWeaponFlag::Shotgun ? 640 : 1024;

		//Friendly fire is allowed
		if( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= maxDistance && flDot >= 0.5 /*&& NoFriendlyFire()*/ )
		{
			TraceResult	tr;

			CBaseEntity* pEnemy = m_hEnemy;

			if( !pEnemy->IsPlayer() && flDist <= 64 )
			{
				// kick nonclients, but don't shoot at them.
				return FALSE;
			}

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

void CHGruntAlly :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
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

int CHGruntAlly :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	Forget( bits_MEMORY_INCOVER );

	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CTalkSquadMonster::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );

	if( pev->deadflag != DEAD_NO )
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

void CHGruntAlly :: IdleSound( void )
{
	if (FOkToSpeak() && (g_fGruntAllyQuestion || RANDOM_LONG(0,1)))
	{
		if (!g_fGruntAllyQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0,2))
			{
			case 0: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CHECK", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntAllyQuestion = 1;
				break;
			case 1: // question
				SENTENCEG_PlayRndSz(ENT(pev), "FG_QUEST", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fGruntAllyQuestion = 2;
				break;
			case 2: // statement
				SENTENCEG_PlayRndSz(ENT(pev), "FG_IDLE", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fGruntAllyQuestion)
			{
			case 1: // check in
				SENTENCEG_PlayRndSz(ENT(pev), "FG_CLEAR", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question 
				SENTENCEG_PlayRndSz(ENT(pev), "FG_ANSWER", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fGruntAllyQuestion = 0;
		}
		JustSpoke();
	}
}

int	CHGruntAlly :: Classify ( void )
{
	return	CLASS_HUMAN_MILITARY_FRIENDLY;
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
			CBaseHGrunt::HandleAnimEvent( pEvent );
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

	SetUse( &CHGruntAlly::FollowerUse );
}

void CHGruntAlly :: Precache()
{
	PRECACHE_MODEL("models/hgrunt_opfor.mdl");

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

	PRECACHE_SOUND( "weapons/saw_fire1.wav" );
	PRECACHE_SOUND( "weapons/saw_fire2.wav" );
	PRECACHE_SOUND( "weapons/saw_fire3.wav" );
	PRECACHE_SOUND( "weapons/saw_reload.wav" );

	PRECACHE_SOUND( "fgrunt/medic.wav" );

	// get voice pitch
	m_voicePitch = 100;

	m_iSawShell = PRECACHE_MODEL("models/saw_shell.mdl");
	m_iSawLink = PRECACHE_MODEL("models/saw_link.mdl");

	CTalkSquadMonster::Precache();
}	

void CHGruntAlly :: PainSound ( void )
{
	if ( gpGlobals->time > m_flNextPainTime )
	{
#if 0
		if ( RANDOM_LONG(0,99) < 5 )
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "FG_PAIN", HGRUNT_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
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

void CHGruntAlly :: DeathSound ( void )
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
		iSequence = CBaseHGrunt::GetActivitySequence(NewActivity);
		break;
	}

	return iSequence;
}

Schedule_t *CHGruntAlly :: GetMonsterStateSchedule( void )
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
		// can grenade launch
		else if ( FBitSet( pev->weapons, HGRUNT_GRENADELAUNCHER) && HasConditions ( bits_COND_CAN_RANGE_ATTACK2 ) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
		{
			// shoot a grenade if you can
			return GetScheduleOfType( SCHED_RANGE_ATTACK2 );
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
	return CBaseHGrunt::GetMonsterStateSchedule();
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
						SENTENCEG_PlayRndSz( ENT(pev), "FG_THROW", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
			return CBaseHGrunt:: GetScheduleOfType ( Type );
		}
	}
}

int CHGruntAlly::ObjectCaps()
{
	return FCAP_ACROSS_TRANSITION | FCAP_IMPULSE_USE;
}

void CHGruntAlly::TalkInit()
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

void CHGruntAlly::AlertSound()
{
	if( m_hEnemy && FOkToSpeak() )
	{
		PlaySentence( "FG_ATTACK", RANDOM_FLOAT( 2.8, 3.2 ), VOL_NORM, ATTN_NORM );
	}
}

void CHGruntAlly::DeclineFollowing()
{
	PlaySentence( "FG_POK", 2, VOL_NORM, ATTN_NORM );
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

void CHGruntAlly::Killed( entvars_t* pevAttacker, int iGib )
{
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
	CTalkSquadMonster::Killed( pevAttacker, iGib );
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
