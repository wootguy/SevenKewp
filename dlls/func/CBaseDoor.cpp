#include "extdll.h"
#include "util.h"
#include "CBaseButton.h"
#include "CBaseDoor.h"

TYPEDESCRIPTION	CBaseDoor::m_SaveData[] =
{
	DEFINE_FIELD(CBaseDoor, m_bHealthValue, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bMoveSnd, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bStopSnd, FIELD_CHARACTER),

	DEFINE_FIELD(CBaseDoor, m_bLockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bLockedSentence, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bUnlockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseDoor, m_bUnlockedSentence, FIELD_CHARACTER),

};

IMPLEMENT_SAVERESTORE(CBaseDoor, CBaseToggle)


#define DOOR_SENTENCEWAIT	6
#define DOOR_SOUNDWAIT		3
#define BUTTON_SOUNDWAIT	0.5

// minimum amount of seconds before applying block damage again
// (should be more than 0 to prevent damage every frame when a player gets stuck inside a door)
#define DOOR_SMASH_MIN_DELAY 0.5

// play door or button locked or unlocked sounds. 
// pass in pointer to valid locksound struct. 
// if flocked is true, play 'door is locked' sound,
// otherwise play 'door is unlocked' sound
// NOTE: this routine is shared by doors and buttons

void PlayLockSounds(entvars_t* pev, locksound_t* pls, int flocked, int fbutton)
{
	// LOCKED SOUND

	// CONSIDER: consolidate the locksound_t struct (all entries are duplicates for lock/unlock)
	// CONSIDER: and condense this code.
	float flsoundwait;

	if (fbutton)
		flsoundwait = BUTTON_SOUNDWAIT;
	else
		flsoundwait = DOOR_SOUNDWAIT;

	if (flocked)
	{
		int fplaysound = (pls->sLockedSound && gpGlobals->time > pls->flwaitSound);
		int fplaysentence = (pls->sLockedSentence && !pls->bEOFLocked && gpGlobals->time > pls->flwaitSentence);
		float fvol;

		if (fplaysound && fplaysentence)
			fvol = 0.25;
		else
			fvol = 1.0;

		// if there is a locked sound, and we've debounced, play sound
		if (fplaysound)
		{
			// play 'door locked' sound
			EMIT_SOUND(ENT(pev), CHAN_ITEM, (char*)STRING(pls->sLockedSound), fvol, ATTN_NORM);
			pls->flwaitSound = gpGlobals->time + flsoundwait;
		}

		// if there is a sentence, we've not played all in list, and we've debounced, play sound
		if (fplaysentence)
		{
			// play next 'door locked' sentence in group
			int iprev = pls->iLockedSentence;

			pls->iLockedSentence = SENTENCEG_PlaySequentialSz(ENT(pev), STRING(pls->sLockedSentence),
				0.85, ATTN_NORM, 0, 100, pls->iLockedSentence, FALSE);
			pls->iUnlockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFLocked = (iprev == pls->iLockedSentence);

			pls->flwaitSentence = gpGlobals->time + DOOR_SENTENCEWAIT;
		}
	}
	else
	{
		// UNLOCKED SOUND

		int fplaysound = (pls->sUnlockedSound && gpGlobals->time > pls->flwaitSound);
		int fplaysentence = (pls->sUnlockedSentence && !pls->bEOFUnlocked && gpGlobals->time > pls->flwaitSentence);
		float fvol;

		// if playing both sentence and sound, lower sound volume so we hear sentence
		if (fplaysound && fplaysentence)
			fvol = 0.25;
		else
			fvol = 1.0;

		// play 'door unlocked' sound if set
		if (fplaysound)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, (char*)STRING(pls->sUnlockedSound), fvol, ATTN_NORM);
			pls->flwaitSound = gpGlobals->time + flsoundwait;
		}

		// play next 'door unlocked' sentence in group
		if (fplaysentence)
		{
			int iprev = pls->iUnlockedSentence;

			pls->iUnlockedSentence = SENTENCEG_PlaySequentialSz(ENT(pev), STRING(pls->sUnlockedSentence),
				0.85, ATTN_NORM, 0, 100, pls->iUnlockedSentence, FALSE);
			pls->iLockedSentence = 0;

			// make sure we don't keep calling last sentence in list
			pls->bEOFUnlocked = (iprev == pls->iUnlockedSentence);
			pls->flwaitSentence = gpGlobals->time + DOOR_SENTENCEWAIT;
		}
	}
}

//
// Cache user-entity-field values until spawn is called.
//

void CBaseDoor::KeyValue(KeyValueData* pkvd)
{

	if (FStrEq(pkvd->szKeyName, "skin"))//skin is used for content type
	{
		pev->skin = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "stopsnd"))
	{
		m_bStopSnd = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "healthvalue"))
	{
		m_bHealthValue = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "locked_sound"))
	{
		m_bLockedSound = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "locked_sentence"))
	{
		m_bLockedSentence = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "unlocked_sound"))
	{
		m_bUnlockedSound = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "unlocked_sentence"))
	{
		m_bUnlockedSentence = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "WaveHeight"))
	{
		pev->scale = atof(pkvd->szValue) * (1.0 / 8.0);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_iObeyTriggerMode"))
	{
		m_iObeyTriggerMode = (ObeyTriggerMode)atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "m_fIgnoreTargetname"))
	{
		m_fIgnoreTargetname = atoi(pkvd->szValue) != 0;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

/*QUAKED func_door (0 .5 .8) ? START_OPEN x DOOR_DONT_LINK TOGGLE
if two doors touch, they are assumed to be connected and operate as a unit.

TOGGLE causes the door to wait in both the start and end states for a trigger event.

START_OPEN causes the door to move to its destination when spawned, and operate in reverse.
It is used to temporarily or permanently close off an area when triggered (not usefull for
touch or takedamage doors).

"angle"         determines the opening direction
"targetname"	if set, no touch field will be spawned and a remote button or trigger
				field activates the door.
"health"        if set, door must be shot open
"speed"         movement speed (100 default)
"wait"          wait before returning (3 default, -1 = never return)
"lip"           lip remaining at end of move (8 default)
"dmg"           damage to inflict when blocked (2 default)
"sounds"
0)      no sound
1)      stone
2)      base
3)      stone chain
4)      screechy metal
*/

LINK_ENTITY_TO_CLASS(func_door, CBaseDoor)
//
// func_water - same as a door. 
//
LINK_ENTITY_TO_CLASS(func_water, CBaseDoor)


void CBaseDoor::Spawn()
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	CBaseEntity::Spawn();
	Precache();
	SetMovedir(pev);

	if (pev->skin == 0)
	{//normal door
		if (FBitSet(pev->spawnflags, SF_DOOR_PASSABLE))
			pev->solid = SOLID_NOT;
		else
			pev->solid = SOLID_BSP;
	}
	else
	{// special contents
		pev->solid = SOLID_NOT;
		SetBits(pev->spawnflags, SF_DOOR_SILENT);	// water is silent for now
		g_textureStats.tex_water = true;
	}

	pev->movetype = MOVETYPE_PUSH;
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;

	m_vecPosition1 = pev->origin;
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));
	ASSERTSZ(m_vecPosition1 != m_vecPosition2, "door start/end positions are equal\n");

	if (FBitSet(pev->spawnflags, SF_DOOR_START_OPEN))
	{	// swap pos1 and pos2, put door at pos2
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_vecPosition2 = m_vecPosition1;
		m_vecPosition1 = pev->origin;
	}

	InitStateTriggers();

	m_toggle_state = TS_AT_BOTTOM;

	// if the door is flagged for USE button activation only, use NULL touch function
	if (FBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
	{
		SetTouch(NULL);
	}
	else // touchable button
		SetTouch(&CBaseDoor::DoorTouch);
}

void CBaseDoor::InitStateTriggers() {
	CBaseToggle::InitStateTriggers();

	if (FBitSet(pev->spawnflags, SF_DOOR_START_OPEN)) {
		SWAP(m_fireOnCloseStart, m_fireOnOpenStart, string_t);
		SWAP(m_fireOnCloseEnd, m_fireOnOpenEnd, string_t);
		SWAP(m_fireOnCloseStartMode, m_fireOnOpenStartMode, USE_TYPE);
		SWAP(m_fireOnCloseEndMode, m_fireOnOpenEndMode, USE_TYPE);
	}
}


void CBaseDoor::SetToggleState(int state)
{
	if (state == TS_AT_TOP)
		UTIL_SetOrigin(pev, m_vecPosition2);
	else
		UTIL_SetOrigin(pev, m_vecPosition1);
}


void CBaseDoor::Precache(void)
{
	const char* pszSound;

	// set the door's "in-motion" sound
	switch (m_bMoveSnd)
	{
	case	0:
		pev->door_noiseMoving = ALLOC_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("doors/doormove1.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove1.wav");
		break;
	case	2:
		PRECACHE_SOUND("doors/doormove2.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove2.wav");
		break;
	case	3:
		PRECACHE_SOUND("doors/doormove3.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove3.wav");
		break;
	case	4:
		PRECACHE_SOUND("doors/doormove4.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove4.wav");
		break;
	case	5:
		PRECACHE_SOUND("doors/doormove5.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove5.wav");
		break;
	case	6:
		PRECACHE_SOUND("doors/doormove6.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove6.wav");
		break;
	case	7:
		PRECACHE_SOUND("doors/doormove7.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove7.wav");
		break;
	case	8:
		PRECACHE_SOUND("doors/doormove8.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove8.wav");
		break;
	case	9:
		PRECACHE_SOUND("doors/doormove9.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove9.wav");
		break;
	case	10:
		PRECACHE_SOUND("doors/doormove10.wav");
		pev->door_noiseMoving = ALLOC_STRING("doors/doormove10.wav");
		break;
	default:
		pev->door_noiseMoving = ALLOC_STRING("common/null.wav");
		break;
	}

	// set the door's 'reached destination' stop sound
	switch (m_bStopSnd)
	{
	case	0:
		pev->door_noiseArrived = ALLOC_STRING("common/null.wav");
		break;
	case	1:
		PRECACHE_SOUND("doors/doorstop1.wav");
		pev->door_noiseArrived = ALLOC_STRING("doors/doorstop1.wav");
		break;
	case	2:
		PRECACHE_SOUND("doors/doorstop2.wav");
		pev->door_noiseArrived = ALLOC_STRING("doors/doorstop2.wav");
		break;
	case	3:
		PRECACHE_SOUND("doors/doorstop3.wav");
		pev->door_noiseArrived = ALLOC_STRING("doors/doorstop3.wav");
		break;
	case	4:
		PRECACHE_SOUND("doors/doorstop4.wav");
		pev->door_noiseArrived = ALLOC_STRING("doors/doorstop4.wav");
		break;
	case	5:
		PRECACHE_SOUND("doors/doorstop5.wav");
		pev->door_noiseArrived = ALLOC_STRING("doors/doorstop5.wav");
		break;
	case	6:
		PRECACHE_SOUND("doors/doorstop6.wav");
		pev->door_noiseArrived = ALLOC_STRING("doors/doorstop6.wav");
		break;
	case	7:
		PRECACHE_SOUND("doors/doorstop7.wav");
		pev->door_noiseArrived = ALLOC_STRING("doors/doorstop7.wav");
		break;
	case	8:
		PRECACHE_SOUND("doors/doorstop8.wav");
		pev->door_noiseArrived = ALLOC_STRING("doors/doorstop8.wav");
		break;
	default:
		pev->door_noiseArrived = ALLOC_STRING("common/null.wav");
		break;
	}

	// get door button sounds, for doors which are directly 'touched' to open

	if (m_bLockedSound)
	{
		pszSound = ButtonSound((int)m_bLockedSound);
		PRECACHE_SOUND(pszSound);
		m_ls.sLockedSound = ALLOC_STRING(pszSound);
	}

	if (m_bUnlockedSound)
	{
		pszSound = ButtonSound((int)m_bUnlockedSound);
		PRECACHE_SOUND(pszSound);
		m_ls.sUnlockedSound = ALLOC_STRING(pszSound);
	}

	// get sentence group names, for doors which are directly 'touched' to open

	switch (m_bLockedSentence)
	{
	case 1: m_ls.sLockedSentence = ALLOC_STRING("NA"); break; // access denied
	case 2: m_ls.sLockedSentence = ALLOC_STRING("ND"); break; // security lockout
	case 3: m_ls.sLockedSentence = ALLOC_STRING("NF"); break; // blast door
	case 4: m_ls.sLockedSentence = ALLOC_STRING("NFIRE"); break; // fire door
	case 5: m_ls.sLockedSentence = ALLOC_STRING("NCHEM"); break; // chemical door
	case 6: m_ls.sLockedSentence = ALLOC_STRING("NRAD"); break; // radiation door
	case 7: m_ls.sLockedSentence = ALLOC_STRING("NCON"); break; // gen containment
	case 8: m_ls.sLockedSentence = ALLOC_STRING("NH"); break; // maintenance door
	case 9: m_ls.sLockedSentence = ALLOC_STRING("NG"); break; // broken door

	default: m_ls.sLockedSentence = 0; break;
	}

	switch (m_bUnlockedSentence)
	{
	case 1: m_ls.sUnlockedSentence = ALLOC_STRING("EA"); break; // access granted
	case 2: m_ls.sUnlockedSentence = ALLOC_STRING("ED"); break; // security door
	case 3: m_ls.sUnlockedSentence = ALLOC_STRING("EF"); break; // blast door
	case 4: m_ls.sUnlockedSentence = ALLOC_STRING("EFIRE"); break; // fire door
	case 5: m_ls.sUnlockedSentence = ALLOC_STRING("ECHEM"); break; // chemical door
	case 6: m_ls.sUnlockedSentence = ALLOC_STRING("ERAD"); break; // radiation door
	case 7: m_ls.sUnlockedSentence = ALLOC_STRING("ECON"); break; // gen containment
	case 8: m_ls.sUnlockedSentence = ALLOC_STRING("EH"); break; // maintenance door

	default: m_ls.sUnlockedSentence = 0; break;
	}
}

//
// Doors not tied to anything (e.g. button, another door) can be touched, to make them activate.
//
void CBaseDoor::DoorTouch(CBaseEntity* pOther)
{
	entvars_t* pevToucher = pOther->pev;

	// Ignore touches by anything but players
	if (!FClassnameIs(pevToucher, "player"))
		return;

	// If door has master, and it's not ready to trigger, 
	// play 'locked' sound

	if (m_sMaster && !UTIL_IsMasterTriggered(m_sMaster, pOther))
		PlayLockSounds(pev, &m_ls, TRUE, FALSE);

	// If door is somebody's target, then touching does nothing.
	// You have to activate the owner (e.g. button).

	if (!m_fIgnoreTargetname && !FStringNull(pev->targetname))
	{
		// play locked sound
		PlayLockSounds(pev, &m_ls, TRUE, FALSE);
		return;
	}

	if (!RunInventoryRules(pOther)) {
		return;
	}

	m_hActivator = pOther;// remember who activated the door

	if (DoorActivate(USE_TOGGLE))
		SetTouch(NULL); // Temporarily disable the touch function, until movement is finished.
}


//
// Used by SUB_UseTargets, when a door is the target of a button.
//
void CBaseDoor::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (BreakableUse(pActivator, pCaller, useType, value))
		return;

	m_hActivator = pActivator;

	bool isStopped = m_toggle_state == TS_AT_BOTTOM || m_toggle_state == TS_AT_TOP;
	bool doorOpening = m_toggle_state == TS_GOING_UP || m_toggle_state == TS_AT_TOP;
	bool doorClosing = m_toggle_state == TS_GOING_DOWN || m_toggle_state == TS_AT_BOTTOM;

	if (!RunInventoryRules(pCaller)) {
		return;
	}

	if (m_iObeyTriggerMode == DOOR_OBEY_NO) {
		// if not ready to be used, ignore "use" command.
		if (m_toggle_state == TS_AT_BOTTOM || (FBitSet(pev->spawnflags, SF_DOOR_NO_AUTO_RETURN) && m_toggle_state == TS_AT_TOP))
			DoorActivate(USE_TOGGLE);
	}
	else if (m_iObeyTriggerMode == DOOR_OBEY_YES || m_iObeyTriggerMode == DOOR_OBEY_YES_MOVING) {
		if (doorClosing && useType == USE_OFF) {
			return;
		}
		if (doorOpening && useType == USE_ON) {
			return;
		}

		if (isStopped || m_iObeyTriggerMode == DOOR_OBEY_YES_MOVING)
			DoorActivate(useType);
	}
}

//
// Causes the door to "do its thing", i.e. start moving, and cascade activation.
//
int CBaseDoor::DoorActivate(USE_TYPE useType)
{
	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
		return 0;

	bool doorOpening = m_toggle_state == TS_GOING_UP || m_toggle_state == TS_AT_TOP;
	bool shouldClose = (doorOpening && useType == USE_TOGGLE) || useType == USE_OFF;

	// TODO: this makes sense to do, logically, but sven doesn't do this
	/*
	if (FBitSet(pev->spawnflags, SF_DOOR_START_OPEN) && useType != USE_TOGGLE) {
		shouldClose = !shouldClose;
	}
	*/

	if (FBitSet(pev->spawnflags, SF_DOOR_NO_AUTO_RETURN) && shouldClose)
	{// door should close
		if (m_toggle_state != TS_AT_BOTTOM)
			DoorGoDown();
	}
	else if (m_toggle_state != TS_AT_TOP)
	{// door should open

		if (m_hActivator != NULL && m_hActivator->IsPlayer())
		{// give health if player opened the door (medikit)
		// VARS( m_eoActivator )->health += m_bHealthValue;

			m_hActivator->TakeHealth(m_bHealthValue, DMG_GENERIC);

		}

		// play door unlock sounds
		PlayLockSounds(pev, &m_ls, FALSE, FALSE);

		DoorGoUp();
	}

	return 1;
}

extern Vector VecBModelOrigin(entvars_t* pevBModel);

//
// Starts the door going to its "up" position (simply ToggleData->vecPosition2).
//
void CBaseDoor::DoorGoUp(void)
{
	entvars_t* pevActivator;

	// emit door moving and stop sounds on CHAN_STATIC so that the multicast doesn't
	// filter them out and leave a client stuck with looping door sounds!
	if (!FBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		if (m_toggle_state != TS_GOING_UP && m_toggle_state != TS_GOING_DOWN)
			EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseMoving), 1, ATTN_NORM);
	}

	m_toggle_state = TS_GOING_UP;

	SetMoveDone(&CBaseDoor::DoorHitTop);
	if (FClassnameIs(pev, "func_door_rotating"))		// !!! BUGBUG Triggered doors don't work with this yet
	{
		float	sign = 1.0;

		if (m_hActivator != NULL)
		{
			pevActivator = m_hActivator->pev;

			if (!FBitSet(pev->spawnflags, SF_DOOR_ONEWAY) && pev->movedir.y) 		// Y axis rotation, move away from the player
			{
				Vector vec = pevActivator->origin - pev->origin;
				Vector angles = pevActivator->angles;
				angles.x = 0;
				angles.z = 0;
				UTIL_MakeVectors(angles);
				//			Vector vnext = (pevToucher->origin + (pevToucher->velocity * 10)) - pev->origin;
				UTIL_MakeVectors(pevActivator->angles);
				Vector vnext = (pevActivator->origin + (gpGlobals->v_forward * 10)) - pev->origin;
				if ((vec.x * vnext.y - vec.y * vnext.x) < 0)
					sign = -1.0;
			}
		}
		AngularMove(m_vecAngle2 * sign, pev->speed);
	}
	else
		LinearMove(m_vecPosition2, pev->speed);

	FireStateTriggers();
}


//
// The door has reached the "up" position.  Either go back down, or wait for another activation.
//
void CBaseDoor::DoorHitTop(void)
{
	if (!FBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseMoving));
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseArrived), 1, ATTN_NORM);
	}

	ASSERT(m_toggle_state == TS_GOING_UP);
	m_toggle_state = TS_AT_TOP;

	// toggle-doors don't come down automatically, they wait for refire.
	if (FBitSet(pev->spawnflags, SF_DOOR_NO_AUTO_RETURN))
	{
		// Re-instate touch method, movement is complete
		if (!FBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
			SetTouch(&CBaseDoor::DoorTouch);
	}
	else
	{
		// In flWait seconds, DoorGoDown will fire, unless wait is -1, then door stays open
		pev->nextthink = pev->ltime + m_flWait;
		SetThink(&CBaseDoor::DoorGoDown);

		if (m_flWait == -1)
		{
			pev->nextthink = -1;
		}
	}

	FireStateTriggers();

	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0); // this isn't finished
}


//
// Starts the door going to its "down" position (simply ToggleData->vecPosition1).
//
void CBaseDoor::DoorGoDown(void)
{
	if (!FBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		if (m_toggle_state != TS_GOING_UP && m_toggle_state != TS_GOING_DOWN)
			EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseMoving), 1, ATTN_NORM);
	}

#ifdef DOOR_ASSERT
	ASSERT(m_toggle_state == TS_AT_TOP);
#endif // DOOR_ASSERT
	m_toggle_state = TS_GOING_DOWN;

	SetMoveDone(&CBaseDoor::DoorHitBottom);
	if (FClassnameIs(pev, "func_door_rotating"))//rotating door
		AngularMove(m_vecAngle1, pev->speed);
	else
		LinearMove(m_vecPosition1, pev->speed);

	FireStateTriggers();
}

//
// The door has reached the "down" position.  Back to quiescence.
//
void CBaseDoor::DoorHitBottom(void)
{
	if (!FBitSet(pev->spawnflags, SF_DOOR_SILENT))
	{
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseMoving));
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseArrived), 1, ATTN_NORM);
	}

	ASSERT(m_toggle_state == TS_GOING_DOWN);
	m_toggle_state = TS_AT_BOTTOM;

	// Re-instate touch method, cycle is complete
	if (FBitSet(pev->spawnflags, SF_DOOR_USE_ONLY))
	{// use only door
		SetTouch(NULL);
	}
	else // touchable door
		SetTouch(&CBaseDoor::DoorTouch);

	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0); // this isn't finished

	FireStateTriggers();
}

void CBaseDoor::Blocked(CBaseEntity* pOther)
{
	CBaseEntity* pentTarget = NULL;
	CBaseDoor* pDoor = NULL;

	// Hurt the blocker a little.
	if (pev->dmg || FClassnameIs(pOther->pev, "monster_tripmine")) {
		if (gpGlobals->time - lastDamage > DOOR_SMASH_MIN_DELAY) {
			lastDamage = gpGlobals->time;
			pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);
		}
	}

	if (pOther->IsMonster() && !pOther->IsAlive()) {
		// don't let corpses block doors
		pOther->Killed(pev, GIB_ALWAYS);
		lastDamage = 0; // allow gibbing multiple players
		return; // don't bounce back because the path is clear now
	}

	// if a door has a negative wait, it would never come back if blocked,
	// so let it just squash the object to death real fast

	if (m_flWait >= 0)
	{
		if (m_toggle_state == TS_GOING_DOWN)
		{
			DoorGoUp();
		}
		else
		{
			DoorGoDown();
		}
	}

	// Block all door pieces with the same targetname here.
	if (!FStringNull(pev->targetname))
	{
		for (;;)
		{
			pentTarget = UTIL_FindEntityByTargetname(pentTarget, STRING(pev->targetname));

			if (!pentTarget)
				break;

			if (pentTarget->pev != pev)
			{
				if (FClassnameIs(pentTarget->pev, "func_door") || FClassnameIs(pentTarget->pev, "func_door_rotating"))
				{

					pDoor = GetClassPtr((CBaseDoor*)VARS(pentTarget->edict()));

					if (pDoor->m_flWait >= 0)
					{
						if (pDoor->pev->velocity == pev->velocity && pDoor->pev->avelocity == pev->velocity)
						{
							// this is the most hacked, evil, bastardized thing I've ever seen. kjb
							if (FClassnameIs(pentTarget->pev, "func_door"))
							{// set origin to realign normal doors
								pDoor->pev->origin = pev->origin;
								pDoor->pev->velocity = g_vecZero;// stop!
							}
							else
							{// set angles to realign rotating doors
								pDoor->pev->angles = pev->angles;
								pDoor->pev->avelocity = g_vecZero;
							}
						}

						if (!FBitSet(pev->spawnflags, SF_DOOR_SILENT))
							STOP_SOUND(ENT(pev), CHAN_STATIC, (char*)STRING(pev->door_noiseMoving));

						if (pDoor->m_toggle_state == TS_GOING_DOWN)
							pDoor->DoorGoUp();
						else
							pDoor->DoorGoDown();
					}
				}
			}
		}
	}
}
