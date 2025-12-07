#pragma once
#include "extdll.h"
#include "CActAnimating.h"
#include "CSprite.h"

#define SF_XEN_PLANT_LIGHT_IGNORE_PLAYER 64

#define XEN_PLANT_GLOW_SPRITE		"sprites/flare3.spr"
#define XEN_PLANT_HIDE_TIME			5

class EXPORT CXenPLight : public CActAnimating
{
public:
	void		Spawn(void);
	void		Precache(void);
	void		Touch(CBaseEntity* pOther);
	void		AnimateThink(void);
	void		DropThink(void);

	void		LightOn(void);
	void		LightOff(void);

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

private:
	CSprite* m_pGlow;
};
