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

#include "particleman.h"
extern IParticleMan *g_pParticleMan;

#define MAX_CLIENTS 32

#if !defined( _TFC )
extern BEAM *pBeam;
extern BEAM *pBeam2;
#endif 

#if defined( _TFC )
void ClearEventList( void );
#endif

/// USER-DEFINED SERVER MESSAGE HANDLERS

int CHud :: MsgFunc_ResetHUD(const char *pszName, int iSize, void *pbuf )
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

void CAM_ToFirstPerson(void);

void CHud :: MsgFunc_ViewMode( const char *pszName, int iSize, void *pbuf )
{
	CAM_ToFirstPerson();
}

void CHud :: MsgFunc_InitHUD( const char *pszName, int iSize, void *pbuf )
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
#endif
}


int CHud :: MsgFunc_GameMode(const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_Teamplay = READ_BYTE();

	return 1;
}


int CHud :: MsgFunc_Damage(const char *pszName, int iSize, void *pbuf )
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

int CHud :: MsgFunc_Concuss( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize );
	m_iConcussionEffect = READ_BYTE();
	if (m_iConcussionEffect)
		this->m_StatusIcons.EnableIcon("dmg_concuss",255,160,0);
	else
		this->m_StatusIcons.DisableIcon("dmg_concuss");
	return 1;
}

void ToxicCloudCallback(tempent_s* tempent, float frametime, float curtime) {
	tempent->entity.origin = tempent->entity.origin + tempent->entity.curstate.velocity*frametime;
	tempent->entity.curstate.scale = fminf(15.0f, tempent->entity.curstate.scale + 10*frametime);
	tempent->entity.angles.z += 100*frametime;

	float timeLeft = tempent->die - curtime;
	if (timeLeft < 1) {
		tempent->entity.curstate.renderamt = 255 * timeLeft;
	}
}

#include "com_weapons.h"

int CHud::MsgFunc_ToxicCloud(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int idx = READ_SHORT();
	int seed = READ_BYTE();
	float colorRand = READ_BYTE() * 0.01f;

	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(idx);

	if (ent) {
		int modelindex;
		model_s* model = gEngfuncs.CL_LoadModel("sprites/puff1.spr", &modelindex);
		Vector pos = ent->origin;

		tempent_s* temp = gEngfuncs.pEfxAPI->CL_TentEntAllocCustom(&pos.x, model, 1, ToxicCloudCallback);
		temp->die = gEngfuncs.GetClientTime() + 3.0f;

		float x = UTIL_SharedRandomLong(seed, -100, 100)*0.01f;
		seed = UTIL_SharedRandomLong(seed, 0, 255);
		float y = UTIL_SharedRandomLong(seed, -100, 100)*0.01f;
		seed = UTIL_SharedRandomLong(seed, 0, 255);
		float speed = UTIL_SharedRandomLong(seed, 60, 90);

		// TODO: lighting is calculated for each particle in sven, every frame, but maybe that's too expensive?
		float redScale = ent->cvFloorColor.r / 255.0;
		float blueScale = ent->cvFloorColor.b / 255.0f;
		
		temp->entity.curstate.velocity = Vector(x, y, 1.0f) * speed;

		temp->entity.curstate.scale = 0.01;
		temp->entity.curstate.renderamt = 255;
		temp->entity.curstate.rendermode = kRenderTransAlpha;
		temp->entity.curstate.rendercolor.r = 240 * colorRand * redScale;
		temp->entity.curstate.rendercolor.b = 255 * colorRand * blueScale;
	}

	return 1;
}
