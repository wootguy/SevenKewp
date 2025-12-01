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

#include "CPipewrench.h"

#define PIPEWRENCH_BODYHIT_VOLUME 128
#define PIPEWRENCH_WALLHIT_VOLUME 512

enum PipewrenchAnim
{
	PWRENCH_IDLE1 = 0,
	PWRENCH_IDLE2,
	PWRENCH_IDLE3,
	PWRENCH_DRAW,
	PWRENCH_HOLSTER,
	PWRENCH_HIT1,
	PWRENCH_MISS1,
	PWRENCH_HIT2,
	PWRENCH_MISS2,
	PWRENCH_HIT3,
	PWRENCH_MISS3,
	PWRENCH_BIGWIND,
	PWRENCH_BIGHIT,
	PWRENCH_BIGMISS,
	PWRENCH_BIGLOOP,
};

/*
BEGIN_DATAMAP(CPipewrench)
DEFINE_FIELD(m_flBigSwingStart, FIELD_TIME),
	DEFINE_FIELD(m_iSwing, FIELD_INTEGER),
	DEFINE_FIELD(m_iSwingMode, FIELD_INTEGER),
	DEFINE_FUNCTION(SwingAgain),
	DEFINE_FUNCTION(Smack),
	DEFINE_FUNCTION(BigSwing),
	END_DATAMAP();
*/

LINK_ENTITY_TO_CLASS(weapon_pipewrench, CPipewrench)

void CPipewrench::Spawn()
{
	Precache();
	m_iId = WEAPON_PIPEWRENCH;
	SetWeaponModelW();
	m_iClip = WEAPON_NOCLIP;
	m_iDefaultAmmo = DISPLACER_DEFAULT_GIVE;

	FallInit();// get ready to fall down.
}

void CPipewrench::Precache()
{
	m_hasHandModels = true;
	m_defaultModelV = "models/v_pipe_wrench.mdl";
	m_defaultModelP = "models/p_pipe_wrench.mdl";
	m_defaultModelW = "models/w_pipe_wrench.mdl";
	CBasePlayerWeapon::Precache();

	PRECACHE_SOUND("weapons/pwrench_big_hit1.wav");
	PRECACHE_SOUND("weapons/pwrench_big_hit2.wav");
	PRECACHE_SOUND("weapons/pwrench_big_hitbod1.wav");
	PRECACHE_SOUND("weapons/pwrench_big_hitbod2.wav");
	PRECACHE_SOUND("weapons/pwrench_big_miss.wav");
	PRECACHE_SOUND("weapons/pwrench_hit1.wav");
	PRECACHE_SOUND("weapons/pwrench_hit2.wav");
	PRECACHE_SOUND("weapons/pwrench_hitbod1.wav");
	PRECACHE_SOUND("weapons/pwrench_hitbod2.wav");
	PRECACHE_SOUND("weapons/pwrench_hitbod3.wav");
	PRECACHE_SOUND("weapons/pwrench_miss1.wav");
	PRECACHE_SOUND("weapons/pwrench_miss2.wav");

	//m_usPipewrench = PRECACHE_EVENT(1, "events/pipewrench.sc");

	// client-side HUD sprites and config
	PRECACHE_HUD_FILES("sprites/weapon_pipewrench.txt");
}

BOOL CPipewrench::Deploy()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return FALSE;

	return DefaultDeploy(GetModelV(), GetModelP(), PIPEWRENCH_DRAW, "crowbar");
}

void CPipewrench::Holster(int skiplocal)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	// Cancel any swing in progress.
	m_iSwingMode = SWING_NONE;
	SetThink(nullptr);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim(PIPEWRENCH_HOLSTER);
}

void CPipewrench::PrimaryAttack()
{
	if (m_iSwingMode == SWING_NONE && !Swing(true))
	{
#ifndef CLIENT_DLL
		SetThink(&CPipewrench::SwingAgain);
		pev->nextthink = gpGlobals->time + 0.1;
#endif
	}
}

void CPipewrench::SecondaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_iSwingMode != SWING_START_BIG)
	{
		SendWeaponAnim(PIPEWRENCH_BIG_SWING_START);
		m_flBigSwingStart = gpGlobals->time;

		
		if (m_pPlayer->m_playerModelAnimSet != PMODEL_ANIMS_HALF_LIFE) {
			strcpy_safe(m_pPlayer->m_szAnimExtention, "wrench", 32);
			strcpy_safe(m_pPlayer->m_szAnimAction, "cock", 32);
		}
		m_pPlayer->SetAnimation(PLAYER_COCK_WEAPON);
	}
	else if (m_pPlayer->m_playerModelAnimSet == PMODEL_ANIMS_HALF_LIFE) {
		// reset the cocking animation if interrupted 
		if (m_pPlayer->m_Activity == ACT_WALK) {
			m_pPlayer->SetAnimation(PLAYER_COCK_WEAPON);
			m_pPlayer->pev->frame = 1; // last frame estimate
			m_pPlayer->SetAnimation(PLAYER_WALK); // find the actual last frame
		}
	}

	m_iSwingMode = SWING_START_BIG;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = GetNextAttackDelay(0.1);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2;
}

void CPipewrench::Smack()
{
	DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
}

void CPipewrench::SwingAgain()
{
	Swing(false);
}

bool CPipewrench::Swing(const bool bFirst)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return false;

	bool bDidHit = false;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

#ifndef CLIENT_DLL
	SolidifyNearbyCorpses(false);

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, m_pPlayer->edict(), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos; // This is the point on the actual surface (the hull could have hit space)
		}
	}

	m_pPlayer->WaterSplashTrace(vecSrc, 32, head_hull, 0.4f);

	SolidifyNearbyCorpses(true);
#endif

	if (bFirst)
	{
		/*
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usPipewrench,
			0.0, g_vecZero, g_vecZero, 0, 0, 0,
			0.0, 0, static_cast<int>(tr.flFraction < 1));
			*/
	}


	if (tr.flFraction >= 1.0)
	{
		if (bFirst)
		{
			// miss
			m_flNextPrimaryAttack = GetNextAttackDelay(0.75);
			m_flNextSecondaryAttack = GetNextAttackDelay(0.75);
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;

			// Shepard - In Opposing Force, the "miss" sound is
			// played twice (maybe it's a mistake from Gearbox or
			// an intended feature), if you only want a single
			// sound, comment this "switch" or the one in the
			// event (EV_Pipewrench)
			/*
			switch ( ((m_iSwing++) % 1) )
			{
			case 0: m_pPlayer->EmitSound(CHAN_ITEM, "weapons/pwrench_miss1.wav", 1, ATTN_NORM); break;
			case 1: m_pPlayer->EmitSound(CHAN_ITEM, "weapons/pwrench_miss2.wav", 1, ATTN_NORM); break;
			}
			*/

			PlayerModelSwingAnim();

			switch (((m_iSwing++) % 2) + 1)
			{
			case 0:
				SendWeaponAnim(PWRENCH_MISS1);
				break;
			case 1:
				SendWeaponAnim(PWRENCH_MISS2);
				break;
			case 2:
				SendWeaponAnim(PWRENCH_MISS3);
				break;
			}

			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/pwrench_miss1.wav", 1, ATTN_NORM);
				break;
			case 1:
				EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/pwrench_miss2.wav", 1, ATTN_NORM);
				break;
			}
		}
	}
	else
	{
		switch (((m_iSwing++) % 2) + 1)
		{
		case 0:
			SendWeaponAnim(PIPEWRENCH_ATTACK1HIT);
			break;
		case 1:
			SendWeaponAnim(PIPEWRENCH_ATTACK2HIT);
			break;
		case 2:
			SendWeaponAnim(PIPEWRENCH_ATTACK3HIT);
			break;
		}

		PlayerModelSwingAnim();

#ifndef CLIENT_DLL

		// hit
		bDidHit = true;
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool bHitWorld = true;

		if (pEntity)
		{
			ClearMultiDamage();

			float flDamage = gSkillData.sk_plr_pipewrench / 2;

			if ((m_flNextPrimaryAttack + 1 < UTIL_WeaponTimeBase()) || gSkillData.sk_plr_pipewrench_full_damage != 0)
			{
				// only first swing does full damage
				flDamage = gSkillData.sk_plr_pipewrench;
			}

			bool repairable = (m_pPlayer->IRelationship(pEntity) == R_AL && pEntity->IsMachine())
				|| (pEntity->IsBreakable() && (pEntity->m_breakFlags & FL_BREAK_REPAIRABLE));

			if (repairable) {
				flDamage *= -1;
				bHitWorld = false;

				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_big_hit1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				case 1:
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_big_hit2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				}
			}

			if (repairable) {
				pEntity->TakeHealth(-GetDamage(flDamage), DMG_CLUB, pEntity->pev->max_health);
			}
			else {
				pEntity->TraceAttack(m_pPlayer->pev, GetDamage(flDamage), gpGlobals->v_forward, &tr, DMG_CLUB);
			}

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
		}

#endif

		//if (GetSkillFloat("chainsaw_melee") == 0)
		{
			m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
			m_flNextSecondaryAttack = GetNextAttackDelay(0.5);
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;

#ifndef CLIENT_DLL

		if (pEntity)
		{
			bool suitableClass = pEntity->Classify() != CLASS_NONE || pEntity->IsPlayerCorpse();
			if (suitableClass && !pEntity->IsMachine() && !pEntity->IsBSPModel())
			{
				// play thwack or smack sound
				switch (RANDOM_LONG(0, 2))
				{
				case 0:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hitbod1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hitbod2.wav", 1, ATTN_NORM);
					break;
				case 2:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hitbod3.wav", 1, ATTN_NORM);
					break;
				}
				m_pPlayer->m_iWeaponVolume = PIPEWRENCH_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return true;
				else
					flVol = 0.1;

				bHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (bHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer,
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play pipe wrench strike
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}
			

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * PIPEWRENCH_WALLHIT_VOLUME;

		SetThink(&CPipewrench::Smack);
		pev->nextthink = gpGlobals->time + 0.2;
#endif

		//if (GetSkillFloat("chainsaw_melee") != 0)
		{
			m_flNextPrimaryAttack = GetNextAttackDelay(0.5);
			m_flNextSecondaryAttack = GetNextAttackDelay(0.5);
		}
	}
	return bDidHit;
}

void CPipewrench::BigSwing()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	TraceResult tr;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

#ifndef CLIENT_DLL
	SolidifyNearbyCorpses(false);

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);

	if (tr.flFraction >= 1.0)
	{
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, m_pPlayer->edict(), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
			if (!pHit || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
			vecEnd = tr.vecEndPos; // This is the point on the actual surface (the hull could have hit space)
		}
	}

	m_pPlayer->WaterSplashTrace(vecSrc, 32, head_hull, 0.6f);

	SolidifyNearbyCorpses(true);
#endif

	//PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usPipewrench,
	//	0.0, g_vecZero, g_vecZero, 0, 0, 0,
	//	0.0, 1, static_cast<int>(tr.flFraction < 1));

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/pwrench_big_miss.wav", VOL_NORM, ATTN_NORM, 0, 94 + RANDOM_LONG(0, 15));

	if (tr.flFraction >= 1.0)
	{
		// miss
		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = GetNextAttackDelay(1.0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;

		SendWeaponAnim(PIPEWRENCH_BIG_SWING_MISS);
	}
	else
	{
		SendWeaponAnim(PIPEWRENCH_BIG_SWING_HIT);

#ifndef CLIENT_DLL

		// hit
		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		// play thwack, smack, or dong sound
		float flVol = 1.0;
		bool bHitWorld = true;

		if (pEntity)
		{
			ClearMultiDamage();

			// Charge wrench for 2.5s for full damage. Does 50% damage with no charge.
			const float minAttackTime = 1.1f;
			float chargeTime = (gpGlobals->time - m_flBigSwingStart) - minAttackTime;
			float multiplier = V_min(chargeTime*0.2f + 0.5f, 1.0f);
			float flDamage = multiplier * gSkillData.sk_plr_pipewrench_full_damage;

			if (m_pPlayer->IRelationship(pEntity) == R_AL && (pEntity->IsMachine() || pEntity->IsBSPModel())) {
				flDamage *= -1;
				bHitWorld = false;

				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_big_hit1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				case 1:
					EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_big_hit2.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				}
			}

			pEntity->TraceAttack(m_pPlayer->pev, GetDamage(flDamage), gpGlobals->v_forward, &tr, DMG_CLUB);

			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);
		}

		if (pEntity)
		{
			bool suitableClass = pEntity->Classify() != CLASS_NONE || pEntity->IsPlayerCorpse();
			if (suitableClass && !pEntity->IsMachine() && !pEntity->IsBSPModel())
			{
				// play thwack or smack sound
				// (not using the big hit sound because it lags behind the impact effect)
				/*
				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_big_hitbod1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_big_hitbod2.wav", 1, ATTN_NORM);
					break;
				}
				*/
				switch (RANDOM_LONG(0, 2))
				{
				case 0:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hitbod1.wav", 1, ATTN_NORM);
					break;
				case 1:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hitbod2.wav", 1, ATTN_NORM);
					break;
				case 2:
					EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hitbod3.wav", 1, ATTN_NORM);
					break;
				}

				m_pPlayer->m_iWeaponVolume = PIPEWRENCH_BODYHIT_VOLUME;
				if (!pEntity->IsAlive())
					return;
				else
					flVol = 0.1;

				bHitWorld = false;
			}
		}

		// play texture hit sound
		// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

		if (bHitWorld)
		{
			float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

			if (g_pGameRules->IsMultiplayer())
			{
				// override the volume here, cause we don't play texture sounds in multiplayer,
				// and fvolbar is going to be 0 from the above call.

				fvolbar = 1;
			}

			// also play pipe wrench strike
			// Shepard - The commented sounds below are unused
			// in Opposing Force, if you wish to use them,
			// uncomment all the appropriate lines.
			switch (RANDOM_LONG(0, 1))
			{
			case 0:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hit1.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			case 1:
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/pwrench_hit2.wav", fvolbar, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
				break;
			}

			// delay the decal a bit
			m_trHit = tr;
		}

		m_pPlayer->m_iWeaponVolume = flVol * PIPEWRENCH_WALLHIT_VOLUME;

		// Shepard - The original Opposing Force's pipe wrench
		// doesn't make a bullet hole decal when making a big
		// swing. If you want that decal, just uncomment the
		// 2 lines below.
		/*SetThink( &CPipewrench::Smack );
		SetNextThink( UTIL_WeaponTimeBase() + 0.2 );*/
#endif
		m_flNextPrimaryAttack = GetNextAttackDelay(1.0);
		m_flNextSecondaryAttack = GetNextAttackDelay(1.0);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
	}

	PlayerModelSwingAnim();
	m_iSwingMode = SWING_NONE;
}

void CPipewrench::WeaponIdle()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iSwingMode == SWING_START_BIG)
	{
		if (gpGlobals->time > m_flBigSwingStart + 1)
		{
			m_iSwingMode = SWING_DOING_BIG;

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2;
			SetThink(&CPipewrench::BigSwing);
			pev->nextthink = gpGlobals->time + 0.1;
		}
	}
	else
	{
		m_iSwingMode = SWING_NONE;
		int iAnim;
		float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

		if (flRand <= 0.5)
		{
			iAnim = PIPEWRENCH_IDLE3;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}
		else if (flRand <= 0.9)
		{
			iAnim = PIPEWRENCH_IDLE1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0;
		}
		else
		{
			iAnim = PIPEWRENCH_IDLE2;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0;
		}

		SendWeaponAnim(iAnim);
	}
}

void CPipewrench::PlayerModelSwingAnim() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;
	
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	if (m_pPlayer->m_playerModelAnimSet != PMODEL_ANIMS_HALF_LIFE) {
		strcpy_safe(m_pPlayer->m_szAnimExtention, "crowbar", 32);
		strcpy_safe(m_pPlayer->m_szAnimAction, "aim", 32);
	}
}

int CPipewrench::GetItemInfo(ItemInfo* p)
{
	// hack to force client to load HUD config from the hlcoop folder
	p->pszName = MOD_SPRITE_FOLDER "weapon_pipewrench";

	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = WEAPON_PIPEWRENCH;
	p->iWeight = PIPEWRENCH_WEIGHT;
	return 1;
}
