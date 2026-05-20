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
#define MEDKIT_REVIVE_TIME 2.0f

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
	m_hasHandModels = true;
	m_defaultModelV = "models/v_medkit.mdl";
	m_defaultModelP = "models/p_medkit.mdl";
	m_defaultModelW = "models/w_pmedkit.mdl";
	CBasePlayerWeapon::Precache();

	PRECACHE_SOUND("items/medshot4.wav");
	PRECACHE_SOUND("items/medshotno1.wav");
	PRECACHE_SOUND("items/suitchargeok1.wav");
	PRECACHE_SOUND("weapons/electro4.wav");

	m_reviveSpriteIdx = PRECACHE_MODEL("sprites/saveme.spr");

	PRECACHE_HUD_FILES("sprites/weapon_medkit.txt");
}

int CMedkit::GetItemInfo(ItemInfo *p)
{
	p->pszName = MOD_SPRITE_FOLDER "weapon_medkit";
	p->pszAmmo1 = "health";
	p->pszAmmo2 = NULL;
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

	h_reviveTarget = NULL;

	return FALSE;
}

void CMedkit::Holster( int skiplocal /* = 0 */ )
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	SendWeaponAnim( MEDKIT_HOLSTER );
	CancelRevive();
}

void CMedkit::WeaponIdle()
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	RechargeAmmo();
	CancelRevive();

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

void CMedkit::ItemPostFrame(void) {
	CBasePlayerWeapon::ItemPostFrame();

	if (m_nextSpriteHint > gpGlobals->time)
		return;

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_nextSpriteHint = gpGlobals->time + 0.2f;

	edict_t* ed = m_pPlayer->edict();
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (!plr || plr == m_pPlayer || plr->IsAlive() || (plr->pev->effects & EF_NODRAW))
			continue;

		Vector pos = plr->pev->origin + Vector(0, 0, -8);
		const int flags = 1 | 2 | 4 | 8;
		const int scale = 8;
		const int fps = 10;

		if (UTIL_TestPVS(pos, ed)) {
			UTIL_ExplosionMsg(pos, m_reviveSpriteIdx, scale, fps, flags, MSG_ONE_UNRELIABLE, ed);
		}
	}
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

	healAmount = (int)V_min(ammoLeft, ceilf(healAmount));

	if (mon->IsAlive() && CanHealTarget(mon) && healAmount > 0) {
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		SendWeaponAnim(MEDKIT_SHORTUSE);
		m_pPlayer->m_iWeaponVolume = MEDKIT_VOLUME;

		mon->TakeHealth(healAmount, DMG_MEDKITHEAL);
		mon->GiveScorePoints(m_pPlayer->pev, -healAmount);

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

	CBaseEntity* bestPlayerCorpse = NULL;
	CBaseMonster* bestTarget = NULL;
	float bestDist = FLT_MAX;
	float bestPlayerCorpseDist = FLT_MAX;

	CBaseEntity* ent = NULL;
	while ((ent = UTIL_FindEntityInSphere(ent, m_pPlayer->pev->origin, MEDKIT_REVIVE_RADIUS)) != 0) {
		CBaseMonster* mon = ent ? ent->MyMonsterPointer() : NULL;
		
		if (!mon || mon->IsAlive() || (!mon->IsNormalMonster() && !mon->IsPlayer() && !mon->IsPlayerCorpse()) || mon->IsMachine()) {
			continue;
		}

		if (mon->IsPlayer() && (mon->pev->iuser1 || (mon->pev->effects & EF_NODRAW))) {
			continue; // don't revive spectators nor gibs
		}

		float dist = (mon->pev->origin - m_pPlayer->pev->origin).Length();
		
		if (mon->IsPlayerCorpse()) {
			if (dist < bestPlayerCorpseDist) {
				bestPlayerCorpseDist = dist;
				bestPlayerCorpse = mon;
			}
			continue;
		}

		if (bestTarget == NULL) {
			bestDist = dist;
			bestTarget = mon;
			continue;
		}
		
		// prefer reviving players over monsters, which sometimes have death poses far from where they died
		bool isBetterClass = mon->IsPlayer() && !bestTarget->IsPlayer();
		bool isWorseClass = !mon->IsPlayer() && bestTarget->IsPlayer();

		if ((dist < bestDist && !isWorseClass) || isBetterClass) {
			bestDist = dist;
			bestTarget = mon;
		}
	}

	if (!bestTarget) {
		EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "items/medshotno1.wav", 1, ATTN_NORM);
		m_flNextSecondaryAttack = GetNextAttackDelay(0.5f);
		CancelRevive();

		if (bestPlayerCorpse) {
			CBasePlayer* owner = UTIL_PlayerByIndex(bestPlayerCorpse->pev->renderamt);
			const char* name = owner ? owner->DisplayName() : "\\disconnected\\";
			UTIL_ClientPrint(m_pPlayer, print_center,
				UTIL_VarArgs("The spirit of\n%s\nhas left this body\n", name));
		}

		return;
	}

	if (bestTarget->m_isFadingOut) {
		bestTarget->pev->renderamt = 255;
		bestTarget->pev->nextthink = gpGlobals->time + 2.0f;
	}

	h_reviveTarget = bestTarget;

	CBasePlayer* bestPlr = bestTarget->MyPlayerPointer();
	if (bestPlr && !bestPlr->m_reviveAttempted) {
		// extend the respawn time so the player has time to react to the revive noise
		// while still giving them to reject the revive
		bestPlr->m_reviveAttempted = true;
		bestPlr->m_tempRespawnDelay = MEDKIT_REVIVE_TIME / 2;

		float deadTime = gpGlobals->time - bestPlr->m_killedTime;
		float respawnDelay = mp_respawndelay.value + bestPlr->m_extraRespawnDelay + bestPlr->m_tempRespawnDelay;
		if (deadTime > respawnDelay) {
			// prevent the player respawning instantly if they've already waited out the timer
			bestPlr->m_killedTime = gpGlobals->time - (respawnDelay - bestPlr->m_tempRespawnDelay);
		}
	}

	if (!m_reviveChargedTime) {
		SendWeaponAnim(MEDKIT_LONGUSE);
		m_reviveChargedTime = gpGlobals->time + MEDKIT_REVIVE_TIME;
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "items/suitchargeok1.wav", 1, ATTN_NORM);
		return;
	}

	if (m_nextMessageTime < gpGlobals->time) {
		m_nextMessageTime = gpGlobals->time + 0.05f;
		std::string progress = "[";
		const int barCount = 37;
		float timeLeft = m_reviveChargedTime - gpGlobals->time;
		float t = 1.0f - (timeLeft / MEDKIT_REVIVE_TIME);
		int idx = t * barCount;
		for (int i = 0; i < barCount; i++) {
			if (i > idx) {
				progress += " ";
			}
			else {
				progress += "|";
			}
		}
		progress += "]";
		UTIL_ClientPrint(m_pPlayer, print_center, UTIL_VarArgs("Reviving %s\n%s\n",
			bestTarget->DisplayName(), progress.c_str()));

		CBasePlayer* plr = bestTarget->MyPlayerPointer();
		if (plr) {
			UTIL_ClientPrint(bestTarget, print_center, UTIL_VarArgs("%s is reviving you\n%s\n",
				m_pPlayer->DisplayName(), progress.c_str()));
			plr->m_lastSpawnMessage = gpGlobals->time; // don't also show the respawn timer message
		}	
	}

	if (m_reviveChargedTime < gpGlobals->time) {
		UTIL_ClientPrint(m_pPlayer, print_center, "");
		UTIL_ClientPrint(bestTarget, print_center, "");

		m_reviveChargedTime = 0;
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		SendWeaponAnim(MEDKIT_SHORTUSE);
		EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "weapons/electro4.wav", 1, ATTN_NORM);
		m_flNextSecondaryAttack = GetNextAttackDelay(2.0f);

		bestTarget->Revive();
		bestTarget->pev->health = V_min(bestTarget->pev->max_health, 50);

		bestTarget->GiveScorePoints(m_pPlayer->pev, -bestTarget->pev->health);

		CTalkSquadMonster* talkMon = bestTarget->MyTalkSquadMonsterPointer();
		if (talkMon) {
			talkMon->UnprovokeFriends();
		}

		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= gSkillData.sk_plr_medkit_revive_cost;
	}
}

void CMedkit::CancelRevive() {
	if (!m_reviveChargedTime) {
		h_reviveTarget = NULL;
		return;
	}

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	UTIL_ClientPrint(m_pPlayer, print_center, "Revive cancelled\n");

	if (h_reviveTarget) {
		CBaseEntity* reviveTarget = h_reviveTarget.GetEntity();
		CBasePlayer* revivePlr = reviveTarget ? reviveTarget->MyPlayerPointer() : NULL;

		if (revivePlr) {
			if (revivePlr->IsAlive()) {
				// respawned normally
				UTIL_ClientPrint(h_reviveTarget.GetEntity(), print_center, "");
			}
			else {
				UTIL_ClientPrint(h_reviveTarget.GetEntity(), print_center,
					UTIL_VarArgs("%s stopped reviving you", m_pPlayer->DisplayName()));
			}

			// so you can't hit-and-run players you don't like with an increased delay
			revivePlr->m_tempRespawnDelay = 0;
		}

		h_reviveTarget = NULL;
	}

	m_reviveChargedTime = 0;
	m_flNextSecondaryAttack = GetNextAttackDelay(0.5f);
	EMIT_SOUND(m_pPlayer->edict(), CHAN_ITEM, "items/medshotno1.wav", 1, ATTN_NORM);
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