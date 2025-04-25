#pragma once
#include "CBasePlayerWeapon.h"

class CKnife : public CBasePlayerWeapon
{
public:
	void Spawn( void );
	void Precache( void );
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

	//virtual int MergedModelBody() { return MERGE_MDL_W_CROWBAR; }

	BOOL IsClientWeapon() { return FALSE; }

	BOOL UseDecrement(void)
	{
#if defined( CLIENT_WEAPONS )
		return TRUE;
#else
		return FALSE;
#endif
	}
};