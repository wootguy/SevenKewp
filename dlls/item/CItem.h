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
#ifndef CITEM_H
#define CITEM_H

extern int gmsgItemPickup;

class CItem : public CBaseEntity
{
public:
	void	Spawn(void);
	void	KeyValue(KeyValueData* pkvd);
	CBaseEntity* Respawn(void);
	void	EXPORT ItemTouch(CBaseEntity* pOther);
	void	EXPORT Materialize(void);
	void	EXPORT ItemUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual BOOL MyTouch(CBasePlayer* pPlayer) { return FALSE; };
	virtual BOOL ShouldRespawn();
	const char* GetModel();
	void SetSize(Vector defaultMins, Vector defaultMaxs);

	const char* m_defaultModel;

	Vector m_minHullSize;
	Vector m_maxHullSize;
};

#endif // ITEMS_H
