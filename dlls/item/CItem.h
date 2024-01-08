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
	CBaseEntity* Respawn(void);
	void	EXPORT ItemTouch(CBaseEntity* pOther);
	void	EXPORT Materialize(void);
	virtual BOOL MyTouch(CBasePlayer* pPlayer) { return FALSE; };
	const char* GetModel();

	const char* m_defaultModel;
};

#endif // ITEMS_H
