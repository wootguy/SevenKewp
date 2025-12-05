#pragma once
#include "CBaseDoor.h"

#define SF_BUTTON_DONTMOVE		1
#define SF_ROTBUTTON_NOTSOLID	1
#define	SF_BUTTON_TOGGLE		32	// button stays pushed until reactivated
#define	SF_BUTTON_SPARK_IF_OFF	64	// button sparks in OFF state
#define SF_BUTTON_TOUCH_ONLY	256	// button only fires as a result of USE key.

const char* ButtonSound(int sound);				// get string of button sound number

void DoSpark(entvars_t* pev, const Vector& location);

extern const char* g_sparkSounds[6];

//
// Generic Button
//
class EXPORT CBaseButton : public CBaseToggle
{
public:
	void Spawn(void);
	virtual void Precache(void);
	void RotSpawn(void);
	virtual void KeyValue(KeyValueData* pkvd);
	virtual const char* DisplayName() { return "Button"; }

	void ButtonActivate();
	void SparkSoundCache(void);

	void ButtonShot(void);
	void ButtonTouch(CBaseEntity* pOther);
	void ButtonSpark(void);
	void TriggerAndWait(void);
	void ButtonReturn(void);
	void ButtonBackHome(void);
	void ButtonUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	virtual int		TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType);
	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	virtual BOOL IsAllowedToSpeak() { return TRUE; }
	virtual BOOL IsButton() { return TRUE; }
	virtual const char* DisplayHint();

	enum BUTTON_CODE { BUTTON_NOTHING, BUTTON_ACTIVATE, BUTTON_RETURN };
	BUTTON_CODE	ButtonResponseToTouch(void);

	static	TYPEDESCRIPTION m_SaveData[];
	// Buttons that don't take damage can be IMPULSE used
	virtual int	ObjectCaps(void) { return (CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | (pev->takedamage ? 0 : FCAP_IMPULSE_USE); }

	BOOL	m_fStayPushed;	// button stays pushed in until touched again?
	BOOL	m_fRotating;		// a rotating button?  default is a sliding button.

	string_t m_strChangeTarget;	// if this field is not null, this is an index into the engine string array.
							// when this button is touched, it's target entity's TARGET field will be set
							// to the button's ChangeTarget. This allows you to make a func_train switch paths, etc.

	locksound_t m_ls;			// door lock sounds

	BYTE	m_bLockedSound;		// ordinals from entity selection
	BYTE	m_bLockedSentence;
	BYTE	m_bUnlockedSound;
	BYTE	m_bUnlockedSentence;
	int		m_sounds;
};
