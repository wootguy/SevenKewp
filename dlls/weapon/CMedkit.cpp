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

#include "extdll.h"
#include "util.h"
#include "skill.h"
#include "weapons.h"
#include "CBasePlayer.h"
#include "gamerules.h"
#include "CMedkit.h"
#include "monsters.h"
#include "CTalkSquadMonster.h"

LINK_ENTITY_TO_CLASS( weapon_medkit, CMedkit )

enum medkit_e {
	MEDKIT_IDLE = 0,
	MEDKIT_LONGIDLE,
	MEDKIT_LONGUSE,
	MEDKIT_SHORTUSE,
	MEDKIT_HOLSTER,
	MEDKIT_DRAW,
};

#define MEDKIT_VOLUME 128
#define MEDKIT_REVIVE_RADIUS 64

void CMedkit::Spawn( )
{
	pev->classname = MAKE_STRING("weapon_medkit"); // hack to allow for alternate names

	Precache( );
	m_iId = WEAPON_MEDKIT;
	SetWeaponModelW();
	m_iClip = -1;

	FallInit();// get ready to fall down.
}


void CMedkit::Precache( void )
{
	m_defaultModelV = "models/v_medkit.mdl";
	m_defaultModelP = "models/p_medkit.mdl";
	m_defaultModelW = "models/w_pmedkit.mdl";
	CBasePlayerWeapon::Precache();

	PRECACHE_SOUND("items/medshot4.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
	PRECACHE_SOUND("items/suitchargeok1.wav");
	PRECACHE_SOUND("weapons/electro4.wav");

	PRECACHE_HUD_FILES("sprites/weapon_medkit.txt");
}

int CMedkit::GetItemInfo(ItemInfo *p)
{
	p->pszName = MOD_SPRITE_FOLDER "weapon_medkit";
	p->pszAmmo1 = "health";
	p->iMaxAmmo1 = gSkillData.sk_ammo_max_medkit;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = WEAPON_MEDKIT;
	p->iWeight = MEDKIT_WEIGHT;
	p->iFlags = ITEM_FLAG_NOAUTORELOAD | ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY;
	return 1;
}



BOOL CMedkit::Deploy( )
{
	if (DefaultDeploy(GetModelV(), GetModelP(), MEDKIT_DRAW, "trip")) {
		RechargeAmmo();
		return TRUE;
	}

	return FALSE;
}

void CMedkit::Holster( int skiplocal /* = 0 */ )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim( MEDKIT_HOLSTER );
}

void CMedkit::WeaponIdle()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	RechargeAmmo();

	if (m_reviveChargedTime) {
		m_reviveChargedTime = 0;
		m_flNextSecondaryAttack = GetNextAttackDelay(0.5f);
		EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "items/medshotno1.wav", 1, ATTN_NORM);
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;
	float flRand = UTIL_SharedRandomFloat(m_pPlayer->random_seed, 0.0, 1.0);

	if (flRand <= 0.2)
	{
		iAnim = MEDKIT_IDLE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2*2;
	}
	else
	{
		iAnim = MEDKIT_LONGIDLE;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.4*2;
	}

	SendWeaponAnim(iAnim);
}

void CMedkit::PrimaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	RechargeAmmo();

	TraceResult tr;
	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();

	Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

	lagcomp_begin(m_pPlayer);

	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);
	if (tr.flFraction >= 1.0) {
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
	}

	lagcomp_end();

	CBaseMonster* mon = tr.pHit ? CBaseEntity::Instance(tr.pHit)->MyMonsterPointer() : NULL;
	int ammoLeft = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];

	if (!mon || ammoLeft <= 0) {
		return;
	}

	float healAmount = V_min(gSkillData.sk_plr_hpmedic, mon->pev->max_health - mon->pev->health);

	// slowly lower pitch
	if (ammoLeft < gSkillData.sk_plr_hpmedic*2) {
		healAmount = V_min(gSkillData.sk_plr_hpmedic*0.5f, healAmount);
	}
	else if (ammoLeft < gSkillData.sk_plr_hpmedic) {
		healAmount = V_min(gSkillData.sk_plr_hpmedic*0.2f, healAmount);
	}

	healAmount = (int)V_min(ammoLeft, healAmount);

	if (mon->IsAlive() && CanHealTarget(mon) && healAmount > 0) {
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		SendWeaponAnim(MEDKIT_SHORTUSE);
		m_pPlayer->m_iWeaponVolume = MEDKIT_VOLUME;

		mon->TakeHealth(healAmount, DMG_MEDKITHEAL);
		CTalkSquadMonster* talkMon = mon->MyTalkSquadMonsterPointer();
		if (talkMon) {
			talkMon->Unprovoke(true);
		}

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= healAmount;

		int pitch = 100;
		if (ammoLeft < gSkillData.sk_plr_hpmedic * 2) {
			pitch = ((float)ammoLeft / (gSkillData.sk_plr_hpmedic*2)) * 20.5f + 80;
		}
		
		EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "items/medshot4.wav", 1, ATTN_NORM, 0, pitch);

		m_flNextPrimaryAttack = GetNextAttackDelay(0.5f);
	}
}

void CMedkit::SecondaryAttack()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	RechargeAmmo();

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= gSkillData.sk_plr_medkit_revive_cost) {
		EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "items/medshotno1.wav", 1, ATTN_NORM);
		m_flNextSecondaryAttack = GetNextAttackDelay(0.5f);
		m_reviveChargedTime = 0;
		return;
	}

	CBaseMonster* bestTarget = NULL;
	float bestDist = FLT_MAX;

	CBaseEntity* ent = NULL;
	while ((ent = UTIL_FindEntityInSphere(ent, m_pPlayer->pev->origin, MEDKIT_REVIVE_RADIUS)) != 0) {
		CBaseMonster* mon = ent ? ent->MyMonsterPointer() : NULL;
		
		if (!mon || mon->IsAlive() || (!mon->IsNormalMonster() && !mon->IsPlayer()) || mon->IsMachine()) {
			continue;
		}
		
		float dist = (mon->pev->origin - m_pPlayer->pev->origin).Length();

		if (dist < bestDist) {
			bestDist = dist;
			bestTarget = mon;
		}
	}

	if (!bestTarget) {
		EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "items/medshotno1.wav", 1, ATTN_NORM);
		m_flNextSecondaryAttack = GetNextAttackDelay(0.5f);
		m_reviveChargedTime = 0;
		return;
	}

	if (bestTarget->m_isFadingOut) {
		bestTarget->pev->renderamt = 255;
		bestTarget->pev->nextthink = gpGlobals->time + 2.0f;
	}

	if (!m_reviveChargedTime) {
		SendWeaponAnim(MEDKIT_LONGUSE);
		m_reviveChargedTime = gpGlobals->time + 2.0f;
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM);
		return;
	}

	if (m_reviveChargedTime < gpGlobals->time) {
		m_reviveChargedTime = 0;
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		SendWeaponAnim(MEDKIT_SHORTUSE);
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/electro4.wav", 1, ATTN_NORM);
		m_flNextSecondaryAttack = GetNextAttackDelay(2.0f);
		bestTarget->Revive();

		bestTarget->pev->health = V_min(bestTarget->pev->max_health, 50);

		CTalkSquadMonster* talkMon = bestTarget->MyTalkSquadMonsterPointer();
		if (talkMon) {
			talkMon->UnprovokeFriends();
		}

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= gSkillData.sk_plr_medkit_revive_cost;
	}
}

bool CMedkit::CanHealTarget(CBaseEntity* ent) {
	CBaseMonster* mon = ent ? ent->MyMonsterPointer() : NULL;

	if (!mon) {
		return false;
	}

	if (mon->pev->health >= mon->pev->max_health) {
		return false;
	}

	return true;
}

void CMedkit::GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
	ammoEntName = "ammo_medkit";
	dropAmount = 10;
}

int CMedkit::AddToPlayer(CBasePlayer* pPlayer) {
	if (CBasePlayerWeapon::AddToPlayer(pPlayer)) {
		if (gSkillData.sk_plr_medkit_recharge_delay) {
			m_rechargeTime = gpGlobals->time + gSkillData.sk_plr_medkit_recharge_delay;
		}
		return TRUE;
	}

	return FALSE;
}

void CMedkit::RechargeAmmo() {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (m_rechargeTime) {
		while (m_rechargeTime < gpGlobals->time) {
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] + 1;

			if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > gSkillData.sk_ammo_max_medkit) {
				m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = gSkillData.sk_ammo_max_medkit;
				m_rechargeTime = gpGlobals->time + gSkillData.sk_plr_medkit_recharge_delay;
				break;
			}
				
			m_rechargeTime += gSkillData.sk_plr_medkit_recharge_delay;
		}
	}
}