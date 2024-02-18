#ifndef CLASER_H
#define CLASER_H

#include "CBeam.h"

class CLaser : public CBeam
{
public:
	void	Spawn(void);
	void	Precache(void);
	void	KeyValue(KeyValueData* pkvd);

	void	TurnOn(void);
	void	TurnOff(void);
	int		IsOn(void);

	void	FireAtPoint(TraceResult& point);

	void	EXPORT StrikeThink(void);
	void	Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	EHANDLE m_hSprite;
	int		m_iszSpriteName;
	Vector  m_firePosition;
};

#endif
