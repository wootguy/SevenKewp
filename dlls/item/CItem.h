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

// constant items
#define ITEM_HEALTHKIT		1
#define ITEM_ANTIDOTE		2
#define ITEM_SECURITY		3
#define ITEM_BATTERY		4

#define MAX_NORMAL_BATTERY	100

class EXPORT CItem : public CBaseAnimating
{
public:
	void	Spawn(void);
	void KeyValue(KeyValueData* pkvd);
	CBaseEntity* Respawn(void);
	void ItemTouch(CBaseEntity* pOther);
	void Materialize(void);
	void ItemUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual BOOL MyTouch(CBasePlayer* pPlayer) { return FALSE; };
	virtual BOOL ShouldRespawn();
	const char* GetModel();
	void SetSize(Vector defaultMins, Vector defaultMaxs);
	void SetItemModel();
	virtual int MergedModelBody() { return -1; }

	const char* m_defaultModel;
	string_t m_sequence_name;

	Vector m_minHullSize;
	Vector m_maxHullSize;

	float m_flCustomRespawnTime;
};

#endif // ITEMS_H
