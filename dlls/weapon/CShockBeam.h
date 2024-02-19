
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
#ifndef WEAPONS_CSHOCKBEAM_H
#define WEAPONS_CSHOCKBEAM_H

#include "CGrenade.h"

//TODO: this class needs save/restore for the effects so they can be cleaned up if a save occurs while this is in flight
class CShockBeam : public CGrenade
{
public:
	using BaseClass = CGrenade;

	void Precache() override;

	void Spawn() override;

	int Classify() override { return CBaseMonster::Classify(CLASS_NONE); }

	void EXPORT FlyThink();

	void EXPORT ExplodeThink();

	void EXPORT WaterExplodeThink();

	void EXPORT BallTouch( CBaseEntity* pOther );

	static CShockBeam* CreateShockBeam( const Vector& vecOrigin, const Vector& vecAngles, CBaseEntity* pOwner );

	const char* GetDeathNoticeWeapon() { return "weapon_9mmAR"; }

#ifndef CLIENT_DLL
	int Save(CSave& save) override;
	int Restore(CRestore& restore) override;

	static TYPEDESCRIPTION m_SaveData[];
#endif

private:
	void Explode();

public:
	EHANDLE m_hBeam1;
	EHANDLE m_hBeam2;

	EHANDLE m_hSprite;

	int m_iBeams;
};

#endif
