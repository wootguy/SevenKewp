/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   This source code contains proprietary and confidential information of
*   Valve LLC and its suppliers.  Access to this code is restricted to
*   persons who have executed a written SDK license with Valve.  Any access,
*   use or distribution of this code by or to any unlicensed person is illegal.
*
****/
//=========================================================
// hgrunt
//=========================================================

//=========================================================
// Hit groups!
//=========================================================
/*

  1 - Head
  2 - Stomach
  3 - Gun

*/


#include "extdll.h"
#include "plane.h"
#include "util.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "CTalkSquadMonster.h"
#include "weapons.h"
#include "effects.h"
#include "customentity.h"
#include "CSoundEnt.h"
#include "CSpore.h"
#include "CShockBeam.h"
#include "CBaseGrunt.h"
#include "skill.h"

int g_fShockTrooperQuestion; // true if an idle grunt asked a question. Cleared when someone answers.
static int iShockTrooperMuzzleFlash;

//=========================================================
// monster-specific DEFINE's
//=========================================================
#define ShockTrooper_SENTENCE_VOLUME (float)0.5	// volume of grunt sentences

namespace STrooperBodyGroup
{
	enum STrooperBodyGroup
	{
		Weapons = 1,
	};
}

namespace STrooperWeapon
{
	enum STrooperWeapon
	{
		Roach = 0,
		None = 1
	};
}

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
#define STROOPER_AE_RELOAD (2)
#define STROOPER_AE_KICK (3)
#define STROOPER_AE_SHOOT (4)
#define STROOPER_AE_GREN_TOSS (7)
#define STROOPER_AE_CAUGHT_ENEMY (10) // grunt established sight with an enemy (player only) that had previously eluded the squad.
#define STROOPER_AE_DROP_GUN (11)	  // grunt (probably dead) is dropping his mp5.

class CShockTrooper : public CTalkSquadMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void SetYawSpeed() override;
	int Classify() override;
	const char* DisplayName();
	int ISoundMask() override;
	void HandleAnimEvent(MonsterEvent_t* pEvent) override;
	BOOL FCanCheckAttacks() override;
	BOOL CheckMeleeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack1(float flDot, float flDist) override;
	BOOL CheckRangeAttack2(float flDot, float flDist) override;
	void CheckAmmo() override;
	void SetActivity(Activity NewActivity) override;
	void StartTask(Task_t* pTask) override;
	void RunTask(Task_t* pTask) override;
	void PainSound() override;
	void IdleSound() override;
	void StartFollowingSound();
	void StopFollowingSound();
	void CantFollowSound();
	Vector GetGunPosition() override;
	void Shoot();
	void PrescheduleThink() override;
	void GibMonster() override;
	void SpeakSentence();

	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-24, -24, 0);
		pev->absmax = pev->origin + Vector(24, 24, 88);
	}

	BOOL Save(CSave& save) override;
	BOOL Restore(CRestore& restore) override;

	CBaseEntity* Kick();
	Schedule_t* GetSchedule() override;
	Schedule_t* GetScheduleOfType(int Type) override;
	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	int IRelationship(CBaseEntity* pTarget) override;

	bool FOkToSpeak();
	void JustSpoke();

	void MonsterThink() override;

	CUSTOM_SCHEDULES;
	static TYPEDESCRIPTION m_SaveData[];

	// checking the feasibility of a grenade toss is kind of costly, so we do it every couple of seconds,
	// not every server frame.
	float m_flNextGrenadeCheck;
	float m_flNextPainTime;
	float m_flLastEnemySightTime;

	Vector m_vecTossVelocity;

	bool m_fThrowGrenade;
	bool m_fStanding;
	bool m_fFirstEncounter; // only put on the handsign show in the squad's first encounter.
	int m_cClipSize;

	float m_flLastShot;

	int m_voicePitch;

	int m_iBrassShell;
	int m_iShotgunShell;

	int m_iSentence;

	float m_flLastChargeTime;
	float m_flLastBlinkTime;
	float m_flLastBlinkInterval;

	static const char* pGruntSentences[];
	static const char* pPainSounds[];
	static const char* pDieSounds[]; // TODO: unused?
	static const char* pAlertSounds[];
};

LINK_ENTITY_TO_CLASS(monster_shocktrooper, CShockTrooper)

const char* CShockTrooper::pPainSounds[] =
{
	"shocktrooper/shock_trooper_pain1.wav",
	"shocktrooper/shock_trooper_pain2.wav",
	"shocktrooper/shock_trooper_pain3.wav",
	"shocktrooper/shock_trooper_pain4.wav",
	"shocktrooper/shock_trooper_pain5.wav",
};

const char* CShockTrooper::pDieSounds[] =
{
	"shocktrooper/shock_trooper_die1.wav",
	"shocktrooper/shock_trooper_die2.wav",
	"shocktrooper/shock_trooper_die3.wav",
	"shocktrooper/shock_trooper_die4.wav",
};

const char* CShockTrooper::pAlertSounds[] =
{
	"shocktrooper/st_alert0.wav",
	"shocktrooper/st_alert1.wav",
	"shocktrooper/st_alert2.wav",
};


TYPEDESCRIPTION CShockTrooper::m_SaveData[] =
{
	DEFINE_FIELD(CShockTrooper, m_flNextGrenadeCheck, FIELD_TIME),
	DEFINE_FIELD(CShockTrooper, m_flNextPainTime, FIELD_TIME),
	//	DEFINE_FIELD( CShockTrooper, m_flLastEnemySightTime, FIELD_TIME ), // don't save, go to zero
	DEFINE_FIELD(CShockTrooper, m_vecTossVelocity, FIELD_VECTOR),
	DEFINE_FIELD(CShockTrooper, m_fThrowGrenade, FIELD_BOOLEAN),
	DEFINE_FIELD(CShockTrooper, m_fStanding, FIELD_BOOLEAN),
	DEFINE_FIELD(CShockTrooper, m_fFirstEncounter, FIELD_BOOLEAN),
	DEFINE_FIELD(CShockTrooper, m_cClipSize, FIELD_INTEGER),
	DEFINE_FIELD(CShockTrooper, m_voicePitch, FIELD_INTEGER),
	//  DEFINE_FIELD( CShotgun, m_iBrassShell, FIELD_INTEGER ),
	//  DEFINE_FIELD( CShotgun, m_iShotgunShell, FIELD_INTEGER ),
	DEFINE_FIELD(CShockTrooper, m_iSentence, FIELD_INTEGER),
	DEFINE_FIELD(CShockTrooper, m_flLastChargeTime, FIELD_FLOAT),
	DEFINE_FIELD(CShockTrooper, m_flLastShot, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CShockTrooper, CTalkSquadMonster)

const char* CShockTrooper::pGruntSentences[] =
{
	"ST_GREN",	  // grenade scared grunt
	"ST_ALERT",	  // sees player
	"ST_MONSTER", // sees monster
	"ST_COVER",	  // running to cover
	"ST_THROW",	  // about to throw grenade
	"ST_CHARGE",  // running out to get the enemy
	"ST_TAUNT",	  // say rude things
};

enum ShockTrooper_SENTENCE_TYPES
{
	ShockTrooper_SENT_NONE = -1,
	ShockTrooper_SENT_GREN = 0,
	ShockTrooper_SENT_ALERT,
	ShockTrooper_SENT_MONSTER,
	ShockTrooper_SENT_COVER,
	ShockTrooper_SENT_THROW,
	ShockTrooper_SENT_CHARGE,
	ShockTrooper_SENT_TAUNT,
};

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
void CShockTrooper::SpeakSentence()
{
	if (m_iSentence == ShockTrooper_SENT_NONE)
	{
		// no sentence cued up.
		return;
	}

	if (FOkToSpeak())
	{
		//SENTENCEG_PlayRndSz(ENT(pev), pGruntSentences[m_iSentence], ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
		
		// custom sentences can't be sent to vanilla Half-Life clients, so sentences are combined
		// into wav files and played normally
		switch (m_iSentence) {
		case 0:
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_gren0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			break;
		case 1:
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_alert0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			break;
		case 2:
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_monster0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			break;
		case 3:
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_cover0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			break;
		case 4:
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_throw0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			break;
		case 5:
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_charge0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			break;
		case 6:
			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_taunt0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			break;
		}

		JustSpoke();
	}
}

//=========================================================
// IRelationship - overridden because Alien Grunts are
// Human Grunt's nemesis.
//=========================================================
int CShockTrooper::IRelationship(CBaseEntity* pTarget)
{
	int r = CTalkSquadMonster::IRelationship(pTarget);

	if ((r >= R_DL) && (FClassnameIs(pTarget->pev, "monster_alien_grunt") || (FClassnameIs(pTarget->pev, "monster_gargantua"))))
	{
		return R_NM;
	}
	else
		return r;
}

//=========================================================
// GibMonster - make gun fly through the air.
//=========================================================
void CShockTrooper::GibMonster()
{
	Vector vecGunPos;
	Vector vecGunAngles;

	if (GetBodygroup(STrooperBodyGroup::Weapons) != STrooperWeapon::None && npc_dropweapons.value)
	{ // throw a gun if the grunt has one
		GetAttachment(0, vecGunPos, vecGunAngles);

		//Only copy the yaw
		vecGunAngles.x = vecGunAngles.z = 0;

		CBaseEntity* pGun = DropItem("monster_shockroach", vecGunPos + Vector(0, 0, 32), vecGunAngles);
		CBaseMonster* mon = pGun ? pGun->MyMonsterPointer() : NULL;

		if (mon)
		{
			// prevent dropping roach inside walls
			TraceResult tr;
			TRACE_MONSTER_HULL(mon->edict(), mon->pev->origin, mon->pev->origin, ignore_monsters, NULL, &tr);
			if (tr.fStartSolid) {
				UTIL_SetOrigin(mon->pev, pev->origin + Vector(0, 0, pev->maxs.z * 0.5f));
			}

			mon->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));
			mon->pev->avelocity = Vector(0, RANDOM_FLOAT(200, 400), 0);

			if (CBaseEntity::IRelationship(Classify(), CLASS_PLAYER) == R_AL) {
				mon->pev->skin = 1;
				mon->m_IsPlayerAlly = TRUE;
			}
		}

		//TODO: change body group
	}

	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM);

	if (CVAR_GET_FLOAT("violence_agibs") != 0) // Should never get here, but someone might call it directly
	{
		CGib::SpawnRandomMergedGibs( pev, 6, MERGE_MDL_STROOPER_GIBS, 0 );	// Throw alien gibs
	}

	// don't remove players!
	SetThink(&CBaseMonster::SUB_Remove);
	pev->nextthink = gpGlobals->time;

	// if the entity is outside the valid range for sound origins, then it needs
	// to be networked at least until the sound starts playing or else clients won't hear it.
	if (!UTIL_IsValidTempEntOrigin(pev->origin)) {
		pev->flags &= ~FL_MONSTER; // prevent the crowbar thinking this is a valid target
		pev->renderamt = 0;
		pev->rendermode = kRenderTransTexture;
		pev->nextthink = gpGlobals->time + 0.1f;
	}
}

//=========================================================
// ISoundMask - Overidden for human grunts because they
// hear the DANGER sound that is made by hand grenades and
// other dangerous items.
//=========================================================
int CShockTrooper::ISoundMask()
{
	return bits_SOUND_WORLD |
		bits_SOUND_COMBAT |
		bits_SOUND_PLAYER |
		bits_SOUND_DANGER;
}

//=========================================================
// someone else is talking - don't speak
//=========================================================
bool CShockTrooper::FOkToSpeak()
{
	// if someone else is talking, don't speak
	if (gpGlobals->time <= CTalkSquadMonster::g_talkWaitTime)
		return false;

	if ((pev->spawnflags & SF_MONSTER_GAG) != 0)
	{
		if (m_MonsterState != MONSTERSTATE_COMBAT)
		{
			// no talking outside of combat if gagged.
			return false;
		}
	}

	// if player is not in pvs, don't speak
	//	if (FNullEnt(FIND_CLIENT_IN_PVS(edict())))
	//		return false;

	return true;
}

//=========================================================
//=========================================================
void CShockTrooper::JustSpoke()
{
	CTalkSquadMonster::g_talkWaitTime = gpGlobals->time + RANDOM_FLOAT(1.5, 2.0);
	m_iSentence = ShockTrooper_SENT_NONE;
}

//=========================================================
// PrescheduleThink - this function runs after conditions
// are collected and before scheduling code is run.
//=========================================================
void CShockTrooper::PrescheduleThink()
{
	if (InSquad() && m_hEnemy != NULL)
	{
		if (HasConditions(bits_COND_SEE_ENEMY))
		{
			// update the squad's last enemy sighting time.
			MySquadLeader()->m_flLastEnemySightTime = gpGlobals->time;
		}
		else
		{
			if (gpGlobals->time - MySquadLeader()->m_flLastEnemySightTime > 5)
			{
				// been a while since we've seen the enemy
				MySquadLeader()->m_fEnemyEluded = true;
			}
		}
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
BOOL CShockTrooper::FCanCheckAttacks()
{
	// if you can see the enemy but can't move into optimal range, try attacking it anyway	
	return !HasConditions(bits_COND_ENEMY_TOOFAR) || HasMemory(bits_MEMORY_MOVE_FAILED);
}


//=========================================================
// CheckMeleeAttack1
//=========================================================
BOOL CShockTrooper::CheckMeleeAttack1(float flDot, float flDist)
{
	CBaseMonster* pEnemy = m_hEnemy != NULL ? m_hEnemy->MyMonsterPointer() : NULL;

	if (!pEnemy)
	{
		return false;
	}

	if (flDist <= 64 && flDot >= 0.7 &&
		pEnemy->Classify() != CLASS_ALIEN_BIOWEAPON &&
		pEnemy->Classify() != CLASS_PLAYER_BIOWEAPON)
	{
		return true;
	}
	return false;
}

//=========================================================
// CheckRangeAttack1 - overridden for HGrunt, cause
// FCanCheckAttacks() doesn't disqualify all attacks based
// on whether or not the enemy is occluded because unlike
// the base class, the HGrunt can attack when the enemy is
// occluded (throw grenade over wall, etc). We must
// disqualify the machine gun attack if the enemy is occluded.
//=========================================================
BOOL CShockTrooper::CheckRangeAttack1(float flDot, float flDist)
{
	if (gpGlobals->time - m_flLastShot > 0.175 && !HasConditions(bits_COND_ENEMY_OCCLUDED) && flDist <= 2048 && flDot >= 0.5 && NoFriendlyFire())
	{
		TraceResult tr;

		if (!m_hEnemy->IsPlayer() && flDist <= 64)
		{
			// kick nonclients, but don't shoot at them.
			return false;
		}

		Vector vecSrc = GetGunPosition();

		// verify that a bullet fired from the gun will hit the enemy before the world.
		UTIL_TraceLine(vecSrc, m_hEnemy->BodyTarget(vecSrc), ignore_monsters, ignore_glass, ENT(pev), &tr);

		if (tr.flFraction == 1.0 || tr.pHit == m_hEnemy->edict())
		{
			return true;
		}
	}

	return false;
}

//=========================================================
// CheckRangeAttack2 - this checks the Grunt's grenade
// attack.
//=========================================================
BOOL CShockTrooper::CheckRangeAttack2(float flDot, float flDist)
{
	if (!FBitSet(pev->weapons, HGRUNT_HANDGRENADE))
	{
		return false;
	}

	// if the grunt isn't moving, it's ok to check.
	if (m_flGroundSpeed != 0)
	{
		m_fThrowGrenade = false;
		return m_fThrowGrenade;
	}

	// assume things haven't changed too much since last time
	if (gpGlobals->time < m_flNextGrenadeCheck)
	{
		return m_fThrowGrenade;
	}

	if (!FBitSet(m_hEnemy->pev->flags, FL_ONGROUND) && m_hEnemy->pev->waterlevel == 0 && m_vecEnemyLKP.z > pev->absmax.z)
	{
		//!!!BUGBUG - we should make this check movetype and make sure it isn't FLY? Players who jump a lot are unlikely to
		// be grenaded.
		// don't throw grenades at anything that isn't on the ground!
		m_fThrowGrenade = false;
		return m_fThrowGrenade;
	}

	Vector vecTarget;

	if (FBitSet(pev->weapons, HGRUNT_HANDGRENADE))
	{
		// find feet
		if (RANDOM_LONG(0, 1))
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
		if (HasConditions(bits_COND_SEE_ENEMY))
			vecTarget = vecTarget + ((vecTarget - pev->origin).Length() / gSkillData.sk_shocktrooper_grenadespeed) * m_hEnemy->pev->velocity;
	}

	// are any of my squad members near the intended grenade impact area?
	if (InSquad())
	{
		if (SquadMemberInRange(vecTarget, 256))
		{
			// crap, I might blow my own guy up. Don't throw a grenade and don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
			m_fThrowGrenade = false;
		}
	}

	if ((vecTarget - pev->origin).Length2D() <= 256)
	{
		// crap, I don't want to blow myself up
		m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		m_fThrowGrenade = false;
		return m_fThrowGrenade;
	}


	if (FBitSet(pev->weapons, HGRUNT_HANDGRENADE))
	{
		Vector vecToss = VecCheckToss(pev, GetGunPosition(), vecTarget, 0.5);

		if (vecToss != g_vecZero)
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = true;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = false;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}
	else
	{
		Vector vecToss = VecCheckThrow(pev, GetGunPosition(), vecTarget, gSkillData.sk_shocktrooper_grenadespeed, 0.5);

		if (vecToss != g_vecZero)
		{
			m_vecTossVelocity = vecToss;

			// throw a hand grenade
			m_fThrowGrenade = true;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 0.3; // 1/3 second.
		}
		else
		{
			// don't throw
			m_fThrowGrenade = false;
			// don't check again for a while.
			m_flNextGrenadeCheck = gpGlobals->time + 1; // one full second.
		}
	}



	return m_fThrowGrenade;
}


//=========================================================
// TraceAttack - make sure we're not taking it in the helmet
//=========================================================
void CShockTrooper::TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType)
{
	CTalkSquadMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}


//=========================================================
// TakeDamage - overridden for the grunt because the grunt
// needs to forget that he is in cover if he's hurt. (Obviously
// not in a safe place anymore).
//=========================================================
int CShockTrooper::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (IsImmune(pevAttacker, flDamage))
		return 0;

	Forget(bits_MEMORY_INCOVER);

	return CTalkSquadMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

//=========================================================
// SetYawSpeed - allows each sequence to have a different
// turn rate associated with it.
//=========================================================
void CShockTrooper::SetYawSpeed()
{
	int ys;

	switch (m_Activity)
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

void CShockTrooper::IdleSound()
{
	if (!mp_npcidletalk.value) {
		return;
	}

	if (FOkToSpeak() && (0 != g_fShockTrooperQuestion || RANDOM_LONG(0, 1)))
	{
		if (0 == g_fShockTrooperQuestion)
		{
			// ask question or make statement
			switch (RANDOM_LONG(0, 2))
			{
			case 0: // check in
				//SENTENCEG_PlayRndSz(ENT(pev), "ST_CHECK", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_check0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fShockTrooperQuestion = 1;
				break;
			case 1: // question
				//SENTENCEG_PlayRndSz(ENT(pev), "ST_QUEST", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_quest0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				g_fShockTrooperQuestion = 2;
				break;
			case 2: // statement
				//SENTENCEG_PlayRndSz(ENT(pev), "ST_IDLE", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_idle0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
		}
		else
		{
			switch (g_fShockTrooperQuestion)
			{
			case 1: // check in
				//SENTENCEG_PlayRndSz(ENT(pev), "ST_CLEAR", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_clear0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			case 2: // question
				//SENTENCEG_PlayRndSz(ENT(pev), "ST_ANSWER", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_answer0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
				break;
			}
			g_fShockTrooperQuestion = 0;
		}
		JustSpoke();
	}
}

void CShockTrooper::StartFollowingSound() {
	int r = RANDOM_LONG(0, 2);

	// complicated/energetic speech
	switch (r) {
	case 0:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_quest0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
		break;
	case 1:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_cover0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
		break;
	case 2:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_throw0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
		break;
	}
	JustSpoke();
}

void CShockTrooper::StopFollowingSound() {
	int r = RANDOM_LONG(0, 2);

	// simple 2 word speech, low energy
	switch (r) {
	case 0:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_taunt0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
		break;
	case 1:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_check0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
		break;
	case 2:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_answer0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
		break;
	}
	JustSpoke();
}

void CShockTrooper::CantFollowSound() {
	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_taunt0.wav", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, m_voicePitch);
	JustSpoke();
}

//=========================================================
// CheckAmmo - overridden for the grunt because he actually
// uses ammo! (base class doesn't)
//=========================================================
void CShockTrooper::CheckAmmo()
{
	if (m_cAmmoLoaded <= 0)
	{
		SetConditions(bits_COND_NO_AMMO_LOADED);
	}
}

//=========================================================
// Classify - indicates this monster's place in the
// relationship table.
//=========================================================
int CShockTrooper::Classify()
{
	return CBaseMonster::Classify(CLASS_ALIEN_RACE_X);
}

const char* CShockTrooper::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Shock Trooper";
}

//=========================================================
//=========================================================
CBaseEntity* CShockTrooper::Kick()
{
	TraceResult tr;

	UTIL_MakeVectors(pev->angles);
	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * 70);

	UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

	if (tr.pHit)
	{
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);
		return pEntity;
	}

	return NULL;
}

//=========================================================
// GetGunPosition	return the end of the barrel
//=========================================================

Vector CShockTrooper::GetGunPosition()
{
	UTIL_MakeVectors(pev->angles);

	Vector vecShootOrigin, vecShootDir;

	GetAttachment(0, vecShootOrigin, vecShootDir);

	return gpGlobals->v_forward * 32 + vecShootOrigin;
}

//=========================================================
// Shoot
//=========================================================
void CShockTrooper::Shoot()
{
	if (m_hEnemy == NULL)
	{
		return;
	}

	if (gpGlobals->time - m_flLastShot <= 0.11)
	{
		return;
	}

	Vector vecShootOrigin = GetGunPosition();
	Vector vecShootDir = ShootAtEnemy(vecShootOrigin);

	Vector angDir = UTIL_VecToAngles(vecShootDir);

	UTIL_MakeVectors(pev->angles);

	auto shootAngles = angDir;

	shootAngles.x = -shootAngles.x;

	auto pBeam = CShockBeam::CreateShockBeam(vecShootOrigin, shootAngles, this);
	CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_GUN_VOLUME, 0.3);

	pBeam->pev->velocity = vecShootDir * 2000;
	pBeam->pev->speed = 2000;

	m_cAmmoLoaded--; // take away a bullet!

	SetBlending(0, angDir.x);

	m_flLastShot = gpGlobals->time;
}

//=========================================================
// HandleAnimEvent - catches the monster-specific messages
// that occur when tagged animation frames are played.
//=========================================================
void CShockTrooper::HandleAnimEvent(MonsterEvent_t* pEvent)
{
	Vector vecShootDir;
	Vector vecShootOrigin;

	switch (pEvent->event)
	{
	case STROOPER_AE_DROP_GUN:
	{
		if (GetBodygroup(STrooperBodyGroup::Weapons) != STrooperWeapon::None && npc_dropweapons.value)
		{
			Vector vecGunPos;
			//Zero this out so we don't end up with garbage angles later on
			Vector vecGunAngles = g_vecZero;

			GetAttachment(0, vecGunPos, vecGunAngles);

			//Only copy yaw
			vecGunAngles.x = vecGunAngles.z = 0;

			// switch to body group with no gun.
			SetBodygroup(STrooperBodyGroup::Weapons, STrooperWeapon::None);

			// now spawn a gun.
			CBaseEntity* pRoach = DropItem("monster_shockroach", pev->origin + Vector(0, 0, 48), vecGunAngles);
			CBaseMonster* mon = pRoach ? pRoach->MyMonsterPointer() : NULL;

			if (mon)
			{
				mon->pev->velocity = Vector(RANDOM_FLOAT(-20, 20), RANDOM_FLOAT(-20, 20), RANDOM_FLOAT(20, 30));
				mon->pev->avelocity = Vector(0, RANDOM_FLOAT(20, 40), 0);

				if (CBaseEntity::IRelationship(Classify(), CLASS_PLAYER) == R_AL) {
					mon->pev->skin = 1;
					mon->m_IsPlayerAlly = TRUE;
				}
			}
		}
	}
	break;

	case STROOPER_AE_GREN_TOSS:
	{
		UTIL_MakeVectors(pev->angles);
		// CGrenade::ShootTimed( pev, pev->origin + gpGlobals->v_forward * 34 + Vector (0, 0, 32), m_vecTossVelocity, 3.5 );
		CSpore::CreateSpore(pev->origin + Vector(0, 0, 98), m_vecTossVelocity, this, CSpore::SporeType::GRENADE, true, false);

		m_fThrowGrenade = false;
		m_flNextGrenadeCheck = gpGlobals->time + 6; // wait six seconds before even looking again to see if a grenade can be thrown.
													// !!!LATER - when in a group, only try to throw grenade if ordered.
	}
	break;

	case STROOPER_AE_SHOOT:
	{
		Vector vecArmPos;
		Vector vecArmAngles;

		GetAttachment(0, vecArmPos, vecArmAngles);

		UTIL_Sprite(vecArmPos, iShockTrooperMuzzleFlash, 4, 128);

		Shoot();

		EMIT_SOUND(ENT(pev), CHAN_WEAPON, "weapons/shock_fire.wav", 1, ATTN_NORM);

		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, 384, 0.3);
	}
	break;

	case STROOPER_AE_KICK:
	{
		CBaseEntity* pHurt = Kick();

		if (pHurt)
		{
			// SOUND HERE!
			UTIL_MakeVectors(pev->angles);
			pHurt->pev->punchangle.x = 15;
			pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_forward * 100 + gpGlobals->v_up * 50;
			pHurt->TakeDamage(pev, pev, gSkillData.sk_shocktrooper_kick, DMG_CLUB);
		}
	}
	break;

	case STROOPER_AE_CAUGHT_ENEMY:
	{
		if (FOkToSpeak())
		{
			SENTENCEG_PlayRndSz(ENT(pev), "ST_ALERT", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
			JustSpoke();
		}
	}
	break;

	default:
		CTalkSquadMonster::HandleAnimEvent(pEvent);
		break;
	}
}

//=========================================================
// Spawn
//=========================================================
void CShockTrooper::Spawn()
{
	Precache();

	m_skinFrames = 4;
	InitModel();
	SetSize(Vector(-24, -24, 0), Vector(24, 24, 72));

	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_STEP;
	m_bloodColor = BLOOD_COLOR_GREEN;
	pev->effects = 0;
	m_flFieldOfView = 0.2; // indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState = MONSTERSTATE_NONE;
	m_flNextGrenadeCheck = gpGlobals->time + 1;
	m_flNextPainTime = gpGlobals->time;
	m_iSentence = ShockTrooper_SENT_NONE;

	m_afCapability = bits_CAP_SQUAD | bits_CAP_TURN_HEAD | bits_CAP_DOORS_GROUP;

	m_fEnemyEluded = false;
	m_fFirstEncounter = true; // this is true when the grunt spawns, because he hasn't encountered an enemy yet.

	m_HackedGunPos = Vector(0, 0, 55);

	SetBodygroup(STrooperBodyGroup::Weapons, STrooperWeapon::Roach);

	pev->weapons = HGRUNT_9MMAR | HGRUNT_HANDGRENADE;

	m_cClipSize = gSkillData.sk_shocktrooper_maxcharge;
	m_cAmmoLoaded = m_cClipSize;

	m_flLastChargeTime = m_flLastBlinkTime = m_flLastBlinkInterval = gpGlobals->time;

	CTalkSquadMonster::g_talkWaitTime = 0;

	m_flLastShot = gpGlobals->time;

	MonsterInit();
}

//=========================================================
// Precache - precaches all resources this monster needs
//=========================================================
void CShockTrooper::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/strooper.mdl";
	PRECACHE_MODEL(GetModel());
	PRECACHE_REPLACEMENT_MODEL("models/strooper_gibs.mdl");

	PRECACHE_SOUND("weapons/shock_fire.wav");
	PRECACHE_SOUND("shocktrooper/shock_trooper_attack.wav");

	if (mp_npcidletalk.value) {
		PRECACHE_SOUND("shocktrooper/st_check0.wav");
		PRECACHE_SOUND("shocktrooper/st_quest0.wav");
		PRECACHE_SOUND("shocktrooper/st_idle0.wav");
		PRECACHE_SOUND("shocktrooper/st_clear0.wav");
		PRECACHE_SOUND("shocktrooper/st_answer0.wav");
	}

	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDieSounds);
	PRECACHE_SOUND_ARRAY(pAlertSounds);

	PRECACHE_SOUND("shocktrooper/st_charge0.wav");
	PRECACHE_SOUND("shocktrooper/st_gren0.wav");
	PRECACHE_SOUND("shocktrooper/st_alert0.wav");
	PRECACHE_SOUND("shocktrooper/st_monst0.wav");
	PRECACHE_SOUND("shocktrooper/st_throw0.wav");
	PRECACHE_SOUND("shocktrooper/st_taunt0.wav");
	PRECACHE_SOUND("shocktrooper/st_cover0.wav");

	// get voice pitch
	if (RANDOM_LONG(0, 1))
		m_voicePitch = 109 + RANDOM_LONG(0, 7);
	else
		m_voicePitch = 100;

	UTIL_PrecacheOther("monster_shockroach");
	UTIL_PrecacheOther("shock_beam");
	UTIL_PrecacheOther("spore");

	iShockTrooperMuzzleFlash = PRECACHE_MODEL("sprites/muzzle_shock.spr");
}

//=========================================================
// start task
//=========================================================
void CShockTrooper::StartTask(Task_t* pTask)
{
	m_iTaskStatus = TASKSTATUS_RUNNING;

	switch (pTask->iTask)
	{
	case TASK_GRUNT_CHECK_FIRE:
		if (!NoFriendlyFire())
		{
			SetConditions(bits_COND_GRUNT_NOFIRE);
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
		Forget(bits_MEMORY_INCOVER);
		CTalkSquadMonster::StartTask(pTask);
		break;

	case TASK_RELOAD:
		m_IdealActivity = ACT_RELOAD;
		break;

	case TASK_GRUNT_FACE_TOSS_DIR:
		break;

	case TASK_FACE_IDEAL:
	case TASK_FACE_ENEMY:
		CTalkSquadMonster::StartTask(pTask);
		if (pev->movetype == MOVETYPE_FLY || pev->movetype == MOVETYPE_BOUNCE)
		{
			m_IdealActivity = ACT_GLIDE;
		}
		break;

	default:
		CTalkSquadMonster::StartTask(pTask);
		break;
	}
}

//=========================================================
// RunTask
//=========================================================
void CShockTrooper::RunTask(Task_t* pTask)
{
	switch (pTask->iTask)
	{
	case TASK_GRUNT_FACE_TOSS_DIR:
	{
		// project a point along the toss vector and turn to face that point.
		MakeIdealYaw(pev->origin + m_vecTossVelocity * 64);
		ChangeYaw(pev->yaw_speed);

		if (FacingIdeal())
		{
			m_iTaskStatus = TASKSTATUS_COMPLETE;
		}
		break;
	}
	default:
	{
		CTalkSquadMonster::RunTask(pTask);
		break;
	}
	}
}

//=========================================================
// PainSound
//=========================================================
void CShockTrooper::PainSound()
{
	if (gpGlobals->time > m_flNextPainTime)
	{
#if 0
		if (RANDOM_LONG(0, 99) < 5)
		{
			// pain sentences are rare
			if (FOkToSpeak())
			{
				SENTENCEG_PlayRndSz(ENT(pev), "ST_PAIN", ShockTrooper_SENTENCE_VOLUME, ATTN_NORM, 0, PITCH_NORM);
				JustSpoke();
				return;
			}
		}
#endif
		EMIT_SOUND(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1, ATTN_NORM);

		m_flNextPainTime = gpGlobals->time + 1;
	}
}

//=========================================================
// AI Schedules Specific to this monster
//=========================================================

//=========================================================
// GruntFail
//=========================================================
Task_t tlShockTrooperFail[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_SET_ACTIVITY, (float)ACT_IDLE},
	{TASK_WAIT, (float)2},
	{TASK_WAIT_PVS, (float)0},
};

Schedule_t slShockTrooperFail[] =
{
	{tlShockTrooperFail,
		ARRAYSIZE(tlShockTrooperFail),
		bits_COND_CAN_RANGE_ATTACK1 |
			bits_COND_CAN_RANGE_ATTACK2 |
			bits_COND_CAN_MELEE_ATTACK1 |
			bits_COND_CAN_MELEE_ATTACK2,
		0,
		"SHOCK_FAIL"},
};

//=========================================================
// Grunt Combat Fail
//=========================================================
Task_t tlShockTrooperCombatFail[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_SET_ACTIVITY, (float)ACT_IDLE},
	{TASK_REMEMBER, (float)bits_MEMORY_MOVE_FAILED},
	{TASK_WAIT_FACE_ENEMY, (float)2},
	{TASK_WAIT_PVS, (float)0},
};

Schedule_t slShockTrooperCombatFail[] =
{
	{tlShockTrooperCombatFail,
		ARRAYSIZE(tlShockTrooperCombatFail),
		bits_COND_CAN_RANGE_ATTACK1 |
			bits_COND_CAN_RANGE_ATTACK2,
		0,
		"SHOCK_COMBAT_FAIL"},
};

//=========================================================
// Victory dance!
//=========================================================
Task_t tlShockTrooperVictoryDance[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_SET_ACTIVITY, (float)ACT_IDLE},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_WAIT, (float)1.5},
	{TASK_GET_PATH_TO_ENEMY_CORPSE, (float)0},
	{TASK_WALK_PATH, (float)0},
	{TASK_WAIT_FOR_MOVEMENT, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_PLAY_SEQUENCE, (float)ACT_VICTORY_DANCE},
};

Schedule_t slShockTrooperVictoryDance[] =
{
	{tlShockTrooperVictoryDance,
		ARRAYSIZE(tlShockTrooperVictoryDance),
		bits_COND_NEW_ENEMY |
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE,
		0,
		"SHOCK_VICTORY_DANCE"},
};

//=========================================================
// Establish line of fire - move to a position that allows
// the grunt to attack.
//=========================================================
Task_t tlShockTrooperEstablishLineOfFire[] =
{
	{TASK_SET_FAIL_SCHEDULE, (float)SCHED_GRUNT_ELOF_FAIL},
	{TASK_GET_PATH_TO_ENEMY, (float)0},
	{TASK_GRUNT_SPEAK_SENTENCE, (float)0},
	{TASK_RUN_PATH, (float)0},
	{TASK_WAIT_FOR_MOVEMENT, (float)0},
};

Schedule_t slShockTrooperEstablishLineOfFire[] =
{
	{tlShockTrooperEstablishLineOfFire,
		ARRAYSIZE(tlShockTrooperEstablishLineOfFire),
		bits_COND_NEW_ENEMY |
			bits_COND_ENEMY_DEAD |
			bits_COND_CAN_RANGE_ATTACK1 |
			bits_COND_CAN_MELEE_ATTACK1 |
			bits_COND_CAN_RANGE_ATTACK2 |
			bits_COND_CAN_MELEE_ATTACK2 |
			bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"SHOCK_ESTABLISH_LINE_OF_FIRE"},
};

//=========================================================
// GruntFoundEnemy - grunt established sight with an enemy
// that was hiding from the squad.
//=========================================================
Task_t tlShockTrooperFoundEnemy[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_SIGNAL1},
};

Schedule_t slShockTrooperFoundEnemy[] =
{
	{tlShockTrooperFoundEnemy,
		ARRAYSIZE(tlShockTrooperFoundEnemy),
		bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"SHOCK_FOUND_ENEMY"},
};

//=========================================================
// GruntCombatFace Schedule
//=========================================================
Task_t tlShockTrooperCombatFace1[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_SET_ACTIVITY, (float)ACT_IDLE},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_WAIT, (float)1.5},
	{TASK_SET_SCHEDULE, (float)SCHED_GRUNT_SWEEP},
};

Schedule_t slShockTrooperCombatFace[] =
{
	{tlShockTrooperCombatFace1,
		ARRAYSIZE(tlShockTrooperCombatFace1),
		bits_COND_NEW_ENEMY |
			bits_COND_ENEMY_DEAD |
			bits_COND_CAN_RANGE_ATTACK1 |
			bits_COND_CAN_RANGE_ATTACK2,
		0,
		"SHOCK_COMBAT_FACE"},
};

//=========================================================
// Suppressing fire - don't stop shooting until the clip is
// empty or grunt gets hurt.
//=========================================================
Task_t tlShockTrooperSignalSuppress[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_FACE_IDEAL, (float)0},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_SIGNAL2},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
};

Schedule_t slShockTrooperSignalSuppress[] =
{
	{tlShockTrooperSignalSuppress,
		ARRAYSIZE(tlShockTrooperSignalSuppress),
		bits_COND_ENEMY_DEAD |
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_HEAR_SOUND |
			bits_COND_GRUNT_NOFIRE |
			bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"SHOCK_SIGNAL_SUPPRESS"},
};

Task_t tlShockTrooperSuppress[] =
{
	{TASK_STOP_MOVING, 0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
};

Schedule_t slShockTrooperSuppress[] =
{
	{tlShockTrooperSuppress,
		ARRAYSIZE(tlShockTrooperSuppress),
		bits_COND_ENEMY_DEAD |
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_HEAR_SOUND |
			bits_COND_GRUNT_NOFIRE |
			bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"SHOCK_SUPPRESS"},
};


//=========================================================
// grunt wait in cover - we don't allow danger or the ability
// to attack to break a grunt's run to cover schedule, but
// when a grunt is in cover, we do want them to attack if they can.
//=========================================================
Task_t tlShockTrooperWaitInCover[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_SET_ACTIVITY, (float)ACT_IDLE},
	{TASK_WAIT_FACE_ENEMY, (float)1},
};

Schedule_t slShockTrooperWaitInCover[] =
{
	{tlShockTrooperWaitInCover,
		ARRAYSIZE(tlShockTrooperWaitInCover),
		bits_COND_NEW_ENEMY |
			bits_COND_HEAR_SOUND |
			bits_COND_CAN_RANGE_ATTACK1 |
			bits_COND_CAN_RANGE_ATTACK2 |
			bits_COND_CAN_MELEE_ATTACK1 |
			bits_COND_CAN_MELEE_ATTACK2,

		bits_SOUND_DANGER,
		"SHOCK_WAIT_IN_COVER"},
};

//=========================================================
// run to cover.
// !!!BUGBUG - set a decent fail schedule here.
//=========================================================
Task_t tlShockTrooperTakeCover1[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_SET_FAIL_SCHEDULE, (float)SCHED_GRUNT_TAKECOVER_FAILED},
	{TASK_WAIT, (float)0.2},
	{TASK_FIND_COVER_FROM_ENEMY, (float)0},
	{TASK_GRUNT_SPEAK_SENTENCE, (float)0},
	{TASK_RUN_PATH, (float)0},
	{TASK_WAIT_FOR_MOVEMENT, (float)0},
	{TASK_REMEMBER, (float)bits_MEMORY_INCOVER},
	{TASK_SET_SCHEDULE, (float)SCHED_GRUNT_WAIT_FACE_ENEMY},
};

Schedule_t slShockTrooperTakeCover[] =
{
	{tlShockTrooperTakeCover1,
		ARRAYSIZE(tlShockTrooperTakeCover1),
		0,
		0,
		"SHOCK_TAKE_COVER"},
};

//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t tlShockTrooperGrenadeCover1[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_FIND_COVER_FROM_ENEMY, (float)99},
	{TASK_FIND_FAR_NODE_COVER_FROM_ENEMY, (float)384},
	{TASK_PLAY_SEQUENCE, (float)ACT_SPECIAL_ATTACK1},
	{TASK_CLEAR_MOVE_WAIT, (float)0},
	{TASK_RUN_PATH, (float)0},
	{TASK_WAIT_FOR_MOVEMENT, (float)0},
	{TASK_SET_SCHEDULE, (float)SCHED_GRUNT_WAIT_FACE_ENEMY},
};

Schedule_t slShockTrooperGrenadeCover[] =
{
	{tlShockTrooperGrenadeCover1,
		ARRAYSIZE(tlShockTrooperGrenadeCover1),
		0,
		0,
		"SHOCK_GRENADE_COVER"},
};


//=========================================================
// drop grenade then run to cover.
//=========================================================
Task_t tlShockTrooperTossGrenadeCover1[] =
{
	{TASK_FACE_ENEMY, (float)0},
	{TASK_RANGE_ATTACK2, (float)0},
	{TASK_SET_SCHEDULE, (float)SCHED_TAKE_COVER_FROM_ENEMY},
};

Schedule_t slShockTrooperTossGrenadeCover[] =
{
	{tlShockTrooperTossGrenadeCover1,
		ARRAYSIZE(tlShockTrooperTossGrenadeCover1),
		0,
		0,
		"SHOCK_TOSS_GRENADE_COVER"},
};

//=========================================================
// hide from the loudest sound source (to run from grenade)
//=========================================================
Task_t tlShockTrooperTakeCoverFromBestSound[] =
{
	{TASK_SET_FAIL_SCHEDULE, (float)SCHED_COWER}, // duck and cover if cannot move from explosion
	{TASK_STOP_MOVING, (float)0},
	{TASK_FIND_COVER_FROM_BEST_SOUND, (float)0},
	{TASK_RUN_PATH, (float)0},
	{TASK_WAIT_FOR_MOVEMENT, (float)0},
	{TASK_REMEMBER, (float)bits_MEMORY_INCOVER},
	{TASK_TURN_LEFT, (float)179},
};

Schedule_t slShockTrooperTakeCoverFromBestSound[] =
{
	{tlShockTrooperTakeCoverFromBestSound,
		ARRAYSIZE(tlShockTrooperTakeCoverFromBestSound),
		0,
		0,
		"SHOCK_TAKE_COVER_FROM_BEST_SOUND"},
};

//=========================================================
// Grunt reload schedule
//=========================================================
Task_t tlShockTrooperHideReload[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_SET_FAIL_SCHEDULE, (float)SCHED_RELOAD},
	{TASK_FIND_COVER_FROM_ENEMY, (float)0},
	{TASK_RUN_PATH, (float)0},
	{TASK_WAIT_FOR_MOVEMENT, (float)0},
	{TASK_REMEMBER, (float)bits_MEMORY_INCOVER},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_PLAY_SEQUENCE, (float)ACT_RELOAD},
};

Schedule_t slShockTrooperHideReload[] =
{
	{tlShockTrooperHideReload,
		ARRAYSIZE(tlShockTrooperHideReload),
		bits_COND_HEAVY_DAMAGE |
			bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"SHOCK_HIDE_RELOAD"} };

//=========================================================
// Do a turning sweep of the area
//=========================================================
Task_t tlShockTrooperSweep[] =
{
	{TASK_TURN_LEFT, (float)179},
	{TASK_WAIT, (float)1},
	{TASK_TURN_LEFT, (float)179},
	{TASK_WAIT, (float)1},
};

Schedule_t slShockTrooperSweep[] =
{
	{tlShockTrooperSweep,
		ARRAYSIZE(tlShockTrooperSweep),

		bits_COND_NEW_ENEMY |
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_CAN_RANGE_ATTACK1 |
			bits_COND_CAN_RANGE_ATTACK2 |
			bits_COND_HEAR_SOUND,

		bits_SOUND_WORLD | // sound flags
			bits_SOUND_DANGER |
			bits_SOUND_PLAYER,

		"SHOCK_SWEEP"
	},
};

//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t tlShockTrooperRangeAttack1A[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_CROUCH},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
};

Schedule_t slShockTrooperRangeAttack1A[] =
{
	{tlShockTrooperRangeAttack1A,
		ARRAYSIZE(tlShockTrooperRangeAttack1A),
		bits_COND_NEW_ENEMY |
			bits_COND_ENEMY_DEAD |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_ENEMY_OCCLUDED |
			bits_COND_HEAR_SOUND |
			bits_COND_GRUNT_NOFIRE |
			bits_COND_NO_AMMO_LOADED,

		bits_SOUND_DANGER,
		"SHOCK_RANGE_ATTACK1A"},
};


//=========================================================
// primary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t tlShockTrooperRangeAttack1B[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_PLAY_SEQUENCE_FACE_ENEMY, (float)ACT_IDLE_ANGRY},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_GRUNT_CHECK_FIRE, (float)0},
	{TASK_RANGE_ATTACK1, (float)0},
};

Schedule_t slShockTrooperRangeAttack1B[] =
{
	{tlShockTrooperRangeAttack1B,
		ARRAYSIZE(tlShockTrooperRangeAttack1B),
		bits_COND_NEW_ENEMY |
			bits_COND_ENEMY_DEAD |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_ENEMY_OCCLUDED |
			bits_COND_NO_AMMO_LOADED |
			bits_COND_GRUNT_NOFIRE |
			bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER,
		"SHOCK_RANGE_ATTACK1B"},
};

//=========================================================
// secondary range attack. Overriden because base class stops attacking when the enemy is occluded.
// grunt's grenade toss requires the enemy be occluded.
//=========================================================
Task_t tlShockTrooperRangeAttack2[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_GRUNT_FACE_TOSS_DIR, (float)0},
	{TASK_PLAY_SEQUENCE, (float)ACT_RANGE_ATTACK2},
	{TASK_SET_SCHEDULE, (float)SCHED_GRUNT_WAIT_FACE_ENEMY}, // don't run immediately after throwing grenade.
};

Schedule_t slShockTrooperRangeAttack2[] =
{
	{tlShockTrooperRangeAttack2,
		ARRAYSIZE(tlShockTrooperRangeAttack2),
		0,
		0,
		"SHOCK_RANGE_ATTACK"},
};


//=========================================================
// repel
//=========================================================
Task_t tlShockTrooperRepel[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_FACE_IDEAL, (float)0},
	{TASK_PLAY_SEQUENCE, (float)ACT_GLIDE},
};

Schedule_t slShockTrooperRepel[] =
{
	{tlShockTrooperRepel,
		ARRAYSIZE(tlShockTrooperRepel),
		bits_COND_SEE_ENEMY |
			bits_COND_NEW_ENEMY |
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER |
			bits_SOUND_COMBAT |
			bits_SOUND_PLAYER,
		"SHOCK_REPEL"},
};


//=========================================================
// repel
//=========================================================
Task_t tlShockTrooperRepelAttack[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_FACE_ENEMY, (float)0},
	{TASK_PLAY_SEQUENCE, (float)ACT_FLY},
};

Schedule_t slShockTrooperRepelAttack[] =
{
	{tlShockTrooperRepelAttack,
		ARRAYSIZE(tlShockTrooperRepelAttack),
		bits_COND_ENEMY_OCCLUDED,
		0,
		"SHOCK_REPEL_ATTACK"},
};

//=========================================================
// repel land
//=========================================================
Task_t tlShockTrooperRepelLand[] =
{
	{TASK_STOP_MOVING, (float)0},
	{TASK_PLAY_SEQUENCE, (float)ACT_LAND},
	{TASK_GET_PATH_TO_LASTPOSITION, (float)0},
	{TASK_RUN_PATH, (float)0},
	{TASK_WAIT_FOR_MOVEMENT, (float)0},
	{TASK_CLEAR_LASTPOSITION, (float)0},
};

Schedule_t slShockTrooperRepelLand[] =
{
	{tlShockTrooperRepelLand,
		ARRAYSIZE(tlShockTrooperRepelLand),
		bits_COND_SEE_ENEMY |
			bits_COND_NEW_ENEMY |
			bits_COND_LIGHT_DAMAGE |
			bits_COND_HEAVY_DAMAGE |
			bits_COND_HEAR_SOUND,

		bits_SOUND_DANGER |
			bits_SOUND_COMBAT |
			bits_SOUND_PLAYER,
		"SHOCK_REPEL_LAND"
	},
};


DEFINE_CUSTOM_SCHEDULES(CShockTrooper) {
	slShockTrooperFail,
	slShockTrooperCombatFail,
	slShockTrooperVictoryDance,
	slShockTrooperEstablishLineOfFire,
	slShockTrooperFoundEnemy,
	slShockTrooperCombatFace,
	slShockTrooperSignalSuppress,
	slShockTrooperSuppress,
	slShockTrooperWaitInCover,
	slShockTrooperTakeCover,
	slShockTrooperGrenadeCover,
	slShockTrooperTossGrenadeCover,
	slShockTrooperTakeCoverFromBestSound,
	slShockTrooperHideReload,
	slShockTrooperSweep,
	slShockTrooperRangeAttack1A,
	slShockTrooperRangeAttack1B,
	slShockTrooperRangeAttack2,
	slShockTrooperRepel,
	slShockTrooperRepelAttack,
	slShockTrooperRepelLand,
};

IMPLEMENT_CUSTOM_SCHEDULES(CShockTrooper, CTalkSquadMonster)

//=========================================================
// SetActivity
//=========================================================
void CShockTrooper::SetActivity(Activity NewActivity)
{
	int iSequence = ACTIVITY_NOT_AVAILABLE;

	switch (NewActivity)
	{
	case ACT_RANGE_ATTACK1:
		// grunt is either shooting standing or shooting crouched
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
	case ACT_RANGE_ATTACK2:
		// grunt is going to a secondary long range attack. This may be a thrown
		// grenade or fired grenade, we must determine which and pick proper sequence
		// get toss anim
		iSequence = LookupSequence("throwgrenade");
		break;
	case ACT_RUN:
		if (pev->health <= HGRUNT_LIMP_HEALTH)
		{
			// limp!
			iSequence = LookupActivity(ACT_RUN_HURT);
		}
		else
		{
			iSequence = LookupActivity(NewActivity);
		}
		break;
	case ACT_WALK:
		if (pev->health <= HGRUNT_LIMP_HEALTH)
		{
			// limp!
			iSequence = LookupActivity(ACT_WALK_HURT);
		}
		else
		{
			iSequence = LookupActivity(NewActivity);
		}
		break;
	case ACT_IDLE:
		if (m_MonsterState == MONSTERSTATE_COMBAT)
		{
			NewActivity = ACT_IDLE_ANGRY;
		}
		iSequence = LookupActivity(NewActivity);
		break;
	default:
		iSequence = LookupActivity(NewActivity);
		break;
	}

	m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

	// Set to the desired anim, or default anim if the desired is not present
	if (iSequence > ACTIVITY_NOT_AVAILABLE)
	{
		if (pev->sequence != iSequence || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		pev->sequence = iSequence; // Set to the reset anim (if it's there)
		ResetSequenceInfo();
		SetYawSpeed();
	}
	else
	{
		// Not available try to get default anim
		ALERT(at_console, "%s has no sequence for act:%d\n", STRING(pev->classname), NewActivity);
		pev->sequence = 0; // Set to the reset anim (if it's there)
	}
}

//=========================================================
// Get Schedule!
//=========================================================
Schedule_t* CShockTrooper::GetSchedule()
{

	// clear old sentence
	m_iSentence = ShockTrooper_SENT_NONE;

	// flying? If PRONE, barnacle has me. IF not, it's assumed I am rapelling.
	if ((pev->movetype == MOVETYPE_FLY || pev->movetype == MOVETYPE_BOUNCE) && m_MonsterState != MONSTERSTATE_PRONE)
	{
		if ((pev->flags & FL_ONGROUND) != 0)
		{
			// just landed
			pev->movetype = MOVETYPE_STEP;
			pev->gravity = 0;
			pev->friction = 0;
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
			if ((pSound->m_iType & bits_SOUND_DANGER) != 0)
			{
				// dangerous sound nearby!

				//!!!KELLY - currently, this is the grunt's signal that a grenade has landed nearby,
				// and the grunt should find cover from the blast
				// good place for "SHIT!" or some other colorful verbal indicator of dismay.
				// It's not safe to play a verbal order here "Scatter", etc cause
				// this may only affect a single individual in a squad.

				if (FOkToSpeak())
				{
					//SENTENCEG_PlayRndSz(ENT(pev), "ST_GREN", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_gren0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
	switch (m_MonsterState)
	{
	case MONSTERSTATE_COMBAT:
	{
		// dead enemy
		if (HasConditions(bits_COND_ENEMY_DEAD))
		{
			// call base class, all code to handle dead enemies is centralized there.
			return CBaseMonster::GetSchedule();
		}

		// new enemy
		if (HasConditions(bits_COND_NEW_ENEMY))
		{
			if (InSquad())
			{
				MySquadLeader()->m_fEnemyEluded = false;

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
					if (FOkToSpeak()) // && RANDOM_LONG(0,1))
					{
						if ((m_hEnemy != NULL) && m_hEnemy->IsPlayer())
							// player
							//SENTENCEG_PlayRndSz(ENT(pev), "ST_ALERT", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pAlertSounds), ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
						else if ((m_hEnemy != NULL) &&
							(m_hEnemy->Classify() != CLASS_PLAYER_ALLY) &&
							(m_hEnemy->Classify() != CLASS_HUMAN_PASSIVE) &&
							(m_hEnemy->Classify() != CLASS_MACHINE))
							// monster
							//SENTENCEG_PlayRndSz(ENT(pev), "ST_MONST", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
							EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_monst0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
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
		}
		// no ammo
		else if (HasConditions(bits_COND_NO_AMMO_LOADED))
		{
			//!!!KELLY - this individual just realized he's out of bullet ammo.
			// He's going to try to find cover to run to and reload, but rarely, if
			// none is available, he'll drop and reload in the open here.
			return GetScheduleOfType(SCHED_GRUNT_COVER_AND_RELOAD);
		}

		// damaged just a little
		else if (HasConditions(bits_COND_LIGHT_DAMAGE))
		{
			// if hurt:
			// 90% chance of taking cover
			// 10% chance of flinch.
			int iPercent = RANDOM_LONG(0, 99);

			if (iPercent <= 90 && m_hEnemy != NULL)
			{
				// only try to take cover if we actually have an enemy!

				//!!!KELLY - this grunt was hit and is going to run to cover.
				if (FOkToSpeak()) // && RANDOM_LONG(0,1))
				{
					//SENTENCEG_PlayRndSz( ENT(pev), "ST_COVER", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					m_iSentence = ShockTrooper_SENT_COVER;
					//JustSpoke();
				}
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
			}
			else
			{
				return GetScheduleOfType(SCHED_SMALL_FLINCH);
			}
		}
		// can kick
		else if (HasConditions(bits_COND_CAN_MELEE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_MELEE_ATTACK1);
		}
		// can shoot
		else if (HasConditions(bits_COND_CAN_RANGE_ATTACK1))
		{
			if (InSquad())
			{
				// if the enemy has eluded the squad and a squad member has just located the enemy
				// and the enemy does not see the squad member, issue a call to the squad to waste a
				// little time and give the player a chance to turn.
				if (MySquadLeader()->m_fEnemyEluded && !HasConditions(bits_COND_ENEMY_FACING_ME))
				{
					MySquadLeader()->m_fEnemyEluded = false;
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
				// hide!
				return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
			}
		}
		// can't see enemy
		else if (HasConditions(bits_COND_ENEMY_OCCLUDED))
		{
			if (HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
			{
				//!!!KELLY - this grunt is about to throw or fire a grenade at the player. Great place for "fire in the hole"  "frag out" etc
				if (FOkToSpeak())
				{
					//SENTENCEG_PlayRndSz(ENT(pev), "ST_THROW", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_throw0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return GetScheduleOfType(SCHED_RANGE_ATTACK2);
			}
			else if (OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
			{
				//!!!KELLY - grunt cannot see the enemy and has just decided to
				// charge the enemy's position.
				if (FOkToSpeak()) // && RANDOM_LONG(0,1))
				{
					//SENTENCEG_PlayRndSz( ENT(pev), "ST_CHARGE", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					m_iSentence = ShockTrooper_SENT_CHARGE;
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
					//SENTENCEG_PlayRndSz(ENT(pev), "ST_TAUNT", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_taunt0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return GetScheduleOfType(SCHED_STANDOFF);
			}
		}

		if (HasConditions(bits_COND_SEE_ENEMY) && !HasConditions(bits_COND_CAN_RANGE_ATTACK1))
		{
			return GetScheduleOfType(SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE);
		}
	}
	default:
		break;
	}

	// no special cases here, call the base class
	return CTalkSquadMonster::GetSchedule();
}

//=========================================================
//=========================================================
Schedule_t* CShockTrooper::GetScheduleOfType(int Type)
{
	switch (Type)
	{
	case SCHED_TAKE_COVER_FROM_ENEMY:
	{
		if (InSquad())
		{
			if (gSkillData.sk_hgrunt_gcover && HasConditions(bits_COND_CAN_RANGE_ATTACK2) && OccupySlot(bits_SLOTS_HGRUNT_GRENADE))
			{
				if (FOkToSpeak())
				{
					//SENTENCEG_PlayRndSz(ENT(pev), "ST_THROW", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "shocktrooper/st_throw0.wav", ShockTrooper_SENTENCE_VOLUME, GRUNT_ATTN, 0, m_voicePitch);
					JustSpoke();
				}
				return slShockTrooperTossGrenadeCover;
			}
			else
			{
				return &slShockTrooperTakeCover[0];
			}
		}
		else
		{
			if (RANDOM_LONG(0, 1))
			{
				return &slShockTrooperTakeCover[0];
			}
			else
			{
				return &slShockTrooperGrenadeCover[0];
			}
		}
	}
	case SCHED_TAKE_COVER_FROM_BEST_SOUND:
	{
		return &slShockTrooperTakeCoverFromBestSound[0];
	}
	case SCHED_GRUNT_TAKECOVER_FAILED:
	{
		if (HasConditions(bits_COND_CAN_RANGE_ATTACK1) && OccupySlot(bits_SLOTS_HGRUNT_ENGAGE))
		{
			return GetScheduleOfType(SCHED_RANGE_ATTACK1);
		}

		return GetScheduleOfType(SCHED_FAIL);
	}
	break;
	case SCHED_GRUNT_ELOF_FAIL:
	{
		// human grunt is unable to move to a position that allows him to attack the enemy.
		return GetScheduleOfType(SCHED_TAKE_COVER_FROM_ENEMY);
	}
	break;
	case SCHED_GRUNT_ESTABLISH_LINE_OF_FIRE:
	{
		return &slShockTrooperEstablishLineOfFire[0];
	}
	break;
	case SCHED_RANGE_ATTACK1:
	{
		// randomly stand or crouch
		if (RANDOM_LONG(0, 9) == 0)
			m_fStanding = RANDOM_LONG(0, 1);

		if (m_fStanding)
			return &slShockTrooperRangeAttack1B[0];
		else
			return &slShockTrooperRangeAttack1A[0];
	}
	case SCHED_RANGE_ATTACK2:
	{
		return &slShockTrooperRangeAttack2[0];
	}
	case SCHED_COMBAT_FACE:
	{
		return &slShockTrooperCombatFace[0];
	}
	case SCHED_GRUNT_WAIT_FACE_ENEMY:
	{
		return &slShockTrooperWaitInCover[0];
	}
	case SCHED_GRUNT_SWEEP:
	{
		return &slShockTrooperSweep[0];
	}
	case SCHED_GRUNT_COVER_AND_RELOAD:
	{
		return &slShockTrooperHideReload[0];
	}
	case SCHED_GRUNT_FOUND_ENEMY:
	{
		return &slShockTrooperFoundEnemy[0];
	}
	case SCHED_VICTORY_DANCE:
	{
		if (InSquad())
		{
			if (!IsLeader())
			{
				return &slShockTrooperFail[0];
			}
		}

		return &slShockTrooperVictoryDance[0];
	}
	case SCHED_GRUNT_SUPPRESS:
	{
		if (m_hEnemy->IsPlayer() && m_fFirstEncounter)
		{
			m_fFirstEncounter = false; // after first encounter, leader won't issue handsigns anymore when he has a new enemy
			return &slShockTrooperSignalSuppress[0];
		}
		else
		{
			return &slShockTrooperSuppress[0];
		}
	}
	case SCHED_FAIL:
	{
		if (m_hEnemy != NULL)
		{
			// grunt has an enemy, so pick a different default fail schedule most likely to help recover.
			return &slShockTrooperCombatFail[0];
		}

		return &slShockTrooperFail[0];
	}
	case SCHED_GRUNT_REPEL:
	{
		if (pev->velocity.z > -128)
			pev->velocity.z -= 32;
		return &slShockTrooperRepel[0];
	}
	case SCHED_GRUNT_REPEL_ATTACK:
	{
		if (pev->velocity.z > -128)
			pev->velocity.z -= 32;
		return &slShockTrooperRepelAttack[0];
	}
	case SCHED_GRUNT_REPEL_LAND:
	{
		return &slShockTrooperRepelLand[0];
	}
	default:
	{
		return CTalkSquadMonster::GetScheduleOfType(Type);
	}
	}
}

void CShockTrooper::MonsterThink()
{
	if (gpGlobals->time - m_flLastChargeTime >= gSkillData.sk_shocktrooper_rechargespeed)
	{
		if (m_cAmmoLoaded < m_cClipSize)
		{
			m_cAmmoLoaded = m_cClipSize;
			m_flLastChargeTime = gpGlobals->time;
			ALERT(at_aiconsole, "Shocktrooper Reload: %d\n", m_cAmmoLoaded);
		}
	}

	const auto lastBlink = gpGlobals->time - m_flLastBlinkTime;

	if (lastBlink >= 3.0)
	{
		if (lastBlink >= RANDOM_FLOAT(3.0, 7.0))
		{
			pev->skin = m_skinBase + 1;

			m_flLastBlinkInterval = gpGlobals->time;
			m_flLastBlinkTime = gpGlobals->time;
		}
	}

	if (pev->skin > m_skinBase)
	{
		if (gpGlobals->time - m_flLastBlinkInterval >= 0.1)
		{
			if (pev->skin == m_skinBase + 3)
				pev->skin = m_skinBase;
			else
				++pev->skin;

			m_flLastBlinkInterval = gpGlobals->time;
		}
	}

	CBaseMonster::MonsterThink();
}

//=========================================================
// CShockTrooperRepel - when triggered, spawns a monster_shocktrooper
// repelling down a line.
//=========================================================

class CShockTrooperRepel : public CBaseMonster
{
public:
	void Spawn() override;
	void Precache() override;
	void EXPORT RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int m_iSpriteTexture; // Don't save, precache
};

LINK_ENTITY_TO_CLASS(monster_shocktrooper_repel, CShockTrooperRepel)

void CShockTrooperRepel::Spawn()
{
	Precache();
	pev->solid = SOLID_NOT;

	SetUse(&CShockTrooperRepel::RepelUse);
}

void CShockTrooperRepel::Precache()
{
	CBaseMonster::Precache();
	UTIL_PrecacheOther("monster_shocktrooper");
	m_iSpriteTexture = PRECACHE_MODEL("sprites/rope.spr");
}

void CShockTrooperRepel::RepelUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -4096.0), dont_ignore_monsters, ENT(pev), &tr);
	/*
	if ( tr.pHit && Instance( tr.pHit )->pev->solid != SOLID_BSP)
		return NULL;
	*/

	CBaseEntity* pEntity = Create("monster_shocktrooper", pev->origin, pev->angles);
	CBaseMonster* pGrunt = pEntity->MyMonsterPointer();
	
	// FLY movetype but with client interpolation
	pGrunt->pev->movetype = MOVETYPE_BOUNCE;
	pGrunt->pev->gravity = FLT_MIN;
	pGrunt->pev->friction = 1.0f;

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



//=========================================================
// DEAD SHOCKTROOPER PROP
//=========================================================
class CDeadShockTrooper : public CBaseMonster
{
public:
	void Spawn() override;
	int Classify() override { return CBaseMonster::Classify(CLASS_HUMAN_MILITARY); }

	void KeyValue(KeyValueData* pkvd) override;

	int m_iPose; // which sequence to display	-- temporary, don't need to save
	static const char* m_szPoses[3];
};

const char* CDeadShockTrooper::m_szPoses[] = { "deadstomach", "deadside", "deadsitting" };

void CDeadShockTrooper::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(monster_ShockTrooper_dead, CDeadShockTrooper)

//=========================================================
// ********** DeadHGrunt SPAWN **********
//=========================================================
void CDeadShockTrooper::Spawn()
{
	m_defaultModel = "models/strooper.mdl";
	PRECACHE_MODEL(GetModel());
	InitModel();

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	m_bloodColor = BLOOD_COLOR_GREEN;

	pev->sequence = LookupSequence(m_szPoses[m_iPose]);

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead ShockTrooper with bad pose\n");
	}

	// Corpses have less health
	pev->health = 8;

	MonsterInitDead();
}
