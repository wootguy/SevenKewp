#pragma once

#define plat_noiseMoving noise
#define plat_noiseArrived noise1

class CFuncPlat : public CBasePlatTrain
{
public:
	void Spawn(void);
	void Precache(void);
	void Setup(void);

	virtual void Blocked(CBaseEntity* pOther);


	void EXPORT PlatUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void	EXPORT CallGoDown(void) { GoDown(); }
	void	EXPORT CallHitTop(void) { HitTop(); }
	void	EXPORT CallHitBottom(void) { HitBottom(); }

	virtual void GoUp(void);
	virtual void GoDown(void);
	virtual void HitTop(void);
	virtual void HitBottom(void);
};