#pragma once
#include "CBaseAnimating.h"

#define SF_XEN_PLANT_DROP_TO_FLOOR 2

class EXPORT CActAnimating : public CBaseAnimating
{
public:
	void			SetActivity(Activity act);
	inline Activity	GetActivity(void) { return m_Activity; }

	virtual int	ObjectCaps(void) { return CBaseAnimating::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	virtual int	Save(CSave& save);
	virtual int	Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

protected:
	void DropToFloor();
private:
	Activity	m_Activity;
};
