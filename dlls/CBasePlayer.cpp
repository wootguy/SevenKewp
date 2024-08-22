	/***
*
*	Copyright (c) 1996-2001, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

===== player.cpp ========================================================

  functions dealing with the player

*/

#include "extdll.h"
#include "util.h"

#include "cbase.h"
#include "CBasePlayer.h"
#include "trains.h"
#include "nodes.h"
#include "weapons.h"
#include "env/CSoundEnt.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"
#include "game.h"
#include "pm_shared.h"
#include "hltv.h"
#include "CFuncTrackTrain.h"
#include "user_messages.h"
#include "CSprayCan.h"
#include "CBloodSplat.h"
#include "CBaseDMStart.h"
#include "CAmbientGeneric.h"
#include "game.h"
#include "CWeaponBox.h"
#include "CBasePlayerWeapon.h"
#include "CItem.h"
#include "CTripmine.h"
#include "skill.h"
#include "CBreakable.h"

// #define DUCKFIX

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL		g_fGameOver;
extern DLL_GLOBAL	BOOL	g_fDrawLines;
int gEvilImpulse101;
extern DLL_GLOBAL int gDisplayTitle;
extern float g_flWeaponCheat;

BOOL gInitHUD = TRUE;

extern void CopyToBodyQue(entvars_t* pev);
extern void respawn(entvars_t *pev, BOOL fCopyCorpse);
extern Vector VecBModelOrigin(entvars_t *pevBModel );

// the world node graph
extern CGraph	WorldGraph;

#define TRAIN_ACTIVE	0x80 
#define TRAIN_NEW		0xc0
#define TRAIN_OFF		0x00
#define TRAIN_NEUTRAL	0x01
#define TRAIN_SLOW		0x02
#define TRAIN_MEDIUM	0x03
#define TRAIN_FAST		0x04 
#define TRAIN_BACK		0x05

#define	FLASH_DRAIN_TIME	 1.2 //100 units/3 minutes
#define	FLASH_CHARGE_TIME	 0.2 // 100 units/20 seconds  (seconds per unit)

#define CLIMB_SHAKE_FREQUENCY	22	// how many frames in between screen shakes when climbing
#define	MAX_CLIMB_SPEED			200	// fastest vertical climbing speed possible
#define	CLIMB_SPEED_DEC			15	// climbing deceleration rate
#define	CLIMB_PUNCH_X			-7  // how far to 'punch' client X axis when climbing
#define CLIMB_PUNCH_Z			7	// how far to 'punch' client Z axis when climbing

#define DOT_1DEGREE   0.9998476951564
#define DOT_2DEGREE   0.9993908270191
#define DOT_3DEGREE   0.9986295347546
#define DOT_4DEGREE   0.9975640502598
#define DOT_5DEGREE   0.9961946980917
#define DOT_6DEGREE   0.9945218953683
#define DOT_7DEGREE   0.9925461516413
#define DOT_8DEGREE   0.9902680687416
#define DOT_9DEGREE   0.9876883405951
#define DOT_10DEGREE  0.9848077530122
#define DOT_15DEGREE  0.9659258262891
#define DOT_20DEGREE  0.9396926207859
#define DOT_25DEGREE  0.9063077870367

#define ARMOR_RATIO	 0.2	// Armor Takes 80% of the damage
#define ARMOR_BONUS  0.5	// Each Point of Armor is work 1/x points of health

#define GEIGERDELAY 0.25
#define SUITUPDATETIME	3.5
#define AIRTIME	12		// lung full of air lasts this many seconds
#define SUITFIRSTUPDATETIME 0.1

#define	PLAYER_SEARCH_RADIUS	(float)64 // +use distance

// Global Savedata for player
TYPEDESCRIPTION	CBasePlayer::m_playerSaveData[] = 
{
	DEFINE_FIELD( CBasePlayer, m_flFlashLightTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_iFlashBattery, FIELD_INTEGER ),

	DEFINE_FIELD( CBasePlayer, m_afButtonLast, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_afButtonPressed, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_afButtonReleased, FIELD_INTEGER ),

	DEFINE_ARRAY( CBasePlayer, m_rgItems, FIELD_INTEGER, MAX_ITEMS ),
	DEFINE_FIELD( CBasePlayer, m_afPhysicsFlags, FIELD_INTEGER ),

	DEFINE_FIELD( CBasePlayer, m_flTimeStepSound, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flTimeWeaponIdle, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flSwimTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flDuckTime, FIELD_TIME ),
	DEFINE_FIELD( CBasePlayer, m_flWallJumpTime, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, m_flSuitUpdate, FIELD_TIME ),
	DEFINE_ARRAY( CBasePlayer, m_rgSuitPlayList, FIELD_INTEGER, CSUITPLAYLIST ),
	DEFINE_FIELD( CBasePlayer, m_iSuitPlayNext, FIELD_INTEGER ),
	DEFINE_ARRAY( CBasePlayer, m_rgiSuitNoRepeat, FIELD_INTEGER, CSUITNOREPEAT ),
	DEFINE_ARRAY( CBasePlayer, m_rgflSuitNoRepeatTime, FIELD_TIME, CSUITNOREPEAT ),
	DEFINE_FIELD( CBasePlayer, m_lastDamageAmount, FIELD_INTEGER ),

	DEFINE_ARRAY( CBasePlayer, m_rgpPlayerItems, FIELD_EHANDLE, MAX_ITEM_TYPES ),
	DEFINE_FIELD( CBasePlayer, m_pActiveItem, FIELD_EHANDLE ),
	DEFINE_FIELD( CBasePlayer, m_pLastItem, FIELD_EHANDLE ),
	
	DEFINE_ARRAY( CBasePlayer, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_SLOTS ),
	DEFINE_FIELD( CBasePlayer, m_idrowndmg, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_idrownrestored, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_tSneaking, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, m_iTrain, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_bitsHUDDamage, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_flFallVelocity, FIELD_FLOAT ),
	DEFINE_FIELD( CBasePlayer, m_iTargetVolume, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iWeaponVolume, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iExtraSoundTypes, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iWeaponFlash, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_fLongJump, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBasePlayer, m_fInitHUD, FIELD_BOOLEAN ),
	DEFINE_FIELD( CBasePlayer, m_tbdPrev, FIELD_TIME ),

	DEFINE_FIELD( CBasePlayer, m_pTank, FIELD_EHANDLE ),
	DEFINE_FIELD( CBasePlayer, m_iHideHUD, FIELD_INTEGER ),
	DEFINE_FIELD( CBasePlayer, m_iFOV, FIELD_INTEGER ),
	
	//DEFINE_FIELD( CBasePlayer, m_fDeadTime, FIELD_FLOAT ), // only used in multiplayer games
	//DEFINE_FIELD( CBasePlayer, m_fGameHUDInitialized, FIELD_INTEGER ), // only used in multiplayer games
	//DEFINE_FIELD( CBasePlayer, m_flStopExtraSoundTime, FIELD_TIME ),
	//DEFINE_FIELD( CBasePlayer, m_fKnownItem, FIELD_INTEGER ), // reset to zero on load
	//DEFINE_FIELD( CBasePlayer, m_iPlayerSound, FIELD_INTEGER ),	// Don't restore, set in Precache()
	//DEFINE_FIELD( CBasePlayer, m_pentSndLast, FIELD_EDICT ),	// Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_flSndRoomtype, FIELD_FLOAT ),	// Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_flSndRange, FIELD_FLOAT ),	// Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_fNewAmmo, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_flgeigerRange, FIELD_FLOAT ),	// Don't restore, reset in Precache()
	//DEFINE_FIELD( CBasePlayer, m_flgeigerDelay, FIELD_FLOAT ),	// Don't restore, reset in Precache()
	//DEFINE_FIELD( CBasePlayer, m_igeigerRangePrev, FIELD_FLOAT ),	// Don't restore, reset in Precache()
	//DEFINE_FIELD( CBasePlayer, m_iStepLeft, FIELD_INTEGER ), // Don't need to restore
	//DEFINE_ARRAY( CBasePlayer, m_szTextureName, FIELD_CHARACTER, CBTEXTURENAMEMAX ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_chTextureType, FIELD_CHARACTER ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_fNoPlayerSound, FIELD_BOOLEAN ), // Don't need to restore, debug
	//DEFINE_FIELD( CBasePlayer, m_iUpdateTime, FIELD_INTEGER ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_iClientHealth, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_iClientBattery, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_iClientHideHUD, FIELD_INTEGER ), // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_fWeapon, FIELD_BOOLEAN ),  // Don't restore, client needs reset
	//DEFINE_FIELD( CBasePlayer, m_nCustomSprayFrames, FIELD_INTEGER ), // Don't restore, depends on server message after spawning and only matters in multiplayer
	//DEFINE_FIELD( CBasePlayer, m_vecAutoAim, FIELD_VECTOR ), // Don't save/restore - this is recomputed
	//DEFINE_ARRAY( CBasePlayer, m_rgAmmoLast, FIELD_INTEGER, MAX_AMMO_SLOTS ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_fOnTarget, FIELD_BOOLEAN ), // Don't need to restore
	//DEFINE_FIELD( CBasePlayer, m_nCustomSprayFrames, FIELD_INTEGER ), // Don't need to restore
	
};


LINK_ENTITY_TO_CLASS( player, CBasePlayer );

/* 
 *
 */
Vector VecVelocityForDamage(float flDamage)
{
	Vector vec(RANDOM_FLOAT(-100,100), RANDOM_FLOAT(-100,100), RANDOM_FLOAT(200,300));

	if (flDamage > -50)
		vec = vec * 0.7;
	else if (flDamage > -200)
		vec = vec * 2;
	else
		vec = vec * 10;
	
	return vec;
}

#if 0 /*
static void ThrowGib(entvars_t *pev, char *szGibModel, float flDamage)
{
	edict_t *pentNew = CREATE_ENTITY();
	entvars_t *pevNew = VARS(pentNew);

	pevNew->origin = pev->origin;
	SET_MODEL(ENT(pevNew), szGibModel);
	UTIL_SetSize(pevNew, g_vecZero, g_vecZero);

	pevNew->velocity		= VecVelocityForDamage(flDamage);
	pevNew->movetype		= MOVETYPE_BOUNCE;
	pevNew->solid			= SOLID_NOT;
	pevNew->avelocity.x		= RANDOM_FLOAT(0,600);
	pevNew->avelocity.y		= RANDOM_FLOAT(0,600);
	pevNew->avelocity.z		= RANDOM_FLOAT(0,600);
	CHANGE_METHOD(ENT(pevNew), em_think, SUB_Remove);
	pevNew->ltime		= gpGlobals->time;
	pevNew->nextthink	= gpGlobals->time + RANDOM_FLOAT(10,20);
	pevNew->frame		= 0;
	pevNew->flags		= 0;
}
	
	
static void ThrowHead(entvars_t *pev, char *szGibModel, floatflDamage)
{
	SET_MODEL(ENT(pev), szGibModel);
	pev->frame			= 0;
	pev->nextthink		= -1;
	pev->movetype		= MOVETYPE_BOUNCE;
	pev->takedamage		= DAMAGE_NO;
	pev->solid			= SOLID_NOT;
	pev->view_ofs		= Vector(0,0,8);
	UTIL_SetSize(pev, Vector(-16,-16,0), Vector(16,16,56));
	pev->velocity		= VecVelocityForDamage(flDamage);
	pev->avelocity		= RANDOM_FLOAT(-1,1) * Vector(0,600,0);
	pev->origin.z -= 24;
	ClearBits(pev->flags, FL_ONGROUND);
}


*/ 
#endif

int TrainSpeed(int iSpeed, int iMax)
{
	float fSpeed, fMax;
	int iRet = 0;

	fMax = (float)iMax;
	fSpeed = iSpeed;

	fSpeed = fSpeed/fMax;

	if (iSpeed < 0)
		iRet = TRAIN_BACK;
	else if (iSpeed == 0)
		iRet = TRAIN_NEUTRAL;
	else if (fSpeed < 0.33)
		iRet = TRAIN_SLOW;
	else if (fSpeed < 0.66)
		iRet = TRAIN_MEDIUM;
	else
		iRet = TRAIN_FAST;

	return iRet;
}

void CBasePlayer :: DeathSound( void )
{
	// water death sounds
	/*
	if (pev->waterlevel == 3)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/h2odeath.wav", 1, ATTN_NONE);
		return;
	}
	*/

	// temporarily using pain sounds for death sounds
	// Note: disabled because they never worked. CHAN_VOICE is immediately overwritten with the HEV suit sound.
	// Change the sound channel to hear gordon's voice.
	/*
	switch (RANDOM_LONG(1,5)) 
	{
	case 1: 
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain5.wav", 1, ATTN_NORM);
		break;
	case 2: 
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain6.wav", 1, ATTN_NORM);
		break;
	case 3: 
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_pain7.wav", 1, ATTN_NORM);
		break;
	}
	*/

	// play one of the suit death alarms
	EMIT_GROUPNAME_SUIT(ENT(pev), "HEV_DEAD");
}

// override takehealth
// bitsDamageType indicates type of damage healed. 

int CBasePlayer :: TakeHealth( float flHealth, int bitsDamageType, float healthcap)
{
	return CBaseMonster :: TakeHealth (flHealth, bitsDamageType, healthcap);

}

Vector CBasePlayer :: GetGunPosition( )
{
//	UTIL_MakeVectors(pev->v_angle);
//	m_HackedGunPos = pev->view_ofs;
	Vector origin;
	
	origin = pev->origin + pev->view_ofs;

	return origin;
}

//=========================================================
// TraceAttack
//=========================================================
void CBasePlayer :: TraceAttack( entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if ( pev->takedamage )
	{
		m_LastHitGroup = ptr->iHitgroup;

		switch ( ptr->iHitgroup )
		{
		case HITGROUP_GENERIC:
			break;
		case HITGROUP_HEAD:
			if (bitsDamageType & (DMG_BULLET | DMG_CLUB)) {
				flDamage *= gSkillData.sk_player_head;
				EMIT_SOUND_DYN(edict(), CHAN_BODY, "player/bhit_helmet-1.wav", 1.0f, ATTN_STATIC, 0, RANDOM_LONG(90, 110));
				m_headshot = IsAlive(); // don't play effect while dying
				m_headshotDir = Vector(vecDir.x, vecDir.y, 0).Normalize();
			}
			else {
				// don't take headshot damage for things other than bullets and some melee attacks
				flDamage *= gSkillData.sk_player_chest;
			}
			break;
		case HITGROUP_CHEST:
			flDamage *= gSkillData.sk_player_chest;
			break;
		case HITGROUP_STOMACH:
			flDamage *= gSkillData.sk_player_stomach;
			break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			flDamage *= gSkillData.sk_player_arm;
			break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG:
			flDamage *= gSkillData.sk_player_leg;
			break;
		default:
			break;
		}

		if (bitsDamageType & DMG_BLOOD) {
			SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);// a little surface blood.
			TraceBleed( flDamage, vecDir, ptr, bitsDamageType );
		}

		AddMultiDamage( pevAttacker, this, flDamage, bitsDamageType );
	}
}

/*
	Take some damage.  
	NOTE: each call to TakeDamage with bitsDamageType set to a time-based damage
	type will cause the damage time countdown to be reset.  Thus the ongoing effects of poison, radiation
	etc are implemented with subsequent calls to TakeDamage using DMG_GENERIC.
*/
int CBasePlayer :: TakeDamage( entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType )
{
	// have suit diagnose the problem - ie: report damage type
	int bitsDamage = bitsDamageType;
	int ffound = TRUE;
	int fmajor;
	int fcritical;
	int fTookDamage;
	int ftrivial;
	float flRatio;
	float flBonus;
	float flHealthPrev = pev->health;

	flBonus = ARMOR_BONUS;
	flRatio = ARMOR_RATIO;

	if ( ( bitsDamageType & DMG_BLAST ) && g_pGameRules->IsMultiplayer() )
	{
		// blasts damage armor more.
		flBonus *= 2;
	}

	// Already dead
	if ( !IsAlive() )
		return 0;
	// go take the damage first

	
	CBaseEntity *pAttacker = CBaseEntity::Instance(pevAttacker);

	if ( !g_pGameRules->FPlayerCanTakeDamage( this, pAttacker ) )
	{
		// Refuse the damage
		return 0;
	}

	// keep track of amount of damage last sustained
	m_lastDamageAmount = flDamage;

	// Armor. 
	if (pev->armorvalue && !(bitsDamageType & (DMG_FALL | DMG_DROWN)) )// armor doesn't protect against fall or drown damage!
	{
		float flNew = flDamage * flRatio;

		float flArmor;

		flArmor = (flDamage - flNew) * flBonus;

		// Does this use more armor than we have?
		if (flArmor > pev->armorvalue)
		{
			flArmor = pev->armorvalue;
			flArmor *= (1/flBonus);
			flNew = flDamage - flArmor;
			pev->armorvalue = 0;
		}
		else
			pev->armorvalue -= flArmor;
		
		flDamage = flNew;
	}

	// this cast to INT is critical!!! If a player ends up with 0.5 health, the engine will get that
	// as an int (zero) and think the player is dead! (this will incite a clientside screentilt, etc)
	fTookDamage = CBaseMonster::TakeDamage(pevInflictor, pevAttacker, (int)flDamage, bitsDamageType);

	// reset damage time countdown for each type of time based damage player just sustained

	{
		for (int i = 0; i < CDMG_TIMEBASED; i++)
			if (bitsDamageType & (DMG_PARALYZE << i))
				m_rgbTimeBasedDamage[i] = 0;
	}

	// tell director about it
	MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
		WRITE_BYTE ( 9 );	// command length in bytes
		WRITE_BYTE ( DRC_CMD_EVENT );	// take damage event
		WRITE_SHORT( ENTINDEX(this->edict()) );	// index number of primary entity
		WRITE_SHORT( ENTINDEX(ENT(pevInflictor)) );	// index number of secondary entity
		WRITE_LONG( 5 );   // eventflags (priority and flags)
	MESSAGE_END();


	// how bad is it, doc?

	ftrivial = (pev->health > 75 || m_lastDamageAmount < 5);
	fmajor = (m_lastDamageAmount > 25);
	fcritical = (pev->health < 30);

	// handle all bits set in this damage message,
	// let the suit give player the diagnosis

	// UNDONE: add sounds for types of damage sustained (ie: burn, shock, slash )

	// UNDONE: still need to record damage and heal messages for the following types

		// DMG_BURN	
		// DMG_FREEZE
		// DMG_BLAST
		// DMG_SHOCK

	m_bitsDamageType |= bitsDamage; // Save this so we can report it to the client
	m_bitsHUDDamage = -1;  // make sure the damage bits get resent

	while (fTookDamage && (!ftrivial || (bitsDamage & DMG_TIMEBASED)) && ffound && bitsDamage)
	{
		ffound = FALSE;

		if (bitsDamage & DMG_CLUB)
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG4", FALSE, SUIT_NEXT_IN_30SEC);	// minor fracture
			bitsDamage &= ~DMG_CLUB;
			ffound = TRUE;
		}
		if (bitsDamage & (DMG_FALL | DMG_CRUSH))
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG5", FALSE, SUIT_NEXT_IN_30SEC);	// major fracture
			else
				SetSuitUpdate("!HEV_DMG4", FALSE, SUIT_NEXT_IN_30SEC);	// minor fracture
	
			bitsDamage &= ~(DMG_FALL | DMG_CRUSH);
			ffound = TRUE;
		}
		
		if (bitsDamage & DMG_BULLET)
		{
			if (m_lastDamageAmount > 5)
				SetSuitUpdate("!HEV_DMG6", FALSE, SUIT_NEXT_IN_30SEC);	// blood loss detected
			//else
			//	SetSuitUpdate("!HEV_DMG0", FALSE, SUIT_NEXT_IN_30SEC);	// minor laceration
			
			bitsDamage &= ~DMG_BULLET;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_SLASH)
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG1", FALSE, SUIT_NEXT_IN_30SEC);	// major laceration
			else
				SetSuitUpdate("!HEV_DMG0", FALSE, SUIT_NEXT_IN_30SEC);	// minor laceration

			bitsDamage &= ~DMG_SLASH;
			ffound = TRUE;
		}
		
		if (bitsDamage & DMG_SONIC)
		{
			if (fmajor)
				SetSuitUpdate("!HEV_DMG2", FALSE, SUIT_NEXT_IN_1MIN);	// internal bleeding
			bitsDamage &= ~DMG_SONIC;
			ffound = TRUE;
		}

		if (bitsDamage & (DMG_POISON | DMG_PARALYZE))
		{
			SetSuitUpdate("!HEV_DMG3", FALSE, SUIT_NEXT_IN_1MIN);	// blood toxins detected
			bitsDamage &= ~(DMG_POISON | DMG_PARALYZE);
			ffound = TRUE;
		}

		if (bitsDamage & DMG_ACID)
		{
			SetSuitUpdate("!HEV_DET1", FALSE, SUIT_NEXT_IN_1MIN);	// hazardous chemicals detected
			bitsDamage &= ~DMG_ACID;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_NERVEGAS)
		{
			SetSuitUpdate("!HEV_DET0", FALSE, SUIT_NEXT_IN_1MIN);	// biohazard detected
			bitsDamage &= ~DMG_NERVEGAS;
			ffound = TRUE;
		}

		if (bitsDamage & DMG_RADIATION)
		{
			SetSuitUpdate("!HEV_DET2", FALSE, SUIT_NEXT_IN_1MIN);	// radiation detected
			bitsDamage &= ~DMG_RADIATION;
			ffound = TRUE;
		}
		if (bitsDamage & DMG_SHOCK)
		{
			bitsDamage &= ~DMG_SHOCK;
			ffound = TRUE;
		}
	}

	if (m_headshot) {
		MAKE_VECTORS(Vector(0, pev->angles.y, 0));
		pev->punchangle.x = DotProduct(m_headshotDir, gpGlobals->v_forward) * 10;
		pev->punchangle.z = DotProduct(m_headshotDir, gpGlobals->v_right) * 10;
	}
	else if (fabs(pev->punchangle.x) < 2) {
		pev->punchangle.x = -2;
	}
	m_headshot = false;

	if (fTookDamage && !ftrivial && fmajor && flHealthPrev >= 75) 
	{
		// first time we take major damage...
		// turn automedic on if not on
		SetSuitUpdate("!HEV_MED1", FALSE, SUIT_NEXT_IN_30MIN);	// automedic on

		// give morphine shot if not given recently
		SetSuitUpdate("!HEV_HEAL7", FALSE, SUIT_NEXT_IN_30MIN);	// morphine shot
	}
	
	if (fTookDamage && !ftrivial && fcritical && flHealthPrev < 75)
	{

		// already took major damage, now it's critical...
		if (pev->health < 6)
			SetSuitUpdate("!HEV_HLTH3", FALSE, SUIT_NEXT_IN_10MIN);	// near death
		else if (pev->health < 20)
			SetSuitUpdate("!HEV_HLTH2", FALSE, SUIT_NEXT_IN_10MIN);	// health critical
	
		// give critical health warnings
		if (!RANDOM_LONG(0,3) && flHealthPrev < 50)
			SetSuitUpdate("!HEV_DMG7", FALSE, SUIT_NEXT_IN_5MIN); //seek medical attention
	}

	// if we're taking time based damage, warn about its continuing effects
	if (fTookDamage && (bitsDamageType & DMG_TIMEBASED) && flHealthPrev < 75)
		{
			if (flHealthPrev < 50)
			{
				if (!RANDOM_LONG(0,3))
					SetSuitUpdate("!HEV_DMG7", FALSE, SUIT_NEXT_IN_5MIN); //seek medical attention
			}
			else
				SetSuitUpdate("!HEV_HLTH1", FALSE, SUIT_NEXT_IN_10MIN);	// health dropping
		}

	return fTookDamage;
}

//=========================================================
// PackDeadPlayerItems - call this when a player dies to
// pack up the appropriate weapons and ammo items, and to
// destroy anything that shouldn't be packed.
//
// This is pretty brute force :(
//=========================================================
void CBasePlayer::PackDeadPlayerItems( void )
{
	int iWeaponRules;
	int iAmmoRules;
	int i;
	CBasePlayerWeapon *rgpPackWeapons[ 20 ];// 20 hardcoded for now. How to determine exactly how many weapons we have?
	int iPackAmmo[ MAX_AMMO_SLOTS + 1];
	int iPW = 0;// index into packweapons array
	int iPA = 0;// index into packammo array

	memset(rgpPackWeapons, 0, sizeof(rgpPackWeapons) );
	memset(iPackAmmo, -1, sizeof(iPackAmmo) );

	// get the game rules 
	iWeaponRules = g_pGameRules->DeadPlayerWeapons( this );
 	iAmmoRules = g_pGameRules->DeadPlayerAmmo( this );

	if ( iWeaponRules == GR_PLR_DROP_GUN_NO && iAmmoRules == GR_PLR_DROP_AMMO_NO )
	{
		// nothing to pack. Remove the weapons and return. Don't call create on the box!
		RemoveAllItems( TRUE );
		return;
	}

// go through all of the weapons and make a list of the ones to pack
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerItem *pPlayerItem = (CBasePlayerItem*)m_rgpPlayerItems[i].GetEntity();

			while ( pPlayerItem )
			{
				switch( iWeaponRules )
				{
				case GR_PLR_DROP_GUN_ACTIVE:
					if ( m_pActiveItem && pPlayerItem == m_pActiveItem.GetEntity() )
					{
						CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pPlayerItem;
						int nIndex = iPW++;

						// this is the active item. Pack it.
						rgpPackWeapons[nIndex] = pWeapon;

						//Reload the weapon before dropping it if we have ammo
						int j = V_min( pWeapon->iMaxClip() - pWeapon->m_iClip, m_rgAmmo[pWeapon->m_iPrimaryAmmoType] );

						// Add them to the clip
						pWeapon->m_iClip += j;
						m_rgAmmo[pWeapon->m_iPrimaryAmmoType] -= j;

						TabulateAmmo();
					}
					break;

				case GR_PLR_DROP_GUN_ALL:
					rgpPackWeapons[ iPW++ ] = (CBasePlayerWeapon *)pPlayerItem;
					break;

				default:
					break;
				}

				pPlayerItem = (CBasePlayerItem*)pPlayerItem->m_pNext.GetEntity();
			}
		}
	}

// now go through ammo and make a list of which types to pack.
	if ( iAmmoRules != GR_PLR_DROP_AMMO_NO )
	{
		for ( i = 0 ; i < MAX_AMMO_SLOTS ; i++ )
		{
			if ( m_rgAmmo[ i ] > 0 )
			{
				// player has some ammo of this type.
				switch ( iAmmoRules )
				{
				case GR_PLR_DROP_AMMO_ALL:
					iPackAmmo[ iPA++ ] = i;
					break;

				case GR_PLR_DROP_AMMO_ACTIVE: {
					CBasePlayerItem* item = (CBasePlayerItem*)m_pActiveItem.GetEntity();
					if (item && i == item->PrimaryAmmoIndex())
					{
						// this is the primary ammo type for the active weapon
						iPackAmmo[iPA++] = i;
					}
					else if (item && i == item->SecondaryAmmoIndex())
					{
						// this is the secondary ammo type for the active weapon
						iPackAmmo[iPA++] = i;
					}
					break;
				}
				default:
					break;
				}
			}
		}
	}

	CBasePlayerWeapon* firstWep = rgpPackWeapons[0];
	if (iWeaponRules == GR_PLR_DROP_GUN_ACTIVE && firstWep && !strcmp(STRING(firstWep->pev->classname), "weapon_shockrifle")) {
		if (RemovePlayerItem(firstWep)) {
			// fixme: logic duplicated in weapon drop code
			static std::map<std::string, std::string> keys = { {"is_player_ally", "1"} };
			Vector angles(0, pev->angles.y, 0);
			CBaseEntity* pRoach = CBaseEntity::Create("monster_shockroach",
				pev->origin + gpGlobals->v_forward * 10, angles, edict(), keys);
			pRoach->pev->velocity = pev->velocity * 1.2;
		
		}

		RemoveAllItems(TRUE);// now strip off everything that wasn't handled by the code above.

		return;
	}

// create a box to pack the stuff into.
	CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create( "weaponbox", pev->origin, pev->angles, edict() );

	pWeaponBox->pev->angles.x = 0;// don't let weaponbox tilt.
	pWeaponBox->pev->angles.z = 0;

	pWeaponBox->SetThink( &CWeaponBox::Kill );
	pWeaponBox->pev->nextthink = gpGlobals->time + item_despawn_time.value;

// back these two lists up to their first elements
	iPA = 0;
	iPW = 0;

	bool bPackItems = TRUE;

	if (iAmmoRules == GR_PLR_DROP_AMMO_ACTIVE && iWeaponRules == GR_PLR_DROP_GUN_ACTIVE)
	{
		if (firstWep && FClassnameIs(firstWep->pev, "weapon_satchel") && (iPackAmmo[0] == -1 || (m_rgAmmo[iPackAmmo[0]] == 0)))
		{
			bPackItems = FALSE;
		}
	}

	if (bPackItems) {
		// pack the ammo
		while (iPackAmmo[iPA] != -1)
		{
			pWeaponBox->PackAmmo(MAKE_STRING(CBasePlayerItem::AmmoInfoArray[iPackAmmo[iPA]].pszName), m_rgAmmo[iPackAmmo[iPA]]);
			iPA++;
		}

		// now pack all of the items in the lists
		while (rgpPackWeapons[iPW])
		{
			// weapon unhooked from the player. Pack it into der box.
			pWeaponBox->PackWeapon(rgpPackWeapons[iPW]);

			if (iAmmoRules == GR_PLR_DROP_AMMO_ACTIVE) {
				SET_MODEL(pWeaponBox->edict(), rgpPackWeapons[iPW]->GetModelW());

				if (!strcmp(STRING(rgpPackWeapons[iPW]->pev->classname), "weapon_tripmine")) {
					pWeaponBox->pev->body = 3;
					pWeaponBox->pev->sequence = TRIPMINE_GROUND;
				}
			}

			iPW++;
		}
	}

	pWeaponBox->pev->velocity = pev->velocity * 1.2;// weaponbox has player's velocity, then some.
	pWeaponBox->pev->avelocity = Vector(0, 256, 256);

	RemoveAllItems( TRUE );// now strip off everything that wasn't handled by the code above.

	CleanupWeaponboxes();
}

void CBasePlayer::RemoveAllItems( BOOL removeSuit )
{
	if (!g_serveractive) {
		// game will crash if removing custom weapons defined in plugins that were unloaded during level change
		return;
	}

	if (m_pActiveItem)
	{
		CBasePlayerItem* item = (CBasePlayerItem*)m_pActiveItem.GetEntity();
		ResetAutoaim( );
		item->Holster( );
		m_pActiveItem = NULL;
	}

	m_pLastItem = NULL;

	ReleaseControlledObjects();

	int i;
	CBasePlayerItem *pPendingItem;
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		m_pActiveItem = m_rgpPlayerItems[i].GetEntity();
		while (m_pActiveItem)
		{
			CBasePlayerItem* item = (CBasePlayerItem*)m_pActiveItem.GetEntity();
			pPendingItem = (CBasePlayerItem*)item->m_pNext.GetEntity();
			item->Drop( );
			m_pActiveItem = pPendingItem;
		}
		m_rgpPlayerItems[i] = NULL;
	}
	m_pActiveItem = NULL;

	pev->viewmodel		= 0;
	pev->weaponmodel	= 0;
	
	if ( removeSuit )
		pev->weapons = 0;
	else
		pev->weapons &= ~WEAPON_ALLWEAPONS;

	for ( i = 0; i < MAX_AMMO_SLOTS;i++)
		m_rgAmmo[i] = 0;

	UpdateClientData();
	// send Selected Weapon Message to our client
	MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
		WRITE_BYTE(0);
		WRITE_BYTE(0);
		WRITE_BYTE(0);
	MESSAGE_END();
}

/*
 * GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
 *
 * ENTITY_METHOD(PlayerDie)
 */
entvars_t *g_pevLastInflictor;  // Set in combat.cpp.  Used to pass the damage inflictor for death messages.
								// Better solution:  Add as parameter to all Killed() functions.

void CBasePlayer::Killed( entvars_t *pevAttacker, int iGib )
{
	CSound *pSound;

	if (m_deathMessageSent) {
		return; // don't repeat kill messages when gibbed
	}

	m_lastKillTime = gpGlobals->time;

	// Holster weapon immediately, to allow it to cleanup
	if ( m_pActiveItem )
		((CBasePlayerItem*)m_pActiveItem.GetEntity())->Holster();

	g_pGameRules->PlayerKilled( this, pevAttacker, g_pevLastInflictor );

	ReleaseControlledObjects();

	// this client isn't going to be thinking for a while, so reset the sound until they respawn
	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( edict() ) );
	{
		if ( pSound )
		{
			pSound->Reset();
		}
	}

	SetAnimation( PLAYER_DIE );
	
	m_flRespawnTimer = 0;

	pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes

	pev->deadflag		= DEAD_DYING;
	pev->movetype		= MOVETYPE_TOSS;
	ClearBits( pev->flags, FL_ONGROUND );
	if (pev->velocity.z < 10)
		pev->velocity.z += RANDOM_FLOAT(0,300);

	// clear out the suit message cache so we don't keep chattering
	SetSuitUpdate(NULL, FALSE, 0);

	// send "health" update message to zero
	m_iClientHealth = 0;
	MESSAGE_BEGIN( MSG_ONE, gmsgHealth, NULL, pev );
		WRITE_BYTE( m_iClientHealth );
	MESSAGE_END();

	// Tell Ammo Hud that the player is dead
	MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
		WRITE_BYTE(0);
		WRITE_BYTE(0XFF);
		WRITE_BYTE(0xFF);
	MESSAGE_END();

	// reset FOV
	pev->fov = m_iFOV = m_iClientFOV = 0;

	MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
		WRITE_BYTE(0);
	MESSAGE_END();

	// resend hud color
	MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
	WRITE_BYTE(entindex());	// client number
	WRITE_SHORT(pev->frags);
	WRITE_SHORT(m_iDeaths);
	WRITE_SHORT(0);
	WRITE_SHORT(DEFAULT_TEAM_COLOR);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
	WRITE_BYTE(entindex());
	WRITE_STRING(DEFAULT_TEAM_NAME);
	MESSAGE_END();


	// UNDONE: Put this in, but add FFADE_PERMANENT and make fade time 8.8 instead of 4.12
	// UTIL_ScreenFade( edict(), Vector(128,0,0), 6, 15, 255, FFADE_OUT | FFADE_MODULATE );

	if (pev->health > 0) {
		pev->health = 0; // client won't rotate view otherwise
	}

	if ( ( pev->health < -40 && iGib != GIB_NEVER ) || iGib == GIB_ALWAYS )
	{
		pev->solid			= SOLID_NOT;
		GibMonster();	// This clears pev->model
		pev->effects |= EF_NODRAW;
		return;
	}

	DeathSound();
	
	pev->angles.x = 0;
	pev->angles.z = 0;
	pev->solid = SOLID_NOT; // fix laggy movement on corpses

	SetThink(&CBasePlayer::PlayerDeathThink);
	pev->nextthink = gpGlobals->time + 0.1;
}

Vector CBasePlayer::BodyTarget(const Vector& posSrc) {
	// have enemies sometimes aim for the head, but mostly not (bias towards lower values)
	float bias = RANDOM_FLOAT(0, 1.0f);
	bias = bias * bias * bias;

	Vector headHeight = (pev->flags & FL_DUCKING) ? pev->view_ofs*2.0f : pev->view_ofs;

	return Center() + headHeight * (bias*0.5f + 0.5f);
}

// Set the activity based on an event or current state
void CBasePlayer::SetAnimation( PLAYER_ANIM playerAnim, float duration)
{
	int animDesired;
	float speed;
	char szAnim[64];

	speed = pev->velocity.Length2D();

	if (pev->flags & FL_FROZEN)
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	switch (playerAnim) 
	{
	case PLAYER_JUMP:
		m_IdealActivity = ACT_HOP;
		break;
	
	case PLAYER_SUPERJUMP:
		m_IdealActivity = ACT_LEAP;
		break;
	
	case PLAYER_DIE:
		m_IdealActivity = ACT_DIESIMPLE;
		m_IdealActivity = GetDeathActivity( );
		break;

	case PLAYER_RELOAD:
	case PLAYER_ATTACK1:
	case PLAYER_USE:
	case PLAYER_DROP_ITEM:
		switch( m_Activity )
		{
		case ACT_DIESIMPLE:
			m_IdealActivity = m_Activity;
			break;
		default:
			if (playerAnim == PLAYER_RELOAD) {
				m_IdealActivity = ACT_RELOAD;
			}
			else if (playerAnim == PLAYER_ATTACK1) {
				m_IdealActivity = ACT_RANGE_ATTACK1;
			}
			else if (playerAnim == PLAYER_DROP_ITEM) {
				m_IdealActivity = ACT_ARM;
			}
			else {
				m_IdealActivity = ACT_USE;
			}
			break;
		}
		break;
	case PLAYER_IDLE:
	case PLAYER_WALK:
	{
		if (pev->waterlevel > 1)
		{
			if (speed == 0)
				m_IdealActivity = ACT_HOVER;
			else
				m_IdealActivity = ACT_SWIM;
		}
		else if (!FBitSet(pev->flags, FL_ONGROUND) && (m_Activity == ACT_HOP || m_Activity == ACT_LEAP))	// Still jumping
		{
			m_IdealActivity = m_Activity;
		}
		else
		{
			m_IdealActivity = ACT_WALK;
		}
		break;
	}
	}

	switch (m_IdealActivity)
	{
	case ACT_HOVER:
	case ACT_LEAP:
	case ACT_SWIM:
	case ACT_HOP:
	case ACT_DIESIMPLE:
	default:
	{
		if (m_Activity == m_IdealActivity)
			return;

		bool isFloating = m_IdealActivity == ACT_SWIM || m_IdealActivity == ACT_HOVER;
		if (isFloating && m_Activity == ACT_RANGE_ATTACK1 && !m_fSequenceFinished) {
			pev->gaitsequence = LookupActivity(ACT_HOVER);
			return;
		}

		m_Activity = m_IdealActivity;

		animDesired = LookupActivity(m_Activity);
		// Already using the desired animation?
		if (pev->sequence == animDesired)
			return;

		pev->gaitsequence = 0;
		pev->sequence = animDesired;
		pev->frame = 0;
		ResetSequenceInfo();
		return;
	}
	case ACT_RANGE_ATTACK1:
	{
		if ((m_Activity == ACT_HOP || m_Activity == ACT_LEAP) && pev->frame < 220) {
			// jump animation has priority
			return;
		}

		if (FBitSet(pev->flags, FL_DUCKING))	// crouching
			strcpy_safe(szAnim, "crouch_shoot_", 64);
		else
			strcpy_safe(szAnim, "ref_shoot_", 64);

		if (!strcmp(m_szAnimExtention, "bow")) {
			strcat_safe(szAnim, "shotgun", 64); // hl bow shoot anim is the same as aiming
		}
		else {
			strcat_safe(szAnim, m_szAnimExtention, 64);
		}

		animDesired = LookupSequence(szAnim);
		if (animDesired == -1)
			animDesired = 0;

		if (pev->sequence != animDesired || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		if (!m_fSequenceLoops)
		{
			pev->effects |= EF_NOINTERP;
		}
		
		m_Activity = m_IdealActivity;

		pev->sequence = animDesired;
		ResetSequenceInfo();
		break;
	}

	case ACT_USE:
	case ACT_ARM:
	case ACT_RELOAD:
	{
		if ((m_Activity == ACT_HOP || m_Activity == ACT_LEAP) && pev->frame < 200) {
			// jump animation has priority
			return;
		}

		bool ducking = FBitSet(pev->flags, FL_DUCKING);
		const char* seqName = "";

		if (m_IdealActivity == ACT_ARM) {
			seqName = ducking ? "crouch_shoot_squeak" : "ref_shoot_squeak";
		}
		else if (m_IdealActivity == ACT_USE) {
			seqName = ducking ? "crouch_shoot_trip" : "ref_shoot_trip";
		}
		else {
			seqName = ducking ? "crouch_aim_squeak" : "ref_aim_squeak";
		}

		animDesired = LookupSequence(seqName);

		if (animDesired == -1)
			animDesired = 0;

		if (pev->sequence != animDesired || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		//if (!m_fSequenceLoops)
		{
			pev->effects |= EF_NOINTERP;
		}

		m_Activity = m_IdealActivity;

		pev->sequence = animDesired;
		ResetSequenceInfo();
		break;
	}
	case ACT_WALK:
	{
		bool upperBodyActing = (m_Activity == ACT_RANGE_ATTACK1 || m_Activity == ACT_RELOAD 
			|| m_Activity == ACT_USE || m_Activity == ACT_ARM);
		if (!upperBodyActing || m_fSequenceFinished)
		{
			if (FBitSet(pev->flags, FL_DUCKING))	// crouching
				strcpy_safe(szAnim, "crouch_aim_", 64);
			else
				strcpy_safe(szAnim, "ref_aim_", 64);
			strcat_safe(szAnim, m_szAnimExtention, 64);
			animDesired = LookupSequence(szAnim);
			if (animDesired == -1)
				animDesired = 0;
			m_Activity = ACT_WALK;
		}
		else
		{
			animDesired = pev->sequence;
		}
	}
	}

	if ( FBitSet( pev->flags, FL_DUCKING ) )
	{
		if ( speed == 0)
		{
			pev->gaitsequence	= LookupActivity( ACT_CROUCHIDLE );
			// pev->gaitsequence	= LookupActivity( ACT_CROUCH );
		}
		else
		{
			pev->gaitsequence	= LookupActivity( ACT_CROUCH );
		}
	}
	else if ( speed > 220 )
	{
		pev->gaitsequence	= LookupActivity( ACT_RUN );
	}
	else if (speed > 0)
	{
		pev->gaitsequence	= LookupActivity( ACT_WALK );
	}
	else
	{
		// pev->gaitsequence	= LookupActivity( ACT_WALK );
		pev->gaitsequence	= LookupSequence( "deep_idle" );
	}

	if (duration) {
		studiohdr_t* pstudiohdr = (studiohdr_t*)GET_MODEL_PTR(edict());
		mstudioseqdesc_t* pseqdesc;

		if (pstudiohdr && pev->sequence >= 0 && pev->sequence < pstudiohdr->numseq)
		{
			pseqdesc = (mstudioseqdesc_t*)((byte*)pstudiohdr + pstudiohdr->seqindex) + (int)pev->sequence;
			float animDuration = (pseqdesc->numframes - 1) / pseqdesc->fps;
			pev->framerate = animDuration / duration;
			return;
		}
	}

	// Already using the desired animation?
	if (pev->sequence == animDesired)
		return;

	//ALERT( at_console, "Set animation to %d\n", animDesired );
	// Reset to first frame of desired animation
	pev->sequence		= animDesired;
	pev->frame			= 0;
	ResetSequenceInfo( );
}

/*
===========
TabulateAmmo
This function is used to find and store 
all the ammo we have into the ammo vars.
============
*/
void CBasePlayer::TabulateAmmo()
{
	ammo_9mm = AmmoInventory( GetAmmoIndex( "9mm" ) );
	ammo_357 = AmmoInventory( GetAmmoIndex( "357" ) );
	ammo_argrens = AmmoInventory( GetAmmoIndex( "ARgrenades" ) );
	ammo_bolts = AmmoInventory( GetAmmoIndex( "bolts" ) );
	ammo_buckshot = AmmoInventory( GetAmmoIndex( "buckshot" ) );
	ammo_rockets = AmmoInventory( GetAmmoIndex( "rockets" ) );
	ammo_uranium = AmmoInventory( GetAmmoIndex( "uranium" ) );
	ammo_hornets = AmmoInventory( GetAmmoIndex( "Hornets" ) );
}

int CBasePlayer::rgAmmo(int ammoIdx) {
	if (ammoIdx < 0 || ammoIdx >= MAX_AMMO_TYPES) {
		ALERT(at_console, "Invalid ammo index %d\n", ammoIdx);
		return -1;
	}

	return m_rgAmmo[ammoIdx];
}

void CBasePlayer::rgAmmo(int ammoIdx, int newCount) {
	if (ammoIdx < 0 || ammoIdx >= MAX_AMMO_TYPES) {
		ALERT(at_console, "Invalid ammo index %d\n", ammoIdx);
		return;
	}

	m_rgAmmo[ammoIdx] = newCount;
}

void CBasePlayer::ReleaseControlledObjects() {
	if (m_pTank)
	{
		m_pTank->Use(this, this, USE_OFF, 0);
		m_pTank = NULL;
	}
	if (m_pPushable)
	{
		m_pPushable->Use(this, this, USE_OFF, 0);
		m_pPushable = NULL;
	}
}

/*
===========
WaterMove
============
*/
void CBasePlayer::WaterMove()
{
	int air;

	if (pev->movetype == MOVETYPE_NOCLIP)
		return;

	if (pev->health < 0)
		return;

	// waterlevel 0 - not in water
	// waterlevel 1 - feet in water
	// waterlevel 2 - waist in water
	// waterlevel 3 - head in water

	if (pev->waterlevel != 3) 
	{
		// not underwater
		
		// play 'up for air' sound
		if (pev->air_finished < gpGlobals->time)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade1.wav", 1, ATTN_NORM);
		else if (pev->air_finished < gpGlobals->time + 9)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade2.wav", 1, ATTN_NORM);

		pev->air_finished = gpGlobals->time + AIRTIME;
		pev->dmg = 2;

		// if we took drowning damage, give it back slowly
		if (m_idrowndmg > m_idrownrestored)
		{
			// set drowning damage bit.  hack - dmg_drownrecover actually
			// makes the time based damage code 'give back' health over time.
			// make sure counter is cleared so we start count correctly.
			
			// NOTE: this actually causes the count to continue restarting
			// until all drowning damage is healed.

			m_bitsDamageType |= DMG_DROWNRECOVER;
			m_bitsDamageType &= ~DMG_DROWN;
			m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;
		}

	}
	else
	{	// fully under water
		// stop restoring damage while underwater
		m_bitsDamageType &= ~DMG_DROWNRECOVER;
		m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;

		if (pev->air_finished < gpGlobals->time)		// drown!
		{
			if (pev->pain_finished < gpGlobals->time)
			{
				// take drowning damage
				pev->dmg += 1;
				if (pev->dmg > 5)
					pev->dmg = 5;
				TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), pev->dmg, DMG_DROWN);
				pev->pain_finished = gpGlobals->time + 1;
				
				// track drowning damage, give it back when
				// player finally takes a breath

				m_idrowndmg += pev->dmg;
			} 
		}
		else
		{
			m_bitsDamageType &= ~DMG_DROWN;
		}
	}

	if (!pev->waterlevel)
	{
		if (FBitSet(pev->flags, FL_INWATER))
		{       
			ClearBits(pev->flags, FL_INWATER);
		}
		return;
	}
	
	// make bubbles

	air = (int)(pev->air_finished - gpGlobals->time);
	if (!RANDOM_LONG(0,0x1f) && RANDOM_LONG(0,AIRTIME-1) >= air)
	{
		switch (RANDOM_LONG(0,3))
			{
			case 0:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim1.wav", 0.8, ATTN_NORM); break;
			case 1:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim2.wav", 0.8, ATTN_NORM); break;
			case 2:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim3.wav", 0.8, ATTN_NORM); break;
			case 3:	EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim4.wav", 0.8, ATTN_NORM); break;
		}
	}

	if (pev->watertype == CONTENT_LAVA)		// do damage
	{
		if (pev->dmgtime < gpGlobals->time)
			TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 10 * pev->waterlevel, DMG_BURN);
	}
	else if (pev->watertype == CONTENT_SLIME)		// do damage
	{
		pev->dmgtime = gpGlobals->time + 1;
		TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 4 * pev->waterlevel, DMG_ACID);
	}
	
	if (!FBitSet(pev->flags, FL_INWATER))
	{
		SetBits(pev->flags, FL_INWATER);
		pev->dmgtime = 0;
	}
}

// TRUE if the player is attached to a ladder
BOOL CBasePlayer::IsOnLadder( void )
{ 
	return ( pev->movetype == MOVETYPE_FLY );
}

void CBasePlayer::PlayerDeathThink(void)
{
	float flForward;

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		flForward = pev->velocity.Length() - 20;
		if (flForward <= 0)
			pev->velocity = g_vecZero;
		else    
			pev->velocity = flForward * pev->velocity.Normalize();
	}

	if ( HasWeapons() )
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
	}

	float deadTime = gpGlobals->time - m_lastKillTime;
	if (deadTime < mp_respawndelay.value + 1.0f) {
		if (gpGlobals->time - m_lastSpawnMessage > 0.2f) {
			int timeLeft = (int)ceilf(mp_respawndelay.value - deadTime);
			const char* msg = UTIL_VarArgs("Respawn allowed in %d seconds", timeLeft);

			if (deadTime >= mp_respawndelay.value) {
				msg = "You can respawn now!";
			} else if (timeLeft == 1) {
				msg = "Respawn allowed in 1 second";
			}

			CLIENT_PRINTF(edict(), print_center, msg);
			m_lastSpawnMessage = gpGlobals->time;
		}
	}

	if (pev->modelindex && (!m_fSequenceFinished) && (pev->deadflag == DEAD_DYING))
	{
		StudioFrameAdvance( );

		m_flRespawnTimer += gpGlobals->frametime;
		if (m_flRespawnTimer < 4.0f)   // 120 frames at 30fps -- animations should be no longer than this
			return;
	}

	if (pev->deadflag == DEAD_DYING)
	{
		//Once we finish animating, if we're in multiplayer just make a copy of our body right away.
		if (m_fSequenceFinished && g_pGameRules->IsMultiplayer() && pev->movetype == MOVETYPE_NONE)
		{
			CopyToBodyQue(pev);
			pev->modelindex = 0;
		}

		pev->deadflag = DEAD_DEAD;
	}

	// once we're done animating our death and we're on the ground, we want to set movetype to None so our dead body won't do collisions and stuff anymore
	// this prevents a bug where the dead body would go to a player's head if he walked over it while the dead player was clicking their button to respawn
	if ( pev->movetype != MOVETYPE_NONE && FBitSet(pev->flags, FL_ONGROUND) )
		pev->movetype = MOVETYPE_NONE;
	
	StopAnimation();

	pev->effects |= EF_NOINTERP;
	pev->framerate = 0.0;

	BOOL fAnyButtonDown = (pev->button & ~IN_SCORE );
	
	// wait for all buttons released
	if (pev->deadflag == DEAD_DEAD)
	{
		if (fAnyButtonDown)
			return;

		if ( g_pGameRules->FPlayerCanRespawn( this ) )
		{
			m_fDeadTime = gpGlobals->time;
			pev->deadflag = DEAD_RESPAWNABLE;
		}
		
		return;
	}

// if the player has been dead for one second longer than allowed by forcerespawn, 
// forcerespawn isn't on. Send the player off to an intermission camera until they 
// choose to respawn.
	/*
	if ( g_pGameRules->IsMultiplayer() && ( gpGlobals->time > (m_fDeadTime + 6) ) && !(m_afPhysicsFlags & PFLAG_OBSERVER) )
	{
		// go to dead camera. 
		StartDeathCam();
	}
	*/

	if ( pev->iuser1 )	// player is in spectator mode
		return;	
	
// wait for any button down,  or mp_forcerespawn is set and the respawn time is up
	if (!fAnyButtonDown 
		&& !( g_pGameRules->IsMultiplayer() && forcerespawn.value > 0 && (gpGlobals->time > (m_fDeadTime + 5))) )
		return;

	pev->button = 0;
	m_flRespawnTimer = 0.0f;

	//ALERT(at_console, "Respawn\n");

	respawn(pev, !(m_afPhysicsFlags & PFLAG_OBSERVER) );// don't copy a corpse if we're in deathcam.
	pev->nextthink = -1;
}

//=========================================================
// StartDeathCam - find an intermission spot and send the
// player off into observer mode
//=========================================================
void CBasePlayer::StartDeathCam( void )
{
	edict_t *pSpot, *pNewSpot;
	int iRand;

	if ( pev->view_ofs == g_vecZero )
	{
		// don't accept subsequent attempts to StartDeathCam()
		return;
	}

	pSpot = FIND_ENTITY_BY_CLASSNAME( NULL, "info_intermission");	

	if ( !FNullEnt( pSpot ) )
	{
		// at least one intermission spot in the world.
		iRand = RANDOM_LONG( 0, 3 );

		while ( iRand > 0 )
		{
			pNewSpot = FIND_ENTITY_BY_CLASSNAME( pSpot, "info_intermission");
			
			if ( pNewSpot )
			{
				pSpot = pNewSpot;
			}

			iRand--;
		}

		CopyToBodyQue( pev );

		UTIL_SetOrigin( pev, pSpot->v.origin );
		pev->angles = pev->v_angle = pSpot->v.v_angle;
	}
	else
	{
		// no intermission spot. Push them up in the air, looking down at their corpse
		TraceResult tr;
		CopyToBodyQue( pev );
		UTIL_TraceLine( pev->origin, pev->origin + Vector( 0, 0, 128 ), ignore_monsters, edict(), &tr );

		UTIL_SetOrigin( pev, tr.vecEndPos );
		pev->angles = pev->v_angle = UTIL_VecToAngles( tr.vecEndPos - pev->origin  );
	}

	// start death cam

	m_afPhysicsFlags |= PFLAG_OBSERVER;
	pev->view_ofs = g_vecZero;
	pev->fixangle = TRUE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	pev->movetype = MOVETYPE_NONE;
	pev->modelindex = 0;
}

void CBasePlayer::StartObserver( Vector vecPosition, Vector vecViewAngle )
{
	// clear any clientside entities attached to this player
	MESSAGE_BEGIN( MSG_PAS, SVC_TEMPENTITY, pev->origin );
		WRITE_BYTE( TE_KILLPLAYERATTACHMENTS );
		WRITE_BYTE( (BYTE)entindex() );
	MESSAGE_END();

	// Holster weapon immediately, to allow it to cleanup
	if (m_pActiveItem)
		((CBasePlayerItem*)m_pActiveItem.GetEntity())->Holster();

	ReleaseControlledObjects();

	// clear out the suit message cache so we don't keep chattering
	SetSuitUpdate(NULL, FALSE, 0);

	// Tell Ammo Hud that the player is dead
	MESSAGE_BEGIN( MSG_ONE, gmsgCurWeapon, NULL, pev );
		WRITE_BYTE(0);
		WRITE_BYTE(0XFF);
		WRITE_BYTE(0xFF);
	MESSAGE_END();

	// reset FOV
	m_iFOV = m_iClientFOV = 0;
	pev->fov = m_iFOV;
	MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
		WRITE_BYTE(0);
	MESSAGE_END();

	// Setup flags
	m_iHideHUD = (HIDEHUD_HEALTH | HIDEHUD_WEAPONS);
	m_afPhysicsFlags |= PFLAG_OBSERVER;
	pev->effects = EF_NODRAW;
	pev->view_ofs = g_vecZero;
	pev->angles = pev->v_angle = vecViewAngle;
	pev->fixangle = TRUE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	pev->movetype = MOVETYPE_NONE;
	ClearBits( m_afPhysicsFlags, PFLAG_DUCKING );
	ClearBits( pev->flags, FL_DUCKING );
	pev->deadflag = DEAD_RESPAWNABLE;
	pev->health = 1;
	m_isObserver = true;
	m_lastObserverSwitch = gpGlobals->time;

	// Clear out the status bar
	m_fInitHUD = TRUE;

	pev->team =  0;
	MESSAGE_BEGIN( MSG_ALL, gmsgTeamInfo );
		WRITE_BYTE( ENTINDEX(edict()) );
		WRITE_STRING( "" );
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
	WRITE_BYTE(entindex());	// client number
	WRITE_SHORT(pev->frags);
	WRITE_SHORT(m_iDeaths);
	WRITE_SHORT(0);
	WRITE_SHORT(OBSERVER_TEAM_COLOR);
	MESSAGE_END();

	// Remove all the player's stuff
	RemoveAllItems( FALSE );

	// Move them to the new position
	UTIL_SetOrigin( pev, vecPosition );

	// Find a player to watch
	m_flNextObserverInput = 0;
	Observer_SetMode( m_iObserverLastMode );
}

void CBasePlayer::LeaveObserver()
{
	ClearBits( m_afPhysicsFlags, PFLAG_OBSERVER );
	pev->iuser1 = OBS_NONE;
	m_isObserver = false;
	m_lastObserverSwitch = gpGlobals->time;
	m_iHideHUD = 0;

	MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
	WRITE_BYTE(ENTINDEX(edict()));
	WRITE_STRING(DEFAULT_TEAM_NAME);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
	WRITE_BYTE(entindex());	// client number
	WRITE_SHORT(pev->frags);
	WRITE_SHORT(m_iDeaths);
	WRITE_SHORT(0);
	WRITE_SHORT(GetNameColor());
	MESSAGE_END();
}

// 
// PlayerUse - handles USE keypress
//
void CBasePlayer::PlayerUse ( void )
{
	if ( IsObserver() )
		return;

	// Was use pressed or released?
	if ( ! ((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
		return;

	// Hit Use on a train?
	if ( m_afButtonPressed & IN_USE )
	{
		SetAnimation(PLAYER_USE);

		if ( m_pTank != NULL )
		{
			// Stop controlling the tank
			// TODO: Send HUD Update
			m_pTank->Use( this, this, USE_OFF, 0 );
			m_pTank = NULL;
			return;
		}
		else
		{
			if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
			{
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW|TRAIN_OFF;
				return;
			}
			else
			{	// Start controlling the train!
				CBaseEntity *pTrain = CBaseEntity::Instance( pev->groundentity );

				if ( pTrain && !(pev->button & IN_JUMP) && FBitSet(pev->flags, FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(pev) )
				{
					m_afPhysicsFlags |= PFLAG_ONTRAIN;
					m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
					m_iTrain |= TRAIN_NEW;
					EMIT_SOUND( ENT(pev), CHAN_ITEM, "plats/train_use1.wav", 0.8, ATTN_NORM);
					return;
				}
			}
		}
	}

	if (m_afButtonReleased & IN_USE) {
		if (m_pPushable != NULL) {
			// Stop lifting the pushable
			m_pPushable->Use(this, this, USE_OFF, 0);
			m_pPushable = NULL;
			return;
		}
	}

	CBaseEntity *pObject = NULL;
	CBaseEntity *pClosest = NULL;
	CBaseEntity *pInside = NULL;
	Vector		vecLOS;
	Vector viewPos = pev->origin + pev->view_ofs;
	float flMaxDot = VIEW_FIELD_NARROW;
	float flDot;
	float bestDist = FLT_MAX;

	UTIL_MakeVectors ( pev->v_angle );// so we know which way we are facing
	
	while ((pObject = UTIL_FindEntityInSphere( pObject, pev->origin, PLAYER_SEARCH_RADIUS )) != NULL)
	{

		if (pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE))
		{
			// !!!PERFORMANCE- should this check be done on a per case basis AFTER we've determined that
			// this object is actually usable? This dot is being done for every object within PLAYER_SEARCH_RADIUS
			// when player hits the use key. How many objects can be in that area, anyway? (sjb)
			vecLOS = (VecBModelOrigin( pObject->pev ) - viewPos);
			
			// This essentially moves the origin of the target to the corner nearest the player to test to see 
			// if it's "hull" is in the view cone
			vecLOS = UTIL_ClampVectorToBox( vecLOS, pObject->pev->size * 0.5 );
			
			flDot = DotProduct (vecLOS , gpGlobals->v_forward);
			if (flDot > flMaxDot)
			{// only if the item is in front of the user
				pClosest = pObject;
				flMaxDot = flDot;
//				ALERT( at_console, "%s : %f\n", STRING( pObject->pev->classname ), flDot );
			}

			// player is INSIDE the usable entity. View direction may the opposite of expected
			// (example: big momma has abs box much larger than its collision box)
			float dist = (vecLOS - pObject->Center()).Length();
			if (dist < bestDist && Intersects(pObject)) {
				pInside = pObject;
				bestDist = dist;
			}

//			ALERT( at_console, "%s : %f\n", STRING( pObject->pev->classname ), flDot );
		}
	}
	pObject = pClosest ? pClosest : pInside;

	// Found an object
	if (pObject )
	{
		//!!!UNDONE: traceline here to prevent USEing buttons through walls			
		int caps = pObject->ObjectCaps();

		if ( m_afButtonPressed & IN_USE )
			EMIT_SOUND( ENT(pev), CHAN_ITEM, "common/wpn_select.wav", 0.4, ATTN_NORM);

		if ( ( (pev->button & IN_USE) && (caps & FCAP_CONTINUOUS_USE) ) ||
			 ( (m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE|FCAP_ONOFF_USE)) ) )
		{
			if ( caps & FCAP_CONTINUOUS_USE )
				m_afPhysicsFlags |= PFLAG_USING;

			pObject->Use( this, this, USE_SET, 1 );
		}
		// UNDONE: Send different USE codes for ON/OFF.  Cache last ONOFF_USE object to send 'off' if you turn away
		else if ( (m_afButtonReleased & IN_USE) && (pObject->ObjectCaps() & FCAP_ONOFF_USE) )	// BUGBUG This is an "off" use
		{
			pObject->Use( this, this, USE_SET, 0 );
		}
	}
	else
	{
		if ( m_afButtonPressed & IN_USE )
			EMIT_SOUND( ENT(pev), CHAN_ITEM, "common/wpn_denyselect.wav", 0.4, ATTN_NORM);
	}
}

void CBasePlayer::Jump()
{
	Vector		vecWallCheckDir;// direction we're tracing a line to find a wall when walljumping
	Vector		vecAdjustedVelocity;
	Vector		vecSpot;
	TraceResult	tr;
	
	if (FBitSet(pev->flags, FL_WATERJUMP))
		return;
	
	if (pev->waterlevel >= 2)
	{
		return;
	}

	// jump velocity is sqrt( height * gravity * 2)

	// If this isn't the first frame pressing the jump button, break out.
	if ( !FBitSet( m_afButtonPressed, IN_JUMP ) )
		return;         // don't pogo stick

	if ( !(pev->flags & FL_ONGROUND) || !pev->groundentity )
	{
		return;
	}

// many features in this function use v_forward, so makevectors now.
	UTIL_MakeVectors (pev->angles);

	// ClearBits(pev->flags, FL_ONGROUND);		// don't stairwalk
	
	SetAnimation( PLAYER_JUMP );

	if ( m_fLongJump &&
		(pev->button & IN_DUCK) &&
		( pev->flDuckTime > 0 ) &&
		pev->velocity.Length() > 50 )
	{
		SetAnimation( PLAYER_SUPERJUMP );
	}

	// If you're standing on a conveyor, add it's velocity to yours (for momentum)
	entvars_t *pevGround = VARS(pev->groundentity);
	if ( pevGround && (pevGround->flags & FL_CONVEYOR) )
	{
		pev->velocity = pev->velocity + pev->basevelocity;
	}
}

// This is a glorious hack to find free space when you've crouched into some solid space
// Our crouching collisions do not work correctly for some reason and this is easier
// than fixing the problem :(
void FixPlayerCrouchStuck( edict_t *pPlayer )
{
	TraceResult trace;

	// Move up as many as 18 pixels if the player is stuck.
	for ( int i = 0; i < 18; i++ )
	{
		UTIL_TraceHull( pPlayer->v.origin, pPlayer->v.origin, dont_ignore_monsters, head_hull, pPlayer, &trace );
		if ( trace.fStartSolid )
			pPlayer->v.origin.z ++;
		else
			break;
	}
}

void CBasePlayer::Duck( )
{
	if (pev->button & IN_DUCK) 
	{
		if ( m_IdealActivity != ACT_LEAP )
		{
			SetAnimation( PLAYER_WALK );
		}
	}
}

//
// ID's player as such.
//
int  CBasePlayer::Classify ( void )
{
	return CLASS_PLAYER;
}

void CBasePlayer::AddPoints( int score, BOOL bAllowNegativeScore )
{
	// Positive score always adds
	if ( score < 0 )
	{
		if ( !bAllowNegativeScore )
		{
			if ( pev->frags < 0 )		// Can't go more negative
				return;
			
			if ( -score > pev->frags )	// Will this go negative?
			{
				score = -pev->frags;		// Sum will be 0
			}
		}
	}

	pev->frags += score;

	MESSAGE_BEGIN( MSG_ALL, gmsgScoreInfo );
		WRITE_BYTE( ENTINDEX(edict()) );
		WRITE_SHORT( pev->frags );
		WRITE_SHORT( m_iDeaths );
		WRITE_SHORT( 0 );
		WRITE_SHORT( g_pGameRules->GetTeamIndex( m_szTeamName ) + 1 );
	MESSAGE_END();
}

void CBasePlayer::AddPointsToTeam( int score, BOOL bAllowNegativeScore )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

		if (pPlayer && IRelationship( pPlayer ) == R_AL )
		{
			pPlayer->AddPoints( score, bAllowNegativeScore );
		}
	}
}

//Player ID
void CBasePlayer::InitStatusBar()
{
	m_flStatusBarDisappearDelay = 0;
	m_SbarString1[0] = m_SbarString0[0] = 0; 
}

void CBasePlayer::UpdateStatusBar()
{
	int newSBarState[ SBAR_END ];
	char sbuf0[ SBAR_STRING_SIZE ];
	char sbuf1[ SBAR_STRING_SIZE ];

	memset( newSBarState, 0, sizeof(newSBarState) );
	strcpy_safe( sbuf0, m_SbarString0, SBAR_STRING_SIZE);
	strcpy_safe( sbuf1, m_SbarString1, SBAR_STRING_SIZE);

	// Find an ID Target
	TraceResult tr;
	UTIL_MakeVectors( pev->v_angle + pev->punchangle );
	Vector vecSrc = EyePosition();
	Vector vecEnd = vecSrc + (gpGlobals->v_forward * MAX_ID_RANGE);

	int oldSolid = pev->solid; // not just passing edict() to trace so we can see owned items (e.g. dropped shockroach)
	pev->solid = SOLID_NOT;
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, NULL, &tr);
	pev->solid = oldSolid;

	if (tr.flFraction != 1.0)
	{
		if ( !FNullEnt( tr.pHit ) )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

			if (pEntity->Classify() == CLASS_PLAYER )
			{
				newSBarState[ SBAR_ID_TARGETNAME ] = ENTINDEX( pEntity->edict() );
				strcpy_safe( sbuf1, "1 %p1", SBAR_STRING_SIZE );
				strcpy_safe( sbuf0, "2 Health: %i2\n3 Armor: %i3", SBAR_STRING_SIZE);

				newSBarState[ SBAR_ID_TARGETHEALTH ] = pEntity->pev->health;
				newSBarState[ SBAR_ID_TARGETARMOR ] = pEntity->pev->armorvalue;

				m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
			}
			else if (pEntity->IsMonster() && pEntity->IsAlive()) {
				
				std::string name = replaceString(pEntity->DisplayName(), "\n", " ");
				int hp = roundf(pEntity->pev->health);

				int irel = IRelationship(pEntity);

				const char* srel = "";

				if (irel == R_AL) {
					srel = "Friend: ";
				}
				else if (irel == R_DL || irel == R_HT || irel == R_NM) {
					srel = "Enemy: ";
				}

				strcpy_safe(sbuf1, UTIL_VarArgs("1 %s%s", srel, name.c_str()), SBAR_STRING_SIZE);

				if ((pEntity->pev->flags & FL_GODMODE) || (pEntity->pev->takedamage == DAMAGE_NO) || pEntity->pev->health > 2147483647) {
					strcpy_safe(sbuf0, "2 Health: Invincible", SBAR_STRING_SIZE);
					hp = 1; // client won't show health text if this is an insane value
				}
				else if (hp == 0) {
					strcpy_safe(sbuf0, "2 Health: 0", SBAR_STRING_SIZE);
					hp = 1; // client won't show health text if this is 0
				}
				else {
					strcpy_safe(sbuf0, UTIL_VarArgs("2 Health: %d", hp), SBAR_STRING_SIZE);
				}

				newSBarState[SBAR_ID_TARGETNAME] = ENTINDEX(pEntity->edict());
				newSBarState[SBAR_ID_TARGETHEALTH] = hp;

				m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
			}
			else if (pEntity->IsBreakable() && !(pEntity->pev->spawnflags & SF_BREAK_TRIGGER_ONLY)) {

				std::string name = replaceString(pEntity->DisplayName(), "\n", " ");
				int hp = roundf(pEntity->pev->health);

				const char* hint = "";
				if (IRelationship(pEntity) == R_AL) {
					hint = " (wrench repairs)";
				}
				else if (pEntity->pev->spawnflags & SF_BREAK_EXPLOSIVES_ONLY) {
					hint = " (explosives only)";
				}
				else if (pEntity->pev->spawnflags & SF_BREAK_INSTANT) {
					CBreakable* breakable = (CBreakable*)pEntity;

					if (breakable->m_instantBreakWeapon == BREAK_INSTANT_WRENCH) {
						hint = " (use wrench)";
					}
					else {
						hint = " (use crowbar)";
					}
				}

				strcpy_safe(sbuf1, UTIL_VarArgs("1 %s%s", name.c_str(), hint), SBAR_STRING_SIZE);
				strcpy_safe(sbuf0, UTIL_VarArgs("2 Health: %d", hp), SBAR_STRING_SIZE);

				newSBarState[SBAR_ID_TARGETNAME] = ENTINDEX(pEntity->edict());
				newSBarState[SBAR_ID_TARGETHEALTH] = hp;

				m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
			}
			else if (FClassnameIs(pEntity->pev, "func_pushable")) {

				std::string name = replaceString(pEntity->DisplayName(), "\n", " ");

				const char* hint = "";
				if (pEntity->pev->spawnflags & SF_PUSH_LIFTABLE) {
					strcpy_safe(sbuf0, "2 Press USE key to lift", SBAR_STRING_SIZE);
				}
				else {
					strcpy_safe(sbuf0, "2 Cannot lift", SBAR_STRING_SIZE);
				}

				strcpy_safe(sbuf1, UTIL_VarArgs("1 %s%s", name.c_str(), hint), SBAR_STRING_SIZE);

				newSBarState[SBAR_ID_TARGETNAME] = ENTINDEX(pEntity->edict());
				newSBarState[SBAR_ID_TARGETHEALTH] = 1;

				m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
			}
		}
		else if ( m_flStatusBarDisappearDelay > gpGlobals->time )
		{
			// hold the values for a short amount of time after viewing the object
			newSBarState[ SBAR_ID_TARGETNAME ] = m_izSBarState[ SBAR_ID_TARGETNAME ];
			newSBarState[ SBAR_ID_TARGETHEALTH ] = m_izSBarState[ SBAR_ID_TARGETHEALTH ];
			newSBarState[ SBAR_ID_TARGETARMOR ] = m_izSBarState[ SBAR_ID_TARGETARMOR ];
		}
	}

	BOOL bForceResend = FALSE;

	if ( strncmp( sbuf0, m_SbarString0, SBAR_STRING_SIZE) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStatusText, NULL, pev );
			WRITE_BYTE( 0 );
			WRITE_STRING( sbuf0 );
		MESSAGE_END();

		strcpy_safe( m_SbarString0, sbuf0, SBAR_STRING_SIZE);

		// make sure everything's resent
		bForceResend = TRUE;
	}

	if ( strncmp( sbuf1, m_SbarString1, SBAR_STRING_SIZE) )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgStatusText, NULL, pev );
			WRITE_BYTE( 1 );
			WRITE_STRING( sbuf1 );
		MESSAGE_END();

		strcpy_safe( m_SbarString1, sbuf1, SBAR_STRING_SIZE);

		// make sure everything's resent
		bForceResend = TRUE;
	}

	// Check values and send if they don't match
	for (int i = 1; i < SBAR_END; i++)
	{
		if ( newSBarState[i] != m_izSBarState[i] || bForceResend )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgStatusValue, NULL, pev );
				WRITE_BYTE( i );
				WRITE_SHORT( newSBarState[i] );
			MESSAGE_END();

			m_izSBarState[i] = newSBarState[i];
		}
	}
}

void CBasePlayer::PreThink(void)
{
	int buttonsChanged = (m_afButtonLast ^ pev->button);	// These buttons have changed this frame
	
	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed =  buttonsChanged & pev->button;		// The changed ones still down are "pressed"
	m_afButtonReleased = buttonsChanged & (~pev->button);	// The ones not down are "released"

	g_pGameRules->PlayerThink( this );

	if (m_initSoundTime && gpGlobals->time >= m_initSoundTime) {
		m_initSoundTime = 0;
		edict_t* ent = NULL;

		while (!FNullEnt(ent = FIND_ENTITY_BY_CLASSNAME(ent, "ambient_generic"))) {
			CAmbientGeneric* ambient = (CAmbientGeneric*)GET_PRIVATE(ent);
			if (ambient)
				ambient->InitSoundForNewJoiner(edict());
		}

		ent = NULL;
		while (!FNullEnt(ent = FIND_ENTITY_BY_CLASSNAME(ent, "ambient_music"))) {
			CAmbientGeneric* ambient = (CAmbientGeneric*)GET_PRIVATE(ent);
			if (ambient)
				ambient->InitSoundForNewJoiner(edict());
		}
	}

	UpdateShockEffect();

	if ( g_fGameOver )
		return;         // intermission or finale

	UTIL_MakeVectors(pev->v_angle);             // is this still used?
	
	ItemPreFrame( );
	WaterMove();

	if ( g_pGameRules && g_pGameRules->FAllowFlashlight() )
		m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
	else
		m_iHideHUD |= HIDEHUD_FLASHLIGHT;


	// JOHN: checks if new client data (for HUD and view control) needs to be sent to the client
	UpdateClientData();
	
	CheckTimeBasedDamage();

	CheckSuitUpdate();

	// hide weapon hud if player has no weapons
	if (pev->weapons == (1 << WEAPON_SUIT)) {
		m_iHideHUD |= HIDEHUD_WEAPONS;
	}
	else {
		m_iHideHUD &= ~HIDEHUD_WEAPONS;
	}
	

	// Observer Button Handling
	if ( IsObserver() )
	{
		Observer_HandleButtons();
		Observer_CheckTarget();
		Observer_CheckProperties();
		pev->impulse = 0;
		return;
	}

	if (pev->deadflag >= DEAD_DYING)
	{
		PlayerDeathThink();
		return;
	}

	// So the correct flags get sent to client asap.
	//
	if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
		pev->flags |= FL_ONTRAIN;
	else 
		pev->flags &= ~FL_ONTRAIN;

	// Train speed control
	if ( m_afPhysicsFlags & PFLAG_ONTRAIN )
	{
		CBaseEntity *pTrain = CBaseEntity::Instance( pev->groundentity );
		float vel;
		
		if ( !pTrain )
		{
			TraceResult trainTrace;
			// Maybe this is on the other side of a level transition
			UTIL_TraceLine( pev->origin, pev->origin + Vector(0,0,-38), ignore_monsters, ENT(pev), &trainTrace );

			// HACKHACK - Just look for the func_tracktrain classname
			if ( trainTrace.flFraction != 1.0 && trainTrace.pHit )
			pTrain = CBaseEntity::Instance( trainTrace.pHit );


			if ( !pTrain || !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) || !pTrain->OnControls(pev) )
			{
				//ALERT( at_error, "In train mode with no train!\n" );
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				m_iTrain = TRAIN_NEW|TRAIN_OFF;
				return;
			}
		}
		else if ( !FBitSet( pev->flags, FL_ONGROUND ) || FBitSet( pTrain->pev->spawnflags, SF_TRACKTRAIN_NOCONTROL ) || (pev->button & (IN_MOVELEFT|IN_MOVERIGHT) ) )
		{
			// Turn off the train if you jump, strafe, or the train controls go dead
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			m_iTrain = TRAIN_NEW|TRAIN_OFF;
			return;
		}

		pev->velocity = g_vecZero;
		vel = 0;
		if ( m_afButtonPressed & IN_FORWARD )
		{
			vel = 1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}
		else if ( m_afButtonPressed & IN_BACK )
		{
			vel = -1;
			pTrain->Use( this, this, USE_SET, (float)vel );
		}

		if (vel)
		{
			m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
			m_iTrain |= TRAIN_ACTIVE|TRAIN_NEW;
		}

	} else if (m_iTrain & TRAIN_ACTIVE)
		m_iTrain = TRAIN_NEW; // turn off train

	if (pev->button & IN_JUMP)
	{
		// If on a ladder, jump off the ladder
		// else Jump
		Jump();
	}


	// If trying to duck, already ducked, or in the process of ducking
	if ((pev->button & IN_DUCK) || FBitSet(pev->flags,FL_DUCKING) || (m_afPhysicsFlags & PFLAG_DUCKING) )
		Duck();

	if ( !FBitSet ( pev->flags, FL_ONGROUND ) )
	{
		m_flFallVelocity = -pev->velocity.z;
	}

	// StudioFrameAdvance( );//!!!HACKHACK!!! Can't be hit by traceline when not animating?

	// Clear out ladder pointer
	m_hEnemy = NULL;

	if ( m_afPhysicsFlags & PFLAG_ONBARNACLE )
	{
		pev->velocity = g_vecZero;
	}
}

/* Time based Damage works as follows: 
	1) There are several types of timebased damage:

		#define DMG_PARALYZE		(1 << 14)	// slows affected creature down
		#define DMG_NERVEGAS		(1 << 15)	// nerve toxins, very bad
		#define DMG_POISON			(1 << 16)	// blood poisioning
		#define DMG_RADIATION		(1 << 17)	// radiation exposure
		#define DMG_DROWNRECOVER	(1 << 18)	// drown recovery
		#define DMG_ACID			(1 << 19)	// toxic chemicals or acid burns
		#define DMG_SLOWBURN		(1 << 20)	// in an oven
		#define DMG_SLOWFREEZE		(1 << 21)	// in a subzero freezer

	2) A new hit inflicting tbd restarts the tbd counter - each monster has an 8bit counter,
		per damage type. The counter is decremented every second, so the maximum time 
		an effect will last is 255/60 = 4.25 minutes.  Of course, staying within the radius
		of a damaging effect like fire, nervegas, radiation will continually reset the counter to max.

	3) Every second that a tbd counter is running, the player takes damage.  The damage
		is determined by the type of tdb.  
			Paralyze		- 1/2 movement rate, 30 second duration.
			Nervegas		- 5 points per second, 16 second duration = 80 points max dose.
			Poison			- 2 points per second, 25 second duration = 50 points max dose.
			Radiation		- 1 point per second, 50 second duration = 50 points max dose.
			Drown			- 5 points per second, 2 second duration.
			Acid/Chemical	- 5 points per second, 10 second duration = 50 points max.
			Burn			- 10 points per second, 2 second duration.
			Freeze			- 3 points per second, 10 second duration = 30 points max.

	4) Certain actions or countermeasures counteract the damaging effects of tbds:

		Armor/Heater/Cooler - Chemical(acid),burn, freeze all do damage to armor power, then to body
							- recharged by suit recharger
		Air In Lungs		- drowning damage is done to air in lungs first, then to body
							- recharged by poking head out of water
							- 10 seconds if swiming fast
		Air In SCUBA		- drowning damage is done to air in tanks first, then to body
							- 2 minutes in tanks. Need new tank once empty.
		Radiation Syringe	- Each syringe full provides protection vs one radiation dosage
		Antitoxin Syringe	- Each syringe full provides protection vs one poisoning (nervegas or poison).
		Health kit			- Immediate stop to acid/chemical, fire or freeze damage.
		Radiation Shower	- Immediate stop to radiation damage, acid/chemical or fire damage.
		
	
*/

// If player is taking time based damage, continue doing damage to player -
// this simulates the effect of being poisoned, gassed, dosed with radiation etc -
// anything that continues to do damage even after the initial contact stops.
// Update all time based damage counters, and shut off any that are done.

// The m_bitsDamageType bit MUST be set if any damage is to be taken.
// This routine will detect the initial on value of the m_bitsDamageType
// and init the appropriate counter.  Only processes damage every second.

//#define PARALYZE_DURATION	30		// number of 2 second intervals to take damage
//#define PARALYZE_DAMAGE		0.0		// damage to take each 2 second interval
/* */
void CBasePlayer::CheckTimeBasedDamage() 
{
	int i;
	BYTE bDuration = 0;

	if (!(m_bitsDamageType & DMG_TIMEBASED))
		return;

	// only check for time based damage approx. every 2 seconds
	if ( fabs(gpGlobals->time - m_tbdPrev) < 2.0)
		return;
	
	m_tbdPrev = gpGlobals->time;

	for (i = 0; i < CDMG_TIMEBASED; i++)
	{
		// make sure bit is set for damage type
		if (m_bitsDamageType & (DMG_PARALYZE << i))
		{
			switch (i)
			{
			case itbd_Paralyze:
				// UNDONE - flag movement as half-speed
				bDuration = PARALYZE_DURATION;
				break;
			case itbd_NerveGas:
//				TakeDamage(pev, pev, NERVEGAS_DAMAGE, DMG_GENERIC);	
				bDuration = NERVEGAS_DURATION;
				break;
			case itbd_Poison:
				TakeDamage(pev, pev, POISON_DAMAGE, DMG_GENERIC);
				bDuration = POISON_DURATION;
				break;
			case itbd_Radiation:
//				TakeDamage(pev, pev, RADIATION_DAMAGE, DMG_GENERIC);
				bDuration = RADIATION_DURATION;
				break;
			case itbd_DrownRecover:
				// NOTE: this hack is actually used to RESTORE health
				// after the player has been drowning and finally takes a breath
				if (m_idrowndmg > m_idrownrestored)
				{
					int idif = V_min(m_idrowndmg - m_idrownrestored, 10);

					TakeHealth(idif, DMG_GENERIC);
					m_idrownrestored += idif;
				}
				bDuration = 4;	// get up to 5*10 = 50 points back
				break;
			case itbd_Acid:
//				TakeDamage(pev, pev, ACID_DAMAGE, DMG_GENERIC);
				bDuration = ACID_DURATION;
				break;
			case itbd_SlowBurn:
//				TakeDamage(pev, pev, SLOWBURN_DAMAGE, DMG_GENERIC);
				bDuration = SLOWBURN_DURATION;
				break;
			case itbd_SlowFreeze:
//				TakeDamage(pev, pev, SLOWFREEZE_DAMAGE, DMG_GENERIC);
				bDuration = SLOWFREEZE_DURATION;
				break;
			default:
				bDuration = 0;
			}

			if (m_rgbTimeBasedDamage[i])
			{
				// use up an antitoxin on poison or nervegas after a few seconds of damage					
				if (((i == itbd_NerveGas) && (m_rgbTimeBasedDamage[i] < NERVEGAS_DURATION)) ||
					((i == itbd_Poison)   && (m_rgbTimeBasedDamage[i] < POISON_DURATION)))
				{
					if (m_rgItems[ITEM_ANTIDOTE])
					{
						m_rgbTimeBasedDamage[i] = 0;
						m_rgItems[ITEM_ANTIDOTE]--;
						SetSuitUpdate("!HEV_HEAL4", FALSE, SUIT_REPEAT_OK);
					}
				}


				// decrement damage duration, detect when done.
				if (!m_rgbTimeBasedDamage[i] || --m_rgbTimeBasedDamage[i] == 0)
				{
					m_rgbTimeBasedDamage[i] = 0;
					// if we're done, clear damage bits
					m_bitsDamageType &= ~(DMG_PARALYZE << i);	
				}
			}
			else
				// first time taking this damage type - init damage duration
				m_rgbTimeBasedDamage[i] = bDuration;
		}
	}
}

/*
THE POWER SUIT

The Suit provides 3 main functions: Protection, Notification and Augmentation. 
Some functions are automatic, some require power. 
The player gets the suit shortly after getting off the train in C1A0 and it stays
with him for the entire game.

Protection

	Heat/Cold
		When the player enters a hot/cold area, the heating/cooling indicator on the suit 
		will come on and the battery will drain while the player stays in the area. 
		After the battery is dead, the player starts to take damage. 
		This feature is built into the suit and is automatically engaged.
	Radiation Syringe
		This will cause the player to be immune from the effects of radiation for N seconds. Single use item.
	Anti-Toxin Syringe
		This will cure the player from being poisoned. Single use item.
	Health
		Small (1st aid kits, food, etc.)
		Large (boxes on walls)
	Armor
		The armor works using energy to create a protective field that deflects a
		percentage of damage projectile and explosive attacks. After the armor has been deployed,
		it will attempt to recharge itself to full capacity with the energy reserves from the battery.
		It takes the armor N seconds to fully charge. 

Notification (via the HUD)

x	Health
x	Ammo  
x	Automatic Health Care
		Notifies the player when automatic healing has been engaged. 
x	Geiger counter
		Classic Geiger counter sound and status bar at top of HUD 
		alerts player to dangerous levels of radiation. This is not visible when radiation levels are normal.
x	Poison
	Armor
		Displays the current level of armor. 

Augmentation 

	Reanimation (w/adrenaline)
		Causes the player to come back to life after he has been dead for 3 seconds. 
		Will not work if player was gibbed. Single use.
	Long Jump
		Used by hitting the ??? key(s). Caused the player to further than normal.
	SCUBA	
		Used automatically after picked up and after player enters the water. 
		Works for N seconds. Single use.	
	
Things powered by the battery

	Armor		
		Uses N watts for every M units of damage.
	Heat/Cool	
		Uses N watts for every second in hot/cold area.
	Long Jump	
		Uses N watts for every jump.
	Alien Cloak	
		Uses N watts for each use. Each use lasts M seconds.
	Alien Shield	
		Augments armor. Reduces Armor drain by one half
 
*/

// if in range of radiation source, ping geiger counter
void CBasePlayer :: UpdateGeigerCounter( void )
{
	BYTE range;

	// delay per update ie: don't flood net with these msgs
	if (gpGlobals->time < m_flgeigerDelay)
		return;

	m_flgeigerDelay = gpGlobals->time + GEIGERDELAY;
		
	// send range to radition source to client

	range = (BYTE) (m_flgeigerRange / 4);

	if (range != m_igeigerRangePrev)
	{
		m_igeigerRangePrev = range;

		MESSAGE_BEGIN( MSG_ONE, gmsgGeigerRange, NULL, pev );
			WRITE_BYTE( range );
		MESSAGE_END();
	}

	// reset counter and semaphore
	if (!RANDOM_LONG(0,3))
		m_flgeigerRange = 1000;

}

/*
================
CheckSuitUpdate

Play suit update if it's time
================
*/
void CBasePlayer::CheckSuitUpdate()
{
	int i;
	int isentence = 0;
	int isearch = m_iSuitPlayNext;
	
	// Ignore suit updates if no suit
	if ( !(pev->weapons & (1<<WEAPON_SUIT)) )
		return;

	// if in range of radiation source, ping geiger counter
	UpdateGeigerCounter();

	if ( g_pGameRules->IsMultiplayer() )
	{
		// don't bother updating HEV voice in multiplayer.
		return;
	}

	if ( gpGlobals->time >= m_flSuitUpdate && m_flSuitUpdate > 0)
	{
		// play a sentence off of the end of the queue
		for (i = 0; i < CSUITPLAYLIST; i++)
		{
			if ((isentence = m_rgSuitPlayList[isearch]) != 0)
				break;
			
			if (++isearch == CSUITPLAYLIST)
				isearch = 0;
		}

		if (isentence)
		{
			m_rgSuitPlayList[isearch] = 0;
			if (isentence > 0)
			{
				// play sentence number

				char sentence[CBSENTENCENAME_MAX+1];
				strcpy_safe(sentence, "!", CBSENTENCENAME_MAX + 1);
				strcat_safe(sentence, gszallsentencenames[isentence], CBSENTENCENAME_MAX + 1);
				EMIT_SOUND_SUIT(ENT(pev), sentence);
			}
			else
			{
				// play sentence group
				EMIT_GROUPID_SUIT(ENT(pev), -isentence);
			}
		m_flSuitUpdate = gpGlobals->time + SUITUPDATETIME;
		}
		else
			// queue is empty, don't check 
			m_flSuitUpdate = 0;
	}
}
 
// add sentence to suit playlist queue. if fgroup is true, then
// name is a sentence group (HEV_AA), otherwise name is a specific
// sentence name ie: !HEV_AA0.  If iNoRepeat is specified in
// seconds, then we won't repeat playback of this word or sentence
// for at least that number of seconds.
void CBasePlayer::SetSuitUpdate(const char *name, int fgroup, int iNoRepeatTime)
{
	int i;
	int isentence;
	int iempty = -1;
	
	
	// Ignore suit updates if no suit
	if ( !(pev->weapons & (1<<WEAPON_SUIT)) )
		return;

	if ( g_pGameRules->IsMultiplayer() )
	{
		// due to static channel design, etc. We don't play HEV sounds in multiplayer right now.
		return;
	}

	// if name == NULL, then clear out the queue

	if (!name)
	{
		for (i = 0; i < CSUITPLAYLIST; i++)
			m_rgSuitPlayList[i] = 0;
		return;
	}
	// get sentence or group number
	if (!fgroup)
	{
		isentence = SENTENCEG_Lookup(name, NULL, 0);
		if (isentence < 0)
			return;
	}
	else
		// mark group number as negative
		isentence = -SENTENCEG_GetIndex(name);

	// check norepeat list - this list lets us cancel
	// the playback of words or sentences that have already
	// been played within a certain time.

	for (i = 0; i < CSUITNOREPEAT; i++)
	{
		if (isentence == m_rgiSuitNoRepeat[i])
			{
			// this sentence or group is already in 
			// the norepeat list

			if (m_rgflSuitNoRepeatTime[i] < gpGlobals->time)
				{
				// norepeat time has expired, clear it out
				m_rgiSuitNoRepeat[i] = 0;
				m_rgflSuitNoRepeatTime[i] = 0.0;
				iempty = i;
				break;
				}
			else
				{
				// don't play, still marked as norepeat
				return;
				}
			}
		// keep track of empty slot
		if (!m_rgiSuitNoRepeat[i])
			iempty = i;
	}

	// sentence is not in norepeat list, save if norepeat time was given

	if (iNoRepeatTime)
	{
		if (iempty < 0)
			iempty = RANDOM_LONG(0, CSUITNOREPEAT-1); // pick random slot to take over
		m_rgiSuitNoRepeat[iempty] = isentence;
		m_rgflSuitNoRepeatTime[iempty] = iNoRepeatTime + gpGlobals->time;
	}

	// find empty spot in queue, or overwrite last spot
	
	m_rgSuitPlayList[m_iSuitPlayNext++] = isentence;
	if (m_iSuitPlayNext == CSUITPLAYLIST)
		m_iSuitPlayNext = 0;

	if (m_flSuitUpdate <= gpGlobals->time)
	{
		if (m_flSuitUpdate == 0)
			// play queue is empty, don't delay too long before playback
			m_flSuitUpdate = gpGlobals->time + SUITFIRSTUPDATETIME;
		else 
			m_flSuitUpdate = gpGlobals->time + SUITUPDATETIME; 
	}

}

/*
================
CheckPowerups

Check for turning off powerups

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
================
*/
	static void
CheckPowerups(entvars_t *pev)
{
	if (pev->health <= 0)
		return;

	pev->modelindex = g_ulModelIndexPlayer;    // don't use eyes
}

//=========================================================
// UpdatePlayerSound - updates the position of the player's
// reserved sound slot in the sound list.
//=========================================================
void CBasePlayer :: UpdatePlayerSound ( void )
{
	int iBodyVolume;
	int iVolume;
	CSound *pSound;

	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt :: ClientSoundIndex( edict() ) );

	if ( !pSound )
	{
		ALERT ( at_console, "Client lost reserved sound!\n" );
		return;
	}

	pSound->m_iType = bits_SOUND_NONE;

	// now calculate the best target volume for the sound. If the player's weapon
	// is louder than his body/movement, use the weapon volume, else, use the body volume.
	
	if ( FBitSet ( pev->flags, FL_ONGROUND ) )
	{	
		iBodyVolume = pev->velocity.Length(); 

		// clamp the noise that can be made by the body, in case a push trigger,
		// weapon recoil, or anything shoves the player abnormally fast. 
		if ( iBodyVolume > 512 )
		{
			iBodyVolume = 512;
		}
	}
	else
	{
		iBodyVolume = 0;
	}

	if ( pev->button & IN_JUMP )
	{
		iBodyVolume += 100;
	}

// convert player move speed and actions into sound audible by monsters.
	if ( m_iWeaponVolume > iBodyVolume )
	{
		m_iTargetVolume = m_iWeaponVolume;

		// OR in the bits for COMBAT sound if the weapon is being louder than the player. 
		pSound->m_iType |= bits_SOUND_COMBAT;
	}
	else
	{
		m_iTargetVolume = iBodyVolume;
	}

	// decay weapon volume over time so bits_SOUND_COMBAT stays set for a while
	m_iWeaponVolume -= 250 * gpGlobals->frametime;
	if ( m_iWeaponVolume < 0 )
	{
		iVolume = 0;
	}


	// if target volume is greater than the player sound's current volume, we paste the new volume in 
	// immediately. If target is less than the current volume, current volume is not set immediately to the
	// lower volume, rather works itself towards target volume over time. This gives monsters a much better chance
	// to hear a sound, especially if they don't listen every frame.
	iVolume = pSound->m_iVolume;

	if ( m_iTargetVolume > iVolume )
	{
		iVolume = m_iTargetVolume;
	}
	else if ( iVolume > m_iTargetVolume )
	{
		iVolume -= 250 * gpGlobals->frametime;

		if ( iVolume < m_iTargetVolume )
		{
			iVolume = 0;
		}
	}

	if ( m_fNoPlayerSound )
	{
		// debugging flag, lets players move around and shoot without monsters hearing.
		iVolume = 0;
	}

	if ( gpGlobals->time > m_flStopExtraSoundTime )
	{
		// since the extra sound that a weapon emits only lasts for one client frame, we keep that sound around for a server frame or two 
		// after actual emission to make sure it gets heard.
		m_iExtraSoundTypes = 0;
	}

	if ( pSound )
	{
		pSound->m_vecOrigin = pev->origin;
		pSound->m_iType |= ( bits_SOUND_PLAYER | m_iExtraSoundTypes );
		pSound->m_iVolume = iVolume;
	}

	// keep track of virtual muzzle flash
	m_iWeaponFlash -= 256 * gpGlobals->frametime;
	if (m_iWeaponFlash < 0)
		m_iWeaponFlash = 0;

	//UTIL_MakeVectors ( pev->angles );
	//gpGlobals->v_forward.z = 0;

	// Below are a couple of useful little bits that make it easier to determine just how much noise the 
	// player is making. 
	// UTIL_ParticleEffect ( pev->origin + gpGlobals->v_forward * iVolume, g_vecZero, 255, 25 );
	//ALERT ( at_console, "%d/%d\n", iVolume, m_iTargetVolume );
}

void CBasePlayer::PostThink()
{
	if ( g_fGameOver )
		goto pt_end;         // intermission or finale

	if (!IsAlive())
		goto pt_end;

	// Handle Tank controlling
	if ( m_pTank != NULL )
	{ // if they've moved too far from the gun,  or selected a weapon, unuse the gun
		if ( m_pTank->OnControls( pev ) && !pev->weaponmodel )
		{  
			m_pTank->Use( this, this, USE_SET, 2 );	// try fire the gun
		}
		else
		{  // they've moved off the platform
			m_pTank->Use( this, this, USE_OFF, 0 );
			m_pTank = NULL;
		}
	}

// do weapon stuff
	ItemPostFrame( );

// check to see if player landed hard enough to make a sound
// falling farther than half of the maximum safe distance, but not as far a max safe distance will
// play a bootscrape sound, and no damage will be inflicted. Fallling a distance shorter than half
// of maximum safe distance will make no sound. Falling farther than max safe distance will play a 
// fallpain sound, and damage will be inflicted based on how far the player fell

	if ( (FBitSet(pev->flags, FL_ONGROUND)) && (pev->health > 0) && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD )
	{
		// ALERT ( at_console, "%f\n", m_flFallVelocity );

		if (pev->watertype == CONTENT_WATER)
		{
			// Did he hit the world or a non-moving entity?
			// BUG - this happens all the time in water, especially when 
			// BUG - water has current force
			// if ( !pev->groundentity || VARS(pev->groundentity)->velocity.z == 0 )
				// EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_wade1.wav", 1, ATTN_NORM);
		}
		else if ( m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED )
		{// after this point, we start doing damage
			
			float flFallDamage = g_pGameRules->FlPlayerFallDamage( this );

			TraceResult tr;
			int hullType = (pev->flags & FL_DUCKING) ? head_hull : human_hull;
			TRACE_HULL(pev->origin, pev->origin + Vector(0,0,-1), dont_ignore_monsters, hullType, edict(), &tr);

			// split fall damage with self and whatever ent was landed on
			CBaseEntity* ent = CBaseEntity::Instance(tr.pHit);
			if (ent->IsMonster()) {
				flFallDamage *= 0.5f;

				if (ent->IsPlayer()) {
					ent->pev->punchangle.x += 4 + 12 * (V_min(100, flFallDamage) / 100.0f);
					UTIL_ScreenShake(ent->pev->origin, 255.0f, 255.0f, 0.5f, 1.0f);
				}
				
				// bypass friendly fire checks in case target is blocking
				// (e.g. player sitting at the bottom of a deep tunnel with no way to avoid them)
				ent->pev->health -= flFallDamage;
				if (ent->pev->health <= 0) {
					ent->Killed(pev, GIB_NORMAL);
				}
			}

			if ( flFallDamage > pev->health )
			{//splat
				// note: play on item channel because we play footstep landing on body channel
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/bodysplat.wav", 1, ATTN_NORM);
			}

			if ( flFallDamage > 0 )
			{
				TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), flFallDamage, DMG_FALL ); 
				pev->punchangle.x = 0;
			}
		}

		if ( IsAlive() )
		{
			SetAnimation( PLAYER_WALK );
		}
    }

	if (FBitSet(pev->flags, FL_ONGROUND))
	{		
		if (m_flFallVelocity > 64 && !g_pGameRules->IsMultiplayer())
		{
			CSoundEnt::InsertSound ( bits_SOUND_PLAYER, pev->origin, m_flFallVelocity, 0.2 );
			// ALERT( at_console, "fall %f\n", m_flFallVelocity );
		}
		m_flFallVelocity = 0;
	}

	// select the proper animation for the player character	
	if ( IsAlive() )
	{
		if (!pev->velocity.x && !pev->velocity.y)
			SetAnimation( PLAYER_IDLE );
		else if ((pev->velocity.x || pev->velocity.y) && (FBitSet(pev->flags, FL_ONGROUND)))
			SetAnimation( PLAYER_WALK );
		else if (pev->waterlevel > 1)
			SetAnimation( PLAYER_WALK );
	}

	StudioFrameAdvance( );
	CheckPowerups(pev);

	UpdatePlayerSound();

	//UpdateMonsterInfo();
	UpdateScore();

pt_end:
#if defined( CLIENT_WEAPONS )
		// Decay timers on weapons
	// go through all of the weapons and make a list of the ones to pack
	for ( int i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			CBasePlayerItem *pPlayerItem = (CBasePlayerItem*)m_rgpPlayerItems[i].GetEntity();

			while ( pPlayerItem )
			{
				CBasePlayerWeapon *gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();
				
				if ( gun && gun->UseDecrement() )
				{
					gun->m_flNextPrimaryAttack		= V_max( gun->m_flNextPrimaryAttack - gpGlobals->frametime, -1.0f );
					gun->m_flNextSecondaryAttack	= V_max( gun->m_flNextSecondaryAttack - gpGlobals->frametime, -0.001f );

					if ( gun->m_flTimeWeaponIdle != 1000 )
					{
						gun->m_flTimeWeaponIdle		= V_max( gun->m_flTimeWeaponIdle - gpGlobals->frametime, -0.001f );
					}

					if ( gun->pev->fuser1 != 1000 )
					{
						gun->pev->fuser1	= V_max( gun->pev->fuser1 - gpGlobals->frametime, -0.001f );
					}

					// Only decrement if not flagged as NO_DECREMENT
//					if ( gun->m_flPumpTime != 1000 )
				//	{
				//		gun->m_flPumpTime	= V_max( gun->m_flPumpTime - gpGlobals->frametime, -0.001f );
				//	}
					
				}

				pPlayerItem = (CBasePlayerItem*)pPlayerItem->m_pNext.GetEntity();
			}
		}
	}

	m_flNextAttack -= gpGlobals->frametime;
	if ( m_flNextAttack < -0.001 )
		m_flNextAttack = -0.001;
	
	if ( m_flNextAmmoBurn != 1000 )
	{
		m_flNextAmmoBurn -= gpGlobals->frametime;
		
		if ( m_flNextAmmoBurn < -0.001 )
			m_flNextAmmoBurn = -0.001;
	}

	if ( m_flAmmoStartCharge != 1000 )
	{
		m_flAmmoStartCharge -= gpGlobals->frametime;
		
		if ( m_flAmmoStartCharge < -0.001 )
			m_flAmmoStartCharge = -0.001;
	}
#endif

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = pev->button;
	pev->oldorigin = pev->origin; // for func_clip
}

void CBasePlayer::Spawn( void )
{
	m_flStartCharge = gpGlobals->time;

	pev->classname		= MAKE_STRING("player");
	pev->health			= 100;
	pev->armorvalue		= 0;
	pev->takedamage		= DAMAGE_AIM;
	pev->solid			= SOLID_SLIDEBOX;
	pev->movetype		= MOVETYPE_WALK;
	pev->health			= mp_starthealth.value;
	pev->armorvalue		= mp_startarmor.value;
	pev->max_health		= pev->health;
	pev->flags		   &= FL_PROXY;	// keep proxy flag sey by engine
	pev->flags		   |= FL_CLIENT;
	pev->air_finished	= gpGlobals->time + 12;
	pev->dmg			= 2;				// initial water damage
	pev->effects		= 0;
	pev->deadflag		= DEAD_NO;
	pev->dmg_take		= 0;
	pev->dmg_save		= 0;
	pev->friction		= 1.0;
	pev->gravity		= 1.0;
	pev->rendermode = 0;
	pev->renderamt = 0;
	pev->renderfx = 0;
	pev->rendercolor = Vector(0,0,0);
	m_lastDropTime = 0;
	memset(m_nextItemPickups, 0, sizeof(float) * MAX_WEAPONS);

	if( pev->iuser1 != OBS_NONE )
		LeaveObserver();

	m_bitsHUDDamage		= -1;
	m_bitsDamageType	= 0;
	m_afPhysicsFlags	= 0;
	m_fLongJump			= FALSE;// no longjump module. 

	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );
	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "hl", "1" );

	pev->fov = m_iFOV				= 0;// init field of view.
	m_iClientFOV		= -1; // make sure fov reset is sent

	m_flNextDecalTime	= 0;// let this player decal as soon as he spawns.

	m_flgeigerDelay = gpGlobals->time + 2.0;	// wait a few seconds until user-defined message registrations
												// are recieved by all clients
	
	m_flTimeStepSound	= 0;
	m_iStepLeft = 0;
	m_flFieldOfView		= 0.5;// some monsters use this to determine whether or not the player is looking at them.

	m_bloodColor	= BLOOD_COLOR_RED;
	m_flNextAttack	= UTIL_WeaponTimeBase();
	StartSneaking();

	m_iFlashBattery = 99;
	m_flFlashLightTime = 1; // force first message

// dont let uninitialized value here hurt the player
	m_flFallVelocity = 0;
	m_deathMessageSent = false;

	g_pGameRules->SetDefaultPlayerTeam( this );
	edict_t* pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot( this );

	pev->origin = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);
	pev->v_angle = g_vecZero;
	pev->velocity = g_vecZero;
	pev->angles = VARS(pentSpawnSpot)->angles;
	pev->punchangle = g_vecZero;
	pev->fixangle = TRUE;

    SET_MODEL(ENT(pev), "models/player.mdl");
    g_ulModelIndexPlayer = pev->modelindex;
	pev->sequence		= LookupActivity( ACT_IDLE );

	if ( FBitSet(pev->flags, FL_DUCKING) ) 
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

    pev->view_ofs = VEC_VIEW;
	Precache();
	m_HackedGunPos		= Vector( 0, 32, 0 );

	if ( m_iPlayerSound == SOUNDLIST_EMPTY )
	{
		ALERT ( at_console, "Couldn't alloc player sound slot!\n" );
	}

	m_fNoPlayerSound = FALSE;// normal sound behavior.

	m_pLastItem = NULL;
	m_fInitHUD = TRUE;
	m_iClientHideHUD = -1;  // force this to be recalculated
	m_fWeapon = FALSE;
	m_pClientActiveItem = NULL;
	m_iClientBattery = -1;

	// reset all ammo values to 0
	for ( int i = 0; i < MAX_AMMO_SLOTS; i++ )
	{
		m_rgAmmo[i] = 0;
		m_rgAmmoLast[i] = 0;  // client ammo values also have to be reset  (the death hud clear messages does on the client side)
	}

	m_lastx = m_lasty = 0;
	
	m_flNextChatTime = gpGlobals->time;

	// clear respawn text
	// (also prevents bug where last printed text shows during intermission)
	CLIENT_PRINTF(edict(), print_center, "");

	// reset sound environment to default
	if (m_flLastSetRoomtype) {
		m_pentSndLast = NULL;
		m_flLastSetRoomtype = 0;
		MESSAGE_BEGIN(MSG_ONE, SVC_ROOMTYPE, NULL, edict());
		WRITE_SHORT(0);
		MESSAGE_END();
	}

	// change hud color
	MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
	WRITE_BYTE(entindex());	// client number
	WRITE_SHORT(pev->frags);
	WRITE_SHORT(m_iDeaths);
	WRITE_SHORT(0);
	WRITE_SHORT(DEFAULT_TEAM_COLOR);
	MESSAGE_END();

	// view can be set to null(?) if the level changes while a camera is active
	// which means you won't see any entities in the next level
	if (!m_hActiveCamera) {
		SET_VIEW(edict(), edict());
	}
	

	g_pGameRules->PlayerSpawn( this );
}

void CBasePlayer :: Precache( void )
{
	// in the event that the player JUST spawned, and the level node graph
	// was loaded, fix all of the node graph pointers before the game starts.
	
	// !!!BUGBUG - now that we have multiplayer, this needs to be moved!
	if ( WorldGraph.m_fGraphPresent && !WorldGraph.m_fGraphPointersSet )
	{
		if ( !WorldGraph.FSetGraphPointers() )
		{
			ALERT ( at_console, "**Graph pointers were not set!\n");
		}
		else
		{
			ALERT ( at_console, "**Graph Pointers Set!\n" );
		} 
	}

	// SOUNDS / MODELS ARE PRECACHED in ClientPrecache() (game specific)
	// because they need to precache before any clients have connected

	// init geiger counter vars during spawn and each time
	// we cross a level transition

	m_flgeigerRange = 1000;
	m_igeigerRangePrev = 1000;

	m_bitsDamageType = 0;
	m_bitsHUDDamage = -1;

	m_iClientBattery = -1;

	m_iTrain = TRAIN_NEW;

	// Make sure any necessary user messages have been registered
	LinkUserMessages();

	m_iUpdateTime = 5;  // won't update for 1/2 a second

	if ( gInitHUD )
		m_fInitHUD = TRUE;
}

int CBasePlayer::Save( CSave &save )
{
	if ( !CBaseMonster::Save(save) )
		return 0;

	return save.WriteFields( "PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData) );
}

//
// Marks everything as new so the player will resend this to the hud.
//
void CBasePlayer::RenewItems(void)
{

}

int CBasePlayer::Restore( CRestore &restore )
{
	if ( !CBaseMonster::Restore(restore) )
		return 0;

	int status = restore.ReadFields( "PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData) );

	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;
	// landmark isn't present.
	if ( !pSaveData->fUseLandmark )
	{
		ALERT( at_console, "No Landmark:%s\n", pSaveData->szLandmarkName );

		// default to normal spawn
		edict_t* pentSpawnSpot = EntSelectSpawnPoint( this );
		pev->origin = VARS(pentSpawnSpot)->origin + Vector(0,0,1);
		pev->angles = VARS(pentSpawnSpot)->angles;
	}
	pev->v_angle.z = 0;	// Clear out roll
	pev->angles = pev->v_angle;

	pev->fixangle = TRUE;           // turn this way immediately

// Copied from spawn() for now
	m_bloodColor	= BLOOD_COLOR_RED;

    g_ulModelIndexPlayer = pev->modelindex;

	if ( FBitSet(pev->flags, FL_DUCKING) ) 
	{
		// Use the crouch HACK
		//FixPlayerCrouchStuck( edict() );
		// Don't need to do this with new player prediction code.
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	}
	else
	{
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);
	}

	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "hl", "1" );

	if ( m_fLongJump )
	{
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "1" );
	}
	else
	{
		g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );
	}

	RenewItems();

	//Resync ammo data so you can reload - Solokiller
	TabulateAmmo();

#if defined( CLIENT_WEAPONS )
	// HACK:	This variable is saved/restored in CBaseMonster as a time variable, but we're using it
	//			as just a counter.  Ideally, this needs its own variable that's saved as a plain float.
	//			Barring that, we clear it out here instead of using the incorrect restored time value.
	m_flNextAttack = UTIL_WeaponTimeBase();
#endif

	// Force a flashlight update for the HUD
	if (m_flFlashLightTime == 0)
	{
		m_flFlashLightTime = 1;
	}

	return status;
}

void CBasePlayer::SelectNextItem( int iItem )
{
	CBasePlayerItem *pItem;

	pItem = (CBasePlayerItem*)m_rgpPlayerItems[iItem].GetEntity();
	CBasePlayerItem* activeItem = (CBasePlayerItem*)m_pActiveItem.GetEntity();

	if (!pItem)
		return;

	if (pItem == activeItem)
	{
		// select the next one in the chain
		pItem = (CBasePlayerItem*)activeItem->m_pNext.GetEntity();;
		if (! pItem)
		{
			return;
		}

		CBasePlayerItem *pLast;
		pLast = pItem;
		while (pLast->m_pNext)
			pLast = (CBasePlayerItem*)pLast->m_pNext.GetEntity();

		// relink chain
		pLast->m_pNext = activeItem;
		activeItem->m_pNext = NULL;
		m_rgpPlayerItems[ iItem ] = pItem;
	}

	ResetAutoaim( );

	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
	{
		activeItem->Holster( );
	}
	
	m_pActiveItem = pItem;

	if (m_pActiveItem)
	{
		activeItem = (CBasePlayerItem*)m_pActiveItem.GetEntity();
		activeItem->Deploy( );
		activeItem->UpdateItemInfo( );
	}
}

void CBasePlayer::SelectItem(const char *pstr)
{
	if (!pstr)
		return;

	CBasePlayerItem *pItem = NULL;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			pItem = (CBasePlayerItem*)m_rgpPlayerItems[i].GetEntity();
	
			while (pItem)
			{
				if (FClassnameIs(pItem->pev, pstr))
					break;
				pItem = (CBasePlayerItem*)pItem->m_pNext.GetEntity();
			}
		}

		if (pItem)
			break;
	}

	if (!pItem)
		return;

	
	if (pItem == m_pActiveItem.GetEntity())
		return;

	ResetAutoaim( );

	CBasePlayerItem* activeItem = (CBasePlayerItem*)m_pActiveItem.GetEntity();

	// FIX, this needs to queue them up and delay
	if (m_pActiveItem)
		activeItem->Holster( );
	
	m_pLastItem = activeItem;
	m_pActiveItem = pItem;

	if (m_pActiveItem)
	{
		activeItem = (CBasePlayerItem*)m_pActiveItem.GetEntity();
		activeItem->Deploy( );
		activeItem->UpdateItemInfo( );
	}
}

void CBasePlayer::SelectLastItem(void)
{
	if (!m_pLastItem)
	{
		return;
	}

	CBasePlayerItem* activeItem = (CBasePlayerItem*)m_pActiveItem.GetEntity();

	if (activeItem && !activeItem->CanHolster() )
	{
		return;
	}

	ResetAutoaim( );

	// FIX, this needs to queue them up and delay
	if (activeItem)
		activeItem->Holster( );
	
	CBasePlayerItem *pTemp = activeItem;
	m_pActiveItem = m_pLastItem.GetEntity();
	m_pLastItem = pTemp;

	if (m_pActiveItem) {
		activeItem = (CBasePlayerItem*)m_pActiveItem.GetEntity();
		activeItem->Deploy();
		activeItem->UpdateItemInfo();
	}
}

//==============================================
// HasWeapons - do I have any weapons at all?
//==============================================
BOOL CBasePlayer::HasWeapons( void )
{
	int i;

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CBasePlayer::SelectPrevItem( int iItem )
{
}

const char *CBasePlayer::TeamID( void )
{
	if ( pev == NULL )		// Not fully connected yet
		return "";

	// return their team name
	return m_szTeamName;
}

void CBasePlayer::GiveNamedItem( const char *pszName )
{
	edict_t	*pent;

	int istr = MAKE_STRING(pszName);

	pent = CREATE_NAMED_ENTITY(istr);
	if ( FNullEnt( pent ) )
	{
		ALERT ( at_console, UTIL_VarArgs("NULL Ent '%s' in GiveNamedItem!\n", pszName) );
		return;
	}
	VARS( pent )->origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn( pent );
	DispatchTouch( pent, ENT( pev ) );
}

CBaseEntity *FindEntityForward( CBaseEntity *pMe )
{
	TraceResult tr;

	UTIL_MakeVectors(pMe->pev->v_angle);
	UTIL_TraceLine(pMe->pev->origin + pMe->pev->view_ofs,pMe->pev->origin + pMe->pev->view_ofs + gpGlobals->v_forward * 8192,dont_ignore_monsters, pMe->edict(), &tr );
	if ( tr.flFraction != 1.0 && !FNullEnt( tr.pHit) )
	{
		CBaseEntity *pHit = CBaseEntity::Instance( tr.pHit );
		return pHit;
	}
	return NULL;
}

BOOL CBasePlayer :: FlashlightIsOn( void )
{
	return FBitSet(pev->effects, EF_DIMLIGHT);
}

void CBasePlayer :: FlashlightTurnOn( void )
{
	if ( !g_pGameRules->FAllowFlashlight() )
	{
		return;
	}

	if ( (pev->weapons & (1<<WEAPON_SUIT)) )
	{
		EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, SOUND_FLASHLIGHT_ON, 1.0, ATTN_NORM, 0, PITCH_NORM );
		SetBits(pev->effects, EF_DIMLIGHT);
		MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
		WRITE_BYTE(1);
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();

		m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;

	}
}

void CBasePlayer :: FlashlightTurnOff( void )
{
	EMIT_SOUND_DYN( ENT(pev), CHAN_WEAPON, SOUND_FLASHLIGHT_OFF, 1.0, ATTN_NORM, 0, PITCH_NORM );
    ClearBits(pev->effects, EF_DIMLIGHT);
	MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
	WRITE_BYTE(0);
	WRITE_BYTE(m_iFlashBattery);
	MESSAGE_END();

	m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;

}

/*
===============
ForceClientDllUpdate

When recording a demo, we need to have the server tell us the entire client state
so that the client side .dll can behave correctly.
Reset stuff so that the state is transmitted.
===============
*/
void CBasePlayer :: ForceClientDllUpdate( void )
{
	m_iClientHealth  = -1;
	m_iClientBattery = -1;
	m_iTrain |= TRAIN_NEW;  // Force new train message.
	m_fWeapon = FALSE;          // Force weapon send
	m_fKnownItem = FALSE;    // Force weaponinit messages.
	m_fInitHUD = TRUE;		// Force HUD gmsgResetHUD message

	// Now force all the necessary messages
	//  to be sent.
	UpdateClientData();
}

/*
============
ImpulseCommands
============
*/
void CBasePlayer::ImpulseCommands( )
{
	TraceResult	tr;// UNDONE: kill me! This is temporary for PreAlpha CDs

	// Handle use events
	PlayerUse();
		
	int iImpulse = (int)pev->impulse;
	switch (iImpulse)
	{
	case 99:
		{

		int iOn;

		if (!gmsgLogo)
		{
			iOn = 1;
			gmsgLogo = REG_USER_MSG("Logo", 1);
		} 
		else 
		{
			iOn = 0;
		}
		
		ASSERT( gmsgLogo > 0 );
		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgLogo, NULL, pev );
			WRITE_BYTE(iOn);
		MESSAGE_END();

		if(!iOn)
			gmsgLogo = 0;
		break;
		}
	case 100:
        // temporary flashlight for level designers
        if ( FlashlightIsOn() )
		{
			FlashlightTurnOff();
		}
        else 
		{
			FlashlightTurnOn();
		}
		break;

	case	201:// paint decal
		
		if ( gpGlobals->time < m_flNextDecalTime )
		{
			// too early!
			break;
		}

		UTIL_MakeVectors(pev->v_angle);
		UTIL_TraceLine ( pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), & tr);

		if ( tr.flFraction != 1.0 )
		{// line hit something, so paint a decal
			m_flNextDecalTime = gpGlobals->time + decalfrequency.value;
			CSprayCan *pCan = GetClassPtr((CSprayCan *)NULL);
			pCan->Spawn( pev );
		}

		break;

	default:
		// check all of the cheat impulse commands now
		CheatImpulseCommands( iImpulse );
		break;
	}
	
	pev->impulse = 0;
}

#define RETURN_IF_CHEATS_DISABLED() if (CVAR_GET_FLOAT("sv_cheats") == 0) return;
//=========================================================
//=========================================================
void CBasePlayer::CheatImpulseCommands( int iImpulse )
{
	CBaseEntity *pEntity;

	switch ( iImpulse )
	{
	case 76:
		RETURN_IF_CHEATS_DISABLED()
		{
			if (!giPrecacheGrunt)
			{
				giPrecacheGrunt = 1;
				ALERT(at_console, "You must now restart to use Grunt-o-matic.\n");
			}
			else
			{
				UTIL_MakeVectors( Vector( 0, pev->v_angle.y, 0 ) );
				Create("monster_human_grunt", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			}
			break;
		}


	case 101:
		// impulse 101 cheat
		RETURN_IF_CHEATS_DISABLED()
		gEvilImpulse101 = TRUE;
		GiveNamedItem( "item_suit" );
		GiveNamedItem( "item_battery" );
		GiveNamedItem( "weapon_crowbar" );
		GiveNamedItem( "weapon_9mmhandgun" );
		GiveNamedItem( "ammo_9mmclip" );
		GiveNamedItem( "weapon_shotgun" );
		GiveNamedItem( "ammo_buckshot" );
		GiveNamedItem( "weapon_9mmAR" );
		GiveNamedItem( "ammo_9mmAR" );
		GiveNamedItem( "ammo_ARgrenades" );
		GiveNamedItem( "weapon_handgrenade" );
		GiveNamedItem( "weapon_tripmine" );
		GiveNamedItem( "weapon_357" );
		GiveNamedItem( "ammo_357" );
		GiveNamedItem( "weapon_crossbow" );
		GiveNamedItem( "ammo_crossbow" );
		GiveNamedItem( "weapon_egon" );
		GiveNamedItem( "weapon_gauss" );
		GiveNamedItem( "ammo_gaussclip" );
		GiveNamedItem( "weapon_rpg" );
		GiveNamedItem( "ammo_rpgclip" );
		GiveNamedItem( "weapon_satchel" );
		GiveNamedItem( "weapon_snark" );
		GiveNamedItem( "weapon_hornetgun" );
		GiveNamedItem( "weapon_grapple" );
		GiveNamedItem( "weapon_pipewrench" );
		GiveNamedItem( "weapon_displacer" );
		GiveNamedItem( "weapon_shockrifle" );
		GiveNamedItem( "weapon_sporelauncher" );
		if (!m_fLongJump) {
			GiveNamedItem("item_longjump");
		}
		gEvilImpulse101 = FALSE;
		break;

	case 102:
		RETURN_IF_CHEATS_DISABLED()
		// Gibbage!!!
		CGib::SpawnRandomGibs( pev, 1, 1 );
		break;

	case 103:
		RETURN_IF_CHEATS_DISABLED()
		// What the hell are you doing?
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			CBaseMonster *pMonster = pEntity->MyMonsterPointer();
			if ( pMonster )
				pMonster->ReportAIState();
		}
		break;

	case 104:
		RETURN_IF_CHEATS_DISABLED()
		// Dump all of the global state varaibles (and global entity names)
		gGlobalState.DumpGlobals();
		break;

	case	105:// player makes no sound for monsters to hear.
		RETURN_IF_CHEATS_DISABLED()
		{
			if ( m_fNoPlayerSound )
			{
				ALERT ( at_console, "Player is audible\n" );
				m_fNoPlayerSound = FALSE;
			}
			else
			{
				ALERT ( at_console, "Player is silent\n" );
				m_fNoPlayerSound = TRUE;
			}
			break;
		}

	case 106:
		RETURN_IF_CHEATS_DISABLED()
		// Give me the classname and targetname of this entity.
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			ALERT ( at_console, "Classname: %s", STRING( pEntity->pev->classname ) );
			
			if ( !FStringNull ( pEntity->pev->targetname ) )
			{
				ALERT ( at_console, " - Targetname: %s\n", STRING( pEntity->pev->targetname ) );
			}
			else
			{
				ALERT ( at_console, " - TargetName: No Targetname\n" );
			}

			ALERT ( at_console, "Model: %s\n", STRING( pEntity->pev->model ) );
			if ( pEntity->pev->globalname )
				ALERT ( at_console, "Globalname: %s\n", STRING( pEntity->pev->globalname ) );
		}
		break;

	case 107:
		RETURN_IF_CHEATS_DISABLED()
		{
			TraceResult tr;

			edict_t		*pWorld = g_engfuncs.pfnPEntityOfEntIndex( 0 );

			Vector start = pev->origin + pev->view_ofs;
			Vector end = start + gpGlobals->v_forward * 1024;
			UTIL_TraceLine( start, end, ignore_monsters, edict(), &tr );
			if ( tr.pHit )
				pWorld = tr.pHit;
			const char *pTextureName = TRACE_TEXTURE( pWorld, start, end );
			if ( pTextureName )
				ALERT( at_console, "Texture: %s\n", pTextureName );
		}
		break;
	case	195:// show shortest paths for entire level to nearest node
		RETURN_IF_CHEATS_DISABLED()
		{
			Create("node_viewer_fly", pev->origin, pev->angles);
		}
		break;
	case	196:// show shortest paths for entire level to nearest node
		RETURN_IF_CHEATS_DISABLED()
		{
			Create("node_viewer_large", pev->origin, pev->angles);
		}
		break;
	case	197:// show shortest paths for entire level to nearest node
		RETURN_IF_CHEATS_DISABLED()
		{
			Create("node_viewer_human", pev->origin, pev->angles);
		}
		break;
	case	199:// show nearest node and all connections
		RETURN_IF_CHEATS_DISABLED()
		{
			ALERT ( at_console, "%d\n", WorldGraph.FindNearestNode ( pev->origin, bits_NODE_GROUP_REALM ) );
			WorldGraph.ShowNodeConnections ( WorldGraph.FindNearestNode ( pev->origin, bits_NODE_GROUP_REALM ) );
		}
		break;
	case	202:// Random blood splatter
	{
		RETURN_IF_CHEATS_DISABLED();
		UTIL_MakeVectors(pev->v_angle);
	
		TraceResult tr;
		UTIL_TraceLine(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, ENT(pev), &tr);

		if (tr.flFraction != 1.0)
		{// line hit something, so paint a decal
			CBloodSplat* pBlood = GetClassPtr((CBloodSplat*)NULL);
			pBlood->Spawn(pev);
		}
		break;
	}
	case	203:// remove creature.
		RETURN_IF_CHEATS_DISABLED()
		pEntity = FindEntityForward( this );
		if ( pEntity )
		{
			if ( pEntity->pev->takedamage )
				pEntity->SetThink(&CBaseEntity::SUB_Remove);
		}
		break;
	}
}

//
// Add a weapon to the player (Item == Weapon == Selectable Object)
//
int CBasePlayer::AddPlayerItem( CBasePlayerItem *pItem )
{
	CBasePlayerItem *pInsert;
	
	pInsert = (CBasePlayerItem*)m_rgpPlayerItems[pItem->iItemSlot()].GetEntity();

	while (pInsert)
	{
		if (FClassnameIs( pInsert->pev, STRING( pItem->pev->classname) ))
		{
			if (pItem->AddDuplicate( pInsert ))
			{
				g_pGameRules->PlayerGotWeapon ( this, pItem );
				pItem->CheckRespawn();

				// ugly hack to update clip w/o an update clip message
				pInsert->UpdateItemInfo( );
				if (m_pActiveItem)
					((CBasePlayerItem*)m_pActiveItem.GetEntity())->UpdateItemInfo();

				pItem->Kill( );
			}
			else if (gEvilImpulse101)
			{
				// FIXME: remove anyway for deathmatch testing
				pItem->Kill( );
			}
			return FALSE;
		}
		pInsert = (CBasePlayerItem*)pInsert->m_pNext.GetEntity();
	}


	if (pItem->AddToPlayer( this ))
	{
		g_pGameRules->PlayerGotWeapon ( this, pItem );
		pItem->CheckRespawn();

		pItem->m_pNext = (CBasePlayerItem*)m_rgpPlayerItems[pItem->iItemSlot()].GetEntity();
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem;

		// should we switch to this item?
		if ( g_pGameRules->FShouldSwitchWeapon( this, pItem ) )
		{
			SwitchWeapon( pItem );
		}

		return TRUE;
	}
	else if (gEvilImpulse101)
	{
		// FIXME: remove anyway for deathmatch testing
		pItem->Kill( );
	}
	return FALSE;
}

int CBasePlayer::RemovePlayerItem( CBasePlayerItem *pItem )
{
	if (m_pActiveItem.GetEntity() == pItem)
	{
		ResetAutoaim( );
		pItem->Holster( );
		pItem->pev->nextthink = 0;// crowbar may be trying to swing again, etc.
		pItem->SetThink( NULL );
		m_pActiveItem = NULL;
		pev->viewmodel = 0;
		pev->weaponmodel = 0;
	}
	else if ( m_pLastItem.GetEntity() == pItem )
		m_pLastItem = NULL;

	CBasePlayerItem *pPrev = (CBasePlayerItem*)m_rgpPlayerItems[pItem->iItemSlot()].GetEntity();

	if (pPrev == pItem)
	{
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem->m_pNext;
		return TRUE;
	}
	else
	{
		while (pPrev && pPrev->m_pNext.GetEntity() != pItem)
		{
			pPrev = (CBasePlayerItem*)pPrev->m_pNext.GetEntity();
		}
		if (pPrev)
		{
			pPrev->m_pNext = pItem->m_pNext;
			return TRUE;
		}
	}
	return FALSE;
}

//
// Returns the unique ID for the ammo, or -1 if error
//
int CBasePlayer :: GiveAmmo( int iCount, const char *szName, int iMax )
{
	if ( !szName )
	{
		// no ammo.
		return -1;
	}

	if ( !g_pGameRules->CanHaveAmmo( this, szName, iMax ) )
	{
		// game rules say I can't have any more of this ammo type.
		return -1;
	}

	int i = 0;

	i = GetAmmoIndex( szName );

	if ( i < 0 || i >= MAX_AMMO_SLOTS )
		return -1;

	int iAdd = V_min( iCount, iMax - m_rgAmmo[i] );
	if ( iAdd < 1 )
		return i;

	m_rgAmmo[ i ] += iAdd;


	if ( gmsgAmmoPickup )  // make sure the ammo messages have been linked first
	{
		// Send the message that ammo has been picked up
		MESSAGE_BEGIN( MSG_ONE, gmsgAmmoPickup, NULL, pev );
			WRITE_BYTE( GetAmmoIndex(szName) );		// ammo ID
			WRITE_BYTE( iAdd );		// amount
		MESSAGE_END();
	}

	TabulateAmmo();

	return i;
}

/*
============
ItemPreFrame

Called every frame by the player PreThink
============
*/
void CBasePlayer::ItemPreFrame()
{
#if defined( CLIENT_WEAPONS )
    if ( m_flNextAttack > 0 )
#else
    if ( gpGlobals->time < m_flNextAttack )
#endif
	{
		return;
	}

	if (!m_pActiveItem)
		return;

	CBasePlayerItem* activeItem = (CBasePlayerItem*)m_pActiveItem.GetEntity();
	activeItem->ItemPreFrame( );
}

/*
============
ItemPostFrame

Called every frame by the player PostThink
============
*/
void CBasePlayer::ItemPostFrame()
{
	// check if the player is using a tank
	if ( m_pTank != NULL )
		return;

#if defined( CLIENT_WEAPONS )
    if ( m_flNextAttack > 0 )
#else
    if ( gpGlobals->time < m_flNextAttack )
#endif
	{
		return;
	}

	ImpulseCommands();

	if (!m_pActiveItem)
		return;

	CBasePlayerItem* activeItem = (CBasePlayerItem*)m_pActiveItem.GetEntity();
	activeItem->ItemPostFrame( );
}

int CBasePlayer::AmmoInventory( int iAmmoIndex )
{
	if (iAmmoIndex == -1)
	{
		return -1;
	}

	return m_rgAmmo[ iAmmoIndex ];
}

int CBasePlayer::GetAmmoIndex(const char *psz)
{
	int i;

	if (!psz)
		return -1;

	for (i = 1; i < MAX_AMMO_SLOTS; i++)
	{
		if ( !CBasePlayerItem::AmmoInfoArray[i].pszName )
			continue;

		if (stricmp( psz, CBasePlayerItem::AmmoInfoArray[i].pszName ) == 0)
			return i;
	}

	return -1;
}

// Called from UpdateClientData
// makes sure the client has all the necessary ammo info,  if values have changed
void CBasePlayer::SendAmmoUpdate(void)
{
	for (int i=0; i < MAX_AMMO_SLOTS;i++)
	{
		if (m_rgAmmo[i] != m_rgAmmoLast[i])
		{
			m_rgAmmoLast[i] = m_rgAmmo[i];

			ASSERT( m_rgAmmo[i] >= 0 );
			ASSERT( m_rgAmmo[i] < 255 );

			// send "Ammo" update message
			MESSAGE_BEGIN( MSG_ONE, gmsgAmmoX, NULL, pev );
				WRITE_BYTE( i );
				WRITE_BYTE( V_max( V_min( m_rgAmmo[i], 254 ), 0 ) );  // clamp the value to one byte
			MESSAGE_END();
		}
	}
}

/*
=========================================================
	UpdateClientData

resends any changed player HUD info to the client.
Called every frame by PlayerPreThink
Also called at start of demo recording and playback by
ForceClientDllUpdate to ensure the demo gets messages
reflecting all of the HUD state info.
=========================================================
*/
void CBasePlayer :: UpdateClientData( void )
{
	if (m_fInitHUD)
	{
		m_fInitHUD = FALSE;
		gInitHUD = FALSE;
		
		MESSAGE_BEGIN( MSG_ONE, gmsgResetHUD, NULL, pev );
			WRITE_BYTE( 0 );
		MESSAGE_END();

		// resend team info
		// TODO: the scoreboard team will flicker for a moment on respawn
		{
			MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
			WRITE_BYTE(entindex());
			WRITE_STRING(IsObserver() ? "" : DEFAULT_TEAM_NAME);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
			WRITE_BYTE(entindex());	// client number
			WRITE_SHORT(pev->frags);
			WRITE_SHORT(m_iDeaths);
			WRITE_SHORT(0);
			WRITE_SHORT(GetNameColor());
			MESSAGE_END();
		}

		if ( !m_fGameHUDInitialized )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgInitHUD, NULL, pev );
			MESSAGE_END();

			g_pGameRules->InitHUD( this );
			m_fGameHUDInitialized = TRUE;
			
			m_iObserverLastMode = OBS_ROAMING;
			
			if ( g_pGameRules->IsMultiplayer() )
			{
				FireTargets( "game_playerjoin", this, this, USE_TOGGLE, 0 );
			}
		}

		FireTargets( "game_playerspawn", this, this, USE_TOGGLE, 0 );

		InitStatusBar();
	}

	if ( m_iHideHUD != m_iClientHideHUD )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgHideWeapon, NULL, pev );
			WRITE_BYTE( m_iHideHUD );
		MESSAGE_END();

		m_iClientHideHUD = m_iHideHUD;
	}

	if ( m_iFOV != m_iClientFOV )
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgSetFOV, NULL, pev );
			WRITE_BYTE( m_iFOV );
		MESSAGE_END();

		// cache FOV change at end of function, so weapon updates can see that FOV has changed
	}

	// HACKHACK -- send the message to display the game title
	if (gDisplayTitle)
	{
		MESSAGE_BEGIN( MSG_ONE, gmsgShowGameTitle, NULL, pev );
		WRITE_BYTE( 0 );
		MESSAGE_END();
		gDisplayTitle = 0;
	}

	if (pev->health != m_iClientHealth)
	{
		int iHealth = clampf( pev->health, 0, 255 );  // make sure that no negative health values are sent
		if ( pev->health > 0.0f && pev->health <= 1.0f )
			iHealth = 1;

		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgHealth, NULL, pev );
			WRITE_BYTE( iHealth );
		MESSAGE_END();

		m_iClientHealth = pev->health;
	}


	if (pev->armorvalue != m_iClientBattery)
	{
		m_iClientBattery = pev->armorvalue;

		ASSERT( gmsgBattery > 0 );
		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgBattery, NULL, pev );
			WRITE_SHORT( (int)pev->armorvalue);
		MESSAGE_END();
	}

	if (pev->dmg_take || pev->dmg_save || m_bitsHUDDamage != m_bitsDamageType)
	{
		// Comes from inside me if not set
		Vector damageOrigin = pev->origin;
		// send "damage" message
		// causes screen to flash, and pain compass to show direction of damage
		edict_t *other = pev->dmg_inflictor;
		if ( other )
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(other);
			if ( pEntity )
				damageOrigin = pEntity->Center();
		}

		// only send down damage type that have hud art
		int visibleDamageBits = m_bitsDamageType & DMG_SHOWNHUD;

		MESSAGE_BEGIN( MSG_ONE, gmsgDamage, NULL, pev );
			WRITE_BYTE( pev->dmg_save );
			WRITE_BYTE( pev->dmg_take );
			WRITE_LONG( visibleDamageBits );
			WRITE_COORD( damageOrigin.x );
			WRITE_COORD( damageOrigin.y );
			WRITE_COORD( damageOrigin.z );
		MESSAGE_END();
	
		pev->dmg_take = 0;
		pev->dmg_save = 0;
		m_bitsHUDDamage = m_bitsDamageType;
		
		// Clear off non-time-based damage indicators
		m_bitsDamageType &= DMG_TIMEBASED;
	}

	// Update Flashlight
	if ((m_flFlashLightTime) && (m_flFlashLightTime <= gpGlobals->time))
	{
		if (FlashlightIsOn())
		{
			if (m_iFlashBattery)
			{
				m_flFlashLightTime = FLASH_DRAIN_TIME + gpGlobals->time;
				m_iFlashBattery--;
				
				if (!m_iFlashBattery)
					FlashlightTurnOff();
			}
		}
		else
		{
			if (m_iFlashBattery < 100)
			{
				m_flFlashLightTime = FLASH_CHARGE_TIME + gpGlobals->time;
				m_iFlashBattery++;
			}
			else
				m_flFlashLightTime = 0;
		}

		MESSAGE_BEGIN( MSG_ONE, gmsgFlashBattery, NULL, pev );
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();
	}


	if (m_iTrain & TRAIN_NEW)
	{
		ASSERT( gmsgTrain > 0 );
		// send "health" update message
		MESSAGE_BEGIN( MSG_ONE, gmsgTrain, NULL, pev );
			WRITE_BYTE(m_iTrain & 0xF);
		MESSAGE_END();

		m_iTrain &= ~TRAIN_NEW;
	}

	//
	// New Weapon?
	//
	if (!m_fKnownItem)
	{
		m_fKnownItem = TRUE;

	// WeaponInit Message
	// byte  = # of weapons
	//
	// for each weapon:
	// byte		name str length (not including null)
	// bytes... name
	// byte		Ammo Type
	// byte		Ammo2 Type
	// byte		bucket
	// byte		bucket pos
	// byte		flags	
	// ????		Icons
		
		// Send ALL the weapon info now
		int i;

		for (i = 0; i < MAX_WEAPONS; i++)
		{
			ItemInfo& II = CBasePlayerItem::ItemInfoArray[i];

			if ( !II.iId )
				continue;

			const char *pszName;
			if (!II.pszName)
				pszName = "Empty";
			else
				pszName = II.pszName;

			MESSAGE_BEGIN( MSG_ONE, gmsgWeaponList, NULL, pev );  
				WRITE_STRING(pszName);			// string	weapon name
				WRITE_BYTE(GetAmmoIndex(II.pszAmmo1));	// byte		Ammo Type
				WRITE_BYTE(II.iMaxAmmo1);				// byte     Max Ammo 1
				WRITE_BYTE(GetAmmoIndex(II.pszAmmo2));	// byte		Ammo2 Type
				WRITE_BYTE(II.iMaxAmmo2);				// byte     Max Ammo 2
				WRITE_BYTE(II.iSlot);					// byte		bucket
				WRITE_BYTE(II.iPosition);				// byte		bucket pos
				WRITE_BYTE(II.iId);						// byte		id (bit index into pev->weapons)
				WRITE_BYTE(II.iFlags);					// byte		Flags
			MESSAGE_END();
		}
	}


	SendAmmoUpdate();

	// Update all the items
	for ( int i = 0; i < MAX_ITEM_TYPES; i++ )
	{
		if ( m_rgpPlayerItems[i] )  // each item updates it's successors
			((CBasePlayerItem*)m_rgpPlayerItems[i].GetEntity())->UpdateClientData(this);
	}

	// Cache and client weapon change
	m_pClientActiveItem = (CBasePlayerItem*)m_pActiveItem.GetEntity();
	m_iClientFOV = m_iFOV;

	// Update Status Bar
	if ( m_flNextSBarUpdateTime < gpGlobals->time )
	{
		lagcomp_begin(this);
		UpdateStatusBar();
		lagcomp_end();
		m_flNextSBarUpdateTime = gpGlobals->time + 0.2;
	}
}

void CBasePlayer::SetPrefsFromUserinfo(char* infobuffer)
{
	const char* pszKeyVal;

	// Set autoswitch preference
	pszKeyVal = g_engfuncs.pfnInfoKeyValue(infobuffer, "cl_autowepswitch");
	if (FStrEq(pszKeyVal, ""))
	{
		m_iAutoWepSwitch = 1;
	}
	else
	{
		m_iAutoWepSwitch = atoi(pszKeyVal);
	}
}

//=========================================================
// FBecomeProne - Overridden for the player to set the proper
// physics flags when a barnacle grabs player.
//=========================================================
BOOL CBasePlayer :: FBecomeProne ( void )
{
	m_afPhysicsFlags |= PFLAG_ONBARNACLE;
	return TRUE;
}

//=========================================================
// BarnacleVictimBitten - bad name for a function that is called
// by Barnacle victims when the barnacle pulls their head
// into its mouth. For the player, just die.
//=========================================================
void CBasePlayer :: BarnacleVictimBitten ( entvars_t *pevBarnacle )
{
	TakeDamage ( pevBarnacle, pevBarnacle, pev->health + pev->armorvalue, DMG_SLASH | DMG_ALWAYSGIB );
}

//=========================================================
// BarnacleVictimReleased - overridden for player who has
// physics flags concerns. 
//=========================================================
void CBasePlayer :: BarnacleVictimReleased ( void )
{
	m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
}

//=========================================================
// Illumination 
// return player light level plus virtual muzzle flash
//=========================================================
int CBasePlayer :: Illumination( void )
{
	int iIllum = CBaseEntity::Illumination( );

	iIllum += m_iWeaponFlash;
	if (iIllum > 255)
		return 255;
	return iIllum;
}

void CBasePlayer :: EnableControl(BOOL fControl)
{
	if (!fControl)
		pev->flags |= FL_FROZEN;
	else
		pev->flags &= ~FL_FROZEN;

}

//=========================================================
// Autoaim
// set crosshair position to point to enemey
//=========================================================
Vector CBasePlayer :: GetAutoaimVector( float flDelta )
{
	if (g_iSkillLevel == SKILL_HARD)
	{
		UTIL_MakeVectors( pev->v_angle + pev->punchangle );
		return gpGlobals->v_forward;
	}

	Vector vecSrc = GetGunPosition( );
	float flDist = 8192;

	// always use non-sticky autoaim
	// UNDONE: use sever variable to chose!
	//if (g_iSkillLevel == SKILL_MEDIUM)
	{
		m_vecAutoAim = Vector( 0, 0, 0 );
		// flDelta *= 0.5;
	}

	BOOL m_fOldTargeting = m_fOnTarget;
	Vector angles = AutoaimDeflection(vecSrc, flDist, flDelta );

	// update ontarget if changed
	if ( !g_pGameRules->AllowAutoTargetCrosshair() )
		m_fOnTarget = 0;
	else if (m_fOldTargeting != m_fOnTarget)
	{
		((CBasePlayerItem*)m_pActiveItem.GetEntity())->UpdateItemInfo();
	}

	if (angles.x > 180)
		angles.x -= 360;
	if (angles.x < -180)
		angles.x += 360;
	if (angles.y > 180)
		angles.y -= 360;
	if (angles.y < -180)
		angles.y += 360;

	if (angles.x > 25)
		angles.x = 25;
	if (angles.x < -25)
		angles.x = -25;
	if (angles.y > 12)
		angles.y = 12;
	if (angles.y < -12)
		angles.y = -12;


	// always use non-sticky autoaim
	// UNDONE: use sever variable to chose!
	if (0 || g_iSkillLevel == SKILL_EASY)
	{
		m_vecAutoAim = m_vecAutoAim * 0.67 + angles * 0.33;
	}
	else
	{
		m_vecAutoAim = angles * 0.9;
	}

	// m_vecAutoAim = m_vecAutoAim * 0.99;

	// Don't send across network if sv_aim is 0
	if ( g_psv_aim->value != 0 && g_psv_allow_autoaim->value != 0)
	{
		if ( m_vecAutoAim.x != m_lastx ||
			 m_vecAutoAim.y != m_lasty )
		{
			SET_CROSSHAIRANGLE( edict(), -m_vecAutoAim.x, m_vecAutoAim.y );
			
			m_lastx = m_vecAutoAim.x;
			m_lasty = m_vecAutoAim.y;
		}
	}

	// ALERT( at_console, "%f %f\n", angles.x, angles.y );

	UTIL_MakeVectors( pev->v_angle + pev->punchangle + m_vecAutoAim );
	return gpGlobals->v_forward;
}

Vector CBasePlayer :: AutoaimDeflection( Vector &vecSrc, float flDist, float flDelta  )
{
	edict_t		*pEdict = g_engfuncs.pfnPEntityOfEntIndex( 1 );
	CBaseEntity	*pEntity;
	float		bestdot;
	Vector		bestdir;
	edict_t		*bestent;
	TraceResult tr;

	if ( g_psv_aim->value == 0 || g_psv_allow_autoaim->value == 0)
	{
		m_fOnTarget = FALSE;
		return g_vecZero;
	}

	UTIL_MakeVectors( pev->v_angle + pev->punchangle + m_vecAutoAim );

	// try all possible entities
	bestdir = gpGlobals->v_forward;
	bestdot = flDelta; // +- 10 degrees
	bestent = NULL;

	m_fOnTarget = FALSE;

	UTIL_TraceLine( vecSrc, vecSrc + bestdir * flDist, dont_ignore_monsters, edict(), &tr );


	if ( tr.pHit && tr.pHit->v.takedamage != DAMAGE_NO)
	{
		// don't look through water
		if (!((pev->waterlevel != 3 && tr.pHit->v.waterlevel == 3) 
			|| (pev->waterlevel == 3 && tr.pHit->v.waterlevel == 0)))
		{
			if (tr.pHit->v.takedamage == DAMAGE_AIM)
				m_fOnTarget = TRUE;

			return m_vecAutoAim;
		}
	}

	for ( int i = 1; i < gpGlobals->maxEntities; i++, pEdict++ )
	{
		Vector center;
		Vector dir;
		float dot;

		if ( pEdict->free )	// Not in use
			continue;
		
		if (pEdict->v.takedamage != DAMAGE_AIM)
			continue;
		if (pEdict == edict())
			continue;
//		if (pev->team > 0 && pEdict->v.team == pev->team)
//			continue;	// don't aim at teammate
		if ( !g_pGameRules->ShouldAutoAim( this, pEdict ) )
			continue;

		pEntity = Instance( pEdict );
		if (pEntity == NULL)
			continue;

		if (!pEntity->IsAlive())
			continue;

		// don't look through water
		if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) 
			|| (pev->waterlevel == 3 && pEntity->pev->waterlevel == 0))
			continue;

		center = pEntity->BodyTarget( vecSrc );

		dir = (center - vecSrc).Normalize( );

		// make sure it's in front of the player
		if (DotProduct (dir, gpGlobals->v_forward ) < 0)
			continue;

		dot = fabs( DotProduct (dir, gpGlobals->v_right ) ) 
			+ fabs( DotProduct (dir, gpGlobals->v_up ) ) * 0.5;

		// tweek for distance
		dot *= 1.0 + 0.2 * ((center - vecSrc).Length() / flDist);

		if (dot > bestdot)
			continue;	// to far to turn

		UTIL_TraceLine( vecSrc, center, dont_ignore_monsters, edict(), &tr );
		if (tr.flFraction != 1.0 && tr.pHit != pEdict)
		{
			// ALERT( at_console, "hit %s, can't see %s\n", STRING( tr.pHit->v.classname ), STRING( pEdict->v.classname ) );
			continue;
		}

		// don't shoot at friends
		if (IRelationship( pEntity ) < 0)
		{
			if ( !pEntity->IsPlayer() && !g_pGameRules->IsDeathmatch())
				// ALERT( at_console, "friend\n");
				continue;
		}

		// can shoot at this one
		bestdot = dot;
		bestent = pEdict;
		bestdir = dir;
	}

	if (bestent)
	{
		bestdir = UTIL_VecToAngles (bestdir);
		bestdir.x = -bestdir.x;
		bestdir = bestdir - pev->v_angle - pev->punchangle;

		if (bestent->v.takedamage == DAMAGE_AIM)
			m_fOnTarget = TRUE;

		return bestdir;
	}

	return Vector( 0, 0, 0 );
}

void CBasePlayer :: ResetAutoaim( )
{
	if (m_vecAutoAim.x != 0 || m_vecAutoAim.y != 0)
	{
		m_vecAutoAim = Vector( 0, 0, 0 );
		SET_CROSSHAIRANGLE( edict(), 0, 0 );
	}
	m_fOnTarget = FALSE;
}

/*
=============
SetCustomDecalFrames

  UNDONE:  Determine real frame limit, 8 is a placeholder.
  Note:  -1 means no custom frames present.
=============
*/
void CBasePlayer :: SetCustomDecalFrames( int nFrames )
{
	if (nFrames > 0 &&
		nFrames < 8)
		m_nCustomSprayFrames = nFrames;
	else
		m_nCustomSprayFrames = -1;
}

/*
=============
GetCustomDecalFrames

  Returns the # of custom frames this player's custom clan logo contains.
=============
*/
int CBasePlayer :: GetCustomDecalFrames( void )
{
	return m_nCustomSprayFrames;
}

void CBasePlayer::CleanupWeaponboxes(void)
{
	edict_t* ent = NULL;
	CBaseEntity* oldestBox = NULL;
	float oldestTime = FLT_MAX;
	int totalBoxes = 0;
	int thisEntIdx = ENTINDEX(edict());
	while (!FNullEnt(ent = FIND_ENTITY_BY_CLASSNAME(ent, "weaponbox"))) {
		CWeaponBox* oldbox = (CWeaponBox*)GET_PRIVATE(ent);
		if (oldbox && ent->v.owner && ENTINDEX(ent->v.owner) == thisEntIdx) {
			totalBoxes++;
			if (oldbox->m_spawnTime < oldestTime) {
				oldestBox = oldbox;
				oldestTime = oldbox->m_spawnTime;
			}
		}
	}

	if (totalBoxes > max_item_drops.value) {
		UTIL_Remove(oldestBox);
	}
}

//=========================================================
// DropPlayerItem - drop the named item, or if no name,
// the active item. 
//=========================================================
void CBasePlayer::DropPlayerItem ( char *pszItemName )
{
	if ( !g_pGameRules->IsMultiplayer() )
	{
		// no dropping in single player.
		return;
	}

	if (gpGlobals->time - m_lastDropTime < 0.2f) {
		return; // cooldown
	}

	if ( !strlen( pszItemName ) )
	{
		// if this string has no length, the client didn't type a name!
		// assume player wants to drop the active item.
		// make the string null to make future operations in this function easier
		pszItemName = NULL;
	} 

	CBasePlayerItem *pWeapon;
	int i;

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pWeapon = (CBasePlayerItem*)m_rgpPlayerItems[i].GetEntity();

		while ( pWeapon )
		{
			if ( pszItemName )
			{
				// try to match by name. 
				if ( !strcmp( pszItemName, STRING( pWeapon->pev->classname ) ) )
				{
					// match! 
					break;
				}
			}
			else
			{
				// trying to drop active item
				if ( pWeapon == m_pActiveItem.GetEntity() )
				{
					// active item!
					break;
				}
			}

			pWeapon = (CBasePlayerItem*)pWeapon->m_pNext.GetEntity();
		}

		
		// if we land here with a valid pWeapon pointer, that's because we found the 
		// item we want to drop and hit a BREAK;  pWeapon is the item.
		if ( pWeapon )
		{
			if ( !g_pGameRules->GetNextBestWeapon( this, pWeapon ) && !pWeapon->CanHolster())
				return; // can't drop the item they asked for, may be something we can't holster

			m_lastDropTime = gpGlobals->time;

			UTIL_MakeVectors ( pev->v_angle ); 

			pev->weapons &= ~(1<<pWeapon->m_iId);// take item off hud

			if (!strcmp(STRING(pWeapon->pev->classname), "weapon_shockrifle")) {
				// fixme: logic duplicated in kill code
				if (RemovePlayerItem(pWeapon)) {
					static std::map<std::string, std::string> keys = { {"is_player_ally", "1"} };
					Vector angles(0, pev->angles.y, 0);
					CBaseEntity* pRoach = CBaseEntity::Create("monster_shockroach",
						pev->origin + gpGlobals->v_forward * 10, angles, edict(), keys);
					pRoach->pev->velocity = gpGlobals->v_forward * 400;
				}
				return;
			}

			CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create( "weaponbox", pev->origin + gpGlobals->v_forward * 10, pev->angles, edict() );
			pWeaponBox->pev->angles.x = 0;
			pWeaponBox->pev->angles.z = 0;
			pWeaponBox->pev->avelocity = Vector(0, 256, 256);

			if (!strcmp(STRING(pWeapon->pev->classname), "weapon_9mmAR")) {
				// mp5 bounces look better with X rotations
				pWeaponBox->pev->avelocity = Vector(256, 256, 0);
			}

			pWeaponBox->PackWeapon( pWeapon );
			pWeaponBox->pev->velocity = pev->velocity + gpGlobals->v_forward * 400;
			
			CBasePlayerWeapon* wep = pWeapon->GetWeaponPtr();
			if (wep) {
				SET_MODEL(pWeaponBox->edict(), wep->GetModelW());
				if (!strcmp(STRING(pWeapon->pev->classname), "weapon_tripmine")) {
					pWeaponBox->pev->body = 3;
					pWeaponBox->pev->sequence = TRIPMINE_GROUND;
				}
			}

			// prevent players dropping at pickup points to get more ammo
			// unless the pickup point already has a cooldown via the LIMITINWORLD flag
			if (!(pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD))
				m_nextItemPickups[pWeapon->m_iId] = gpGlobals->time + item_repick_time.value;

			pWeaponBox->SetThink(&CWeaponBox::Kill);
			pWeaponBox->pev->nextthink = gpGlobals->time + item_despawn_time.value;

			// drop half of the ammo for this weapon.
			int	iAmmoIndex;

			iAmmoIndex = GetAmmoIndex ( pWeapon->pszAmmo1() ); // ???
			
			if ( iAmmoIndex != -1 )
			{
				// this weapon weapon uses ammo, so pack an appropriate amount.
				if ( pWeapon->iFlags() & ITEM_FLAG_EXHAUSTIBLE )
				{
					// pack up all the ammo, this weapon is its own ammo type
					pWeaponBox->PackAmmo( MAKE_STRING(pWeapon->pszAmmo1()), m_rgAmmo[ iAmmoIndex ] );
					m_rgAmmo[ iAmmoIndex ] = 0; 

				}
				else
				{
					// pack half of the ammo
					pWeaponBox->PackAmmo( MAKE_STRING(pWeapon->pszAmmo1()), m_rgAmmo[ iAmmoIndex ] / 2 );
					m_rgAmmo[ iAmmoIndex ] /= 2; 
				}

			}

			CleanupWeaponboxes();

			SetAnimation(PLAYER_DROP_ITEM);

			return;// we're done, so stop searching with the FOR loop.
		}
	}
}

void CBasePlayer::DropAmmo(bool secondary) {
	if (!m_pActiveItem) {
		return;
	}

	CBasePlayerWeapon* wep = m_pActiveItem.GetEntity()->GetWeaponPtr();

	if (!wep) {
		return;
	}

	int ammoIdx = secondary ? wep->SecondaryAmmoIndex() : wep->PrimaryAmmoIndex();

	if (ammoIdx < 0 || ammoIdx > MAX_AMMO_TYPES) {
		return;
	}

	int ammoLeft = m_rgAmmo[ammoIdx];
	int ammoTake = 0;
	const char* ammoEntName = NULL;
	
	wep->GetAmmoDropInfo(secondary, ammoEntName, ammoTake);

	ItemInfo info;
	wep->GetItemInfo(&info);
	bool isLastAmmo = ammoTake >= ammoLeft && info.iMaxClip < 0;

	if (!ammoEntName || ammoTake > ammoLeft || isLastAmmo) {
		return;
	}

	m_rgAmmo[ammoIdx] = ammoLeft - ammoTake;

	UTIL_MakeVectors(pev->v_angle);
	
	// get ammo entity model
	string_t model = 0;
	CBaseEntity* ammoEnt = (CBaseEntity*)CBaseEntity::Create(ammoEntName, pev->origin, pev->angles, edict());
	if (!ammoEnt) {
		ALERT(at_console, "Invalid ent in DropAmmo: %s\n", ammoEntName);
		return;
	}

	model = ammoEnt->pev->model;

	CWeaponBox* pWeaponBox = (CWeaponBox*)CBaseEntity::Create("weaponbox", pev->origin + gpGlobals->v_forward * 10, pev->angles, edict());
	pWeaponBox->pev->angles.x = 0;
	pWeaponBox->pev->angles.z = 0;
	pWeaponBox->pev->avelocity = Vector(0, 256, 256);
	pWeaponBox->pev->velocity = pev->velocity + gpGlobals->v_forward * 400;

	if (wep->iFlags() & ITEM_FLAG_EXHAUSTIBLE) {
		CBasePlayerWeapon* ammoWep = ammoEnt->GetWeaponPtr();
		if (ammoWep) {
			ammoWep->pev->solid = SOLID_NOT;
			pWeaponBox->PackWeapon(ammoWep);
		}
		else {
			REMOVE_ENTITY(ammoEnt->edict());
			ALERT(at_console, "Failed to drop ammo for exhaustible weapon %s\n", STRING(ammoEntName));
		}
		
	}
	else {
		REMOVE_ENTITY(ammoEnt->edict());
		const char* ammoName = secondary ? wep->pszAmmo2() : wep->pszAmmo1();
		pWeaponBox->PackAmmo(MAKE_STRING(ammoName), ammoTake);
	}
	

	if (model) {
		SET_MODEL(pWeaponBox->edict(), STRING(model));
		if (!strcmp(ammoEntName, "weapon_tripmine")) {
			pWeaponBox->pev->body = 3;
			pWeaponBox->pev->sequence = TRIPMINE_GROUND;
		}
	}

	pWeaponBox->SetThink(&CWeaponBox::Kill);
	pWeaponBox->pev->nextthink = gpGlobals->time + item_despawn_time.value;

	CleanupWeaponboxes();

	SetAnimation(PLAYER_DROP_ITEM);
}

//=========================================================
// HasPlayerItem Does the player already have this item?
//=========================================================
BOOL CBasePlayer::HasPlayerItem( CBasePlayerItem *pCheckItem )
{
	CBasePlayerItem* pItem = (CBasePlayerItem*)m_rgpPlayerItems[pCheckItem->iItemSlot()].GetEntity();

	while (pItem)
	{
		if (FClassnameIs( pItem->pev, STRING( pCheckItem->pev->classname) ))
		{
			return TRUE;
		}
		pItem = (CBasePlayerItem*)pItem->m_pNext.GetEntity();
	}

	return FALSE;
}

//=========================================================
// HasNamedPlayerItem Does the player already have this item?
//=========================================================
BOOL CBasePlayer::HasNamedPlayerItem( const char *pszItemName )
{
	CBasePlayerItem *pItem;
	int i;
 
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pItem = (CBasePlayerItem*)m_rgpPlayerItems[i].GetEntity();
		
		while (pItem)
		{
			if ( !strcmp( pszItemName, STRING( pItem->pev->classname ) ) )
			{
				return TRUE;
			}
			pItem = (CBasePlayerItem*)pItem->m_pNext.GetEntity();
		}
	}

	return FALSE;
}

//=========================================================
// 
//=========================================================
BOOL CBasePlayer :: SwitchWeapon( CBasePlayerItem *pWeapon ) 
{
	if ( !pWeapon->CanDeploy() )
	{
		return FALSE;
	}
	
	ResetAutoaim( );
	
	if (m_pActiveItem)
	{
		((CBasePlayerItem*)m_pActiveItem.GetEntity())->Holster();
	}

	m_pActiveItem = pWeapon;
	pWeapon->Deploy( );

	return TRUE;
}

// Find the next client in the game for this player to spectate
void CBasePlayer::Observer_FindNextPlayer(bool bReverse)
{
	// MOD AUTHORS: Modify the logic of this function if you want to restrict the observer to watching
	//				only a subset of the players. e.g. Make it check the target's team.

	int		iStart;
	if (m_hObserverTarget)
		iStart = ENTINDEX(m_hObserverTarget->edict());
	else
		iStart = ENTINDEX(edict());
	int	    iCurrent = iStart;
	m_hObserverTarget = NULL;
	int iDir = bReverse ? -1 : 1;

	do
	{
		iCurrent += iDir;

		// Loop through the clients
		if (iCurrent > gpGlobals->maxClients)
			iCurrent = 1;
		if (iCurrent < 1)
			iCurrent = gpGlobals->maxClients;

		CBaseEntity* pEnt = UTIL_PlayerByIndex(iCurrent);
		if (!pEnt)
			continue;
		if (pEnt == this)
			continue;
		// Don't spec observers or players who haven't picked a class yet
		if (((CBasePlayer*)pEnt)->IsObserver() || (pEnt->pev->effects & EF_NODRAW))
			continue;

		// MOD AUTHORS: Add checks on target here.

		m_hObserverTarget = pEnt;
		break;

	} while (iCurrent != iStart);

	// Did we find a target?
	if (m_hObserverTarget)
	{
		// Move to the target
		UTIL_SetOrigin(pev, m_hObserverTarget->pev->origin);

		// ALERT( at_console, "Now Tracking %s\n", STRING( m_hObserverTarget->pev->netname ) );

		// Store the target in pev so the physics DLL can get to it
		if (pev->iuser1 != OBS_ROAMING)
			pev->iuser2 = ENTINDEX(m_hObserverTarget->edict());



	}
}

// Handle buttons in observer mode
void CBasePlayer::Observer_HandleButtons()
{
	// Slow down mouse clicks
	if (m_flNextObserverInput > gpGlobals->time)
		return;

	// Jump changes from modes: Chase to Roaming
	if (m_afButtonPressed & IN_JUMP)
	{
		if (pev->iuser1 == OBS_CHASE_LOCKED)
			Observer_SetMode(OBS_CHASE_FREE);

		else if (pev->iuser1 == OBS_CHASE_FREE)
			Observer_SetMode(OBS_IN_EYE);

		else if (pev->iuser1 == OBS_IN_EYE)
			Observer_SetMode(OBS_ROAMING);

		else if (pev->iuser1 == OBS_ROAMING)
			Observer_SetMode(OBS_MAP_FREE);

		else if (pev->iuser1 == OBS_MAP_FREE)
			Observer_SetMode(OBS_MAP_CHASE);

		else
			Observer_SetMode(OBS_CHASE_LOCKED);	// don't use OBS_CHASE_LOCKED anymore

		m_flNextObserverInput = gpGlobals->time + 0.05;
	}

	// Attack moves to the next player
	if (m_afButtonPressed & IN_ATTACK)//&& pev->iuser1 != OBS_ROAMING )
	{
		Observer_FindNextPlayer(false);

		m_flNextObserverInput = gpGlobals->time + 0.05;
	}

	// Attack2 moves to the prev player
	if (m_afButtonPressed & IN_ATTACK2)// && pev->iuser1 != OBS_ROAMING )
	{
		Observer_FindNextPlayer(true);

		m_flNextObserverInput = gpGlobals->time + 0.05;
	}
}

void CBasePlayer::Observer_CheckTarget()
{
	if (pev->iuser1 == OBS_ROAMING)
		return;

	// try to find a traget if we have no current one
	if (m_hObserverTarget == NULL)
	{
		Observer_FindNextPlayer(false);

		if (m_hObserverTarget == NULL)
		{
			// no target found at all 

			int lastMode = pev->iuser1;

			Observer_SetMode(OBS_ROAMING);

			m_iObserverLastMode = lastMode;	// don't overwrite users lastmode

			return;	// we still have np target return
		}
	}

	CBasePlayer* target = (CBasePlayer*)(UTIL_PlayerByIndex(ENTINDEX(m_hObserverTarget->edict())));

	if (!target)
	{
		Observer_FindNextPlayer(false);
		return;
	}

	// check taget
	if (target->pev->deadflag == DEAD_DEAD)
	{
		if ((target->m_fDeadTime + 2.0f) < gpGlobals->time)
		{
			// 3 secs after death change target
			Observer_FindNextPlayer(false);
			return;
		}
	}
}

void CBasePlayer::Observer_CheckProperties()
{
	// try to find a traget if we have no current one
	if (pev->iuser1 == OBS_IN_EYE && m_hObserverTarget != NULL)
	{
		CBasePlayer* target = (CBasePlayer*)(UTIL_PlayerByIndex(ENTINDEX(m_hObserverTarget->edict())));

		if (!target)
			return;

		int weapon = target->m_pActiveItem ? ((CBasePlayerItem*)target->m_pActiveItem.GetEntity())->m_iId : 0;
		// use fov of tracked client
		if (m_iFOV != target->m_iFOV || m_iObserverWeapon != weapon)
		{
			m_iFOV = target->m_iFOV;
			m_iClientFOV = m_iFOV;
			// write fov before wepon data, so zoomed crosshair is set correctly
			MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
			WRITE_BYTE(m_iFOV);
			MESSAGE_END();


			m_iObserverWeapon = weapon;
			//send weapon update
			MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
			WRITE_BYTE(1);	// 1 = current weapon, not on target
			WRITE_BYTE(m_iObserverWeapon);
			WRITE_BYTE(0);	// clip
			MESSAGE_END();
		}
	}
	else
	{
		m_iFOV = 90;

		if (m_iObserverWeapon != 0)
		{
			m_iObserverWeapon = 0;

			MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
			WRITE_BYTE(1);	// 1 = current weapon
			WRITE_BYTE(m_iObserverWeapon);
			WRITE_BYTE(0);	// clip
			MESSAGE_END();
		}
	}
}

// Attempt to change the observer mode
void CBasePlayer::Observer_SetMode(int iMode)
{

	// Just abort if we're changing to the mode we're already in
	if (iMode == pev->iuser1)
		return;

	// is valid mode ?
	if (iMode < OBS_CHASE_LOCKED || iMode > OBS_MAP_CHASE)
		iMode = OBS_IN_EYE; // now it is
	// verify observer target again
	if (m_hObserverTarget != NULL)
	{
		CBaseEntity* pEnt = m_hObserverTarget;

		if ((pEnt == this) || (pEnt == NULL))
			m_hObserverTarget = NULL;
		else if (((CBasePlayer*)pEnt)->IsObserver() || (pEnt->pev->effects & EF_NODRAW))
			m_hObserverTarget = NULL;
	}

	// set spectator mode
	pev->iuser1 = iMode;

	// if we are not roaming, we need a valid target to track
	if ((iMode != OBS_ROAMING) && (m_hObserverTarget == NULL))
	{
		Observer_FindNextPlayer(false);

		// if we didn't find a valid target switch to roaming
		if (m_hObserverTarget == NULL)
		{
			UTIL_ClientPrint(edict(), print_center, "#Spec_NoTarget");
			pev->iuser1 = OBS_ROAMING;
		}
	}

	// set target if not roaming
	if (pev->iuser1 == OBS_ROAMING)
	{
		pev->iuser2 = 0;
	}
	else
		pev->iuser2 = ENTINDEX(m_hObserverTarget->edict());

	pev->iuser3 = 0;	// clear second target from death cam

	// print spepctaor mode on client screen

	switch (pev->iuser1) {
	case OBS_CHASE_LOCKED:
		UTIL_ClientPrint(edict(), print_center, "Locked Chase Cam\n[1/6]");
		break;
	case OBS_CHASE_FREE:
		UTIL_ClientPrint(edict(), print_center, "Free Chase Cam\n[2/6]");
		break;
	case OBS_IN_EYE:
		UTIL_ClientPrint(edict(), print_center, "First-Person Cam\n[3/6]");
		break;
	case OBS_ROAMING:
		UTIL_ClientPrint(edict(), print_center, "Free-Look\n[4/6]");
		break;
	case OBS_MAP_FREE:
		UTIL_ClientPrint(edict(), print_center, "Free Map Overview\n[5/6]");
		break;
	case OBS_MAP_CHASE:
		UTIL_ClientPrint(edict(), print_center, "Chase Map Overview\n[6/6]");
		break;
	}

	m_iObserverLastMode = iMode;
}

/*
// sven-style monster info
// TODO: try fixing the forced text fadeout
void CBasePlayer::UpdateMonsterInfo() {
	
	if (g_engfuncs.pfnTime() - m_lastMonsterInfoTrace < 0.1f) {
		return;
	}
	m_lastMonsterInfoTrace = g_engfuncs.pfnTime();

	UTIL_MakeVectors(pev->v_angle + pev->punchangle);
	Vector aimDir = gpGlobals->v_forward;

	TraceResult tr;
	TRACE_LINE(GetGunPosition(), GetGunPosition() + aimDir * 4096, dont_ignore_monsters, edict(), &tr);

	CBaseEntity* phit = CBaseEntity::Instance(tr.pHit);
	bool isLivingMonster = phit->IsMonster() && phit->IsAlive();

	if (!FNullEnt(phit) && isLivingMonster) {
		hudtextparms_t params;
		memset(&params, 0, sizeof(hudtextparms_t));
		params.effect = 0;
		params.fadeinTime = 0;
		params.fadeoutTime = 0.1;
		params.holdTime = 1.5f;
		params.x = 0.04;
		params.y = 0.582;
		params.channel = 3;

		int rel = IRelationship(phit);
		bool isFriendly = rel == R_AL || rel == R_NO;

		const char* relStr = "";
		const char* displayName = STRING(phit->pev->classname);
		if (phit->IsPlayer()) {
			params.r1 = 6;
			params.g1 = 170;
			params.b1 = 94;
			relStr = "Player: ";
		} else if (isFriendly) {
			params.r1 = 6;
			params.g1 = 170;
			params.b1 = 94;
			relStr = "Friend: ";
		} else {
			params.r1 = 255;
			params.g1 = 16;
			params.b1 = 16;
			relStr = "Enemy: ";
		}

		int hp = ceilf(phit->pev->health);
		const char* infoText = UTIL_VarArgs("%s%s\nHealth: %d", relStr, displayName, hp);
		
		float timeSinceLastMsg = g_engfuncs.pfnTime() - m_lastMonsterInfoMsg;
		
		if (strncmp(infoText, m_lastMonsterInfoText, 128) || timeSinceLastMsg > 1.0f) {
			UTIL_HudMessage(this, params, m_lastMonsterInfoText);
			m_lastMonsterInfoMsg = g_engfuncs.pfnTime();
			ALERT(at_console, "SEND MSG %f\n", m_lastMonsterInfoMsg);
		}
		
		strncpy(m_lastMonsterInfoText, infoText, 128);
		m_lastMonsterInfoText[128 - 1] = '\0';
	}
}
*/

void CBasePlayer::UpdateScore() {
	if (m_lastScore == (int)pev->frags || g_engfuncs.pfnTime() - m_lastScoreUpdate < 0.1f) {
		return;
	}
	m_lastScoreUpdate = g_engfuncs.pfnTime();
	m_lastScore = pev->frags;

	MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
	WRITE_BYTE(ENTINDEX(edict()));
	WRITE_SHORT(m_lastScore);
	WRITE_SHORT(m_iDeaths);
	WRITE_SHORT(0);
	WRITE_SHORT(g_pGameRules->GetTeamIndex(m_szTeamName));
	MESSAGE_END();
}

int CBasePlayer::GetNameColor() {
	if (IsObserver()) {
		return OBSERVER_TEAM_COLOR;
	}

	return DEFAULT_TEAM_COLOR;
}