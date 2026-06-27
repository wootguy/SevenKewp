#pragma once

class CBaseEntity;
typedef struct edict_s edict_t;

#ifndef CLIENT_DLL
#define STUB_INT
#endif

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
	edict_t* GetEdict(void) STUB_INT;
	CBaseEntity* GetEntity(void) STUB_INT;
	edict_t* Set(edict_t* pent) STUB_INT;

	operator CBaseEntity* () STUB_INT;

	CBaseEntity* operator = (CBaseEntity* pEntity) STUB_INT;
	CBaseEntity* operator ->() STUB_INT;
};