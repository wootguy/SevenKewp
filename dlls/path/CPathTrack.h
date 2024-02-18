#ifndef CPATHTRACK_H
#define CPATHTRACK_H

// Spawnflag for CPathTrack
#define SF_PATH_DISABLED		0x00000001
#define SF_PATH_FIREONCE		0x00000002
#define SF_PATH_ALTREVERSE		0x00000004
#define SF_PATH_DISABLE_TRAIN	0x00000008
#define SF_PATH_ALTERNATE		0x00008000

//#define PATH_SPARKLE_DEBUG		1	// This makes a particle effect around path_track entities for debugging
class CPathTrack : public CPointEntity
{
public:
	void		Spawn(void);
	void		Activate(void);
	void		KeyValue(KeyValueData* pkvd);

	void		SetPrevious(CPathTrack* pprevious);
	void		Link(void);
	void		Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	CPathTrack* ValidPath(CPathTrack* ppath, int testFlag);		// Returns ppath if enabled, NULL otherwise
	void		Project(CPathTrack* pstart, CPathTrack* pend, Vector* origin, float dist);

	static CPathTrack* Instance(edict_t* pent);

	CPathTrack* LookAhead(Vector* origin, float dist, int move);
	CPathTrack* Nearest(Vector origin);

	CPathTrack* GetNext(void);
	CPathTrack* GetPrevious(void);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];
#if PATH_SPARKLE_DEBUG
	void EXPORT Sparkle(void);
#endif

	float		m_length;
	string_t	m_altName;
	EHANDLE m_hNext;
	EHANDLE m_hPrevious;
	EHANDLE m_hAltpath;
};

#endif