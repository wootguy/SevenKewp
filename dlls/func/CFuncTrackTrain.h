#include "path/CPathTrack.h"

// Tracktrain spawn flags
#define SF_TRACKTRAIN_NOPITCH		0x0001
#define SF_TRACKTRAIN_NOCONTROL		0x0002
#define SF_TRACKTRAIN_FORWARDONLY	0x0004
#define SF_TRACKTRAIN_PASSABLE		0x0008

class CFuncTrackTrain : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);

	void Blocked(CBaseEntity* pOther);
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void KeyValue(KeyValueData* pkvd);

	void EXPORT Next(void);
	void EXPORT Find(void);
	void EXPORT NearestPath(void);
	void EXPORT DeadEnd(void);

	void		NextThink(float thinkTime, BOOL alwaysThink);

	void SetTrack(CPathTrack* track) { m_ppath = track->Nearest(pev->origin); }
	void SetControls(entvars_t* pevControls);
	BOOL OnControls(entvars_t* pev);

	void StopSound(void);
	void UpdateSound(void);

	static CFuncTrackTrain* Instance(edict_t* pent);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];
	virtual int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DIRECTIONAL_USE; }

	virtual void	OverrideReset(void);

	CPathTrack* m_ppath;
	float		m_length;
	float		m_height;
	float		m_speed;
	float		m_dir;
	float		m_startSpeed;
	Vector		m_controlMins;
	Vector		m_controlMaxs;
	int			m_soundPlaying;
	int			m_sounds;
	float		m_flVolume;
	float		m_flBank;
	float		m_oldSpeed;

private:
	unsigned short m_usAdjustPitch;
};