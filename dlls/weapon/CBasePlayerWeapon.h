#pragma once
#include "CBasePlayerItem.h"
#include "lagcomp.h"

class EXPORT CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual void KeyValue(KeyValueData* pkvd);
	void Precache(); // custom weapons call this
	virtual void PrecacheEvents() {} // server must always call this for weapons which the client loads by default
	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );
	
	static	TYPEDESCRIPTION m_SaveData[];

	// generic weapon versions of CBasePlayerItem calls
	virtual int AddToPlayer( CBasePlayer *pPlayer );
	virtual int AddDuplicate( CBasePlayerItem *pItem );

	virtual int ExtractAmmo( CBasePlayerWeapon *pWeapon ); //{ return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up
	virtual int ExtractClipAmmo( CBasePlayerWeapon *pWeapon );// { return TRUE; };			// Return TRUE if you can add ammo to yourself when picked up

	virtual int AddWeapon( void ) { ExtractAmmo( this ); return TRUE; };	// Return TRUE if you want to add yourself to the player

	// generic "shared" ammo handlers
	BOOL AddPrimaryAmmo( int iCount, char *szName, int iMaxClip, int iMaxCarry );
	BOOL AddSecondaryAmmo( int iCount, char *szName, int iMaxCarry );

	virtual void UpdateItemInfo( void ) {};	// updates HUD state

	virtual BOOL PlayEmptySound( void );
	virtual void ResetEmptySound( void );

	virtual void SendWeaponAnim( int iAnim, int skiplocal = 1, int body = 0 );  // skiplocal is 1 if client is predicting weapon animations
	virtual void SendWeaponAnimSpec(int iAnim); // send a weapon anim to spectators, and the owner if it isn't a SevenKewp client that can predict this anim

	virtual BOOL CanDeploy( void );
	virtual BOOL IsUseable( void );
	BOOL DefaultDeploy(const char *szViewModel, const char *szWeaponModel, int iAnim, const char *szAnimExt, int skiplocal = 0, int body = 0 );
	int DefaultReload( int iClipSize, int iAnim, float fDelay, int body = 0 );
	
	virtual void GetAmmoDropInfo(bool isSecondary, const char*& ammoEntName, int& dropAmount);
	virtual void ItemPostFrame( void );	// called each frame by the player PostThink
	// called by CBasePlayerWeapons ItemPostFrame()
	virtual void PrimaryAttack( void ) { return; }				// do "+ATTACK"
	virtual void SecondaryAttack( void ) { return; }			// do "+ATTACK2"
	virtual void TertiaryAttack( void ) { return; }
	virtual void Reload( void ) { return; }						// do "+RELOAD"
	virtual void WeaponIdle( void ) { return; }					// called when no buttons pressed
	virtual int UpdateClientData( CBasePlayer *pPlayer );		// sends hud info to client dll, if things have changed
	virtual void RetireWeapon( void );
	virtual BOOL ShouldWeaponIdle( void ) {return FALSE; };
	virtual void Holster( int skiplocal = 0 );
	virtual BOOL UseDecrement( void ) { return FALSE; };
	virtual BOOL IsClientWeapon() { return TRUE; }; // true if the client DLL predicts this weapon
	virtual BOOL IsSevenKewpWeapon() { return FALSE; } // true if HL players can't use this
	virtual BOOL CanAkimbo() { return FALSE; } // true if akimbo can be activated now
	virtual BOOL IsAkimbo() { return FALSE; } // true if akimbo mode is active right now
	virtual BOOL IsAkimboWeapon() { return FALSE; } // true if this weapon can ever enter akimbo mode
	inline int GetAkimboClip() { return IsAkimbo() ? m_chargeReady : -1; }
	inline void SetAkimboClip(int clip) { m_chargeReady = clip; }
	
	int	PrimaryAmmoIndex(); 
	int	SecondaryAmmoIndex(); 

	void PrintState( void );

	virtual CBasePlayerWeapon*GetWeaponPtr( void ) { return this; };
	virtual CBaseEntity* Respawn(void);// copy a weapon
	float GetNextAttackDelay( float delay );

	virtual const char* GetModelV(const char* defaultModel=NULL);
	virtual const char* GetModelP();
	virtual const char* GetModelW();
	virtual int MergedModelBody() { return -1; } // body index to use in the merged items model (-1 = don't use merged model)
	virtual int MergedModelBodyAkimbo() { return -1; } // body index to use in the merged items model (-1 = don't use merged model)
	void SetWeaponModelW(); // accounts for merged models

	// hack to allow corpse gibbing of non-solid corpses
	void SolidifyNearbyCorpses(bool solidState);

	virtual float GetDamageModifier() {
		CBasePlayer* plr = GetPlayer();
		return plr ? plr->GetDamageModifier() : 1.0f;
	}

	int m_iPlayEmptySound;
	int m_fFireOnEmpty;		// True when the gun is empty and the player is still holding down the
	// attack key(s)

	float m_flPumpTime;
	int		m_fInSpecialReload;									// Are we in the middle of a reload for the shotguns
	float	m_flNextPrimaryAttack;								// soonest time ItemPostFrame will call PrimaryAttack
	float	m_flNextSecondaryAttack;							// soonest time ItemPostFrame will call SecondaryAttack
	float	m_flNextTertiaryAttack;								// soonest time ItemPostFrame will call TertiaryAttack
	float	m_flTimeWeaponIdle;									// soonest time ItemPostFrame will call WeaponIdle
	int		m_iPrimaryAmmoType;									// "primary" ammo index into players m_rgAmmo[]
	int		m_iSecondaryAmmoType;								// "secondary" ammo index into players m_rgAmmo[]
	int		m_iClip;											// number of shots left in the primary weapon clip, -1 it not used
	int		m_iClientClip;										// the last version of m_iClip sent to hud dll
	int		m_iClientWeaponState;								// the last version of the weapon state sent to hud dll (is current weapon, is on target)
	int		m_fInReload;										// Are we in the middle of a reload;

	int		m_iDefaultAmmo;// how much ammo you get when you pick up this weapon as placed by a level designer.
	
	// hle time creep vars
	float	m_flPrevPrimaryAttack;
	float	m_flLastFireTime;			

	const char* m_defaultModelV;
	const char* m_defaultModelP;
	const char* m_defaultModelW;
	string_t m_customModelV;
	string_t m_customModelP;
	string_t m_customModelW;

	const char* m_ammoModel;
	const char* m_ammoModel2;

	bool m_hasHandModels; // true if hands are visible on this model and can be swapped
};
