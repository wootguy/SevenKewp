#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "CBasePlayer.h"
#include "saverestore.h"
#include "trains.h"			// trigger_camera has train functionality
#include "gamerules.h"
#include "player_util.h"

void PlayCDTrack(int iTrack)
{
	edict_t* pClient;

	// manually find the single player. 
	pClient = g_engfuncs.pfnPEntityOfEntIndex(1);

	// Can't play if the client is not connected!
	if (!pClient)
		return;

	if (iTrack < -1 || iTrack > 30)
	{
		ALERT(at_console, "TriggerCDAudio - Track %d out of range\n");
		return;
	}

	if (iTrack == -1)
	{
		CLIENT_COMMAND(pClient, "cd stop\n");
	}
	else
	{
		char string[64];

		sprintf(string, "cd play %3d\n", iTrack);
		CLIENT_COMMAND(pClient, string);
	}
}