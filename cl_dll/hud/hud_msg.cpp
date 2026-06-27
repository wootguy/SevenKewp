/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
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
//  hud_msg.cpp
//

#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "r_efx.h"
#include "camera.h"
#include "mstream.h"
#include "com_weapons.h"
#include "input.h"

#include "particleman.h"
extern IParticleMan *g_pParticleMan;

#define MAX_CLIENTS 32

#if !defined( _TFC )
extern BEAM *pBeam;
extern BEAM *pBeam2;
extern TEMPENTITY* pFlare;	// Vit_amiN
extern TEMPENTITY* pLaserDot;
#endif 

#if defined( _TFC )
void ClearEventList( void );
#endif

/// USER-DEFINED SERVER MESSAGE HANDLERS

int CHud::MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf )
{
	ASSERT( iSize == 0 );

	// clear all hud data
	HUDLIST *pList = m_pHudList;

	while ( pList )
	{
		if ( pList->p )
			pList->p->Reset();
		pList = pList->pNext;
	}

	// reset sensitivity
	m_flMouseSensitivity = 0;

	// reset concussion effect
	m_iConcussionEffect = 0;

	return 1;
}

int CHud::MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf )
{
	CAM_ToFirstPerson();
	return 1;
}

int CHud::MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
{
	// prepare all hud data
	HUDLIST *pList = m_pHudList;

	while (pList)
	{
		if ( pList->p )
			pList->p->InitHUDData();
		pList = pList->pNext;
	}

#if defined( _TFC )
	ClearEventList();

	// catch up on any building events that are going on
	gEngfuncs.pfnServerCmd("sendevents");
#endif

	if ( g_pParticleMan )
		 g_pParticleMan->ResetParticles();

#if !defined( _TFC )
	//Probably not a good place to put this.
	pBeam = pBeam2 = NULL;
	pFlare = pLaserDot = NULL;
#endif

	return 1;
}

int CHud::MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_Teamplay = READ_BYTE();

	if ( m_Teamplay )
		gEngfuncs.pfnClientCmd("richpresence_gamemode Teamplay\n");
	else
		gEngfuncs.pfnClientCmd("richpresence_gamemode\n"); // reset

	gEngfuncs.pfnClientCmd("richpresence_update\n");

	return 1;
}

int CHud::MsgFunc_Damage(const char *pszName, int iSize, void *pbuf )
{
	int		armor, blood;
	Vector	from;
	int		i;
	float	count;
	
	BEGIN_READ( pbuf, iSize );
	armor = READ_BYTE();
	blood = READ_BYTE();

	for (i=0 ; i<3 ; i++)
		from[i] = READ_COORD();

	count = (blood * 0.5) + (armor * 0.5);

	if (count < 10)
		count = 10;

	// TODO: kick viewangles,  show damage visually

	return 1;
}

int CHud::MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iConcussionEffect = READ_BYTE();
	if (m_iConcussionEffect)
		this->m_StatusIcons.EnableIcon("dmg_concuss",255,160,0);
	else
		this->m_StatusIcons.DisableIcon("dmg_concuss");
	return 1;
}

int CHud::MsgFunc_TagInfo(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);
	uint8_t cl = READ_BYTE();
	uint32_t health = UTIL_DecompressUint(READ_SHORT());
	uint32_t max_health = UTIL_DecompressUint(READ_SHORT());
	uint32_t armor = UTIL_DecompressUint(READ_SHORT());
	uint8_t observer = READ_BYTE();

	if (cl > 0 && cl <= MAX_PLAYERS)
	{
		g_PlayerExtraInfo[cl].health = health;
		g_PlayerExtraInfo[cl].max_health = max_health;
		g_PlayerExtraInfo[cl].armor = armor;
		g_PlayerExtraInfo[cl].specMode = observer & 0x7;
		g_PlayerExtraInfo[cl].specTarget = observer >> 3;

		// not needed yet because hp isn't displayed anywhere in VGUI yet
		//UpdateOnPlayerInfo();
	}

	return 1;
}

int CHud::MsgFunc_PlayerPos(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	static uint8_t tagData[32 * 8];
	mstream dat((char*)tagData, 32 * 8);

	int sz = READ_BYTE();
	for (int i = 0; i < sz; i++) {
		tagData[i] = READ_BYTE();
	}

	for (int i = 0; i < 32; i++) {
		if (!dat.readBit())
			continue;

		extra_player_info_t& info = g_PlayerExtraInfo[i + 1];

		if (dat.readBit())
			info.x = SIGN_EXTEND_FIXED(dat.readBits(13), 13) * 8;
		if (dat.readBit())
			info.y = SIGN_EXTEND_FIXED(dat.readBits(13), 13) * 8;
		if (dat.readBit())
			info.z = SIGN_EXTEND_FIXED(dat.readBits(13), 13) * 8;
	}

	return 1;
}

int CHud::MsgFunc_HudColor(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	uint32_t r = READ_BYTE();
	uint32_t g = READ_BYTE();
	uint32_t b = READ_BYTE();

	m_sv_hud_color = (r << 16) | (g << 8) | b;

	return 1;
}

int CHud::MsgFunc_SetFOV(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int newfov = READ_BYTE();
	int def_fov = CVAR_GET_FLOAT("default_fov");

	//Weapon prediction already takes care of changing the fog. ( g_lastFOV ).
	if (cl_lw && cl_lw->value)
		return 1;

	g_lastFOV = newfov;

	if (newfov == 0)
	{
		m_iFOV = def_fov;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if (m_iFOV == def_fov)
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = IN_GetMouseSensitivity() * ((float)newfov / (float)def_fov) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	return 1;
}


#define CUSTOM_MESSAGES() \
    X(Logo)		\
    X(ResetHUD)	\
    X(GameMode)	\
    X(InitHUD)	\
    X(ViewMode)	\
    X(Concuss)	\
    X(TagInfo)	\
    X(PlayerPos)\
    X(HudColor)	\
    X(SetFOV)	\

#define X(x) int __MsgFunc_##x(const char *pszName, int iSize, void *pbuf) \
	{ return gHUD.MsgFunc_##x(pszName, iSize, pbuf ); }
CUSTOM_MESSAGES()
#undef X

void CHud::HookHudMessages() {
#define X HOOK_MESSAGE
	CUSTOM_MESSAGES()
#undef X
}