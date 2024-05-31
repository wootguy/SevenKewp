#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "CTalkSquadMonster.h"
#include "gamerules.h"

class CEnvSound : public CPointEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_NORMAL; } // TODO: needed for VIS checks but it shouldn't be
	virtual bool ForceVisChecks() { return true; }
	void KeyValue(KeyValueData* pkvd);
	void Spawn(void);

	void Think(void);

	virtual int		Save(CSave& save);
	virtual int		Restore(CRestore& restore);
	static	TYPEDESCRIPTION m_SaveData[];

	float m_flRadius;
	float m_flRoomtype;
	int clientIdx;
};

LINK_ENTITY_TO_CLASS(env_sound, CEnvSound);
TYPEDESCRIPTION	CEnvSound::m_SaveData[] =
{
	DEFINE_FIELD(CEnvSound, m_flRadius, FIELD_FLOAT),
	DEFINE_FIELD(CEnvSound, m_flRoomtype, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CEnvSound, CBaseEntity);


void CEnvSound::KeyValue(KeyValueData* pkvd)
{

	if (FStrEq(pkvd->szKeyName, "radius"))
	{
		m_flRadius = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	if (FStrEq(pkvd->szKeyName, "roomtype"))
	{
		m_flRoomtype = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

// returns TRUE if the given sound entity (pev) is in range 
// and can see the given player entity (pevTarget)

BOOL FEnvSoundInRange(entvars_t* pev, entvars_t* pevTarget, float* pflRange)
{
	CEnvSound* pSound = GetClassPtr((CEnvSound*)pev);
	Vector vecSpot1 = pev->origin + pev->view_ofs;
	Vector vecSpot2 = pevTarget->origin + pevTarget->view_ofs;
	Vector vecRange;
	float flRange;
	TraceResult tr;

	UTIL_TraceLine(vecSpot1, vecSpot2, ignore_monsters, ENT(pev), &tr);

	// check if line of sight crosses water boundary, or is blocked

	if ((tr.fInOpen && tr.fInWater) || tr.flFraction != 1)
		return FALSE;

	// calc range from sound entity to player

	vecRange = tr.vecEndPos - vecSpot1;
	flRange = vecRange.Length();

	if (pSound->m_flRadius < flRange)
		return FALSE;

	if (pflRange)
		*pflRange = flRange;

	return TRUE;
}

//
// A client that is visible and in range of a sound entity will
// have its room_type set by that sound entity.  If two or more
// sound entities are contending for a client, then the nearest
// sound entity to the client will set the client's room_type.
// A client's room_type will remain set to its prior value until
// a new in-range, visible sound entity resets a new room_type.
//

// CONSIDER: if player in water state, autoset roomtype to 14,15 or 16. 

void CEnvSound::Think(void)
{
	edict_t* pentPlayer = NULL;
	CBasePlayer* pPlayer = NULL;
	float flRange;

	if (!UTIL_IsClientInPVS(edict()))
		goto env_sound_Think_slow; // no player in pvs of sound entity, slow it down

	// cycle through players on each think (0.32s update delay per player, for 32 players)
	
	for (int i = 0; i < 32; i++) {
		clientIdx = (clientIdx + 1) % gpGlobals->maxClients;
		edict_t* ent = INDEXENT(clientIdx + 1);

		if (IsValidPlayer(ent)) {
			pentPlayer = ent;
			break;
		}
	}

	if (!pentPlayer) {
		goto env_sound_Think_slow;
	}

	pPlayer = GetClassPtr((CBasePlayer*)VARS(pentPlayer));

	// check to see if this is the sound entity that is 
	// currently affecting this player

	if (!FNullEnt(pPlayer->m_pentSndLast) && (pPlayer->m_pentSndLast.GetEntity() == this)) {

		// this is the entity currently affecting player, check
		// for validity

		if (pPlayer->m_flSndRoomtype != 0 && pPlayer->m_flSndRange != 0) {

			// we're looking at a valid sound entity affecting
			// player, make sure it's still valid, update range

			if (FEnvSoundInRange(pev, VARS(pentPlayer), &flRange)) {
				pPlayer->m_flSndRange = flRange;
				goto env_sound_Think_fast;
			}
			else {

				// current sound entity affecting player is no longer valid,
				// flag this state by clearing room_type and range.
				// NOTE: we do not actually change the player's room_type
				// NOTE: until we have a new valid room_type to change it to.

				pPlayer->m_flSndRange = 0;
				pPlayer->m_flSndRoomtype = 0;
				goto env_sound_Think_slow;
			}
		}
		else {
			// entity is affecting player but is out of range,
			// wait passively for another entity to usurp it...
			goto env_sound_Think_slow;
		}
	}

	// if we got this far, we're looking at an entity that is contending
	// for current player sound. the closest entity to player wins.

	if (FEnvSoundInRange(pev, VARS(pentPlayer), &flRange))
	{
		if (flRange < pPlayer->m_flSndRange || pPlayer->m_flSndRange == 0)
		{
			// new entity is closer to player, so it wins.
			pPlayer->m_pentSndLast = this;
			pPlayer->m_flSndRoomtype = m_flRoomtype;
			pPlayer->m_flSndRange = flRange;

			// send room_type command to player's server.
			// this should be a rare event - once per change of room_type
			// only!

			//CLIENT_COMMAND(pentPlayer, "room_type %f", m_flRoomtype);

			pPlayer->m_flLastSetRoomtype = m_flRoomtype;

			MESSAGE_BEGIN(MSG_ONE, SVC_ROOMTYPE, NULL, pentPlayer);		// use the magic #1 for "one client"
			WRITE_SHORT((short)m_flRoomtype);					// sequence number
			MESSAGE_END();

			// crank up nextthink rate for new active sound entity
			// by falling through to think_fast...
		}
		// player is not closer to the contending sound entity,
		// just fall through to think_fast. this effectively
		// cranks up the think_rate of entities near the player.
	}

	// player is in pvs of sound entity, but either not visible or
	// not in range. do nothing, fall through to think_fast...

env_sound_Think_fast:
	pev->nextthink = gpGlobals->time + 0.01;
	return;

env_sound_Think_slow:
	pev->nextthink = gpGlobals->time + 0.2;
	return;
}

//
// env_sound - spawn a sound entity that will set player roomtype
// when player moves in range and sight.
//
//
void CEnvSound::Spawn()
{
	// spread think times
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.0, 0.5);

	// must set a model for rehlds to call AddFullToPack
	SET_MODEL(edict(), "models/player.mdl");
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 0;
}
