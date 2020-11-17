#include "CFuncTrackTrain.h"

#define SF_TRACK_ACTIVATETRAIN		0x00000001
#define SF_TRACK_RELINK				0x00000002
#define SF_TRACK_ROTMOVE			0x00000004
#define SF_TRACK_STARTBOTTOM		0x00000008
#define SF_TRACK_DONT_MOVE			0x00000010

//
// This entity is a rotating/moving platform that will carry a train to a new track.
// It must be larger in X-Y planar area than the train, since it must contain the
// train within these dimensions in order to operate when the train is near it.
//

typedef enum { TRAIN_SAFE, TRAIN_BLOCKING, TRAIN_FOLLOWING } TRAIN_CODE;

class CFuncTrackChange : public CFuncPlatRot
{
public:
	void Spawn(void);
	void Precache(void);

	//	virtual void	Blocked( void );
	virtual void	EXPORT GoUp(void);
	virtual void	EXPORT GoDown(void);

	void			KeyValue(KeyValueData* pkvd);
	void			Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void			EXPORT Find(void);
	TRAIN_CODE		EvaluateTrain(CPathTrack* pcurrent);
	void			UpdateTrain(Vector& dest);
	virtual void	HitBottom(void);
	virtual void	HitTop(void);
	void			Touch(CBaseEntity* pOther);
	virtual void	UpdateAutoTargets(int toggleState);
	virtual	BOOL	IsTogglePlat(void) { return TRUE; }

	void			DisableUse(void) { m_use = 0; }
	void			EnableUse(void) { m_use = 1; }
	int				UseEnabled(void) { return m_use; }

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	virtual void	OverrideReset(void);


	CPathTrack* m_trackTop;
	CPathTrack* m_trackBottom;

	CFuncTrackTrain* m_train;

	int				m_trackTopName;
	int				m_trackBottomName;
	int				m_trainName;
	TRAIN_CODE		m_code;
	int				m_targetState;
	int				m_use;
};