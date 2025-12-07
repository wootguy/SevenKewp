#pragma once
#include	"extdll.h"
#include	"CBaseMonster.h"

#define		ROACH_IDLE				0
#define		ROACH_BORED				1
#define		ROACH_SCARED_BY_ENT		2
#define		ROACH_SCARED_BY_LIGHT	3
#define		ROACH_SMELL_FOOD		4
#define		ROACH_EAT				5

//=========================================================
// Monster's Anim Events Go Here
//=========================================================
class EXPORT CRoach : public CBaseMonster
{
public:
	virtual int	ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_IMPULSE_USE; }
	void Spawn( void );
	void Precache( void );
	void SetYawSpeed( void );
	void MonsterThink ( void );
	void Move ( float flInterval );
	void PickNewDest ( int iCondition );
	void Touch ( CBaseEntity *pOther );
	void Killed( entvars_t *pevAttacker, int iGib );

	float	m_flLastLightLevel;
	float	m_flNextSmellTime;
	int		Classify ( void );
	const char* DisplayName();
	void	Look ( int iDistance );
	int		ISoundMask ( void );
	
	// UNDONE: These don't necessarily need to be save/restored, but if we add more data, it may
	BOOL	m_fLightHacked;
	int		m_iMode;
	// -----------------------------
};