#pragma once

#define plat_noiseMoving noise
#define plat_noiseArrived noise1

class CFuncPlat : public CBasePlatTrain
{
public:
	void Spawn(void);
	void Precache(void);
	void Setup(void);

	virtual const char* DisplayName() { return "Elevator"; }

	virtual void Blocked(CBaseEntity* pOther);

	// This is done to fix spawn flag collisions between this class and a derived class
	virtual BOOL IsTogglePlat(void) { return (pev->spawnflags & SF_PLAT_TOGGLE) ? TRUE : FALSE; }

	void EXPORT PlatUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void	EXPORT CallGoDown(void) { GoDown(); }
	void	EXPORT CallHitTop(void) { HitTop(); }
	void	EXPORT CallHitBottom(void) { HitBottom(); }

	virtual void GoUp(void);
	virtual void GoDown(void);
	virtual void HitTop(void);
	virtual void HitBottom(void);

	float	m_volume;			// Sound volume
};