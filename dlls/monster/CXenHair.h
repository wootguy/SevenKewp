#pragma once
#include "extdll.h"
#include "CActAnimating.h"

#define SF_HAIR_SYNC		0x0001

class EXPORT CXenHair : public CActAnimating
{
public:
	void		Spawn(void);
	void		Precache(void);
	void		AnimateThink(void);
	void		DropThink(void);
};
