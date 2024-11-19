#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "weapons.h"
#include "nodes.h"
#include "env/CSoundEnt.h"
#include "decals.h"
#include "gamerules.h"
#include "CBasePlayerWeapon.h"
#include "user_messages.h"
#include "pm_shared.h"

TYPEDESCRIPTION	CBasePlayerWeapon::m_SaveData[] =
{
#if defined( CLIENT_WEAPONS )
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_FLOAT),
#else	// CLIENT_WEAPONS
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextPrimaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_flNextSecondaryAttack, FIELD_TIME),
	DEFINE_FIELD(CBasePlayerWeapon, m_flTimeWeaponIdle, FIELD_TIME),
#endif	// CLIENT_WEAPONS
	DEFINE_FIELD(CBasePlayerWeapon, m_iPrimaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iSecondaryAmmoType, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iClip, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayerWeapon, m_iDefaultAmmo, FIELD_INTEGER),
	//	DEFINE_FIELD( CBasePlayerWeapon, m_iClientClip, FIELD_INTEGER )	 , reset to zero on load so hud gets updated correctly
	//  DEFINE_FIELD( CBasePlayerWeapon, m_iClientWeaponState, FIELD_INTEGER ), reset to zero on load so hud gets updated correctly
};

IMPLEMENT_SAVERESTORE(CBasePlayerWeapon, CBasePlayerItem)

BOOL CanAttack(float attack_time, float curtime, BOOL isPredicted)
{
#if defined( CLIENT_WEAPONS )
	if (!isPredicted)
#else
	if (1)
#endif
	{
		return (attack_time <= curtime) ? TRUE : FALSE;
	}
	else
	{
		return (attack_time <= 0.0) ? TRUE : FALSE;
	}
}

void CBasePlayerWeapon::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wpn_v_model"))
	{
		m_customModelV = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if(FStrEq(pkvd->szKeyName, "wpn_p_model"))
	{
		m_customModelP = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wpn_w_model"))
	{
		m_customModelW = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBasePlayerItem::KeyValue(pkvd);
}

void CBasePlayerWeapon::Precache() {
	if (GetModelV())
		PRECACHE_MODEL(GetModelV());
	if (GetModelW())
		PRECACHE_MODEL(GetModelW());
	if (GetModelP())
		PRECACHE_MODEL(GetModelP());
}

void CBasePlayerWeapon::GetAmmoDropInfo(bool isSecondary, const char*& ammoEntName, int& dropAmount) {
	ammoEntName = NULL;
	dropAmount = 0;

	if (isSecondary) {
		const char* ammoName2 = pszAmmo2();

		if (!ammoName2) {
			return;
		}

		if (!strcmp(ammoName2, "ARgrenades")) {
			ammoEntName = "ammo_ARgrenades";
			dropAmount = 2;
		}
		else {
			ALERT(at_console, "Weapon %s should implement GetAmmoDropInfo\n", STRING(pev->classname));
		}

		return;
	}

	const char* ammoName = pszAmmo1();

	if (!ammoName) {
		return;
	}

	if (!strcmp(ammoName, "9mm")) {
		ammoEntName = "ammo_9mmAR";
		dropAmount = 50;
	}
	else if (!strcmp(ammoName, "357")) {
		ammoEntName = "ammo_357";
		dropAmount = 6;
	}
	else if (!strcmp(ammoName, "buckshot")) {
		ammoEntName = "ammo_buckshot";
		dropAmount = 12;
	}
	else if (!strcmp(ammoName, "bolts")) {
		ammoEntName = "ammo_crossbow";
		dropAmount = 5;
	}
	else if (!strcmp(ammoName, "rockets")) {
		ammoEntName = "ammo_rpgclip";
		dropAmount = 1;
	}
	else if (!strcmp(ammoName, "uranium")) {
		ammoEntName = "ammo_gaussclip";
		dropAmount = 20;
	}
	else if (!strcmp(ammoName, "Hand Grenade")) {
		ammoEntName = "weapon_handgrenade";
		dropAmount = 5;
	}
	else if (!strcmp(ammoName, "Satchel Charge")) {
		ammoEntName = "weapon_satchel";
		dropAmount = 1;
	}
	else if (!strcmp(ammoName, "Trip Mine")) {
		ammoEntName = "weapon_tripmine";
		dropAmount = 1;
	}
	else if (!strcmp(ammoName, "Snarks")) {
		ammoEntName = "weapon_snark";
		dropAmount = 5;
	}
	else if (!strcmp(ammoName, "spores")) {
		ammoEntName = "ammo_sporeclip";
		dropAmount = 1;
	}
	else if (!strcmp(ammoName, "Hornets") || !strcmp(ammoName, "shock")) {
		// can't drop ammo for these weapons
	}
	else {
		ALERT(at_console, "Weapon %s should implement GetAmmoDropInfo\n", STRING(pev->classname));
	}
}

void CBasePlayerWeapon::ItemPostFrame(void)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if ((m_fInReload) && (m_pPlayer->m_flNextAttack <= UTIL_WeaponTimeBase()))
	{
		// complete the reload. 
		int j = V_min(iMaxClip() - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;

		m_pPlayer->TabulateAmmo();

		m_fInReload = FALSE;
	}

	if (!(m_pPlayer->pev->button & IN_ATTACK))
	{
		m_flLastFireTime = 0.0f;
	}

	if ((m_pPlayer->pev->button & IN_ATTACK2) && CanAttack(m_flNextSecondaryAttack, gpGlobals->time, UseDecrement()))
	{
		if (pszAmmo2() && !m_pPlayer->m_rgAmmo[SecondaryAmmoIndex()])
		{
			m_fFireOnEmpty = TRUE;
		}

		m_pPlayer->TabulateAmmo();
		SecondaryAttack();
		m_pPlayer->pev->button &= ~IN_ATTACK2;
	}
	else if ((m_pPlayer->pev->button & IN_ATTACK) && CanAttack(m_flNextPrimaryAttack, gpGlobals->time, UseDecrement()))
	{
		if ((m_iClip == 0 && pszAmmo1()) || (iMaxClip() == -1 && !m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()]))
		{
			m_fFireOnEmpty = TRUE;
		}

		m_pPlayer->TabulateAmmo();
		PrimaryAttack();
	}
	else if (m_pPlayer->pev->button & IN_RELOAD && iMaxClip() != WEAPON_NOCLIP && !m_fInReload)
	{
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reload();
	}
	else if (!(m_pPlayer->pev->button & (IN_ATTACK | IN_ATTACK2)))
	{
		// no fire buttons down

		m_fFireOnEmpty = FALSE;

		if (!IsUseable() && m_flNextPrimaryAttack < (UseDecrement() ? 0.0 : gpGlobals->time))
		{
			// weapon isn't useable, switch.
			if (!(iFlags() & ITEM_FLAG_NOAUTOSWITCHEMPTY) && g_pGameRules->GetNextBestWeapon(m_pPlayer, this))
			{
				m_flNextPrimaryAttack = (UseDecrement() ? 0.0 : gpGlobals->time) + 0.3;
				return;
			}
		}
		else
		{
			// weapon is useable. Reload if empty and weapon has waited as long as it has to after firing
			if (m_iClip == 0 && !(iFlags() & ITEM_FLAG_NOAUTORELOAD) && m_flNextPrimaryAttack < (UseDecrement() ? 0.0 : gpGlobals->time))
			{
				Reload();
				return;
			}
		}

		WeaponIdle();
		return;
	}

	// catch all
	if (ShouldWeaponIdle())
	{
		WeaponIdle();
	}
}

// CALLED THROUGH the newly-touched weapon's instance. The existing player weapon is pOriginal
int CBasePlayerWeapon::AddDuplicate(CBasePlayerItem* pOriginal)
{
	if (m_iDefaultAmmo)
	{
		return ExtractAmmo((CBasePlayerWeapon*)pOriginal);
	}
	else
	{
		// a dead player dropped this.
		return ExtractClipAmmo((CBasePlayerWeapon*)pOriginal);
	}
}


int CBasePlayerWeapon::AddToPlayer(CBasePlayer* pPlayer)
{
	int bResult = CBasePlayerItem::AddToPlayer(pPlayer);

	pPlayer->pev->weapons |= (1 << m_iId);

	if (!m_iPrimaryAmmoType)
	{
		m_iPrimaryAmmoType = pPlayer->GetAmmoIndex(pszAmmo1());
		m_iSecondaryAmmoType = pPlayer->GetAmmoIndex(pszAmmo2());
	}


	if (bResult && AddWeapon()) {
		MESSAGE_BEGIN(MSG_ONE, gmsgWeapPickup, NULL, pPlayer->pev);
		WRITE_BYTE(m_iId);
		MESSAGE_END();

		return TRUE;
	}

	return FALSE;
}

int CBasePlayerWeapon::UpdateClientData(CBasePlayer* pPlayer)
{
	BOOL bSend = FALSE;
	int state = 0;
	if (pPlayer->m_pActiveItem.GetEntity() == this)
	{
		if (pPlayer->m_fOnTarget)
			state = WEAPON_IS_ONTARGET;
		else
			state = 1;
	}

	// Forcing send of all data!
	if (!pPlayer->m_fWeapon)
	{
		bSend = TRUE;
	}

	// This is the current or last weapon, so the state will need to be updated
	if (this == pPlayer->m_pActiveItem.GetEntity() ||
		this == pPlayer->m_pClientActiveItem.GetEntity())
	{
		if (pPlayer->m_pActiveItem.GetEntity() != pPlayer->m_pClientActiveItem.GetEntity())
		{
			bSend = TRUE;
		}
	}

	// If the ammo, state, or fov has changed, update the weapon
	if (m_iClip != m_iClientClip ||
		state != m_iClientWeaponState ||
		pPlayer->m_iFOV != pPlayer->m_iClientFOV)
	{
		bSend = TRUE;
	}

	if (bSend)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pPlayer->pev);
		WRITE_BYTE(state);
		WRITE_BYTE(m_iId);
		WRITE_BYTE(m_iClip);
		MESSAGE_END();

		m_iClientClip = m_iClip;
		m_iClientWeaponState = state;
		pPlayer->m_fWeapon = TRUE;
	}

	if (m_pNext)
		((CBasePlayerItem*)m_pNext.GetEntity())->UpdateClientData(pPlayer);

	return 1;
}


void CBasePlayerWeapon::SendWeaponAnim(int iAnim, int skiplocal, int body)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	if (UseDecrement())
		skiplocal = 1;
	else
		skiplocal = 0;

	m_pPlayer->pev->weaponanim = iAnim;

#if defined( CLIENT_WEAPONS )
	if (skiplocal && IsClientWeapon() && ENGINE_CANSKIP(m_pPlayer->edict()))
		return;
#endif

	MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, m_pPlayer->pev);
	WRITE_BYTE(iAnim);						// sequence number
	WRITE_BYTE(pev->body);					// weaponmodel bodygroup.
	MESSAGE_END();

	// play animation for spectators
	for (int i = 1; i < gpGlobals->maxClients; i++) {
		CBasePlayer* spec = (CBasePlayer*)UTIL_PlayerByIndex(i);

		if (spec && spec->pev->iuser1 == OBS_IN_EYE && spec->m_hObserverTarget.GetEntity() == m_pPlayer) {
			MESSAGE_BEGIN(MSG_ONE, SVC_WEAPONANIM, NULL, spec->pev);
			WRITE_BYTE(iAnim);						// sequence number
			WRITE_BYTE(pev->body);					// weaponmodel bodygroup.
			MESSAGE_END();
		}
	}
}

BOOL CBasePlayerWeapon::AddPrimaryAmmo(int iCount, char* szName, int iMaxClip, int iMaxCarry)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	int iIdAmmo;

	if (iMaxClip < 1)
	{
		m_iClip = -1;
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);
	}
	else if (m_iClip == 0)
	{
		int i;
		i = V_min(m_iClip + iCount, iMaxClip) - m_iClip;
		m_iClip += i;
		iIdAmmo = m_pPlayer->GiveAmmo(iCount - i, szName, iMaxCarry);
	}
	else
	{
		iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMaxCarry);
	}

	// m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = iMaxCarry; // hack for testing

	if (iIdAmmo > 0)
	{
		m_iPrimaryAmmoType = iIdAmmo;
		if (m_pPlayer->HasPlayerItem(this))
		{
			// play the "got ammo" sound only if we gave some ammo to a player that already had this gun.
			// if the player is just getting this gun for the first time, DefaultTouch will play the "picked up gun" sound for us.
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
	}

	return iIdAmmo > 0 ? TRUE : FALSE;
}


BOOL CBasePlayerWeapon::AddSecondaryAmmo(int iCount, char* szName, int iMax)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	int iIdAmmo;

	iIdAmmo = m_pPlayer->GiveAmmo(iCount, szName, iMax);

	//m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = iMax; // hack for testing

	if (iIdAmmo > 0)
	{
		m_iSecondaryAmmoType = iIdAmmo;
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
	}
	return iIdAmmo > 0 ? TRUE : FALSE;
}

//=========================================================
// IsUseable - this function determines whether or not a 
// weapon is useable by the player in its current state. 
// (does it have ammo loaded? do I have any ammo for the 
// weapon?, etc)
//=========================================================
BOOL CBasePlayerWeapon::IsUseable(void)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	if (m_iClip <= 0)
	{
		if (m_pPlayer->m_rgAmmo[PrimaryAmmoIndex()] <= 0 && iMaxAmmo1() != -1)
		{
			// clip is empty (or nonexistant) and the player has no more ammo of this type. 
			return CanDeploy();
		}
	}

	return TRUE;
}

BOOL CBasePlayerWeapon::CanDeploy(void)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	BOOL bHasAmmo = 0;

	if (!pszAmmo1())
	{
		// this weapon doesn't use ammo, can always deploy.
		return TRUE;
	}

	if (pszAmmo1())
	{
		bHasAmmo |= (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0);
	}
	if (pszAmmo2())
	{
		bHasAmmo |= (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0);
	}
	if (m_iClip > 0)
	{
		bHasAmmo |= 1;
	}
	if (!bHasAmmo)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CBasePlayerWeapon::DefaultDeploy(const char* szViewModel, const char* szWeaponModel, int iAnim, const char* szAnimExt, int skiplocal /* = 0 */, int body)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	if (!CanDeploy())
		return FALSE;

	m_pPlayer->TabulateAmmo();
	m_pPlayer->pev->viewmodel = MAKE_STRING(GET_MODEL(szViewModel));
	m_pPlayer->pev->weaponmodel = MAKE_STRING(GET_MODEL(szWeaponModel));
	strcpy_safe(m_pPlayer->m_szAnimExtention, szAnimExt, 32);
	SendWeaponAnim(iAnim, skiplocal, body);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0;
	m_flLastFireTime = 0.0;

	return TRUE;
}


BOOL CBasePlayerWeapon::DefaultReload(int iClipSize, int iAnim, float fDelay, int body)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return FALSE;

	int j = V_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

	if (j == 0)
		return FALSE;

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + fDelay;

	//!!UNDONE -- reload sound goes here !!!
	SendWeaponAnim(iAnim, UseDecrement() ? 1 : 0);

	m_fInReload = TRUE;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3;
	return TRUE;
}

BOOL CBasePlayerWeapon::PlayEmptySound(void)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	if (m_iPlayEmptySound)
	{
#ifndef CLIENT_DLL
		// send sound to all players except the shooter, who is predicting the sound locally
		edict_t* plr = m_pPlayer->edict();
		uint32_t messageTargets = 0xffffffff & ~PLRBIT(plr);
		StartSound(plr, CHAN_WEAPON, "weapons/357_cock1.wav", 0.8f,
			ATTN_NORM, SND_FL_PREDICTED, 100, m_pPlayer->pev->origin, messageTargets);
#endif
		m_iPlayEmptySound = 0;
		return 0;
	}
	return 0;
}

void CBasePlayerWeapon::ResetEmptySound(void)
{
	m_iPlayEmptySound = 1;
}

//=========================================================
//=========================================================
int CBasePlayerWeapon::PrimaryAmmoIndex(void)
{
	return m_iPrimaryAmmoType;
}

//=========================================================
//=========================================================
int CBasePlayerWeapon::SecondaryAmmoIndex(void)
{
	return -1;
}

void CBasePlayerWeapon::Holster(int skiplocal /* = 0 */)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	m_fInReload = FALSE; // cancel any reload in progress.
	m_pPlayer->pev->viewmodel = 0;
	m_pPlayer->pev->weaponmodel = 0;
}

//=========================================================
// called by the new item with the existing item as parameter
//
// if we call ExtractAmmo(), it's because the player is picking up this type of weapon for 
// the first time. If it is spawned by the world, m_iDefaultAmmo will have a default ammo amount in it.
// if  this is a weapon dropped by a dying player, has 0 m_iDefaultAmmo, which means only the ammo in 
// the weapon clip comes along. 
//=========================================================
int CBasePlayerWeapon::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	int			iReturn = 0;

	if (pszAmmo1() != NULL)
	{
		// blindly call with m_iDefaultAmmo. It's either going to be a value or zero. If it is zero,
		// we only get the ammo in the weapon's clip, which is what we want. 
		iReturn = pWeapon->AddPrimaryAmmo(m_iDefaultAmmo, (char*)pszAmmo1(), iMaxClip(), iMaxAmmo1());
		m_iDefaultAmmo = 0;
	}

	if (pszAmmo2() != NULL)
	{
		iReturn = pWeapon->AddSecondaryAmmo(0, (char*)pszAmmo2(), iMaxAmmo2());
	}

	return iReturn;
}

//=========================================================
// called by the new item's class with the existing item as parameter
//=========================================================
int CBasePlayerWeapon::ExtractClipAmmo(CBasePlayerWeapon* pWeapon)
{
	int			iAmmo;

	if (m_iClip == WEAPON_NOCLIP)
	{
		iAmmo = 0;// guns with no clips always come empty if they are second-hand
	}
	else
	{
		iAmmo = m_iClip;
	}

	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return 0;

	return m_pPlayer->GiveAmmo(iAmmo, pszAmmo1(), iMaxAmmo1()); // , &m_iPrimaryAmmoType
}

//=========================================================
// RetireWeapon - no more ammo for this gun, put it away.
//=========================================================
void CBasePlayerWeapon::RetireWeapon(void)
{
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	// first, no viewmodel at all.
	m_pPlayer->pev->viewmodel = iStringNull;
	m_pPlayer->pev->weaponmodel = iStringNull;
	//m_pPlayer->pev->viewmodelindex = NULL;

	g_pGameRules->GetNextBestWeapon(m_pPlayer, this);
}

CBaseEntity* CBasePlayerWeapon::Respawn(void)
{
	CBaseEntity* pNewWeapon = CBasePlayerItem::Respawn();

	CBasePlayerWeapon* wep = pNewWeapon->GetWeaponPtr();
	if (wep) {
		wep->m_customModelV = m_customModelV;
		wep->m_customModelP = m_customModelP;
		wep->m_customModelW = m_customModelW;
		wep->pev->movetype = pev->movetype;
		SET_MODEL(wep->edict(), GetModelW());
	}

	return pNewWeapon;
}

//=========================================================================
// GetNextAttackDelay - An accurate way of calcualting the next attack time.
//=========================================================================
float CBasePlayerWeapon::GetNextAttackDelay(float delay)
{
	if (m_flLastFireTime == 0 || m_flNextPrimaryAttack == -1)
	{
		// At this point, we are assuming that the client has stopped firing
		// and we are going to reset our book keeping variables.
		m_flLastFireTime = gpGlobals->time;
		m_flPrevPrimaryAttack = delay;
	}
	// calculate the time between this shot and the previous
	float flTimeBetweenFires = gpGlobals->time - m_flLastFireTime;
	float flCreep = 0.0f;
	if (flTimeBetweenFires > 0)
		flCreep = flTimeBetweenFires - m_flPrevPrimaryAttack; // postive or negative

// save the last fire time
	m_flLastFireTime = gpGlobals->time;

	float flNextAttack = UTIL_WeaponTimeBase() + delay - flCreep;
	// we need to remember what the m_flNextPrimaryAttack time is set to for each shot, 
	// store it as m_flPrevPrimaryAttack.
	m_flPrevPrimaryAttack = flNextAttack - UTIL_WeaponTimeBase();
	// 	char szMsg[256];
	// 	_snprintf( szMsg, sizeof(szMsg), "next attack time: %0.4f\n", gpGlobals->time + flNextAttack );
	// 	OutputDebugString( szMsg );
	return flNextAttack;
}

const char* CBasePlayerWeapon::GetModelV() {
	return m_customModelV ? STRING(m_customModelV) : m_defaultModelV;
}

const char* CBasePlayerWeapon::GetModelP() {
	return m_customModelP ? STRING(m_customModelP) : m_defaultModelP;
}

const char* CBasePlayerWeapon::GetModelW() {
	if (m_customModelW) {
		return STRING(m_customModelW);
	}

	return mp_mergemodels.value && MergedModelBody() != -1 ? MERGED_ITEMS_MODEL : m_defaultModelW;
}

void CBasePlayerWeapon::SetWeaponModelW() {
	if (m_customModelW || MergedModelBody() == -1) {
		SET_MODEL(ENT(pev), GetModelW());
	}
	else {
		SET_MODEL_MERGED(ENT(pev), GetModelW(), MergedModelBody());
	}
}

void CBasePlayerWeapon::PrintState(void)
{
	ALERT(at_console, "primary:  %f\n", m_flNextPrimaryAttack);
	ALERT(at_console, "idle   :  %f\n", m_flTimeWeaponIdle);

	//	ALERT( at_console, "nextrl :  %f\n", m_flNextReload );
	//	ALERT( at_console, "nextpum:  %f\n", m_flPumpTime );

	//	ALERT( at_console, "m_frt  :  %f\n", m_fReloadTime );
	ALERT(at_console, "m_finre:  %i\n", m_fInReload);
	//	ALERT( at_console, "m_finsr:  %i\n", m_fInSpecialReload );

	ALERT(at_console, "m_iclip:  %i\n", m_iClip);
}

edict_t* nearbyCorpses[256];
int numNearbyCorpses = 0;

void CBasePlayerWeapon::SolidifyNearbyCorpses(bool solidState) {
	CBasePlayer* m_pPlayer = GetPlayer();
	if (!m_pPlayer)
		return;

	Vector vecSrc = m_pPlayer->GetGunPosition();

	if (!solidState) {
		// solidfy nearby corpses so they can be gibbed
		// TODO: This is expensive. Try to fix the bug where players have laggy movement on corpses instead.
		// For some reason the shocktrooper on auspices has no laggy movement when player walks on it,
		// though it seems like the bbox is set incorrectly (much higher than 2 units).
		numNearbyCorpses = 0;

		edict_t* ent = NULL;
		while (!FNullEnt(ent = FIND_ENTITY_IN_SPHERE(ent, vecSrc, 192)) && numNearbyCorpses < 256) {
			if ((ent->v.flags & (FL_MONSTER | FL_CLIENT)) && ent->v.deadflag >= DEAD_DEAD && ent->v.solid == SOLID_NOT && !(ent->v.effects & EF_NODRAW)) {
				nearbyCorpses[numNearbyCorpses++] = ent;
				ent->v.solid = SOLID_BBOX;
				ent->v.iuser4 = 1337; // HACKAROO: flag meaning "this entity is a gibbable corpse which is about to be attacked so it should have a large bounding box right now, but usually it shouldn't have that"
				UTIL_SetOrigin(&ent->v, ent->v.origin);
			}
		}
	}
	else {
		//ALERT(at_console, "solidfied %d nearby corpses\n", numNearbyCorpses);
		// revert back to nonsolid so that the player doesn't get gibbed by a door or have laggy movement
		for (int i = 0; i < numNearbyCorpses; i++) {
			edict_t* ent = nearbyCorpses[i];
			ent->v.solid = SOLID_NOT;
			ent->v.iuser4 = 0;
			UTIL_SetOrigin(&ent->v, ent->v.origin); // reset abs bbox
		}
	}
}