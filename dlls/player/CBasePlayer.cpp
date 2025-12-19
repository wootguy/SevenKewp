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
#include "CBasePlayerAmmo.h"
#include "CItem.h"
#include "CTripmine.h"
#include "skill.h"
#include "CBreakable.h"
#include "CFuncVehicle.h"
#include "PluginManager.h"
#include "hlds_hooks.h"
#include "CItemInventory.h"
#include "CWeaponInventory.h"
#include "CGamePlayerEquip.h"
#include "CBaseButton.h"
#include "te_effects.h"
#include "CGib.h"
#include "bodyque.h"
#include "CWeaponCustom.h"
#include "CEnvWeather.h"
#include "prediction_files.h"
#include "CWorld.h"

// #define DUCKFIX

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL		g_fGameOver;
extern DLL_GLOBAL	BOOL	g_fDrawLines;
int gEvilImpulse101;
extern DLL_GLOBAL int gDisplayTitle;
extern float g_flWeaponCheat;

// how long the use key can be held before cancelling the use for antiblock or momentary buttons
#define MAX_USE_HOLD_TIME 0.3f

#define CVAR_REQ_SEVENKEWP			0	// SevenKewp test
#define CVAR_REQ_BUGFIXEDHL			1	// BugfixedHL test
#define CVAR_REQ_ADRENALINE_GAMER	2	// Adrenaline Gamer test
#define CVAR_REQ_HL25				3	// HL25 test
#define CVAR_REQ_OPENGL				4	// OpenGL test
#define CVAR_REQ_LINUX				5	// Linux test

BOOL gInitHUD = TRUE;

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


LINK_ENTITY_TO_CLASS( player, CBasePlayer )

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
		
		float oldArmor = pev->armorvalue;

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

		
		if (oldArmor > 0 && pev->armorvalue < 1) {
			SetSuitUpdate("!HEV_E0", FALSE, SUIT_NEXT_IN_30SEC); // armor compromised
		}
		else if (oldArmor - pev->armorvalue > 50) {
			// heavy hit
			SetSuitUpdate("!HEV_E4", FALSE, SUIT_NEXT_IN_30SEC); // hev damage sustained
		}
	}

	fTookDamage = CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);

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

	DropAllInventoryItems(true, false);

	if ( iWeaponRules == GR_PLR_DROP_GUN_NO && iAmmoRules == GR_PLR_DROP_AMMO_NO )
	{
		// nothing to pack. Remove the weapons and return. Don't call create on the box!
		return;
	}

// go through all of the weapons and make a list of the ones to pack
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			// there's a weapon here. Should I pack it?
			CBaseEntity* item = m_rgpPlayerItems[i].GetEntity();
			CBasePlayerWeapon *pPlayerItem = item ? item->GetWeaponPtr() : NULL;

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

				CBaseEntity* next = pPlayerItem->m_pNext.GetEntity();
				pPlayerItem = next ? next->GetWeaponPtr() : NULL;
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
			static StringMap keys = { {"is_player_ally", "1"} };
			Vector angles(0, pev->angles.y, 0);
			CBaseEntity* pRoach = CBaseEntity::Create("monster_shockroach",
				pev->origin + gpGlobals->v_forward * 10, angles, true, edict(), keys);
			pRoach->pev->velocity = pev->velocity * 1.2;
		
		}

		return;
	}

// create a box to pack the stuff into.
	CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create( "weaponbox", pev->origin, pev->angles, true, edict() );

	pWeaponBox->pev->angles.x = 0;// don't let weaponbox tilt.
	pWeaponBox->pev->angles.z = 0;

	pWeaponBox->SetThink( &CWeaponBox::Kill );
	pWeaponBox->pev->nextthink = gpGlobals->time + item_despawn_time.value;

	CWeaponCustom* cwep = firstWep ? firstWep->MyWeaponCustomPtr() : NULL;
	if (cwep && (cwep->params.flags & FL_WC_WEP_USE_ONLY)) {
		pWeaponBox->SetTouch(&CBaseEntity::ItemBounceTouch);
	}

// back these two lists up to their first elements
	iPA = 0;
	iPW = 0;

	bool bPackItems = rgpPackWeapons[iPW];

	if (iAmmoRules == GR_PLR_DROP_AMMO_ACTIVE && iWeaponRules == GR_PLR_DROP_GUN_ACTIVE)
	{
		if (firstWep && FClassnameIs(firstWep->pev, "weapon_satchel") && (iPackAmmo[0] == -1 || (m_rgAmmo[iPackAmmo[0]] == 0)))
		{
			bPackItems = FALSE;
		}
		if (firstWep && FClassnameIs(firstWep->pev, "weapon_inventory")) {
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
				int mergeBody = rgpPackWeapons[iPW]->CanAkimbo() ?
					rgpPackWeapons[iPW]->MergedModelBodyAkimbo() :
					rgpPackWeapons[iPW]->MergedModelBody();
				pWeaponBox->pev->body = mergeBody != -1 ? mergeBody : 0;
				pWeaponBox->pev->sequence = rgpPackWeapons[iPW]->pev->sequence;

				if (!strcmp(STRING(rgpPackWeapons[iPW]->pev->classname), "weapon_tripmine")) {
					pWeaponBox->pev->body = 3;
					pWeaponBox->pev->sequence = TRIPMINE_GROUND;
				}
			}

			iPW++;
		}

		pWeaponBox->pev->velocity = pev->velocity * 1.2;// weaponbox has player's velocity, then some.
		pWeaponBox->pev->avelocity = Vector(0, 256, 256);
	}
	else {
		UTIL_Remove(pWeaponBox);
	}

	CleanupWeaponboxes();
}

void CBasePlayer::HideAllItems(bool hideSuit) {
	pev->viewmodel = 0;
	pev->weaponmodel = 0;
	m_weaponBits = hideSuit ? 0 : (1ULL << WEAPON_SUIT);

	UpdateClientData();

	// send Selected Weapon Message to our client
	MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	MESSAGE_END();
}

void CBasePlayer::RemoveAllItems( BOOL removeSuit, BOOL removeItemsOnly)
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

	int i;
	CBaseEntity*pPendingItem;
	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		m_pActiveItem = m_rgpPlayerItems[i].GetEntity();
		while (m_pActiveItem)
		{
			CBaseEntity* ent = m_pActiveItem.GetEntity();
			CBasePlayerItem* item = ent ? ent->GetWeaponPtr() : NULL;
			pPendingItem = item ? item->m_pNext.GetEntity() : NULL;
			if (item)
				item->Drop( );
			m_pActiveItem = pPendingItem;
		}
		m_rgpPlayerItems[i] = NULL;
	}
	m_pActiveItem = NULL;

	for (int k = 0; k < MAX_AMMO_SLOTS; k++)
		m_rgAmmo[k] = 0;

	if (!removeItemsOnly)
		HideAllItems(removeSuit);

	ApplyEffects();
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

	SetRevivalVars();

	m_killedTime = gpGlobals->time;

	// Holster weapon immediately, to allow it to cleanup
	if ( m_pActiveItem )
		((CBasePlayerItem*)m_pActiveItem.GetEntity())->Holster();

	PenalizeDeath();

	g_pGameRules->PlayerKilled( this, pevAttacker, g_pevLastInflictor );

	HideAllItems(true);
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
	UpdateTeamInfo();


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

	m_waterFriction = 0.95f;
	m_buoyancy = 0.2f;
	
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

	bool hasNewAnims = m_playerModelAnimSet != PMODEL_ANIMS_HALF_LIFE;

	if (!hasNewAnims && playerAnim == PLAYER_DEPLOY_WEAPON) {
		return; // HL models don't have these animations
	}

	speed = pev->velocity.Length2D();

	bool upperBodyActing = false;

	switch (m_Activity) {
	case ACT_RANGE_ATTACK1:
	case ACT_SPECIAL_ATTACK1:
	case ACT_SPECIAL_ATTACK2:
	case ACT_RELOAD:
	case ACT_SIGNAL1:
	case ACT_SIGNAL2:
	case ACT_USE:
	case ACT_ARM:
	case ACT_DISARM:
	case ACT_THREAT_DISPLAY:
		upperBodyActing = true;
		break;
	default:
		break;
	}

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
	case PLAYER_RELOAD2:
	case PLAYER_RELOAD3:
	case PLAYER_ATTACK1:
	case PLAYER_ATTACK2:
	case PLAYER_ATTACK3:
	case PLAYER_USE:
	case PLAYER_DROP_ITEM:
	case PLAYER_DEPLOY_WEAPON:
	case PLAYER_COCK_WEAPON:
		switch( m_Activity )
		{
		case ACT_DIESIMPLE:
			m_IdealActivity = m_Activity;
			break;
		default:
			if (playerAnim == PLAYER_RELOAD) {
				m_IdealActivity = ACT_RELOAD;
			}
			else if (playerAnim == PLAYER_RELOAD2) {
				m_IdealActivity = ACT_SIGNAL1;
			}
			else if (playerAnim == PLAYER_RELOAD3) {
				m_IdealActivity = ACT_SIGNAL2;
			}
			else if (playerAnim == PLAYER_ATTACK1) {
				m_IdealActivity = ACT_RANGE_ATTACK1;
			}
			else if (playerAnim == PLAYER_ATTACK2) {
				m_IdealActivity = ACT_SPECIAL_ATTACK1;
			}
			else if (playerAnim == PLAYER_ATTACK3) {
				m_IdealActivity = ACT_SPECIAL_ATTACK2;
			}
			else if (playerAnim == PLAYER_DROP_ITEM) {
				m_IdealActivity = ACT_DISARM;
			}
			else if (playerAnim == PLAYER_DEPLOY_WEAPON) {
				m_IdealActivity = ACT_ARM;
			}
			else if (playerAnim == PLAYER_COCK_WEAPON) {
				m_IdealActivity = ACT_THREAT_DISPLAY;
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
		if (m_afPhysicsFlags & PFLAG_ONBARNACLE) {
			m_IdealActivity = m_isBarnacleFood ? ACT_BARNACLE_CHEW : ACT_BARNACLE_PULL;
		}
		else if (pev->waterlevel > 1)
		{
			if (speed < 128 && !(pev->flags & FL_DUCKING))
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
	case PLAYER_BARNACLE_HIT:
		m_IdealActivity = ACT_BARNACLE_HIT;
		break;
	case PLAYER_BARNACLE_CRUNCH:
		m_IdealActivity = ACT_BARNACLE_CHOMP;
		break;
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
		if (isFloating && upperBodyActing && !m_fSequenceFinished) {
			pev->gaitsequence = LookupActivity(ACT_HOVER);
			return;
		}

		m_Activity = m_IdealActivity;

		animDesired = LookupActivity(m_Activity);
		// Already using the desired animation?
		if (pev->sequence == animDesired)
			return;

		if (hasNewAnims) {
			if (m_IdealActivity == ACT_HOP) {
				animDesired = LookupSequence("jump2"); // totally different anim
			}
			if (m_IdealActivity == ACT_LEAP) {
				animDesired = LookupSequence("long_jump2"); // more accurate hitboxes
			}
		}

		pev->gaitsequence = 0;
		pev->sequence = animDesired;
		pev->frame = 0;
		ResetSequenceInfo();
		return;
	}
	case ACT_RANGE_ATTACK1:
	case ACT_SPECIAL_ATTACK1:
	case ACT_SPECIAL_ATTACK2:
	{
		int minAttackFrame = hasNewAnims ? 58 : 220;
		if ((m_Activity == ACT_HOP || m_Activity == ACT_LEAP) && pev->frame < minAttackFrame) {
			// jump animation has priority
			return;
		}

		if (FBitSet(pev->flags, FL_DUCKING))	// crouching
			strcpy_safe(szAnim, "crouch_shoot_", 64);
		else
			strcpy_safe(szAnim, "ref_shoot_", 64);

		strcat_safe(szAnim, m_szAnimExtention, 64);

		if (hasNewAnims && !strcmp(m_szAnimExtention, "shotgun")) {
			strcat_safe(szAnim, "2", 64); // the second anim includes cocking
		}

		if (hasNewAnims && !strcmp(m_szAnimExtention, "uzis")) {
			if (m_IdealActivity == ACT_RANGE_ATTACK1) {
				strcat_safe(szAnim, "_both", 64);
			}
			else if (m_IdealActivity == ACT_SPECIAL_ATTACK1) {
				strcat_safe(szAnim, "_left", 64);
			}
			else if (m_IdealActivity == ACT_SPECIAL_ATTACK2) {
				strcat_safe(szAnim, "_right", 64);
			}
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

		if (hasNewAnims) {
			pev->framerate = 1.0f; // undo custom model adjustments (xbow shoot)
		}

		break;
	}
	
	case ACT_BARNACLE_CHEW:
	case ACT_BARNACLE_PULL:
	{
		bool specialAnimPlaying = m_Activity == ACT_BARNACLE_HIT || m_Activity == ACT_BARNACLE_CHOMP;
		if (specialAnimPlaying && !m_fSequenceFinished) {
			return;
		}

		if (hasNewAnims) {
			if (m_IdealActivity == ACT_BARNACLE_CHEW)
				animDesired = LookupSequence("barnaclechew");
			else
				animDesired = LookupSequence("barnaclepull");
		}
		else {
			animDesired = LookupSequence("headshot");
		}

		if (pev->sequence == animDesired)
			return;

		m_Activity = m_IdealActivity;
		pev->sequence = animDesired;

		if (hasNewAnims) {
			pev->gaitsequence = pev->sequence;
			pev->frame = 0;
			ResetSequenceInfo();
		}
		else {
			pev->gaitsequence = LookupSequence("treadwater");
			pev->frame = (2.0f / 28.0f) * 255.0f;
			ResetSequenceInfo();
			pev->framerate = FLT_MIN;
		}
		return;
	}

	case ACT_BARNACLE_HIT:
		animDesired = LookupSequence(hasNewAnims ? "barnaclehit" : "headshot");
		m_Activity = m_IdealActivity;
		pev->sequence = animDesired;
		
		if (hasNewAnims) {
			pev->gaitsequence = pev->sequence;
			pev->frame = 0;
			ResetSequenceInfo();
		}
		else {
			pev->gaitsequence = LookupSequence("treadwater");
			pev->frame = (2.0f / 28.0f) * 255.0f;
			ResetSequenceInfo();
			pev->framerate = FLT_MIN;
		}
		return;

	case ACT_BARNACLE_CHOMP:
		animDesired = LookupSequence(hasNewAnims ? "barnaclecrunch" : "headshot");
		m_Activity = m_IdealActivity;
		pev->sequence = animDesired;
		pev->frame = 0;
		ResetSequenceInfo();

		if (hasNewAnims) {
			pev->gaitsequence = pev->sequence;
			pev->frame = 0;
			ResetSequenceInfo();
		}
		else {
			pev->gaitsequence = LookupSequence("treadwater");
			pev->frame = (3.0f / 28.0f) * 255.0f;
			ResetSequenceInfo();
			pev->framerate = FLT_MIN;
		}
		return;

	case ACT_USE:
	case ACT_DISARM:
	case ACT_ARM:
	case ACT_RELOAD:
	case ACT_SIGNAL1:
	case ACT_SIGNAL2:
	case ACT_THREAT_DISPLAY:
	{
		if ((m_Activity == ACT_HOP || m_Activity == ACT_LEAP) && pev->frame < 200) {
			// jump animation has priority
			return;
		}

		bool ducking = FBitSet(pev->flags, FL_DUCKING);
		const char* seqName = "";

		if (m_IdealActivity == ACT_ARM) {
			strcpy_safe(szAnim, ducking ? "crouch_draw_" : "ref_draw_", 64);
			strcat_safe(szAnim, m_szAnimExtention, 64);
			seqName = szAnim;
		}
		else if (m_IdealActivity == ACT_DISARM) {
			seqName = ducking ? "crouch_shoot_squeak" : "ref_shoot_squeak";
		}
		else if (m_IdealActivity == ACT_USE) {
			seqName = ducking ? "crouch_shoot_trip" : "ref_shoot_trip";
		}
		else if (m_IdealActivity == ACT_THREAT_DISPLAY) {
			if (hasNewAnims) {
				strcpy_safe(szAnim, ducking ? "crouch_cock_" : "ref_cock_", 64);
				strcat_safe(szAnim, m_szAnimExtention, 64);
				seqName = szAnim;
			}
			else {
				// the only animation that looks like charging up is a reversed crowbar swing
				seqName = "ref_shoot_crowbar";
			}
		}
		else { // reload

			// snark petting kind of looks like a reload in HL
			seqName = ducking ? "crouch_aim_squeak" : "ref_aim_squeak";

			if (hasNewAnims) {
				duration = 0; // the hacky way to add new anims is not needed for SC models where each gun has its own anim.
				strcpy_safe(szAnim, ducking ? "crouch_reload_" : "ref_reload_", 64);
				strcat_safe(szAnim, m_szAnimExtention, 64);
				seqName = szAnim;

				if (!strcmp(m_szAnimExtention, "uzis")) {
					if (m_IdealActivity == ACT_SIGNAL1)
						strcat_safe(szAnim, "_left", 64);
					else if (m_IdealActivity == ACT_SIGNAL2)
						strcat_safe(szAnim, "_right", 64);
				}
			}
		}

		void* mdl = m_playerModel ? m_playerModel : GET_MODEL_PTR(ENT(pev));
		animDesired = ::LookupSequence(mdl, seqName);

		if (animDesired == -1)
			animDesired = 0;

		if (pev->sequence != animDesired || !m_fSequenceLoops)
		{
			pev->frame = 0;
		}

		//if (!m_fSequenceLoops)
		{
			// TODO: this isn't enough. Deploy anims are skipping some early frames.
			pev->effects |= EF_NOINTERP;
		}

		m_Activity = m_IdealActivity;

		pev->sequence = animDesired;
		ResetSequenceInfo();

		if (m_IdealActivity == ACT_THREAT_DISPLAY && !hasNewAnims) {
			bool isGrenade = !strcmp(m_szAnimExtention, "grenade");

			if (isGrenade) {
				pev->frame = (3.0f / 13.0f) * 255.0f;
				ResetSequenceInfo();
				pev->framerate = -0.08f;
			}
			else { // wrench
				pev->frame = (3.5f / 13.0f) * 255.0f;
				ResetSequenceInfo();
				pev->framerate = -0.05f;
			}
		}

		break;
	}
	case ACT_WALK:
	{
		if (!upperBodyActing || m_fSequenceFinished)
		{
			if (FBitSet(pev->flags, FL_DUCKING))	// crouching
				strcpy_safe(szAnim, "crouch_", 64);
			else
				strcpy_safe(szAnim, "ref_", 64);

			strcat_safe(szAnim, m_szAnimAction, 64);
			strcat_safe(szAnim, "_", 64);
			strcat_safe(szAnim, m_szAnimExtention, 64);
			animDesired = LookupSequence(szAnim);

			if (animDesired == -1) {
				// no weapons held animations for the upper body

				if (FBitSet(pev->flags, FL_DUCKING)) {
					if (speed > 0) {
						animDesired = 6;
						SyncGaitAnimations(animDesired, speed, 0.0135f);
					}
					else if (speed == 0) {
						animDesired = 7;
					}
				}
				else {
					if (speed > 220) {
						animDesired = 3;
						SyncGaitAnimations(animDesired, speed, 0.003f);
					}
					else if (speed > 0) {
						animDesired = 4;
						SyncGaitAnimations(animDesired, speed, 0.0155f);
					}
					else if (speed == 0) {
						animDesired = 0;
					}
				}
			}

			m_Activity = ACT_WALK;
		}
		else
		{
			animDesired = pev->sequence;

			if (m_Activity == ACT_THREAT_DISPLAY && !hasNewAnims) {
				// stop the chargeup animation
				bool isGrenade = !strcmp(m_szAnimExtention, "grenade");
				float endFrame = ((isGrenade ? 2.2f : 2.5f) / 13.0f) * 255.0f;

				if (pev->frame < endFrame) {
					pev->frame = endFrame;
					pev->framerate = FLT_MIN;
				}
			}
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
		if (hasNewAnims) {
			pev->gaitsequence = pev->sequence;
		}
		else {
			pev->gaitsequence = LookupSequence("deep_idle");
		}
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

void CBasePlayer::TabulateWeapons(void) {
	for (int i = 0; i < MAX_ITEM_TYPES; i++) {
		if (m_rgpPlayerItems[i]) {
			CBaseEntity* ent = m_rgpPlayerItems[i].GetEntity();
			CBasePlayerItem* pPlayerItem = ent ? ent->GetWeaponPtr() : NULL;

			while (pPlayerItem) {
				m_weaponBits |= g_weaponSlotMasks[pPlayerItem->m_iId];

				CBaseEntity* next = pPlayerItem->m_pNext.GetEntity();
				pPlayerItem = next ? next->GetWeaponPtr() : NULL;
			}
		}
	}

	if (!g_noSuit)
		m_weaponBits |= (1ULL << WEAPON_SUIT);
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
	{
		pev->air_finished = gpGlobals->time + AIRTIME;
		return;
	}

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

		pev->air_finished = gpGlobals->time + AIRTIME + m_airTimeModifier;
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
	if (!RANDOM_LONG(0,0x1f) && RANDOM_LONG(0,(AIRTIME + m_airTimeModifier)-1) >= air && IsAlive())
	{
		int irand = RANDOM_LONG(0, g_footstepVariety - 1) + (RANDOM_LONG(0, 1) * 2);
		EMIT_SOUND(ENT(pev), CHAN_BODY, g_swimSounds[irand], 0.8, ATTN_NORM);
	}

	if (pev->watertype == CONTENTS_LAVA)		// do damage
	{
		if (pev->dmgtime < gpGlobals->time) {
			TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 10 * pev->waterlevel, DMG_BURN);
			SetSuitUpdate("!HEV_FIRE", FALSE, SUIT_NEXT_IN_1MIN); // extreme heat damage
			pev->dmgtime = gpGlobals->time + 0.1f;
		}
	}
	else if (pev->watertype == CONTENTS_SLIME)		// do damage
	{
		if (pev->dmgtime < gpGlobals->time) {
			TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), 10 * pev->waterlevel, DMG_ACID);
			pev->dmgtime = gpGlobals->time + 0.1f;
		}
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

	// make sure a death animation is playing
	// (can be different if killed by multiple explosives during the same frame by shooting them)
	switch (m_IdealActivity) {
	case ACT_DIESIMPLE:
	case ACT_DIE_HEADSHOT:
	case ACT_DIE_GUTSHOT:
	case ACT_DIEFORWARD:
	case ACT_DIEBACKWARD:
		break;
	default:
		SetAnimation(PLAYER_DIE);
	}

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		flForward = pev->velocity.Length() - 20;
		if (flForward <= 0)
			pev->velocity = g_vecZero;
		else    
			pev->velocity = flForward * pev->velocity.Normalize();
	}

	if ( !m_droppedDeathWeapons )
	{
		// we drop the guns here because weapons that have an area effect and can kill their user
		// will sometimes crash coming back from CBasePlayer::Killed() if they kill their owner because the
		// player class sometimes is freed. It's safer to manipulate the weapons once we know
		// we aren't calling into any of their code anymore through the player pointer.
		PackDeadPlayerItems();
		m_droppedDeathWeapons = true;
	}

	float deadTime = gpGlobals->time - m_killedTime;
	float respawnDelay = mp_respawndelay.value + m_extraRespawnDelay;

	if (deadTime > 1.0f && deadTime < respawnDelay + 2.0f) {
		if (gpGlobals->time - m_lastSpawnMessage > 0.2f) {
			int timeLeft = (int)ceilf(respawnDelay - deadTime);
			std::string delayMsg = UTIL_VarArgs("Respawn allowed in %d seconds", timeLeft);

			if (deadTime >= respawnDelay) {
				delayMsg = "You can respawn now!";
			} else if (timeLeft == 1) {
				delayMsg = "Respawn allowed in 1 second";
			}

			std::string pointsMessage;
			if (mp_score_mode.value == 1) {
				// deaths are penalized. Show current score multiplier
				bool sameAsLast = m_iDeaths > 1 && fabs(GetScoreMultiplier() - GetScoreMultiplier(m_iDeaths - 1)) < 0.001f;

				if (sameAsLast) {
					pointsMessage = UTIL_VarArgs("\n\nScore multiplier: %d%%",
						(int)roundf(m_scoreMultiplier * 100.0f));
				}
				else {
					pointsMessage = UTIL_VarArgs("\n\nNew score multiplier: %d%% (%d death%s)",
						(int)roundf(m_scoreMultiplier * 100.0f), m_iDeaths, m_iDeaths == 1 ? "" : "s");
				}
			}

			const char* msg = UTIL_VarArgs("%s%s", delayMsg.c_str(), pointsMessage.c_str());

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
			CreatePlayerCorpse(this);
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
	CBaseEntity *pSpot, *pNewSpot;
	int iRand;

	if ( pev->view_ofs == g_vecZero )
	{
		// don't accept subsequent attempts to StartDeathCam()
		return;
	}

	pSpot = UTIL_FindEntityByClassname( NULL, "info_intermission");

	if ( pSpot )
	{
		// at least one intermission spot in the world.
		iRand = RANDOM_LONG( 0, 3 );

		while ( iRand > 0 )
		{
			pNewSpot = UTIL_FindEntityByClassname( pSpot, "info_intermission");
			
			if ( pNewSpot )
			{
				pSpot = pNewSpot;
			}

			iRand--;
		}

		CreatePlayerCorpse( this );

		UTIL_SetOrigin( pev, pSpot->pev->origin );
		pev->angles = pev->v_angle = pSpot->pev->v_angle;
	}
	else
	{
		// no intermission spot. Push them up in the air, looking down at their corpse
		TraceResult tr;
		CreatePlayerCorpse( this );
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
	if (IsAlive()) {
		SetRevivalVars();
		PenalizeDeath();
	}

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
	m_iHideHUD = (HIDEHUD_HEALTH_AND_ARMOR | HIDEHUD_WEAPONS);
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
	pev->viewmodel = 0; // prevent floating view models
	m_weaponBits = 0; // no weapon switching

	// Clear out the status bar
	m_fInitHUD = TRUE;

	pev->team =  0;
	UpdateTeamInfo();

	// Move them to the new position
	UTIL_SetOrigin( pev, vecPosition );

	// Find a player to watch
	m_flNextObserverInput = 0;
	Observer_SetMode( m_iObserverLastMode );
}

void CBasePlayer::LeaveObserver(bool respawn)
{
	if (!m_isObserver) {
		return;
	}

	ClearBits( m_afPhysicsFlags, PFLAG_OBSERVER );
	pev->iuser1 = OBS_NONE;
	pev->iuser2 = 0;
	pev->iuser3 = 0;
	m_isObserver = false;
	m_lastObserverSwitch = gpGlobals->time;
	m_iHideHUD = 0;

	pev->effects &= ~EF_NODRAW;
	pev->takedamage = DAMAGE_YES;

	UpdateTeamInfo();

	// fixes scoreboard
	m_fInitHUD = true;
	//m_fGameHUDInitialized = false;

	if (respawn)
		Spawn();
}

// 
// PlayerUse - handles USE keypress
//
void CBasePlayer::PlayerUse ( void )
{
	CALL_HOOKS_VOID(pfnPlayerUse, this);

	if ( IsObserver() )
		return;

	// Was use pressed or released?
	if ( ! ((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_USE) )
		return;

	// continuous use animation
	if ((pev->button & IN_USE) && (m_Activity == ACT_WALK || m_Activity == ACT_USE)) {
		if (m_Activity != ACT_USE) {
			SetAnimation(PLAYER_USE);
		}
		else {
			if (pev->framerate > 0 && pev->frame > 85) {
				pev->frame = 85;
				pev->framerate = -0.2f;
			}
			else if (pev->framerate < 0 && pev->frame < 65) {
				pev->frame = 65;
				pev->framerate = 0.2f;
			}
		}
	}
	else if (m_Activity == ACT_USE) {
		pev->framerate = 1.0f;
	}

	Vector viewPos = pev->origin + pev->view_ofs;
	UTIL_MakeVectors(pev->v_angle);// so we know which way we are facing

	// Hit Use on a train?
	if ( m_afButtonPressed & IN_USE )
	{
		SetAnimation(PLAYER_USE);

		// funny splashes
		WaterSplashTrace(viewPos, 48, point_hull, 0.4f);

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

				CBaseEntity* pTrain = CBaseEntity::Instance(pev->groundentity);
				if (pTrain && (pTrain->Classify() == CLASS_VEHICLE))
					((CFuncVehicle*)pTrain)->m_pDriver = NULL;

				m_useExpired = true;

				return;
			}
			else
			{	// Start controlling the train!
				CBaseEntity *pTrain = CBaseEntity::Instance( pev->groundentity );

				if ( pTrain && !(pev->button & IN_JUMP) && FBitSet(pev->flags, FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(pev) )
				{
					if (!pTrain->RunInventoryRules(this)) {
						return;
					}

					m_afPhysicsFlags |= PFLAG_ONTRAIN;
					m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
					m_iTrain |= TRAIN_NEW;
					if (pTrain->Classify() == CLASS_VEHICLE)
					{
						EMIT_SOUND(ENT(pev), CHAN_ITEM, "plats/vehicle_ignition.wav", 0.8, ATTN_NORM);
						((CFuncVehicle*)pTrain)->m_pDriver = this;
					}
					else
						EMIT_SOUND(ENT(pev), CHAN_ITEM, "plats/train_use1.wav", 0.8, ATTN_NORM);

					m_useExpired = true;

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

	CBaseEntity* pObject = NULL;
	CBaseEntity* pClosest = NULL;
	CBaseEntity* pLooking = NULL;
	float flMaxDot = VIEW_FIELD_NARROW;

	TraceResult tr;
	TRACE_LINE(viewPos, viewPos + gpGlobals->v_forward * PLAYER_SEARCH_RADIUS*2, dont_ignore_monsters, edict(), &tr);
	pLooking = Instance(tr.pHit);

	while ((pObject = UTIL_FindEntityInSphere(pObject, pev->origin, PLAYER_SEARCH_RADIUS)) != NULL)
	{
		if (pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE))
		{
			// object is being looked at directly. No other option can be better than this
			if (pObject == pLooking) {
				pClosest = pObject;
				break;
			}

			Vector vecLOS;

			if (pObject->IsBSPModel()) {
				// This essentially moves the origin of the target to the corner nearest the player
				// to test to see if it's "hull" is in the view cone. This is required for using very
				// tall buttons. Looking at the bottom of one doesn't pass the FOV test otherwise.
				vecLOS = (VecBModelOrigin(pObject->pev) - viewPos);
				vecLOS = UTIL_ClampVectorToBox(vecLOS, pObject->pev->size * 0.5);
			}
			else {
				// for regular-sized box-shaped entities the center makes the most sense
				// otherwise it's hard to pick out items in clusters
				vecLOS = pObject->Center() - viewPos;
			}

			//te_debug_beam(viewPos, viewPos + vecLOS, 1, RGB(255, 0, 0));

			float flDot = DotProduct(vecLOS.Normalize(), gpGlobals->v_forward);

			if (flDot > flMaxDot)
			{// only if the item is in front of the user
				pClosest = pObject;
				flMaxDot = flDot;
				//ALERT( at_console, "%s : %f\n", STRING( pObject->pev->classname ), flDot );
			}
		}
	}
	//if (pClosest)
	//	te_debug_box(pClosest->pev->absmin, pClosest->pev->absmax, 1, RGB(0, 255, 0));

	pObject = pClosest;

	bool useExpiring = (mp_antiblock.value && GetUseTime() > MAX_USE_HOLD_TIME)
					|| (!mp_antiblock.value && (m_afButtonPressed & IN_USE));

	if (!m_useExpired && useExpiring && mp_antiblock.value && (!m_usingMomentary || pLooking->IsPlayer())) {
		int antiblockRet = TryAntiBlock();

		if (antiblockRet) {
			if (antiblockRet == 1) {
				EMIT_SOUND_DYN(edict(), CHAN_BODY, "weapons/xbow_hitbod2.wav", 0.7f, 1.0f, 0, 130 + RANDOM_LONG(0, 10));
			}
			else {
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/wpn_denyselect.wav", 0.8, ATTN_NORM);
			}
			
			// swap logic ran. Don't also use an entity
			m_useExpired = true;
			return;
		}
	}

	if (pObject)
	{
		//!!!UNDONE: traceline here to prevent USEing buttons through walls			
		int caps = pObject->ObjectCaps();
		bool isContinuousUse = (pev->button & IN_USE) && (caps & FCAP_CONTINUOUS_USE);
		bool shouldToggle = (m_afButtonReleased & IN_USE) || useExpiring;

		// if antiblock is enabled, don't trigger things until the button is released
		// because the player might be holding it down for an antiblock attempt
		bool isToggleUse = shouldToggle && (caps & (FCAP_IMPULSE_USE | FCAP_ONOFF_USE));

		if (isContinuousUse || (isToggleUse && !m_useExpired))
		{
			if ((isToggleUse || ((m_afButtonPressed & IN_USE) && isContinuousUse)) && !m_usingMomentary)
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/wpn_select.wav", 0.4, ATTN_NORM);

			if (isToggleUse) {
				m_useExpired = true;
			}
			else {
				m_usingMomentary = true;
			}

			if ( caps & FCAP_CONTINUOUS_USE )
				m_afPhysicsFlags |= PFLAG_USING;

			FireTarget(pObject, this, this, USE_SET, 1);

			return;
		}
	}

	// nothing has been activated within the use time limit, cancel use and don't consider any more
	// uses until the next key press
	if (useExpiring || (m_afButtonReleased & IN_USE)) {
		if (!m_usingMomentary && !m_useExpired)
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/wpn_denyselect.wav", 0.8, ATTN_NORM);
		m_useExpired = true; // don't consider any more uses until the button is pressed again
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

	// JoshA: CS behaviour does this for tracktrain + train as well,
	// but let's just do this for func_vehicle to avoid breaking existing content.
	//
	// If you're standing on a moving train... then add the velocity of the train to yours.
	if (pevGround && ( /*(!strcmp( "func_tracktrain", STRING(pevGround->classname))) ||
							(!strcmp( "func_train", STRING(pevGround->classname))) ) ||*/
		(!strcmp("func_vehicle", STRING(pevGround->classname))))) {
			pev->velocity = pev->velocity + pevGround->velocity;
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

	UpdateTeamInfo();
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
	if (IsSevenKewpClient())
		return; // the custom client handles this

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

	struct FakePlayerInfo {
		bool enabled;
		int color;
		const char* name;
	};

	FakePlayerInfo fakePlayerInfo;
	fakePlayerInfo.enabled = false;
	fakePlayerInfo.color = DEFAULT_TEAM_COLOR;
	fakePlayerInfo.name = "\\no name\\";

	std::string name;
	bool lookingAtStatusEnt = false;
	bool shouldShowHud = !IsObserver() || pev->iuser1 == OBS_MAP_FREE || pev->iuser1 == OBS_ROAMING;

	if (shouldShowHud && tr.flFraction != 1.0 && !FNullEnt( tr.pHit ) )
	{
		CBaseEntity *pEntity = CBaseEntity::Instance( tr.pHit );

		bool ignoreMonster = FClassnameIs(pEntity->pev, "monster_furniture");

		if (pEntity->IsPlayer() )
		{
			newSBarState[ SBAR_ID_TARGETNAME ] = ENTINDEX( pEntity->edict() );
			strcpy_safe( sbuf1, "1 %p1", SBAR_STRING_SIZE );
			strcpy_safe( sbuf0, "2 Health: %i2\n3 Armor: %i3", SBAR_STRING_SIZE);

			newSBarState[ SBAR_ID_TARGETHEALTH ] = clampi(roundf(pEntity->pev->health), INT16_MIN, INT16_MAX);
			newSBarState[ SBAR_ID_TARGETARMOR ] = clampi(roundf(pEntity->pev->armorvalue), INT16_MIN, INT16_MAX);

			lookingAtStatusEnt = true;
		}
		else if (pEntity->IsMonster() && pEntity->IsAlive() && !ignoreMonster) {
			name = replaceString(pEntity->DisplayName(), "\n", " ");
			long long hp = roundf(pEntity->pev->health);

			int irel = IRelationship(pEntity);

			fakePlayerInfo.enabled = true;
			fakePlayerInfo.color = NEUTRAL_TEAM_COLOR;
			fakePlayerInfo.name = name.c_str();
				
			if (irel == R_AL) {
				fakePlayerInfo.color = FRIEND_TEAM_COLOR;
			}
			else if (irel == R_DL || irel == R_HT || irel == R_NM) {
				fakePlayerInfo.color = ENEMY_TEAM_COLOR;
			}

			std::string desc = "";
			if (name.size() > 32) {
				// player names can only be 32 chars long
				desc = name.substr(31).c_str();
			}

			strcpy_safe(sbuf1, UTIL_VarArgs("1 %%p1%s", desc.c_str()), SBAR_STRING_SIZE);

			if ((pEntity->pev->flags & FL_GODMODE) || (pEntity->pev->takedamage == DAMAGE_NO) || pEntity->pev->health > 2147483647) {
				strcpy_safe(sbuf0, "2 Health: Invincible", SBAR_STRING_SIZE);
				hp = 1; // client won't show health text if this is an insane value
			}
			else if (hp == 0) {
				strcpy_safe(sbuf0, "2 Health: 0", SBAR_STRING_SIZE);
				hp = 1; // client won't show health text if this is 0
			}
			else {
				strcpy_safe(sbuf0, UTIL_VarArgs("2 Health: %lld", hp), SBAR_STRING_SIZE);
				hp = 1;
			}

			newSBarState[SBAR_ID_TARGETNAME] = entindex();
			newSBarState[SBAR_ID_TARGETHEALTH] = clampi(hp, INT16_MIN, INT16_MAX);

			lookingAtStatusEnt = true;
		}
		else if (pEntity->IsButton()) {
			const char* hint = pEntity->DisplayHint();

			if (hint[0]) {
				name = replaceString(pEntity->DisplayName(), "\n", " ");

				strcpy_safe(sbuf1, UTIL_VarArgs("1 %s (%s)", name.c_str(), hint), SBAR_STRING_SIZE);
				newSBarState[SBAR_ID_TARGETNAME] = ENTINDEX(pEntity->edict());

				lookingAtStatusEnt = true;
			}
		}
		else if (pEntity->IsBreakable() && !(pEntity->m_breakFlags & FL_BREAK_TRIGGER_ONLY)) {

			name = replaceString(pEntity->DisplayName(), "\n", " ");
			int hp = roundf(pEntity->pev->health);
			int irel = IRelationship(pEntity);

			const char* hint = "";
			if (irel == R_AL) {
				hint = " (wrench repairs)";
			}
			else if (pEntity->m_breakFlags & FL_BREAK_EXPLOSIVES_ONLY) {
				hint = " (explosives only)";
			}
			else if (pEntity->m_breakFlags & FL_BREAK_INSTANT) {
				CBreakable* breakable = (CBreakable*)pEntity;

				if (breakable->pev->health > 20) {
					if (breakable->m_breakWeapon == BREAK_INSTANT_WRENCH) {
						hint = " (wrench instant break)";
					}
					else {
						hint = " (crowbar instant break)";
					}
				}
			}

			if (irel == R_AL) {
				// use fake player info
				fakePlayerInfo.enabled = true;
				fakePlayerInfo.color = FRIEND_TEAM_COLOR;
				fakePlayerInfo.name = name.c_str();

				std::string desc = "";
				if (name.size() > 32) {
					// player names can only be 32 chars long
					desc = name.substr(31).c_str();
				}

				newSBarState[SBAR_ID_TARGETNAME] = entindex();
				strcpy_safe(sbuf1, UTIL_VarArgs("1 %%p1%s", desc.c_str()), SBAR_STRING_SIZE);
			}
			else {
				strcpy_safe(sbuf1, UTIL_VarArgs("1 %s%s", name.c_str(), hint), SBAR_STRING_SIZE);
				newSBarState[SBAR_ID_TARGETNAME] = ENTINDEX(pEntity->edict());
			}
				
			strcpy_safe(sbuf0, UTIL_VarArgs("2 Health: %d", hp), SBAR_STRING_SIZE);
			newSBarState[SBAR_ID_TARGETHEALTH] = clampi(hp, INT16_MIN, INT16_MAX);

			lookingAtStatusEnt = true;
		}
		else if (pEntity->IsPushable()) {

			const char* dname = "Pushable";
			if (pEntity->m_displayName) {
				dname = STRING(pEntity->m_displayName);
			}
			else if (pEntity->m_breakExplodeMag) {
				dname = "Pushable Explosives";
			}

			name = replaceString(dname, "\n", " ");

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

			lookingAtStatusEnt = true;
		}
		
		if (lookingAtStatusEnt) {
			m_flStatusBarDisappearDelay = gpGlobals->time + 1.0;
		}

		CALL_HOOKS_VOID(pfnClientStatusBar, pEntity, this, sbuf1, sbuf0, SBAR_STRING_SIZE);
	}
	
	if ( !lookingAtStatusEnt && m_flStatusBarDisappearDelay > gpGlobals->time )
	{
		// hold the values for a short amount of time after viewing the object
		newSBarState[ SBAR_ID_TARGETNAME ] = m_izSBarState[ SBAR_ID_TARGETNAME ];
		newSBarState[ SBAR_ID_TARGETHEALTH ] = m_izSBarState[ SBAR_ID_TARGETHEALTH ];
		newSBarState[ SBAR_ID_TARGETARMOR ] = m_izSBarState[ SBAR_ID_TARGETARMOR ];
	}

	BOOL bForceResend = FALSE;

	// second line of status bar, despite "0"
	if ( strncmp( sbuf0, m_SbarString0, SBAR_STRING_SIZE) )
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgStatusText, NULL, pev);
		WRITE_BYTE(0);
		WRITE_STRING(sbuf0);
		MESSAGE_END();

		strcpy_safe(m_SbarString0, sbuf0, SBAR_STRING_SIZE);

		// make sure everything's resent
		bForceResend = TRUE;
	}

	//if (fakePlayerInfo.enabled && (pev->button & IN_SCORE) || tempNameActive > 1) {
	if (tempNameActive) {
		tempNameActive++;
	}

	bool statusChanged = bForceResend;
	for (int i = 1; i < SBAR_END; i++) {
		if (newSBarState[i] != m_izSBarState[i]) {
			statusChanged = true;
			break;
		}
	}

	if ( strncmp( sbuf1, m_SbarString1, SBAR_STRING_SIZE) || (fakePlayerInfo.enabled && statusChanged))
	{
		if (fakePlayerInfo.enabled) {
			Rename(fakePlayerInfo.name, true, edict());
			UpdateTeamInfo(fakePlayerInfo.color, MSG_ONE, edict());

			tempNameActive = 1;
			memset(m_tempName, 0, SBAR_STRING_SIZE);
			strncpy(m_tempName, fakePlayerInfo.name, SBAR_STRING_SIZE);
			m_tempName[SBAR_STRING_SIZE - 1] = 0;
			m_tempTeam = fakePlayerInfo.color;
		}
		
		MESSAGE_BEGIN(MSG_ONE, gmsgStatusText, NULL, pev);
		WRITE_BYTE(1);
		WRITE_STRING(sbuf1);
		MESSAGE_END();

		strcpy_safe(m_SbarString1, sbuf1, SBAR_STRING_SIZE);

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

	if (tempNameActive >= 10) {
		// have a to wait a bit before resetting the player name and color because
		// the client doesn't parse the new status bar text until its next rendering frame.
		// With packet loss and lag, this message can arrive at the same time the status bar
		// was last updated, causing the player name to show instead of the npc name.
		tempNameActive = 0;
		Rename(STRING(pev->netname), false, edict());
		UpdateTeamInfo(-1, MSG_ONE, edict());
	}
}

void CBasePlayer::PreThink(void)
{
	CALL_HOOKS_VOID(pfnPlayerPreThink, this);

	int buttonsChanged = (m_afButtonLast ^ pev->button);	// These buttons have changed this frame
	
	// Debounced button codes for pressed/released
	// UNDONE: Do we need auto-repeat?
	m_afButtonPressed =  buttonsChanged & pev->button;		// The changed ones still down are "pressed"
	m_afButtonReleased = buttonsChanged & (~pev->button);	// The ones not down are "released"

	g_pGameRules->PlayerThink( this );

	if (m_initSoundTime && gpGlobals->time >= m_initSoundTime) {
		m_initSoundTime = 0;
		CAmbientGeneric::InitAllSoundsForNewJoiner(edict());
	}

	UpdateShockEffect();

	if ( g_fGameOver )
		return;         // intermission or finale

	UTIL_MakeVectors(pev->v_angle);             // is this still used?
	
	ItemPreFrame( );
	WaterMove();

	if ( g_pGameRules && g_pGameRules->FAllowFlashlight() && m_hasFlashlight )
		m_iHideHUD &= ~HIDEHUD_FLASHLIGHT;
	else
		m_iHideHUD |= HIDEHUD_FLASHLIGHT;

	if (HasSuit()) {
		// set every frame for lossy connections that don't initialize on join properly
		m_iHideHUD &= ~HIDEHUD_HEALTH_AND_ARMOR;
		m_fakeSuit = false;
	}
	else if (m_weaponBits) {
		// Tell client they have a suit if they don't but DO have weapons.
		// Otherwise they can't switch weapons
		m_fakeSuit = true;
		m_iHideHUD = HIDEHUD_FLASHLIGHT | HIDEHUD_HEALTH_AND_ARMOR;
	}

	// JOHN: checks if new client data (for HUD and view control) needs to be sent to the client
	UpdateClientData();
	
	CheckTimeBasedDamage();

	CheckSuitUpdate();

	// only show weapon hud if player has a weapon besides the suit
	if ((m_weaponBits & ~(1ULL << WEAPON_SUIT)) && !m_weaponsDisabled) {
		m_iHideHUD &= ~HIDEHUD_WEAPONS;
	}
	else {
		m_iHideHUD |= HIDEHUD_WEAPONS;
	}

	// Observer Button Handling
	if ( IsObserver() )
	{
		Observer_HandleButtons();
		Observer_CheckTarget();
		Observer_CheckProperties();
		pev->impulse = 0;

		static float lastCheck = 0;

		if (m_wantToExitObserver && (lastCheck > gpGlobals->time || gpGlobals->time - lastCheck > 1.0f)) {
			lastCheck = gpGlobals->time;
			edict_t* pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot(this);

			if (!FNullEnt(pentSpawnSpot)) {
				LeaveObserver();
				m_wantToExitObserver = false;
			}
			else {
				UTIL_ClientPrint(this, print_center, "Waiting to spawn...\n");
			}
		}

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
				if (pTrain && pTrain->Classify() == CLASS_VEHICLE)
					((CFuncVehicle*)pTrain)->m_pDriver = NULL;
				return;
			}
		}
		else if (!FBitSet(pev->flags, FL_ONGROUND) || FBitSet(pTrain->pev->spawnflags, SF_TRACKTRAIN_NOCONTROL) || ((pev->button & (IN_MOVELEFT | IN_MOVERIGHT)) && pTrain->Classify() != CLASS_VEHICLE))
		{
			// Turn off the train if you jump, strafe, or the train controls go dead
			// and it isn't a func_vehicle.
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			m_iTrain = TRAIN_NEW|TRAIN_OFF;
			if (pTrain->Classify() == CLASS_VEHICLE)
				((CFuncVehicle*)pTrain)->m_pDriver = NULL;
			return;
		}

		pev->velocity = g_vecZero;
		vel = 0;
		if (pTrain->Classify() == CLASS_VEHICLE)
		{
			if (pev->button & IN_FORWARD)
			{
				vel = 1;
				pTrain->Use(this, this, USE_SET, (float)vel);
			}
			if (pev->button & IN_BACK)
			{
				vel = -1;
				pTrain->Use(this, this, USE_SET, (float)vel);
			}
			if (pev->button & IN_MOVELEFT)
			{
				vel = 20;
				pTrain->Use(this, this, USE_SET, (float)vel);
			}
			if (pev->button & IN_MOVERIGHT)
			{
				vel = 30;
				pTrain->Use(this, this, USE_SET, (float)vel);
			}
		}
		else
		{
			if (m_afButtonPressed & IN_FORWARD)
			{
				vel = 1;
				pTrain->Use(this, this, USE_SET, (float)vel);
			}
			else if (m_afButtonPressed & IN_BACK)
			{
				vel = -1;
				pTrain->Use(this, this, USE_SET, (float)vel);
			}
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
	entvars_t* attackerPev = pev;

	if (m_lastDamageEnt) {
		attackerPev = &m_lastDamageEnt.GetEdict()->v;
	}

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
			{
				int oldTime = m_rgbTimeBasedDamage[i];
				TakeDamage(attackerPev, attackerPev, POISON_DAMAGE, DMG_POISON);
				m_rgbTimeBasedDamage[i] = oldTime; // don't reset damage timer
				bDuration = POISON_DURATION;
				break;
			}
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
	if ( !HasSuit() )
		return;

	// if in range of radiation source, ping geiger counter
	UpdateGeigerCounter();

	if (!mp_hevsuit_voice.value)
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

bool IsBatteryUpdateSuitSentence(const char* name) {
	int len = strlen(name);
	return len >= 7 && len <= 8 && strstr(name, "!HEV_") == name
		&& isdigit(name[5]) && ((isdigit(name[6]) && name[7] == 'P') || name[6] == 'P');
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
	
	if (!mp_hevsuit_voice.value)
	{
		// due to static channel design, etc. We don't play HEV sounds in multiplayer right now.
		return;
	}

	// if name == NULL, then clear out the queue
	if (!name) {
		for (i = 0; i < CSUITPLAYLIST; i++)
			m_rgSuitPlayList[i] = 0;
		return;
	}
	
	// Ignore suit updates if no suit
	if ( !HasSuit() )
		return;
	
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

	bool replacedExisting = false;
	if (IsBatteryUpdateSuitSentence(name)) {
		for (int k = 0; k < CSUITPLAYLIST; k++) {
			if (!m_rgSuitPlayList[k]) {
				continue;
			}

			char sentence[CBSENTENCENAME_MAX + 1];
			strcpy_safe(sentence, "!", CBSENTENCENAME_MAX + 1);
			strcat_safe(sentence, gszallsentencenames[m_rgSuitPlayList[k]], CBSENTENCENAME_MAX + 1);

			if (IsBatteryUpdateSuitSentence(sentence)) {
				m_rgSuitPlayList[k] = isentence;
				replacedExisting = true;
				break;
			}
		}
	}

	if (!replacedExisting) {
		// find empty spot in queue, or overwrite last spot
		m_rgSuitPlayList[m_iSuitPlayNext++] = isentence;
		if (m_iSuitPlayNext == CSUITPLAYLIST)
			m_iSuitPlayNext = 0;
	}

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
	CALL_HOOKS_VOID(pfnPlayerPostThink, this);

	if ( g_fGameOver )
		goto pt_end;         // intermission or finale

	UpdateTag();
	UpdateTagPos();

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

	ImpulseCommands();

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

		if (pev->watertype == CONTENTS_WATER)
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
			TRACE_HULL(pev->origin, pev->origin + Vector(0,0,-8), dont_ignore_monsters, hullType, edict(), &tr);

			// split fall damage with self and whatever ent was landed on
			CBaseMonster* ent = CBaseEntity::Instance(tr.pHit)->MyMonsterPointer();
			if (ent) {
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
					ent->m_lastDamageType = DMG_FALL;
					g_pGameRules->DeathNotice(ent, pev, pev);
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

	NightvisionUpdate();

	DebugThink();

	SyncWeaponBits();

pt_end:
#if defined( CLIENT_WEAPONS )
		// Decay timers on weapons
	// go through all of the weapons and make a list of the ones to pack
	for ( int i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( m_rgpPlayerItems[ i ] )
		{
			CBaseEntity *pPlayerItem = m_rgpPlayerItems[i].GetEntity();

			while ( pPlayerItem )
			{
				CBasePlayerWeapon *gun = pPlayerItem->GetWeaponPtr();
				
				if ( gun && gun->UseDecrement() )
				{
					gun->m_flNextPrimaryAttack		= V_max( gun->m_flNextPrimaryAttack - gpGlobals->frametime, -1.0f );
					gun->m_flNextSecondaryAttack	= V_max( gun->m_flNextSecondaryAttack - gpGlobals->frametime, -0.001f );
					gun->m_flNextTertiaryAttack		= V_max( gun->m_flNextTertiaryAttack - gpGlobals->frametime, -0.001f );

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

				pPlayerItem = gun ? gun->m_pNext.GetEntity() : NULL;
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

	// reset friction if not touching any friction triggers
	if (m_friction_modifier != 1.0f && (gpGlobals->time - m_last_friction_trigger_touch > 0.2f)) {
		m_friction_modifier = 1.0f;
		ApplyEffects();
	}

	// Track button info so we can detect 'pressed' and 'released' buttons next frame
	m_afButtonLast = pev->button;
	pev->oldorigin = pev->origin; // for func_clip
}

void CBasePlayer::Spawn( void )
{
	DropAllInventoryItems(false, true);

	// not hiding HUD elements because the client can end up with no HUD after spawn finishes
	RemoveAllItems(TRUE, TRUE);

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
	pev->flags		   &= (FL_PROXY | FL_FAKECLIENT);	// keep proxy flag sey by engine
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
	m_lastDamageEnt = NULL;
	m_lastDamageType = 0;
	m_waterFriction = 1.0f;
	m_buoyancy = 0.0f;
	ResetEffects();
	m_droppedDeathWeapons = false;
	if (m_flashlightEnabled && flashlight.value >= 2) {
		UTIL_ScreenFade(this, g_vecZero, 0, 0, 0, 0, true); // remove nightvision fade
	}
	m_flashlightEnabled = false;
	CBasePlayerWeapon::ResetPickupLimits(this);
	memset(m_lastHurtTriggers, 0, sizeof(m_lastHurtTriggers));

	if( pev->iuser1 != OBS_NONE )
		LeaveObserver();

	m_bitsHUDDamage		= -1;
	m_bitsDamageType	= 0;
	m_afPhysicsFlags	= 0;
	m_fLongJump			= FALSE;// no longjump module. 

	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "slj", "0" );
	g_engfuncs.pfnSetPhysicsKeyValue( edict(), "hl", "1" );
	SetJumpPower(0);

	pev->fov = m_iFOV				= 0;// init field of view.
	m_iClientFOV		= -1; // make sure fov reset is sent

	m_flNextDecalTime	= 0;// let this player decal as soon as he spawns.

	m_flgeigerDelay = gpGlobals->time + 2.0;	// wait a few seconds until user-defined message registrations
												// are recieved by all clients
	
	m_flTimeStepSound	= 0;
	m_iStepLeft = 0;
	m_flFieldOfView		= 0.5;// some monsters use this to determine whether or not the player is looking at them.

	m_bloodColor	= BloodColorHuman();
	m_flNextAttack	= UTIL_WeaponTimeBase();
	StartSneaking();

	m_flFlashLightTime = 1; // force first message
	m_iFlashBattery = V_min(mp_startflashlight.value, 100) - 1;
	m_flFlashLightCarry = 0;
	if (mp_startflashlight.value < 0) {
		// don't start with flashlight, but still set a default battery level
		m_iFlashBattery = V_min(-(m_iFlashBattery + 1), 100) - 1;
	}
	

// dont let uninitialized value here hurt the player
	m_flFallVelocity = 0;
	m_deathMessageSent = false;

	SetThirdPersonWeaponAnim(0);

	g_pGameRules->SetDefaultPlayerTeam( this );
	edict_t* pentSpawnSpot = g_pGameRules->GetPlayerSpawnSpot( this );

	pev->view_ofs = VEC_VIEW;

	if (!FNullEnt(pentSpawnSpot)) {
		CBaseDMStart* spawn = (CBaseDMStart*)CBaseEntity::Instance(pentSpawnSpot);
		spawn->SpawnPlayer(this);
	}
	else {
		pev->origin = Vector(0, 0, 1);
		pev->v_angle = g_vecZero;
		pev->velocity = g_vecZero;
		pev->angles = g_vecZero;
		pev->punchangle = g_vecZero;
		pev->fixangle = TRUE;
	}

    SET_MODEL(ENT(pev), "models/player.mdl");
    g_ulModelIndexPlayer = pev->modelindex;
	pev->sequence		= LookupActivity( ACT_IDLE );

	if ( FBitSet(pev->flags, FL_DUCKING) ) 
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

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
	m_hasFlashlight = mp_startflashlight.value >= 0;

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
	UpdateTeamInfo();

	// view can be set to null(?) if the level changes while a camera is active
	// which means you won't see any entities in the next level
	if (!m_hActiveCamera) {
		SET_VIEW(edict(), edict());
	}
	
	if (m_clientSystem != CLIENT_SYSTEM_NOT_CHECKED)
		g_pGameRules->PlayerSpawn(this);

	if (FNullEnt(pentSpawnSpot)) {
		edict_t* anySpawnPoint = EntSelectSpawnPoint(this, true);
		StartObserver(anySpawnPoint->v.origin, anySpawnPoint->v.angles);
		m_wantToExitObserver = true;
	}

	ApplyEffects();

	// don't play suit sounds for items given when spawning
	SetSuitUpdate(NULL, FALSE, 0);

	// for when mic audio breaks due to teleports or something else I don't understand yet
	UTIL_ResetVoiceChannel(this);

	CALL_HOOKS_VOID(pfnPlayerSpawn, this);
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
	m_bloodColor	= BloodColorHuman();

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
	CBaseEntity* pItemEnt = m_rgpPlayerItems[iItem].GetEntity();
	CBaseEntity* activeItemEnt = m_pActiveItem.GetEntity();
	CBasePlayerWeapon* pItem = pItemEnt ? pItemEnt->GetWeaponPtr() : NULL;
	CBasePlayerWeapon* activeItem = activeItemEnt ? activeItemEnt->GetWeaponPtr() : NULL;

	if (!pItem)
		return;

	if (pItem == activeItem)
	{
		// select the next one in the chain
		CBaseEntity* nextWep = activeItem->m_pNext.GetEntity();
		pItem = nextWep ? nextWep->GetWeaponPtr() : NULL;
		if (! pItem)
		{
			return;
		}

		CBasePlayerWeapon* pLast = pItem;
		while (pLast->m_pNext) {
			CBaseEntity* pLastEnt = pLast->m_pNext.GetEntity();
			pLast = pLastEnt ? pLastEnt->GetWeaponPtr() : NULL;
		}

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

	if (pItem)
	{
		pItem->Deploy( );
		pItem->UpdateItemInfo( );
	}
}

void CBasePlayer::SelectItem(const char *pstr)
{
	if (!pstr)
		return;

	if (m_weaponsDisabled) {
		return;
	}

	CWeaponCustom* wc = m_pActiveItem ? m_pActiveItem->MyWeaponCustomPtr() : NULL;
	if (wc && wc->IsExclusiveHold()) {
		UTIL_ClientPrint(this, print_center, "Drop this weapon to select another.");
		return;
	}

	CBasePlayerItem *pItem = NULL;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			CBaseEntity* itemEnt = m_rgpPlayerItems[i].GetEntity();
			pItem = itemEnt ? itemEnt->GetWeaponPtr() : NULL;
	
			while (pItem)
			{
				if (FClassnameIs(pItem->pev, pstr))
					break;

				CBaseEntity* next = pItem->m_pNext.GetEntity();
				pItem = next ? next->GetWeaponPtr() : NULL;
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

	if (pItem)
	{
		pItem->Deploy( );
		pItem->UpdateItemInfo( );
	}
}

void CBasePlayer::SelectLastItem(void)
{
	if (!m_pLastItem)
	{
		return;
	}

	if (m_weaponsDisabled) {
		return;
	}

	CWeaponCustom* wc = m_pActiveItem ? m_pActiveItem->MyWeaponCustomPtr() : NULL;
	if (wc && wc->IsExclusiveHold()) {
		UTIL_ClientPrint(this, print_center, "Drop this weapon to select another.");
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

	DispatchSpawnGame( pent );
	CBaseEntity* pEntity = CBaseEntity::Instance(pent); // may have been relocated by spawn code
	
	CBasePlayerWeapon* wep = pEntity->GetWeaponPtr();
	CBasePlayerAmmo* ammo = pEntity->MyAmmoPtr();
	CItem* item = pEntity->MyItemPointer();

	// call add functions directly in case the item is use-only
	if (wep) {
		wep->m_isDroppedWeapon = true;
		wep->DefaultTouch(this);
		if (wep->pev->movetype != MOVETYPE_FOLLOW)
			UTIL_Remove(pEntity); // wasn't collected
	}
	else if (ammo) {
		ammo->DefaultUse(this, this, USE_TOGGLE, 0);
		UTIL_Remove(pEntity);
	}
	else if (item) {
		item->ItemUse(this, this, USE_TOGGLE, 0);
		UTIL_Remove(pEntity);
	}
	else {
		pEntity->Touch(this);
		UTIL_Remove(pEntity);
	}
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
	return m_flashlightEnabled;
}

void CBasePlayer :: FlashlightTurnOn( void )
{
	if ( !g_pGameRules->FAllowFlashlight() || !m_hasFlashlight)
	{
		return;
	}

	if (mp_flashlight_charge.value == 0 && m_iFlashBattery <= 0) {
		return;
	}

	CALL_HOOKS_VOID(pfnPlayerFlashlightToggle, this, true);

	if (HasSuit())
	{
		m_flashlightEnabled = true;

		if (flashlight.value == 1) {
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, SOUND_FLASHLIGHT_ON, 1.0, ATTN_NORM, 0, PITCH_NORM);
			SetBits(pev->effects, EF_DIMLIGHT);
		}
		else {
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, SOUND_NIGHTVISION_ON, 1.0, ATTN_NORM, 0, PITCH_NORM);
			UTIL_ScreenFade(this, m_nightvisionColor.ToVector(), 0.1f, 5.0f, 255, FFADE_MODULATE | FFADE_OUT);
			
			// give some time to fade in
			m_lastNightvisionUpdate = m_lastNightvisionFadeUpdate = g_engfuncs.pfnTime();
		}
		
		MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
		WRITE_BYTE(1);
		WRITE_BYTE(m_iFlashBattery);
		MESSAGE_END();

		if (mp_flashlight_drain.value) {
			m_flFlashLightTime = (FLASH_DRAIN_TIME / mp_flashlight_drain.value) + gpGlobals->time - m_flFlashLightCarry;
		}
	}
}

void CBasePlayer :: FlashlightTurnOff( void )
{
	CALL_HOOKS_VOID(pfnPlayerFlashlightToggle, this, false);

	m_flashlightEnabled = false;

	if (flashlight.value == 1) {
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, SOUND_FLASHLIGHT_OFF, 1.0, ATTN_NORM, 0, PITCH_NORM);
		ClearBits(pev->effects, EF_DIMLIGHT);
	}
	else {
		EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, SOUND_NIGHTVISION_OFF, 1.0, ATTN_NORM, 0, PITCH_NORM);
		UTIL_ScreenFade(this, m_nightvisionColor.ToVector(), 0.1f, 0.15f, 255, FFADE_MODULATE | FFADE_IN);
	}

	MESSAGE_BEGIN( MSG_ONE, gmsgFlashlight, NULL, pev );
	WRITE_BYTE(0);
	WRITE_BYTE(m_iFlashBattery);
	MESSAGE_END();

	// remember the time spent so it can be subtracted next time the flashlight turns on. This
	// prevents infinite battery when quickly toggling once per second (or longer for low drain speeds)
	if (mp_flashlight_drain.value) {
		float drainDelay = (FLASH_DRAIN_TIME / mp_flashlight_drain.value);
		m_flFlashLightCarry = drainDelay - (m_flFlashLightTime - gpGlobals->time);
	}

	if (mp_flashlight_charge.value) {
		m_flFlashLightTime = (FLASH_CHARGE_TIME / mp_flashlight_charge.value) + gpGlobals->time;
	}
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

	bool tertiaryPressed = false;
		
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
	case 222:
		// tertiary attack
		// An impulse is used because buttons can only have 16 bits and all are used
		tertiaryPressed = true;
		break;

	default:
		// check all of the cheat impulse commands now
		CheatImpulseCommands( iImpulse );
		break;
	}

	if (tertiaryPressed) {
		pev->button |= IN_ATTACK3;
		m_afButtonPressed |= IN_ATTACK3;
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
		GiveNamedItem( "weapon_shotgun" );
		GiveNamedItem( "ammo_buckshot" );
		GiveNamedItem( "weapon_9mmAR" );
		GiveNamedItem( "ammo_9mmbox" );
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
		GiveNamedItem( "weapon_medkit" );
		if (!m_fLongJump) {
			GiveNamedItem("item_longjump");
		}
		if (IsSevenKewpClient()) {
			GiveNamedItem("weapon_m249");
			GiveNamedItem("weapon_sniperrifle");
			GiveNamedItem("weapon_uziakimbo");
			GiveNamedItem("weapon_eagle");
			GiveNamedItem("ammo_556");
			GiveNamedItem("ammo_762");
		}
		gEvilImpulse101 = FALSE;
		break;

	case 102:
		RETURN_IF_CHEATS_DISABLED()
		// Gibbage!!!
		CGib::SpawnMonsterGibs( pev, 1, 1 );
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
	const char* altName = "";

	if (IsSevenKewpClient()) {
		const char* remap = g_weaponRemapHL.get(STRING(pItem->pev->classname));
		if (remap)
			altName = remap;
	}
	else {
		CWeaponCustom* cwep = pItem ? pItem->MyWeaponCustomPtr() : NULL;
		if (cwep && cwep->wrongClientWeapon)
			altName = cwep->wrongClientWeapon;
	}

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		CBaseEntity* pInsertEnt = m_rgpPlayerItems[i].GetEntity();
		CBasePlayerItem* pInsert = pInsertEnt ? pInsertEnt->GetWeaponPtr() : NULL;

		while (pInsert) {

			if (FClassnameIs(pInsert->pev, STRING(pItem->pev->classname)) || FClassnameIs(pInsert->pev, altName))
			{
				if (pItem->AddDuplicate(pInsert))
				{
					g_pGameRules->PlayerGotWeapon(this, pItem);
					pItem->m_pickupPlayers |= PLRBIT(edict());
					pItem->CheckRespawn();

					// ugly hack to update clip w/o an update clip message
					pInsert->UpdateItemInfo();
					if (m_pActiveItem) {
						CBasePlayerWeapon* activeWep = m_pActiveItem.GetEntity()->GetWeaponPtr();
						if (activeWep)
							activeWep->UpdateItemInfo();
					}

					pItem->Kill();
				}
				else if (gEvilImpulse101)
				{
					// FIXME: remove anyway for deathmatch testing
					pItem->Kill();
				}
				return FALSE;
			}

			CBaseEntity* next = pInsert->m_pNext.GetEntity();
			pInsert = next ? next->GetWeaponPtr() : NULL;
		}
	}


	if (pItem->AddToPlayer( this ))
	{
		pItem->m_pickupPlayers |= PLRBIT(edict()); // add new flag to the respawning weapon
		pItem->m_isDroppedWeapon = true;
		g_pGameRules->PlayerGotWeapon ( this, pItem );
		pItem->CheckRespawn();

		pItem->m_pNext = m_rgpPlayerItems[pItem->iItemSlot()].GetEntity();
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem;

		// should we switch to this item?
		if ( g_pGameRules->FShouldSwitchWeapon( this, pItem ) )
		{
			SwitchWeapon( pItem );
		}

		ResolveWeaponSlotConflict(pItem->m_iId);

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
	if (!pItem) {
		return FALSE;
	}

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

	m_weaponBits &= ~(1ULL << pItem->m_iId); // take item off hud

	CBaseEntity* pPrevEnt = m_rgpPlayerItems[pItem->iItemSlot()].GetEntity();
	CBasePlayerItem *pPrev = pPrevEnt ? pPrevEnt->GetWeaponPtr() : NULL;

	if (pPrev == pItem)
	{
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem->m_pNext.GetEntity();
		return TRUE;
	}
	else
	{
		while (pPrev && pPrev->m_pNext.GetEntity() != pItem)
		{
			pPrevEnt = pPrev->m_pNext.GetEntity();
			pPrev = pPrevEnt ? pPrevEnt->GetWeaponPtr() : NULL;
		}
		if (pPrev)
		{
			pPrev->m_pNext = pItem->m_pNext.GetEntity();
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

	if (!m_pActiveItem) {
		m_szAnimExtention[0] = 0; // use no-weapons animation set
		return;
	}

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

			if (IsSevenKewpClient()) {
				uint16_t ammoVal = V_max(0, m_rgAmmo[i]);

				// can't update max ammo counter without a client update
				// this will at least show the client that ammo is being spent
				if (m_rgAmmo[i] > 999) {
					ammoVal = 990 + (m_rgAmmo[i] % 10);
				}

				MESSAGE_BEGIN(MSG_ONE, gmsgAmmoXX, NULL, pev);
				WRITE_BYTE(i);
				WRITE_SHORT(ammoVal);
				MESSAGE_END();
			}
			else {
				uint8_t ammoVal = V_max(0, m_rgAmmo[i]);

				// can't update max ammo counter without a client update
				// this will at least show the client that ammo is being spent
				if (m_rgAmmo[i] > 255) {
					ammoVal = 250 + (m_rgAmmo[i] % 6);
				}

				MESSAGE_BEGIN(MSG_ONE, gmsgAmmoX, NULL, pev);
				WRITE_BYTE(i);
				WRITE_BYTE(ammoVal);
				MESSAGE_END();
			}
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
	if (!m_clientCheckFinished) 
		return; // need to know client type before sending client-specific HUD messages

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
			UpdateTeamInfo();
		}

		if ( !m_fGameHUDInitialized )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgInitHUD, NULL, pev );
			MESSAGE_END();

			g_pGameRules->InitHUD( this );
			m_fGameHUDInitialized = TRUE;
			
			m_iObserverLastMode = OBS_ROAMING;

			if (g_pGameRules->IsMultiplayer()) {
				FireTargets("game_playerjoin", this, this, USE_TOGGLE, 0);
			}

			CALL_HOOKS_VOID(pfnClientJoin, this);
		}

		FireTargets( "game_playerspawn", this, this, USE_TOGGLE, 0 );

		InitStatusBar();

		if (m_fLongJump) {
			MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pev);
			WRITE_STRING("item_longjump");
			MESSAGE_END();

			if (HasSuit())
				EMIT_SOUND_SUIT(edict(), "!HEV_A1");
		}
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

	if ((int)pev->health != m_iClientHealth)
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


	if ((int)pev->armorvalue != m_iClientBattery)
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
			if (m_iFlashBattery && mp_flashlight_drain.value != 0)
			{
				m_flFlashLightTime = (FLASH_DRAIN_TIME / mp_flashlight_drain.value) + gpGlobals->time;
				m_iFlashBattery--;
				m_flFlashLightCarry = 0;

				if (m_iFlashBattery < 8) {
					SetSuitUpdate("!HEV_0P", FALSE, SUIT_NEXT_IN_1MIN);
				}
			}
			
			if (!m_iFlashBattery)
				FlashlightTurnOff();
		}
		else if (mp_flashlight_charge.value != 0)
		{
			if (m_iFlashBattery < 100)
			{
				m_flFlashLightTime = (FLASH_CHARGE_TIME / mp_flashlight_charge.value) + gpGlobals->time;
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
		
		// Send ALL the weapon info now
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			ItemInfo* II = &CBasePlayerItem::ItemInfoArray[i];

			int conflictId = GetCurrentIdForConflictedSlot(i);
			if (conflictId != -1 && conflictId != i) {
				// all weapon infos that share a slot must point to currently held weapon in the shared slot
				// or else the client won't render the menu correctly
				II = &CBasePlayerItem::ItemInfoArray[conflictId];
			}

			if ( !II->iId )
				continue;

			if (!IsSevenKewpClient() && II->iId >= 32)
				continue; // crash on AG if sending too high of an ID

			MESSAGE_BEGIN(MSG_ONE, gmsgWeaponList, NULL, pev);
			WRITE_STRING(II->pszName);			// string	weapon name
			WRITE_BYTE(GetAmmoIndex(II->pszAmmo1));	// byte		Ammo Type
			WRITE_BYTE(II->iMaxAmmo1);				// byte     Max Ammo 1
			WRITE_BYTE(GetAmmoIndex(II->pszAmmo2));	// byte		Ammo2 Type
			WRITE_BYTE(II->iMaxAmmo2);				// byte     Max Ammo 2
			WRITE_BYTE(II->iSlot);					// byte		bucket
			WRITE_BYTE(II->iPosition);				// byte		bucket pos
			WRITE_BYTE(i);							// byte		id (bit index into pev->weapons)
			WRITE_BYTE(II->iFlags);					// byte		Flags
			MESSAGE_END();

			if (IsSevenKewpClient()) {
				MESSAGE_BEGIN(MSG_ONE, gmsgWeaponListX, NULL, pev);
				WRITE_BYTE(i);
				WRITE_BYTE(II->iFlagsEx);
				WRITE_SHORT(V_min(65535, II->fAccuracyDeg * 100));
				WRITE_SHORT(V_min(65535, II->fAccuracyDeg2 * 100));
				WRITE_SHORT(V_min(65535, II->fAccuracyDegY * 100));
				WRITE_SHORT(V_min(65535, II->fAccuracyDegY2 * 100));
				MESSAGE_END();

				// send custom sprite dir for weapons that player spawned with
				// (this is sent on pickup too, but comes too early for new joiners)
				const char* cname = CBasePlayerWeapon::GetClassFromInfoName(II->pszName);
				const char* customSpriteDir = g_defaultSpriteDirs.get(cname);
				if (customSpriteDir && GetNamedPlayerItem(cname)) {
					MESSAGE_BEGIN(MSG_ONE, gmsgCustomHud, NULL, pev);
					WRITE_BYTE(i);
					WRITE_STRING(customSpriteDir ? customSpriteDir : "");
					MESSAGE_END();
				}
			}
		}
	
		FixSharedWeaponSlotClipCount();
	}

	SendAmmoUpdate();

	// Update all the items
	for ( int i = 0; i < MAX_ITEM_TYPES; i++ )
	{
		CBaseEntity* ent = m_rgpPlayerItems[i].GetEntity();
		CBasePlayerItem* item = ent ? ent->GetWeaponPtr() : NULL;
		if (item)  // each item updates it's successors
			item->UpdateClientData(this);
	}

	// Cache and client weapon change
	m_pClientActiveItem = m_pActiveItem.GetEntity();
	m_iClientFOV = m_iFOV;

	// Update Status Bar
	if ( m_flNextSBarUpdateTime < gpGlobals->time && !IsBot() )
	{
		lagcomp_begin(this);
		UpdateStatusBar();
		lagcomp_end();
		m_flNextSBarUpdateTime = gpGlobals->time + 0.1f;
	}
}

void CBasePlayer::Rename(const char* newName, bool fast, edict_t* dst) {
	char* info = g_engfuncs.pfnGetInfoKeyBuffer(edict());

	if (!info || info[0] == '\0') {
		return;
	}

	// not doing this because it triggers the "changed name" chat message
	//g_engfuncs.pfnSetClientKeyValue(entindex(), info, "name", (char*)newName);

	static char userinfo[512];

	if (fast) {
		// only send the essential values for rendering
		// TODO: are the other values even used by the client? or only the server?
		char* model = g_engfuncs.pfnInfoKeyValue(info, "model");
		char* topcolor = g_engfuncs.pfnInfoKeyValue(info, "topcolor");
		char* botcolor = g_engfuncs.pfnInfoKeyValue(info, "bottomcolor");

		if (strlen(model) + strlen(topcolor) + strlen(botcolor) + strlen(newName) + 40 >= 512) {
			ALERT(at_error, "Can't rename player. Userinfo too long\n"); // shouldn't ever happen
			return;
		}

		memset(userinfo, 0, 512);
		strcat(userinfo, "\\name\\");
		strcat(userinfo, newName);
		strcat(userinfo, "\\model\\");
		strcat(userinfo, model);
		strcat(userinfo, "\\topcolor\\");
		strcat(userinfo, topcolor);
		strcat(userinfo, "\\bottomcolor\\");
		strcat(userinfo, botcolor);

		if (dst) {
			UTIL_SendUserInfo_hooked(dst, edict(), userinfo);
		}
		else {
			for (int i = 1; i < gpGlobals->maxClients; i++) {
				CBasePlayer* msgPlr = UTIL_PlayerByIndex(i);
				if (!msgPlr)
					continue;

				UTIL_SendUserInfo_hooked(msgPlr->edict(), edict(), userinfo);
			}
		}

		return;
	}

	char* nameStart = strstr(info, "\\name\\") + 6;
	if (!nameStart) {
		return;
	}

	char* nameEnd = strstr(nameStart, "\\");

	if (strlen(info) + strlen(newName) >= 512 && nameStart) {
		return;
	}

	strncpy(userinfo, info, nameStart - info);
	int offset = nameStart - info;

	strcpy(userinfo + offset, newName);
	offset += strlen(newName);

	if (nameEnd)
		strcpy(userinfo + offset, nameEnd);

	if (dst) {
		UTIL_SendUserInfo_hooked(dst, edict(), userinfo);
	}
	else {
		for (int i = 1; i < gpGlobals->maxClients; i++) {
			CBasePlayer* msgPlr = UTIL_PlayerByIndex(i);
			if (!msgPlr)
				continue;

			UTIL_SendUserInfo_hooked(msgPlr->edict(), edict(), userinfo);
		}
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
BOOL CBasePlayer ::BarnacleVictimCaught( void )
{
	m_afPhysicsFlags |= PFLAG_ONBARNACLE;
	m_isBarnacleFood = false;
	SetAnimation(PLAYER_BARNACLE_HIT);
	return TRUE;
}

//=========================================================
// BarnacleVictimBitten - bad name for a function that is called
// by Barnacle victims when the barnacle pulls their head
// into its mouth. For the player, just die.
//=========================================================
void CBasePlayer :: BarnacleVictimBitten ( entvars_t *pevBarnacle )
{
	m_isBarnacleFood = true;
	TakeDamage ( pevBarnacle, pevBarnacle, gSkillData.sk_barnacle_dmg_bite, DMG_SLASH | DMG_ALWAYSGIB );
	SetAnimation(PLAYER_BARNACLE_CRUNCH);
}

//=========================================================
// BarnacleVictimReleased - overridden for player who has
// physics flags concerns. 
//=========================================================
void CBasePlayer :: BarnacleVictimReleased ( void )
{
	m_isBarnacleFood = false;
	m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
	
	if (pev->movetype == MOVETYPE_FLY) {
		pev->movetype = MOVETYPE_WALK; // needed when released inside a wall
	}
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

void CBasePlayer::DisableWeapons(bool disable) {
	bool changed = m_weaponsDisabled != disable;
	m_weaponsDisabled = disable;

	if (!changed) {
		return;
	}

	if (disable) {
		if (HasNamedPlayerItem("weapon_inventory")) {
			SelectItem("weapon_inventory");
		}
		else if (m_pActiveItem) {
			CBasePlayerWeapon* wep = m_pActiveItem.GetEntity()->GetWeaponPtr();
			wep->Holster();
			m_pLastItem = m_pActiveItem;
			m_pActiveItem = NULL;
		}
	}
	else {
		CBasePlayerWeapon* wep = m_pLastItem ? m_pLastItem.GetEntity()->GetWeaponPtr() : NULL;
		if (wep) {
			SwitchWeapon(wep);
		}
		else {
			g_pGameRules->GetNextBestWeapon(this, NULL);
		}
	}
}

//=========================================================
// Autoaim
// set crosshair position to point to enemey
//=========================================================
Vector CBasePlayer :: GetAutoaimVector( float flDelta )
{
	//if (g_iSkillLevel == SKILL_HARD)
	{
		return GetLookDirection();
	}
	/*

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
	*/
}

Vector CBasePlayer::GetLookDirection() {
	MAKE_VECTORS(pev->v_angle + pev->punchangle);
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
	CBaseEntity* ent = NULL;
	CBaseEntity* oldestBox = NULL;
	float oldestTime = FLT_MAX;
	int totalBoxes = 0;
	int thisEntIdx = ENTINDEX(edict());
	while ((ent = UTIL_FindEntityByClassname(ent, "weaponbox"))) {
		CWeaponBox* oldbox = (CWeaponBox*)ent;
		if (oldbox && ent->pev->owner && ENTINDEX(ent->pev->owner) == thisEntIdx) {
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
void CBasePlayer::DropPlayerItem ( const char *pszItemName )
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

	for ( int i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		CBaseEntity* ent = m_rgpPlayerItems[i].GetEntity();
		CBasePlayerItem* pWeapon = ent ? ent->GetWeaponPtr() : NULL;

		while ( pWeapon )
		{
			ItemInfo info;
			pWeapon->GetItemInfo(&info);

			if ( pszItemName )
			{
				// try to match by classname or name. 
				if ( !strcmp( pszItemName, STRING( pWeapon->pev->classname ) ) || !strcmp(pszItemName, info.pszName))
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

			CBaseEntity* next = pWeapon->m_pNext.GetEntity();
			pWeapon = next ? next->GetWeaponPtr() : NULL;
		}

		
		// if we land here with a valid pWeapon pointer, that's because we found the 
		// item we want to drop and hit a BREAK;  pWeapon is the item.
		if ( pWeapon )
		{
			if (!strcmp(STRING(pWeapon->pev->classname), "weapon_inventory")) {
				DropAllInventoryItems();
				return;
			}

			if (!pWeapon->CanHolster())
				return; // can't drop the item they asked for, may be something we can't holster

			m_lastDropTime = gpGlobals->time;
			UTIL_MakeVectors ( pev->v_angle ); 
			SetAnimation(PLAYER_DROP_ITEM);
			SetJumpPower(0);

			CWeaponCustom* cwep = pWeapon->MyWeaponCustomPtr();
			if (cwep && cwep->CanAkimbo()) {
				// only drop the left weapon
				Vector angles(0, pev->angles.y, 0);
				CBaseEntity* pOneWep = CBaseEntity::Create(STRING(pWeapon->pev->classname),
					pev->origin + gpGlobals->v_forward * 10, angles, true, edict());
				pOneWep->pev->velocity = gpGlobals->v_forward * 400;
				CWeaponCustom* cOneWep = pOneWep->MyWeaponCustomPtr();
				cOneWep->m_iClip = cwep->GetAkimboClip();
				cOneWep->m_iDefaultAmmo = 0;
				cwep->SetAkimboClip(0);
				cwep->SetCanAkimbo(false);
				cwep->SetAkimbo(false);
				pWeapon = cOneWep;
			}
			else {
				m_weaponBits &= ~(1ULL << pWeapon->m_iId);// take item off hud
				g_pGameRules->GetNextBestWeapon(this, pWeapon);
			}

			pWeapon->m_isDroppedWeapon = true;

			if (!strcmp(STRING(pWeapon->pev->classname), "weapon_shockrifle")) {
				// fixme: logic duplicated in kill code
				if (RemovePlayerItem(pWeapon)) {
					static StringMap keys = { {"is_player_ally", "1"} };
					Vector angles(0, pev->angles.y, 0);
					CBaseEntity* pRoach = CBaseEntity::Create("monster_shockroach",
						pev->origin + gpGlobals->v_forward * 10, angles, true, edict(), keys);
					pRoach->pev->velocity = gpGlobals->v_forward * 400;
				}
				return;
			}

			CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create( "weaponbox", pev->origin + gpGlobals->v_forward * 10, pev->angles, true, edict() );
			pWeaponBox->pev->angles.x = 0;
			pWeaponBox->pev->angles.z = 0;
			pWeaponBox->pev->avelocity = Vector(0, 256, 256);

			if (!strcmp(STRING(pWeapon->pev->classname), "weapon_9mmAR")) {
				// mp5 bounces look better with X rotations
				pWeaponBox->pev->avelocity = Vector(256, 256, 0);
			}

			pWeaponBox->PackWeapon( pWeapon ); // will also unlink the weapon from this player
			pWeaponBox->pev->velocity = pev->velocity + gpGlobals->v_forward * 400;
			
			CBasePlayerWeapon* wep = pWeapon->GetWeaponPtr();
			if (wep) {
				SET_MODEL(pWeaponBox->edict(), wep->GetModelW());
				int mergeBody = wep->CanAkimbo() ? wep->MergedModelBodyAkimbo() : wep->MergedModelBody();
				pWeaponBox->pev->body = mergeBody ? mergeBody : 0;
				pWeaponBox->pev->sequence = wep->pev->sequence;

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

			if (cwep && (cwep->params.flags & FL_WC_WEP_USE_ONLY)) {
				pWeaponBox->SetTouch(&CBaseEntity::ItemBounceTouch);
			}

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

			ApplyEffects();
			CleanupWeaponboxes();

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
	int body = 0;
	int sequence = 0;
	CBaseEntity* ammoEnt = (CBaseEntity*)CBaseEntity::Create(ammoEntName, pev->origin, pev->angles, true, edict());
	if (!ammoEnt) {
		ALERT(at_console, "Invalid ent in DropAmmo: %s\n", ammoEntName);
		return;
	}

	model = ammoEnt->pev->model;
	body = ammoEnt->pev->body;
	sequence = ammoEnt->pev->sequence;

	CWeaponBox* pWeaponBox = (CWeaponBox*)CBaseEntity::Create("weaponbox", pev->origin + gpGlobals->v_forward * 10, pev->angles, true, edict());
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
			UTIL_Remove(ammoEnt);
			ALERT(at_console, "Failed to drop ammo for exhaustible weapon %s\n", STRING(ammoEntName));
		}
		
	}
	else {
		UTIL_Remove(ammoEnt);
		const char* ammoName = secondary ? wep->pszAmmo2() : wep->pszAmmo1();
		pWeaponBox->PackAmmo(MAKE_STRING(ammoName), ammoTake);
	}
	

	if (model) {
		SET_MODEL(pWeaponBox->edict(), STRING(model));
		pWeaponBox->pev->body = body;
		pWeaponBox->pev->sequence = sequence;

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
	CBaseEntity* ent = m_rgpPlayerItems[pCheckItem->iItemSlot()].GetEntity();
	CBasePlayerItem* pItem = ent ? ent->GetWeaponPtr() : NULL;

	while (pItem)
	{
		if (FClassnameIs( pItem->pev, STRING( pCheckItem->pev->classname) ))
		{
			return TRUE;
		}

		ent = pItem->m_pNext.GetEntity();
		pItem = ent ? ent->GetWeaponPtr() : NULL;
	}

	return FALSE;
}

CBasePlayerItem* CBasePlayer::GetNamedPlayerItem(const char* pszItemName) {
	CBasePlayerItem* pItem;
	int i;
	
	if (!pszItemName)
		return NULL;

	pszItemName = CBasePlayerWeapon::GetClassFromInfoName(pszItemName);

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		pItem = (CBasePlayerItem*)m_rgpPlayerItems[i].GetEntity();

		while (pItem)
		{
			if (!strcmp(pszItemName, STRING(pItem->pev->classname)))
			{
				return pItem;
			}
			pItem = (CBasePlayerItem*)pItem->m_pNext.GetEntity();
		}
	}

	return NULL;
}

CBasePlayerItem* CBasePlayer::GetPlayerItemById(int id) {
	for (int i = 0; i < MAX_ITEM_TYPES; i++) {
		CBasePlayerItem* pItem = (CBasePlayerItem*)m_rgpPlayerItems[i].GetEntity();

		while (pItem) {
			CBasePlayerWeapon* wep = pItem->GetWeaponPtr();

			if (wep && wep->m_iId == id) {
				return pItem;
			}

			pItem = (CBasePlayerItem*)pItem->m_pNext.GetEntity();
		}
	}

	return NULL;
}

//=========================================================
// HasNamedPlayerItem Does the player already have this item?
//=========================================================
BOOL CBasePlayer::HasNamedPlayerItem( const char *pszItemName )
{
	return GetNamedPlayerItem(pszItemName) ? TRUE : FALSE;
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

	if (m_weaponsDisabled) {
		return FALSE;
	}

	if (m_pActiveItem)
	{
		CBasePlayerWeapon* wep = m_pActiveItem->GetWeaponPtr();
		if (wep) {
			wep->Holster();
		}
		
	}

	ResetAutoaim();

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
			Observer_SetMode(OBS_ROAMING);

		else if (pev->iuser1 == OBS_IN_EYE)
			Observer_SetMode(OBS_CHASE_LOCKED);

		else if (pev->iuser1 == OBS_ROAMING)
			Observer_SetMode(OBS_IN_EYE);

		else if (pev->iuser1 == OBS_MAP_FREE)
			Observer_SetMode(OBS_CHASE_LOCKED);

		else
			Observer_SetMode(OBS_CHASE_LOCKED);

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

	// keep copying origin so that the view doesn't break after teleporting large distances
	UTIL_SetOrigin(pev, target->pev->origin);

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
			UTIL_ClientPrint(this, print_center, "#Spec_NoTarget");
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
	case OBS_IN_EYE:
		UTIL_ClientPrint(this, print_center, "First-Person Cam\n[1/4]");
		break;
	case OBS_CHASE_LOCKED:
		UTIL_ClientPrint(this, print_center, "Locked Chase Cam\n[2/4]");
		break;
	case OBS_CHASE_FREE:
		UTIL_ClientPrint(this, print_center, "Free Chase Cam\n[3/4]");
		break;
	case OBS_ROAMING:
		UTIL_ClientPrint(this, print_center, "Free-Look\n[4/4]");
		break;
	case OBS_MAP_FREE:
		UTIL_ClientPrint(this, print_center, "Free Map Overview\n");
		break;
	case OBS_MAP_CHASE:
		UTIL_ClientPrint(this, print_center, "Chase Map Overview\n");
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
	bool statusChanged = GetScoreboardStatus() != m_lastScoreStatus;
	bool scoreChanged = m_lastScore != (int)pev->frags;

	if (IsSevenKewpClient()) {
		float now = g_engfuncs.pfnTime();
		if (now - m_lastTimeLeftUpdate > 0.9f) { // a little fast so seconds aren't skipped
			int timeleft = timelimit.value*60 - gpGlobals->time;
			MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgTimeLeft, 0, edict());
			WRITE_LONG(timelimit.value > 0 ? timeleft : -1);
			MESSAGE_END();

			const char* nextmap = CVAR_GET_STRING("mp_nextmap");
			if (m_lastTimeLeftUpdate == 0 || strcmp(m_lastNextMap, nextmap)) {
				strcpy_safe(m_lastNextMap, nextmap, sizeof(m_lastNextMap));
				MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgNextMap, 0, edict());
				WRITE_STRING(nextmap);
				MESSAGE_END();
			}
			m_lastTimeLeftUpdate = now;
		}
	}	

	if ((!statusChanged && !scoreChanged) || g_engfuncs.pfnTime() - m_lastScoreUpdate < 0.1f) {
		return;
	}
	m_lastScoreUpdate = g_engfuncs.pfnTime();
	m_lastScore = pev->frags;

	if (tempNameActive) {
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBaseEntity* pPlayer = UTIL_PlayerByIndex(i);

			if (!pPlayer) {
				continue;
			}

			// prevent flashing NPC names while gaining score shooting them
			UpdateTeamInfo(i == entindex() ? m_tempTeam : -1, MSG_ONE, edict());
		}
	}
	else {
		UpdateTeamInfo();
	}
}

void CBasePlayer::UpdateTag(CBasePlayer* dst) {
	if (!dst && g_engfuncs.pfnTime() - m_lastTagUpdate < 0.05f)
		return;

	uint16_t hp = UTIL_CompressUint(clampf(pev->health, 0, UINT_MAX));
	uint16_t maxHp = UTIL_CompressUint(clampf(pev->max_health, 0, UINT_MAX));
	uint16_t armor = UTIL_CompressUint(clampf(pev->armorvalue, 0, UINT_MAX));
	uint8_t observer = ((pev->iuser2-1) << 3) | (pev->iuser1 & 0x7);
	bool statusChanged = hp != m_lastTagHp || maxHp != m_lastTagMaxHp || armor != m_lastTagArmor
		|| observer != m_lastTagObserver;

	if (!dst && !statusChanged) {
		return;
	}

	m_lastTagUpdate = g_engfuncs.pfnTime();
	m_lastTagHp = hp;
	m_lastTagMaxHp = maxHp;
	m_lastTagArmor = armor;
	m_lastTagObserver = observer;

	for (int i = 1; i < gpGlobals->maxClients; i++) {
		CBasePlayer* targetPlr = UTIL_PlayerByIndex(i);

		if (!targetPlr || !targetPlr->IsSevenKewpClient())
			continue;

		if (dst && dst != targetPlr)
			continue;

		MESSAGE_BEGIN(MSG_ONE, gmsgTagInfo, 0, targetPlr->edict());
		WRITE_BYTE(entindex());
		WRITE_SHORT(hp);
		WRITE_SHORT(maxHp);
		WRITE_SHORT(armor);
		WRITE_BYTE(observer);
		MESSAGE_END();
	}	
}

void CBasePlayer::UpdateTagPos() {
	if (!IsSevenKewpClient())
		return;

	if (g_engfuncs.pfnTime() - m_lastTagPosUpdate < 0.1f) {
		return;
	}
	m_lastTagPosUpdate = g_engfuncs.pfnTime();

	char* info = g_engfuncs.pfnGetInfoKeyBuffer(edict());
	char* nametags = g_engfuncs.pfnInfoKeyValue(info, "cl_nametags");
	if (!nametags || atoi(nametags) < 2) {
		return;
	}

	static uint8_t tagData[32*8];
	mstream dat((char*)tagData, 32 * 8);

	bool anyUpdates = false;

	for (int i = 1; i < gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (!plr || plr == this || plr->InPVS(edict())) {
			dat.writeBit(0);
			continue;
		}
			
		int16_t x = clamp((int)(plr->pev->origin.x / 8), INT16_MIN, INT16_MAX);
		int16_t y = clamp((int)(plr->pev->origin.y / 8), INT16_MIN, INT16_MAX);
		int16_t z = clamp((int)(plr->pev->origin.z / 8), INT16_MIN, INT16_MAX);

		int k = i - 1;
		if (m_lastTagPos[k][0] == x && m_lastTagPos[k][1] == y && m_lastTagPos[k][2] == z) {
			dat.writeBit(0);
			continue;
		}

		if (dat.tell() >= 180) {
			dat.writeBit(0);
			ALERT(at_error, "Exceeded max PlayerPos bytes\n");
			continue;
		}

		dat.writeBit(1);

		if (x != m_lastTagPos[k][0]) {
			dat.writeBit(1);
			dat.writeBits(x, 13);
		}
		else {
			dat.writeBit(0);
		}

		if (y != m_lastTagPos[k][1]) {
			dat.writeBit(1);
			dat.writeBits(y, 13);
		}
		else {
			dat.writeBit(0);
		}

		if (z != m_lastTagPos[k][2]) {
			dat.writeBit(1);
			dat.writeBits(z, 13);
		}
		else {
			dat.writeBit(0);
		}

		m_lastTagPos[k][0] = x;
		m_lastTagPos[k][1] = y;
		m_lastTagPos[k][2] = z;
		
		anyUpdates = true;
	}

	dat.endBitWriting();
	int sz = dat.tell();

	if (!anyUpdates)
		return;

	MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, gmsgPlayerPos, 0, edict());
	WRITE_BYTE(sz);
	for (int i = 0; i < sz; i++) {
		WRITE_BYTE(tagData[i]);
	}
	MESSAGE_END();
}

uint16_t CBasePlayer::GetScoreboardStatus() {
	uint16_t status = 0;
	float idleTime = g_engfuncs.pfnTime() - m_lastUserInput;
	if (idleTime >= PLR_IDLE_STATE_TIME3)
		status = 5;
	else if (idleTime >= PLR_IDLE_STATE_TIME2)
		status = 4;
	else if (idleTime >= PLR_IDLE_STATE_TIME)
		status = 3;
	else if (IsAlive())
		status = 1;
	else if (IsObserver())
		status = 2;

	return status;
}

void CBasePlayer::UpdateTeamInfo(int color, int msg_mode, edict_t* dst) {
	m_lastScoreStatus = GetScoreboardStatus();

	MESSAGE_BEGIN(msg_mode, gmsgScoreInfo, 0, dst);
	WRITE_BYTE(entindex());	// client number
	WRITE_SHORT(clampf(pev->frags, INT16_MIN, INT16_MAX));
	WRITE_SHORT(m_iDeaths);
	WRITE_SHORT(m_lastScoreStatus); // playerclass, overridden to mean status
	WRITE_SHORT(color == -1 ? GetNameColor() : color);
	MESSAGE_END();

	MESSAGE_BEGIN(msg_mode, gmsgTeamInfo, 0, dst);
	WRITE_BYTE(entindex());
	WRITE_STRING((IsObserver() && color == -1) ? "" : GetTeamName());
	MESSAGE_END();
}

int CBasePlayer::GetNameColor() {
	if (IsObserver()) {
		return OBSERVER_TEAM_COLOR;
	}

	if (m_allowFriendlyFire) {
		return ENEMY_TEAM_COLOR;
	}

	return DEFAULT_TEAM_COLOR;
}

const char* CBasePlayer::GetTeamName() {
	if (IsObserver()) {
		return "";
	}
	
	if (m_allowFriendlyFire) {
		return ENEMY_TEAM_NAME;
	}

	return DEFAULT_TEAM_NAME;
}

void CBasePlayer::QueryClientType() {
	if (IsBot()) {
		m_clientEngineVersion = CLIENT_ENGINE_BOT;
		m_clientModVersion = CLIENT_MOD_BOT;
		return;
	}

	m_sevenkewpVersion = 0;
	memset(m_queryResults, 0, sizeof(string_t) * 6);

	g_engfuncs.pfnQueryClientCvarValue2(edict(), "hlcoop_version", CVAR_REQ_SEVENKEWP);
	g_engfuncs.pfnQueryClientCvarValue2(edict(), "aghl_version", CVAR_REQ_BUGFIXEDHL);
	g_engfuncs.pfnQueryClientCvarValue2(edict(), "cl_autorecord", CVAR_REQ_ADRENALINE_GAMER);
	g_engfuncs.pfnQueryClientCvarValue2(edict(), "sv_allow_shaders", CVAR_REQ_HL25);
	g_engfuncs.pfnQueryClientCvarValue2(edict(), "gl_fog", CVAR_REQ_OPENGL);
	g_engfuncs.pfnQueryClientCvarValue2(edict(), "m_mousethread_sleep", CVAR_REQ_LINUX);
}

void CBasePlayer::HandleClientCvarResponse(int requestID, const char* pszCvarName, const char* pszValue) {
	if (requestID >= 0 && requestID < 6) {
		m_queryResults[requestID] = ALLOC_STRING(pszValue);

		bool hasCvar[6];
		for (int i = 0; i < 6; i++) {
			if (!m_queryResults[i]) {
				return;
			}
			hasCvar[i] = strstr(STRING(m_queryResults[i]), "Bad CVAR request") == NULL;
		}

		// all queries finished

		if (hasCvar[CVAR_REQ_SEVENKEWP] + hasCvar[CVAR_REQ_BUGFIXEDHL] + hasCvar[CVAR_REQ_ADRENALINE_GAMER] > 1) {
			std::string clientStr;
			if (hasCvar[CVAR_REQ_SEVENKEWP])
				clientStr += "SevenKewp. ";
			if (hasCvar[CVAR_REQ_BUGFIXEDHL])
				clientStr += "BHL, ";
			if (hasCvar[CVAR_REQ_ADRENALINE_GAMER])
				clientStr += "AG, ";
			clientStr = clientStr.substr(0, clientStr.size() - 2);
			ALERT(at_error, "Client detection error. Client has cvars from multiple clients: %s\n", clientStr.c_str());
		}
		else if (hasCvar[CVAR_REQ_SEVENKEWP]) {
			m_sevenkewpVersion = atoi(STRING(m_queryResults[CVAR_REQ_SEVENKEWP]));
			m_clientModVersion = CLIENT_MOD_SEVENKEWP;
			m_clientModVersionString = ALLOC_STRING(UTIL_SevenKewpClientString(m_sevenkewpVersion));
		}
		else if (hasCvar[CVAR_REQ_BUGFIXEDHL]) {
			m_clientModVersion = CLIENT_MOD_HLBUGFIXED;
			m_clientModVersionString = ALLOC_STRING(UTIL_VarArgs("BugfixedHL %s", STRING(m_queryResults[CVAR_REQ_BUGFIXEDHL])));
		}
		else if (hasCvar[CVAR_REQ_ADRENALINE_GAMER]) {
			m_clientModVersion = CLIENT_MOD_ADRENALINE;
			m_clientModVersionString = ALLOC_STRING("Adrenaline Gamer");
		}
		else {
			// could also be using an unknown custom client, no way to know...
			m_clientModVersion = CLIENT_MOD_HL;
			m_clientModVersionString = MAKE_STRING("Half-Life");
		}

		m_clientEngineVersion = hasCvar[CVAR_REQ_HL25] ? CLIENT_ENGINE_HL_LATEST : CLIENT_ENGINE_HL_LEGACY;
		m_clientRenderer = hasCvar[CVAR_REQ_OPENGL] ? CLIENT_RENDERER_OPENGL : CLIENT_RENDERER_SOFTWARE;
		m_clientSystem = hasCvar[CVAR_REQ_LINUX] ? CLIENT_SYSTEM_WINDOWS : CLIENT_SYSTEM_LINUX;
		
		UTIL_LogPlayerEvent(edict(), "Client version: %s\n", GetClientVersionString());
		QueryClientTypeFinished();
	}
}

void CBasePlayer::QueryClientTypeFinished() {
	if (IsSevenKewpClient()) {
		// send prediction file replacements
		MESSAGE_BEGIN(MSG_ONE, gmsgPredFiles, NULL, pev);
		WRITE_BYTE(g_predMsgLen);
		WRITE_BYTES(g_predMsgData, g_predMsgLen);
		MESSAGE_END();

		UTIL_SendPredictionCvars(this);

		// send replacement file paths (client HUD won't initialize without this)
		CWorld* world = (CWorld*)CBaseEntity::Instance(ENT(0));
		MESSAGE_BEGIN(MSG_ONE, gmsgMatsPath, NULL, pev);
		WRITE_STRING(STRING(world->m_materialsFile));
		WRITE_STRING(STRING(world->m_hudFile));
		MESSAGE_END();

		// default hud color
		int r, g, b;
		if (UTIL_ParseHexColor(mp_hud_color.string, r, g, b)) {
			MESSAGE_BEGIN(MSG_ONE, gmsgHudColor, NULL, pev);
			WRITE_BYTE(r);
			WRITE_BYTE(g);
			WRITE_BYTE(b);
			MESSAGE_END();
		}

		// activate fog
		if (g_fog_enabled) {
			CBaseEntity* ent = NULL;
			while ((ent = UTIL_FindEntityByClassname(ent, "env_*"))) {
				CEnvWeather* weather = ent->MyWeatherPointer();
				if (weather && weather->m_useFog && weather->m_isActive) {
					weather->SendFogMessage(this);
					break;
				}
			}
		}

		// get health of other players
		for (int i = 1; i < gpGlobals->maxClients; i++) {
			CBasePlayer* otherPlr = UTIL_PlayerByIndex(i);

			if (otherPlr)
				otherPlr->UpdateTag(this);
		}
	}

	// reset update timers
	m_lastTagHp = 0;
	m_lastTagUpdate = 0;
	m_lastTagPosUpdate = 0;

	for (int i = 0; i < 32; i++) {
		memset(m_lastTagPos[i], 0, sizeof(m_lastTagPos[i]));
	}

	// can init the weapon hud now without crashing certain clients
	m_clientCheckFinished = true;

	// equip the player now that we know which weapons they can use
	g_pGameRules->PlayerSpawn(this);

	// recalculate HUD visibility
	m_iClientHideHUD = -1;

	// hide crosshair in case the player had one last map (m_pWeapon doesn't reset on the client)
	if (!m_pActiveItem)
		UTIL_UpdateWeaponState(this, 1, WEAPON_CROWBAR, -1);
}

client_info_t CBasePlayer::GetClientInfo() {
	client_info_t info;

	info.engine_version = m_clientEngineVersion;
	info.mod_version = m_clientModVersion;
	info.renderer = m_clientRenderer;

	switch (m_clientEngineVersion) {
	case CLIENT_ENGINE_HL_LATEST:
		info.max_edicts = MAX_CLIENT_ENTS;
		info.max_packet_entities = MAX_PACKET_ENTITIES;
		break;
	case CLIENT_ENGINE_HL_LEGACY:
	default: // better safe than sorry
		info.max_edicts = MAX_LEGACY_CLIENT_ENTS;
		info.max_packet_entities = MAX_LEGACY_PACKET_ENTITIES;
		break;
	}

	return info;
}

void CBasePlayer::SendLegacyClientWarning() {
	if (m_sentClientWarning || m_clientEngineVersion != CLIENT_ENGINE_HL_LEGACY) {
		return;
	}

	m_sentClientWarning = true;
	UTIL_ClientPrint(this, print_chat, "[info] This map does not function properly with steam_legacy clients. Check your console for more information.\n");
		
	UTIL_ClientPrint(this, print_console, "\n-------------------------------------------------------------------------\n");
	UTIL_ClientPrint(this, print_console, "This mod is not 100% compatible with the \"Pre-25th Anniversary Build\" of Half-Life.\n");
	UTIL_ClientPrint(this, print_console, "Some objects and effects have been made invisible to you so that you aren't kicked.\n");
	UTIL_ClientPrint(this, print_console, "To fix this, set \"Beta Participation\" to \"None\" in Steam, or wait for the next map.\n");
	UTIL_ClientPrint(this, print_console, "-------------------------------------------------------------------------\n\n");

	UTIL_LogPlayerEvent(edict(), "was sent the steam_legacy client warning\n");
}

void CBasePlayer::SendSevenKewpClientNotice(const char* weaponName) {
	if (!mp_sevenkewp_client_notice.value) {
		return;
	}

	std::string clientReq = UTIL_SevenKewpClientString(SEVENKEWP_VERSION);
	UTIL_ClientPrint(this, print_chat, UTIL_VarArgs(
		"The \"%s\" requires the \"%s\" client. Check your console for more info.\n",
		weaponName, clientReq.c_str()));

	UTIL_ClientPrint(this, print_console, "\n-----------------------------------------------------------------\n");
	UTIL_ClientPrint(this, print_console, UTIL_VarArgs("This server requires the \"%s\" client to use some\n", clientReq.c_str()));
	UTIL_ClientPrint(this, print_console, "weapons. The reason for this is that custom hitscan weapons feel ");
	UTIL_ClientPrint(this, print_console, "awful without client-side prediction. You already have a similar");
	UTIL_ClientPrint(this, print_console, "Half-Life weapon and cannot pick up the one you wanted.\n\n");
	UTIL_ClientPrint(this, print_console, "If you don't know what weapon prediction is, type in \"cl_lw 0\" and ");
	UTIL_ClientPrint(this, print_console, "try playing the game that way. It's not fun with the average ping.\n\n");
	UTIL_ClientPrint(this, print_console, "Download the latest SevenKewp client here:\n");
	UTIL_ClientPrint(this, print_console, "https://github.com/wootguy/SevenKewp/releases/latest\n\n");
	UTIL_ClientPrint(this, print_console, "The server detected that you are using this client:\n");
	UTIL_ClientPrint(this, print_console, STRING(m_clientModVersionString));
	UTIL_ClientPrint(this, print_console, "\n-----------------------------------------------------------------\n\n");
	m_sentSevenKewpNotice = true;
}

const char* CBasePlayer::GetClientVersionString() {
	const char* engineVersion = "";
	const char* renderer = "";
	const char* system = "";

	if (m_clientEngineVersion == CLIENT_ENGINE_HL_LEGACY)
		engineVersion = " (steam_legacy)";

	if (m_clientRenderer == CLIENT_RENDERER_OPENGL)
		renderer = " (OpenGL)";
	else if (m_clientRenderer == CLIENT_RENDERER_SOFTWARE)
		renderer = " (Software)";

	if (m_clientSystem == CLIENT_SYSTEM_WINDOWS)
		system = " (Windows)";
	else if (m_clientSystem == CLIENT_SYSTEM_LINUX)
		system = " (Linux)";

	return UTIL_VarArgs("%s%s%s%s", STRING(m_clientModVersionString), engineVersion, renderer, system);
}

const char* CBasePlayer::GetSteamID() {
	return g_engfuncs.pfnGetPlayerAuthId(edict());
}

uint64_t CBasePlayer::GetSteamID64() {
	const char* id = GetSteamID();

	if (!strcmp(id, "STEAM_ID_NULL") || !strcmp(id, "STEAM_ID_LAN") || !strcmp(id, "BOT")) {
		return 0;
	}

	return steamid_to_steamid64(id);
}

int CBasePlayer::GetUserID() {
	return  g_engfuncs.pfnGetPlayerUserId(edict());
}

void CBasePlayer::ShowInteractMessage(const char* msg) {
	if (gpGlobals->time - m_lastInteractMessage < 1.0f) {
		// prevent message spam
		return;
	}

	m_lastInteractMessage = gpGlobals->time;

	CWeaponInventory* wep = (CWeaponInventory*)GetNamedPlayerItem("weapon_inventory");

	if (wep && m_pActiveItem.GetEntity() == wep) {
		// weapon has a constant center print active, so append to that instead of starting a new message
		wep->SetError(msg);
	}
	else {
		UTIL_ClientPrint(this, print_center, msg);
	}
}

bool CBasePlayer::DropAllInventoryItems(bool deathDrop, bool respawnDrop, bool forceDrop) {
	if (!m_inventory) {
		return true;
	}

	int itemCount = CountInventoryItems();

	float dropSpread = itemCount == 1 ? 0 : V_min(90, 10.0f * itemCount);
	float spreadStep = dropSpread / (float)(itemCount - 1);

	// drop all inventory items instead of the weapon itself
	CItemInventory* item = m_inventory.GetEntity()->MyInventoryPointer();
	float spread = -dropSpread * 0.5f;

	CWeaponInventory* invWep = (CWeaponInventory*)GetNamedPlayerItem("weapon_inventory");

	Vector old_vangle = pev->v_angle;

	while (item) {
		CItemInventory* nextItem = item->m_pNext ? item->m_pNext.GetEntity()->MyInventoryPointer() : NULL;

		bool shouldDrop = item->m_holder_can_drop && !deathDrop && !respawnDrop;
		if (deathDrop) {
			shouldDrop = !item->m_holder_keep_on_death;
		}
		if (respawnDrop) {
			shouldDrop = !item->m_holder_keep_on_respawn;
		}
		if (forceDrop) {
			shouldDrop = true;
		}

		if (shouldDrop) {
			pev->v_angle = Vector(old_vangle.x, old_vangle.y + spread, 0);
			item->Detach(true);
		}
		else if (invWep && !deathDrop && !respawnDrop) {
			invWep->SetError(INV_ERROR_CANT_DROP);
			item->FireInvTargets(this, item->m_target_cant_drop);
		}

		item = nextItem;
		spread += spreadStep;
	}

	pev->v_angle = old_vangle;

	return !m_inventory;
}

void CBasePlayer::Revive() {
	LeaveObserver(false);

	// in case revived in a vent or something
	UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	pev->flags |= FL_DUCKING;
	m_afPhysicsFlags |= PFLAG_DUCKING;
	pev->view_ofs = VEC_DUCK_VIEW;

	pev->max_health = m_deathHealthMax;
	pev->movetype = m_deathMovetype;
	pev->rendermode = m_deathRenderMode;
	pev->renderamt = m_deathRenderAmt;
	pev->renderfx = m_deathRenderFx;
	pev->rendercolor = m_deathRenderColor;

	pev->health = pev->max_health * 0.5f;
	pev->deadflag = DEAD_NO;
	pev->solid = SOLID_SLIDEBOX;
	pev->effects &= ~EF_NODRAW;

	pev->fov = m_iFOV = 0;// init field of view.
	m_iClientFOV = -1; // make sure fov reset is sent
	m_flNextAttack = UTIL_WeaponTimeBase();
	m_flFlashLightTime = 1; // force first message
	m_deathMessageSent = false;
	m_droppedDeathWeapons = false;

	m_pLastItem = NULL;
	m_fInitHUD = TRUE;
	m_iClientHideHUD = -1;  // force this to be recalculated
	m_pClientActiveItem = NULL;
	m_iClientBattery = -1;
	memset(m_rgAmmoLast, 0, sizeof(int) * MAX_AMMO_SLOTS); // init ammo hud

	pev->nextthink = 0;
	SetThink(NULL);

	if (!m_hActiveCamera) {
		SET_VIEW(edict(), edict());
	}

	TabulateWeapons();
	ApplyEffects();

	// unholster held weapon
	CBasePlayerWeapon* wep = m_pActiveItem ? m_pActiveItem->GetWeaponPtr() : NULL;
	if (wep) {
		wep->Deploy();
	}
	else {
		// or next best weapon
		g_pGameRules->GetNextBestWeapon(this, NULL);
	}
}

float CBasePlayer::GetDamage(float defaultDamage) {
	return m_pActiveItem ? m_pActiveItem->GetDamage(defaultDamage) : CBaseMonster::GetDamage(defaultDamage);
}

void CBasePlayer::PenalizeDeath() {
	m_iDeaths += 1;
	m_scoreMultiplier = GetScoreMultiplier();
}

float CBasePlayer::GetScoreMultiplier(int deaths) {
	if (deaths < 0) {
		deaths = m_iDeaths;
	}
	return V_max(mp_min_score_mult.value / 100.0f, 1.0f / (deaths + 1));
}

float CBasePlayer::GetIdleTime() {
	return g_engfuncs.pfnTime() - m_lastUserInput;
}

float CBasePlayer::GetUseTime() {
	if (!m_useKeyTime) {
		return 0;
	}

	return g_engfuncs.pfnTime() - m_useKeyTime;
}

Vector CBasePlayer::GetSnappedLookDir() {
	Vector angles = pev->v_angle;

	// snap to 90 degree angles
	angles.y = (int((angles.y + 180 + 45) / 90) * 90) - 180;
	angles.x = (int((angles.x + 180 + 45) / 90) * 90) - 180;

	// vertical unblocking has priority, unless on the floor and looking down
	CBaseEntity* groundEnt = Instance(pev->groundentity);
	bool lookingDownOnBspModel = (pev->flags & FL_ONGROUND) && angles.x > 0
		&& groundEnt && groundEnt->IsBSPModel();

	if (angles.x != 0 && !lookingDownOnBspModel) {
		angles.y = 0;
	}
	else {
		angles.x = 0;
	}

	MAKE_VECTORS(angles);

	return gpGlobals->v_forward;
}

int CBasePlayer::GetTraceHull() {
	return (pev->flags & FL_DUCKING) ? head_hull : human_hull;
}

CBaseEntity* CBasePlayer::AntiBlockTrace() {
	const float distance = 8.0f; // long enough so antiblock works on monsters walking away from you

	TraceResult tr;
	Vector snappedLookDir = GetSnappedLookDir();
	int hullType = GetTraceHull();
	TRACE_HULL(pev->origin, pev->origin + snappedLookDir * distance, dont_ignore_monsters, hullType, edict(), &tr);

	// try again in case the blocker is on a slope or stair
	if (snappedLookDir.z == 0 && tr.pHit && (tr.pHit->v.solid == SOLID_BSP || tr.pHit->v.movetype == MOVETYPE_PUSHSTEP)) {
		Vector verticalDir = Vector(0, 0, 36);
		if (!(pev->flags & FL_ONGROUND)) {
			// probably on the ceiling, so try starting the trace lower instead (e.g. negative gravity or ladder)
			verticalDir.z = -36;
		}

		// first see how far up/down the trace can go
		TRACE_HULL(pev->origin, pev->origin + verticalDir, dont_ignore_monsters, hullType, edict(), &tr);

		if (!tr.pHit || (tr.pHit->v.solid == SOLID_BSP || tr.pHit->v.movetype == MOVETYPE_PUSHSTEP)) {
			// now do the outward trace
			TRACE_HULL(tr.vecEndPos, tr.vecEndPos + snappedLookDir * distance, dont_ignore_monsters, hullType, edict(), &tr);
		}
	}

	if (tr.pHit) {
		Vector dir = (tr.pHit->v.origin - pev->origin).Normalize();
		if (DotProduct(dir, GetLookDirection()) < 0) {
			// probably stuck inside of the target
			// skipping prevents double swapping where both players are moving forward against
			// each other and swapping at the same time
			return NULL; 
		}
	}

	return Instance(tr.pHit);
}

int CBasePlayer::TryAntiBlock() {
	CBaseEntity* target = AntiBlockTrace();

	if (!target || (!target->IsNormalMonster() && !target->IsPlayer())) {
		return 0;
	}

	if (target->IsPlayer() && m_nextAntiBlock > gpGlobals->time) {
		float timeLeft = m_nextAntiBlock - gpGlobals->time;
		UTIL_ClientPrint(this, print_center, UTIL_VarArgs("Wait %.1fs", timeLeft + 0.05f));
		return 2;
	}

	CBaseMonster* mon = target->MyMonsterPointer();

	if (mon) {
		if (IRelationship(mon) > R_NO) {
			UTIL_ClientPrint(this, print_center, "Can't swap with enemy NPCs");
			return 2;
		}
		if (mon->m_hCine) {
			UTIL_ClientPrint(this, print_center, "Swap failed; target is busy");
			return 2;
		}
	}

	CBasePlayer* targetPlr = target->MyPlayerPointer();
	Vector lookDir = GetLookDirection();
	Vector targetLookDir = target->GetLookDirection();

	// long cooldown if the target doesn't see this happening. The swapper is
	// probably being rude or impatient. If not, it's still jarring to be
	// moved around by something you didn't see coming
	bool canBeRudeTo = !targetPlr || targetPlr->GetIdleTime() > 5;
	bool isRude = !canBeRudeTo && DotProduct(lookDir, targetLookDir) > -0.5;

	Vector srcOri = pev->origin;
	bool srcDucking = (pev->flags & FL_DUCKING) != 0;
	bool dstDucking = (target->pev->flags & FL_DUCKING) != 0 || !targetPlr;

	Vector oldOriginSelf = pev->origin;
	Vector oldOriginTarget = target->pev->origin;

	pev->origin = target->pev->origin;
	target->pev->origin = srcOri;

	if (!targetPlr) {
		// swapping with a monster
		float zDiff = srcDucking ? 18 : 36;
		pev->origin.z += zDiff;
		target->pev->origin.z -= zDiff;
	}

	bool dstElev = !FNullEnt(target->pev->groundentity) && target->pev->groundentity->v.velocity.z != 0;
	bool srcElev = !FNullEnt(pev->groundentity) && pev->groundentity->v.velocity.z != 0;

	// prevent elevator gibbing
	if (dstElev) {
		pev->origin.z += 18;
	}
	if (srcElev) {
		target->pev->origin.z += 18;
	}

	if (dstDucking) {
		pev->flDuckTime = 26;
		pev->flags |= FL_DUCKING;
		pev->view_ofs = Vector(0, 0, 12);

		// prevent gibbing on elevators when swapper is crouching and swappee is not
		// (additional height needed on top of the default extra height)
		if (!srcDucking && dstElev) {
			pev->origin.z += 18;
		}
	}

	if (srcDucking) {
		target->pev->flDuckTime = 26;
		target->pev->flags |= FL_DUCKING;
		target->pev->view_ofs = Vector(0, 0, 12);

		if (!dstDucking && srcElev) {
			target->pev->origin.z += 18;
		}
	}

	TraceResult tr;
	if (targetPlr) {
		TRACE_HULL(target->pev->origin, target->pev->origin, ignore_monsters, targetPlr->GetTraceHull(), NULL, &tr);
	}
	else {
		TRACE_MONSTER_HULL(target->edict(), target->pev->origin, target->pev->origin, ignore_monsters, NULL, &tr);
	}
	if (tr.fStartSolid) {
		pev->origin = oldOriginSelf;
		target->pev->origin = oldOriginTarget;
		UTIL_ClientPrint(this, print_center, "Swap failed; target would be stuck");
		return 2;
	}

	// link the edicts for triggers
	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetOrigin(target->pev, target->pev->origin);

	if (isRude) {
		m_nextAntiBlock = gpGlobals->time + mp_antiblock_cooldown.value;
	}
	
	return 1;
}

void CBasePlayer::SaveScore() {
	player_score_t score;
	score.frags = pev->frags;
	score.deaths = m_iDeaths;
	score.multiplier = m_scoreMultiplier;
	g_playerScores[GetSteamID64()] = score;
}

void CBasePlayer::LoadScore() {
	auto previousScore = g_playerScores.find(GetSteamID64());
	if (previousScore != g_playerScores.end()) {
		player_score_t score = previousScore->second;
		pev->frags = score.frags;
		m_iDeaths = score.deaths;
		m_scoreMultiplier = score.multiplier;
	}
	else {
		pev->frags = 0;
		m_iDeaths = 0;
		m_scoreMultiplier = 1.0f;
	}
}

void CBasePlayer::SaveInventory() {
	player_inventory_t inv;
	
	for (int i = 0; i < MAX_ITEM_TYPES; i++) {
		if (m_rgpPlayerItems[i]) {
			CBaseEntity* ent = m_rgpPlayerItems[i].GetEntity();
			CBasePlayerWeapon* wep = ent ? ent->GetWeaponPtr() : NULL;

			while (wep) {
				inv.weapons.put(STRING(wep->pev->classname));
				inv.weaponClips[wep->m_iId] = wep->m_iClip;
				CBaseEntity* next = wep->m_pNext.GetEntity();
				wep = next ? next->GetWeaponPtr() : NULL;
			}
		}
	}

	CBaseEntity* activeItem = m_pActiveItem.GetEntity();
	CBasePlayerWeapon* activeWep = activeItem ? activeItem->GetWeaponPtr() : NULL;

	inv.health = pev->health;
	inv.armor = pev->armorvalue;
	inv.hasLongjump = m_fLongJump;
	inv.flashlightBattery = m_iFlashBattery;
	inv.activeWeaponId = activeWep ? activeWep->m_iId : NULL;

	memcpy(inv.m_rgAmmo, m_rgAmmo, MAX_AMMO_SLOTS * sizeof(int));
	
	g_playerInventory[GetSteamID64()] = inv;
}

bool CBasePlayer::LoadInventory() {
	auto previousInv = g_playerInventory.find(GetSteamID64());
	if (previousInv != g_playerInventory.end()) {
		player_inventory_t inv = previousInv->second;

		if (mp_keep_inventory.value >= 2 && inv.health < 1) {
			return false; // player was dead during level change. Give them the default loadout.
		}
		
		StringSet::iterator_t iter;
		while (inv.weapons.iterate(iter)) {
			if (!HasNamedPlayerItem(iter.key)) {
				const char* itemName = STRING(ALLOC_STRING(iter.key));
				GiveNamedItem(itemName);
				CBasePlayerItem* item = GetNamedPlayerItem(itemName);
				CBasePlayerWeapon* wep = item ? item->GetWeaponPtr() : NULL;
				if (wep) {
					wep->m_iClip = inv.weaponClips[wep->m_iId];
				}
			}
		}

		for (int i = 0; i < MAX_AMMO_SLOTS; i++) {
			m_rgAmmo[i] = V_max(m_rgAmmo[i], inv.m_rgAmmo[i]);
		}

		CBasePlayerItem* item = GetPlayerItemById(inv.activeWeaponId);
		if (item)
			SwitchWeapon(item);		

		if (inv.hasLongjump) {
			m_fLongJump = TRUE;
			g_engfuncs.pfnSetPhysicsKeyValue(edict(), "slj", "1");
		}

		if (mp_keep_inventory.value >= 2) {
			pev->health = inv.health;
			pev->armorvalue = inv.armor;
			m_iFlashBattery = inv.flashlightBattery;

			// only load inventory once. Then use map inventory after death
			g_playerInventory.erase(previousInv);
		}

		return true;
	}

	return false;
}

void CBasePlayer::ResolveWeaponSlotConflict(int wepId) {
	uint64_t mask = g_weaponSlotMasks[wepId];

	if (count_bits_set(mask) <= 1) {
		return; // impossible for there to be a conflict
	}

	ItemInfo& II = CBasePlayerItem::ItemInfoArray[wepId];
	CBaseEntity* heldEnt = m_pActiveItem.GetEntity();
	CBasePlayerWeapon* heldItem = heldEnt ? heldEnt->GetWeaponPtr() : NULL;

	// fix hud conflict on the client
	for (int i = 0; i < MAX_WEAPONS; i++) {
		uint64_t bit = (1ULL << i);
		if (mask & bit) {
			if ((m_weaponBits & bit) && i != wepId) {
				// player is already holding a weapon that fills this slot.
				// Drop this held weapon because the player won't be able to choose which
				// weapon they want from that slot.
				ItemInfo& dropInfo = CBasePlayerItem::ItemInfoArray[i];
				DropPlayerItem(dropInfo.pszName);

				// dropping the held weapon?
				if (i == heldItem->m_iId) {
					// switch to the weapon that resolved the conflict
					int activeId = GetCurrentIdForConflictedSlot(i);
					CBasePlayerItem* item = GetPlayerItemById(activeId);
					if (item)
						SwitchWeapon(item);
				}
				else {
					// keep the held item active
					SwitchWeapon(heldItem);
				}
			}

			// redirect all item info to the weapon with the given ID
			MESSAGE_BEGIN(MSG_ONE, gmsgWeaponList, NULL, pev);
			WRITE_STRING(II.pszName);			// string	weapon name
			WRITE_BYTE(GetAmmoIndex(II.pszAmmo1));	// byte		Ammo Type
			WRITE_BYTE(II.iMaxAmmo1);				// byte     Max Ammo 1
			WRITE_BYTE(GetAmmoIndex(II.pszAmmo2));	// byte		Ammo2 Type
			WRITE_BYTE(II.iMaxAmmo2);				// byte     Max Ammo 2
			WRITE_BYTE(II.iSlot);					// byte		bucket
			WRITE_BYTE(II.iPosition);				// byte		bucket pos
			WRITE_BYTE(i);							// byte		id (bit index into pev->weapons)
			WRITE_BYTE(II.iFlags);					// byte		Flags
			MESSAGE_END();

			if (IsSevenKewpClient()) {
				MESSAGE_BEGIN(MSG_ONE, gmsgWeaponListX, NULL, pev);
				WRITE_BYTE(i);
				WRITE_BYTE(II.iFlagsEx);
				WRITE_SHORT(V_min(65535, II.fAccuracyDeg * 100));
				WRITE_SHORT(V_min(65535, II.fAccuracyDeg2 * 100));
				WRITE_SHORT(V_min(65535, II.fAccuracyDegY * 100));
				WRITE_SHORT(V_min(65535, II.fAccuracyDegY2 * 100));
				MESSAGE_END();
			}
		}
	}

	FixSharedWeaponSlotClipCount();
}

void CBasePlayer::FixSharedWeaponSlotClipCount(int thisIdOnly) {
	if (!m_fKnownItem)
		return; // player will reset clips again once weapon list is sent

	for (int i = 0; i < MAX_WEAPONS; i++) {
		if (thisIdOnly >= 0 && thisIdOnly != i)
			continue;
		
		uint64_t mask = g_weaponSlotMasks[i];
		if (!mask || count_bits_set(mask) <= 1)
			continue;

		// this slot has a conflict
		int activeId = GetCurrentIdForConflictedSlot(i);
		if (activeId == -1)
			continue;

		// weapon that we're resolving the conflict with
		ItemInfo& II = CBasePlayerItem::ItemInfoArray[activeId];
		if (!II.pszName)
			continue; // not in this map
		if (II.iMaxClip <= 0)
			continue; // ok for this wp clip to be reset to 0 randomly, because it's always 0.

		CBasePlayerItem* item = GetNamedPlayerItem(II.pszName);
		CBasePlayerWeapon* resolveWep = item ? item->GetWeaponPtr() : NULL;

		if (!resolveWep)
			continue;

		// send active clip for all IDs that share this slot
		for (int k = 0; k < MAX_WEAPONS; k++) {
			if (i == k && thisIdOnly != -1)
				continue; // caller will send this weapon clip later

			uint64_t bit = (1ULL << k);
			if (mask & bit) {
				UTIL_UpdateWeaponState(this, 0, k, resolveWep->m_iClip);
			}
		}
	}
}

int CBasePlayer::GetCurrentIdForConflictedSlot(int wepId) {
	uint64_t mask = g_weaponSlotMasks[wepId];

	if (count_bits_set(mask) <= 1) {
		return wepId; // impossible for there to be a conflict
	}

	for (int i = 0; i < MAX_WEAPONS; i++) {
		uint64_t bit = (1ULL << i);
		if ((mask & bit) && (m_weaponBits & bit)) {
			return i;
		}
	}

	return -1;
}

const char* CBasePlayer::GetDeathNoticeWeapon() {
	return m_pActiveItem ? m_pActiveItem->GetDeathNoticeWeapon() : "skull";
}

void CBasePlayer::NightvisionUpdate() {

	if (!m_flashlightEnabled || flashlight.value < 2 || g_engfuncs.pfnTime() - m_lastNightvisionUpdate < 0.05f) {
		return;
	}

	m_lastNightvisionUpdate = g_engfuncs.pfnTime();

	const int radius = 100; // 255 makes more sense, but it's really laggy for all PCs
	const RGB color = RGB(128, 128, 128);
	const int life = 2;
	const int decay = 1;

	if (UTIL_IsValidTempEntOrigin(pev->origin)) {
		// unreliable messages can be sent faster
		MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, SVC_TEMPENTITY, NULL, pev);
		WRITE_BYTE(TE_DLIGHT);
		WRITE_COORD_VECTOR(pev->origin);
		WRITE_BYTE(radius);
		WRITE_BYTE(color.r);
		WRITE_BYTE(color.g);
		WRITE_BYTE(color.b);
		WRITE_BYTE(life);
		WRITE_BYTE(decay);
		MESSAGE_END();
	}
	else {
		UTIL_DLight(pev->origin, radius, color, life, decay);
	}

	if (g_engfuncs.pfnTime() - m_lastNightvisionFadeUpdate < 1.0f) {
		return;
	}
	m_lastNightvisionFadeUpdate = g_engfuncs.pfnTime();

	UTIL_ScreenFade(this, m_nightvisionColor.ToVector(), 0.1f, 3.0f, 255, FFADE_MODULATE | FFADE_IN, true);
}

void CBasePlayer::ResetSequenceInfo() {
	CBaseAnimating::ResetSequenceInfo();

	if (m_playerModel) {
		// m_flFrameRate must match the model or else the animations stutters
		float oldFrameRate = m_flFrameRate;
		GetSequenceInfo(m_playerModel, pev, &m_flFrameRate, &m_flGroundSpeed);

		if (m_flFrameRate == 0) {
			// Fun fact: allowing 0 makes hitboxes gigantic sometimes (bigratshit on assaultmesa2).
			m_flFrameRate = oldFrameRate;
		}

		pev->framerate *= oldFrameRate / m_flFrameRate;
	}
}

void CBasePlayer::SyncGaitAnimations(int animDesired, float gaitSpeed, float defaultSyncMultiplier) {
	pev->framerate = gaitSpeed * defaultSyncMultiplier;
	
	if (!m_playerModel) {
		return;
	}

	studiohdr_t* defaultModel = (studiohdr_t*)GET_MODEL_PTR(ENT(pev));

	if (!defaultModel || animDesired >= defaultModel->numseq || animDesired >= m_playerModel->numseq) {
		return;
	}

	mstudioseqdesc_t* defaultSeq = (mstudioseqdesc_t*)((byte*)defaultModel + defaultModel->seqindex) + animDesired;
	mstudioseqdesc_t* customSeq = (mstudioseqdesc_t*)((byte*)m_playerModel + m_playerModel->seqindex) + animDesired;

	float defaultFramerate = 256 * defaultSeq->fps / (defaultSeq->numframes - 1);
	float customFramerate = 256 * customSeq->fps / (customSeq->numframes - 1);

	if (!customFramerate || !customSeq->linearmovement[0])
		return;

	// client-side gait fps depends on linear movement value stored in their model
	// which may not match the default player.mdl value.
	pev->framerate *= defaultSeq->linearmovement[0] / customSeq->linearmovement[0];

	// undo the framerate adjustment in ResetSequenceInfo
	pev->framerate *= defaultFramerate / customFramerate;

	// TODO: predict gait frame calculated by the client.
	// Arm and leg movements are starting on the wrong frames.
}

void CBasePlayer::ChangePlayerModel(const char* newModel, bool broadcast) {
	std::string oldModelStd = STRING(m_playerModelName);
	std::string newModelStd = toLowerCase(newModel);

	if (oldModelStd != newModelStd) {
		m_playerModel = GetPlayerModelPtr(newModel, m_playerModelSize);
		m_playerModelName = ALLOC_STRING(newModel);
		m_playerModelAnimSet = GetPlayerModelAnimSet(m_playerModel);
		CALL_HOOKS_VOID(pfnPlayerModelChanged, this, oldModelStd.c_str(), newModelStd.c_str());
	
		if (broadcast) {
			BroadcastUserInfo();
		}
	}
}

void CBasePlayer::BroadcastUserInfo() {
	char* info = g_engfuncs.pfnGetInfoKeyBuffer(edict());

	for (int i = 1; i < gpGlobals->maxClients; i++) {
		CBasePlayer* msgPlr = UTIL_PlayerByIndex(i);
		if (!msgPlr)
			continue;

		UTIL_SendUserInfo(msgPlr->edict(), edict(), info);
	}
}

void CBasePlayer::DebugThink() {
	if (!m_debugFlags) {
		return;
	}

	if ((m_debugFlags & DF_NODES) && gpGlobals->time - m_lastNodeUpdate > 0.25f) {
		m_lastNodeUpdate = gpGlobals->time;

		Vector viewOri = GetViewPosition();

		struct RenderLink {
			uint8_t flags;
			entvars_t* blocker;
		};

		// maps a node pair to its link flags and blocking entity
		std::unordered_map<uint32_t, RenderLink> nodePairLinks;

		int drawCount = 0;

		CNode* closestNode = NULL;
		int closestNodeId = 0;
		float closestDist = FLT_MAX;
		int modelIdx = MODEL_INDEX("sprites/laserbeam.spr");
		int w = 4;
		int lif = 2;
		const float drawDist = 1024;

		for (int i = 0; i < WorldGraph.m_cNodes; i++) {
			CNode& srcNode = WorldGraph.m_pNodes[i];

			float dist = (srcNode.m_vecOriginPeek - viewOri).Length();

			if (dist < closestDist) {
				closestDist = dist;
				closestNode = &srcNode;
				closestNodeId = i;
			}

			for (int k = 0; k < srcNode.m_cNumLinks; k++) {
				CLink& link = WorldGraph.NodeLink(i, k);
				CNode& dstNode = WorldGraph.m_pNodes[ link.m_iDestNode ];

				TraceResult tr, tr2;
				UTIL_TraceLine(viewOri, srcNode.m_vecOriginPeek, ignore_monsters, NULL, &tr);
				UTIL_TraceLine(viewOri, dstNode.m_vecOriginPeek, ignore_monsters, NULL, &tr2);
				if (tr.flFraction < 1.0f && tr2.flFraction < 1.0f)
					continue;

				float dist2 = (dstNode.m_vecOriginPeek - viewOri).Length();
				if (dist > drawDist && dist2 > drawDist) {
					continue;
				}

				uint32_t src = i;
				uint32_t dst = link.m_iDestNode;
				uint32_t nodePairHash = src > dst ? ((src << 16) | dst) : ((dst << 16) | src);

				uint8_t flags = link.m_afLinkInfo;

				if (nodePairLinks.find(nodePairHash) != nodePairLinks.end()) {
					if (nodePairLinks[nodePairHash].flags != flags) {
						if (flags > nodePairLinks[nodePairHash].flags) {
							// display the smallest hull that can cross both directions
							flags = link.m_afLinkInfo;
						}
						//ALERT(at_console, "asymetric link between %d and %d\n", src, dst);
					}
				}

				RenderLink& rlink = nodePairLinks[nodePairHash];
				rlink.flags = flags;
				if (link.m_pLinkEnt) {
					rlink.blocker = link.m_pLinkEnt;
				}
				rlink.flags = flags;
			}
		}

		for (auto item : nodePairLinks) {
			uint32_t hash = item.first;
			uint16_t nodeA = hash >> 16;
			uint16_t nodeB = hash & 0xffff;
			uint8_t linkFlags = item.second.flags;
			entvars_t* blocker = item.second.blocker;

			CNode& srcNode = WorldGraph.m_pNodes[nodeA];
			CNode& dstNode = WorldGraph.m_pNodes[nodeB];

			RGBA color(0, 0, 0, 255);
			if (linkFlags & bits_LINK_LARGE_HULL) {
				color.r += 255;
				color.b += 128;
			}
			else if (linkFlags & bits_LINK_HUMAN_HULL) {
				color.g += 128;
				color.b += 255;
			}
			else if (linkFlags & bits_LINK_SMALL_HULL) {
				color.r += 255;
				color.g += 128;
			}
			else if (linkFlags & bits_LINK_FLY_HULL) {
				color.r += 255;
				color.g += 255;
			}
			else {
				color.r += 255;
			}

			Vector dir = (dstNode.m_vecOriginPeek - srcNode.m_vecOriginPeek).Normalize();
			UTIL_MakeVectors(UTIL_VecToAngles(dir));
			Vector offset = Vector(0,0,0);
			Vector start = srcNode.m_vecOriginPeek + offset;
			Vector end = dstNode.m_vecOriginPeek + offset;

			int width = blocker ? w*8 : w;
			UTIL_BeamPoints(start, end, modelIdx, 0, 0, lif, width, 0, color, 0, MSG_ONE_UNRELIABLE, 0, edict());
			drawCount++;
		}

		if (closestNode) {
			hudtextparms_t params;
			memset(&params, 0, sizeof(hudtextparms_t));
			params.effect = 0;
			params.fadeinTime = 0;
			params.fadeoutTime = 0.1;
			params.holdTime = 1.0f;
			params.x = 0;
			params.y = -1;
			params.channel = 3;
			params.r1 = 255;
			params.g1 = 255;
			params.b1 = 255;
			params.a1 = 128;

			std::string hintStr = "None";
			if (closestNode->m_sHintType) {
				hintStr = UTIL_VarArgs("Type %d, Act %d, Yaw %.1f", (int)closestNode->m_sHintType,
					(int)closestNode->m_sHintActivity, closestNode->m_flHintYaw);
			}

			UTIL_HudMessage(this, params, UTIL_VarArgs("Node %d\nType: %s\nOrigin: %.2f %.2f %.2f\nHint: %s\n\nVisible Links: %d",
				closestNodeId,
				(closestNode->m_afNodeInfo & bits_NODE_AIR) ? "Air" : "Land",
				closestNode->m_vecOriginPeek.x, closestNode->m_vecOriginPeek.y, closestNode->m_vecOriginPeek.z,
				hintStr.c_str(), drawCount));
			Vector closestPos = closestNode->m_vecOriginPeek;
			UTIL_BeamPoints(closestPos, closestPos + Vector(0, 0, 32), modelIdx, 0, 0, lif, w, 0, RGB(255, 255, 255), 0, MSG_ONE_UNRELIABLE, 0, edict());
		}
	}
}

bool CBasePlayer::IsSevenKewpClient() {
	//return true;
	return UTIL_AreSevenKewpVersionsCompatible(m_sevenkewpVersion, SEVENKEWP_VERSION);
}

void CBasePlayer::SetThirdPersonWeaponAnim(int sequence, float fps) {
	for (int i = 1; i < gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);
		
		if (!plr || !plr->IsSevenKewpClient())
			continue;

		MESSAGE_BEGIN(MSG_ONE, gmsgPmodelAnim, 0, plr->edict());
		WRITE_BYTE(entindex() - 1);
		WRITE_BYTE(sequence);
		MESSAGE_END();
	}
}

void CBasePlayer::SetJumpPower(int power) {
	g_engfuncs.pfnSetPhysicsKeyValue(edict(), "jmp", UTIL_VarArgs("%d", power));
}

void CBasePlayer::SyncWeaponBits() {
	// weapons field sent to HL clients
	pev->weapons = m_weaponBits & 0xffffffff;
	
	if (m_weaponBits == m_lastWeaponBits || !IsSevenKewpClient()) {
		return;
	}

	// TODO: slot conflict logic from UpdateClientData

	uint64_t sentValue = m_weaponBits;

	for (int i = 0; i < MAX_WEAPONS; i++) {
		if (m_weaponBits & (1ULL << i)) {
			// weapons that share slots occupy multiple bits so that the menu renders correctly
			// (client may think the slot is empty if only one weapon is held from a shared slot)
			sentValue |= g_weaponSlotMasks[i];
		}
	}

	uint32_t high = sentValue >> 32;
	uint32_t low = sentValue & 0xffffffff;

	MESSAGE_BEGIN(MSG_ONE, gmsgWeaponBits, 0, edict());
	WRITE_LONG(low);
	WRITE_LONG(high);
	MESSAGE_END();

	m_lastWeaponBits = m_weaponBits;
}

bool CBasePlayer::HasSuit() {
	return m_weaponBits & (1ULL << WEAPON_SUIT);
}

void CBasePlayer::WaterSplashTrace(Vector vecSrc, float dist, int hull, float scale) {
	TraceResult waterTrace;
	Vector waterTraceBottom = gpGlobals->v_forward * dist;

	if (hull != point_hull) {
		waterTraceBottom = waterTraceBottom + gpGlobals->v_forward * 8 - gpGlobals->v_up * 8;
	}

	UTIL_TraceLine(vecSrc, vecSrc + waterTraceBottom, ignore_monsters, NULL, &waterTrace);
	UTIL_WaterSplashTrace(vecSrc, waterTrace.vecEndPos, scale, 3, NULL);
}
