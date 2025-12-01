#pragma once
#include "CBaseEntity.h"
#include "CBasePlayer.h"
#include "weapons.h"

// Items that the player has in their inventory that they can use
class EXPORT CBasePlayerItem : public CBaseAnimating
{
public:
	virtual void SetObjectCollisionBox( void );
	void KeyValue(KeyValueData* pkvd);

	virtual int		Save( CSave &save );
	virtual int		Restore( CRestore &restore );

	virtual int	ObjectCaps(void);
	
	static	TYPEDESCRIPTION m_SaveData[];

	virtual int AddToPlayer( CBasePlayer *pPlayer );	// return TRUE if the item you want the item added to the player inventory
	virtual int AddDuplicate( CBasePlayerItem *pItem ) { return FALSE; }	// return TRUE if you want your duplicate removed from world
	void DestroyItem( void );
	void DefaultTouch( CBaseEntity *pOther );	// default weapon touch
	void DefaultUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void FallThink ( void );// when an item is first spawned, this think is run to determine when the object has hit the ground.
	virtual void Materialize( void );// make a weapon visible and tangible
	void AttemptToMaterialize( void );  // the weapon desires to become visible and tangible, if the game rules allow for it
	virtual CBaseEntity* Respawn ( void );// copy a weapon
	void FallInit( void );
	void CheckRespawn( void );
	virtual int GetItemInfo(ItemInfo *p) { return 0; };	// returns 0 if struct not filled out
	virtual BOOL CanDeploy( void ) { return TRUE; };
	virtual BOOL Deploy( )								// returns is deploy was successful
		 { return TRUE; };
	virtual BOOL	IsItem() { return TRUE; }

	virtual BOOL CanHolster( void ) { return TRUE; };// can this weapon be put away right now?
	virtual void Holster( int skiplocal = 0 );
	virtual void UpdateItemInfo( void ) { return; };

	virtual void ItemPreFrame( void )	{ return; }		// called each frame by the player PreThink
	virtual void ItemPostFrame( void ) { return; }		// called each frame by the player PostThink

	virtual void Drop( void );
	virtual void Kill( void );
	virtual void AttachToPlayer ( CBasePlayer *pPlayer );

	virtual int PrimaryAmmoIndex() { return -1; };
	virtual int SecondaryAmmoIndex() { return -1; };

	virtual int UpdateClientData( CBasePlayer *pPlayer ) { return 0; }

	virtual const char* GetDeathNoticeWeapon() { return pszName(); }

	static ItemInfo ItemInfoArray[ MAX_WEAPONS ];
	static AmmoInfo AmmoInfoArray[ MAX_AMMO_SLOTS ];

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

	// lazy fix for ammo duping in dropped weapons
	bool m_isDroppedWeapon;

	virtual int iItemSlot( void ) { return 0; }			// return 0 to MAX_ITEMS_SLOTS, used in hud

	int			iItemPosition( void ) { return ItemInfoArray[ m_iId ].iPosition; }
	const char	*pszAmmo1( void )	{ return ItemInfoArray[ m_iId ].pszAmmo1; }
	int			iMaxAmmo1( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo1; }
	const char	*pszAmmo2( void )	{ return ItemInfoArray[ m_iId ].pszAmmo2; }
	int			iMaxAmmo2( void )	{ return ItemInfoArray[ m_iId ].iMaxAmmo2; }
	const char	*pszName( void )	{ return ItemInfoArray[ m_iId ].pszName; }
	int			iMaxClip( void )	{ return ItemInfoArray[ m_iId ].iMaxClip; }
	int			iWeight( void )		{ return ItemInfoArray[ m_iId ].iWeight; }
	int			iFlags( void )		{ return ItemInfoArray[ m_iId ].iFlags; }

	// int		m_iIdPrimary;										// Unique Id for primary ammo
	// int		m_iIdSecondary;										// Unique Id for secondary ammo
};
