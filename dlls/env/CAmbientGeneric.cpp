#include "extdll.h"
#include "util.h"
#include "gamerules.h"
#include "CAmbientGeneric.h"
#include "CBasePlayer.h"

#include <string>

#define SF_MUSIC_START_SILENT 1
#define SF_MUSIC_LOOP 2
#define SF_MUSIC_ACTIVATOR_ONLY 4

dynpitchvol_t rgdpvpreset[CDPVPRESETMAX] =
{
	// pitch	pstart	spinup	spindwn	volrun	volstrt	fadein	fadeout	lfotype	lforate	modptch modvol	cspnup		
	{1,	255,	 75,	95,		95,		10,		1,		50,		95, 	0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{2,	255,	 85,	70,		88,		10,		1,		20,		88,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{3,	255,	100,	50,		75,		10,		1,		10,		75,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{4,	100,	100,	0,		0,		10,		1,		90,		90,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{5,	100,	100,	0,		0,		10,		1,		80,		80,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{6,	100,	100,	0,		0,		10,		1,		50,		70,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{7,	100,	100,	0,		0,		 5,		1,		40,		50,		1,		50,		0,		10,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{8,	100,	100,	0,		0,		 5,		1,		40,		50,		1,		150,	0,		10,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{9,	100,	100,	0,		0,		 5,		1,		40,		50,		1,		750,	0,		10,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{10,128,	100,	50,		75,		10,		1,		30,		40,		2,		 8,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{11,128,	100,	50,		75,		10,		1,		30,		40,		2,		25,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{12,128,	100,	50,		75,		10,		1,		30,		40,		2,		70,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{13,50,		 50,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{14,70,		 70,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{15,90,		 90,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{16,120,	120,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{17,180,	180,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{18,255,	255,	0,		0,		10,		1,		20,		50,		0,		0,		0,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{19,200,	 75,	90,		90,		10,		1,		50,		90,		2,		100,	20,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{20,255,	 75,	97,		90,		10,		1,		50,		90,		1,		40,		50,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{21,100,	100,	0,		0,		10,		1,		30,		50,		3,		15,		20,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{22,160,	160,	0,		0,		10,		1,		50,		50,		3,		500,	25,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{23,255,	 75,	88,		0,		10,		1,		40,		0,		0,		0,		0,		0,		5,		0,0,0,0,0,0,0,0,0,0,0},
	{24,200,	 20,	95,	    70,		10,		1,		70,		70,		3,		20,		50,		0,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{25,180,	100,	50,		60,		10,		1,		40,		60,		2,		90,		100,	100,	0,		0,0,0,0,0,0,0,0,0,0,0},
	{26,60,		 60,	0,		0,		10,		1,		40,		70,		3,		80,		20,		50,		0,		0,0,0,0,0,0,0,0,0,0,0},
	{27,128,	 90,	10,		10,		10,		1,		20,		40,		1,		5,		10,		20,		0,		0,0,0,0,0,0,0,0,0,0,0}
};

// Last entity that started global MP3 playback.
// This is used to prevent other entities stopping each other from playing.
// If another entity starts playing music, all others are stopped automatically because the client
// can only play one track at a time (no more overlapping music :>)
EHANDLE g_lastMp3PlayerEnt;

LINK_ENTITY_TO_CLASS(ambient_generic, CAmbientGeneric)
LINK_ENTITY_TO_CLASS(ambient_music, CAmbientGeneric)

TYPEDESCRIPTION	CAmbientGeneric::m_SaveData[] =
{
	DEFINE_FIELD(CAmbientGeneric, m_flAttenuation, FIELD_FLOAT),
	DEFINE_FIELD(CAmbientGeneric, m_fActive, FIELD_BOOLEAN),
	DEFINE_FIELD(CAmbientGeneric, m_fLooping, FIELD_BOOLEAN),

	// HACKHACK - This is not really in the spirit of the save/restore design, but save this
	// out as a binary data block.  If the dynpitchvol_t is changed, old saved games will NOT
	// load these correctly, so bump the save/restore version if you change the size of the struct
	// The right way to do this is to split the input parms (read in keyvalue) into members and re-init this
	// struct in Precache(), but it's unlikely that the struct will change, so it's not worth the time right now.
	DEFINE_ARRAY(CAmbientGeneric, m_dpv, FIELD_CHARACTER, sizeof(dynpitchvol_t)),
};

IMPLEMENT_SAVERESTORE(CAmbientGeneric, CBaseEntity)

//
// ambient_generic - general-purpose user-defined static sound
//
void CAmbientGeneric::Spawn(void)
{
	if (FStrEq(STRING(pev->classname), "ambient_music")) {
		m_flAttenuation = ATTN_NONE;
		m_activatorOnly = pev->spawnflags & SF_MUSIC_ACTIVATOR_ONLY;
		m_forceLoop = pev->spawnflags & SF_MUSIC_LOOP;

		if (m_activatorOnly) {
			ALERT(at_error, "Activator only music not implemented!\n");
		}
	}
	else {
		/*
			-1 : "Default"
			0  : "Everywhere"
			200 : "Small Radius"
			125 : "Medium Radius"
			80  : "Large Radius"
		*/
		if (FBitSet(pev->spawnflags, AMBIENT_SOUND_EVERYWHERE))
		{
			m_flAttenuation = ATTN_NONE;
		}
		else if (FBitSet(pev->spawnflags, AMBIENT_SOUND_SMALLRADIUS))
		{
			m_flAttenuation = ATTN_IDLE;
		}
		else if (FBitSet(pev->spawnflags, AMBIENT_SOUND_MEDIUMRADIUS))
		{
			m_flAttenuation = ATTN_STATIC;
		}
		else if (FBitSet(pev->spawnflags, AMBIENT_SOUND_LARGERADIUS))
		{
			m_flAttenuation = ATTN_NORM;
		}
		else
		{// if the designer didn't set a sound attenuation, default to one.
			m_flAttenuation = ATTN_STATIC;
		}

		m_forceLoop = m_playmode == PLAYMODE_LOOP || m_playmode == PLAYMODE_LOOP_LINEAR;
		m_forceOnce = m_playmode == PLAYMODE_ONCE || m_playmode == PLAYMODE_ONCE_LINEAR;
		m_isLinear = m_playmode == PLAYMODE_LOOP_LINEAR || m_playmode == PLAYMODE_ONCE_LINEAR;
	}	

	char* szSoundFile = (char*)STRING(pev->message);

	if (FStringNull(pev->message) || strlen(szSoundFile) < 1)
	{
		ALERT(at_console, "EMPTY AMBIENT AT: %f, %f, %f\n", pev->origin.x, pev->origin.y, pev->origin.z);
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CAmbientGeneric::SUB_Remove);
		return;
	}
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	// Set up think function for dynamic modification 
	// of ambient sound's pitch or volume. Don't
	// start thinking yet.

	SetThink(&CAmbientGeneric::RampThink);
	pev->nextthink = 0;

	// allow on/off switching via 'use' function.

	SetUse(&CAmbientGeneric::ToggleUse);

	m_fActive = FALSE;
	m_fLooping = (!FBitSet(pev->spawnflags, AMBIENT_SOUND_NOT_LOOPING) && !m_forceOnce) || m_forceLoop;
	
	m_isGlobalMp3 = toLowerCase(szSoundFile).find(".mp3") == strlen(szSoundFile) - 4;
	
	if (toLowerCase(szSoundFile).find(".wav") == strlen(szSoundFile) - 4) {
		m_wavInfo = getWaveFileInfo(szSoundFile);
		m_isWav = true;

		if (m_forceLoop && !m_wavInfo.isLooped) {
			// prefer using cue points because then the sound won't reset whenever the player
			// enters the audible range. Also network usage will be reduced from server not needing
			// to send loop/start messages every second.
			ALERT(at_console, "ambient_generic forced loop mode on unlooped WAVE: %s\n", szSoundFile);
		}
		if (m_forceOnce && m_wavInfo.isLooped) {
			// prefer removing cue points to prevent sound looping for a fraction of a second
			// due to client/server lag
			ALERT(at_console, "ambient_generic forced play-once mode on looped WAVE: %s\n", szSoundFile);
		}
	}

	if (m_isGlobalMp3) {
		m_fLooping = m_forceLoop;
	}

	// prevent sound direction looping around to the other side of the world due to short coordinates
	Vector v = pev->origin;
	UTIL_SetOrigin(pev, Vector(clampf(v.x, -4095, 4095), clampf(v.y, -4095, 4095), clampf(v.z, -4095, 4095)));

	Precache();
}

void CAmbientGeneric::Precache(void)
{
	char* szSoundFile = (char*)STRING(pev->message);

	if (!FStringNull(pev->message) && strlen(szSoundFile) > 1)
	{
		if (m_isGlobalMp3)
			PRECACHE_GENERIC((std::string("sound/") + szSoundFile).c_str());
		else if (*szSoundFile != '!')
			PRECACHE_SOUND(szSoundFile);
	}
	// init all dynamic modulation parms
	InitModulationParms();

	bool startSilent = FBitSet(pev->spawnflags, AMBIENT_SOUND_START_SILENT);

	if (FStrEq(STRING(pev->classname), "ambient_music")) {
		startSilent = FBitSet(pev->spawnflags, SF_MUSIC_START_SILENT);
	}

	if (!startSilent)
	{
		// start the sound ASAP
		if (m_fLooping)
			m_fActive = TRUE;
	}
	if (m_fActive)
	{
		if (m_isGlobalMp3) {
			g_lastMp3PlayerEnt = this;
			UTIL_PlayGlobalMp3(szSoundFile, m_fLooping);
		}
		else {
			// SND_SPAWNING sounds are sent to clients when they join
			// but for some reason they don't play. The sound messages
			// need to be delayed
			/*
			UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile,
				(m_dpv.vol * 0.01), m_flAttenuation, SND_SPAWNING, m_dpv.pitch);
			*/
		}

		pev->nextthink = gpGlobals->time + 0.1;
	}
}

int	CAmbientGeneric::GetEntindexPriority() {
	char* szSoundFile = (char*)STRING(pev->message);
	bool isGlobalMp3 = toLowerCase(szSoundFile).find(".mp3") == strlen(szSoundFile) - 4;

	return isGlobalMp3 ? ENTIDX_PRIORITY_LOW : ENTIDX_PRIORITY_NORMAL;
}

// RampThink - Think at 5hz if we are dynamically modifying 
// pitch or volume of the playing sound.  This function will
// ramp pitch and/or volume up or down, modify pitch/volume
// with lfo if active.

void CAmbientGeneric::RampThink(void)
{
	char* szSoundFile = (char*)STRING(pev->message);
	int pitch = m_dpv.pitch;
	int vol = m_dpv.vol;
	int flags = 0;
	int fChanged = 0;		// FALSE if pitch and vol remain unchanged this round
	int	prev;

	if (m_isWav && ((m_wavInfo.isLooped && m_forceOnce) || (!m_wavInfo.isLooped && m_forceLoop))) {
		// forcing a looped sound to play once, or an unlooped sound to loop
		float endTime = m_lastPlayTime + (m_wavInfo.durationMillis / 1000.0f);
		float timeLeft = endTime - g_engfuncs.pfnTime();

		if (m_forceLoop && m_fActive) {
			if (!m_isLinear) {
				// restart the sound when it stops
				UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile,
					(vol * 0.01), m_flAttenuation, SND_CHANGE_PITCH, pitch);
			}
			else {
				// force new volumes to be calculated and replay the sound
				memset(m_lastLinearVolume, 0, sizeof(m_lastLinearVolume));
			}

			if (timeLeft <= 0) {
				m_lastPlayTime = g_engfuncs.pfnTime();
			}
			
			if (m_flAttenuation == 0) {
				pev->nextthink = gpGlobals->time + timeLeft;
			}
			else {
				// periodically send sound messages because the client will not start
				// the sound unless they are close enough to hear it.
				// TODO: only send messages while they're within the audible range
				//       and only repeat those messages for a few seconds to account for ping
				pev->nextthink = gpGlobals->time + V_min(1.0f, timeLeft);
			}	
		}

		if (m_forceOnce && m_lastPlayTime) {
			if (timeLeft <= 0) {
				// stop the sound loop
				UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile, 0, 0, SND_STOP, 0);
				m_lastPlayTime = 0;
			}
			else {
				pev->nextthink = gpGlobals->time + timeLeft;
			}
		}

		// Time() from the previous map carries over to the current map on startup, so this can be negative
		// if the ambient sound starts on, which would then break sound looping/stopping
		pev->nextthink = V_max(pev->nextthink, gpGlobals->time);
	}

	UpdateLinearVolume();

	if (!m_dpv.spinup && !m_dpv.spindown && !m_dpv.fadein && !m_dpv.fadeout && !m_dpv.lfotype) {
		return;						// no ramps or lfo, stop thinking
	}

	// ==============
	// pitch envelope
	// ==============
	if (m_dpv.spinup || m_dpv.spindown)
	{
		prev = m_dpv.pitchfrac >> 8;

		if (m_dpv.spinup > 0)
			m_dpv.pitchfrac += m_dpv.spinup;
		else if (m_dpv.spindown > 0)
			m_dpv.pitchfrac -= m_dpv.spindown;

		pitch = m_dpv.pitchfrac >> 8;

		if (pitch > m_dpv.pitchrun)
		{
			pitch = m_dpv.pitchrun;
			m_dpv.spinup = 0;				// done with ramp up
		}

		if (pitch < m_dpv.pitchstart)
		{
			pitch = m_dpv.pitchstart;
			m_dpv.spindown = 0;				// done with ramp down

			// shut sound off
			if (m_isGlobalMp3) {
				if (g_lastMp3PlayerEnt.GetEntity() == this)
					UTIL_StopGlobalMp3();
			}
			else {
				UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile, 0, 0, SND_STOP, 0);
			}
			

			// return without setting nextthink
			return;
		}

		if (pitch > 255) pitch = 255;
		if (pitch < 1) pitch = 1;

		m_dpv.pitch = pitch;

		fChanged |= (prev != pitch);
		flags |= SND_CHANGE_PITCH;
	}

	// ==================
	// amplitude envelope
	// ==================
	if (m_dpv.fadein || m_dpv.fadeout)
	{
		prev = m_dpv.volfrac >> 8;

		if (m_dpv.fadein > 0)
			m_dpv.volfrac += m_dpv.fadein;
		else if (m_dpv.fadeout > 0)
			m_dpv.volfrac -= m_dpv.fadeout;

		vol = m_dpv.volfrac >> 8;

		if (vol > m_dpv.volrun)
		{
			vol = m_dpv.volrun;
			m_dpv.fadein = 0;				// done with ramp up
		}

		if (vol < m_dpv.volstart)
		{
			vol = m_dpv.volstart;
			m_dpv.fadeout = 0;				// done with ramp down

			// shut sound off
			if (m_isGlobalMp3) {
				if (g_lastMp3PlayerEnt.GetEntity() == this)
					UTIL_StopGlobalMp3();
			}
			else {
				UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile, 0, 0, SND_STOP, 0);
			}
			

			// return without setting nextthink
			return;
		}

		if (vol > 100) vol = 100;
		if (vol < 1) vol = 1;

		m_dpv.vol = vol;

		fChanged |= (prev != vol);
		flags |= SND_CHANGE_VOL;
	}

	// ===================
	// pitch/amplitude LFO
	// ===================
	if (m_dpv.lfotype)
	{
		int pos;

		if (m_dpv.lfofrac > 0x6fffffff)
			m_dpv.lfofrac = 0;

		// update lfo, lfofrac/255 makes a triangle wave 0-255
		m_dpv.lfofrac += m_dpv.lforate;
		pos = m_dpv.lfofrac >> 8;

		if (m_dpv.lfofrac < 0)
		{
			m_dpv.lfofrac = 0;
			m_dpv.lforate = abs(m_dpv.lforate);
			pos = 0;
		}
		else if (pos > 255)
		{
			pos = 255;
			m_dpv.lfofrac = (255 << 8);
			m_dpv.lforate = -abs(m_dpv.lforate);
		}

		switch (m_dpv.lfotype)
		{
		case LFO_SQUARE:
			if (pos < 128)
				m_dpv.lfomult = 255;
			else
				m_dpv.lfomult = 0;

			break;
		case LFO_RANDOM:
			if (pos == 255)
				m_dpv.lfomult = RANDOM_LONG(0, 255);
			break;
		case LFO_TRIANGLE:
		default:
			m_dpv.lfomult = pos;
			break;
		}

		if (m_dpv.lfomodpitch)
		{
			prev = pitch;

			// pitch 0-255
			pitch += ((m_dpv.lfomult - 128) * m_dpv.lfomodpitch) / 100;

			if (pitch > 255) pitch = 255;
			if (pitch < 1) pitch = 1;


			fChanged |= (prev != pitch);
			flags |= SND_CHANGE_PITCH;
		}

		if (m_dpv.lfomodvol)
		{
			// vol 0-100
			prev = vol;

			vol += ((m_dpv.lfomult - 128) * m_dpv.lfomodvol) / 100;

			if (vol > 100) vol = 100;
			if (vol < 0) vol = 0;

			fChanged |= (prev != vol);
			flags |= SND_CHANGE_VOL;
		}

	}

	// Send update to playing sound only if we actually changed
	// pitch or volume in this routine.

	if (flags && fChanged && !m_isGlobalMp3)
	{
		if (pitch == PITCH_NORM)
			pitch = PITCH_NORM + 1; // don't send 'no pitch' !

		UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile,
			(vol * 0.01), m_flAttenuation, flags, pitch);
	}

	// update ramps at 5hz
	pev->nextthink = gpGlobals->time + 0.2;
	return;
}

void CAmbientGeneric::UpdateLinearVolume() {
	if ((m_fLooping && !m_fActive) || !m_isLinear)
		return;

	float endTime = m_lastPlayTime + (m_wavInfo.durationMillis / 1000.0f);
	if (endTime - gpGlobals->time < 0.2f) {
		return; // don't update too close to the end or the sound will restart
	}

	if (!pev->nextthink || pev->nextthink - gpGlobals->time > 0.1f)
		pev->nextthink = gpGlobals->time + 0.1f;

	char* szSoundFile = (char*)STRING(pev->message);

	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);
		int idx = i - 1;

		if (!plr)
			continue;

		float dist = (plr->GetViewPosition() - pev->origin).Length();

		if (dist > m_linearEnd) {
			if (m_lastLinearVolume[idx] != 1) {
				// "stop" the sound, but set volume to the minimum possible so that the client keeps
				// playing it. This way music doesn't restart when briefly leaving the audible range.
				const float minVolume = 1.0f / 255.0f;
				UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile,
					minVolume, 0.0f, SND_CHANGE_VOL, m_dpv.pitch, plr->edict());
			}

			m_lastLinearVolume[idx] = 1;
			continue;
		}

		float t = V_min(1.0f, 1.0f - ((dist - m_linearStart) / (m_linearEnd - m_linearStart)));
		float vol = m_dpv.vol * t * 0.01f;

		int ivol = ((int)(vol * 255) / 5) * 5; // round to multiple of 5 for fewer messages

		if (ivol != m_lastLinearVolume[idx]) {
			UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile, vol, 0.0f,
				SND_CHANGE_VOL, m_dpv.pitch, plr->edict());
			//ALERT(at_console, "Linear vol for %s: %f %f (%d)\n", szSoundFile, vol, t, ivol);
		}
		
		m_lastLinearVolume[idx] = ivol;
	}
}

// Init all ramp params in preparation to 
// play a new sound

void CAmbientGeneric::InitModulationParms(void)
{
	int pitchinc;

	m_dpv.volrun = pev->health * 10;	// 0 - 100
	if (m_dpv.volrun > 100) m_dpv.volrun = 100;
	if (m_dpv.volrun < 0) m_dpv.volrun = 0;

	// get presets
	if (m_dpv.preset != 0 && m_dpv.preset <= CDPVPRESETMAX)
	{
		// load preset values
		m_dpv = rgdpvpreset[m_dpv.preset - 1];

		// fixup preset values, just like
		// fixups in KeyValue routine.
		if (m_dpv.spindown > 0)
			m_dpv.spindown = (101 - m_dpv.spindown) * 64;
		if (m_dpv.spinup > 0)
			m_dpv.spinup = (101 - m_dpv.spinup) * 64;

		m_dpv.volstart *= 10;
		m_dpv.volrun *= 10;

		if (m_dpv.fadein > 0)
			m_dpv.fadein = (101 - m_dpv.fadein) * 64;
		if (m_dpv.fadeout > 0)
			m_dpv.fadeout = (101 - m_dpv.fadeout) * 64;

		m_dpv.lforate *= 256;

		m_dpv.fadeinsav = m_dpv.fadein;
		m_dpv.fadeoutsav = m_dpv.fadeout;
		m_dpv.spinupsav = m_dpv.spinup;
		m_dpv.spindownsav = m_dpv.spindown;
	}

	m_dpv.fadein = m_dpv.fadeinsav;
	m_dpv.fadeout = 0;

	if (m_dpv.fadein)
		m_dpv.vol = m_dpv.volstart;
	else
		m_dpv.vol = m_dpv.volrun;

	m_dpv.spinup = m_dpv.spinupsav;
	m_dpv.spindown = 0;

	if (m_dpv.spinup)
		m_dpv.pitch = m_dpv.pitchstart;
	else
		m_dpv.pitch = m_dpv.pitchrun;

	if (m_dpv.pitch == 0)
		m_dpv.pitch = PITCH_NORM;

	m_dpv.pitchfrac = m_dpv.pitch << 8;
	m_dpv.volfrac = m_dpv.vol << 8;

	m_dpv.lfofrac = 0;
	m_dpv.lforate = abs(m_dpv.lforate);

	m_dpv.cspincount = 1;

	if (m_dpv.cspinup)
	{
		pitchinc = (255 - m_dpv.pitchstart) / m_dpv.cspinup;

		m_dpv.pitchrun = m_dpv.pitchstart + pitchinc;
		if (m_dpv.pitchrun > 255) m_dpv.pitchrun = 255;
	}

	if ((m_dpv.spinupsav || m_dpv.spindownsav || (m_dpv.lfotype && m_dpv.lfomodpitch))
		&& (m_dpv.pitch == PITCH_NORM))
		m_dpv.pitch = PITCH_NORM + 1; // must never send 'no pitch' as first pitch
									  // if we intend to pitch shift later!
}

//
// ToggleUse - turns an ambient sound on or off.  If the 
// ambient is a looping sound, mark sound as active (m_fActive)
// if it's playing, innactive if not.  If the sound is not
// a looping sound, never mark it as active.
//
void CAmbientGeneric::ToggleUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	char* szSoundFile = (char*)STRING(pev->message);
	float fraction;

	if (useType != USE_TOGGLE)
	{
		if ((m_fActive && useType == USE_ON) || (!m_fActive && useType == USE_OFF))
			return;
	}
	// Directly change pitch if arg passed. Only works if sound is already playing.

	if (useType == USE_SET && m_fActive && !m_isGlobalMp3)		// Momentary buttons will pass down a float in here
	{

		fraction = value;

		if (fraction > 1.0)
			fraction = 1.0;
		if (fraction < 0.0)
			fraction = 0.01;

		m_dpv.pitch = fraction * 255;

		UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile,
			0, 0, SND_CHANGE_PITCH, m_dpv.pitch);

		return;
	}

	// Toggle

	// m_fActive is TRUE only if a looping sound is playing.

	if (m_fActive)
	{// turn sound off

		if (m_dpv.cspinup)
		{
			// Don't actually shut off. Each toggle causes
			// incremental spinup to max pitch

			if (m_dpv.cspincount <= m_dpv.cspinup)
			{
				int pitchinc;

				// start a new spinup
				m_dpv.cspincount++;

				pitchinc = (255 - m_dpv.pitchstart) / m_dpv.cspinup;

				m_dpv.spinup = m_dpv.spinupsav;
				m_dpv.spindown = 0;

				m_dpv.pitchrun = m_dpv.pitchstart + pitchinc * m_dpv.cspincount;
				if (m_dpv.pitchrun > 255) m_dpv.pitchrun = 255;

				pev->nextthink = gpGlobals->time + 0.1;
			}

		}
		else
		{
			m_fActive = FALSE;

			// HACKHACK - this makes the code in Precache() work properly after a save/restore
			pev->spawnflags |= AMBIENT_SOUND_START_SILENT;

			if (m_dpv.spindownsav || m_dpv.fadeoutsav)
			{
				// spin it down (or fade it) before shutoff if spindown is set
				m_dpv.spindown = m_dpv.spindownsav;
				m_dpv.spinup = 0;

				m_dpv.fadeout = m_dpv.fadeoutsav;
				m_dpv.fadein = 0;
				pev->nextthink = gpGlobals->time + 0.1;
			}
			else {
				if (m_isGlobalMp3) {
					if (g_lastMp3PlayerEnt.GetEntity() == this)
						UTIL_StopGlobalMp3();
				}
				else {
					UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile, 0, 0, SND_STOP, 0);
				}
			}
				
		}
	}
	else
	{// turn sound on

		// only toggle if this is a looping sound.  If not looping, each
		// trigger will cause the sound to play.  If the sound is still
		// playing from a previous trigger press, it will be shut off
		// and then restarted.

		if (m_fLooping || m_isGlobalMp3)
			m_fActive = TRUE;
		else {
			// shut sound off now - may be interrupting a long non-looping sound
			if (m_isGlobalMp3) {
				if (g_lastMp3PlayerEnt.GetEntity() == this)
					UTIL_StopGlobalMp3();
			}
			else {
				UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile, 0, 0, SND_STOP, 0);
			}
		}
			

		// init all ramp params for startup

		InitModulationParms();

		if (m_isGlobalMp3) {
			UTIL_PlayGlobalMp3(szSoundFile, m_fLooping);
			g_lastMp3PlayerEnt = this;
		}
		else if (m_isLinear) {
			UpdateLinearVolume();
			
			// Need to send the message twice for first playback for some reason
			memset(m_lastLinearVolume, 0, sizeof(m_lastLinearVolume));
		}
		else {
			UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile,
				(m_dpv.vol * 0.01), m_flAttenuation, 0, m_dpv.pitch);
		}

		m_lastPlayTime = g_engfuncs.pfnTime();
		pev->nextthink = gpGlobals->time + 0.1;

	}
}

// KeyValue - load keyvalue pairs into member data of the
// ambient generic. NOTE: called BEFORE spawn!
void CAmbientGeneric::KeyValue(KeyValueData* pkvd)
{
	// NOTE: changing any of the modifiers in this code
	// NOTE: also requires changing InitModulationParms code.

	// preset
	if (FStrEq(pkvd->szKeyName, "preset"))
	{
		m_dpv.preset = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}

	// pitchrun
	else if (FStrEq(pkvd->szKeyName, "pitch"))
	{
		m_dpv.pitchrun = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		if (m_dpv.pitchrun > 255) m_dpv.pitchrun = 255;
		if (m_dpv.pitchrun < 0) m_dpv.pitchrun = 0;
	}

	// pitchstart
	else if (FStrEq(pkvd->szKeyName, "pitchstart"))
	{
		m_dpv.pitchstart = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		if (m_dpv.pitchstart > 255) m_dpv.pitchstart = 255;
		if (m_dpv.pitchstart < 0) m_dpv.pitchstart = 0;
	}

	// spinup
	else if (FStrEq(pkvd->szKeyName, "spinup"))
	{
		m_dpv.spinup = atoi(pkvd->szValue);

		if (m_dpv.spinup > 100) m_dpv.spinup = 100;
		if (m_dpv.spinup < 0) m_dpv.spinup = 0;

		if (m_dpv.spinup > 0)
			m_dpv.spinup = (101 - m_dpv.spinup) * 64;
		m_dpv.spinupsav = m_dpv.spinup;
		pkvd->fHandled = TRUE;
	}

	// spindown
	else if (FStrEq(pkvd->szKeyName, "spindown"))
	{
		m_dpv.spindown = atoi(pkvd->szValue);

		if (m_dpv.spindown > 100) m_dpv.spindown = 100;
		if (m_dpv.spindown < 0) m_dpv.spindown = 0;

		if (m_dpv.spindown > 0)
			m_dpv.spindown = (101 - m_dpv.spindown) * 64;
		m_dpv.spindownsav = m_dpv.spindown;
		pkvd->fHandled = TRUE;
	}

	// volstart
	else if (FStrEq(pkvd->szKeyName, "volstart"))
	{
		m_dpv.volstart = atoi(pkvd->szValue);

		if (m_dpv.volstart > 10) m_dpv.volstart = 10;
		if (m_dpv.volstart < 0) m_dpv.volstart = 0;

		m_dpv.volstart *= 10;	// 0 - 100

		pkvd->fHandled = TRUE;
	}

	// fadein
	else if (FStrEq(pkvd->szKeyName, "fadein"))
	{
		m_dpv.fadein = atoi(pkvd->szValue);

		if (m_dpv.fadein > 100) m_dpv.fadein = 100;
		if (m_dpv.fadein < 0) m_dpv.fadein = 0;

		if (m_dpv.fadein > 0)
			m_dpv.fadein = (101 - m_dpv.fadein) * 64;
		m_dpv.fadeinsav = m_dpv.fadein;
		pkvd->fHandled = TRUE;
	}

	// fadeout
	else if (FStrEq(pkvd->szKeyName, "fadeout"))
	{
		m_dpv.fadeout = atoi(pkvd->szValue);

		if (m_dpv.fadeout > 100) m_dpv.fadeout = 100;
		if (m_dpv.fadeout < 0) m_dpv.fadeout = 0;

		if (m_dpv.fadeout > 0)
			m_dpv.fadeout = (101 - m_dpv.fadeout) * 64;
		m_dpv.fadeoutsav = m_dpv.fadeout;
		pkvd->fHandled = TRUE;
	}

	// lfotype
	else if (FStrEq(pkvd->szKeyName, "lfotype"))
	{
		m_dpv.lfotype = atoi(pkvd->szValue);
		if (m_dpv.lfotype > 4) m_dpv.lfotype = LFO_TRIANGLE;
		pkvd->fHandled = TRUE;
	}

	// lforate
	else if (FStrEq(pkvd->szKeyName, "lforate"))
	{
		m_dpv.lforate = atoi(pkvd->szValue);

		if (m_dpv.lforate > 1000) m_dpv.lforate = 1000;
		if (m_dpv.lforate < 0) m_dpv.lforate = 0;

		m_dpv.lforate *= 256;

		pkvd->fHandled = TRUE;
	}
	// lfomodpitch
	else if (FStrEq(pkvd->szKeyName, "lfomodpitch"))
	{
		m_dpv.lfomodpitch = atoi(pkvd->szValue);
		if (m_dpv.lfomodpitch > 100) m_dpv.lfomodpitch = 100;
		if (m_dpv.lfomodpitch < 0) m_dpv.lfomodpitch = 0;


		pkvd->fHandled = TRUE;
	}

	// lfomodvol
	else if (FStrEq(pkvd->szKeyName, "lfomodvol"))
	{
		m_dpv.lfomodvol = atoi(pkvd->szValue);
		if (m_dpv.lfomodvol > 100) m_dpv.lfomodvol = 100;
		if (m_dpv.lfomodvol < 0) m_dpv.lfomodvol = 0;

		pkvd->fHandled = TRUE;
	}

	// cspinup
	else if (FStrEq(pkvd->szKeyName, "cspinup"))
	{
		m_dpv.cspinup = atoi(pkvd->szValue);
		if (m_dpv.cspinup > 100) m_dpv.cspinup = 100;
		if (m_dpv.cspinup < 0) m_dpv.cspinup = 0;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "playmode"))
	{
		m_playmode = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		pev->health = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "linearmin"))
	{
		m_linearStart = atoi(pkvd->szValue)*256;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "linearmax"))
	{
		m_linearEnd = atoi(pkvd->szValue)*256;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CAmbientGeneric::InitSoundForNewJoiner(edict_t* target) {
	if (m_isLinear)
		return; // these are constantly updated and so don't need manual restarting
	if (!m_isWav)
		return; // mp3 audio is initiliazed elsewhere, don't play here

	float playTime = (g_engfuncs.pfnTime() - m_lastPlayTime) / (m_wavInfo.durationMillis / 1000.0f);
	bool shouldRestartOnceSound = !m_wavInfo.isLooped && m_lastPlayTime && playTime < 0.5f;

	bool isActuallyLooped = m_forceLoop || (m_wavInfo.isLooped && !m_forceOnce);
	bool shouldRestartLoopSound = isActuallyLooped && m_fActive;

	char* szSoundFile = (char*)STRING(pev->message);
	int pitch = m_dpv.pitch;
	int vol = m_dpv.vol;

	if (shouldRestartLoopSound || shouldRestartOnceSound) {
		UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile,
			(vol * 0.01), m_flAttenuation, SND_CHANGE_VOL, pitch, target);
	}
}

void CAmbientGeneric::UpdateOnRemove(void) {
	char* szSoundFile = (char*)STRING(pev->message);

	if (m_fActive) {
		// shut sound off now - may be interrupting a long non-looping sound
		if (m_isGlobalMp3) {
			if (g_lastMp3PlayerEnt.GetEntity() == this)
				UTIL_StopGlobalMp3();
		}
		else {
			UTIL_EmitAmbientSound(ENT(pev), pev->origin, szSoundFile, 0, 0, SND_STOP, 0);
		}
	}

	CBaseEntity::UpdateOnRemove();
}