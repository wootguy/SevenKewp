#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "nodes.h"
#include "env/CSoundEnt.h"
#include "decals.h"
#include "CSprayCan.h"
#include "PluginManager.h"

void CSprayCan::Spawn(entvars_t* pevOwner)
{
	pev->origin = pevOwner->origin + Vector(0, 0, 32);
	pev->angles = pevOwner->v_angle;
	pev->owner = ENT(pevOwner);
	pev->frame = 0;

	SET_MODEL(edict(), "models/player.mdl");
	pev->rendermode = kRenderTransTexture;
	pev->renderamt = 0;

	pev->nextthink = gpGlobals->time + 0.1;
	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/sprayer.wav", 1, ATTN_NORM);
}

void CSprayCan::Think(void)
{
	TraceResult	tr;
	int playernum;
	int nFrames;
	CBasePlayer* pPlayer;

	if (sprayOverride) {
		UTIL_Remove(this);
		return;
	}

	pPlayer = (CBasePlayer*)GET_PRIVATE(pev->owner);

	if (pPlayer)
		nFrames = pPlayer->GetCustomDecalFrames();
	else
		nFrames = -1;

	playernum = ENTINDEX(pev->owner);

	// ALERT(at_console, "Spray by player %i, %i of %i\n", playernum, (int)(pev->frame + 1), nFrames);

	UTIL_MakeVectors(pev->angles);
	UTIL_TraceLine(pev->origin, pev->origin + gpGlobals->v_forward * 128, ignore_monsters, pev->owner, &tr);

	pev->nextthink = gpGlobals->time + 0.1;

	sprayOverride = true;
	CALL_HOOKS_VOID(pfnPlayerSpray, pPlayer, &tr);
	sprayOverride = false;

	// No customization present.
	if (nFrames == -1)
	{
		UTIL_DecalTrace(&tr, DECAL_LAMBDA4);
		UTIL_Remove(this);
	}
	else
	{
		UTIL_PlayerDecalTrace(&tr, playernum, pev->frame, TRUE);
		// Just painted last custom frame.
		if (pev->frame++ >= (nFrames - 1))
			UTIL_Remove(this);
	}
}

