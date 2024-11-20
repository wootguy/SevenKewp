#pragma once

class CBaseEntity;

//
// EHANDLE. Safe way to point to CBaseEntities who may die between frames
//
class EXPORT EHANDLE
{
private:
	edict_t* m_pent;
	int		m_serialnumber;
public:
	EHANDLE() : m_pent(0), m_serialnumber(0) {}
	EHANDLE(edict_t* pent);
	edict_t* GetEdict(void);
	CBaseEntity* GetEntity(void);
	edict_t* Set(edict_t* pent);

	operator CBaseEntity* ();

	CBaseEntity* operator = (CBaseEntity* pEntity);
	CBaseEntity* operator ->();
};