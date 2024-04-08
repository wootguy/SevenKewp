#pragma once

// Spawnflags of CPathCorner
#define SF_CORNER_WAITFORTRIG	0x001
#define SF_CORNER_TELEPORT		0x002
#define SF_CORNER_FIREONCE		0x004

class CPathCorner : public CPointEntity
{
public:
	void Spawn();
	void KeyValue(KeyValueData* pkvd);
	float GetDelay(void) { return m_flWait; }
	//	void Touch( CBaseEntity *pOther );
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

private:
	float	m_flWait;
};
