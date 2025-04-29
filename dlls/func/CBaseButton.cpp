#include "extdll.h"
#include "util.h"
#include "saverestore.h"
#include "CBaseButton.h"
#include "te_effects.h"

// CBaseButton
TYPEDESCRIPTION CBaseButton::m_SaveData[] =
{
	DEFINE_FIELD(CBaseButton, m_fStayPushed, FIELD_BOOLEAN),
	DEFINE_FIELD(CBaseButton, m_fRotating, FIELD_BOOLEAN),

	DEFINE_FIELD(CBaseButton, m_sounds, FIELD_INTEGER),
	DEFINE_FIELD(CBaseButton, m_bLockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseButton, m_bLockedSentence, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseButton, m_bUnlockedSound, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseButton, m_bUnlockedSentence, FIELD_CHARACTER),
	DEFINE_FIELD(CBaseButton, m_strChangeTarget, FIELD_STRING),
	//	DEFINE_FIELD( CBaseButton, m_ls, FIELD_??? ),   // This is restored in Precache()
};

IMPLEMENT_SAVERESTORE(CBaseButton, CBaseToggle)

void CBaseButton::Precache(void)
{
	const char* pszSound;

	if (FBitSet(pev->spawnflags, SF_BUTTON_SPARK_IF_OFF))// this button should spark in OFF state
	{
		PRECACHE_SOUND_ARRAY(g_sparkSounds);
	}

	// get door button sounds, for doors which require buttons to open

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
	case 1: m_ls.sLockedSentence = MAKE_STRING("NA"); break; // access denied
	case 2: m_ls.sLockedSentence = MAKE_STRING("ND"); break; // security lockout
	case 3: m_ls.sLockedSentence = MAKE_STRING("NF"); break; // blast door
	case 4: m_ls.sLockedSentence = MAKE_STRING("NFIRE"); break; // fire door
	case 5: m_ls.sLockedSentence = MAKE_STRING("NCHEM"); break; // chemical door
	case 6: m_ls.sLockedSentence = MAKE_STRING("NRAD"); break; // radiation door
	case 7: m_ls.sLockedSentence = MAKE_STRING("NCON"); break; // gen containment
	case 8: m_ls.sLockedSentence = MAKE_STRING("NH"); break; // maintenance door
	case 9: m_ls.sLockedSentence = MAKE_STRING("NG"); break; // broken door

	default: m_ls.sLockedSentence = 0; break;
	}

	switch (m_bUnlockedSentence)
	{
	case 1: m_ls.sUnlockedSentence = MAKE_STRING("EA"); break; // access granted
	case 2: m_ls.sUnlockedSentence = MAKE_STRING("ED"); break; // security door
	case 3: m_ls.sUnlockedSentence = MAKE_STRING("EF"); break; // blast door
	case 4: m_ls.sUnlockedSentence = MAKE_STRING("EFIRE"); break; // fire door
	case 5: m_ls.sUnlockedSentence = MAKE_STRING("ECHEM"); break; // chemical door
	case 6: m_ls.sUnlockedSentence = MAKE_STRING("ERAD"); break; // radiation door
	case 7: m_ls.sUnlockedSentence = MAKE_STRING("ECON"); break; // gen containment
	case 8: m_ls.sUnlockedSentence = MAKE_STRING("EH"); break; // maintenance door

	default: m_ls.sUnlockedSentence = 0; break;
	}
}

//
// Cache user-entity-field values until spawn is called.
//

void CBaseButton::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "changetarget"))
	{
		m_strChangeTarget = ALLOC_STRING(pkvd->szValue);
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
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

//
// ButtonShot
//
int CBaseButton::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	BUTTON_CODE code = ButtonResponseToTouch();

	if (code == BUTTON_NOTHING)
		return 0;
	// Temporarily disable the touch function, until movement is finished.
	SetTouch(NULL);

	m_hActivator = CBaseEntity::Instance(pevAttacker);
	if (m_hActivator == NULL)
		return 0;

	if (code == BUTTON_RETURN)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

		// Toggle buttons fire when they get back to their "home" position
		if (!(pev->spawnflags & SF_BUTTON_TOGGLE))
			SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);
		ButtonReturn();
	}
	else // code == BUTTON_ACTIVATE
		ButtonActivate();

	return 0;
}

/*QUAKED func_button (0 .5 .8) ?
When a button is touched, it moves some distance in the direction of it's angle,
triggers all of it's targets, waits some time, then returns to it's original position
where it can be triggered again.

"angle"		determines the opening direction
"target"	all entities with a matching targetname will be used
"speed"		override the default 40 speed
"wait"		override the default 1 second wait (-1 = never return)
"lip"		override the default 4 pixel lip remaining at end of move
"health"	if set, the button must be killed instead of touched
"sounds"
0) steam metal
1) wooden clunk
2) metallic click
3) in-out
*/
LINK_ENTITY_TO_CLASS(func_button, CBaseButton)


void CBaseButton::Spawn()
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	const char* pszSound;

	//----------------------------------------------------
	//determine sounds for buttons
	//a sound of 0 should not make a sound
	//----------------------------------------------------
	pszSound = ButtonSound(m_sounds);
	PRECACHE_SOUND(pszSound);
	pev->noise = ALLOC_STRING(pszSound);

	Precache();

	if (FBitSet(pev->spawnflags, SF_BUTTON_SPARK_IF_OFF))// this button should spark in OFF state
	{
		SetThink(&CBaseButton::ButtonSpark);
		pev->nextthink = gpGlobals->time + 0.5;// no hurry, make sure everything else spawns
	}

	SetMovedir(pev);

	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 40;

	if (pev->health > 0)
	{
		pev->takedamage = DAMAGE_YES;
	}

	if (m_flWait == 0)
		m_flWait = 1;
	if (m_flLip == 0)
		m_flLip = 4;

	m_toggle_state = TS_AT_BOTTOM;
	m_vecPosition1 = pev->origin;
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2 = m_vecPosition1 + (pev->movedir * (fabs(pev->movedir.x * (pev->size.x - 2)) + fabs(pev->movedir.y * (pev->size.y - 2)) + fabs(pev->movedir.z * (pev->size.z - 2)) - m_flLip));


	// Is this a non-moving button?
	if (((m_vecPosition2 - m_vecPosition1).Length() < 1) || (pev->spawnflags & SF_BUTTON_DONTMOVE))
		m_vecPosition2 = m_vecPosition1;

	m_fStayPushed = (m_flWait == -1 ? TRUE : FALSE);
	m_fRotating = FALSE;

	// if the button is flagged for USE button activation only, take away it's touch function and add a use function

	if (FBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY)) // touchable button
	{
		SetTouch(&CBaseButton::ButtonTouch);
	}
	else
	{
		SetTouch(NULL);
		SetUse(&CBaseButton::ButtonUse);
	}
}


// Button sound table. 
// Also used by CBaseDoor to get 'touched' door lock/unlock sounds

const char* ButtonSound(int sound)
{
	const char* pszSound;

	switch (sound)
	{
	case 0: pszSound = "common/null.wav";        break;
	case 1: pszSound = "buttons/button1.wav";	break;
	case 2: pszSound = "buttons/button2.wav";	break;
	case 3: pszSound = "buttons/button3.wav";	break;
	case 4: pszSound = "buttons/button4.wav";	break;
	case 5: pszSound = "buttons/button5.wav";	break;
	case 6: pszSound = "buttons/button6.wav";	break;
	case 7: pszSound = "buttons/button7.wav";	break;
	case 8: pszSound = "buttons/button8.wav";	break;
	case 9: pszSound = "buttons/button9.wav";	break;
	case 10: pszSound = "buttons/button10.wav";	break;
	case 11: pszSound = "buttons/button11.wav";	break;
	case 12: pszSound = "buttons/latchlocked1.wav";	break;
	case 13: pszSound = "buttons/latchunlocked1.wav";	break;
	case 14: pszSound = "buttons/lightswitch2.wav"; break;

		// next 6 slots reserved for any additional sliding button sounds we may add

	case 21: pszSound = "buttons/lever1.wav";	break;
	case 22: pszSound = "buttons/lever2.wav";	break;
	case 23: pszSound = "buttons/lever3.wav";	break;
	case 24: pszSound = "buttons/lever4.wav";	break;
	case 25: pszSound = "buttons/lever5.wav";	break;

	default:pszSound = "buttons/button9.wav";	break;
	}

	return pszSound;
}

//
// Makes flagged buttons spark when turned off
//

void DoSpark(entvars_t* pev, const Vector& location)
{
	Vector tmp = location + pev->size * 0.5;
	UTIL_Sparks(tmp);

	float flVolume = RANDOM_FLOAT(0.25, 0.75) * 0.4;//random volume range
	UTIL_TempSound(pev->origin, RANDOM_SOUND_ARRAY(g_sparkSounds), flVolume);
}

void CBaseButton::ButtonSpark(void)
{
	SetThink(&CBaseButton::ButtonSpark);
	pev->nextthink = gpGlobals->time + (0.1 + RANDOM_FLOAT(0, 1.5));// spark again at random interval

	DoSpark(pev, pev->mins);
}


//
// Button's Use function
//
void CBaseButton::ButtonUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	// Ignore touches if button is moving, or pushed-in and waiting to auto-come-out.
	// UNDONE: Should this use ButtonResponseToTouch() too?
	if (m_toggle_state == TS_GOING_UP || m_toggle_state == TS_GOING_DOWN)
		return;

	if (!RunInventoryRules(pCaller)) {
		return;
	}

	m_hActivator = pActivator;
	if (m_toggle_state == TS_AT_TOP)
	{
		if (!m_fStayPushed && FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE))
		{
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

			//SUB_UseTargets( m_eoActivator );
			ButtonReturn();
		}
	}
	else
		ButtonActivate();
}


CBaseButton::BUTTON_CODE CBaseButton::ButtonResponseToTouch(void)
{
	// Ignore touches if button is moving, or pushed-in and waiting to auto-come-out.
	if (m_toggle_state == TS_GOING_UP ||
		m_toggle_state == TS_GOING_DOWN ||
		(m_toggle_state == TS_AT_TOP && !m_fStayPushed && !FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE)))
		return BUTTON_NOTHING;

	if (m_toggle_state == TS_AT_TOP)
	{
		if ((FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE)) && !m_fStayPushed)
		{
			return BUTTON_RETURN;
		}
	}
	else
		return BUTTON_ACTIVATE;

	return BUTTON_NOTHING;
}


//
// Touching a button simply "activates" it.
//
void CBaseButton::ButtonTouch(CBaseEntity* pOther)
{
	// Ignore touches by anything but players
	if (!FClassnameIs(pOther->pev, "player"))
		return;

	m_hActivator = pOther;

	BUTTON_CODE code = ButtonResponseToTouch();

	if (code == BUTTON_NOTHING)
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
	{
		// play button locked sound
		PlayLockSounds(pev, &m_ls, TRUE, TRUE);
		return;
	}

	if (!RunInventoryRules(pOther)) {
		return;
	}

	// Temporarily disable the touch function, until movement is finished.
	SetTouch(NULL);

	if (code == BUTTON_RETURN)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);
		SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);
		ButtonReturn();
	}
	else	// code == BUTTON_ACTIVATE
		ButtonActivate();
}

//
// Starts the button moving "in/up".
//
void CBaseButton::ButtonActivate()
{
	EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
	{
		// button is locked, play locked sound
		PlayLockSounds(pev, &m_ls, TRUE, TRUE);
		return;
	}
	else
	{
		// button is unlocked, play unlocked sound
		PlayLockSounds(pev, &m_ls, FALSE, TRUE);
	}

	ASSERT(m_toggle_state == TS_AT_BOTTOM);
	m_toggle_state = TS_GOING_UP;

	SetMoveDone(&CBaseButton::TriggerAndWait);
	if (!m_fRotating) {
		if (pev->spawnflags & SF_BUTTON_DONTMOVE) {
			TriggerAndWait();
		}
		else {
			LinearMove(m_vecPosition2, pev->speed);
		}
	}
	else
		AngularMove(m_vecAngle2, pev->speed);
}

//
// Button has reached the "in/up" position.  Activate its "targets", and pause before "popping out".
//
void CBaseButton::TriggerAndWait(void)
{
	ASSERT(m_toggle_state == TS_GOING_UP);

	if (!UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
		return;

	m_toggle_state = TS_AT_TOP;

	// If button automatically comes back out, start it moving out.
	// Else re-instate touch method
	if (m_fStayPushed || FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE))
	{
		if (!FBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY)) // this button only works if USED, not touched!
		{
			// ALL buttons are now use only
			SetTouch(NULL);
		}
		else
			SetTouch(&CBaseButton::ButtonTouch);
	}
	else
	{
		pev->nextthink = pev->ltime + m_flWait;
		SetThink(&CBaseButton::ButtonReturn);
	}

	pev->frame = 1;			// use alternate textures


	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);
}


//
// Starts the button moving "out/down".
//
void CBaseButton::ButtonReturn(void)
{
	ASSERT(m_toggle_state == TS_AT_TOP);
	m_toggle_state = TS_GOING_DOWN;

	SetMoveDone(&CBaseButton::ButtonBackHome);
	if (!m_fRotating) {
		if (pev->spawnflags & SF_BUTTON_DONTMOVE) {
			ButtonBackHome();
		}
		else {
			LinearMove(m_vecPosition1, pev->speed);
		}
	}
	else
		AngularMove(m_vecAngle1, pev->speed);

	pev->frame = 0;			// use normal textures
}


//
// Button has returned to start state.  Quiesce it.
//
void CBaseButton::ButtonBackHome(void)
{
	ASSERT(m_toggle_state == TS_GOING_DOWN);
	m_toggle_state = TS_AT_BOTTOM;

	if (FBitSet(pev->spawnflags, SF_BUTTON_TOGGLE))
	{
		//EMIT_SOUND(ENT(pev), CHAN_VOICE, (char*)STRING(pev->noise), 1, ATTN_NORM);

		SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);
	}


	if (!FStringNull(pev->target))
	{
		CBaseEntity* pentTarget = NULL;
		for (;;)
		{
			pentTarget = UTIL_FindEntityByTargetname(pentTarget, STRING(pev->target));

			if (!pentTarget)
				break;

			if (!FClassnameIs(pentTarget->pev, "multisource"))
				continue;

			pentTarget->Use(m_hActivator, this, USE_TOGGLE, 0);
		}
	}

	// Re-instate touch method, movement cycle is complete.
	if (!FBitSet(pev->spawnflags, SF_BUTTON_TOUCH_ONLY)) // this button only works if USED, not touched!
	{
		// All buttons are now use only	
		SetTouch(NULL);
	}
	else
		SetTouch(&CBaseButton::ButtonTouch);

	// reset think for a sparking button
	if (FBitSet(pev->spawnflags, SF_BUTTON_SPARK_IF_OFF))
	{
		SetThink(&CBaseButton::ButtonSpark);
		pev->nextthink = gpGlobals->time + 0.5;// no hurry.
	}
}

