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
//
// teamplay_gamerules.cpp
//
#include	"extdll.h"
#include	"util.h"
#include	"CBasePlayer.h"
#include	"weapons.h"
#include	"gamerules.h"
 
#include	"skill.h"
#include	"game.h"
#include	"item/CItem.h"
#include	"voice_gamemgr.h"
#include	"hltv.h"
#include "CGamePlayerEquip.h"
#include "CBasePlayerAmmo.h"
#include "CBasePlayerItem.h"
#include "CSatchel.h"
#include "monsters.h"

#if !defined ( _WIN32 )
#include <ctype.h>
#endif

extern DLL_GLOBAL CGameRules	*g_pGameRules;
extern DLL_GLOBAL BOOL	g_fGameOver;
extern int gmsgDeathMsg;	// client dll messages
extern int gmsgScoreInfo;
extern int gmsgMOTD;
extern int gmsgServerName;

extern int g_teamplay;
extern int gmsgTeamNames;
extern int gmsgTeamInfo;

#define ITEM_RESPAWN_TIME	30
#define WEAPON_RESPAWN_TIME	20
#define AMMO_RESPAWN_TIME	20

float g_flIntermissionStartTime = 0;

CVoiceGameMgr	g_VoiceGameMgr;

CMultiplayGameMgrHelper g_GameMgrHelper;

bool CMultiplayGameMgrHelper::CanPlayerHearPlayer(CBasePlayer* pListener, CBasePlayer* pTalker)
{
	if (g_teamplay)
	{
		if (g_pGameRules->PlayerRelationship(pListener, pTalker) != GR_TEAMMATE)
		{
			return false;
		}
	}

	return true;
}

//*********************************************************
// Rules for the half-life multiplayer game.
//*********************************************************

CHalfLifeMultiplay :: CHalfLifeMultiplay()
{
	m_flIntermissionEndTime = 0;
	g_flIntermissionStartTime = 0;
	m_sentTimeupMessage = false;
	m_sentTime60Message = false;
	m_sentTime300Message = false;
	m_sentTime600Message = false;
	mapcycle.items = NULL;
	
	// 11/8/98
	// Modified by YWB:  Server .cfg file is now a cvar, so that 
	//  server ops can run multiple game servers, with different server .cfg files,
	//  from a single installed directory.
	// Mapcyclefile is already a cvar.

	// 3/31/99
	// Added lservercfg file cvar, since listen and dedicated servers should not
	// share a single config file. (sjb)
	if ( IS_DEDICATED_SERVER() )
	{
		// this code has been moved into engine, to only run server.cfg once
	}
	else
	{
		// listen server
		char *lservercfgfile = (char *)CVAR_GET_STRING( "lservercfgfile" );

		if ( lservercfgfile && lservercfgfile[0] )
		{
			char szCommand[256];
			
			ALERT( at_console, "Executing listen server config file\n" );
			snprintf( szCommand, 256, "exec %s\n", lservercfgfile );
			SERVER_COMMAND( szCommand );
		}
	}
}

BOOL CHalfLifeMultiplay::ClientCommand( CBasePlayer *pPlayer, const char *pcmd )
{
	if(g_VoiceGameMgr.ClientCommand(pPlayer, pcmd))
		return TRUE;

	return CGameRules::ClientCommand(pPlayer, pcmd);
}

// longest the intermission can last, in seconds
#define MAX_INTERMISSION_TIME		120

extern cvar_t timeleft, fragsleft;

extern cvar_t mp_chattime;

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: Think ( void )
{
	g_VoiceGameMgr.Update(gpGlobals->frametime);

	///// Check game rules /////
	static int last_frags;
	static int last_time;

	int frags_remaining = 0;
	int time_remaining = 0;

	if ( g_fGameOver )   // someone else quit the game already
	{
		// bounds check
		int time = (int)CVAR_GET_FLOAT( "mp_chattime" );
		if ( time < 1 )
			CVAR_SET_STRING( "mp_chattime", "1" );
		else if ( time > MAX_INTERMISSION_TIME )
			CVAR_SET_STRING( "mp_chattime", UTIL_dtos1( MAX_INTERMISSION_TIME ) );

		// check to see if we should change levels now
		if (m_flIntermissionEndTime < gpGlobals->time ) {
			ChangeLevel();
		}

		return;
	}

	float flTimeLimit = timelimit.value * 60;
	float flFragLimit = fraglimit.value;

	time_remaining = (int)(flTimeLimit ? ( flTimeLimit - gpGlobals->time ) : 0);

	if ( flTimeLimit != 0 )
	{
		if (!m_sentTime600Message && timelimit.value > 10 && gpGlobals->time >= flTimeLimit - 60*10) {
			m_sentTime600Message = true;
			UTIL_ClientPrintAll(print_chat, "10 minutes remaining...\n");
		}
		if (!m_sentTime300Message && timelimit.value > 5 && gpGlobals->time >= flTimeLimit - 60*5) {
			m_sentTime300Message = true;
			UTIL_ClientPrintAll(print_chat, "5 minutes remaining...\n");
		}
		if (!m_sentTime60Message && timelimit.value > 1 && gpGlobals->time >= flTimeLimit - 60) {
			m_sentTime60Message = true;
			UTIL_ClientPrintAll(print_chat, "1 minute remaining...\n");
		}
		if (!m_sentTimeupMessage && gpGlobals->time >= flTimeLimit - 1.0f) {
			m_sentTimeupMessage = true;
			UTIL_ClientPrintAll(print_chat, "Time's up!\n");
		}

		if (gpGlobals->time >= flTimeLimit) {
			GoToIntermission();
			return;
		}
	}

	if ( flFragLimit )
	{
		int bestfrags = 9999;
		int remain;

		// check if any player is over the frag limit
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBaseEntity *pPlayer = UTIL_PlayerByIndex( i );

			if ( pPlayer && pPlayer->pev->frags >= flFragLimit )
			{
				GoToIntermission();
				return;
			}


			if ( pPlayer )
			{
				remain = flFragLimit - pPlayer->pev->frags;
				if ( remain < bestfrags )
				{
					bestfrags = remain;
				}
			}

		}
		frags_remaining = bestfrags;
	}

	// Updates when frags change
	if ( frags_remaining != last_frags )
	{
		g_engfuncs.pfnCvar_DirectSet( &fragsleft, UTIL_VarArgs( "%i", frags_remaining ) );
	}

	// Updates once per second
	if ( timeleft.value != last_time )
	{
		g_engfuncs.pfnCvar_DirectSet( &timeleft, UTIL_VarArgs( "%i", time_remaining ) );
	}

	last_frags = frags_remaining;
	last_time  = time_remaining;

	SurvivalModeThink();
}


//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::IsMultiplayer( void )
{
	return TRUE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::IsDeathmatch( void )
{
	return TRUE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::IsCoOp( void )
{
	return gpGlobals->coop;
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::FShouldSwitchWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
	if ( !pWeapon->CanDeploy() )
	{
		// that weapon can't deploy anyway.
		return FALSE;
	}

	if ( !pPlayer->m_pActiveItem )
	{
		// player doesn't have an active item!
		return TRUE;
	}
	CBasePlayerItem* item = (CBasePlayerItem*)pPlayer->m_pActiveItem.GetEntity();

	if ( !item->CanHolster() )
	{
		// can't put away the active item.
		return FALSE;
	}

	if ( pWeapon->iWeight() > item->iWeight() )
	{
		return TRUE;
	}

	return FALSE;
}

BOOL CHalfLifeMultiplay :: GetNextBestWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon )
{

	CBasePlayerItem *pCheck;
	CBasePlayerItem *pBest;// this will be used in the event that we don't find a weapon in the same category.
	int iBestWeight;
	int i;

	iBestWeight = -1;// no weapon lower than -1 can be autoswitched to
	pBest = NULL;

	if (pCurrentWeapon && !pCurrentWeapon->CanHolster() )
	{
		// can't put this gun away right now, so can't switch.
		return FALSE;
	}

	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		pCheck = (CBasePlayerItem*)pPlayer->m_rgpPlayerItems[i].GetEntity();

		while ( pCheck )
		{
			if (pCurrentWeapon && pCheck->iWeight() > -1 && pCheck->iWeight() == pCurrentWeapon->iWeight() && pCheck != pCurrentWeapon )
			{
				// this weapon is from the same category. 
				if ( pCheck->CanDeploy() )
				{
					if ( pPlayer->SwitchWeapon( pCheck ) )
					{
						return TRUE;
					}
				}
			}
			else if ( pCheck->iWeight() > iBestWeight && pCheck != pCurrentWeapon )// don't reselect the weapon we're trying to get rid of
			{
				//ALERT ( at_console, "Considering %s\n", STRING( pCheck->pev->classname ) );
				// we keep updating the 'best' weapon just in case we can't find a weapon of the same weight
				// that the player was using. This will end up leaving the player with his heaviest-weighted 
				// weapon. 
				if ( pCheck->CanDeploy() )
				{
					// if this weapon is useable, flag it as the best
					iBestWeight = pCheck->iWeight();
					pBest = pCheck;
				}
			}

			pCheck = (CBasePlayerItem*)pCheck->m_pNext.GetEntity();
		}
	}

	// if we make it here, we've checked all the weapons and found no useable 
	// weapon in the same catagory as the current weapon. 
	
	// if pBest is null, we didn't find ANYTHING. Shouldn't be possible- should always 
	// at least get the crowbar, but ya never know.
	if ( !pBest )
	{
		return FALSE;
	}

	pPlayer->SwitchWeapon( pBest );

	return TRUE;
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay :: ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] )
{
	string_t oldnetname = pEntity->v.netname;
	pEntity->v.netname = MAKE_STRING(pszName);
	UTIL_LogPlayerEvent(pEntity, "connected from address %s\n",	pszAddress);
	pEntity->v.netname = oldnetname;

	g_VoiceGameMgr.ClientConnected(pEntity);
	return TRUE;
}

extern int gmsgSayText;
extern int gmsgGameMode;

void CHalfLifeMultiplay :: UpdateGameMode( CBasePlayer *pPlayer )
{
	MESSAGE_BEGIN( MSG_ONE, gmsgGameMode, NULL, pPlayer->edict() );
		//WRITE_BYTE( 0 );  // game mode none
		WRITE_BYTE( 1 );  // team game
	MESSAGE_END();
}

void CHalfLifeMultiplay :: InitHUD( CBasePlayer *pl )
{
	// notify other clients of player joining the game
	//UTIL_ClientPrintAll(print_chat, UTIL_VarArgs( "%s has joined the game\n",
	//	( pl->pev->netname && STRING(pl->pev->netname)[0] != 0 ) ? STRING(pl->pev->netname) : "unconnected" ) );

	char* keybuffer = g_engfuncs.pfnGetInfoKeyBuffer(pl->edict());

	UTIL_LogPlayerEvent(pl->edict(), "entered the game as %s(%d,%d)\n",
		g_engfuncs.pfnInfoKeyValue(keybuffer, "model"),
		atoi(g_engfuncs.pfnInfoKeyValue(keybuffer, "topcolor")),
		atoi(g_engfuncs.pfnInfoKeyValue(keybuffer, "bottomcolor"))
	);

	// Send down the team names
	MESSAGE_BEGIN(MSG_ONE, gmsgTeamNames, NULL, pl->edict());
	WRITE_BYTE(4);
	WRITE_STRING(DEFAULT_TEAM_NAME);
	WRITE_STRING(ENEMY_TEAM_NAME);
	WRITE_STRING(DEFAULT_TEAM_NAME);
	WRITE_STRING(DEFAULT_TEAM_NAME);
	MESSAGE_END();

	UpdateGameMode( pl );

	// sending just one score makes the hud scoreboard active;  otherwise
	// it is just disabled for single play
	pl->UpdateTeamInfo();

	SendMOTDToClient( pl->edict() );

	// loop through all active players and send their score info to the new client
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		// FIXME:  Probably don't need to cast this just to read m_iDeaths
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex( i );

		if ( plr && plr != pl )
		{
			MESSAGE_BEGIN( MSG_ONE, gmsgScoreInfo, NULL, pl->edict() );
				WRITE_BYTE( i );	// client number
				WRITE_SHORT( plr->pev->frags );
				WRITE_SHORT( plr->m_iDeaths );
				WRITE_SHORT( 0 );
				WRITE_SHORT(plr->GetNameColor());
				//WRITE_SHORT(DEFAULT_TEAM_COLOR);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ONE, gmsgTeamInfo, NULL, pl->edict());
			WRITE_BYTE(i);
			WRITE_STRING(plr->IsObserver() ? "" : DEFAULT_TEAM_NAME);
			//WRITE_STRING(DEFAULT_TEAM_NAME);
			MESSAGE_END();
		}
	}

	if ( g_fGameOver )
	{
		MESSAGE_BEGIN( MSG_ONE, SVC_INTERMISSION, NULL, pl->edict() );
		MESSAGE_END();
	}
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: ClientDisconnected( edict_t *pClient )
{
	if ( pClient )
	{
		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance( pClient );

		if ( pPlayer )
		{
			FireTargets( "game_playerleave", pPlayer, pPlayer, USE_TOGGLE, 0 );

			if ((int)pClient->v.frags > 0)
				UTIL_LogPlayerEvent(pClient, "disconnected with %d points\n", (int)pClient->v.frags);
			else
				UTIL_LogPlayerEvent(pClient, "disconnected\n");

			player_score_t score;
			score.frags = pPlayer->pev->frags;
			score.deaths = pPlayer->m_iDeaths;
			score.multiplier = pPlayer->m_scoreMultiplier;
			g_playerScores[pPlayer->GetSteamID64()] = score;
			
			pPlayer->pev->frags = 0;
			pPlayer->m_iDeaths = 0;

			pPlayer->RemoveAllItems( TRUE );// destroy all of the players weapons and items
			pPlayer->DropAllInventoryItems();
		}
	}
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay :: FlPlayerFallDamage( CBasePlayer *pPlayer )
{
	int iFallDamage = (int)falldamage.value;

	switch ( iFallDamage )
	{
	case 1://progressive
		pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
		return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
		break;
	default:
	case 0:// fixed
		return 10;
		break;
	case -1:
		return 0; // fall damage disabled
	}
} 

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
	if (!pAttacker) {
		return true;
	}

	bool isOtherPlayer = pAttacker->IsPlayer() && pAttacker->entindex() != pPlayer->entindex();
	return !isOtherPlayer || friendlyfire.value != 0 || pPlayer->m_allowFriendlyFire;
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: PlayerThink( CBasePlayer *pPlayer )
{
	if ( g_fGameOver )
	{
		// clear attack/use commands from player
		pPlayer->m_afButtonPressed = 0;
		pPlayer->pev->button = 0;
		pPlayer->m_afButtonReleased = 0;
	}
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay :: PlayerSpawn( CBasePlayer *pPlayer )
{
	BOOL		addDefault;
	CBaseEntity	*pWeaponEntity = NULL;

	if (!g_noSuit)
		pPlayer->pev->weapons |= (1<<WEAPON_SUIT);

	addDefault = !g_mapCfgExists;

	for (int i = 0; i < MAX_EQUIP; i++) {
		if (!g_mapEquipment[i].itemName) {
			break;
		}
		equipPlayerWithItem(pPlayer, STRING(g_mapEquipment[i].itemName), g_mapEquipment[i].count);
	}

	while ( (pWeaponEntity = UTIL_FindEntityByClassname( pWeaponEntity, "game_player_equip" )) != 0)
	{
		pWeaponEntity->Touch( pPlayer );
		addDefault = FALSE;
	}

	if ( addDefault )
	{
		pPlayer->GiveNamedItem( "weapon_crowbar" );
		pPlayer->GiveNamedItem( "weapon_9mmhandgun" );
		pPlayer->GiveAmmo( 68, "9mm", gSkillData.sk_ammo_max_9mm );// 4 full reloads
	}

	if (mp_default_medkit.value && !g_noMedkit) {
		pPlayer->GiveNamedItem("weapon_medkit");
	}

	pPlayer->m_rgAmmo[pPlayer->GetAmmoIndex("health")] = gSkillData.sk_plr_medkit_start_ammo;
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay :: FPlayerCanRespawn( CBasePlayer *pPlayer )
{
	return TRUE;
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay :: FlPlayerSpawnTime( CBasePlayer *pPlayer )
{
	return gpGlobals->time;//now!
}

BOOL CHalfLifeMultiplay :: AllowAutoTargetCrosshair( void )
{
	return ( aimcrosshair.value != 0 );
}

//=========================================================
// IPointsForKill - how many points awarded to anyone
// that kills this player?
//=========================================================
int CHalfLifeMultiplay :: IPointsForKill( CBasePlayer *pAttacker, CBasePlayer *pKilled )
{
	return 1;
}


//=========================================================
// PlayerKilled - someone/something killed this player
//=========================================================
void CHalfLifeMultiplay :: PlayerKilled( CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor )
{
	pVictim->m_deathMessageSent = true;

	FireTargets( "game_playerdie", pVictim, pVictim, USE_TOGGLE, 0 );
	CBasePlayer *peKiller = NULL;
	CBaseEntity *ktmp = CBaseEntity::Instance( pKiller );
	if ( ktmp && (ktmp->Classify() == CLASS_PLAYER) )
		peKiller = (CBasePlayer*)ktmp;

	if ( pVictim->pev == pKiller )  
	{  // killed self, only penalize if PvP is enabled (npcs don't care that you stole their point)
		if (mp_score_mode.value == 0 || friendlyfire.value == 1)
			pKiller->frags -= 1;
	}
	else if ( ktmp && ktmp->IsPlayer() )
	{
		// if a player dies in a deathmatch game and the killer is a client, award the killer some points
		pKiller->frags += IPointsForKill( peKiller, pVictim );
		
		FireTargets( "game_playerkill", ktmp, ktmp, USE_TOGGLE, 0 );
	}
	else
	{  // killed by the world
		if (mp_score_mode.value == 0 || friendlyfire.value == 1)
			pKiller->frags -= 1;
	}

	// update the scores
	// killed scores
	pVictim->UpdateTeamInfo();

	// killers score, if it's a player
	CBaseEntity *ep = CBaseEntity::Instance( pKiller );
	if ( ep && ep->Classify() == CLASS_PLAYER )
	{
		CBasePlayer *PK = (CBasePlayer*)ep;

		PK->UpdateTeamInfo();

		// let the killer paint another decal as soon as he'd like.
		PK->m_flNextDecalTime = gpGlobals->time;
	}
#ifndef HLDEMO_BUILD
	if ( pVictim->HasNamedPlayerItem("weapon_satchel") )
	{
		DeactivateSatchels( pVictim );
	}
#endif
}

//=========================================================
// Deathnotice. 
//=========================================================
void CHalfLifeMultiplay::DeathNotice( CBaseMonster *pVictim, entvars_t *pKiller, entvars_t *pevInflictor )
{
	if (!pVictim->IsPlayer() && !(pKiller->flags & FL_CLIENT) && mp_killfeed.value < 3) {
		return; // monsters killing other monsters
	}

	if (!mp_killfeed.value || (mp_killfeed.value < 2 && !pVictim->IsPlayer())) {
		return; // players killing monsters
	}

	if (!strcmp(STRING(pVictim->pev->classname), "hornet") 
		|| !strcmp(STRING(pVictim->pev->classname), "monster_cockroach")
		|| !strcmp(STRING(pVictim->pev->classname), "monster_leech")
		|| !strcmp(STRING(pVictim->pev->classname), "monster_snark")
		|| !strcmp(STRING(pVictim->pev->classname), "monster_tripmine")) {
		return; // not worth a kill message
	}

	// Work out what killed the player, and send a message to all clients about it
	CBaseEntity *Killer = CBaseEntity::Instance( pKiller );

	if (!Killer || !pVictim) {
		return;
	}

	const char *killer_weapon_name = "world";		// by default, the player is killed by the world
	int killer_index = 0;
	int victim_index = pVictim->entindex();
	
	// Hack to fix name change
	const char *tau = "tau_cannon";
	const char *gluon = "gluon gun";

	CBaseEntity* Inflictor = CBaseEntity::Instance(pevInflictor);
	if (Inflictor)
		killer_weapon_name = Inflictor->GetDeathNoticeWeapon();

	if ( pKiller->flags & FL_CLIENT )
	{
		killer_index = ENTINDEX(ENT(pKiller));
		
		if ( pevInflictor && pevInflictor == pKiller)
		{
			// If the inflictor is the killer,  then it must be their current weapon doing the damage
			CBasePlayer *pPlayer = (CBasePlayer*)Killer;
				
			if ( pPlayer->m_pActiveItem )
			{
				killer_weapon_name = ((CBasePlayerItem*)pPlayer->m_pActiveItem.GetEntity())->GetDeathNoticeWeapon();
			}
		}

		if (pVictim->m_lastDamageType == DMG_FALL) {
			killer_weapon_name = "skull";
		}
	}

	// strip the monster_* or weapon_* from the inflictor's classname
	if ( strncmp( killer_weapon_name, "weapon_", 7 ) == 0 )
		killer_weapon_name += 7;
	else if ( strncmp( killer_weapon_name, "monster_", 8 ) == 0 )
		killer_weapon_name += 8;
	else if ( strncmp( killer_weapon_name, "func_", 5 ) == 0 )
		killer_weapon_name += 5;

	// HACK: quickly replace another player's name to the monster/damage that killed the player
	// so that the kill feed can show a killer name
	CBasePlayer* hackedPlayer1 = NULL;
	CBasePlayer* hackedPlayer2 = NULL;
	const char* hackedKillerOriginalName = NULL;
	const char* hackedVictimOriginalName = NULL;
	const char* originalKillerName = NULL;
	bool monsterKill = !Killer->IsPlayer() || !pVictim->IsPlayer();
	bool monsterKillingPlayer = !Killer->IsPlayer() && pVictim->IsPlayer();
	bool playerKillingMonster = Killer->IsPlayer() && !pVictim->IsPlayer();
	bool worldKillingPlayer = !Killer->IsMonster() && pVictim->IsPlayer();
	bool worldKillingMonster = !Killer->IsMonster() && !pVictim->IsPlayer();
	bool monsterKillingMonster = !Killer->IsPlayer() && !pVictim->IsPlayer();

	if (monsterKill) {
		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(i);

			if (pPlayer && pPlayer != pVictim && pPlayer != Killer)
			{
				hackedPlayer1 = pPlayer;
				break;
			}
		}

		if (monsterKillingMonster) {
			for (int i = 1; i <= gpGlobals->maxClients; i++) {
				CBasePlayer* pPlayer = (CBasePlayer*)UTIL_PlayerByIndex(i);

				if (pPlayer && pPlayer != hackedPlayer1)
				{
					hackedPlayer2 = pPlayer;
					break;
				}
			}
		}
	}

	if (hackedPlayer1) {
		hackedKillerOriginalName = STRING(hackedPlayer1->pev->netname);

		if (hackedPlayer2)
			hackedVictimOriginalName = STRING(hackedPlayer2->pev->netname);

		if (Killer->IsPlayer()) {
			victim_index = hackedPlayer1->entindex();
		}
		else {
			killer_index = hackedPlayer1->entindex();
		}

		if (monsterKillingMonster) {
			victim_index = hackedPlayer1->entindex();
			killer_index = hackedPlayer2 ? hackedPlayer2->entindex() : 60;
			if (pVictim->entindex() == Killer->entindex()) {
				killer_index = 60; // only show one name for suicide
				if (!strcmp(STRING(pVictim->pev->classname), "monster_shockroach")) {
					killer_weapon_name = "skull"; // roach dying of natural causes
				}
			}
		}
		
		if (worldKillingPlayer) {
			const char* killerName = Killer->DisplayName();

			if (pVictim->m_lastDamageType & DMG_FALL) {
				killerName = "Gravity";
			}
			else if (pVictim->m_lastDamageType & DMG_CRUSH) {
				//killerName = "Crushing force";
				// entity name will be a better description (door, elevator, etc.)
			}
			else if (pVictim->m_lastDamageType & DMG_SHOCK) {
				killerName = "Electricity";
			}
			else if (pVictim->m_lastDamageType & DMG_ENERGYBEAM) {
				killerName = "Laser";
			}
			else if (pVictim->m_lastDamageType & DMG_DROWN) {
				killerName = "Fluid";
			}
			else if (pVictim->m_lastDamageType & (DMG_POISON | DMG_NERVEGAS)) {
				killerName = "Poison";
			}
			else if (pVictim->m_lastDamageType & DMG_RADIATION) {
				killerName = "Radiation";
			}
			else if (pVictim->m_lastDamageType & DMG_ACID) {
				killerName = "Acid";
			}
			else if (pVictim->m_lastDamageType & (DMG_BURN | DMG_SLOWBURN)) {
				killerName = "Heat";
			}
			else if (pVictim->m_lastDamageType & (DMG_FREEZE | DMG_SLOWFREEZE)) {
				killerName = "Cold";
			}
			else if (pVictim->m_lastDamageType & DMG_MORTAR) {
				killerName = "Mortar";
			}
			else if (pVictim->m_lastDamageType & DMG_BLAST) {
				//killerName = "Explosion";
				// entity name should be better
			}
			else if (pVictim->m_lastDamageType & (DMG_PARALYZE)) {
				//killerName = "Nerve damage";
				// entity name should be better
			}
			else if (pVictim->m_lastDamageType & (DMG_SONIC)) {
				killerName = "Sound";
			}
			else if (pVictim->m_lastDamageType & (DMG_CLUB)) {
				killerName = "Club";
			}

			hackedPlayer1->Rename(killerName, true);
		}
		else {
			if (monsterKillingPlayer) {
				hackedPlayer1->Rename(Killer->DisplayName(), true);
			}
			else if (monsterKillingMonster) {
				if (hackedPlayer2) {
					hackedPlayer1->Rename(pVictim->DisplayName(), true);
					hackedPlayer2->Rename(Killer->DisplayName(), true);
				}
				else {
					hackedPlayer1->Rename(pVictim->DisplayName(), true);
				}
			}
			else if (playerKillingMonster) {
				const char* otherAttacker = NULL;
				int attackerCount = 1;

				if (mp_killfeed.value >= 2) {
					int attackerId = g_engfuncs.pfnGetPlayerUserId(Killer->edict());

					for (int i = 0; i < 32; i++) {
						PlayerAttackInfo& info = pVictim->m_attackers[i];

						if (info.userid && info.userid != attackerId) {
							attackerCount++;
							if (!otherAttacker) {
								CBasePlayer* plr = UTIL_PlayerByUserId(info.userid);
								if (plr)
									otherAttacker = plr->DisplayName();
							}
						}
					}
				}

				if (attackerCount > 1 && Killer->IsPlayer() && otherAttacker) {
					CBasePlayer* plr = (CBasePlayer*)Killer;
					originalKillerName = plr->DisplayName();

					static char killerName[40]; // max name length is 32 chars
					snprintf(killerName, 40, "%s", Killer->DisplayName());
					killerName[39] = 0;

					if (attackerCount == 2) {
						int killerNameLen = strlen(killerName);
						int assistNameLen = strlen(otherAttacker);
						if (killerNameLen > 14 && (killerNameLen + assistNameLen + 3) > 31) {
							int cutoff = V_max(14, 31 - (assistNameLen + 3));
							killerName[cutoff] = '\0';
						}

						plr->Rename(UTIL_VarArgs("%s + %s", killerName, otherAttacker), true);
					}
					else {
						killerName[19] = '\0'; // leave room for the player count

						plr->Rename(UTIL_VarArgs("%s + %d players", killerName, attackerCount-1), true);
					}
				}

				hackedPlayer1->Rename(pVictim->DisplayName(), true);
			}
		}

		int monsterTeamColor = ENEMY_TEAM_COLOR;

		switch (Killer->IRelationship(pVictim->Classify(), Killer->Classify())) {
		case R_AL:
			monsterTeamColor = FRIEND_TEAM_COLOR;
			break;
		case R_FR:
		case R_NO:
			monsterTeamColor = NEUTRAL_TEAM_COLOR;
			break;
		default:
			break;
		}

		// change killer name color to match the type entity type
		if (monsterKillingMonster || worldKillingMonster) {
			int victimColor = ENEMY_TEAM_COLOR;
			int killerColor = ENEMY_TEAM_COLOR;

			switch (Killer->IRelationship(Killer->Classify(), CLASS_PLAYER)) {
			case R_AL:
				killerColor = FRIEND_TEAM_COLOR;
				break;
			case R_FR:
			case R_NO:
				killerColor = NEUTRAL_TEAM_COLOR;
				break;
			default:
				break;
			}

			switch (pVictim->IRelationship(pVictim->Classify(), CLASS_PLAYER)) {
			case R_AL:
				victimColor = FRIEND_TEAM_COLOR;
				break;
			case R_FR:
			case R_NO:
				victimColor = NEUTRAL_TEAM_COLOR;
				break;
			default:
				break;
			}

			hackedPlayer1->UpdateTeamInfo(victimColor);
			if (hackedPlayer2)
				hackedPlayer2->UpdateTeamInfo(worldKillingMonster ? NEUTRAL_TEAM_COLOR : killerColor);
		}
		else {
			hackedPlayer1->UpdateTeamInfo(worldKillingPlayer ? NEUTRAL_TEAM_COLOR : monsterTeamColor);
		}
	}

	// restore player names if temporarily changed for the status bar, unless renamed just now
	// or else the wrong name shows in the kill feed
	if (pVictim->IsPlayer()) {
		CBasePlayer* plr = (CBasePlayer*)pVictim;
		if (plr->tempNameActive && plr != hackedPlayer1 && plr != hackedPlayer2) {
			plr->Rename(STRING(plr->pev->netname), true, MSG_ONE, plr->edict());
			plr->UpdateTeamInfo(plr->GetNameColor(), MSG_ONE, plr->edict());
		}
	}
	if (Killer->IsPlayer()) {
		CBasePlayer* plr = (CBasePlayer*)Killer;
		if (plr->tempNameActive && plr != hackedPlayer1 && plr != hackedPlayer2) {
			if (!originalKillerName)
				plr->Rename(STRING(plr->pev->netname), true, MSG_ONE, plr->edict());
			plr->UpdateTeamInfo(plr->GetNameColor(), MSG_ONE, plr->edict());
		}
	}

	// in case the player is alone, don't send invalid indexes because it can crash clients
	killer_index = V_min(killer_index, gpGlobals->maxClients-1);
	victim_index = V_min(victim_index, gpGlobals->maxClients-1);

	// client crash if the killer name is too long
	static char shortened_killer_name[30]; // client only has 32 char buffer which prepends "d_"
	strcpy_safe(shortened_killer_name, killer_weapon_name, 30);

	MESSAGE_BEGIN( MSG_ALL, gmsgDeathMsg );
		WRITE_BYTE( killer_index );				// the killer
		WRITE_BYTE(victim_index);				// the victim
		WRITE_STRING(shortened_killer_name);	// what they were killed by (should this be a string?)
	MESSAGE_END();

	// restore player name and team info
	if (hackedPlayer1) {
		hackedPlayer1->Rename(hackedKillerOriginalName, false);
		hackedPlayer1->UpdateTeamInfo(hackedPlayer1->GetNameColor());
		
		// TODO: this is sending the hacked player double the network packets, and these are already heavy
		if (hackedPlayer1->tempNameActive) {
			hackedPlayer1->Rename(hackedPlayer1->m_tempName, false, MSG_ONE, hackedPlayer1->edict());
			hackedPlayer1->UpdateTeamInfo(hackedPlayer1->m_tempTeam, MSG_ONE, hackedPlayer1->edict());
		}
	}
	if (originalKillerName) {
		CBasePlayer* plr = (CBasePlayer*)Killer;
		plr->Rename(originalKillerName, false);
		plr->UpdateTeamInfo(plr->GetNameColor()); // forces name on client to update NOW
	}
	if (hackedPlayer2) {
		hackedPlayer2->Rename(hackedVictimOriginalName, false);
		hackedPlayer2->UpdateTeamInfo(hackedPlayer2->GetNameColor());

		if (hackedPlayer2->tempNameActive) {
			hackedPlayer2->Rename(hackedPlayer2->m_tempName, false, MSG_ONE, hackedPlayer2->edict());
			hackedPlayer2->UpdateTeamInfo(hackedPlayer2->m_tempTeam, MSG_ONE, hackedPlayer2->edict());
		}
	}

	// back to the temp name, if one exists, so that the status bar renders correctly after a kill
	if (pVictim->IsPlayer()) {
		CBasePlayer* plr = (CBasePlayer*)pVictim;
		if (plr->tempNameActive && plr != hackedPlayer1 && plr != hackedPlayer2) {
			plr->Rename(plr->m_tempName, false, MSG_ONE, plr->edict());
			plr->UpdateTeamInfo(plr->m_tempTeam, MSG_ONE, plr->edict());
		}
	}
	if (Killer->IsPlayer()) {
		CBasePlayer* plr = (CBasePlayer*)Killer;
		if (plr->tempNameActive && plr != hackedPlayer1 && plr != hackedPlayer2) {
			plr->Rename(plr->m_tempName, false, MSG_ONE, plr->edict());
			plr->UpdateTeamInfo(plr->m_tempTeam, MSG_ONE, plr->edict());
		}
	}

	// replace the code names with the 'real' names
	if ( !strcmp( killer_weapon_name, "egon" ) )
		killer_weapon_name = gluon;
	else if ( !strcmp( killer_weapon_name, "gauss" ) )
		killer_weapon_name = tau;

	if (!monsterKillingMonster) {
		if (pVictim->pev == pKiller) {
			// killed self
			UTIL_LogPlayerEvent(pVictim->edict(), "committed suicide with \"%s\"\n", killer_weapon_name);
		}
		else if (pKiller->flags & FL_CLIENT) {
			// killed by other player
			if (pVictim->IsPlayer()) {
				UTIL_LogPlayerEvent(pVictim->edict(), "killed by \\%s\\%s\\ with \"%s\"\n",
					STRING(pKiller->netname),
					GETPLAYERAUTHID(ENT(pKiller)),
					killer_weapon_name);
			}
			else {
				UTIL_LogPlayerEvent(Killer->edict(), "killed \"%s\" with \"%s\"\n",
					pVictim->DisplayName(),
					killer_weapon_name);
			}
		}
		else {
			// killed by a monster or world
			UTIL_LogPlayerEvent(pVictim->edict(), "killed by \"%s\"\n",
				Killer ? Killer->DisplayName() : STRING(pKiller->classname));
		}
	}

	MESSAGE_BEGIN( MSG_SPEC, SVC_DIRECTOR );
		WRITE_BYTE ( 9 );	// command length in bytes
		WRITE_BYTE ( DRC_CMD_EVENT );	// player killed
		WRITE_SHORT( ENTINDEX(pVictim->edict()) );	// index number of primary entity
		if (pevInflictor)
			WRITE_SHORT( ENTINDEX(ENT(pevInflictor)) );	// index number of secondary entity
		else
			WRITE_SHORT( ENTINDEX(ENT(pKiller)) );	// index number of secondary entity
		WRITE_LONG( 7 | DRC_FLAG_DRAMATIC);   // eventflags (priority and flags)
	MESSAGE_END();

//  Print a standard message
	// TODO: make this go direct to console
	return; // just remove for now
}

//=========================================================
// PlayerGotWeapon - player has grabbed a weapon that was
// sitting in the world
//=========================================================
void CHalfLifeMultiplay :: PlayerGotWeapon( CBasePlayer *pPlayer, CBasePlayerItem *pWeapon )
{
}

//=========================================================
// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?
//=========================================================
float CHalfLifeMultiplay :: FlWeaponRespawnTime( CBasePlayerItem *pWeapon )
{
	if (pWeapon->m_flCustomRespawnTime) {
		return gpGlobals->time + pWeapon->m_flCustomRespawnTime;
	}

	if ( weaponstay.value > 0 )
	{
		// make sure it's only certain weapons
		if ( !(pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD) )
		{
			return gpGlobals->time + 0;		// weapon respawns almost instantly
		}
	}

	return gpGlobals->time + WEAPON_RESPAWN_TIME;
}

// when we are within this close to running out of entities,  items 
// marked with the ITEM_FLAG_LIMITINWORLD will delay their respawn
#define ENTITY_INTOLERANCE	100

//=========================================================
// FlWeaponRespawnTime - Returns 0 if the weapon can respawn 
// now,  otherwise it returns the time at which it can try
// to spawn again.
//=========================================================
float CHalfLifeMultiplay :: FlWeaponTryRespawn( CBasePlayerItem *pWeapon )
{
	if ( pWeapon && pWeapon->m_iId && (pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD) )
	{
		if ( NUMBER_OF_ENTITIES() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE) )
			return 0;

		// we're past the entity tolerance level,  so delay the respawn
		return FlWeaponRespawnTime( pWeapon );
	}

	return 0;
}

//=========================================================
// VecWeaponRespawnSpot - where should this weapon spawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeMultiplay :: VecWeaponRespawnSpot( CBasePlayerItem *pWeapon )
{
	return pWeapon->pev->origin;
}

//=========================================================
// WeaponShouldRespawn - any conditions inhibiting the
// respawning of this weapon?
//=========================================================
int CHalfLifeMultiplay :: WeaponShouldRespawn( CBasePlayerItem *pWeapon )
{
	if ( pWeapon->pev->spawnflags & SF_NORESPAWN )
	{
		return GR_WEAPON_RESPAWN_NO;
	}

	return GR_WEAPON_RESPAWN_YES;
}

//=========================================================
// CanHaveWeapon - returns FALSE if the player is not allowed
// to pick up this weapon
//=========================================================
BOOL CHalfLifeMultiplay::CanHavePlayerItem( CBasePlayer *pPlayer, CBasePlayerItem *pItem )
{
	if ( weaponstay.value > 0 )
	{
		if ( (pItem->iFlags() & ITEM_FLAG_LIMITINWORLD) || (pItem->pev->spawnflags & SF_NORESPAWN))
			return CGameRules::CanHavePlayerItem( pPlayer, pItem );

		// check if the player already has this weapon
		for ( int i = 0 ; i < MAX_ITEM_TYPES ; i++ )
		{
			CBasePlayerItem *it = (CBasePlayerItem*)pPlayer->m_rgpPlayerItems[i].GetEntity();

			while ( it != NULL )
			{
				if ( it->m_iId == pItem->m_iId )
				{
					return FALSE;
				}

				it = (CBasePlayerItem*)it->m_pNext.GetEntity();
			}
		}
	}

	return CGameRules::CanHavePlayerItem( pPlayer, pItem );
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::CanHaveItem( CBasePlayer *pPlayer, CItem *pItem )
{
	return TRUE;
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay::PlayerGotItem( CBasePlayer *pPlayer, CItem *pItem )
{
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::ItemShouldRespawn( CItem *pItem )
{
	if ( pItem->pev->spawnflags & SF_NORESPAWN )
	{
		return GR_ITEM_RESPAWN_NO;
	}

	return GR_ITEM_RESPAWN_YES;
}


//=========================================================
// At what time in the future may this Item respawn?
//=========================================================
float CHalfLifeMultiplay::FlItemRespawnTime( CItem *pItem )
{
	float respawnTime = pItem->m_flCustomRespawnTime ? pItem->m_flCustomRespawnTime : ITEM_RESPAWN_TIME;
	return gpGlobals->time + respawnTime;
}

//=========================================================
// Where should this item respawn?
// Some game variations may choose to randomize spawn locations
//=========================================================
Vector CHalfLifeMultiplay::VecItemRespawnSpot( CItem *pItem )
{
	return pItem->pev->origin;
}

//=========================================================
//=========================================================
void CHalfLifeMultiplay::PlayerGotAmmo( CBasePlayer *pPlayer, char *szName, int iCount )
{
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay::IsAllowedToSpawn( CBaseEntity *pEntity )
{
//	if ( pEntity->pev->flags & FL_MONSTER )
//		return FALSE;

	return TRUE;
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::AmmoShouldRespawn( CBasePlayerAmmo *pAmmo )
{
	if ( pAmmo->pev->spawnflags & SF_NORESPAWN )
	{
		return GR_AMMO_RESPAWN_NO;
	}

	return GR_AMMO_RESPAWN_YES;
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay::FlAmmoRespawnTime( CBasePlayerAmmo *pAmmo )
{
	float respawnTime = pAmmo->m_flCustomRespawnTime ? pAmmo->m_flCustomRespawnTime : AMMO_RESPAWN_TIME;
	return gpGlobals->time + respawnTime;
}

//=========================================================
//=========================================================
Vector CHalfLifeMultiplay::VecAmmoRespawnSpot( CBasePlayerAmmo *pAmmo )
{
	return pAmmo->pev->origin;
}

//=========================================================
//=========================================================
float CHalfLifeMultiplay::FlHealthChargerRechargeTime( void )
{
	return 60;
}


float CHalfLifeMultiplay::FlHEVChargerRechargeTime( void )
{
	return 30;
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::DeadPlayerWeapons( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_GUN_ACTIVE;
}

//=========================================================
//=========================================================
int CHalfLifeMultiplay::DeadPlayerAmmo( CBasePlayer *pPlayer )
{
	return GR_PLR_DROP_AMMO_ACTIVE;
}

edict_t *CHalfLifeMultiplay::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	edict_t *pentSpawnSpot = CGameRules::GetPlayerSpawnSpot( pPlayer );	
	return pentSpawnSpot;
}


//=========================================================
//=========================================================
int CHalfLifeMultiplay::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	// half life deathmatch has only enemies
	return GR_NOTTEAMMATE;
}

BOOL CHalfLifeMultiplay :: PlayFootstepSounds( CBasePlayer *pl, float fvol )
{
	if ( g_footsteps && g_footsteps->value == 0 )
		return FALSE;

	if ( pl->IsOnLadder() || pl->pev->velocity.Length2D() > 220 )
		return TRUE;  // only make step sounds in multiplayer if the player is moving fast enough

	return FALSE;
}

BOOL CHalfLifeMultiplay :: FAllowFlashlight( void ) 
{ 
	return flashlight.value != 0; 
}

//=========================================================
//=========================================================
BOOL CHalfLifeMultiplay :: FAllowMonsters( void )
{
	return ( allowmonsters.value != 0 );
}

//=========================================================
//======== CHalfLifeMultiplay private functions ===========
#define INTERMISSION_TIME		6

void CHalfLifeMultiplay :: GoToIntermission( void )
{
	if ( g_fGameOver )
		return;  // intermission has already been triggered, so ignore.

	// undo status bar name changes so scoreboard looks correct
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* pEnt = UTIL_PlayerByIndex(i);

		if (pEnt && pEnt->tempNameActive) {
			pEnt->Rename(pEnt->DisplayName(), false, MSG_ONE, pEnt->edict());
			pEnt->UpdateTeamInfo(-1, MSG_ONE, pEnt->edict());
		}
	}

	// bounds check
	int time = (int)CVAR_GET_FLOAT( "mp_chattime" );
	if ( time < 1 )
		CVAR_SET_STRING( "mp_chattime", "1" );
	else if ( time > MAX_INTERMISSION_TIME )
		CVAR_SET_STRING( "mp_chattime", UTIL_dtos1( MAX_INTERMISSION_TIME ) );

	m_flIntermissionEndTime = gpGlobals->time + ( (int)mp_chattime.value );
	g_flIntermissionStartTime = gpGlobals->time;

	g_fGameOver = TRUE;

	if (mp_series_intermission.value > 0) {
		const char* nextmapname = CVAR_GET_STRING("mp_nextmap");
		mapcycle_item_t* currentMap = g_pGameRules->GetMapCyleMap(STRING(gpGlobals->mapname));
		mapcycle_item_t* nextMap = g_pGameRules->GetMapCyleMap(nextmapname);
		bool nextMapSameSeries = currentMap && nextMap && currentMap->seriesNum == nextMap->seriesNum;

		if (nextMapSameSeries) {
			// the next map is part of the same series, so the "map" isn't really over yet.
			// Not all map series use trigger_changelevel to instantly change maps. Some use game_end 
			// and assume you set up the cycle correctly.

			if (mp_series_intermission.value == 1) {
				// end instantly, as if this were a trigger_changelevel
				m_flIntermissionEndTime = gpGlobals->time;
			}
			else {
				// end almost instantly, but long enough to send these last couple of messages.
				// Using the unreliable channel means these messages won't always be received
				// but it's faster than using reliable channel, which might also not show
				// if its backed up with lots of data.
				MESSAGE_BEGIN(MSG_BROADCAST, SVC_INTERMISSION);
				MESSAGE_END();

				MESSAGE_BEGIN(MSG_BROADCAST, gmsgSayText);
				WRITE_BYTE(0); // not a player
				WRITE_STRING(UTIL_VarArgs("Loading %s...\n", nextmapname));
				MESSAGE_END();

				m_flIntermissionEndTime = gpGlobals->time + 0.05f;
			}

			return;
		}
	}

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();
}



/*
==============
DestroyMapCycle

Clean up memory used by mapcycle when switching it
==============
*/
void DestroyMapCycle( mapcycle_t *cycle )
{
	mapcycle_item_t *p, *n, *start;
	p = cycle->items;
	if ( p )
	{
		start = p;
		p = p->next;
		while ( p != start )
		{
			n = p->next;
			delete p;
			p = n;
		}
		
		delete cycle->items;
	}
	cycle->items = NULL;
}

static char com_token[ 1500 ];

/*
==============
COM_Parse

Parse a token out of a string
==============
*/
char *COM_Parse (char *data)
{
	int             c;
	int             len;
	
	len = 0;
	com_token[0] = 0;
	
	if (!data)
		return NULL;
		
// skip whitespace
skipwhite:
	while ( (c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;                    // end of file;
		data++;
	}
	
// skip // comments
	if (c=='/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;
		goto skipwhite;
	}
	

// handle quoted strings specially
	if (c == '\"')
	{
		data++;
		while (1)
		{
			c = *data++;
			if (c=='\"' || !c)
			{
				com_token[len] = 0;
				return data;
			}
			com_token[len] = c;
			len++;
		}
	}

// parse single characters
	if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c == ',' )
	{
		com_token[len] = c;
		len++;
		com_token[len] = 0;
		return data+1;
	}

// parse a regular word
	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;
	if (c=='{' || c=='}'|| c==')'|| c=='(' || c=='\'' || c == ',' )
			break;
	} while (c>32);
	
	com_token[len] = 0;
	return data;
}

/*
==============
COM_TokenWaiting

Returns 1 if additional data is waiting to be processed on this line
==============
*/
int COM_TokenWaiting( char *buffer )
{
	char *p;

	p = buffer;
	while ( *p && *p!='\n')
	{
		if ( !isspace( *p ) || isalnum( *p ) )
			return 1;

		p++;
	}

	return 0;
}



/*
==============
ReloadMapCycleFile


Parses mapcycle.txt file into mapcycle_t structure
==============
*/
int ReloadMapCycleFile( char *filename, mapcycle_t *cycle )
{
	const int MAX_MAP_NAME_LEN = 32;
	char szMap[MAX_MAP_NAME_LEN];
	int length;
	char *pFileList;
	char *aFileList = pFileList = (char*)LOAD_FILE_FOR_ME( filename, &length );
	mapcycle_item_t *item, *newlist = NULL, *next;

	if ( pFileList && length )
	{
		int seriesIdx = 0;
		int seriesNum = 0;

		while (pFileList)
		{
			if (seriesIdx != 0 && !COM_TokenWaiting(pFileList)) {
				seriesIdx = 0; // end of line
				seriesNum++;
			}

			pFileList = COM_Parse(pFileList);

			if (strlen(com_token) <= 0) {
				seriesIdx = 0;
				seriesNum++;
				continue;
			}

			strncpy(szMap, com_token, MAX_MAP_NAME_LEN);
			szMap[MAX_MAP_NAME_LEN - 1] = 0;

			if (!IS_MAP_VALID(szMap)) {
				ALERT(at_console, "Skipping %s from mapcycle, not a valid map\n", szMap);
				continue;
			}

			// Create entry
			item = new mapcycle_item_t;
			strcpy_safe(item->mapname, szMap, 32);
			item->mapname[31] = 0;
			item->seriesIdx = seriesIdx++;
			item->seriesNum = seriesNum;
			item->next = cycle->items;
			cycle->items = item;
		}

		FREE_FILE( aFileList );
	}

	// Fixup circular list pointer
	item = cycle->items;

	// Reverse it to get original order
	while ( item )
	{
		next = item->next;
		item->next = newlist;
		newlist = item;
		item = next;
	}
	cycle->items = newlist;
	item = cycle->items;

	// Didn't parse anything
	if ( !item )
	{
		return 0;
	}

	while ( item->next )
	{
		item = item->next;
	}
	item->next = cycle->items;

	return 1;
}

/*
==============
CountPlayers

Determine the current # of active players on the server for map cycling logic
==============
*/
int CountPlayers( void )
{
	int	num = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex( i );

		if ( pEnt )
		{
			num = num + 1;
		}
	}

	return num;
}

//=========================================================
// Returns whatever survival mode is enabled
//=========================================================
BOOL CHalfLifeMultiplay::SurvivalModeEnabled( void )
{
	return (int)( mp_survival_supported.value + mp_survival_starton.value ) == 2;
}

//=========================================================
// Returns whatever a player can respawn if survival mode is enabled
//=========================================================
BOOL CHalfLifeMultiplay::SurvivalModeCanSpawn( CBasePlayer* pPlayer )
{
	if( !SurvivalModeEnabled() )
		return TRUE;

	std::string szID = std::string( GETPLAYERAUTHID( pPlayer->edict() ) );

	if( SurvivalPlayerData[ szID ] )
		return FALSE;

	CLIENT_PRINTF( pPlayer->edict(), print_chat, "Survival Mode is enabled, no more respawning allowed." );

	SurvivalPlayerData[ szID ] = true;

	return TRUE;
}

//=========================================================
// Reload the server if mp_survival_restart == 1, else just re-spawn all players
//=========================================================
void CHalfLifeMultiplay::SurvivalModeThink()
{
	if( (int)mp_survival_supported.value != 1 || CountPlayers() == 0 )
		return;

	int iAlivePlayers = 0;

	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CBaseEntity* pPlayer = UTIL_PlayerByIndex( i );

		if( !pPlayer || pPlayer == nullptr )
			continue;

		if( !SurvivalModeEnabled() )
		{
			SurvivalPlayerData[ std::string( GETPLAYERAUTHID( pPlayer->edict() ) ) ] = false;
		}
		else
		{
			iAlivePlayers++;
		}
	}

	if( SurvivalModeEnabled() && iAlivePlayers == 0 )
	{
		SurvivalPlayerData.clear();

		if( (int)mp_survival_restart.value == 1 )
		{
			SERVER_COMMAND( "restart\n" );
		}
	}
}

/*
==============
ExtractCommandString

Parse commands/key value pairs to issue right after map xxx command is issued on server
 level transition
==============
*/
void ExtractCommandString( char *s, char *szCommand )
{
	// Now make rules happen
	char	pkey[512];
	char	value[512];	// use two buffers so compares
								// work without stomping on each other
	char	*o;
	
	if ( *s == '\\' )
		s++;

	while (1)
	{
		o = pkey;
		while ( *s != '\\' )
		{
			if ( !*s )
				return;
			*o++ = *s++;
		}
		*o = 0;
		s++;

		o = value;

		while (*s != '\\' && *s)
		{
			if (!*s)
				return;
			*o++ = *s++;
		}
		*o = 0;

		strcat( szCommand, pkey );
		if ( strlen( value ) > 0 )
		{
			strcat( szCommand, " " );
			strcat( szCommand, value );
		}
		strcat( szCommand, "\n" );

		if (!*s)
			return;
		s++;
	}
}

mapcycle_item_t* CHalfLifeMultiplay::GetMapCyleMap(const char* current_map) {
	if (!mapcycle.items && !ReloadMapCycleFile((char*)CVAR_GET_STRING("mapcyclefile"), &mapcycle)) {
		return NULL;
	}

	mapcycle_item_t* item;

	// Traverse list
	int i = 0;
	for (item = mapcycle.items; item != mapcycle.items || i == 0; item = item->next, i++)
	{
		ASSERT(item != NULL);

		if (item && !strcmp(item->mapname, current_map)) {
			if (!IS_MAP_VALID(item->mapname)) {
				ALERT(at_error, "Invalid map in cycle: '%s'\n", item->next->mapname);
				continue;
			}
			return item;
		}

	}

	return NULL;
}

/*
==============
ChangeLevel

Server is changing to a new level, check mapcycle.txt for map name and setup info
==============
*/
void CHalfLifeMultiplay :: ChangeLevel( void )
{
	static char szPreviousMapCycleFile[ 256 ];
	static uint64_t lastMapCycleModifyTime = 0;

	BOOL do_cycle = TRUE;

	// find the map to change to
	char *mapcfile = (char*)CVAR_GET_STRING( "mapcyclefile" );
	ASSERT( mapcfile != NULL );

	bool shouldReloadMapCycle = lastMapCycleModifyTime == 0;

	// Has the map cycle filename changed?
	if (stricmp(mapcfile, szPreviousMapCycleFile)) {
		strcpy_safe( szPreviousMapCycleFile, mapcfile, 256 );
		shouldReloadMapCycle = true;
	}

	if (!shouldReloadMapCycle) {
		uint64_t modifyTime = getFileModifiedTime(getGameFilePath(mapcfile).c_str());
		if (modifyTime && modifyTime != lastMapCycleModifyTime) {
			if (lastMapCycleModifyTime) {
				ALERT(at_console, "Map cycle '%s' modified. Reloading.\n", mapcfile);
			}
			lastMapCycleModifyTime = modifyTime;
			shouldReloadMapCycle = true;
		}
	}
	
	if (shouldReloadMapCycle) {
		DestroyMapCycle(&mapcycle);

		if (!ReloadMapCycleFile(mapcfile, &mapcycle) || (!mapcycle.items))
		{
			ALERT(at_console, "Unable to load map cycle file %s\n", mapcfile);
			do_cycle = FALSE;
		}
	}

	// restart map if no cycle defined
	const char* current_map = STRING(gpGlobals->mapname);
	const char* next_map = STRING(gpGlobals->mapname);

	const char* nextmapcvar = CVAR_GET_STRING("mp_nextmap");
	bool nextMapCvarSet = strlen(nextmapcvar) > 0;

	if (!IS_MAP_VALID(nextmapcvar)) {
		ALERT(at_error, "Ignoring invalid mp_nextmap '%s'\n", nextmapcvar);
		nextMapCvarSet = false;
	}

	if (nextMapCvarSet) {
		next_map = nextmapcvar;
	}

	if ( do_cycle && mapcycle.items && !nextMapCvarSet )
	{
		mapcycle_item_t* item = GetMapCyleMap(current_map);

		if (item) {
			next_map = item->next->mapname;
		}
		else {
			next_map = mapcycle.items->mapname;
			ALERT(at_console, "Invalid map '%s' in map cycle file. Restarting the map cycle.\n", current_map);
		}
	}

	g_fGameOver = TRUE;
	m_flIntermissionEndTime = 0;

	ALERT( at_console, "CHANGE LEVEL: %s\n", next_map);
	
	CHANGE_LEVEL(next_map, NULL );
}

#define MAX_MOTD_CHUNK	  60
#define MAX_MOTD_LENGTH   1536 // (MAX_MOTD_CHUNK * 4)

void CHalfLifeMultiplay :: SendMOTDToClient( edict_t *client )
{
	// read from the MOTD.txt file
	int length, char_count = 0;
	char *pFileList;
	char *aFileList = pFileList = (char*)LOAD_FILE_FOR_ME( (char *)CVAR_GET_STRING( "motdfile" ), &length );

	// send the server name
	MESSAGE_BEGIN( MSG_ONE, gmsgServerName, NULL, client );
		WRITE_STRING( CVAR_GET_STRING("hostname") );
	MESSAGE_END();

	// Send the message of the day
	// read it chunk-by-chunk,  and send it in parts

	while ( pFileList && *pFileList && char_count < MAX_MOTD_LENGTH )
	{
		char chunk[MAX_MOTD_CHUNK+1];
		
		if ( strlen( pFileList ) < MAX_MOTD_CHUNK )
		{
			strcpy_safe( chunk, pFileList, MAX_MOTD_CHUNK);
		}
		else
		{
			strcpy_safe( chunk, pFileList, MAX_MOTD_CHUNK );
			chunk[MAX_MOTD_CHUNK] = 0;		// strncpy doesn't always append the null terminator
		}

		char_count += strlen( chunk );
		if ( char_count < MAX_MOTD_LENGTH )
			pFileList = aFileList + char_count; 
		else
			*pFileList = 0;

		MESSAGE_BEGIN( MSG_ONE, gmsgMOTD, NULL, client );
			WRITE_BYTE( *pFileList ? FALSE : TRUE );	// FALSE means there is still more message to come
			WRITE_STRING( chunk );
		MESSAGE_END();
	}

	FREE_FILE( aFileList );
}
	

int CHalfLifeMultiplay::GetTeamIndex(const char* pTeamName)
{
	return DEFAULT_TEAM_COLOR;
}

void CHalfLifeMultiplay::ClientUserInfoChanged(CBasePlayer* pPlayer, char* infobuffer)
{
	// Set preferences
	pPlayer->SetPrefsFromUserinfo(infobuffer);
}