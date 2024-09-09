#pragma once
#include "CBasePlayerWeapon.h"

class CCrowbar : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
	void PrecacheEvents();
	int iItemSlot( void ) { return 1; }
	void EXPORT SwingAgain( void );
	void EXPORT Smack( void );
	int GetItemInfo(ItemInfo *p);

	void PrimaryAttack( void );
	int Swing( int fFirst );
	BOOL Deploy( void );
	void Holster( int skiplocal = 0 );
	int m_iSwing;
	TraceResult m_trHit;

	virtual int MergedModelBody() { return MERGE_MDL_W_CROWBAR; }

	virtual BOOL UseDecrement( void )
	{ 
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
private:
	unsigned short m_usCrowbar;
};
