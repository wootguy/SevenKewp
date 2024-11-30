#include	"extdll.h"
#include	"plane.h"
#include	"util.h"
#include	"monsters.h"
#include	"schedule.h"
#include	"animation.h"
#include	"CTalkSquadMonster.h"
#include	"weapons.h"
#include	"CTalkSquadMonster.h"
#include	"env/CSoundEnt.h"
#include	"effects.h"
#include	"customentity.h"
#include	"CBaseGrunt.h"
#include	"defaultai.h"
#include	"weapon/CGrenade.h"
#include	"skill.h"

int g_fGruntQuestion;

TYPEDESCRIPTION	CBaseGrunt::m_SaveData[] = 
{
	DEFINE_FIELD( CBaseGrunt, m_flNextGrenadeCheck, FIELD_TIME ),
	DEFINE_FIELD( CBaseGrunt, m_flNextPainTime, FIELD_TIME ),
//	DEFINE_FIELD( CBaseGrunt, m_flLastEnemySightTime, FIELD_TIME ), // don't save, go to zero
	DEFINE_FIELD( CBaseGrunt, m_vecTossVelocity, FIELD_VECTOR ),
	DEFINE_FIELD( CBaseGrunt, m_fThrowGrenade, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseGrunt, m_fStanding, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseGrunt, m_fFirstEncounter, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBaseGrunt, m_cClipSize, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseGrunt, m_voicePitch, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iBrassShell, FIELD_INTEGER ),
//  DEFINE_FIELD( CShotgun, m_iShotgunShell, FIELD_INTEGER ),
	DEFINE_FIELD( CBaseGrunt, m_iSentence, FIELD_INTEGER ),
};

IMPLEMENT_SAVERESTORE( CBaseGrunt, CTalkSquadMonster )

//=========================================================
// Speak Sentence - say your cued up sentence.
//
// Some grunt sentences (take cover and charge) rely on actually
// being able to execute the intended action. It's really lame
// when a grunt says 'COVER ME' and then doesn't move. The problem
// is that the sentences were played when the decision to TRY
// to move to cover was made. Now the sentence is played after 
// we know for sure that there is a valid path. The schedule
// may still fail but in most cases, well after the grunt has 
// started moving.
//=========================================================
void CBaseGrunt :: SpeakSentence( void )
{
	if ( m_iSentence == HGRUNT_SENT_NONE )
	{
		// no sentence cued up.
		return; 
	}

	if (FOkToSpeak())
	{
		PlaySentenceSound(m_iSentence);
		JustSpoke();
	}
}

int CBaseGrunt::IRelationship ( CBaseEntity *pTarget )
{
	// Only override if there is at least a dislike relationship already,
	// because level designers may override the class.

	int r = CTalkSquadMonster :: IRelationship( pTarget );

	if ( (r >= R_DL) && ( FClassnameIs( pTarget->pev, "monster_alien_grunt" ) || ( FClassnameIs( pTarget->pev,  "monster_gargantua" ) ) ) )
	{
		return R_NM;
	}
	else
		return r;
}

void CBaseGrunt :: GibMonster ( void )
{
	DropEquipment(0, true);
	CBaseMonster::GibMonster();
}

void CBaseGrunt::Killed(entvars_t* pevAttacker, int iGib)
{
	m_deadEquipment = m_iEquipment;

	if (m_MonsterState != MONSTERSTATE_DEAD) //TODO: skip this for medic?
	{
		if (HasMemory(bits_MEMORY_SUSPICIOUS) || IsFacing(pevAttacker, pev->origin))
		{
			Remember(bits_MEMORY_PROVOKED);

			StopFollowing(true);
		}
	}

	if (m_hWaitMedic)
	{
		CTalkSquadMonster* medic = m_hWaitMedic->MyTalkSquadMonsterPointer();
		if (medic->pev->deadflag)
			m_hWaitMedic = NULL;
		else
			medic->HealMe(NULL);
	}

	SetUse(NULL);
	CTalkSquadMonster::Killed(pevAttacker, iGib);
}

//=========================================================
// ISoundMask - Overidden for human grunts because they 
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CBaseGrunt :: ISoundMask ( void )
{
	return	bits_SOUND_WORLD	|
			bits_SOUND_COMBAT	|
			bits_SOUND_PLAYER	|
			bits_SOUND_DANGER;
}

BOOL CBaseGrunt :: FOkToSpeak( void )
{
// if someone else is talking, don't speak
	if (gpGlobals->time <= CTalkSquadMonster::g_talkWaitTime)
		return FALSE;

	if ( pev->spawnflags & SF_MONSTER_GAG )
	{
		if ( m_MonsterState != MONSTERSTATE_COMBAT )
		{
			// no talking outside of combat if gagged.
			return FALSE;
		}
	}

	// if player is not in pvs, don't speak
//	if (FNullEnt(FIND_CLIENT_IN_PVS(edict())))
//		return FALSE;
	
	return TRUE;
}

void CBaseGrunt :: JustSpoke( void )
{
	CTalkSquadMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = HGRUNT_SENT_NONE;
}

void CBaseGrunt :: PrescheduleThink ( void )
{
	if ( InSquad() && m_hEnemy != NULL )
	{
		if ( HasConditions ( bits_COND_SEE_ENEMY ) )
		{
			// update the squad's last enemy sighting time.
			MySquadLeader()->m_flLastEnemySightTime = gpGlobals->time;
		}
		else
		{
			if ( gpGlobals->time - MySquadLeader()->m_flLastEnemySightTime > 5 )
			{
				// been a while since we've seen the enemy
				MySquadLeader()->m_fEnemyEluded = TRUE;
			}
		}
	}

	if (suppressOccludedTarget && m_hEnemy != NULL && !HasConditions(bits_COND_ENEMY_OCCLUDED)) {
		m_flLastEnemySightTime = gpGlobals->time;
	}
}

//=========================================================
// FCanCheckAttacks - this is overridden for human grunts
// because they can throw/shoot grenades when they can't see their
// target and the base class doesn't check attacks if the monster
// cannot see its enemy.
//
// !!!BUGBUG - this gets called before a 3-round burst is fired
// which means that a friendly can still be hit with up to 2 rounds. 
// ALSO, grenades will not be tossed if there is a friendly in front,
// this is a bad bug. Friendly machine gun fire avoidance
// will unecessarily prevent the throwing of a grenade as well.
//=========================================================
BOOL CBaseGrunt :: FCanCheckAttacks ( void )
{
	// if you can see the enemy but can't move into optimal range, try attacking it anyway	
	return !HasConditions(bits_COND_ENEMY_TOOFAR) || HasMemory(bits_MEMORY_MOVE_FAILED);
}


BOOL CBaseGrunt :: CheckMeleeAttack1 ( float flDot, float flDist )
{
	CBaseMonster *pEnemy = m_hEnemy != NULL ? m_hEnemy->MyMonsterPointer() : NULL;

	if ( !pEnemy )
	{
		return FALSE;
	}

	if ( flDist <= 64 && flDot >= 0.7	&& 
		 pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		 pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON )
	{
		return TRUE;
	}
	return FALSE;
}

//=========================================================
// CheckRangeAttack1 - overridden for HGrunt, cause 
// FCanCheckAttacks() doesn't disqualify all attacks based
// on whether or not the enemy is occluded because unlike
// the base class, the HGrunt can attack when the enemy is
// occluded (throw grenade over wall, etc). We must 
// disqualify the machine gun attack if the enemy is occluded.
//=========================================================
BOOL CBaseGrunt :: CheckRangeAttack1 ( float flDot, float flDist )
{
	if (!HasEquipment(ANY_RANGED_WEAPON)) {
		return FALSE;
	}

	if ( !HasConditions( bits_COND_ENEMY_OCCLUDED ) && flDist <= maxShootDist && flDot >= 0.5 && NoFriendlyFire() )
	{
		TraceResult	tr;

		if ( !m_hEnemy->IsPlayer() && flDist <= 64 )
		{
			// kick nonclients, but don't shoot at them.
			return FALSE;
		}

		Vector vecSrc = GetGunPosition();

		// verify that a bullet fired from the gun will hit the enemy before the world.
		UTIL_TraceLine( vecSrc, m_hEnemy->BodyTarget(vecSrc), ignore_monsters, ignore_glass, ENT(pev), &tr);

		if ( tr.flFraction == 1.0 || tr.pHit == m_hEnemy.GetEdict() )
		{
			return TRUE;
		}
		
		m_lastAttackCheck = tr.flFraction == 1.0 ? true : tr.pHit && GET_PRIVATE(tr.pHit) == m_hEnemy;

		return m_lastAttackCheck;
	}

	return FALSE;
}

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade attack. 
//=========================================================
BOOL CBaseGrunt :: CheckRangeAttack2 ( float flDot, float flDist )
{
	if (!HasEquipment(MEQUIP_HAND_GRENADE | MEQUIP_GRENADE_LAUNCHER))
	{
		return FALSE;
	}
	
	// if the grunt isn't moving, it's ok to check.
	if ( m_flGroundSpeed != 0 )
	{
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

	// assume things haven't changed too much since last time
	if (gpGlobals->time < m_flNextGrenadeCheck )
	{
		return m_fThrowGrenade;
	}

	if ( !FBitSet ( m_hEnemy->pev->flags, FL_ONGROUND ) && m_hEnemy->pev->waterlevel == 0 && m_vecEnemyLKP.z > pev->absmax.z  )
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to 
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}
	
	Vector vecTarget;

	if (HasEquipment(MEQUIP_HAND_GRENADE))
	{
		// find feet
		if (RANDOM_LONG(0,1))
		{
			// magically know where they are
			if (m_hEnemy->IsBSPModel()) {
				vecTarget = m_hEnemy->GetTargetOrigin();
			}
			else {
				vecTarget = Vector(m_hEnemy->pev->origin.x, m_hEnemy->pev->origin.y, m_hEnemy->pev->absmin.z);
			}
		}
		else
		{
			// toss it to where you last saw them
			vecTarget = m_vecEnemyLKP;
		}
		// vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget( pev->origin ) - m_hEnemy->pev->origin);
		// estimate position
		// vecTarget = vecTarget + m_hEnemy->pev->velocity * 2;
	}
	else
	{
		// find target
		// vecTarget = m_hEnemy->BodyTarget( pev->origin );
		if (m_hEnemy->IsBSPModel()) {
			vecTarget = m_hEnemy->GetTargetOrigin();
		}
		else {
			vecTarget = m_vecEnemyLKP + (m_hEnemy->BodyTarget(pev->origin) - m_hEnemy->pev->origin);
		}

		// estimate position
		if (HasConditions( bits_COND_SEE_ENEMY))
			vecTarget = vecTarget + ((vecTarget - pev->origin).Length() / gSkillData.sk_hgrunt_gspeed) * m_hEnemy->pev->velocity;
	}

	// are any of my squad members near the intended grenade impact area?
	if ( InSquad() )
	{
		if (SquadMemberInRange( vecTarget, 256 ))
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
			m_fThrowGrenade = FALSE;
		}
	}
	
	if ( ( vecTarget - pev->origin ).Length2D() <= 256 )
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		m_fThrowGrenade = FALSE;
		return m_fThrowGrenade;
	}

		
	if (HasEquipment(MEQUIP_HAND_GRENADE))
	{
		Vector vecToss = VecCheckToss( pev, GetGunPosition(), vecTarget, 0.5 );

		if ( vecToss != g_vecZero )
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = FALSE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}
	else
	{
		Vector vecToss = VecCheckThrow( pev, GetGunPosition(), vecTarget, gSkillData.sk_hgrunt_gspeed, 0.5 );

		if ( vecToss != g_vecZero )
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = TRUE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 0.3; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = FALSE;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}

	

	return m_fThrowGrenade;
}


//=========================================================
// TraceAttack - make sure we're not taking it in the helmet
//=========================================================
void CBaseGrunt :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	// check for helmet shot
	if (ptr->iHitgroup == 11)
	{
		// make sure we're wearing one
		if (HasEquipment(MEQUIP_HELMET) && (bitsDamageType & (DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB)))
		{
			// absorb damage
			flDamage -= 20;
			if (flDamage <= 0)
			{
				UTIL_Ricochet( ptr->vecEndPos, 1.0 );
				flDamage = 0.01;
				bitsDamageType &= ~DMG_BLOOD; // don't spawn blood
			}
		}
		// it's head shot anyways
		ptr->iHitgroup = HITGROUP_HEAD;
	}

	CTalkSquadMonster::TraceAttack( pevAttacker, flDamage, vecDir, ptr, bitsDamageType );
}

//=========================================================
// TakeDamage - overridden for the grunt because the grunt
// needs to forget that he is in cover if he's hurt. (Obviously
// not in a safe place anymore).
//=========================================================
int CBaseGrunt :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	if (IsImmune(pevAttacker))
		return 0;

	Forget( bits_MEMORY_INCOVER );

	// make sure friends talk about it if player hurts talkmonsters...
	int ret = CTalkSquadMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);

	if (pev->deadflag != DEAD_NO || !canBeMadAtPlayer)
		return ret;

	CBaseEntity* attacker = (CBaseEntity*)GET_PRIVATE(ENT(pevAttacker));

	if (m_MonsterState != MONSTERSTATE_PRONE && (pevAttacker->flags & FL_CLIENT) && attacker && IRelationship(attacker) == R_AL)
	{
		Forget(bits_MEMORY_INCOVER);

		m_flPlayerDamage += flDamage;

		// This is a heurstic to determine if the player intended to harm me
		// If I have an enemy, we can't establish intent (may just be crossfire)
		if (m_hEnemy == NULL)
		{
			// If the player was facing directly at me, or I'm already suspicious, get mad
			if (gpGlobals->time - m_flLastHitByPlayer < 4.0 && m_iPlayerHits > 2
				&& ((m_afMemory & bits_MEMORY_SUSPICIOUS) || IsFacing(pevAttacker, pev->origin)))
			{
				// Alright, now I'm pissed!
				PlaySentenceSound(HGRUNT_SENT_MAD);

				Remember(bits_MEMORY_PROVOKED);
				StopFollowing(TRUE);
				ALERT(at_console, "Monster is now MAD!\n");
			}
			else
			{
				// Hey, be careful with that
				PlaySentenceSound(HGRUNT_SENT_SHOT);
				Remember(bits_MEMORY_SUSPICIOUS);

				if (4.0 > gpGlobals->time - m_flLastHitByPlayer)
					++m_iPlayerHits;
				else
					m_iPlayerHits = 0;

				m_flLastHitByPlayer = gpGlobals->time;

				ALERT(at_console, "Monster is now SUSPICIOUS!\n");
			}
		}
		else if (!m_hEnemy->IsPlayer())
		{
			PlaySentenceSound(HGRUNT_SENT_SHOT);
		}
	}

	return ret;
}

void CBaseGrunt :: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_IDLE:	
		ys = 150;		
		break;
	case ACT_RUN:	
		ys = 150;	
		break;
	case ACT_WALK:	
		ys = 180;		
		break;
	case ACT_RANGE_ATTACK1:	
		ys = 120;	
		break;
	case ACT_RANGE_ATTACK2:	
		ys = 120;	
		break;
	case ACT_MELEE_ATTACK1:	
		ys = 120;	
		break;
	case ACT_MELEE_ATTACK2:	
		ys = 120;	
		break;
	case ACT_TURN_LEFT:
	case ACT_TURN_RIGHT:	
		ys = 180;
		break;
	case ACT_GLIDE:
	case ACT_FLY:
		ys = 30;
		break;
	default:
		ys = 90;
		break;
	}

	pev->yaw_speed = ys * gSkillData.sk_yawspeed_mult;
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CBaseGrunt :: CheckAmmo ( void )
{
	if ( HasEquipment(ANY_RANGED_WEAPON) && m_cAmmoLoaded <= 0 )
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

int	CBaseGrunt :: Classify ( void )
{
	return	CBaseMonster::Classify(CLASS_HUMAN_MILITARY);
}

CBaseEntity *CBaseGrunt :: Kick( void )
{
	TraceResult tr;

	UTIL_MakeVectors( pev->angles );
	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * 70);

	UTIL_TraceHull( vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr );
	
	if ( tr.pHit )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );
		return pEntity;
	}

	return NULL;
}

Vector CBaseGrunt :: GetGunPosition( )
{
	if (m_fStanding )
	{
		return pev->origin + Vector( 0, 0, 60 );
	}
	else
	{
		return pev->origin + Vector( 0, 0, 48 );
	}
}

bool CBaseGrunt::HasEquipment(int equipItems) {
	return m_iEquipment & equipItems;
}

void CBaseGrunt::Shoot(bool firstRound)
{
	if (m_hEnemy == NULL) {
		return;
	}	

	Vector vecShootOrigin = GetGunPosition();

	if (shellEjectAttachment >= 0) {
		Vector vecAngles;
		GetAttachment(shellEjectAttachment, vecShootOrigin, vecAngles);
	}

	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	if (HasEquipment(MEQUIP_MP5)) {
		ShootMp5(vecShootOrigin, vecShootDir);
		if (firstRound) {
			// the first round of the three round burst plays the sound and puts a sound in the world sound list.
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, RANDOM_LONG(0, 1) ? "hgrunt/gr_mgun1.wav" : "hgrunt/gr_mgun2.wav", 1, ATTN_NORM);
		}
	}
	else if (HasEquipment(MEQUIP_AKIMBO_UZIS)) {
		ShootUzis(vecShootOrigin, vecShootDir);
	}
	else if (HasEquipment(MEQUIP_MINIGUN)) {
		ShootMinigun(vecShootOrigin, vecShootDir);
	}
	else if (HasEquipment(MEQUIP_SNIPER) && gpGlobals->time - m_flLastShot > 0.11) {
		ShootSniper(vecShootOrigin, vecShootDir);
	}
	else if (HasEquipment(MEQUIP_SHOTGUN)) {
		ShootShotgun(vecShootOrigin, vecShootDir);
	}
	else if (HasEquipment(MEQUIP_SAW)) {
		ShootSaw(vecShootOrigin, vecShootDir);
	}
	else if (HasEquipment(MEQUIP_GLOCK) && gpGlobals->time - m_flLastShot > 0.11) {
		ShootGlock(vecShootOrigin, vecShootDir);
	}
	else if (HasEquipment(MEQUIP_DEAGLE) && gpGlobals->time - m_flLastShot > 0.11) {
		ShootDeagle(vecShootOrigin, vecShootDir);
	}
	else if (HasEquipment(MEQUIP_357) && gpGlobals->time - m_flLastShot > 0.11) {
		Shoot357(vecShootOrigin, vecShootDir);
	}
	else {
		return;
	}

	pev->effects |= EF_MUZZLEFLASH;
	
	m_cAmmoLoaded--;// take away a bullet!

	Vector angDir = UTIL_VecToAngles( vecShootDir );
	SetBlending( 0, angDir.x );

	m_flLastShot = gpGlobals->time;

	if (firstRound) {
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 384, 0.3);
	}
}

void CBaseGrunt::ShootMp5(Vector& vecShootOrigin, Vector& vecShootDir)
{
	UTIL_MakeVectors(pev->angles);

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40, 90) + gpGlobals->v_up * RANDOM_FLOAT(75, 200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass(vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL);
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, BULLET_MONSTER_MP5); // shoot +-5 degrees;
	PLAY_DISTANT_SOUND(edict(), DISTANT_9MM);
}

void CBaseGrunt::ShootUzis(Vector& vecShootOrigin, Vector& vecShootDir) {
	UTIL_MakeVectors(pev->angles);

	Vector	leftShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40, 90) + gpGlobals->v_up * RANDOM_FLOAT(75, 200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	Vector	rightShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40, 90)*-1 + gpGlobals->v_up * RANDOM_FLOAT(75, 200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass(vecShootOrigin - vecShootDir * 24, leftShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL);
	EjectBrass(vecShootOrigin - vecShootDir * 24, rightShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL);
	FireBullets(2, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, BULLET_MONSTER_MP5); // shoot +-5 degrees;
	
	const char* sound = RANDOM_LONG(0, 1) ? "weapons/uzi/fire_both1.wav" : "weapons/uzi/fire_both2.wav";
	EMIT_SOUND(ENT(pev), CHAN_STATIC, sound, 1, ATTN_NORM);
	PLAY_DISTANT_SOUND(edict(), DISTANT_9MM);
}

void CBaseGrunt::ShootMinigun(Vector& vecShootOrigin, Vector& vecShootDir) {
	UTIL_MakeVectors(pev->angles);

	int channel = CHAN_WEAPON;
	switch(RANDOM_LONG(0, 1)) {
	case 0:
		channel = CHAN_WEAPON;
		break;
	case 1:
		channel = CHAN_BODY;
		break;
	}

	const char* sound = RANDOM_LONG(0,1) == 0 ? "hassault/hw_shoot2.wav" : "hassault/hw_shoot3.wav";

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40, 90) + gpGlobals->v_up * RANDOM_FLOAT(75, 200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass(vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iBrassShell, TE_BOUNCE_SHELL);
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 2048, BULLET_PLAYER_556, 2);
	EMIT_SOUND(ENT(pev), channel, sound, 1, ATTN_NORM);
	PLAY_DISTANT_SOUND(edict(), DISTANT_9MM);
}

void CBaseGrunt::ShootSniper(Vector& vecShootOrigin, Vector& vecShootDir) {
	//TODO: why is this 556? is 762 too damaging?
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_1DEGREES, 8192, BULLET_MONSTER_762);
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sniper_fire.wav", 1, 0.3);
}

void CBaseGrunt ::ShootShotgun(Vector& vecShootOrigin, Vector& vecShootDir)
{
	UTIL_MakeVectors ( pev->angles );

	Vector	vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(40,90) + gpGlobals->v_up * RANDOM_FLOAT(75,200) + gpGlobals->v_forward * RANDOM_FLOAT(-40, 40);
	EjectBrass ( vecShootOrigin - vecShootDir * 24, vecShellVelocity, pev->angles.y, m_iShotgunShell, TE_BOUNCE_SHOTSHELL); 
	FireBullets(gSkillData.sk_hgrunt_pellets, vecShootOrigin, vecShootDir, VECTOR_CONE_15DEGREES, 2048, BULLET_PLAYER_BUCKSHOT, 0 ); // shoot +-7.5 degrees

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/sbarrel1.wav", 1, ATTN_NORM);
	PLAY_DISTANT_SOUND(edict(), DISTANT_556);
}

void CBaseGrunt::ShootSaw(Vector& vecShootOrigin, Vector& vecShootDir)
{
	UTIL_MakeVectors(pev->angles);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
	{
		auto vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(75, 200) + gpGlobals->v_up * RANDOM_FLOAT(150, 200) + gpGlobals->v_forward * 25.0;
		EjectBrass(vecShootOrigin - vecShootDir * 6, vecShellVelocity, pev->angles.y, m_iSawLink, TE_BOUNCE_SHELL);
		break;
	}

	case 1:
	{
		auto vecShellVelocity = gpGlobals->v_right * RANDOM_FLOAT(100, 250) + gpGlobals->v_up * RANDOM_FLOAT(100, 150) + gpGlobals->v_forward * 25.0;
		EjectBrass(vecShootOrigin - vecShootDir * 6, vecShellVelocity, pev->angles.y, m_iSawShell, TE_BOUNCE_SHELL);
		break;
	}
	}

	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_10DEGREES, 8192, BULLET_PLAYER_556, 2); // shoot +-5 degrees

	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/saw_fire1.wav", VOL_NORM, ATTN_NORM, 0, RANDOM_LONG(0, 15) + 94);
	PLAY_DISTANT_SOUND(edict(), DISTANT_556);
}

void CBaseGrunt::ShootGlock(Vector& vecShootOrigin, Vector& vecShootDir) {
	UTIL_MakeVectors(pev->angles);
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_MONSTER_9MM); // shoot +-5 degrees

	const auto random = RANDOM_LONG(0, 20);
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/pl_gun3.wav", VOL_NORM, ATTN_NORM, 0, (random <= 10 ? random - 5 : 0) + 100);
	PLAY_DISTANT_SOUND(edict(), DISTANT_9MM);
}

void CBaseGrunt::ShootDeagle(Vector& vecShootOrigin, Vector& vecShootDir) {
	UTIL_MakeVectors(pev->angles);
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_PLAYER_357); // shoot +-5 degrees

	const auto random = RANDOM_LONG(0, 20);
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, "weapons/desert_eagle_fire.wav", VOL_NORM, ATTN_NORM, 0, (random <= 10 ? random - 5 : 0) + 100);
	PLAY_DISTANT_SOUND(edict(), DISTANT_357);
}

void CBaseGrunt::Shoot357(Vector& vecShootOrigin, Vector& vecShootDir) {
	UTIL_MakeVectors(pev->angles);
	FireBullets(1, vecShootOrigin, vecShootDir, VECTOR_CONE_2DEGREES, 1024, BULLET_PLAYER_357); // shoot +-5 degrees

	const char* sound = RANDOM_LONG(0, 1) == 0 ? "weapons/357_shot1.wav" : "weapons/357_shot2.wav";
	const auto random = RANDOM_LONG(0, 20);
	EMIT_SOUND_DYN(edict(), CHAN_WEAPON, sound, VOL_NORM, ATTN_NORM, 0, (random <= 10 ? random - 5 : 0) + 100);
	PLAY_DISTANT_SOUND(edict(), DISTANT_357);
}

void CBaseGrunt::DropEquipmentToss(const char* cname, Vector vecGunPos, Vector vecGunAngles, Vector velocity, Vector aVelocity) {
	CBaseEntity* item = DropItem(cname, vecGunPos, vecGunAngles);

	if (item) {
		item->pev->velocity = velocity;
		item->pev->avelocity = aVelocity;
	}
}

bool CBaseGrunt::DropEquipment(int attachmentIdx, int equipMask, Vector velocity, Vector aVelocity) {
	Vector	vecGunPos;
	Vector	vecGunAngles;
	GetAttachment(attachmentIdx, vecGunPos, vecGunAngles);

	int equipmentToDrop = m_iEquipment & equipMask;
	bool droppedAnything = false;

	// now spawn a gun.
	if (equipmentToDrop & MEQUIP_MP5) {
		DropEquipmentToss("weapon_9mmAR", vecGunPos, vecGunAngles, velocity, aVelocity);
		droppedAnything = true;
	}
	if (equipmentToDrop & MEQUIP_SHOTGUN) {
		DropEquipmentToss("weapon_shotgun", vecGunPos, vecGunAngles, velocity, aVelocity);
		droppedAnything = true;
	}
	if (equipmentToDrop & MEQUIP_GRENADE_LAUNCHER) {
		DropEquipmentToss("ammo_ARgrenades", BodyTarget(pev->origin), vecGunAngles, velocity, aVelocity);
		droppedAnything = true;
	}
	if (equipmentToDrop & MEQUIP_SAW) {
		//DropEquipmentToss("weapon_m249", vecGunPos, vecGunAngles, velocity, aVelocity);
	}
	if (equipmentToDrop & MEQUIP_DEAGLE) {
		//DropEquipmentToss("weapon_eagle", vecGunPos, vecGunAngles, velocity, aVelocity);
		DropEquipmentToss("weapon_357", vecGunPos, vecGunAngles, velocity, aVelocity);
	}
	if (equipmentToDrop & MEQUIP_GLOCK) {
		DropEquipmentToss("weapon_9mmhandgun", vecGunPos, vecGunAngles, velocity, aVelocity);
		droppedAnything = true;
	}
	if (equipmentToDrop & MEQUIP_SNIPER) {
		//DropEquipmentToss("weapon_sniperrifle", vecGunPos, vecGunAngles, velocity, aVelocity);
	}
	if (equipmentToDrop & MEQUIP_MINIGUN) {
		//DropEquipmentToss("weapon_minigun", vecGunPos, vecGunAngles, velocity, aVelocity);
	}
	if (equipmentToDrop & MEQUIP_AKIMBO_UZIS) {
		DropEquipmentToss("weapon_uziakimbo", vecGunPos, vecGunAngles, velocity, aVelocity);
		droppedAnything = true;
	}

	m_iEquipment &= ~equipmentToDrop;

	return droppedAnything;
}

bool CBaseGrunt::DropEquipment(int attachmentIdx, bool randomToss) {
	Vector velocity = Vector(0,0,0);
	Vector aVelocity = Vector(0,0,0);

	if (randomToss) {
		velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
		aVelocity = Vector(0, RANDOM_FLOAT(200, 400), 0);
	}

	return DropEquipment(attachmentIdx, MEQUIP_EVERYTHING, velocity, aVelocity);
}

const char* CBaseGrunt::GetDeathNoticeWeapon() {
	if (HasEquipment(MEQUIP_MP5 | MEQUIP_SAW | MEQUIP_MINIGUN | MEQUIP_AKIMBO_UZIS)) {
		return "weapon_9mmAR";
	}
	if (HasEquipment(MEQUIP_SNIPER)) {
		return "weapon_crossbow";
	}
	if (HasEquipment(MEQUIP_SHOTGUN)) {
		return "weapon_shotgun";
	}
	if (HasEquipment(MEQUIP_DEAGLE)) {
		return "weapon_357";
	}
	if (HasEquipment(MEQUIP_GLOCK)) {
		return "weapon_9mmhandgun";
	}
	return "weapon_crowbar";
}

void CBaseGrunt::Reload() {
	if (HasEquipment(MEQUIP_SAW)) {
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/saw_reload.wav", 1, ATTN_NORM);
	}
	else if (HasEquipment(MEQUIP_DEAGLE)) {
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/desert_eagle_reload.wav", 1, ATTN_NORM);
	}
	else {
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "hgrunt/gr_reload1.wav", 1, ATTN_NORM);
	}

	m_cAmmoLoaded = m_cClipSize;
	ClearConditions(bits_COND_NO_AMMO_LOADED);
}

void CBaseGrunt::PointAtEnemy() {
	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	Vector angDir = UTIL_VecToAngles(vecShootDir);
	SetBlending(0, angDir.x);
}

void CBaseGrunt :: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
		case HGRUNT_AE_DROP_GUN:
			DropEquipment(0, false);
			break;

		case HGRUNT_AE_RELOAD:
			Reload();
			break;

		case HGRUNT_AE_GREN_TOSS:
		{
			UTIL_MakeVectors( pev->angles );
			// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
			CGrenade::ShootTimed( pev, GetGunPosition(), m_vecTossVelocity, 3.5 );

			m_fThrowGrenade = FALSE;
			m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
			// !!!LATER - when in a group, only try to throw grenade if ordered.
		}
		break;

		case HGRUNT_AE_GREN_LAUNCH:
		{
			EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/glauncher.wav", 0.8, ATTN_NORM);
			CGrenade::ShootContact( pev, GetGunPosition(), m_vecTossVelocity );
			m_fThrowGrenade = FALSE;
			if (gSkillData.sk_hgrunt_gwait == 0)
				m_flNextGrenadeCheck = gpGlobals->time + RANDOM_FLOAT( 2, 5 );// wait a random amount of time before shooting again
			else
				m_flNextGrenadeCheck = gpGlobals->time + 6;// wait six seconds before even looking again to see if a grenade can be thrown.
		}
		break;

		case HGRUNT_AE_GREN_DROP:
		{
			UTIL_MakeVectors( pev->angles );
			CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 17 - gpGlobals->v_right * 27 + gpGlobals->v_up * 6, g_vecZero, 3 );
		}
		break;

		case HGRUNT_AE_BURST1:
			Shoot(true);
			break;

		case HGRUNT_AE_BURST2:
		case HGRUNT_AE_BURST3:
			Shoot(false);
			break;

		case HGRUNT_AE_KICK:
		{
			CBaseEntity *pHurt = Kick();

			if ( pHurt )
			{
				// SOUND HERE!
				UTIL_MakeVectors( pev->angles );
				pHurt->pev->punchangle.x = 15;
				pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
				pHurt->TakeDamage( pev, pev, gSkillData.sk_hgrunt_kick, DMG_CLUB );
			}
		}
		break;

		case HGRUNT_AE_CAUGHT_ENEMY:
		{
			if ( FOkToSpeak() )
			{
				PlaySentenceSound(HGRUNT_SENT_ALERT);
				JustSpoke();
			}
			break;
		}

		default:
			CTalkSquadMonster::HandleAnimEvent( pEvent );
			break;
	}
}

void CBaseGrunt::BaseSpawn()
{
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->effects = 0;
	m_flFieldOfView = 0.2;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime = gpGlobals->time;
	m_iSentence = HGRUNT_SENT_NONE;

	m_afCapability = bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_fEnemyEluded = FALSE;
	m_fFirstEncounter = TRUE;// this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector(0, 0, 55);

	CTalkSquadMonster::g_talkWaitTime = 0;

	m_voicePitch = 100;

	m_iEquipment = 0;
	shellEjectAttachment = -1;

	Precache();

	InitModel();
	SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);

	MonsterInit();

	InitAiFlags();
}

void CBaseGrunt::InitAiFlags() {
	// default to HL Grunt AI
	canBeMadAtPlayer = false;
	waitForEnemyFire = false;
	runFromHeavyDamage = false;
	canCallMedic = false;
	suppressOccludedTarget = false;
	maxSuppressTime = 3.0f;
	maxShootDist = 2048;
}

void CBaseGrunt::BasePrecache() {
	CTalkSquadMonster::Precache();

	m_iBrassShell = PRECACHE_MODEL("models/shell.mdl");// brass shell
	m_iShotgunShell = PRECACHE_MODEL("models/shotgunshell.mdl");
	m_iSawShell = PRECACHE_MODEL("models/saw_shell.mdl");
	m_iSawLink = PRECACHE_MODEL("models/saw_link.mdl");

	PRECACHE_SOUND("hgrunt/gr_reload1.wav");
	PRECACHE_SOUND("zombie/claw_miss2.wav");// because we use the basemonster SWIPE animation event

	PrecacheEquipment(m_iEquipment);
}

void CBaseGrunt::PrecacheEquipment(int equipment) {
	if (equipment & MEQUIP_MP5) {
		PRECACHE_SOUND("hgrunt/gr_mgun1.wav");
		PRECACHE_SOUND("hgrunt/gr_mgun2.wav");

		AddPrecacheWeapon("weapon_9mmAR");
	}
	if (equipment & MEQUIP_GRENADE_LAUNCHER) {
		PRECACHE_SOUND("weapons/glauncher.wav");
		AddPrecacheWeapon("weapon_9mmAR");
	}
	if (equipment & MEQUIP_SHOTGUN) {
		PRECACHE_SOUND("weapons/sbarrel1.wav");

		AddPrecacheWeapon("weapon_shotgun");
	}
	if (equipment & MEQUIP_SAW) {
		PRECACHE_SOUND("weapons/saw_fire1.wav");
		PRECACHE_SOUND("weapons/saw_fire2.wav");
		PRECACHE_SOUND("weapons/saw_fire3.wav");
		PRECACHE_SOUND("weapons/saw_reload.wav");

		//AddPrecacheWeapon("weapon_9mmAR");
	}
	if (equipment & MEQUIP_357) {
		PRECACHE_SOUND("weapons/357_shot1.wav");
		PRECACHE_SOUND("weapons/357_shot2.wav");
		AddPrecacheWeapon("weapon_357");
	}
	if (equipment & MEQUIP_DEAGLE) {
		PRECACHE_SOUND("weapons/desert_eagle_fire.wav");
		PRECACHE_SOUND("weapons/desert_eagle_reload.wav");
		AddPrecacheWeapon("weapon_357");
	}
	if (equipment & MEQUIP_GLOCK) {
		AddPrecacheWeapon("weapon_9mmhandgun");
	}
	if (equipment & MEQUIP_SNIPER) {
		PRECACHE_SOUND("weapons/sniper_fire.wav");
		//AddPrecacheWeapon("weapon_9mmAR");
	}
	if (equipment & MEQUIP_MINIGUN) {
		//AddPrecacheWeapon("weapon_9mmAR");
		PRECACHE_SOUND("hassault/hw_shoot2.wav");
		PRECACHE_SOUND("hassault/hw_shoot3.wav");
		PRECACHE_SOUND("hassault/hw_spinup.wav");
		PRECACHE_SOUND("hassault/hw_spin.wav");
		PRECACHE_SOUND("hassault/hw_spindown.wav");
	}
	if (equipment & MEQUIP_AKIMBO_UZIS) {
		PRECACHE_SOUND("weapons/uzi/fire_both1.wav");
		PRECACHE_SOUND("weapons/uzi/fire_both2.wav");

		AddPrecacheWeapon("weapon_9mmAR");
	}
}

void CBaseGrunt :: StartTask ( Task_t *pTask )
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch ( pTask->iTask )
	{
	case TASK_GRUNT_CHECK_FIRE:
		if ( !NoFriendlyFire() )
		{
			SetConditions( bits_COND_GRUNT_NOFIRE );
		}
		TaskComplete();
		break;

	case TASK_GRUNT_SPEAK_SENTENCE:
		SpeakSentence();
		TaskComplete();
		break;
	
	case TASK_WALK_PATH:
	case TASK_RUN_PATH:
		// grunt no longer assumes he is covered if he moves
		Forget( bits_MEMORY_INCOVER );
		CTalkSquadMonster ::StartTask( pTask );
		break;

	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;

	case TASK_GRUNT_FACE_TOSS_DIR:
		break;

	case TASK_FACE_IDEAL:
	case TASK_FACE_ENEMY:
		CTalkSquadMonster :: StartTask( pTask );
		if (pev->movetype == MOVETYPE_FLY)
		{
			m_IdealActivity = ACT_GLIDE;
		}
		break;

	default: 
		CTalkSquadMonster :: StartTask( pTask );
		break;
	}
}

void CBaseGrunt :: RunTask ( Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_GRUNT_FACE_TOSS_DIR:
		{
			// project a point along the toss vector and turn to face that point.
			MakeIdealYaw( pev->origin + m_vecTossVelocity * 64 );
			ChangeYaw( pev->yaw_speed );

			if ( FacingIdeal() )
			{
				m_iTaskStatus = TASKSTATUS_COMPLETE;
			}
			break;
		}
	default:
		{
			CTalkSquadMonster :: RunTask( pTask );
			break;
		}
	}
}

const char* CBaseGrunt::GetTaskName(int taskIdx) {
	switch (taskIdx) {
	case TASK_GRUNT_FACE_TOSS_DIR: return "TASK_GRUNT_FACE_TOSS_DIR";
	case TASK_GRUNT_SPEAK_SENTENCE: return "TASK_GRUNT_SPEAK_SENTENCE";
	case TASK_GRUNT_CHECK_FIRE: return "TASK_GRUNT_CHECK_FIRE";
	default:
		return CTalkSquadMonster::GetTaskName(taskIdx);
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// GruntFail
//=========================================================
Task_t	tlGruntFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_WAIT,				(float)2		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slGruntFail[] =
{
	{
		tlGruntFail,
		ARRAYSIZE ( tlGruntFail ),
		bits_COND_CAN_RANGE_ATTACK1 |
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_CAN_MELEE_ATTACK1 |
		bits_COND_CAN_MELEE_ATTACK2,
		0,
		"GRUNT_FAIL"
	},
};

//=========================================================
// Grunt Combat Fail
//=========================================================
Task_t	tlGruntCombatFail[] =
{
	{ TASK_STOP_MOVING,			0				},
	{ TASK_SET_ACTIVITY,		(float)ACT_IDLE },
	{ TASK_REMEMBER,			(float)bits_MEMORY_MOVE_FAILED},
	{ TASK_WAIT_FACE_ENEMY,		(float)2.0f		},
	{ TASK_WAIT_PVS,			(float)0		},
};

Schedule_t	slGruntCombatFail[] =
{
	{
		tlGruntCombatFail,
		ARRAYSIZE ( tlGruntCombatFail ),
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2 |
		bits_COND_NEW_ENEMY |
		bits_COND_SEE_ENEMY |
		bits_COND_SEE_FEAR |
		bits_COND_LIGHT_DAMAGE |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_PROVOKED |
		bits_COND_HEAR_SOUND,

		bits_SOUND_COMBAT |// sound flags
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER,
		"GRUNT_COMBAT_FAIL"
	},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t	tlGruntVictoryDance[] =
{
	{ TASK_STOP_MOVING,						(float)0					},
	{ TASK_FACE_ENEMY,						(float)0					},
	{ TASK_WAIT,							(float)1.5					},
	{ TASK_GET_PATH_TO_ENEMY_CORPSE,		(float)0					},
	{ TASK_WALK_PATH,						(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0					},
	{ TASK_FACE_ENEMY,						(float)0					},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_VICTORY_DANCE	},
};

Schedule_t	slGruntVictoryDance[] =
{
	{ 
		tlGruntVictoryDance,
		ARRAYSIZE ( tlGruntVictoryDance ), 
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE,
		0,
		"GRUNT_VICTORY_DANCE"
	},
};

//=========================================================
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
Task_t tlGruntEstablishLineOfFire[] = 
{
	{ TASK_SET_FAIL_SCHEDULE,	(float)SCHED_GRUNT_ELOF_FAIL	},
	{ TASK_GET_PATH_TO_ENEMY,	(float)0						},
	{ TASK_GRUNT_SPEAK_SENTENCE,(float)0						},
	{ TASK_RUN_PATH,			(float)0						},
	{ TASK_WAIT_FOR_MOVEMENT,	(float)0						},
};

Schedule_t slGruntEstablishLineOfFire[] =
{
	{ 
		tlGruntEstablishLineOfFire,
		ARRAYSIZE ( tlGruntEstablishLineOfFire ),
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK2	|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"GRUNT_ESTABLISH_LINE_OF_FIRE"
	},
};

//=========================================================
// GruntFoundEnemy - grunt established sight with an enemy
// that was hiding from the squad.
//=========================================================
Task_t	tlGruntFoundEnemy[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_SIGNAL1			},
};

Schedule_t	slGruntFoundEnemy[] =
{
	{ 
		tlGruntFoundEnemy,
		ARRAYSIZE ( tlGruntFoundEnemy ), 
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"GRUNT_FOUND_ENEMY"
	},
};

//=========================================================
// GruntCombatFace Schedule
//=========================================================
Task_t	tlGruntCombatFace1[] =
{
	{ TASK_STOP_MOVING,				0							},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_WAIT,					(float)1.5					},
	{ TASK_SET_SCHEDULE,			(float)SCHED_GRUNT_SWEEP	},
};

Schedule_t	slGruntCombatFace[] =
{
	{ 
		tlGruntCombatFace1,
		ARRAYSIZE ( tlGruntCombatFace1 ), 
		bits_COND_NEW_ENEMY				|
		bits_COND_ENEMY_DEAD			|
		bits_COND_CAN_RANGE_ATTACK1		|
		bits_COND_CAN_RANGE_ATTACK2,
		0,
		"GRUNT_COMBAT_FACE"
	},
};

//=========================================================
// Suppressing fire - don't stop shooting until the clip is
// empty or grunt gets hurt.
//=========================================================
Task_t	tlGruntSignalSuppress[] =
{
	{ TASK_STOP_MOVING,					0						},
	{ TASK_FACE_IDEAL,					(float)0				},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,	(float)ACT_SIGNAL2		},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
	{ TASK_FACE_ENEMY,					(float)0				},
	{ TASK_GRUNT_CHECK_FIRE,			(float)0				},
	{ TASK_RANGE_ATTACK1,				(float)0				},
};

Schedule_t	slGruntSignalSuppress[] =
{
	{ 
		tlGruntSignalSuppress,
		ARRAYSIZE ( tlGruntSignalSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_GRUNT_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"GRUNT_SIGNAL_SUPPRESS"
	},
};

Task_t	tlGruntSuppress[] =
{
	{ TASK_STOP_MOVING,			0							},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
	{ TASK_FACE_ENEMY,			(float)0					},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0					},
	{ TASK_RANGE_ATTACK1,		(float)0					},
};

Schedule_t	slGruntSuppress[] =
{
	{ 
		tlGruntSuppress,
		ARRAYSIZE ( tlGruntSuppress ), 
		bits_COND_ENEMY_DEAD		|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND		|
		bits_COND_GRUNT_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"GRUNT_SUPPRESS"
	},
};


//=========================================================
// grunt wait in cover - we don't allow danger or the ability
// to attack to break a grunt's run to cover schedule, but
// when a grunt is in cover, we do want them to attack if they can.
//=========================================================
Task_t	tlGruntWaitInCover[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_ACTIVITY,			(float)ACT_IDLE				},
	{ TASK_WAIT_FACE_ENEMY,			(float)0.2f					},
};

Schedule_t	slGruntWaitInCover[] =
{
	{ 
		tlGruntWaitInCover,
		ARRAYSIZE ( tlGruntWaitInCover ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_HEAR_SOUND		|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_CAN_MELEE_ATTACK1	|
		bits_COND_CAN_MELEE_ATTACK2,

		bits_SOUND_DANGER,
		"GRUNT_WAIT_IN_COVER"
	},
};

//=========================================================
// run to cover.
// !!!BUGBUG - set a decent fail schedule here.
//=========================================================
Task_t	tlGruntTakeCover1[] =
{
	{ TASK_STOP_MOVING,				(float)0							},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_GRUNT_TAKECOVER_FAILED	},
	{ TASK_WAIT,					(float)0.2							},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0							},
	{ TASK_GRUNT_SPEAK_SENTENCE,	(float)0							},
	{ TASK_RUN_PATH,				(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0							},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER			},
	{ TASK_SET_SCHEDULE,			(float)SCHED_GRUNT_WAIT_FACE_ENEMY	},
};

Schedule_t	slGruntTakeCover[] =
{
	{ 
		tlGruntTakeCover1,
		ARRAYSIZE ( tlGruntTakeCover1 ), 
		0,
		0,
		"GRUNT_TAKE_COVER"
	},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlGruntGrenadeCover1[] =
{
	{ TASK_STOP_MOVING,						(float)0							},
	{ TASK_FIND_COVER_FROM_ENEMY,			(float)99							},
	{ TASK_FIND_FAR_NODE_COVER_FROM_ENEMY,	(float)384							},
	{ TASK_PLAY_SEQUENCE,					(float)ACT_SPECIAL_ATTACK1			},
	{ TASK_CLEAR_MOVE_WAIT,					(float)0							},
	{ TASK_RUN_PATH,						(float)0							},
	{ TASK_WAIT_FOR_MOVEMENT,				(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_GRUNT_WAIT_FACE_ENEMY	},
};

Schedule_t	slGruntGrenadeCover[] =
{
	{ 
		tlGruntGrenadeCover1,
		ARRAYSIZE ( tlGruntGrenadeCover1 ), 
		0,
		0,
		"GRUNT_GRENADE_COVER"
	},
};


//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t	tlGruntTossGrenadeCover1[] =
{
	{ TASK_FACE_ENEMY,						(float)0							},
	{ TASK_RANGE_ATTACK2, 					(float)0							},
	{ TASK_SET_SCHEDULE,					(float)SCHED_TAKE_COVER_FROM_ENEMY	},
};

Schedule_t	slGruntTossGrenadeCover[] =
{
	{ 
		tlGruntTossGrenadeCover1,
		ARRAYSIZE ( tlGruntTossGrenadeCover1 ), 
		0,
		0,
		"GRUNT_TOSS_GRENADE_COVER"
	},
};

//=========================================================
// hide from the loudest sound source (to run from grenade)
//=========================================================
Task_t	tlGruntTakeCoverFromBestSound[] =
{
	{ TASK_SET_FAIL_SCHEDULE,			(float)SCHED_COWER			},// duck and cover if cannot move from explosion
	{ TASK_STOP_MOVING,					(float)0					},
	{ TASK_FIND_COVER_FROM_BEST_SOUND,	(float)0					},
	{ TASK_RUN_PATH,					(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,			(float)0					},
	{ TASK_REMEMBER,					(float)bits_MEMORY_INCOVER	},
	{ TASK_TURN_LEFT,					(float)179					},
};

Schedule_t	slGruntTakeCoverFromBestSound[] =
{
	{ 
		tlGruntTakeCoverFromBestSound,
		ARRAYSIZE ( tlGruntTakeCoverFromBestSound ), 
		0,
		0,
		"GRUNT_TAKE_COVER_FROM_BEST_SOUND"
	},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t	tlGruntHideReload[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_SET_FAIL_SCHEDULE,		(float)SCHED_RELOAD			},
	{ TASK_FIND_COVER_FROM_ENEMY,	(float)0					},
	{ TASK_RUN_PATH,				(float)0					},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0					},
	{ TASK_REMEMBER,				(float)bits_MEMORY_INCOVER	},
	{ TASK_FACE_ENEMY,				(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RELOAD			},
};

Schedule_t slGruntHideReload[] = 
{
	{
		tlGruntHideReload,
		ARRAYSIZE ( tlGruntHideReload ),
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"GRUNT_HIDE_RELOAD"
	}
};

//=========================================================
// Do a turning sweep of the area
//=========================================================
Task_t	tlGruntSweep[] =
{
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)1	},
	{ TASK_TURN_LEFT,			(float)179	},
	{ TASK_WAIT,				(float)1	},
};

Schedule_t	slGruntSweep[] =
{
	{ 
		tlGruntSweep,
		ARRAYSIZE ( tlGruntSweep ), 
		
		bits_COND_NEW_ENEMY		|
		bits_COND_LIGHT_DAMAGE	|
		bits_COND_HEAVY_DAMAGE	|
		bits_COND_CAN_RANGE_ATTACK1	|
		bits_COND_CAN_RANGE_ATTACK2	|
		bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD		|// sound flags
		bits_SOUND_DANGER		|
		bits_SOUND_PLAYER,

		"GRUNT_SWEEP"
	},
};

//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlGruntRangeAttack1A[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,		(float)ACT_CROUCH },
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slGruntRangeAttack1A[] =
{
	{ 
		tlGruntRangeAttack1A,
		ARRAYSIZE ( tlGruntRangeAttack1A ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_HEAR_SOUND		|
		bits_COND_GRUNT_NOFIRE		|
		bits_COND_NO_AMMO_LOADED,
		
		bits_SOUND_DANGER,
		"GRUNT_RANGE_ATTACK_1A"
	},
};


// Range attack with an "angry idle" animation instead of crouching
Task_t	tlGruntRangeAttack1B[] =
{
	{ TASK_STOP_MOVING,				(float)0		},
	{ TASK_PLAY_SEQUENCE_FACE_ENEMY,(float)ACT_IDLE_ANGRY  },
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slGruntRangeAttack1B[] =
{
	{ 
		tlGruntRangeAttack1B,
		ARRAYSIZE ( tlGruntRangeAttack1B ), 
		bits_COND_NEW_ENEMY			|
		bits_COND_ENEMY_DEAD		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_ENEMY_OCCLUDED	|
		bits_COND_NO_AMMO_LOADED	|
		bits_COND_GRUNT_NOFIRE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER,
		"GRUNT_RANGE_ATTACK_1B"
	},
};

// Range attack with no crouching/idle animations (for bodyguard/hwgrunt)
Task_t	tlGruntRangeAttack1C[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_RANGE_ATTACK1,		(float)0		},
};

Schedule_t	slGruntRangeAttack1C[] =
{
	{
		tlGruntRangeAttack1C,
		ARRAYSIZE(tlGruntRangeAttack1C),
		bits_COND_NEW_ENEMY |
		bits_COND_ENEMY_DEAD |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_HEAR_SOUND |
		bits_COND_GRUNT_NOFIRE |
		bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"GRUNT_RANGE_ATTACK_1C"
	},
};

//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t	tlGruntRangeAttack2[] =
{
	{ TASK_STOP_MOVING,				(float)0					},
	{ TASK_GRUNT_FACE_TOSS_DIR,		(float)0					},
	{ TASK_PLAY_SEQUENCE,			(float)ACT_RANGE_ATTACK2	},
	{ TASK_SET_SCHEDULE,			(float)SCHED_GRUNT_WAIT_FACE_ENEMY	},// don't run immediately after throwing grenade.
};

Schedule_t	slGruntRangeAttack2[] =
{
	{ 
		tlGruntRangeAttack2,
		ARRAYSIZE ( tlGruntRangeAttack2 ), 
		0,
		0,
		"GRUNT_RANGE_ATTACK2"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlGruntRepel[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_IDEAL,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_GLIDE 	},
};

Schedule_t	slGruntRepel[] =
{
	{ 
		tlGruntRepel,
		ARRAYSIZE ( tlGruntRepel ), 
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER			|
		bits_SOUND_COMBAT			|
		bits_SOUND_PLAYER, 
		"GRUNT_REPEL"
	},
};


//=========================================================
// repel 
//=========================================================
Task_t	tlGruntRepelAttack[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_FLY 	},
};

Schedule_t	slGruntRepelAttack[] =
{
	{ 
		tlGruntRepelAttack,
		ARRAYSIZE ( tlGruntRepelAttack ), 
		bits_COND_ENEMY_OCCLUDED,
		0,
		"GRUNT_REPEL_ATTACK"
	},
};

//=========================================================
// repel land
//=========================================================
Task_t	tlGruntRepelLand[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_LAND	},
	{ TASK_GET_PATH_TO_LASTPOSITION,(float)0				},
	{ TASK_RUN_PATH,				(float)0				},
	{ TASK_WAIT_FOR_MOVEMENT,		(float)0				},
	{ TASK_CLEAR_LASTPOSITION,		(float)0				},
};

Schedule_t	slGruntRepelLand[] =
{
	{ 
		tlGruntRepelLand,
		ARRAYSIZE ( tlGruntRepelLand ), 
		bits_COND_SEE_ENEMY			|
		bits_COND_NEW_ENEMY			|
		bits_COND_LIGHT_DAMAGE		|
		bits_COND_HEAVY_DAMAGE		|
		bits_COND_HEAR_SOUND,
		
		bits_SOUND_DANGER			|
		bits_SOUND_COMBAT			|
		bits_SOUND_PLAYER, 
		"GRUNT_REPEL_LAND"
	},
};

Task_t	tlMinigunSpinup[] =
{
	{ TASK_STOP_MOVING,			(float)0		},
	{ TASK_FACE_ENEMY,			(float)0		},
	{ TASK_GRUNT_CHECK_FIRE,	(float)0		},
	{ TASK_PLAY_SEQUENCE,		(float)ACT_THREAT_DISPLAY	},
};

Schedule_t	slMinigunSpinup[] =
{
	{
		tlMinigunSpinup,
		ARRAYSIZE(tlMinigunSpinup),
		bits_COND_ENEMY_DEAD |
		bits_COND_HEAVY_DAMAGE |
		bits_COND_ENEMY_OCCLUDED |
		bits_COND_GRUNT_NOFIRE,

		0,
		"GRUNT_MINIGUN_SPINUP"
	},
};

Task_t	tlMinigunSpindown[] =
{
	{ TASK_PLAY_SEQUENCE,		(float)ACT_DISARM	},
};

Schedule_t	slMinigunSpindown[] =
{
	{
		tlMinigunSpindown,
		ARRAYSIZE(tlMinigunSpindown),
		0,

		0,
		"GRUNT_MINIGUN_SPINDOWN"
	},
};

DEFINE_CUSTOM_SCHEDULES( CBaseGrunt )
{
	slGruntFail,
	slGruntCombatFail,
	slGruntVictoryDance,
	slGruntEstablishLineOfFire,
	slGruntFoundEnemy,
	slGruntCombatFace,
	slGruntSignalSuppress,
	slGruntSuppress,
	slGruntWaitInCover,
	slGruntTakeCover,
	slGruntGrenadeCover,
	slGruntTossGrenadeCover,
	slGruntTakeCoverFromBestSound,
	slGruntHideReload,
	slGruntSweep,
	slGruntRangeAttack1A,
	slGruntRangeAttack1B,
	slGruntRangeAttack1C,
	slGruntRangeAttack2,
	slGruntRepel,
	slGruntRepelAttack,
	slGruntRepelLand,
	slMinigunSpinup,
	slMinigunSpindown
};

IMPLEMENT_CUSTOM_SCHEDULES( CBaseGrunt, CTalkSquadMonster )

void CBaseGrunt :: SetActivity ( Activity NewActivity )
{
	int	iSequence = ACTIVITY_NOT_AVAILABLE;

	iSequence = GetActivitySequence(NewActivity);
	
	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// Set to the desired anim, or default anim if the desired is not present
	if ( iSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if ( pev->sequence != iSequence || !m_fSequenceLoops )
		{
			pev->frame = 0;
		}

		pev->sequence		= iSequence;	// Set to the reset anim (if it's there)
		ResetSequenceInfo( );
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		const char* actName = NewActivity < ACT_LAST ? activity_map[NewActivity].name : "Unknown";
		ALERT(at_aiconsole, "%s has no sequence for act %s (%d)\n", STRING(pev->classname), actName, NewActivity);
		pev->sequence		= 0;	// Set to the reset anim (if it's there)
	}
}

int CBaseGrunt::GetActivitySequence(Activity NewActivity) {
	int iSequence = ACTIVITY_NOT_AVAILABLE;

	switch (NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
		if (HasEquipment(MEQUIP_MP5))
		{
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
		}
		else
		{
			if ( m_fStanding )
			{
				// get aimable sequence
				iSequence = LookupSequence( "standing_shotgun" );
			}
			else
			{
				// get crouching shoot
				iSequence = LookupSequence( "crouching_shotgun" );
			}
		}
		break;
	case ACT_RANGE_ATTACK2:
		// grunt is going to a secondary long range attack. This may be a thrown 
		// grenade or fired grenade, we must determine which and pick proper sequence
		if (HasEquipment(MEQUIP_HAND_GRENADE))
		{
			// get toss anim
			iSequence = LookupSequence( "throwgrenade" );
		}
		else
		{
			// get launch anim
			iSequence = LookupSequence( "launchgrenade" );
		}
		break;
	case ACT_RUN:
		if ( pev->health <= HGRUNT_LIMP_HEALTH )
		{
			// limp!
			iSequence = LookupActivity ( ACT_RUN_HURT );
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}
		break;
	case ACT_WALK:
		if ( pev->health <= HGRUNT_LIMP_HEALTH )
		{
			// limp!
			iSequence = LookupActivity ( ACT_WALK_HURT );
		}
		else
		{
			iSequence = LookupActivity ( NewActivity );
		}
		break;
	case ACT_IDLE:
		if ( m_MonsterState == MONSTERSTATE_COMBAT )
		{
			NewActivity = ACT_IDLE_ANGRY;
		}
		iSequence = LookupActivity ( NewActivity );
		break;
	default:
		iSequence = LookupActivity ( NewActivity );
		break;
	}

	return iSequence;
}

Schedule_t* CBaseGrunt::GetNewSquadEnemySchedule(void) {
	MySquadLeader()->m_fEnemyEluded = FALSE;

	if (!IsLeader())
	{
		return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
	}
	else
	{
		//!!!KELLY - the leader of a squad of grunts has just seen the player or a 
		// monster and has made it the squad's enemy. You
		// can check pev->flags for FL_CLIENT to determine whether this is the player
		// or a monster. He's going to immediately start
		// firing, though. If you'd like, we can make an alternate "first sight" 
		// schedule where the leader plays a handsign anim
		// that gives us enough time to hear a short sentence or spoken command
		// before he starts pluggin away.
		if (FOkToSpeak())// && RANDOM_LONG(0,1))
		{
			if ((m_hEnemy != NULL) && m_hEnemy->IsPlayer())
				// player
				PlaySentenceSound(HGRUNT_SENT_ALERT);
			else if ((m_hEnemy != NULL) &&
				(m_hEnemy->Classify() != CLASS_PLAYER_ALLY) &&
				(m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE) &&
				(m_hEnemy->Classify() != CLASS_MACHINE))
				// monster
				PlaySentenceSound(HGRUNT_SENT_MONSTER);

			JustSpoke();
		}

		if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_GRUNT_SUPPRESS);
		}
		else
		{
			return GetScheduleOfType(SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE);
		}
	}
}

Schedule_t* CBaseGrunt::GetShootSchedule(void) {
	if (InSquad())
	{
		// if the enemy has eluded the squad and a squad member has just located the enemy
		// and the enemy does not see the squad member, issue a call to the squad to waste a 
		// little time and give the player a chance to turn.
		if (MySquadLeader()->m_fEnemyEluded && !HasConditions(bits_COND_ENEMY_FACING_ME))
		{
			MySquadLeader()->m_fEnemyEluded = FALSE;
			return GetScheduleOfType(SCHED_GRUNT_FOUND_ENEMY);
		}
	}

	if (OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
	{
		// try to take an available ENGAGE slot
		return GetScheduleOfType(SCHED_RANGE_ATTACK1);
	}
	else if (HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
	{
		// throw a grenade if can and no engage slots are available
		return GetScheduleOfType(SCHED_RANGE_ATTACK2);
	}
	else
	{
		
		if (HasMemory(bits_MEMORY_MOVE_FAILED)) {
			// tried and failed to take cover, just shoot
			return GetScheduleOfType(SCHED_RANGE_ATTACK1);
		}
		else {
			// hide!
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
		}
		
	}
}

Schedule_t* CBaseGrunt::GetLightDamageSchedule(void) {
	// if hurt:
	// 90% chance of taking cover
	// 10% chance of flinch.
	int iPercent = RANDOM_LONG(0, 99);

	if (iPercent <= 90)
	{
		if (m_hEnemy != NULL) {
			// only try to take cover if we actually have an enemy!

			//!!!KELLY - this grunt was hit and is going to run to cover.
			if (FOkToSpeak()) // && RANDOM_LONG(0,1))
			{
				//SENTENCEG_PlayRndSz( ENT(pev), "HG_COVER", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
				m_iSentence = HGRUNT_SENT_COVER;
				//JustSpoke();
			}
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
		}
		else {
			return GetScheduleOfType(SCHED_COMBAT_FACE);
		}
	}
	else
	{
		return GetScheduleOfType(SCHED_SMALL_FLINCH);
	}
}

Schedule_t* CBaseGrunt::GetEnemyOccludedSchedule(void) {
	if (suppressOccludedTarget && gpGlobals->time - m_flLastEnemySightTime < maxSuppressTime) {
		return GetScheduleOfType(SCHED_RANGE_ATTACK1);
	}

	if (HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
	{
		//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
		if (FOkToSpeak())
		{
			PlaySentenceSound(HGRUNT_SENT_THROW);
			JustSpoke();
		}
		return GetScheduleOfType(SCHED_RANGE_ATTACK2);
	}
	else if (OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
	{
		//!!!KELLY - grunt cannot see the enemy and has just decided to 
		// charge the enemy's position. 
		if (FOkToSpeak())// && RANDOM_LONG(0,1))
		{
			//SENTENCEG_PlayRndSz( ENT(pev), "HG_CHARGE", HGRUNT_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			m_iSentence = HGRUNT_SENT_CHARGE;
			//JustSpoke();
		}

		return GetScheduleOfType(SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE);
	}
	else
	{
		//!!!KELLY - grunt is going to stay put for a couple seconds to see if
		// the enemy wanders back out into the open, or approaches the
		// grunt's covered position. Good place for a taunt, I guess?
		if (FOkToSpeak() && RANDOM_LONG(0, 1))
		{
			PlaySentenceSound(HGRUNT_SENT_TAUNT);
			JustSpoke();
		}
		return GetScheduleOfType(SCHED_STANDOFF);
	}
}

Schedule_t* CBaseGrunt::GetMonsterStateSchedule(void) {
	if (HasEquipment(MEQUIP_MINIGUN)) {
		// flinch and run away less while holding a minigun

		if (HasConditions(bits_COND_HEAVY_DAMAGE))
		{
			// flinch for heavy damage but not too often
			if (RANDOM_LONG(0, 2) == 0) {
				return GetScheduleOfType(SCHED_SMALL_FLINCH);
			}
			else {
				ClearConditions(bits_COND_HEAVY_DAMAGE);
				return CTalkSquadMonster::GetSchedule();
			}
		}
		else if (HasConditions(bits_COND_LIGHT_DAMAGE))
		{
			ClearConditions(bits_COND_LIGHT_DAMAGE);
			// never flinch or retreat from light damage
			return CTalkSquadMonster::GetSchedule();
		}
	}

	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
	{
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			if (FOkToSpeak())
			{
				// TODO: not all monsters have this sentence
				PlaySentenceSound(HGRUNT_SENT_KILL);
			}

			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		if (m_hWaitMedic)
		{
			CTalkSquadMonster* pMedic = m_hWaitMedic->MyTalkSquadMonsterPointer();

			if (pMedic->pev->deadflag != DEAD_NO)
				m_hWaitMedic = NULL;
			else
				pMedic->HealMe(NULL);

			m_flMedicWaitTime = gpGlobals->time + 5.0;
		}

		// new enemy
		//Do not fire until fired upon
		if (waitForEnemyFire) {
			if (HasAllConditions(bits_COND_NEW_ENEMY | bits_COND_LIGHT_DAMAGE))
			{
				if (InSquad()) {
					return GetNewSquadEnemySchedule();
				}

				return GetScheduleOfType(SCHED_SMALL_FLINCH);
			}
		} else {
			if (HasAllConditions(bits_COND_NEW_ENEMY))
			{
				return GetNewSquadEnemySchedule();
			}
		}

		if (runFromHeavyDamage && HasConditions(bits_COND_HEAVY_DAMAGE))
			return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);

		// no ammo
		else if (HasConditions(bits_COND_NO_AMMO_LOADED))
		{
			//!!!KELLY - this individual just realized he's out of bullet ammo. 
			// He's going to try to find cover to run to and reload, but rarely, if 
			// none is available, he'll drop and reload in the open here. 
			return GetScheduleOfType(SCHED_GRUNT_COVER_AND_RELOAD);
		}

		// damaged just a little (melee grunts don't care)
		else if (HasConditions(bits_COND_LIGHT_DAMAGE) && (HasEquipment(ANY_RANGED_WEAPON) || !m_hEnemy))
		{
			return GetLightDamageSchedule();
		}
		// can kick
		else if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}
		// can grenade launch

		else if (HasEquipment(MEQUIP_GRENADE_LAUNCHER) && HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
		{
			// shoot a grenade if you can
			return GetScheduleOfType(SCHED_RANGE_ATTACK2);
		}
		// can shoot
		else if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
		{
			return GetShootSchedule();
		}
		// can't see enemy
		else if (HasConditions(bits_COND_ENEMY_OCCLUDED))
		{
			return GetEnemyOccludedSchedule();
		}

		//Only if not following a player
		if (!m_hTargetEnt || !m_hTargetEnt->IsPlayer()) // TODO: does this added condition change HL grunt behavior?
		{
			if (HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_CAN_RANGE_ATTACK1))
			{
				return GetScheduleOfType(SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE);
			}
		}
		break;
	}

	case MONSTERSTATE_ALERT:
	case MONSTERSTATE_IDLE:
	{
		// TODO: do these custons schedules work ok for vanilla HL grunts? these are all op4 specific

		if (HasConditions(bits_COND_LIGHT_DAMAGE | bits_COND_HEAVY_DAMAGE))
		{
			// flinch if hurt
			return GetScheduleOfType(SCHED_SMALL_FLINCH);
		}

		//if we're not waiting on a medic and we're hurt, call out for a medic
		if (canCallMedic && !m_hWaitMedic && gpGlobals->time > m_flMedicWaitTime && pev->health <= 20.0)
		{
			auto pMedic = MySquadMedic();

			if (!pMedic)
			{
				pMedic = FindSquadMedic(1024);
			}

			if (pMedic)
			{
				if (pMedic->pev->deadflag == DEAD_NO)
				{
					ALERT(at_aiconsole, "Injured Grunt found Medic\n");

					if (pMedic->HealMe(this))
					{
						ALERT(at_aiconsole, "Injured Grunt called for Medic\n");

						EMIT_SOUND_DYN(edict(), CHAN_VOICE, "fgrunt/medic.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

						JustSpoke();
						m_flMedicWaitTime = gpGlobals->time + 5.0;
					}
				}
			}
		}

		if (m_hEnemy == NULL && IsFollowing())
		{
			if (!m_hTargetEnt->IsAlive())
			{
				// UNDONE: Comment about the recently dead player here?
				StopFollowing(FALSE);
				break;
			}
			else
			{
				if (HasConditions(bits_COND_CLIENT_PUSH))
				{
					return GetScheduleOfType(SCHED_MOVE_AWAY_FOLLOW);
				}
				return GetScheduleOfType(SCHED_TARGET_FACE);
			}
		}

		if (HasConditions(bits_COND_CLIENT_PUSH))
		{
			return GetScheduleOfType(SCHED_MOVE_AWAY);
		}

		// try to say something about smells
		TrySmellTalk();
		break;
	}
	default:
		break;
	}

	return CTalkSquadMonster::GetSchedule();
}

Schedule_t *CBaseGrunt :: GetSchedule( void )
{
	// clear old sentence
	m_iSentence = HGRUNT_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling. 
	if (pev->movetype == MOVETYPE_FLY && m_MonsterState != MONSTERSTATE_PRONE)
	{
		if (pev->flags & FL_ONGROUND)
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			return GetScheduleOfType(SCHED_GRUNT_REPEL_LAND);
		}
		else
		{
			// repel down a rope, 
			if (m_MonsterState == MONSTERSTATE_COMBAT)
				return GetScheduleOfType(SCHED_GRUNT_REPEL_ATTACK);
			else
				return GetScheduleOfType(SCHED_GRUNT_REPEL);
		}
	}

	// grunts place HIGH priority on running away from danger sounds.
	if (HasConditions(bits_COND_HEAR_SOUND))
	{
		CSound* pSound;
		pSound = PBestSound();

		ASSERT(pSound != NULL);
		if (pSound)
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
					PlaySentenceSound(HGRUNT_SENT_GREN);
					JustSpoke();
				}
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_BEST_SOUND);
			}
			/*
			if (!HasConditions( bits_COND_SEE_ENEMY ) && ( pSound->m_iType & (bits_SOUND_PLAYER | bits_SOUND_COMBAT) ))
			{
				MakeIdealYaw( pSound->m_vecOrigin );
			}
			*/
		}
	}
	
	return GetMonsterStateSchedule();
}

Schedule_t* CBaseGrunt :: GetScheduleOfType ( int Type ) 
{
	switch	( Type )
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
		{
			if ( InSquad() )
			{
				if ( gSkillData.sk_hgrunt_gcover && HasConditions( bits_COND_CAN_RANGE_ATTACK2 ) && HasEquipment(MEQUIP_HAND_GRENADE) && OccupySlot( bits_SLOTS_HGRUNT_GRENADE ) )
				{
					if (FOkToSpeak())
					{
						PlaySentenceSound(HGRUNT_SENT_THROW);
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
				if ( HasEquipment(MEQUIP_HAND_GRENADE) && RANDOM_LONG(0,1) )
				{
					return &slGruntGrenadeCover[0];
				}
				else
				{
					return &slGruntTakeCover[0];
				}
			}
		}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
		{
			return &slGruntTakeCoverFromBestSound[ 0 ];
		}
	case SCHED_GRUNT_TAKECOVER_FAILED:
		{
			if ( HasConditions( bits_COND_CAN_RANGE_ATTACK1 ) && OccupySlot( bits_SLOTS_HGRUNT_ENGAGE ) )
			{
				return GetScheduleOfType( SCHED_RANGE_ATTACK1 );
			}

			return GetScheduleOfType ( SCHED_FAIL );
		}
		break;
	case SCHED_GRUNT_ELOF_FAIL:
		{
			// human grunt is unable to move to a position that allows him to attack the enemy.
			return GetScheduleOfType ( SCHED_TAKE_COVER_FROM_ENEMY );
		}
		break;
	case SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE:
		{
			return &slGruntEstablishLineOfFire[ 0 ];
		}
		break;
	case SCHED_RANGE_ATTACK1:
		{
			// randomly stand or crouch
			if (RANDOM_LONG(0,9) == 0)
				m_fStanding = RANDOM_LONG(0,1);
		 
			if (m_fStanding)
				return &slGruntRangeAttack1B[ 0 ];
			else
				return &slGruntRangeAttack1A[ 0 ];
		}
	case SCHED_RANGE_ATTACK2:
		{
			return &slGruntRangeAttack2[ 0 ];
		}
	case SCHED_COMBAT_FACE:
		{
			return &slGruntCombatFace[ 0 ];
		}
	case SCHED_GRUNT_WAIT_FACE_ENEMY:
		{
			return &slGruntWaitInCover[ 0 ];
		}
	case SCHED_GRUNT_SWEEP:
		{
			return &slGruntSweep[ 0 ];
		}
	case SCHED_GRUNT_COVER_AND_RELOAD:
		{
			return &slGruntHideReload[ 0 ];
		}
	case SCHED_GRUNT_FOUND_ENEMY:
		{
			return &slGruntFoundEnemy[ 0 ];
		}
	case SCHED_VICTORY_DANCE:
		{
			if ( InSquad() )
			{
				if ( !IsLeader() )
				{
					return &slGruntFail[ 0 ];
				}
			}

			return &slGruntVictoryDance[ 0 ];
		}
	case SCHED_GRUNT_SUPPRESS:
		{
			if ( m_hEnemy->IsPlayer() && m_fFirstEncounter )
			{
				m_fFirstEncounter = FALSE;// after first encounter, leader won't issue handsigns anymore when he has a new enemy
				return &slGruntSignalSuppress[ 0 ];
			}
			else
			{
				return &slGruntSuppress[ 0 ];
			}
		}
	case SCHED_FAIL:
		{
			if ( m_hEnemy != NULL )
			{
				// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
				return &slGruntCombatFail[ 0 ];
			}

			return &slGruntFail[ 0 ];
		}
	case SCHED_GRUNT_REPEL:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slGruntRepel[ 0 ];
		}
	case SCHED_GRUNT_REPEL_ATTACK:
		{
			if (pev->velocity.z > -128)
				pev->velocity.z -= 32;
			return &slGruntRepelAttack[ 0 ];
		}
	case SCHED_GRUNT_REPEL_LAND:
		{
			return &slGruntRepelLand[ 0 ];
		}
	case SCHED_TARGET_CHASE:
		return slFollow;

	case SCHED_TARGET_FACE:
	{
		auto pSchedule = CTalkSquadMonster::GetScheduleOfType(SCHED_TARGET_FACE);

		if (pSchedule == slIdleStand)
			return slFaceTarget;
		return pSchedule;
	}

	case SCHED_IDLE_STAND:
	{
		auto pSchedule = CTalkSquadMonster::GetScheduleOfType(SCHED_IDLE_STAND);

		if (pSchedule == slIdleStand)
			return slIdleStand;
		return pSchedule;
	}
	default:
		{
			return CTalkSquadMonster :: GetScheduleOfType ( Type );
		}
	}
}

void CBaseGrunt::Revive() {
	m_iEquipment = m_deadEquipment;

	CBaseMonster::Revive();
}


void CBaseRepel::Spawn(void) {
	Precache();
	pev->solid = SOLID_NOT;
	SetUse(&CBaseRepel::RepelUse);
}

void CBaseRepel::Precache(void) {
	CBaseMonster::Precache();

	std::unordered_map<std::string, std::string> keys;
	if (m_soundReplacementPath)
		keys["soundlist"] = STRING(m_soundReplacementPath);
	if (m_IsPlayerAlly)
		keys["is_player_ally"] = "1";
	if (pev->weapons)
		keys["weapons"] = UTIL_VarArgs("%d", pev->weapons);

	UTIL_PrecacheOther(GetMonsterType(), keys);
	m_iSpriteTexture = PRECACHE_MODEL("sprites/rope.spr");
}

void CBaseRepel::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "head"))
	{
		m_iGruntHead = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "UseSentence"))
	{
		m_iszUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "UnUseSentence"))
	{
		m_iszUnUse = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

void CBaseRepel::RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -4096.0), dont_ignore_monsters, ENT(pev), &tr);
	/*
	if ( tr.pHit && Instance( tr.pHit )->pev->solid != SOLID_BSP)
		return NULL;
	*/

	CBaseEntity* pEntity = Create(GetMonsterType(), pev->origin, pev->angles);
	CBaseMonster* pGrunt = pEntity->MyMonsterPointer();
	CBaseGrunt* baseGrunt = static_cast<CBaseGrunt*>(pEntity->MyTalkSquadMonsterPointer());

	// copy settings from parent
	pGrunt->pev->rendermode = pev->rendermode;
	pGrunt->pev->renderamt = pev->renderamt;
	pGrunt->pev->renderfx = pev->renderfx;
	pGrunt->pev->rendercolor = pev->rendercolor;
	pGrunt->m_iszTriggerTarget = m_iszTriggerTarget;
	pGrunt->m_iTriggerCondition = m_iTriggerCondition;
	pGrunt->m_displayName = m_displayName;
	pGrunt->m_Classify = m_Classify;
	pGrunt->m_IsPlayerAlly = m_IsPlayerAlly;
	pGrunt->pev->owner = pev->owner; // required for repel grunts spawned by monstermaker
	pGrunt->pev->model = pev->model;

	if (baseGrunt)
	{
		baseGrunt->pev->weapons = pev->weapons;
		baseGrunt->pev->netname = pev->netname;

		//Carry over these spawn flags
		baseGrunt->pev->spawnflags |= pev->spawnflags
			& (SF_MONSTER_WAIT_TILL_SEEN
				| SF_MONSTER_GAG
				| SF_MONSTER_HITMONSTERCLIP
				| SF_MONSTER_PRISONER
				| SF_SQUADMONSTER_LEADER
				| SF_MONSTER_PREDISASTER);

		baseGrunt->m_iGruntHead = m_iGruntHead;
		baseGrunt->m_iszUse = m_iszUse;
		baseGrunt->m_iszUnUse = m_iszUnUse;

		//Run logic to set up body groups (Spawn was already called once by Create above)
		baseGrunt->Spawn();
	}

	pGrunt->pev->movetype = MOVETYPE_FLY;
	pGrunt->pev->velocity = Vector(0, 0, RANDOM_FLOAT(-196, -128));
	pGrunt->SetActivity(ACT_GLIDE);
	// UNDONE: position?
	pGrunt->m_vecLastPosition = tr.vecEndPos;

	CBeam* pBeam = CBeam::BeamCreate("sprites/rope.spr", 10);
	pBeam->PointEntInit(pev->origin + Vector(0, 0, 112), pGrunt->entindex());
	pBeam->SetFlags(BEAM_FSOLID);
	pBeam->SetColor(255, 255, 255);
	pBeam->SetThink(&CBeam::SUB_Remove);
	pBeam->pev->nextthink = gpGlobals->time + -4096.0 * tr.flFraction / pGrunt->pev->velocity.z + 0.5;

	UTIL_Remove(this);
}

void CBaseDead::BaseSpawn(const char* model)
{
	m_defaultModel = model;
	PRECACHE_MODEL(GetModel());
	InitModel();

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	m_bloodColor = BLOOD_COLOR_RED;

	pev->sequence = GetPoseSequence();

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead hgrunt with bad pose\n");
	}

	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();
}



void CBaseDead::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "head"))
	{
		m_iGruntHead = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}
