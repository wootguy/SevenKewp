#pragma once
#include "CBaseAnimating.h"

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

	static float		AxisValue(int flags, const Vector& angles);
	static void			AxisDir(entvars_t* pev);
	static float		AxisDelta(int flags, const Vector& angle1, const Vector& angle2);

	string_t m_sMaster;		// If this button has a master switch, this is the targetname.
							// A master switch must be of the multisource type. If all 
							// of the switches in the multisource have been triggered, then
							// the button will be allowed to operate. Otherwise, it will be
							// deactivated.
};
#define SetMoveDone( a ) m_pfnCallWhenMoveDone = static_cast <void (CBaseToggle::*)(void)> (a)