#pragma once
#include "CBaseAnimating.h"

struct CustomSentence;

//
// generic Toggle entity.
//
#define	SF_ITEM_USE_ONLY	256 //  ITEM_USE_ONLY = BUTTON_USE_ONLY = DOOR_USE_ONLY!!! 

class EXPORT CBaseToggle : public CBaseAnimating
{
public:
	void				KeyValue(KeyValueData* pkvd);

	TOGGLE_STATE		m_toggle_state;
	float				m_flActivateFinished;//like attack_finished, but for doors
	float				m_flMoveDistance;// how far a door should slide or rotate
	float				m_flWait;
	float				m_flLip;
	float				m_flTWidth;// for plats
	float				m_flTLength;// for plats

	Vector				m_vecPosition1;
	Vector				m_vecPosition2;
	Vector				m_vecAngle1;
	Vector				m_vecAngle2;

	int					m_cTriggersLeft;		// trigger_counter only, # of activations remaining
	float				m_flHeight;
	EHANDLE				m_hActivator;
	void (CBaseToggle::* m_pfnCallWhenMoveDone)(void);
	Vector				m_vecFinalDest;
	Vector				m_vecFinalAngle;

	int					m_bitsDamageInflict;	// DMG_ damage type that the door or tigger does

	CustomSentence*		m_customSent;
	float				m_customSentStartTime;	// when the sentence began playing
	float				m_customSentVol;		// overall volume for the custom sentence
	float				m_customSentAttn;		// attenuation for the custom sentence
	int					m_customSentLastWord;	// last sentence word which was played

	// TODO: was 12 keyvalues really necessary for this? I think it can be done with 4.
	// what about the multi_manager style with "#1", or using a multi_manager for things that
	// always trigger at the same time as the door. Much ripenting needed.
	string_t m_fireOnOpenStart;
	string_t m_fireOnOpenEnd;
	string_t m_fireOnCloseStart;
	string_t m_fireOnCloseEnd;
	string_t m_fireOnStart;
	string_t m_fireOnStop;
	USE_TYPE m_fireOnOpenStartMode;
	USE_TYPE m_fireOnOpenEndMode;
	USE_TYPE m_fireOnCloseStartMode;
	USE_TYPE m_fireOnCloseEndMode;
	USE_TYPE m_fireOnStartMode;
	USE_TYPE m_fireOnStopMode;

	string_t m_sMaster;		// If this button has a master switch, this is the targetname.
	// A master switch must be of the multisource type. If all 
	// of the switches in the multisource have been triggered, then
	// the button will be allowed to operate. Otherwise, it will be
	// deactivated.

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);

	static	TYPEDESCRIPTION m_SaveData[];

	virtual int		GetToggleState(void) { return m_toggle_state; }
	virtual float	GetDelay(void) { return m_flWait; }

	// common member functions
	void LinearMove(Vector	vecDest, float flSpeed);
	void LinearMoveDone(void);
	void AngularMove(Vector vecDestAngle, float flSpeed);
	void AngularMoveDone(void);
	BOOL IsLockedByMaster(void);

	virtual CBaseToggle* MyTogglePointer(void) { return this; }

	// monsters use this, but so could buttons for instance
	virtual void PlaySentence(const char* pszSentence, float duration, float volume, float attenuation);
	virtual void PlayScriptedSentence(const char* pszSentence, float duration, float volume, float attenuation, BOOL bConcurrent, CBaseEntity* pListener);
	virtual void SentenceStop(void);
	virtual BOOL IsAllowedToSpeak() { return FALSE; }

	// call when toggle state changes to handle open/close triggers
	void FireStateTriggers();
	virtual void InitStateTriggers();

	static float		AxisValue(int flags, const Vector& angles);
	static void			AxisDir(entvars_t* pev);
	static float		AxisDelta(int flags, const Vector& angle1, const Vector& angle2);
};
#define SetMoveDone( a ) m_pfnCallWhenMoveDone = static_cast <void (CBaseToggle::*)(void)> (a)