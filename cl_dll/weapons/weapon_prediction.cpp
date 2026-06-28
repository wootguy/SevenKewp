/*** 
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
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
#include "../dlls/extdll.h"
#include "shared_util.h"
#include "../dlls/weapon/weapons.h"
#include "../dlls/player/CBasePlayer.h"

#include "usercmd.h"
#include "entity_state.h"
#include "demo_api.h"
#include "pm_defs.h"
#include "event_api.h"
#include "r_efx.h"

#include "hud_iface.h"
#include "com_weapons.h"
#include "demo.h"

#include "../dlls/weapon/CGlock.h"
#include "../dlls/weapon/CCrowbar.h"
#include "../dlls/weapon/CPython.h"
#include "../dlls/weapon/CMP5.h"
#include "../dlls/weapon/CCrossbow.h"
#include "../dlls/weapon/CShotgun.h"
#include "../dlls/weapon/CRpg.h"
#include "../dlls/weapon/CGauss.h"
#include "../dlls/weapon/CEgon.h"
#include "../dlls/weapon/CHgun.h"
#include "../dlls/weapon/CHandGrenade.h"
#include "../dlls/weapon/CSatchel.h"
#include "../dlls/weapon/CTripmine.h"
#include "../dlls/weapon/CSqueak.h"
#include "../dlls/weapon/custom/CWeaponCustom.h"
#include "prediction_files.h"
#include "weapon_res.h"
#include "ev_custom.h"

#include "studio.h"

#include "ModPlayerState.h"

#include "hud_iface.h"
#include "cl_dll.h"
#define PRINTF(fmt, ...) gEngfuncs.Con_Printf(fmt, __VA_ARGS__)

extern int g_iUser1;

// Pool of client side entities/entvars_t
static entvars_t	ev[MAX_WEAPONS];
static int			num_ents = 0;

// The entity we'll use to represent the local client
static CBasePlayer	player;

// Local version of game .dll global variables ( time, etc. )
static globalvars_t	Globals; 
globalvars_t* gpGlobals = &Globals;

// Globals used by game logic
EXPORT const Vector g_vecZero = Vector(0, 0, 0);
enginefuncs_t g_engfuncs;

// prediction state for the current frame.
// shared with the rest of client dll, with no need to include server headers.
PredictionState g_prediction;

static CBasePlayerWeapon *g_pWpns[ MAX_WEAPONS ];

ItemInfo CBasePlayerItem::ItemInfoArray[MAX_WEAPONS];

float g_flApplyVel = 0.0;
Vector g_vApplyVel;
int   g_irunninggausspred = 0;
int   g_runningKickbackPred = 0;

vec3_t previousorigin;

// HLDM Weapon placeholder entities.
CGlock g_Glock;
CCrowbar g_Crowbar;
CPython g_Python;
CMP5 g_Mp5;
CCrossbow g_Crossbow;
CShotgun g_Shotgun;
CRpg g_Rpg;
CGauss g_Gauss;
CEgon g_Egon;
CHgun g_HGun;
CHandGrenade g_HandGren;
CSatchel g_Satchel;
CTripmine g_Tripmine;
CSqueak g_Snark;
CWeaponCustom g_customWeapon[MAX_WEAPONS];

ModPlayerState g_modPlayerStates[32];

uint32_t g_cmd_id; // TODO: test what happens at overflow in pred code
uint32_t g_latest_cmd_id; // ID of the most recent command

#define CMD_TIMER_HIST_SZ 8192
uint32_t g_cmd_timer_hist[CMD_TIMER_HIST_SZ]; // stores timer value at each cmd
int g_cmd_timer_idx;
uint32_t g_cmd_timer; // accumulates time from each new user command
bool g_cmd_debug_mode = false;

// get the global cmd timer value at the given cmd ID
uint32_t GetTimeAtCmd(uint32_t cmdId) {
	if (cmdId >= g_latest_cmd_id) {
		return g_cmd_timer;
	}

	int dist = g_latest_cmd_id - cmdId;
	if (dist >= CMD_TIMER_HIST_SZ) {
		PRINTF("CMD timer history index %d > %d\n", dist, CMD_TIMER_HIST_SZ);
		return 0;
	}

	int idx = g_cmd_timer_idx - dist;
	if (idx < 0)
		idx += CMD_TIMER_HIST_SZ;

	return g_cmd_timer_hist[idx];
}

CustomWeaponParams* GetCustomWeaponParams(int id, int which) {
	if (id >= 0 && id < MAX_WEAPONS) {
		switch (which) {
		default:
			return &g_customWeapon[id].GetActiveParams();
		case WC_PARAMS_DEFAULT:
			return &g_customWeapon[id].defaultParams;
		case WC_PARAMS_ALTERNATE:
			return &g_customWeapon[id].alternateParams;
		}
	}

	gEngfuncs.Con_Printf("Invalid custom weapon ID %d\n", id);
	return NULL;
}

const char* PredictionState::GetCustomWeaponStateString() {
	static std::string stateStr;

	if (weapon.isCustom) {
		int state = local.weapondata[local.client.m_iId].iuser3;
		stateStr = CWeaponCustom::GetStateString(state);
		return stateStr.c_str();
	}
	
	return "";
}

const char* PredictionState::GetCustomWeaponChargeStatesString() {
	static std::string stateStr;

	if (weapon.isCustom) {
		int state = local.weapondata[local.client.m_iId].iuser2;
		stateStr = CWeaponCustom::GetChargeStatesString(state);
		return stateStr.c_str();
	}

	return "";
}

bool IsExclusiveWeapon(int id) {
	CustomWeaponParams* params = GetCustomWeaponParams(id, WC_PARAMS_AUTO);
	return params && (params->flags & FL_WC_WEP_EXCLUSIVE_HOLD);
}

void InitCustomWeapon(int id) {
	if (id >= 0 && id < MAX_WEAPONS)
		g_customWeapon[id].m_hasPredictionData = true;
}

void ResetCustomWeaponStates() {
	for (int i = 0; i < MAX_WEAPONS; i++) {
		g_customWeapon[i].m_lastBeamUpdate = 0;
		g_customWeapon[i].m_bInAkimboReload = 0;
		g_customWeapon[i].m_bWantAkimboReload = 0;
		g_customWeapon[i].m_hasLaserAttachment = 0;
		g_customWeapon[i].m_lastDeploy = 0;
		g_customWeapon[i].m_laserOnTime = 0;
		g_customWeapon[i].m_hasPredictionData = false;
		g_customWeapon[i].m_chargeStartCmdTime = 0;
		g_customWeapon[i].m_chargeStopCmdTime = 0;
		g_customWeapon[i].m_stateChangeCmdTime = 0;
		g_customWeapon[i].m_lastCharge = 0;
		g_customWeapon[i].m_idleTime = 0;
		g_customWeapon[i].m_runningKickbackPred = 0;
		g_customWeapon[i].m_kickbackPredVel = Vector(0,0,0);
		g_customWeapon[i].events.m_bulletFireCount = 0;
		g_customWeapon[i].events.m_bulletFireCount2 = 0;
		g_customWeapon[i].m_akimboAnim = 0;
		g_customWeapon[i].m_akimboAnimTime = 0;
		g_customWeapon[i].m_akimboLastEventFrame = 0;
		g_customWeapon[i].m_lastCanAkimbo = 0;
		g_customWeapon[i].ammoFreqs[0] = 0;
		g_customWeapon[i].ammoFreqs[1] = 0;
		g_customWeapon[i].ammoFreqs[2] = 0;
		g_customWeapon[i].events.animCount = 0;
		g_customWeapon[i].m_chargeSoundEvt = 0;
		g_customWeapon[i].m_chargeStartClip = 0;
		g_customWeapon[i].m_stateIconIdx = 0;
		g_customWeapon[i].m_attackChamberCmdTime = 0;
		g_customWeapon[i].m_active_cs_recoil_evt = 0;
		memset(&g_customWeapon[i].m_viewModelSpr, 0, sizeof(g_customWeapon[i].m_viewModelSpr));
		memset(g_customWeapon[i].m_reloadStageCmdTime, 0, sizeof(uint32_t) * WC_RELOAD_STAGES);
		memset(g_customWeapon[i].events.m_beams, 0, sizeof(WcBeam) * MAX_WC_BEAMS);
		memset(&g_customWeapon[i].events.m_beamImpactSprite, 0, sizeof(WcSprite));
		g_customWeapon[i].SetAkimbo(false);
		g_customWeapon[i].SetLaser(false);
	}

	memset(CBasePlayerItem::ItemInfoArray, 0, sizeof(CBasePlayerItem::ItemInfoArray));
	memset(g_modPlayerStates, 0, sizeof(g_modPlayerStates));
	UnloadCustomMuzzleFlashes();
}

void SetItemInfo(WEAPON* wep) {
	if (!wep || wep->iId < 0 || wep->iId >= MAX_WEAPONS)
		return;
	
	ItemInfo& info = CBasePlayerItem::ItemInfoArray[wep->iId];
	info.iFlags = wep->iFlags;
}

/*
======================
AlertMessage

Print debug messages to console
======================
*/
void AlertMessage( ALERT_TYPE atype, const char *szFmt, ... )
{
	va_list		argptr;
	static char	string[1024];
	
	va_start (argptr, szFmt);
	vsprintf (string, szFmt,argptr);
	va_end (argptr);

	gEngfuncs.Con_Printf( "cl:  " );
	gEngfuncs.Con_Printf( string );
}

//Returns if it's multiplayer.
//Mostly used by the client side weapons.
bool bIsMultiplayer ( void )
{
	return gEngfuncs.GetMaxClients() == 1 ? 0 : 1;
}
//Just loads a v_ model.
void LoadVModel ( const char *szViewModel, CBasePlayer *m_pPlayer )
{
	gEngfuncs.CL_LoadModel( szViewModel, &m_pPlayer->pev->viewmodel );
}

/*
=====================
HUD_PrepEntity

Links the raw entity to an entvars_s holder.  If a player is passed in as the owner, then
we set up the m_pPlayer field.
=====================
*/
void HUD_PrepEntity( CBaseEntity *pEntity, CBasePlayer *pWeaponOwner )
{
	memset( &ev[ num_ents ], 0, sizeof( entvars_t ) );
	pEntity->pev = &ev[ num_ents++ ];

	pEntity->Precache();
	pEntity->Spawn();

	if ( pWeaponOwner )
	{
		ItemInfo info;
		
		((CBasePlayerWeapon *)pEntity)->m_hPlayer = pWeaponOwner;
		
		((CBasePlayerWeapon *)pEntity)->GetItemInfo( &info );

		g_pWpns[ info.iId ] = (CBasePlayerWeapon *)pEntity;
	}
}

/*
=====================
HUD_InitClientWeapons

Set up weapons, player and functions needed to run weapons code client-side.
=====================
*/
void HUD_InitClientWeapons( void )
{
	static int initialized = 0;
	if ( initialized )
		return;

	initialized = 1;

	// Fill in current time ( probably not needed )
	gpGlobals->time = gEngfuncs.GetClientTime();

	// Fake functions
	g_engfuncs.pfnPrecacheModel		= stub_PrecacheModel;
	g_engfuncs.pfnPrecacheSound		= stub_PrecacheSound;
	g_engfuncs.pfnPrecacheEvent		= stub_PrecacheEvent;
	g_engfuncs.pfnNameForFunction	= stub_NameForFunction;
	g_engfuncs.pfnSetModel			= stub_SetModel;
	g_engfuncs.pfnSetClientMaxspeed = HUD_SetMaxSpeed;

	// Handled locally
	g_engfuncs.pfnPlaybackEvent		= HUD_PlaybackEvent;
	g_engfuncs.pfnAlertMessage		= AlertMessage;

	// Pass through to engine
	g_engfuncs.pfnPrecacheEvent		= gEngfuncs.pfnPrecacheEvent;
	g_engfuncs.pfnRandomFloat		= gEngfuncs.pfnRandomFloat;
	g_engfuncs.pfnRandomLong		= gEngfuncs.pfnRandomLong;

	// Allocate a slot for the local player
	HUD_PrepEntity( &player		, NULL );

	// Allocate slot(s) for each weapon that we are going to be predicting
	HUD_PrepEntity( &g_Glock	, &player );
	HUD_PrepEntity( &g_Crowbar	, &player );
	HUD_PrepEntity( &g_Python	, &player );
	HUD_PrepEntity( &g_Mp5	, &player );
	HUD_PrepEntity( &g_Crossbow	, &player );
	HUD_PrepEntity( &g_Shotgun	, &player );
	HUD_PrepEntity( &g_Rpg	, &player );
	HUD_PrepEntity( &g_Gauss	, &player );
	HUD_PrepEntity( &g_Egon	, &player );
	HUD_PrepEntity( &g_HGun	, &player );
	HUD_PrepEntity( &g_HandGren	, &player );
	HUD_PrepEntity( &g_Satchel	, &player );
	HUD_PrepEntity( &g_Tripmine	, &player );
	HUD_PrepEntity( &g_Snark	, &player );

	// fill remaining slots with dummy weapons
	for (int i = 1; i < MAX_WEAPONS; i++) {
		if (!g_pWpns[i]) {
			HUD_PrepEntity(&g_customWeapon[i], NULL);
			g_customWeapon[i].m_hPlayer = &player;
			g_customWeapon[i].m_iId = i;
			g_pWpns[i] = &g_customWeapon[i];
		}
	}
}

/*
=====================
HUD_GetLastOrg

Retruns the last position that we stored for egon beam endpoint.
=====================
*/
void HUD_GetLastOrg( float *org )
{
	int i;
	
	// Return last origin
	for ( i = 0; i < 3; i++ )
	{
		org[i] = previousorigin[i];
	}
}

/*
=====================
HUD_SetLastOrg

Remember our exact predicted origin so we can draw the egon to the right position.
=====================
*/
void HUD_SetLastOrg( void )
{
	int i;
	
	// Offset final origin by view_offset
	for ( i = 0; i < 3; i++ )
	{
		previousorigin[i] = g_finalstate->playerstate.origin[i] + g_finalstate->client.view_ofs[ i ];
	}
}

CBasePlayerWeapon* GetPredictedWeapon(int id) {
	// Fill in data based on selected weapon
	// FIXME, make this a method in each weapon?  where you pass in an entity_state_t *?
	switch (id)
	{
	case WEAPON_CROWBAR: return &g_Crowbar;
	case WEAPON_GLOCK: return &g_Glock;
	case WEAPON_PYTHON: return &g_Python;
	case WEAPON_MP5: return &g_Mp5;
	case WEAPON_CROSSBOW: return &g_Crossbow;
	case WEAPON_SHOTGUN: return &g_Shotgun;
	case WEAPON_RPG: return &g_Rpg;
	case WEAPON_GAUSS: return &g_Gauss;
	case WEAPON_EGON: return &g_Egon;
	case WEAPON_HORNETGUN: return &g_HGun;
	case WEAPON_HANDGRENADE: return &g_HandGren;
	case WEAPON_SATCHEL: return &g_Satchel;
	case WEAPON_TRIPMINE: return &g_Tripmine;
	case WEAPON_SNARK: return &g_Snark;
	default:
		if (id >= 0 && id < MAX_WEAPONS)
			return &g_customWeapon[id];
		break;
	}

	return NULL;
}

// load the previous state into current state
void load_prediction_state(local_state_s* from, usercmd_t* cmd, int random_seed) {
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		CBasePlayerWeapon* pCurrent = g_pWpns[i];
		if (!pCurrent)
		{
			continue;
		}

		weapon_data_t* pfrom = &from->weapondata[i];

		pCurrent->m_fInReload = pfrom->m_fInReload;
		pCurrent->m_fInSpecialReload = pfrom->m_fInSpecialReload;
		//		pCurrent->m_flPumpTime			= pfrom->m_flPumpTime;
		pCurrent->m_iClip = pfrom->m_iClip;
		pCurrent->m_iClip2 = pfrom->iuser4;
		pCurrent->m_flNextPrimaryAttack = pfrom->m_flNextPrimaryAttack;
		pCurrent->m_flNextSecondaryAttack = pfrom->m_flNextSecondaryAttack;
		pCurrent->m_flNextTertiaryAttack = pfrom->fuser4;
		pCurrent->m_flTimeWeaponIdle = pfrom->m_flTimeWeaponIdle;
		pCurrent->pev->fuser1 = pfrom->fuser1;
		pCurrent->m_flStartThrow = pfrom->fuser2;
		pCurrent->m_flReleaseThrow = pfrom->fuser3;
		pCurrent->m_chargeReady = pfrom->iuser1;
		pCurrent->m_fInAttack = pfrom->iuser2;
		pCurrent->m_fireState = pfrom->iuser3;
		pCurrent->m_iShotsFired = pfrom->m_fInZoom & 0xf;
		pCurrent->m_iDirection = (pfrom->m_fInZoom >> 4) & 1;
		pCurrent->m_bDelayFire = (pfrom->m_fInZoom >> 5) & 1;
		pCurrent->m_flNextShotsFiredDec = pfrom->m_fNextAimBonus;

		pCurrent->m_iSecondaryAmmoType = (int)from->client.vuser3[2];
		pCurrent->m_iPrimaryAmmoType = (int)from->client.vuser4[0];
		player.m_rgAmmo[pCurrent->m_iPrimaryAmmoType] = (int)from->client.vuser4[1];
		player.m_rgAmmo[pCurrent->m_iSecondaryAmmoType] = (int)from->client.vuser4[2];
	}

	// For random weapon events, use this seed to seed random # generator
	player.random_seed = random_seed;

	// Get old buttons from previous state.
	player.m_afButtonLast = from->playerstate.oldbuttons;

	// Which buttons have changed
	int buttonsChanged = (player.m_afButtonLast ^ cmd->buttons);	// These buttons have changed this frame

	// Debounced button codes for pressed/released
	// The changed ones still down are "pressed"
	player.m_afButtonPressed = buttonsChanged & cmd->buttons;
	// The ones not down are "released"
	player.m_afButtonReleased = buttonsChanged & (~cmd->buttons);
	player.pev->v_angle = cmd->viewangles;
	player.pev->origin = from->client.origin;

	// Set player variables that weapons code might check/alter
	player.pev->button = cmd->buttons;

	player.pev->velocity = from->client.velocity;
	player.pev->flags = from->client.flags;

	player.pev->deadflag = from->client.deadflag;
	player.pev->waterlevel = from->client.waterlevel;
	player.pev->maxspeed = from->client.maxspeed;
	player.pev->fov = from->client.fov;

	player.pev->weaponanim = from->client.weaponanim;
	player.pev->viewmodel = from->client.viewmodel;
	player.m_flNextAttack = from->client.m_flNextAttack;
	player.m_flNextAmmoBurn = from->client.fuser2;
	player.m_flAmmoStartCharge = from->client.fuser3;

	//Stores all our ammo info, so the client side weapons can use them.
	player.ammo_9mm = (int)from->client.vuser1[0];
	player.ammo_357 = (int)from->client.vuser1[1];
	player.ammo_argrens = (int)from->client.vuser1[2];
	player.ammo_bolts = (int)from->client.ammo_nails; //is an int anyways...
	player.ammo_buckshot = (int)from->client.ammo_shells;
	player.ammo_uranium = (int)from->client.ammo_cells;
	player.ammo_hornets = (int)from->client.vuser2[0];
	player.ammo_rockets = (int)from->client.ammo_rockets;

	if (player.m_pActiveItem && player.m_pActiveItem->m_iId == WEAPON_RPG) {
		CRpg* rpg = (CRpg*)player.m_pActiveItem;
		rpg->m_cActiveRockets = (int)from->client.vuser2[2];
	}

	if (cmd->impulse == 222)
		player.pev->button |= IN_ATTACK3; // not enough input bits for this, so using impulse as a toggle
}

// save the current state as the next state
void save_prediction_state(local_state_s* to, usercmd_t* cmd) {
	// Copy in results of prediction code
	to->client.viewmodel = player.pev->viewmodel;
	to->client.fov = player.pev->fov;
	to->client.weaponanim = player.pev->weaponanim;
	to->client.m_flNextAttack = player.m_flNextAttack;
	to->client.fuser2 = player.m_flNextAmmoBurn;
	to->client.fuser3 = player.m_flAmmoStartCharge;
	to->client.maxspeed = player.pev->maxspeed;

	//HL Weapons
	to->client.vuser1[0] = player.ammo_9mm;
	to->client.vuser1[1] = player.ammo_357;
	to->client.vuser1[2] = player.ammo_argrens;

	to->client.ammo_nails = player.ammo_bolts;
	to->client.ammo_shells = player.ammo_buckshot;
	to->client.ammo_cells = player.ammo_uranium;
	to->client.vuser2[0] = player.ammo_hornets;
	to->client.ammo_rockets = player.ammo_rockets;

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		CBasePlayerWeapon* pCurrent = g_pWpns[i];
		weapon_data_t* pto = &to->weapondata[i];

		if (!pCurrent)
		{
			memset(pto, 0, sizeof(weapon_data_t));
			continue;
		}

		pto->m_fInReload = pCurrent->m_fInReload;
		pto->m_fInSpecialReload = pCurrent->m_fInSpecialReload;
		//		pto->m_flPumpTime				= pCurrent->m_flPumpTime;
		pto->m_iClip = pCurrent->m_iClip;
		pto->m_flNextPrimaryAttack = pCurrent->m_flNextPrimaryAttack;
		pto->m_flNextSecondaryAttack = pCurrent->m_flNextSecondaryAttack;
		pto->fuser4 = pCurrent->m_flNextTertiaryAttack;
		pto->m_flTimeWeaponIdle = pCurrent->m_flTimeWeaponIdle;
		pto->fuser1 = pCurrent->pev->fuser1;
		pto->fuser2 = pCurrent->m_flStartThrow;
		pto->fuser3 = pCurrent->m_flReleaseThrow;
		pto->iuser1 = pCurrent->m_chargeReady;
		pto->iuser2 = pCurrent->m_fInAttack;
		pto->iuser3 = pCurrent->m_fireState;
		pto->iuser4 = pCurrent->m_iClip2;
		pto->m_fInZoom = (pCurrent->m_bDelayFire << 5) | (pCurrent->m_iDirection << 4) | (pCurrent->m_iShotsFired & 0xf);
		pto->m_fNextAimBonus = pCurrent->m_flNextShotsFiredDec;

		// Decrement weapon counters, server does this at same time ( during post think, after doing everything else )
		pto->m_flNextReload -= cmd->msec / 1000.0;
		pto->m_fNextAimBonus -= cmd->msec / 1000.0;
		pto->m_flNextPrimaryAttack -= cmd->msec / 1000.0;
		pto->m_flNextSecondaryAttack -= cmd->msec / 1000.0;
		pto->fuser4 -= cmd->msec / 1000.0;
		pto->m_flTimeWeaponIdle -= cmd->msec / 1000.0;
		pto->fuser1 -= cmd->msec / 1000.0;

		to->client.vuser3[2] = pCurrent->m_iSecondaryAmmoType;
		to->client.vuser4[0] = pCurrent->m_iPrimaryAmmoType;
		to->client.vuser4[1] = player.m_rgAmmo[pCurrent->m_iPrimaryAmmoType];
		to->client.vuser4[2] = player.m_rgAmmo[pCurrent->m_iSecondaryAmmoType];

		/*		if ( pto->m_flPumpTime != -9999 )
				{
					pto->m_flPumpTime -= cmd->msec / 1000.0;
					if ( pto->m_flPumpTime < -0.001 )
						pto->m_flPumpTime = -0.001;
				}*/

		if (pto->m_fNextAimBonus < -1.0)
		{
			pto->m_fNextAimBonus = -1.0;
		}

		if (pto->m_flNextPrimaryAttack < -1.0)
		{
			pto->m_flNextPrimaryAttack = -1.0;
		}

		if (pto->m_flNextSecondaryAttack < -0.001)
		{
			pto->m_flNextSecondaryAttack = -0.001;
		}

		if (pto->fuser4 < -0.001)
		{
			pto->fuser4 = -0.001;
		}

		if (pto->m_flTimeWeaponIdle < -0.001)
		{
			pto->m_flTimeWeaponIdle = -0.001;
		}

		if (pto->m_flNextReload < -0.001)
		{
			pto->m_flNextReload = -0.001;
		}

		if (pto->fuser1 < -0.001)
		{
			pto->fuser1 = -0.001;
		}
	}

	// m_flNextAttack is now part of the weapons, but is part of the player instead
	to->client.m_flNextAttack -= cmd->msec / 1000.0;
	if (to->client.m_flNextAttack < -0.001)
	{
		to->client.m_flNextAttack = -0.001;
	}

	to->client.fuser2 -= cmd->msec / 1000.0;
	if (to->client.fuser2 < -0.001)
	{
		to->client.fuser2 = -0.001;
	}

	to->client.fuser3 -= cmd->msec / 1000.0;
	if (to->client.fuser3 < -0.001)
	{
		to->client.fuser3 = -0.001;
	}
}

// handle weapon switching from client commands and from server logic
void handle_weapon_switching(local_state_s* from, local_state_s* to, usercmd_t* cmd,
	CBasePlayerItem* oldActiveItem, CBasePlayerWeapon* pWeapon, bool serverWeaponChanged)
{
	// Assume that we are not going to switch weapons
	to->client.m_iId = from->client.m_iId;

	// Now see if we issued a changeweapon command ( and we're not dead )
	if (cmd->weaponselect && (player.pev->deadflag != (DEAD_DISCARDBODY + 1)))
	{
		// Switched to a different weapon?
		if (from->weapondata[cmd->weaponselect].m_iId == cmd->weaponselect)
		{
			CBasePlayerWeapon* pNew = g_pWpns[cmd->weaponselect];
			if (pNew && (pNew != pWeapon))
			{
				// Put away old weapon
				if (player.m_pActiveItem) {
					player.m_pActiveItem->Holster();
				}

				player.m_pLastItem = player.m_pActiveItem;
				player.m_pActiveItem = pNew;

				// Deploy new weapon
				if (player.m_pActiveItem)
				{
					int oldRunfuncs = g_runfuncs;
					g_runfuncs = 1; // force the animation to play
					player.m_pActiveItem->Deploy();
					g_runfuncs = oldRunfuncs;

					CWeaponCustom* wcNew = pNew->MyWeaponCustomPtr();
					if (g_runfuncs && wcNew)
						wcNew->m_lastDeploy = gpGlobals->time;
				}

				// Update weapon id so we can predict things correctly.
				to->client.m_iId = cmd->weaponselect;
			}
		}
	}

	if (serverWeaponChanged) {
		// weapon changed via server logic
		// Put away old weapon
		if (oldActiveItem) {
			oldActiveItem->Holster();

			CWeaponCustom* wcOld = oldActiveItem->MyWeaponCustomPtr();
			if (g_runfuncs && wcOld)
				wcOld->m_lastDeploy = gpGlobals->time;
		}

		player.m_pLastItem = oldActiveItem;

		// Deploy new weapon
		if (player.m_pActiveItem) {
			int oldRunfuncs = g_runfuncs;
			g_runfuncs = 1; // force the animation to play
			player.m_pActiveItem->Deploy();
			g_runfuncs = oldRunfuncs;
		}
	}
}

// If we are running events/etc. go ahead and see if we
//  managed to die between last frame and this one
// If so, run the appropriate player killed or spawn function
void handle_player_death(local_state_s* to) {
	static int lasthealth;

	if (g_runfuncs)
	{
		if (to->client.health <= 0 && lasthealth > 0)
		{
			player.Killed(NULL, 0);
		}
		else if (to->client.health > 0 && lasthealth <= 0)
		{
			player.Spawn();
		}

		lasthealth = to->client.health;
	}
}

// Make sure that weapon animation matches what the game .dll is telling us
//  over the wire ( fixes some animation glitches )
void fix_desynced_weapon_anim(local_state_s* to, CBasePlayerWeapon* pWeapon) {
	if (g_runfuncs && (HUD_GetWeaponAnim() != to->client.weaponanim) && !pWeapon->IsSevenKewpWeapon())
	{
		int body = 2;

		//Pop the model to body 0.
		if (pWeapon == &g_Tripmine)
			body = 0;

		//Show laser sight/scope combo
		if (pWeapon == &g_Python && bIsMultiplayer())
			body = 1;

		// Force a fixed anim down to viewmodel
		HUD_SendWeaponAnim(to->client.weaponanim, body, 1);
	}
}

/*
=====================
HUD_WeaponsPostThink

Run Weapon firing code on client
=====================
*/
void HUD_WeaponsPostThink( local_state_s *from, local_state_s *to, usercmd_t *cmd, double time, unsigned int random_seed )
{
	HUD_InitClientWeapons();	

	CBasePlayerWeapon* pWeapon = GetPredictedWeapon(from->client.m_iId);
	CWeaponCustom* wc = pWeapon ? pWeapon->MyWeaponCustomPtr() : NULL;

	// Get current clock
	gpGlobals->time = time;

	// Store pointer to our destination entity_state_t so we can get our origin, etc. from it
	//  for setting up events on the client
	g_finalstate = to;

	handle_player_death(to);

	CBasePlayerItem* oldActiveItem = player.m_pActiveItem;
	bool serverWeaponChanged = !player.m_pActiveItem || player.m_pActiveItem->m_iId != to->client.m_iId;

	// Point to current weapon object
	player.m_pActiveItem = from->client.m_iId ? g_pWpns[from->client.m_iId] : NULL;

	if (cmd->buttons & (IN_ATTACK2 | IN_ATTACK)) {
		g_prediction.last_attack_mode = (cmd->buttons & IN_ATTACK2) ? 2 : 1;
		g_prediction.last_attack_time = gEngfuncs.GetClientTime();
	}

	if (!pWeapon || (wc && !wc->m_hasPredictionData)) {
		if (oldActiveItem && oldActiveItem != wc) {
			oldActiveItem->Holster(); // otherwise not called when switching from a predicted weapon
		}

		return; // We are not predicting the current weapon, just bow out here.
	}

	load_prediction_state(from, cmd, random_seed);

	bool weaponModelDeployed = player.pev->viewmodel || (wc && wc->GetActiveParams().vsprite_path);
	bool isDead = player.pev->deadflag == (DEAD_DISCARDBODY + 1) || CL_IsDead();
	if (!isDead && weaponModelDeployed && !g_iUser1 && player.m_flNextAttack <= 0)
	{
		pWeapon->ItemPostFrame(); // Run shared weapon code
	}

	handle_weapon_switching(from, to, cmd, oldActiveItem, pWeapon, serverWeaponChanged);
	
	// TODO: Why is this saving to "from"?
	if (player.m_pActiveItem && player.m_pActiveItem->m_iId == WEAPON_RPG)
	{
		from->client.vuser2[1] = ((CRpg*)player.m_pActiveItem)->m_fSpotActive;
		from->client.vuser2[2] = ((CRpg*)player.m_pActiveItem)->m_cActiveRockets;
	}
	
	save_prediction_state(to, cmd);	

	fix_desynced_weapon_anim(to, pWeapon);
	HUD_SetLastOrg(); // Store off the last position from the predicted state.

	// Wipe it so we can't use it after this frame
	g_finalstate = NULL;
}

// share prediction state with the rest of the code
void update_shared_prediction_state(struct local_state_s* to) {
	if (!g_runfuncs)
		return;

	memcpy(&g_prediction.local, to, sizeof(local_state_s));
	memset(&g_prediction.weapon, 0, sizeof(g_prediction.weapon));

	g_prediction.fov = to->client.fov;

	int id = to->client.m_iId;
	if (id < 0 || id >= MAX_WEAPONS)
		return;

	CWeaponCustom& wc = g_customWeapon[id];
	if (!wc.m_hasPredictionData)
		return;

	PredictionWeaponState& wstate = g_prediction.weapon;

	wstate.isCustom = true;
	wstate.body = wc.pev->body;
	wstate.ironSights = wc.IsIronSights();
	wstate.canAkimbo = wc.CanAkimbo();
	wstate.dynamicAccuracy = wc.GetFlag(FL_WC_WEP_DYNAMIC_ACCURACY);
	wc.events.GetCurrentAccuracy(
		wstate.accuracyX[0], wstate.accuracyY[0], wstate.accuracyX[1], wstate.accuracyY[1]);

	wstate.akimboSeq = wc.m_akimboAnim;
	wstate.akimboAnimTime = wc.m_akimboAnimTime;
	wstate.akimboLastEventFrame = &wc.m_akimboLastEventFrame;
	wstate.isAkimbo = wc.IsAkimbo();

	wstate.clip1 = wc.m_iClip;
	wstate.clip2 = wc.m_iClip2;
	wstate.ammo1 = wc.m_iPrimaryAmmoType >= 0 ? player.m_rgAmmo[wc.m_iPrimaryAmmoType] : 0;
	wstate.ammo2 = wc.m_iSecondaryAmmoType >= 0 ? player.m_rgAmmo[wc.m_iSecondaryAmmoType] : 0;
	wstate.akimboClip = g_customWeapon[id].GetAkimboClip();

	wstate.v_sprite = &wc.m_viewModelSpr;
	wstate.v_model = wc.GetActiveViewModelIdx();
	wstate.stateSpriteIdx = wc.GetFlag(FL_WC_WEP_HAS_STATE_SPRITE) ? wc.m_stateIconIdx : -1;
	wstate.params = &wc.GetActiveParams();
}

// update the player's CMD time and the CMD time history
void update_cmd_time(struct usercmd_s* cmd, int random_seed) {
	if (g_runfuncs && random_seed > g_latest_cmd_id && cmd) {
		g_cmd_timer += cmd->msec;

		g_cmd_timer_idx = random_seed % CMD_TIMER_HIST_SZ;
		g_cmd_timer_hist[g_cmd_timer_idx] = g_cmd_timer;
	}

	player.m_cmdTime = GetTimeAtCmd(random_seed);

	// random seed increases by 1 for each new command, so this can function as cmd ID
	g_cmd_id = random_seed;
	g_latest_cmd_id = V_max(g_latest_cmd_id, random_seed);
}

// troubleshoot prediction by logging info at each step
void debug_cmd(struct local_state_s* from, struct usercmd_s* cmd, double time, unsigned int random_seed) {
	static int replaySeq;
	static float lastTime;
	if (time > lastTime)
		replaySeq++;
	else {
		replaySeq = 0;
	}
	lastTime = time;

	// Set fps_max to 20 for a readable console with this enabled.
	// This helps debug and understand how predicted/replayed commands work.
	// msec times don't flucuate but PostRunCmd time does. "to" data feeds into "from" for the next command.
	bool buttonsPressed = cmd->buttons & (IN_ATTACK | IN_ATTACK2);
	const float holdtime = 0.2f;
	static float lastButtons;
	g_cmd_debug_mode = gEngfuncs.GetClientTime() - lastButtons < holdtime || buttonsPressed;
	if (g_cmd_debug_mode) {
		const char* sep = "------------------------------------------------------------\n";
		int test = from->weapondata[37].iuser3;
		PRINTF("%s> CMD %-5d   MSEC %d   T %f   V %d%s\n",
			replaySeq == 0 ? sep : "",
			random_seed,
			cmd->msec,
			(float)time,
			test,
			g_runfuncs ? "  RUNFUNC" : "");
		if (buttonsPressed)
			lastButtons = gEngfuncs.GetClientTime();
	}
}

// apply velocity from predicted state for movement code
void handle_instant_velocity_change(struct local_state_s* to) {
	if (g_runningKickbackPred == 1)
	{
		to->client.velocity = to->client.velocity + g_vApplyVel;
		g_runningKickbackPred = 0;
	}

	if (g_irunninggausspred == 1)
	{
		Vector forward;
		gEngfuncs.pfnAngleVectors(gPlayerSim.v_angles, forward, NULL, NULL);
		to->client.velocity = to->client.velocity - forward * g_flApplyVel;
		g_irunninggausspred = false;
	}
}

/*
=====================
HUD_PostRunCmd

Client calls this during prediction, after it has moved the player and updated any info changed into to->
time is the current client clock based on prediction
cmd is the command that caused the movement, etc
runfuncs is 1 if this is the first time we've predicted this command.  If so, sounds and effects should play, otherwise, they should
be ignored
=====================
*/
void CL_DLLEXPORT HUD_PostRunCmd(struct local_state_s* from, struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed)
{
	g_runfuncs = runfuncs;
	
	update_cmd_time(cmd, random_seed);
	//debug_cmd(from, cmd, time, random_seed);

#if defined( CLIENT_WEAPONS )
	if ( cl_lw && cl_lw->value )
	{
		HUD_WeaponsPostThink( from, to, cmd, time, random_seed );
	}
	else
#endif
	{
		to->client.fov = g_prediction.fov;
	}

	handle_instant_velocity_change(to);
	update_shared_prediction_state(to);
}
