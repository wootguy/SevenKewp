#include "EHandle.h"

EHANDLE::EHANDLE(edict_t* pent) {
	Set(pent);
}

edict_t* EHANDLE::GetEdict(void)
{
	if (m_pent) {
		byte* edicts = (byte*)ENT(0);
		byte* endEdicts = edicts + sizeof(edict_t) * gpGlobals->maxEntities;
		bool inRange = (byte*)m_pent >= edicts && (byte*)m_pent < endEdicts;
		bool aligned = (((byte*)m_pent - edicts) % sizeof(edict_t)) == 0;

		if (!inRange || !aligned) {
			ALERT(at_error, "An EHANDLE was corrupted! Nulled to avoid crash.\n");
			m_pent = NULL;
			return NULL;
		}

		if (!m_pent->free && m_pent->serialnumber == m_serialnumber) {
			return m_pent;
		}
	}

	return NULL;
}

CBaseEntity* EHANDLE::GetEntity(void)
{
	return (CBaseEntity*)GET_PRIVATE(GetEdict());
}

edict_t* EHANDLE::Set(edict_t* pent)
{
	m_pent = pent;
	if (pent)
		m_serialnumber = m_pent->serialnumber;
	return pent;
}


EHANDLE :: operator CBaseEntity* ()
{
	return (CBaseEntity*)GET_PRIVATE(GetEdict());
}

CBaseEntity* EHANDLE :: operator = (CBaseEntity* pEntity)
{
	if (pEntity)
	{
		m_pent = ENT(pEntity->pev);
		if (m_pent)
			m_serialnumber = m_pent->serialnumber;
	}
	else
	{
		m_pent = NULL;
		m_serialnumber = 0;
	}
	return pEntity;
}

CBaseEntity* EHANDLE :: operator -> ()
{
	return (CBaseEntity*)GET_PRIVATE(GetEdict());
}
