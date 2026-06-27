#pragma once
#include "../dlls/CBaseEntity.h"
#include "../dlls/player/CBasePlayer.h"
#include "weapons.h"

// Items that the player has in their inventory that they can use
class EXPORT CBasePlayerItem : public CBaseAnimating
{
public:
	virtual void SetObjectCollisionBox( void ) STUB_VOID;
	void KeyValue(KeyValueData* pkvd) STUB_VOID;

	virtual int		Save( CSave &save ) STUB_INT;
	virtual int		Restore( CRestore &restore ) STUB_INT;

	virtual int	ObjectCaps(void) STUB_INT;
	
	static	TYPEDESCRIPTION m_SaveData[];

	virtual int AddToPlayer( CBasePlayer *pPlayer ) STUB_INT;	// return TRUE if the item you want the item added to the player inventory
	virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }	// return TRUE if you want your duplicate removed from world
	void DestroyItem( void ) STUB_VOID;
	void DefaultTouch( CBaseEntity *pOther );	// default weapon touch
	void DefaultUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void FallThink ( void );// when an item is first spawned, this think is run to determine when the object has hit the ground.
	virtual void Materialize( void ) STUB_VOID;// make a weapon visible and tangible
	void AttemptToMaterialize( void );  // the weapon desires to become visible and tangible, if the game rules allow for it
	virtual CBaseEntity* Respawn ( void ) STUB_INT;// copy a weapon
	void FallInit( void ) STUB_VOID;
	void CheckRespawn( void );
	virtual int GetItemInfo(ItemInfo *p) { return 0; };	// returns 0 if struct not filled out
	virtual BOOL CanDeploy( void ) { return TRUE; };
	virtual BOOL Deploy( ) { return TRUE; };			// returns is deploy was successful
	virtual BOOL IsItem() { return TRUE; }

	virtual BOOL CanHolster( void ) { return TRUE; };// can this weapon be put away right now?
	virtual void Holster( int skiplocal = 0 ) STUB_VOID;
	virtual void UpdateItemInfo( void ) { return; };

	virtual void ItemPreFrame( void )	{ return; }		// called each frame by the player PreThink
	virtual void ItemPostFrame( void ) { return; }		// called each frame by the player PostThink

	virtual void Drop( void ) STUB_VOID;
	virtual void Kill( void ) STUB_VOID;
	virtual void AttachToPlayer ( CBasePlayer *pPlayer ) STUB_VOID;

	virtual int PrimaryAmmoIndex() { return -1; };
	virtual int SecondaryAmmoIndex() { return -1; };

	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

	virtual const char* GetDeathNoticeWeapon() { return pszName(); }

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[ MAX_AMMO_SLOTS ];

	static void ResetPickupLimits(CBasePlayer* plr);

#ifdef CLIENT_DLL
	CBasePlayer* m_hPlayer;

	inline CBasePlayer* GetPlayer() {
		return m_hPlayer;
	}

#else
	EHANDLE	m_hPlayer;

	inline CBasePlayer* GetPlayer() {
		return (CBasePlayer*)m_hPlayer.GetEntity();
	}
#endif
	

	EHANDLE m_pNext;
	int		m_iId;												// WEAPON_???
	float m_flCustomRespawnTime;

	// fix for ammo duping in dropped weapons, and one-pickup mode test
	bool m_isDroppedWeapon;

	// bitfield flagging which players picked up this item in one-pickup-per-player mode
	uint32_t m_pickupPlayers;

	virtual int iItemSlot( void ) { return ItemInfoArray[m_iId].iSlot; }			// return 0 to MAX_ITEMS_SLOTS, used in hud

	int			iItemPosition( void ) { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1( void )	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	//int			iMaxAmmo1( void )	{ return UTIL_GetMaxAmmo(ItemInfoArray[ m_iId ].pszAmmo1); }
	const char	*pszAmmo2( void )	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	//int			iMaxAmmo2( void )	{ return UTIL_GetMaxAmmo(ItemInfoArray[ m_iId ].pszAmmo2); }
	const char	*pszName( void )	{ return ItemInfoArray[ m_iId ].pszName; }
	int			iMaxClip( void )	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int			iWeight( void )		{ return ItemInfoArray[ m_iId ].iWeight; }
	int			iFlags( void )		{ return ItemInfoArray[ m_iId ].iFlags; }

	// int		m_iIdPrimary;										// Unique Id for primary ammo
	// int		m_iIdSecondary;										// Unique Id for secondary ammo
};
