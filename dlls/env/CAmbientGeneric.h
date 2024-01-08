#pragma once

enum playmodes {
	PLAYMODE_DEFAULT,
	PLAYMODE_ONCE,
	PLAYMODE_LOOP,

	PLAYMODE_UNUSED,
	PLAYMODE_UNUSED2,

	PLAYMODE_ONCE_LINEAR,
	PLAYMODE_LOOP_LINEAR
};

// runtime pitch shift and volume fadein/out structure
// NOTE: IF YOU CHANGE THIS STRUCT YOU MUST CHANGE THE SAVE/RESTORE VERSION NUMBER
// SEE BELOW (in the typedescription for the class)
typedef struct dynpitchvol
{
	// NOTE: do not change the order of these parameters 
	// NOTE: unless you also change order of rgdpvpreset array elements!
	int preset;

	int pitchrun;		// pitch shift % when sound is running 0 - 255
	int pitchstart;		// pitch shift % when sound stops or starts 0 - 255
	int spinup;			// spinup time 0 - 100
	int spindown;		// spindown time 0 - 100

	int volrun;			// volume change % when sound is running 0 - 10
	int volstart;		// volume change % when sound stops or starts 0 - 10
	int fadein;			// volume fade in time 0 - 100
	int fadeout;		// volume fade out time 0 - 100

						// Low Frequency Oscillator
	int	lfotype;		// 0) off 1) square 2) triangle 3) random
	int lforate;		// 0 - 1000, how fast lfo osciallates

	int lfomodpitch;	// 0-100 mod of current pitch. 0 is off.
	int lfomodvol;		// 0-100 mod of current volume. 0 is off.

	int cspinup;		// each trigger hit increments counter and spinup pitch


	int	cspincount;

	int pitch;
	int spinupsav;
	int spindownsav;
	int pitchfrac;

	int vol;
	int fadeinsav;
	int fadeoutsav;
	int volfrac;

	int	lfofrac;
	int	lfomult;


} dynpitchvol_t;

#define CDPVPRESETMAX 27

// presets for runtime pitch and vol modulation of ambient sounds

class CAmbientGeneric : public CBaseEntity
{
public:
	void KeyValue(KeyValueData* pkvd);
	void Spawn(void);
	void Precache(void);
	void EXPORT ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	void EXPORT RampThink(void);
	void InitModulationParms(void);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];
	virtual int	ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION); }

	float m_flAttenuation;		// attenuation value
	dynpitchvol_t m_dpv;

	BOOL	m_fActive;	// only TRUE when the entity is playing a looping sound
	BOOL	m_fLooping;	// TRUE when the sound played will loop

	// use the mp3 player for music, if an mp3 file is used with the "play everywhere" flag
	bool m_isGlobalMp3;
	bool m_isLoopingMp3;

	int m_playmode;
};