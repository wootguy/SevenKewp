#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "schedule.h"
#include "animation.h"
#include "CSprite.h"
#include "weapons.h"
#include "decals.h"

// TODO:
// eat corpse only if can route to it
// range attack when cant reach target
// override idle when attacked
// override route to node if there's a direct route to the player, or player no longer near the node
// attack crouching players or players on head
// fade out corpse
// check fov

#define EVENT_ATTACK1_LEFT 1
#define EVENT_ATTACK1_RIGHT 2
#define EVENT_GRAB_BLOOD 3
#define EVENT_THROW_BLOOD 4
#define EVENT_ATTACK2_SWING0 19
#define EVENT_ATTACK2_SWING1 20
#define EVENT_ATTACK2_SWING2 21
#define EVENT_ATTACK2_SWING3 22
#define EVENT_PLAY_SOUND 1011

#define MELEE_ATTACK1_DISTANCE 80 
#define MELEE_ATTACK2_DISTANCE 56 // the one where it tries to eat you
#define MELEE_CHASE_DISTANCE 600 // don't waste time with ranged attack within this distance

#define MELEE_ATTACK1_SEQUENCE_OFFSET 0
#define MELEE_ATTACK2_SEQUENCE_OFFSET 1 // second ATTACK1 sequence in the model should be gonna-eat-you one

#define GONOME_SPIT_SPRITE "sprites/blood_chnk.spr" 

#define GRAB_BLOOD_SOUND "barnacle/bcl_chew2.wav" // this is new in sven co-op

static int iGonomeSpitSprite;

class CGonome : public CBaseMonster
{
public:
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	int Classify ( void );
	const char* DisplayName();
	void HandleAnimEvent( MonsterEvent_t *pEvent );
	BOOL CheckMeleeAttack1(float flDot, float flDist);
	BOOL CheckRangeAttack1(float flDot, float flDist);
	int LookupActivity(int activity);
	void Killed(entvars_t* pevAttacker, int iGib);
	Schedule_t* GetScheduleOfType(int Type);
	void MonsterThink(void);
	const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	void SetObjectCollisionBox(void)
	{
		pev->absmin = pev->origin + Vector(-24, -24, 0);
		pev->absmax = pev->origin + Vector(24, 24, 88);
	}

	void PainSound(void);
	void AlertSound(void);
	void IdleSound(void);

private:
	float m_rangeAttackCooldown; // next time a range attack can be considered
	float m_nextBloodSound; // next time the grabbing blood sound should be played (should really be an animation event)
	EHANDLE m_hHandBlood;

	static const char* pAttackHitSounds[];
	static const char* pAttackMissSounds[];
	static const char* pIdleSounds[];
	static const char* pPainSounds[];
	static const char* pDieSounds[];
	static const char* pEventSounds[];
};

class CGonomeSpit : public CBaseEntity
{
public:
	void Spawn(void);

	static void Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity);
	void Touch(CBaseEntity* pOther);
	void EXPORT Animate(void);

	int  m_maxFrame;
};


LINK_ENTITY_TO_CLASS(monster_gonome, CGonome);
LINK_ENTITY_TO_CLASS(gonomespit, CGonomeSpit);

const char* CGonome::pAttackHitSounds[] =
{
	"zombie/claw_strike1.wav",
	"zombie/claw_strike2.wav",
	"zombie/claw_strike3.wav",
};

const char* CGonome::pAttackMissSounds[] =
{
	"zombie/claw_miss1.wav",
	"zombie/claw_miss2.wav",
};

const char* CGonome::pIdleSounds[] =
{
	MOD_SND_FOLDER "gonome/gonome_idle1.wav",
	MOD_SND_FOLDER "gonome/gonome_idle2.wav",
	MOD_SND_FOLDER "gonome/gonome_idle3.wav"
};

const char* CGonome::pPainSounds[] =
{
	MOD_SND_FOLDER "gonome/gonome_pain1.wav",
	MOD_SND_FOLDER "gonome/gonome_pain2.wav",
	MOD_SND_FOLDER "gonome/gonome_pain3.wav",
	MOD_SND_FOLDER "gonome/gonome_pain4.wav"
};

const char* CGonome::pDieSounds[] =
{
	MOD_SND_FOLDER "gonome/gonome_death2.wav",
	MOD_SND_FOLDER "gonome/gonome_death3.wav",
	MOD_SND_FOLDER "gonome/gonome_death4.wav",
};

const char* CGonome::pEventSounds[] =
{
	MOD_SND_FOLDER "gonome/gonome_melee1.wav",
	MOD_SND_FOLDER "gonome/gonome_melee2.wav",
	MOD_SND_FOLDER "gonome/gonome_eat.wav",

	// not actually event sounds but wtv
	"bullchicken/bc_acid1.wav",
	"bullchicken/bc_spithit1.wav",
	"bullchicken/bc_spithit2.wav",
};


int	CGonome:: Classify ( void )
{
	return CBaseMonster::Classify(CLASS_ALIEN_MONSTER);
}

const char* CGonome::DisplayName() {
	return m_displayName ? CBaseMonster::DisplayName() : "Gonome";
}

void CGonome:: SetYawSpeed ( void )
{
	int ys;

	switch ( m_Activity )
	{
	case ACT_IDLE:
	default:
		ys = 180;
	}

	pev->yaw_speed = ys * gSkillData.sk_yawspeed_mult;
}

void CGonome:: HandleAnimEvent( MonsterEvent_t *pEvent )
{
	switch( pEvent->event )
	{
	case EVENT_PLAY_SOUND:
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pEvent->options, 1.0, ATTN_NORM, 0, 100);
		break;
	case EVENT_ATTACK1_LEFT:
	case EVENT_ATTACK1_RIGHT:
	case EVENT_ATTACK2_SWING0:
	case EVENT_ATTACK2_SWING1:
	case EVENT_ATTACK2_SWING2:
	case EVENT_ATTACK2_SWING3:
	{
		bool isAttack2 = pEvent->event >= EVENT_ATTACK2_SWING0;
		bool isLeftSwing = pEvent->event == EVENT_ATTACK1_LEFT;
		float attackDistance = isAttack2 ? MELEE_ATTACK2_DISTANCE : MELEE_ATTACK1_DISTANCE;
		float damage = isAttack2 ? gSkillData.sk_gonome_dmg_one_bite : gSkillData.sk_gonome_dmg_one_slash;
		CBaseEntity* pHurt = CheckTraceHullAttack(attackDistance, damage, DMG_SLASH);

		if (pHurt)
		{
			if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
			{
				if (isAttack2) {
					pHurt->pev->punchangle.x = 18;
				} else {
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->punchangle.z = isLeftSwing ? 18 : -18;
					pHurt->pev->velocity = pHurt->pev->velocity + gpGlobals->v_right * (isLeftSwing ? -100 : 100);
				}
			}
			// Play a random attack hit sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackHitSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		}
		else // Play a random attack miss sound
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(pAttackMissSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
		break;
	}
	case EVENT_GRAB_BLOOD:
	{
		CSprite* handBlood = (CSprite*)m_hHandBlood.GetEntity();
		if (handBlood)
			handBlood->TurnOn();

		Vector handOrigin, handAngles;
		GetAttachment(0, handOrigin, handAngles);

		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, handOrigin);
			WRITE_BYTE(TE_BLOODSPRITE);
			WRITE_COORD(handOrigin.x);
			WRITE_COORD(handOrigin.y);
			WRITE_COORD(handOrigin.z);
			WRITE_SHORT(g_sModelIndexBloodSpray);
			WRITE_SHORT(g_sModelIndexBloodDrop);
			WRITE_BYTE(BLOOD_COLOR_RED);
			WRITE_BYTE(4); // size
		MESSAGE_END();

		break;
	}
	case EVENT_THROW_BLOOD:
	{
		CSprite* handBlood = (CSprite*)m_hHandBlood.GetEntity();
		if (handBlood)
			handBlood->TurnOff();

		Vector handOrigin, handAngles;
		GetAttachment(0, handOrigin, handAngles);

		UTIL_MakeVectors(pev->angles);
		Vector vecThrowDir;

		if (m_hEnemy) {
			vecThrowDir = ((m_hEnemy->pev->origin + m_hEnemy->pev->view_ofs) - handOrigin).Normalize();
		}
		else {
			vecThrowDir = gpGlobals->v_forward;
		}

		vecThrowDir.x += RANDOM_FLOAT(-0.01, 0.01);
		vecThrowDir.y += RANDOM_FLOAT(-0.01, 0.01);
		vecThrowDir.z += RANDOM_FLOAT(-0.01, 0.01);
		
		CGonomeSpit::Shoot(pev, handOrigin, vecThrowDir * 1200);

		STOP_SOUND(ENT(pev), CHAN_WEAPON, GRAB_BLOOD_SOUND);
		m_rangeAttackCooldown = gpGlobals->time + RANDOM_FLOAT(4, 6);
		break;
	}
	default:
		CBaseMonster::HandleAnimEvent( pEvent );
		break;
	}
}

void CGonome::Spawn()
{
	Precache( );

	InitModel();
	SetSize(Vector( -16, -16, 0 ), Vector( 16, 16, 72 ) );

	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_STEP;
	m_bloodColor		= BLOOD_COLOR_YELLOW;
	pev->view_ofs		= Vector ( 0, 0, 0 );// position of the eyes relative to monster's origin.
	m_flFieldOfView		= 0.0;// indicates the width of this monster's forward view cone ( as a dotproduct result )
	m_MonsterState		= MONSTERSTATE_NONE;

	MonsterInit();

	CSprite* handBlood = CSprite::SpriteCreate(GONOME_SPIT_SPRITE, pev->origin, TRUE);
	handBlood->SetScale(0.3f);
	handBlood->SetTransparency(kRenderTransAlpha, 255, 255, 255, 255, 0);
	handBlood->SetAttachment(edict(), 1);
	handBlood->TurnOff();
	m_hHandBlood = handBlood;
}

void CGonome::Precache()
{
	CBaseMonster::Precache();

	m_defaultModel = "models/gonome.mdl";
	PRECACHE_MODEL(GetModel());
	iGonomeSpitSprite = PRECACHE_MODEL(GONOME_SPIT_SPRITE);

	PRECACHE_SOUND_ARRAY(pAttackHitSounds);
	PRECACHE_SOUND_ARRAY(pAttackMissSounds);
	PRECACHE_SOUND_ARRAY(pIdleSounds);
	PRECACHE_SOUND_ARRAY(pPainSounds);
	PRECACHE_SOUND_ARRAY(pDieSounds);

	// not affected by mp_soundvariety (but should be)
	for (int i = 0; i < ARRAYSIZE(pEventSounds); i++) \
		PRECACHE_SOUND((char*)pEventSounds[i]);

	PRECACHE_SOUND(GRAB_BLOOD_SOUND);
}

BOOL CGonome::CheckMeleeAttack1(float flDot, float flDist)
{
	if (flDist <= MELEE_ATTACK1_DISTANCE) {
		if (flDist <= MELEE_ATTACK2_DISTANCE) {
			SetConditions(bits_COND_CAN_MELEE_ATTACK2);
		}

		return TRUE;
	}

	return FALSE;
}

BOOL CGonome::CheckRangeAttack1(float flDot, float flDist)
{
	if (flDist > MELEE_CHASE_DISTANCE && m_rangeAttackCooldown < gpGlobals->time)
	{
		return TRUE;
	}
	return FALSE;
}

void CGonome::Killed(entvars_t* pevAttacker, int iGib)
{
	UTIL_Remove(m_hHandBlood);
	m_hHandBlood = NULL;

	if (!ShouldGibMonster(iGib))
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pDieSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(0, 9));

	CBaseMonster::Killed(pevAttacker, iGib);
}

Schedule_t* CGonome::GetScheduleOfType(int Type) {
	m_nextBloodSound = 0;

	CSprite* handBlood = (CSprite*)m_hHandBlood.GetEntity();
	if (handBlood)
		handBlood->TurnOff();

	if (Type == SCHED_RANGE_ATTACK1) {
		// starting to grab blood
		m_nextBloodSound = gpGlobals->time + 0.3;
	}

	return CBaseMonster::GetScheduleOfType(Type);
}

void CGonome::MonsterThink(void) {
	if (m_nextBloodSound && m_nextBloodSound <= gpGlobals->time) {
		m_nextBloodSound = 0;
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, GRAB_BLOOD_SOUND, 0.5, ATTN_NORM, 0, 90);
	}

	CBaseMonster::MonsterThink();
}

// HACK
// Both types of melee attack animations are tagged with ATTACK1,
// so sequence selection is hardcoded here. The second attack 
// animation probably should have been tagged with ATTACK2.
int CGonome::LookupActivity(int activity)
{
	ASSERT(activity != 0);
	void* pmodel = GET_MODEL_PTR(ENT(pev));
	int offset = 0;

	switch(activity) {
	case ACT_MELEE_ATTACK1:
		offset = HasConditions(bits_COND_CAN_MELEE_ATTACK2) ? MELEE_ATTACK2_SEQUENCE_OFFSET : MELEE_ATTACK1_SEQUENCE_OFFSET;
		return ::LookupActivityWithOffset(pmodel, pev, activity, offset);
	default:
		return ::LookupActivity(pmodel, pev, activity);
	}
}

void CGonome::PainSound(void)
{
	int pitch = 100 + RANDOM_LONG(0, 9);

	if (RANDOM_LONG(0, 5) < 2)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pPainSounds), 1.0, ATTN_NORM, 0, pitch);
}

void CGonome::AlertSound(void)
{
	int pitch = 100 + RANDOM_LONG(0, 9);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, pPainSounds[RANDOM_LONG(0, ARRAYSIZE(pPainSounds) - 1)], 1.0, ATTN_NORM, 0, pitch);
}

void CGonome::IdleSound(void)
{
	int pitch = 100 + RANDOM_LONG(-5, 5);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(pIdleSounds), 1.0, ATTN_NORM, 0, pitch);
}

//
// Gonome spit
//

void CGonomeSpit::Spawn(void)
{
	pev->movetype = MOVETYPE_FLY;
	pev->classname = MAKE_STRING("gonomespit");

	pev->solid = SOLID_BBOX;
	pev->rendermode = kRenderTransAlpha;
	pev->renderamt = 255;

	SET_MODEL(ENT(pev), GONOME_SPIT_SPRITE);
	pev->frame = 0;
	pev->scale = 0.3f;

	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;
}

void CGonomeSpit::Animate(void)
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->frame++)
	{
		if (pev->frame > m_maxFrame)
		{
			pev->frame = 0;
		}
	}
}

void CGonomeSpit::Shoot(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity)
{
	CGonomeSpit* pSpit = GetClassPtr((CGonomeSpit*)NULL);
	pSpit->Spawn();

	UTIL_SetOrigin(pSpit->pev, vecStart);
	pSpit->pev->velocity = vecVelocity;
	pSpit->pev->owner = ENT(pevOwner);

	pSpit->SetThink(&CGonomeSpit::Animate);
	pSpit->pev->nextthink = gpGlobals->time + 0.1;
}

void CGonomeSpit::Touch(CBaseEntity* pOther)
{
	TraceResult tr;
	int		iPitch;

	// splat sound
	iPitch = RANDOM_FLOAT(90, 110);

	EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "bullchicken/bc_acid1.wav", 1, ATTN_NORM, 0, iPitch);

	switch (RANDOM_LONG(0, 1))
	{
	case 0:
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit1.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	case 1:
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, "bullchicken/bc_spithit2.wav", 1, ATTN_NORM, 0, iPitch);
		break;
	}

	if (!pOther->pev->takedamage)
	{
		// make a splat on the wall
		UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 10, dont_ignore_monsters, ENT(pev), &tr);
		UTIL_DecalTrace(&tr, DECAL_BLOOD1 + RANDOM_LONG(0, 5));
	}

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_BLOODSPRITE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(g_sModelIndexBloodSpray);
		WRITE_SHORT(g_sModelIndexBloodDrop);
		WRITE_BYTE(BLOOD_COLOR_RED);
		WRITE_BYTE(10);	// size
	MESSAGE_END();

	pOther->TakeDamage(pev, pev, gSkillData.sk_gonome_dmg_guts, DMG_GENERIC);

	SetThink(&CGonomeSpit::SUB_Remove);
	pev->nextthink = gpGlobals->time;
}
