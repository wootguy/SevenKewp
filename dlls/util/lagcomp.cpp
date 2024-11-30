#include "extdll.h"
#include "util.h"
#include "lagcomp.h"
#include <queue>
#include "CBasePlayer.h"

#define MAX_UNLAG_STATES 50
#define MAX_EDICTS 4096 // set larger than the max value gpGlobals->maxEntities can be
#define PING_SMOOTHING_COUNT 100 // number of ping states used to calculate a stable ping value

struct EntState {
	Vector origin;
	Vector angles;
	int sequence;
	float frame;
};

struct WorldState {
	float time;
	EntState ents[MAX_EDICTS];
};

struct RewindInfo {
	bool isCompensated;
	EntState restoreState;
};

int g_historyIdx = 0;

int g_historyWritten = 0;
float g_nextHistoryUpdate = 0;
float g_historyUpdateInterval = 0.02f;
WorldState g_worldHistory[MAX_UNLAG_STATES];
RewindInfo g_rewinds[MAX_EDICTS];
bool g_didRewind = false;
bool g_lagcomp_enabled = false;
cvar_t* sv_unlag = NULL;

int g_pingHistoryIdx = 0;
float g_pingHistory[PING_SMOOTHING_COUNT][32];

int g_currentRewindIdx = 0;

Vector get_lagcomp_offset(int entindex) {
	RewindInfo& info = g_rewinds[entindex];

	if (g_didRewind && info.isCompensated) {
		return info.restoreState.origin - g_worldHistory[g_currentRewindIdx].ents[entindex].origin;
	}

	return g_vecZero;
}

void update_player_pings() {
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBaseEntity* plr = UTIL_PlayerByIndex(i);
		if (!plr) {
			continue;
		}

		int pidx = i - 1;
		int iping, iloss;
		g_engfuncs.pfnGetPlayerStats(plr->edict(), &iping, &iloss);
		g_pingHistory[g_pingHistoryIdx][pidx] = iping / 1000.0f;
	}
	g_pingHistoryIdx = (g_pingHistoryIdx + 1) % PING_SMOOTHING_COUNT;
}

float get_smoothed_ping(CBasePlayer* plr) {
	int pidx = plr->entindex() - 1;
	float smoothedPing = 0;
	for (int k = 0; k < PING_SMOOTHING_COUNT; k++) {
		smoothedPing += g_pingHistory[k][pidx];
	}

	return smoothedPing / (float)PING_SMOOTHING_COUNT;
}

void lagcomp_update() {
	float now = g_engfuncs.pfnTime();

	if (g_nextHistoryUpdate - now > 1.0f) {
		g_nextHistoryUpdate = 0; // level changed.
	}

	if (now < g_nextHistoryUpdate) {
		return;
	}

	if (!sv_unlag) {
		sv_unlag = CVAR_GET_POINTER("sv_unlag");
	}

	if (sv_unlag->value != 2)
		return;

	update_player_pings();

	g_nextHistoryUpdate = now + g_historyUpdateInterval;

	edict_t* edicts = ENT(0);
	WorldState& worldState = g_worldHistory[g_historyIdx];
	worldState.time = g_engfuncs.pfnTime();

	for (int i = gpGlobals->maxClients+1; i < gpGlobals->maxEntities; i++)
	{
		g_rewinds[i].isCompensated = false;

		if (edicts[i].free)
			continue;

		entvars_t& vars = edicts[i].v;

		// TODO: filter by monsters and moving objects too?
		if (!vars.model || (vars.effects & EF_NODRAW) || vars.solid == SOLID_NOT)
			continue;

		g_rewinds[i].isCompensated = true;

		EntState& entState = worldState.ents[i];
		entState.origin = vars.origin;
		entState.angles = vars.angles;
		entState.sequence = vars.sequence;
		entState.frame = vars.frame;
	}

	g_historyIdx = (g_historyIdx+1) % MAX_UNLAG_STATES;
	g_historyWritten = V_min(MAX_UNLAG_STATES, g_historyWritten+1);

	//float duration = g_engfuncs.pfnTime() - now;
}

void lagcomp_begin(CBasePlayer* plr) {
	g_didRewind = false;

	if (sv_unlag->value != 2) {
		return;
	}
	
	int lastHistoryIdx = g_historyIdx - 1;
	if (lastHistoryIdx < 0) {
		lastHistoryIdx = MAX_UNLAG_STATES - 1;
	}

	float ping = get_smoothed_ping(plr);

	int idx = lastHistoryIdx;
	bool foundState = false;
	float now = g_engfuncs.pfnTime();
	float targetTime = now - ping;
	float bestDelta = FLT_MAX;

	for (int i = 0; i < g_historyWritten; i++) {
		float delta = fabs(g_worldHistory[i].time - targetTime);

		if (delta < bestDelta) {
			bestDelta = delta;
			foundState = true;
			idx = i;
		}
	}

	g_currentRewindIdx = idx;

	//ALERT(at_console, "best delta %.3f, comp %.2f ago\n", bestDelta, now - targetTime);

	if (!foundState) {
		ALERT(at_console, "Can't rewind for %.2fs ping.\n", ping);
		return;
	}

	// TODO: you could calculate an in-between state for more accuracy than the update interval allows,
	// but that's a micro optimization with the current method, which is sometimes registering hits
	// several feet away from the monster. For the most part this works (10ms ping feels the same as 500ms) 
	// but sometimes it feels like compensation error is +/-80ms instead of +/-20ms
	WorldState& worldState = g_worldHistory[idx];

	edict_t* edicts = ENT(0);

	// not rewinding players because the engine also does that when sv_unlag != 0
	for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++)
	{		
		if (!g_rewinds[i].isCompensated)
			continue;

		entvars_t& vars = edicts[i].v;

		// save current state for restoring later
		EntState& restoreState = g_rewinds[i].restoreState;
		restoreState.origin = vars.origin;
		restoreState.angles = vars.angles;
		restoreState.sequence = vars.sequence;
		restoreState.frame = vars.frame;

		EntState& rewindState = worldState.ents[i];
		vars.sequence = rewindState.sequence;
		vars.frame = rewindState.frame;
		vars.angles = rewindState.angles;
		UTIL_SetOrigin(&vars, rewindState.origin);
	}

	g_didRewind = true;
}

void lagcomp_end() {
	if (!g_didRewind) {
		return;
	}

	edict_t* edicts = ENT(0);

	for (int i = gpGlobals->maxClients + 1; i < gpGlobals->maxEntities; i++)
	{
		if (!g_rewinds[i].isCompensated)
			continue;

		entvars_t& vars = edicts[i].v;

		EntState& restoreState = g_rewinds[i].restoreState;
		vars.sequence = restoreState.sequence;
		vars.frame = restoreState.frame;
		vars.angles = restoreState.angles;
		UTIL_SetOrigin(&vars, restoreState.origin);
	}

	g_didRewind = false;
}