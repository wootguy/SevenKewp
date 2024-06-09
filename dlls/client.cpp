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
// Robin, 4-22-98: Moved set_suicide_frame() here from player.cpp to allow us to 
//				   have one without a hardcoded player.mdl in tf_client.cpp

/*

===== client.cpp ========================================================

  client/server game specific stuff

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "CBasePlayer.h"
#include "CBaseSpectator.h"
#include "client.h"
#include "env/CSoundEnt.h"
#include "gamerules.h"
#include "game.h"
#include "customentity.h"
#include "weapons.h"
#include "weaponinfo.h"
#include "usercmd.h"
#include "netadr.h"
#include "pm_shared.h"
#include "CBasePlayerItem.h"
#include "CRpg.h"
#include "CMonsterMaker.h"
#include "skill.h"
#include "CGamePlayerEquip.h"
#include "PluginManager.h"

#if !defined ( _WIN32 )
#include <ctype.h>
#endif

extern DLL_GLOBAL ULONG		g_ulModelIndexPlayer;
extern DLL_GLOBAL BOOL		g_fGameOver;
extern DLL_GLOBAL int		g_iSkillLevel;
extern DLL_GLOBAL ULONG		g_ulFrameCount;

extern void CopyToBodyQue(entvars_t* pev);
extern int giPrecacheGrunt;
extern int gmsgSayText;
extern int g_teamplay;

// client index that is receiving AddFullToPack calls
int g_packClientIdx = 0;

void LinkUserMessages( void );

/*
 * used by kill command and disconnect command
 * ROBIN: Moved here from player.cpp, to allow multiple player models
 */
void set_suicide_frame(entvars_t* pev)
{       
	if (!FStrEq(STRING(pev->model), "models/player.mdl"))
		return; // allready gibbed

//	pev->frame		= $deatha11;
	pev->solid		= SOLID_NOT;
	pev->movetype	= MOVETYPE_TOSS;
	pev->deadflag	= DEAD_DEAD;
	pev->nextthink	= -1;
}


/*
===========
ClientConnect

called when a player connects to a server
============
*/
BOOL ClientConnect( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ]  )
{	
	return g_pGameRules->ClientConnected( pEntity, pszName, pszAddress, szRejectReason );

// a client connecting during an intermission can cause problems
//	if (intermission_running)
//		ExitIntermission ();

}


/*
===========
ClientDisconnect

called when a player disconnects from a server

GLOBALS ASSUMED SET:  g_fGameOver
============
*/
void ClientDisconnect( edict_t *pEntity )
{
	if (mp_debugmsg.value) {
		writeNetworkMessageHistory(std::string(STRING(pEntity->v.netname)) 
			+ " dropped on map " + STRING(gpGlobals->mapname));
	}

	if (g_fGameOver)
		return;

	/*
	char text[256] = "" ;
	if ( pEntity->v.netname )
		_snprintf( text, sizeof(text), "- %s has left the game\n", STRING(pEntity->v.netname) );
	text[ sizeof(text) - 1 ] = 0;
	MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
		WRITE_BYTE( ENTINDEX(pEntity) );
		WRITE_STRING( text );
	MESSAGE_END();
	*/

	CSound *pSound;
	pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( pEntity ) );
	{
		// since this client isn't around to think anymore, reset their sound. 
		if ( pSound )
		{
			pSound->Reset();
		}
	}

	edict_t* edicts = ENT(0);
	uint32_t plrbit = PLRBIT(pEntity);

	// remove visibility flags for all entities
	for (int i = 1; i < gpGlobals->maxEntities; i++)
	{
		if (edicts[i].free)
			continue;

		CBaseEntity* ent = (CBaseEntity*)GET_PRIVATE(&edicts[i]);
		if (ent) {
			ent->m_visiblePlayers &= ~plrbit;
			ent->m_audiblePlayers &= ~plrbit;
		}
	}

// since the edict doesn't get deleted, fix it so it doesn't interfere.
	pEntity->v.takedamage = DAMAGE_NO;// don't attract autoaim
	pEntity->v.solid = SOLID_NOT;// nonsolid
	UTIL_SetOrigin ( &pEntity->v, pEntity->v.origin );

	g_pGameRules->ClientDisconnected( pEntity );
}


// called by ClientKill and DeadThink
void respawn(entvars_t* pev, BOOL fCopyCorpse)
{
	CBasePlayer* plr = (CBasePlayer*)GET_PRIVATE(ENT(pev));
	edict_t* spawnSpot = g_pGameRules->GetPlayerSpawnSpot(plr);

	if( !g_pGameRules->SurvivalModeCanSpawn(plr) )
	{
		plr->StartObserver( plr->pev->origin, plr->pev->angles );
		return;
	}

	if (FNullEnt(spawnSpot)) {
		if (gpGlobals->time - plr->m_lastSpawnMessage > 0.5f) {
			CLIENT_PRINTF(plr->edict(), print_center, "No spawn points available");
			plr->m_lastSpawnMessage = gpGlobals->time;
		}
		return;
	}

	float deadTime = gpGlobals->time - plr->m_lastKillTime;
	if (deadTime < mp_respawndelay.value) {
		return;
	}

	if (gpGlobals->coop || gpGlobals->deathmatch)
	{
		if ( fCopyCorpse )
		{
			// make a copy of the dead body for appearances sake
			pev->effects &= ~EF_DIMLIGHT; // turn off flashlight
			CopyToBodyQue(pev);
		}

		// respawn player
		GetClassPtr( (CBasePlayer *)pev)->Spawn( );
	}
	else
	{       // restart the entire server
		SERVER_COMMAND("reload\n");
	}
}

/*
============
ClientKill

Player entered the suicide command

GLOBALS ASSUMED SET:  g_ulModelIndexPlayer
============
*/
void ClientKill( edict_t *pEntity )
{
	entvars_t *pev = &pEntity->v;

	CBasePlayer *pl = (CBasePlayer*) CBasePlayer::Instance( pev );

	if ( pl->m_fNextSuicideTime > gpGlobals->time )
		return;  // prevent suiciding too ofter

	pl->m_fNextSuicideTime = gpGlobals->time + 1;  // don't let them suicide for 5 seconds after suiciding

	// have the player kill themself
	pev->health = 0;
	pl->Killed( pev, GIB_NEVER );

//	pev->modelindex = g_ulModelIndexPlayer;
//	pev->frags -= 2;		// extra penalty
//	respawn( pev );
}

/*
===========
ClientPutInServer

called each time a player is spawned
============
*/
void ClientPutInServer( edict_t *pEntity )
{
	CBasePlayer *pPlayer;

	entvars_t *pev = &pEntity->v;

	pPlayer = GetClassPtr((CBasePlayer *)pev);
	pPlayer->SetCustomDecalFrames(-1); // Assume none;
	pPlayer->m_flLastSetRoomtype = -1; // fixup room type if joining from another server

	if (g_mp3Command.size()) {
		// start global music
		MESSAGE_BEGIN(MSG_ONE, SVC_STUFFTEXT, NULL, pEntity);
		WRITE_STRING(g_mp3Command.c_str());
		MESSAGE_END();
	}
	else {
		// stop any music from the previous map/server
		UTIL_StopGlobalMp3(pEntity);
	}

	// Allocate a CBasePlayer for pev, and call spawn
	pPlayer->Spawn();
	pPlayer->m_initSoundTime = gpGlobals->time + 1.0f;

	// Reset interpolation during first frame
	pPlayer->pev->effects |= EF_NOINTERP;

	pPlayer->pev->iuser1 = 0;	// disable any spec modes
	pPlayer->pev->iuser2 = 0; 
}

/*
========================
ClientUserInfoChanged

called after the player changes
userinfo - gives dll a chance to modify it before
it gets sent into the rest of the engine.
========================
*/
void ClientUserInfoChanged( edict_t *pEntity, char *infobuffer )
{
	// Is the client spawned yet?
	if ( !pEntity->pvPrivateData )
		return;

	// msg everyone if someone changes their name,  and it isn't the first time (changing no name to current name)
	if ( pEntity->v.netname && STRING(pEntity->v.netname)[0] != 0 && !FStrEq( STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" )) )
	{
		char sName[256];
		char *pName = g_engfuncs.pfnInfoKeyValue( infobuffer, "name" );
		strncpy( sName, pName, sizeof(sName) - 1 );
		sName[ sizeof(sName) - 1 ] = '\0';

		// First parse the name and remove any %'s
		for ( char *pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++ )
		{
			// Replace it with a space
			if ( *pApersand == '%' )
				*pApersand = ' ';
		}

		// Set the name
		g_engfuncs.pfnSetClientKeyValue( ENTINDEX(pEntity), infobuffer, "name", sName );

		if (gpGlobals->maxClients > 1)
		{
			char text[256];
			snprintf( text, 256, "* %s changed name to %s\n", STRING(pEntity->v.netname), g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
			MESSAGE_BEGIN( MSG_ALL, gmsgSayText, NULL );
				WRITE_BYTE( ENTINDEX(pEntity) );
				WRITE_STRING( text );
			MESSAGE_END();
		}

		// team match?
		if ( g_teamplay )
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" changed name to \"%s\"\n", 
				STRING( pEntity->v.netname ), 
				GETPLAYERUSERID( pEntity ), 
				GETPLAYERAUTHID( pEntity ),
				g_engfuncs.pfnInfoKeyValue( infobuffer, "model" ), 
				g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		}
		else
		{
			UTIL_LogPrintf( "\"%s<%i><%s><%i>\" changed name to \"%s\"\n", 
				STRING( pEntity->v.netname ), 
				GETPLAYERUSERID( pEntity ), 
				GETPLAYERAUTHID( pEntity ),
				GETPLAYERUSERID( pEntity ), 
				g_engfuncs.pfnInfoKeyValue( infobuffer, "name" ) );
		}
	}

	g_pGameRules->ClientUserInfoChanged( GetClassPtr((CBasePlayer *)&pEntity->v), infobuffer );
}

int g_serveractive = 0;
int g_edictsinit = 0;
bool g_monstersNerfed = false;

void ServerDeactivate( void )
{
	// It's possible that the engine will call this function more times than is necessary
	//  Therefore, only run it one time for each call to ServerActivate 
	if ( g_serveractive != 1 )
	{
		return;
	}

	g_serveractive = 0;
	g_edictsinit = 0;

	g_pluginManager.RemovePlugins(true);

	g_customKeyValues.clear();
	g_monsterSoundReplacements.clear();
	g_shuffledMonsterSounds.clear();
	g_bsp.delete_lumps();

	g_precachedGeneric.clear();
	g_precachedModels.clear();
	g_missingModels.clear();
	g_precachedSounds.clear();
	g_tryPrecacheGeneric.clear();
	g_tryPrecacheModels.clear();
	g_tryPrecacheSounds.clear();
	g_mapWeapons.clear();
	g_wavInfos.clear();
	g_weaponClassnames.clear();
	clearNetworkMessageHistory();
	g_mp3Command = "";
	g_monstersNerfed = false;
	g_cfgsExecuted = false;

	memset(&g_nerfStats, 0, sizeof(NerfStats));
	memset(&g_textureStats, 0, sizeof(TextureTypeStats));

	// Peform any shutdown operations here...
	//
}

#include "lagcomp.h"

void PrecacheWeapons() {
	if (g_mapWeapons.find("weapon_shotgun") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_shotgun");
		UTIL_PrecacheOther("ammo_buckshot");
	}
	if (g_mapWeapons.find("weapon_crowbar") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_crowbar");
	}
	if (g_mapWeapons.find("weapon_9mmhandgun") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_9mmhandgun");
		UTIL_PrecacheOther("ammo_9mmclip");
	}
	if (g_mapWeapons.find("weapon_9mmAR") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_9mmAR");
		UTIL_PrecacheOther("ammo_9mmAR");
		UTIL_PrecacheOther("ammo_ARgrenades");
	}
	if (g_mapWeapons.find("weapon_357") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_357");
		UTIL_PrecacheOther("ammo_357");
	}
	if (g_mapWeapons.find("weapon_gauss") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_gauss");
		UTIL_PrecacheOther("ammo_gaussclip");
	}
	if (g_mapWeapons.find("weapon_rpg") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_rpg");
		UTIL_PrecacheOther("ammo_rpgclip");
	}
	if (g_mapWeapons.find("weapon_crossbow") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_crossbow");
		UTIL_PrecacheOther("ammo_crossbow");
	}
	if (g_mapWeapons.find("weapon_egon") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_egon");
		UTIL_PrecacheOther("ammo_gaussclip");
	}
	if (g_mapWeapons.find("weapon_tripmine") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_tripmine");
	}
	if (g_mapWeapons.find("weapon_satchel") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_satchel");
	}
	if (g_mapWeapons.find("weapon_handgrenade") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_handgrenade");
	}
	if (g_mapWeapons.find("weapon_snark") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_snark");
	}
	if (g_mapWeapons.find("weapon_hornetgun") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_hornetgun");
	}
	if (g_mapWeapons.find("weapon_grapple") != g_mapWeapons.end()) {
		UTIL_PrecacheOther("weapon_grapple");
	}
}

void PrecacheTextureSounds() {
	if (g_textureStats.tex_concrete) {
		PRECACHE_SOUND_ENT(NULL, "player/pl_step1.wav");		// walk on concrete
		PRECACHE_SOUND_ENT(NULL, "player/pl_step2.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_step3.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_step4.wav");
	}

	if (g_textureStats.tex_metal) {
		PRECACHE_SOUND_ENT(NULL, "player/pl_metal1.wav");		// walk on metal
		PRECACHE_SOUND_ENT(NULL, "player/pl_metal2.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_metal3.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_metal4.wav");
	}

	if (g_textureStats.tex_dirt) {
		PRECACHE_SOUND_ENT(NULL, "player/pl_dirt1.wav");		// walk on dirt
		PRECACHE_SOUND_ENT(NULL, "player/pl_dirt2.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_dirt3.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_dirt4.wav");
	}

	if (g_textureStats.tex_duct) {
		PRECACHE_SOUND_ENT(NULL, "player/pl_duct1.wav");		// walk in duct
		PRECACHE_SOUND_ENT(NULL, "player/pl_duct2.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_duct3.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_duct4.wav");
	}

	if (g_textureStats.tex_grate) {
		PRECACHE_SOUND_ENT(NULL, "player/pl_grate1.wav");		// walk on grate
		PRECACHE_SOUND_ENT(NULL, "player/pl_grate2.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_grate3.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_grate4.wav");
	}

	if (g_textureStats.tex_tile) {
		PRECACHE_SOUND_ENT(NULL, "player/pl_tile1.wav");		// walk on tile
		PRECACHE_SOUND_ENT(NULL, "player/pl_tile2.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_tile3.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_tile4.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_tile5.wav");
	}

	if (g_textureStats.tex_wood) {
		PRECACHE_SOUND_ENT(NULL, "debris/wood1.wav");			// hit wood texture
		PRECACHE_SOUND_ENT(NULL, "debris/wood2.wav");
		PRECACHE_SOUND_ENT(NULL, "debris/wood3.wav");
	}

	if (g_textureStats.tex_computer) {
		PRECACHE_SOUND_ENT(NULL, "buttons/spark5.wav");		// hit computer texture
		PRECACHE_SOUND_ENT(NULL, "buttons/spark6.wav");
	}

	if (g_textureStats.tex_computer || g_textureStats.tex_glass) {
		PRECACHE_SOUND_ENT(NULL, "debris/glass1.wav");
		PRECACHE_SOUND_ENT(NULL, "debris/glass2.wav");
		PRECACHE_SOUND_ENT(NULL, "debris/glass3.wav");
	}

	if (g_textureStats.tex_water) {
		PRECACHE_SOUND_ENT(NULL, "player/pl_slosh1.wav");		// walk in shallow water
		PRECACHE_SOUND_ENT(NULL, "player/pl_slosh2.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_slosh3.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_slosh4.wav");

		PRECACHE_SOUND_ENT(NULL, "player/pl_swim1.wav");		// breathe bubbles
		PRECACHE_SOUND_ENT(NULL, "player/pl_swim2.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_swim3.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_swim4.wav");

		// Note: wade sounds are used by the engine for state transtitions
		PRECACHE_SOUND_ENT(NULL, "player/pl_wade1.wav");		// wade in water
		PRECACHE_SOUND_ENT(NULL, "player/pl_wade2.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_wade3.wav");
		PRECACHE_SOUND_ENT(NULL, "player/pl_wade4.wav");
	}
}

void ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
	int				i;
	CBaseEntity		*pClass;

	// Clients have not been initialized yet
	for ( i = 0; i < edictCount; i++ )
	{
		if ( pEdictList[i].free )
			continue;
		
		// Clients aren't necessarily initialized until ClientPutInServer()
		if ( i < clientMax || !pEdictList[i].pvPrivateData )
			continue;

		bool isBspModel = pEdictList[i].v.model && STRING(pEdictList[i].v.model)[0] == '*';
		bool isSwimmable = pEdictList[i].v.skin <= CONTENTS_WATER && pEdictList[i].v.skin > CONTENTS_TRANSLUCENT;
		if (isBspModel && isSwimmable) {
			g_textureStats.tex_water = true;
		}

		pClass = CBaseEntity::Instance( &pEdictList[i] );
		// Activate this entity if it's got a class & isn't dormant
		if ( pClass && !(pClass->pev->flags & FL_DORMANT) )
		{
			pClass->Activate();
		}
		else
		{
			ALERT( at_console, "Can't instance %s\n", STRING(pEdictList[i].v.classname) );
			continue;
		}

		// nerf monster health/spawners now that all entities have spawned (can check/update connections)
		CBaseMonster* monster = pClass->MyMonsterPointer();
		if (monster) {
			pClass->MyMonsterPointer()->Nerf();
		}
	}

	PrecacheWeapons();
	PrecacheTextureSounds();

	// Every call to ServerActivate should be matched by a call to ServerDeactivate
	g_serveractive = 1;

	// Link user messages here to make sure first client can get them...
	LinkUserMessages();

	const char* current_map = STRING(gpGlobals->mapname);
	mapcycle_item_t* cycleMap = g_pGameRules->GetMapCyleMap(current_map);

	if (cycleMap) {
		CVAR_SET_STRING("mp_nextmap", cycleMap->next->mapname);
	}
	else {
		if (g_pGameRules->mapcycle.items) {
			ALERT(at_console, "Map '%s' not in map cycle. Restarting map cycle.\n", current_map);
			CVAR_SET_STRING("mp_nextmap", g_pGameRules->mapcycle.items->mapname);
		}
		else {
			ALERT(at_console, "Map cycle empty. Clearning mp_nextmap.\n");
			CVAR_SET_STRING("mp_nextmap", "");
		}
	}

	PrintEntindexStats();

	g_engfuncs.pfnServerPrint(UTIL_VarArgs("Precache stats: %d models (%d MDL, %d BSP), %d sounds, %d generic, %d events\n",
		g_tryPrecacheModels.size() + g_bsp.modelCount, g_tryPrecacheModels.size(), g_bsp.modelCount, 
		g_tryPrecacheSounds.size(), g_tryPrecacheGeneric.size(), g_tryPrecacheEvents.size()));

	if (g_tryPrecacheModels.size() > g_precachedModels.size()) {
		ALERT(at_error, "Model precache overflow (%d / %d). The following models were not precached:\n",
			g_tryPrecacheModels.size() + g_bsp.modelCount, MAX_PRECACHE);

		for (std::string item : g_tryPrecacheModels) {
			if (!g_precachedModels.count(item)) {
				ALERT(at_console, "    %s\n", item.c_str());
			}
		}
	}
	if (g_tryPrecacheSounds.size() > g_precachedSounds.size()) {
		ALERT(at_error, "Sound precache overflow (%d / %d). The following sounds were not precached:\n",
			g_tryPrecacheSounds.size(), MAX_PRECACHE_SOUND);

		for (std::string item : g_tryPrecacheSounds) {
			if (!g_precachedSounds.count(item)) {
				g_engfuncs.pfnServerPrint(UTIL_VarArgs("    %s\n", item.c_str()));
			}
		}
	}
	if (g_tryPrecacheGeneric.size() > g_precachedGeneric.size()) {
		ALERT(at_error, "Generic precache overflow (%d / %d). The following resources were not precached:\n",
			g_tryPrecacheGeneric.size(), MAX_PRECACHE_MODEL);

		for (std::string item : g_tryPrecacheGeneric) {
			if (!g_precachedGeneric.count(item)) {
				g_engfuncs.pfnServerPrint(UTIL_VarArgs("    %s\n", item.c_str()));
			}
		}
	}
	if (g_tryPrecacheEvents.size() > g_precachedEvents.size()) {
		ALERT(at_error, "Event precache overflow (%d / %d). The following resources were not precached:\n",
			g_tryPrecacheEvents.size(), MAX_PRECACHE_EVENT);

		for (std::string item : g_tryPrecacheEvents) {
			if (!g_precachedEvents.count(item)) {
				g_engfuncs.pfnServerPrint(UTIL_VarArgs("    %s\n", item.c_str()));
			}
		}
	}

	g_pluginManager.CallHooks(&HLCOOP_PLUGIN_HOOKS::pfnMapActivate);
}

/*
================
PlayerPreThink

Called every frame before physics are run
================
*/
void PlayerPreThink( edict_t *pEntity )
{
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PreThink( );
}

/*
================
PlayerPostThink

Called every frame after physics are run
================
*/
void PlayerPostThink( edict_t *pEntity )
{
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->PostThink( );
}



void ParmsNewLevel( void )
{
}


void ParmsChangeLevel( void )
{
	// retrieve the pointer to the save data
	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;

	if ( pSaveData )
		pSaveData->connectionCount = BuildChangeList( pSaveData->levelList, MAX_LEVEL_CONNECTIONS );
}

void NerfMonsters() {
	edict_t* edicts = ENT(0);

	for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++)
	{
		if (edicts[i].free || !edicts[i].pvPrivateData)
			continue;

		CBaseEntity* pClass = CBaseEntity::Instance(&edicts[i]);
		if (!pClass) {
			continue;
		}

		// nerf monster health/spawners now that all entities have spawned (can check/update connections)
		CBaseMonster* monster = pClass->MyMonsterPointer();
		if (monster) {
			pClass->MyMonsterPointer()->Nerf();
		}
	}

	std::string healthStat;
	std::string npcCountStat;

	if (mp_bulletsponges.value != 1) {
		healthStat = UTIL_VarArgs("%dk -> %dk",
			(g_nerfStats.totalMonsterHealth + g_nerfStats.nerfedMonsterHealth) / 1000,
			g_nerfStats.totalMonsterHealth / 1000);
	}
	else {
		healthStat = UTIL_VarArgs("%dk",
			g_nerfStats.totalMonsterHealth / 1000);
	}

	if (mp_maxmonsterrespawns.value >= 0) {
		npcCountStat = UTIL_VarArgs("%d%s -> %d%s",
			g_nerfStats.nerfedMonsterSpawns + g_nerfStats.totalMonsters,
			(g_nerfStats.skippedMonsterInfiniSpawns + g_nerfStats.nerfedMonsterInfiniSpawns) ? "+" : "",
			g_nerfStats.totalMonsters,
			g_nerfStats.skippedMonsterInfiniSpawns ? "+" : "");
	}
	else {
		npcCountStat = UTIL_VarArgs("%d%s",
			g_nerfStats.totalMonsters,
			g_nerfStats.skippedMonsterInfiniSpawns ? "+" : "");
	}

	ALERT(at_logged, "Enemy stats: %s health, %s npcs\n", healthStat.c_str(), npcCountStat.c_str());
}

//
// GLOBALS ASSUMED SET:  g_ulFrameCount
//
void StartFrame( void )
{
	if ( g_pGameRules )
		g_pGameRules->Think();

	if ( g_fGameOver )
		return;

	gpGlobals->teamplay = teamplay.value;
	g_ulFrameCount++;

	if (!g_monstersNerfed && g_cfgsExecuted && g_serveractive) {
		NerfMonsters();
		g_monstersNerfed = true;
	}

	lagcomp_update();
}


void ClientPrecache( void )
{
	// setup precaches always needed
	PRECACHE_SOUND_ENT(NULL, "player/sprayer.wav");			// spray paint sound for PreAlpha
	
	// PRECACHE_SOUND_ENT(NULL, "player/pl_jumpland2.wav");		// UNDONE: play 2x step sound
	
	//PRECACHE_SOUND_ENT(NULL, "player/pl_fallpain2.wav");
	PRECACHE_SOUND_ENT(NULL, "player/pl_fallpain3.wav");
	
	PRECACHE_SOUND_ENT(NULL, "common/npc_step1.wav");		// NPC walking (all texture types)
	PRECACHE_SOUND_ENT(NULL, "common/npc_step2.wav");
	PRECACHE_SOUND_ENT(NULL, "common/npc_step3.wav");
	PRECACHE_SOUND_ENT(NULL, "common/npc_step4.wav");

	PRECACHE_SOUND_ENT(NULL,  SOUND_FLASHLIGHT_ON );
	PRECACHE_SOUND_ENT(NULL,  SOUND_FLASHLIGHT_OFF );

	// player gib sound
	PRECACHE_SOUND_ENT(NULL, "common/bodysplat.wav");

	// unused player pain sounds
	//PRECACHE_SOUND_ENT(NULL, "player/pl_pain2.wav");
	//PRECACHE_SOUND_ENT(NULL, "player/pl_pain4.wav");
	//PRECACHE_SOUND_ENT(NULL, "player/pl_pain5.wav");
	//PRECACHE_SOUND_ENT(NULL, "player/pl_pain6.wav");
	//PRECACHE_SOUND_ENT(NULL, "player/pl_pain7.wav");

	PRECACHE_MODEL("models/player.mdl");
	
#ifdef CLIENT_DLL
	// geiger sounds (used by client only, played automatically when near radioactive trigger_hurt)
	PRECACHE_SOUND_ENT(NULL, "player/geiger6.wav");
	PRECACHE_SOUND_ENT(NULL, "player/geiger5.wav");
	PRECACHE_SOUND_ENT(NULL, "player/geiger4.wav");
	PRECACHE_SOUND_ENT(NULL, "player/geiger3.wav");
	PRECACHE_SOUND_ENT(NULL, "player/geiger2.wav");
	PRECACHE_SOUND_ENT(NULL, "player/geiger1.wav");

	// hud sounds used only by the client
	PRECACHE_SOUND_ENT(NULL, "common/wpn_hudoff.wav");
	PRECACHE_SOUND_ENT(NULL, "common/wpn_hudon.wav");
	PRECACHE_SOUND_ENT(NULL, "common/wpn_moveselect.wav");
#endif

	// client hud + use key sounds
	PRECACHE_SOUND_ENT(NULL, "common/wpn_select.wav");
	PRECACHE_SOUND_ENT(NULL, "common/wpn_denyselect.wav");

	if (giPrecacheGrunt)
		UTIL_PrecacheOther("monster_human_grunt");
}

/*
===============
GetGameDescription

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription()
{
	if ( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return "Half-Life Co-op";
}

/*
================
Sys_Error

Engine is going to shut down, allows setting a breakpoint in game .dll to catch that occasion
================
*/
void Sys_Error( const char *error_string )
{
	// Default case, do nothing.  MOD AUTHORS:  Add code ( e.g., _asm { int 3 }; here to cause a breakpoint for debugging your game .dlls
}

/*
================
PlayerCustomization

A new player customization has been registered on the server
UNDONE:  This only sets the # of frames of the spray can logo
animation right now.
================
*/
void PlayerCustomization( edict_t *pEntity, customization_t *pCust )
{
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pEntity);

	if (!pPlayer)
	{
		ALERT(at_console, "PlayerCustomization:  Couldn't get player!\n");
		return;
	}

	if (!pCust)
	{
		ALERT(at_console, "PlayerCustomization:  NULL customization!\n");
		return;
	}

	switch (pCust->resource.type)
	{
	case t_decal:
		pPlayer->SetCustomDecalFrames(pCust->nUserData2); // Second int is max # of frames.
		break;
	case t_sound:
	case t_skin:
	case t_model:
		// Ignore for now.
		break;
	default:
		ALERT(at_console, "PlayerCustomization:  Unknown customization type!\n");
		break;
	}
}

/*
================
SpectatorConnect

A spectator has joined the game
================
*/
void SpectatorConnect( edict_t *pEntity )
{
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorConnect( );
}

/*
================
SpectatorConnect

A spectator has left the game
================
*/
void SpectatorDisconnect( edict_t *pEntity )
{
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorDisconnect( );
}

/*
================
SpectatorConnect

A spectator has sent a usercmd
================
*/
void SpectatorThink( edict_t *pEntity )
{
	CBaseSpectator *pPlayer = (CBaseSpectator *)GET_PRIVATE(pEntity);

	if (pPlayer)
		pPlayer->SpectatorThink( );
}

////////////////////////////////////////////////////////
// PAS and PVS routines for client messaging
//

int g_numEdictOverflows[33];

/*
================
SetupVisibility

A client can have a separate "view entity" indicating that his/her view should depend on the origin of that
view entity.  If that's the case, then pViewEntity will be non-NULL and will be used.  Otherwise, the current
entity's origin is used.  Either is offset by the view_ofs to get the eye position.

From the eye position, we set up the PAS and PVS to use for filtering network messages to the client.  At this point, we could
 override the actual PAS or PVS values, or use a different origin.

NOTE:  Do not cache the values of pas and pvs, as they depend on reusable memory in the engine, they are only good for this one frame
================
*/
void SetupVisibility( edict_t *pViewEntity, edict_t *pClient, unsigned char **pvs, unsigned char **pas )
{
	Vector org;
	edict_t *pView = pClient;

	// Find the client's PVS
	if ( pViewEntity )
	{
		pView = pViewEntity;
	}

	if ( pClient->v.flags & FL_PROXY )
	{
		*pvs = NULL;	// the spectator proxy sees
		*pas = NULL;	// and hears everything
		return;
	}

	org = pView->v.origin + pView->v.view_ofs;
	if ( pView->v.flags & FL_DUCKING )
	{
		org = org + ( VEC_HULL_MIN - VEC_DUCK_HULL_MIN );
	}

	*pvs = ENGINE_SET_PVS ( (float *)&org );
	*pas = ENGINE_SET_PAS ( (float *)&org );

	CBasePlayer* plr = (CBasePlayer*)GET_PRIVATE(pClient);
	if (plr) {
		plr->m_hViewEntity.Set(pView);
		plr->m_lastPas = *pas;
		plr->m_lastPvs = *pvs;
	}

	g_packClientIdx = ENTINDEX(pClient);

	int pnum = g_packClientIdx - 1;
	if (g_numEdictOverflows[pnum] > 0) {
		ALERT(at_console, "Overflowed %d edicts for client %s\n",
			g_numEdictOverflows[pnum], STRING(pClient->v.netname));
	}

	g_numEdictOverflows[pnum] = 0;
}

#include "entity_state.h"

/*
AddToFullPack

Return 1 if the entity state has been filled in for the ent and the entity will be propagated to the client, 0 otherwise

state is the server maintained copy of the state info that is transmitted to the client
a MOD could alter values copied into state to send the "host" a different look for a particular entity update, etc.
e and ent are the entity that is being added to the update, if 1 is returned
host is the player's edict of the player whom we are sending the update to
player is 1 if the ent/e is a player and 0 otherwise
pSet is either the PAS or PVS that we previous set up.  We can use it to ask the engine to filter the entity against the PAS or PVS.
we could also use the pas/ pvs that we set in SetupVisibility, if we wanted to.  Caching the value is valid in that case, but still only for the current frame
*/
int AddToFullPack( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet )
{
	int					i;

	CBaseEntity* baseent = (CBaseEntity*)GET_PRIVATE(ent);
	if (!baseent)
		return 0; // should never happen?

	uint32_t plrbit = PLRBIT(host);
	baseent->m_audiblePlayers &= ~plrbit;
	baseent->m_visiblePlayers &= ~plrbit;

	// don't send if flagged for NODRAW and it's not the host getting the message
	// Ignore ents without valid / visible models
	// Don't send spectators to other players
	bool invisible = (ent->v.effects & EF_NODRAW) && (ent != host) ||
					 (!ent->v.modelindex || !STRING(ent->v.model)) ||
					 ((ent->v.flags & FL_SPECTATOR) && (ent != host));

	bool forceVisChecks = baseent->ForceVisChecks();

	if (invisible && !forceVisChecks) {
		return 0; // exit now unless a vis check was requested for this ent
	}

	CBasePlayer* plr = (CBasePlayer*)GET_PRIVATE(host);
	if (!plr)
		return 0; // should never happen?

	if (ENGINE_CHECK_VISIBILITY((const struct edict_s*)ent, plr->m_lastPas)) {
		baseent->m_audiblePlayers |= plrbit;
	}

	if (ENGINE_CHECK_VISIBILITY((const struct edict_s*)ent, plr->m_lastPvs)) {
		baseent->m_visiblePlayers |= plrbit;
	}
	else if(ent != host) {
		// Ignore if not the host and not touching a PVS leaf
		// If pSet is NULL, then the test will always succeed and the entity will be added to the update
		return 0;
	}

	if (invisible || forceVisChecks) {
		return 0;
	}

	// Don't send entity to local client if the client says it's predicting the entity itself.
	if ( ent->v.flags & FL_SKIPLOCALHOST )
	{
		if ( ( hostflags & 1 ) && ( ent->v.owner == host ) )
			return 0;
	}
	
	if ( host->v.groupinfo )
	{
		UTIL_SetGroupTrace( host->v.groupinfo, GROUP_OP_AND );

		// Should always be set, of course
		if ( ent->v.groupinfo )
		{
			if ( g_groupop == GROUP_OP_AND )
			{
				if ( !(ent->v.groupinfo & host->v.groupinfo ) )
					return 0;
			}
			else if ( g_groupop == GROUP_OP_NAND )
			{
				if ( ent->v.groupinfo & host->v.groupinfo )
					return 0;
			}
		}

		UTIL_UnsetGroupTrace();
	}

	memset( state, 0, sizeof( *state ) );

	// Assign index so we can track this entity from frame to frame and
	//  delta from it.
	state->number	  = e;
	state->entityType = ENTITY_NORMAL;
	
	// Flag custom entities.
	if ( ent->v.flags & FL_CUSTOMENTITY )
	{
		state->entityType = ENTITY_BEAM;
	}

	// 
	// Copy state data
	//

	// Round animtime to nearest millisecond
	state->animtime   = (int)(1000.0 * ent->v.animtime ) / 1000.0;

	memcpy( state->origin, ent->v.origin, 3 * sizeof( float ) );
	memcpy( state->angles, ent->v.angles, 3 * sizeof( float ) );
	memcpy( state->mins, ent->v.mins, 3 * sizeof( float ) );
	memcpy( state->maxs, ent->v.maxs, 3 * sizeof( float ) );

	memcpy( state->startpos, ent->v.startpos, 3 * sizeof( float ) );
	memcpy( state->endpos, ent->v.endpos, 3 * sizeof( float ) );

	state->impacttime = ent->v.impacttime;
	state->starttime = ent->v.starttime;

	state->modelindex = ent->v.modelindex;
		
	state->frame      = ent->v.frame;

	state->skin       = ent->v.skin;
	state->effects    = ent->v.effects;

	// This non-player entity is being moved by the game .dll and not the physics simulation system
	//  make sure that we interpolate it's position on the client if it moves
	if ( !player &&
		 ent->v.animtime &&
		 ent->v.velocity[ 0 ] == 0 && 
		 ent->v.velocity[ 1 ] == 0 && 
		 ent->v.velocity[ 2 ] == 0 )
	{
		state->eflags |= EFLAG_SLERP;
	}

	state->scale	  = ent->v.scale;
	state->solid	  = ent->v.solid;
	state->colormap   = ent->v.colormap;

	state->movetype   = ent->v.movetype;
	state->sequence   = ent->v.sequence;
	state->framerate  = ent->v.framerate;
	state->body       = ent->v.body;

	for (i = 0; i < 4; i++)
	{
		state->controller[i] = ent->v.controller[i];
	}

	for (i = 0; i < 2; i++)
	{
		state->blending[i]   = ent->v.blending[i];
	}

	state->rendermode    = ent->v.rendermode;
	state->renderamt     = ent->v.renderamt; 
	state->renderfx      = ent->v.renderfx;
	state->rendercolor.r = ent->v.rendercolor.x;
	state->rendercolor.g = ent->v.rendercolor.y;
	state->rendercolor.b = ent->v.rendercolor.z;

	state->aiment = 0;
	if ( ent->v.aiment )
	{
		state->aiment = ENTINDEX( ent->v.aiment );
	}

	state->owner = 0;
	if ( ent->v.owner )
	{
		int owner = ENTINDEX( ent->v.owner );
		
		// Only care if owned by a player
		if ( owner >= 1 && owner <= gpGlobals->maxClients )
		{
			state->owner = owner;	
		}
	}

	// HACK:  Somewhat...
	// Class is overridden for non-players to signify a breakable glass object ( sort of a class? )
	if ( !player )
	{
		state->playerclass  = ent->v.playerclass;
	}

	// Special stuff for players only
	if ( player )
	{
		memcpy( state->basevelocity, ent->v.basevelocity, 3 * sizeof( float ) );

		state->weaponmodel  = MODEL_INDEX( STRING( ent->v.weaponmodel ) );
		state->gaitsequence = ent->v.gaitsequence;
		state->spectator = ent->v.flags & FL_SPECTATOR;
		state->friction     = ent->v.friction;

		state->gravity      = ent->v.gravity;
//		state->team			= ent->v.team;
//		
		state->usehull      = ( ent->v.flags & FL_DUCKING ) ? 1 : 0;
		state->health		= ent->v.health;

		if (g_packClientIdx == e) {
			// clients invert their own player model angle for some reason
			// TODO: fix this in a client mod
			state->angles.x = ent->v.v_angle.x;
		}
		
	}

	// prevent disconnects with this error:
	// Host_Error: CL_EntityNum: 1665 is an invalid number, cl.max_edicts is 1665
	if (sv_max_client_edicts && e >= (int)sv_max_client_edicts->value) {
		ALERT(at_console, "Can't send edict %d '%s' (index too high)\n", e, STRING(ent->v.classname));
		g_numEdictOverflows[player]++;
		return 0;
	}

	return 1;
}

// defaults for clientinfo messages
#define	DEFAULT_VIEWHEIGHT	28

/*
===================
CreateBaseline

Creates baselines used for network encoding, especially for player data since players are not spawned until connect time.
===================
*/
void CreateBaseline( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs )
{
	baseline->origin		= entity->v.origin;
	baseline->angles		= entity->v.angles;
	baseline->frame			= entity->v.frame;
	baseline->skin			= (short)entity->v.skin;

	// render information
	baseline->rendermode	= (byte)entity->v.rendermode;
	baseline->renderamt		= (byte)entity->v.renderamt;
	baseline->rendercolor.r	= (byte)entity->v.rendercolor.x;
	baseline->rendercolor.g	= (byte)entity->v.rendercolor.y;
	baseline->rendercolor.b	= (byte)entity->v.rendercolor.z;
	baseline->renderfx		= (byte)entity->v.renderfx;

	if ( player )
	{
		baseline->mins			= player_mins;
		baseline->maxs			= player_maxs;

		baseline->colormap		= eindex;
		baseline->modelindex	= playermodelindex;
		baseline->friction		= 1.0;
		baseline->movetype		= MOVETYPE_WALK;

		baseline->scale			= entity->v.scale;
		baseline->solid			= SOLID_SLIDEBOX;
		baseline->framerate		= 1.0;
		baseline->gravity		= 1.0;

	}
	else
	{
		baseline->mins			= entity->v.mins;
		baseline->maxs			= entity->v.maxs;

		baseline->colormap		= 0;
		baseline->modelindex	= entity->v.modelindex;//SV_ModelIndex(pr_strings + entity->v.model);
		baseline->movetype		= entity->v.movetype;

		baseline->scale			= entity->v.scale;
		baseline->solid			= entity->v.solid;
		baseline->framerate		= entity->v.framerate;
		baseline->gravity		= entity->v.gravity;
	}
}

typedef struct
{
	char name[32];
	int	 field;
} entity_field_alias_t;

#define FIELD_ORIGIN0			0
#define FIELD_ORIGIN1			1
#define FIELD_ORIGIN2			2
#define FIELD_ANGLES0			3
#define FIELD_ANGLES1			4
#define FIELD_ANGLES2			5

static entity_field_alias_t entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
};

void Entity_FieldInit( struct delta_s *pFields )
{
	entity_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN0 ].name );
	entity_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN1 ].name );
	entity_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ORIGIN2 ].name );
	entity_field_alias[ FIELD_ANGLES0 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES0 ].name );
	entity_field_alias[ FIELD_ANGLES1 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES1 ].name );
	entity_field_alias[ FIELD_ANGLES2 ].field		= DELTA_FINDFIELD( pFields, entity_field_alias[ FIELD_ANGLES2 ].name );
}

/*
==================
Entity_Encode

Callback for sending entity_state_t info over network. 
FIXME:  Move to script
==================
*/
void Entity_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int localplayer = 0;
	static int initialized = 0;

	if ( !initialized )
	{
		Entity_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->impacttime != 0 ) && ( t->starttime != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );

		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ANGLES2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

static entity_field_alias_t player_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
};

void Player_FieldInit( struct delta_s *pFields )
{
	player_field_alias[ FIELD_ORIGIN0 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN0 ].name );
	player_field_alias[ FIELD_ORIGIN1 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN1 ].name );
	player_field_alias[ FIELD_ORIGIN2 ].field		= DELTA_FINDFIELD( pFields, player_field_alias[ FIELD_ORIGIN2 ].name );
}

/*
==================
Player_Encode

Callback for sending entity_state_t for players info over network. 
==================
*/
void Player_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int localplayer = 0;
	static int initialized = 0;

	if ( !initialized )
	{
		Player_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	// Never send origin to local player, it's sent with more resolution in clientdata_t structure
	localplayer =  ( t->number - 1 ) == ENGINE_CURRENT_PLAYER();
	if ( localplayer )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}

	if ( ( t->movetype == MOVETYPE_FOLLOW ) &&
		 ( t->aiment != 0 ) )
	{
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
	else if ( t->aiment != f->aiment )
	{
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN0 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN1 ].field );
		DELTA_SETBYINDEX( pFields, entity_field_alias[ FIELD_ORIGIN2 ].field );
	}
}

#define CUSTOMFIELD_ORIGIN0			0
#define CUSTOMFIELD_ORIGIN1			1
#define CUSTOMFIELD_ORIGIN2			2
#define CUSTOMFIELD_ANGLES0			3
#define CUSTOMFIELD_ANGLES1			4
#define CUSTOMFIELD_ANGLES2			5
#define CUSTOMFIELD_SKIN			6
#define CUSTOMFIELD_SEQUENCE		7
#define CUSTOMFIELD_ANIMTIME		8

entity_field_alias_t custom_entity_field_alias[]=
{
	{ "origin[0]",			0 },
	{ "origin[1]",			0 },
	{ "origin[2]",			0 },
	{ "angles[0]",			0 },
	{ "angles[1]",			0 },
	{ "angles[2]",			0 },
	{ "skin",				0 },
	{ "sequence",			0 },
	{ "animtime",			0 },
};

void Custom_Entity_FieldInit( struct delta_s *pFields )
{
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field	= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].name );
	custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].name );
	custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field= DELTA_FINDFIELD( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].name );
}

/*
==================
Custom_Encode

Callback for sending entity_state_t info ( for custom entities ) over network. 
FIXME:  Move to script
==================
*/
void Custom_Encode( struct delta_s *pFields, const unsigned char *from, const unsigned char *to )
{
	entity_state_t *f, *t;
	int beamType;
	static int initialized = 0;

	if ( !initialized )
	{
		Custom_Entity_FieldInit( pFields );
		initialized = 1;
	}

	f = (entity_state_t *)from;
	t = (entity_state_t *)to;

	beamType = t->rendermode & 0x0f;
		
	if ( beamType != BEAM_POINTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ORIGIN2 ].field );
	}

	if ( beamType != BEAM_POINTS )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES0 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES1 ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANGLES2 ].field );
	}

	if ( beamType != BEAM_ENTS && beamType != BEAM_ENTPOINT )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SKIN ].field );
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_SEQUENCE ].field );
	}

	// animtime is compared by rounding first
	// see if we really shouldn't actually send it
	if ( (int)f->animtime == (int)t->animtime )
	{
		DELTA_UNSETBYINDEX( pFields, custom_entity_field_alias[ CUSTOMFIELD_ANIMTIME ].field );
	}
}

/*
=================
RegisterEncoders

Allows game .dll to override network encoding of certain types of entities and tweak values, etc.
=================
*/
void RegisterEncoders( void )
{
	DELTA_ADDENCODER( "Entity_Encode", Entity_Encode );
	DELTA_ADDENCODER( "Custom_Encode", Custom_Encode );
	DELTA_ADDENCODER( "Player_Encode", Player_Encode );
}

int GetWeaponData( struct edict_s *player, struct weapon_data_s *info )
{
#if defined( CLIENT_WEAPONS )
	int i;
	weapon_data_t *item;
	entvars_t *pev = &player->v;
	CBasePlayer *pl = dynamic_cast< CBasePlayer *>( CBasePlayer::Instance( pev ) );
	CBasePlayerWeapon *gun;
	
	ItemInfo II;

	memset( info, 0, 32 * sizeof( weapon_data_t ) );

	if ( !pl )
		return 1;

	// go through all of the weapons and make a list of the ones to pack
	for ( i = 0 ; i < MAX_ITEM_TYPES ; i++ )
	{
		if ( pl->m_rgpPlayerItems[ i ] )
		{
			// there's a weapon here. Should I pack it?
			CBasePlayerItem *pPlayerItem = (CBasePlayerItem*)pl->m_rgpPlayerItems[i].GetEntity();

			while ( pPlayerItem )
			{
				gun = dynamic_cast<CBasePlayerWeapon *>( pPlayerItem->GetWeaponPtr() );
				if ( gun && gun->UseDecrement() )
				{
					// Get The ID.
					memset( &II, 0, sizeof( II ) );
					gun->GetItemInfo( &II );

					if ( II.iId >= 0 && II.iId < 32 )
					{
						item = &info[ II.iId ];
					 	
						item->m_iId						= II.iId;
						item->m_iClip					= gun->m_iClip;

						item->m_flTimeWeaponIdle		= V_max( gun->m_flTimeWeaponIdle, -0.001 );
						item->m_flNextPrimaryAttack		= V_max( gun->m_flNextPrimaryAttack, -0.001 );
						item->m_flNextSecondaryAttack	= V_max( gun->m_flNextSecondaryAttack, -0.001 );
						item->m_fInReload				= gun->m_fInReload;
						item->m_fInSpecialReload		= gun->m_fInSpecialReload;
						item->fuser1					= V_max( gun->pev->fuser1, -0.001 );
						item->fuser2					= gun->m_flStartThrow;
						item->fuser3					= gun->m_flReleaseThrow;
						item->iuser1					= gun->m_chargeReady;
						item->iuser2					= gun->m_fInAttack;
						item->iuser3					= gun->m_fireState;
						
											
//						item->m_flPumpTime				= V_max( gun->m_flPumpTime, -0.001 );
					}
				}
				pPlayerItem = (CBasePlayerItem*)pPlayerItem->m_pNext.GetEntity();
			}
		}
	}
#else
	memset( info, 0, 32 * sizeof( weapon_data_t ) );
#endif
	return 1;
}

/*
=================
UpdateClientData

Data sent to current client only
engine sets cd to 0 before calling.
=================
*/
void UpdateClientData ( const edict_t *ent, int sendweapons, struct clientdata_s *cd )
{
	if ( !ent || !ent->pvPrivateData )
		return;
	entvars_t *		pev	= (entvars_t *)&ent->v;
	CBasePlayer *	pl	= dynamic_cast< CBasePlayer *>(CBasePlayer::Instance( pev ));
	entvars_t *		pevOrg = NULL;

	// if user is spectating different player in First person, override some vars
	if ( pl && pl->pev->iuser1 == OBS_IN_EYE )
	{
		if ( pl->m_hObserverTarget )
		{
			pevOrg = pev;
			pev = pl->m_hObserverTarget->pev;
			pl = dynamic_cast< CBasePlayer *>(CBasePlayer::Instance( pev ) );
		}
	}

	cd->flags			= pev->flags;
	cd->health			= pev->health;

	cd->viewmodel		= MODEL_INDEX( STRING( pev->viewmodel ) );

	cd->waterlevel		= pev->waterlevel;
	cd->watertype		= pev->watertype;
	cd->weapons			= pev->weapons;

	// Vectors
	cd->origin			= pev->origin;
	cd->velocity		= pev->velocity;
	cd->view_ofs		= pev->view_ofs;
	cd->punchangle		= pev->punchangle;

	cd->bInDuck			= pev->bInDuck;
	cd->flTimeStepSound = pev->flTimeStepSound;
	cd->flDuckTime		= pev->flDuckTime;
	cd->flSwimTime		= pev->flSwimTime;
	cd->waterjumptime	= pev->teleport_time;

	strcpy_safe( cd->physinfo, ENGINE_GETPHYSINFO( ent ), 256 );

	cd->maxspeed		= pev->maxspeed;
	cd->fov				= pev->fov;
	cd->weaponanim		= pev->weaponanim;

	cd->pushmsec		= pev->pushmsec;

	//Spectator mode
	if ( pevOrg != NULL )
	{
		// don't use spec vars from chased player
		cd->iuser1			= pevOrg->iuser1;
		cd->iuser2			= pevOrg->iuser2;
	}
	else
	{
		cd->iuser1			= pev->iuser1;
		cd->iuser2			= pev->iuser2;
	}

	

#if defined( CLIENT_WEAPONS )
	if ( sendweapons )
	{
		if ( pl )
		{
			cd->m_flNextAttack	= pl->m_flNextAttack;
			cd->fuser2			= pl->m_flNextAmmoBurn;
			cd->fuser3			= pl->m_flAmmoStartCharge;
			cd->vuser1.x		= pl->ammo_9mm;
			cd->vuser1.y		= pl->ammo_357;
			cd->vuser1.z		= pl->ammo_argrens;
			cd->ammo_nails		= pl->ammo_bolts;
			cd->ammo_shells		= pl->ammo_buckshot;
			cd->ammo_rockets	= pl->ammo_rockets;
			cd->ammo_cells		= pl->ammo_uranium;
			cd->vuser2.x		= pl->ammo_hornets;
			
			CBasePlayerItem* activeItem = (CBasePlayerItem*)pl->m_pActiveItem.GetEntity();

			if (activeItem)
			{
				CBasePlayerWeapon* gun = (CBasePlayerWeapon*)activeItem->GetWeaponPtr();
				if ( gun && gun->UseDecrement() )
				{
					ItemInfo II;
					memset( &II, 0, sizeof( II ) );
					gun->GetItemInfo( &II );

					cd->m_iId = II.iId;

					cd->vuser3.z	= gun->m_iSecondaryAmmoType;
					cd->vuser4.x	= gun->m_iPrimaryAmmoType;
					cd->vuser4.y	= pl->m_rgAmmo[gun->m_iPrimaryAmmoType];
					cd->vuser4.z	= pl->m_rgAmmo[gun->m_iSecondaryAmmoType];
					
					if (gun->m_iId == WEAPON_RPG )
					{
						cd->vuser2.y = ( ( CRpg * )gun)->m_fSpotActive;
						cd->vuser2.z = ( ( CRpg * )gun)->m_cActiveRockets;
					}
				}
			}
		}
	} 
#endif
}

/*
=================
CmdStart

We're about to run this usercmd for the specified player.  We can set up groupinfo and masking here, etc.
This is the time to examine the usercmd for anything extra.  This call happens even if think does not.
=================
*/
void CmdStart( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed )
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = dynamic_cast< CBasePlayer *>( CBasePlayer::Instance( pev ) );

	if( !pl )
		return;

	if ( pl->pev->groupinfo != 0 )
	{
		UTIL_SetGroupTrace( pl->pev->groupinfo, GROUP_OP_AND );
	}

	pl->random_seed = random_seed;
}

/*
=================
CmdEnd

Each cmdstart is exactly matched with a cmd end, clean up any group trace flags, etc. here
=================
*/
void CmdEnd ( const edict_t *player )
{
	entvars_t *pev = (entvars_t *)&player->v;
	CBasePlayer *pl = dynamic_cast< CBasePlayer *>( CBasePlayer::Instance( pev ) );

	if( !pl )
		return;
	if ( pl->pev->groupinfo != 0 )
	{
		UTIL_UnsetGroupTrace();
	}
}

/*
================================
ConnectionlessPacket

 Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
  size of the response_buffer, so you must zero it out if you choose not to respond.
================================
*/
int	ConnectionlessPacket( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size )
{
	// Parse stuff from args
	//int max_buffer_size = *response_buffer_size;

	// Zero it out since we aren't going to respond.
	// If we wanted to response, we'd write data into response_buffer
	*response_buffer_size = 0;

	// Since we don't listen for anything here, just respond that it's a bogus message
	// If we didn't reject the message, we'd return 1 for success instead.
	return 0;
}

/*
================================
GetHullBounds

  Engine calls this to enumerate player collision hulls, for prediction.  Return 0 if the hullnumber doesn't exist.
================================
*/
int GetHullBounds( int hullnumber, float *mins, float *maxs )
{
	int iret = 0;

	// TODO: what was this meant to do? Can you change player sizes with this?
	// the code below was setting the pointers that were passed here by value... which does nothing.
	// doing a memcpy on them instead had no effect. Didn't try changing the sizes yet.

	switch ( hullnumber )
	{
	case 0:				// Normal player
		//mins = VEC_HULL_MIN;
		//maxs = VEC_HULL_MAX;
		iret = 1;
		break;
	case 1:				// Crouched player
		//mins = VEC_DUCK_HULL_MIN;
		//maxs = VEC_DUCK_HULL_MAX;
		iret = 1;
		break;
	case 2:				// Point based hull
		//mins = Vector( 0, 0, 0 );
		//maxs = Vector( 0, 0, 0 );
		iret = 1;
		break;
	}

	return iret;
}

/*
================================
CreateInstancedBaselines

Create pseudo-baselines for items that aren't placed in the map at spawn time, but which are likely
to be created during play ( e.g., grenades, ammo packs, projectiles, corpses, etc. )
================================
*/
void CreateInstancedBaselines ( void )
{
	entity_state_t state;

	memset( &state, 0, sizeof( state ) );

	// Create any additional baselines here for things like grendates, etc.
	// int iret = ENGINE_INSTANCE_BASELINE( pc->pev->classname, &state );

	// Destroy objects.
	//UTIL_Remove( pc );
}

/*
================================
InconsistentFile

One of the ENGINE_FORCE_UNMODIFIED files failed the consistency check for the specified player
 Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
================================
*/
int	InconsistentFile( const edict_t *player, const char *filename, char *disconnect_message )
{
	// Server doesn't care?
	if ( CVAR_GET_FLOAT( "mp_consistency" ) != 1 )
		return 0;

	// Default behavior is to kick the player
	snprintf( disconnect_message, 256, "Server is enforcing file consistency for %s\n", filename );

	// Kick now with specified disconnect message.
	return 1;
}

/*
================================
AllowLagCompensation

 The game .dll should return 1 if lag compensation should be allowed ( could also just set
  the sv_unlag cvar.
 Most games right now should return 0, until client-side weapon prediction code is written
  and tested for them ( note you can predict weapons, but not do lag compensation, too, 
  if you want.
================================
*/
int AllowLagCompensation( void )
{
	return 1;
}
