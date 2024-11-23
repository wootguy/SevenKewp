#pragma once
#include "CBaseDelay.h"
#include "monsterevent.h"

class EXPORT CBaseAnimating : public CBaseDelay
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; }
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	virtual CBaseAnimating* MyAnimatingPointer(void) { return this; }

	static	TYPEDESCRIPTION m_SaveData[];

	// Basic Monster Animation functions
	float StudioFrameAdvance(float flInterval = 0.0); // accumulate animation frame time from last time called until now
	int	 GetSequenceFlags(void);
	virtual int  LookupActivity(int activity);
	int  LookupActivityHeaviest(int activity);
	int  LookupSequence(const char* label);
	void ResetSequenceInfo();
	void DispatchAnimEvents(float flFutureInterval = 0.1); // Handle events that have happend since last time called up until X seconds into the future
	virtual void HandleAnimEvent(MonsterEvent_t* pEvent) { return; };
	float SetBoneController(int iController, float flValue);
	void InitBoneControllers(void);
	float SetBlending(int iBlender, float flValue);
	void GetBonePosition(int iBone, Vector& origin, Vector& angles);
	void GetAutomovement(Vector& origin, Vector& angles, float flInterval = 0.1);
	int  FindTransition(int iEndingSequence, int iGoalSequence, int* piDir);
	void GetAttachment(int iAttachment, Vector& origin, Vector& angles);
	int GetAttachmentCount();
	void SetBodygroup(int iGroup, int iValue);
	int GetBodygroup(int iGroup);
	int ExtractBbox(int sequence, float* mins, float* maxs);
	void SetSequenceBox(void);
	bool ActivityHasEvent(int activity, int event);

	// animation needs
	float				m_flFrameRate;		// computed FPS for current sequence
	float				m_flGroundSpeed;	// computed linear movement rate for current sequence
	float				m_flLastEventCheck;	// last time the event list was checked
	BOOL				m_fSequenceFinished;// flag set when StudioAdvanceFrame moves across a frame boundry
	BOOL				m_fSequenceLoops;	// true if the sequence loops
};