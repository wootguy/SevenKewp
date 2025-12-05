#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "user_messages.h"
#include "string_deltas.h"

#define MAX_KNOWN_ENT_NAMES 65536
bool g_visibleEntNames[8192]; // entities that should have their names broadcast if needed
bool g_playerEntNamesKnown[32][MAX_KNOWN_ENT_NAMES]; // set of string_t known by each client

HashMap<dstring_t> g_knownStringLookup; // lookup an index in the cache by its string value
string_t g_knownStrings[MAX_KNOWN_ENT_NAMES]; // all strings that can be delta'd to clients
dstring_t g_lastKnownStringIdx = 1; // first index will be a blank string_t

bool g_didOverflowError = false;

void InitStringDeltas() {
	g_lastKnownStringIdx = 1;
	g_knownStringLookup.clear();
}

void InitStringDeltasForPlayer(CBasePlayer* plr) {
	memset(g_playerEntNamesKnown[plr->entindex() - 1], 0, sizeof(g_playerEntNamesKnown[0]));
	g_didOverflowError = false;
}

dstring_t AddDeltaString(string_t newString) {
	if (!STRING(newString)[0])
		return 0;

	if (g_lastKnownStringIdx >= MAX_KNOWN_ENT_NAMES - 1) {
		if (g_didOverflowError) {
			ALERT(at_error, "Overflowed max known ent names!\n");
			g_didOverflowError = true;
		}
		return 0;
	}

	// check if this string was already inserted
	dstring_t* existing = g_knownStringLookup.get(STRING(newString));
	if (existing) {
		return *existing;
	}

	// add the unique name
	dstring_t newIdx = g_lastKnownStringIdx;
	g_knownStrings[newIdx] = newString;
	g_knownStringLookup.put(STRING(newString), newIdx);
	g_lastKnownStringIdx++;

	return newIdx;
}

void BroadcastEntNames() {
	static float lastUpdate = 0;

	if (gpGlobals->time - lastUpdate < 0.05f && lastUpdate < gpGlobals->time) {
		return;
	}

	lastUpdate = gpGlobals->time;

	static dstring_t g_updateEntNames[8192];
	int knownCnt = 0;

	// update cached names and get list of strings that need to be known
	for (int k = 1; k < gpGlobals->maxEntities; k++) {
		if (!g_visibleEntNames[k]) {
			continue;
		}

		CBaseEntity* ent = CBaseEntity::Instance(INDEXENT(k));
		if (!ent)
			continue;

		ent->m_cachedDisplayName = AddDeltaString(ALLOC_STRING(ent->DisplayName()));
		ent->m_cachedDisplayHint = AddDeltaString(ALLOC_STRING(ent->DisplayHint()));

		if (ent->m_cachedDisplayName >= 1024) 
			ALERT(at_error, "Overflow name string delta %d: %s\n", (int)ent->m_cachedDisplayName, ent->DisplayHint());
		else if (ent->m_cachedDisplayName)
			g_updateEntNames[knownCnt++] = ent->m_cachedDisplayName;

		if (ent->m_cachedDisplayHint >= 512)
			ALERT(at_error, "Overflow hint string delta %d: %s\n", (int)ent->m_cachedDisplayHint, ent->DisplayHint());
		else if (ent->m_cachedDisplayHint)
			g_updateEntNames[knownCnt++] = ent->m_cachedDisplayHint;
	}

	// send updates
	for (int i = 1; i < gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);

		if (!plr || !plr->IsSevenKewpClient())
			continue;

		bool* known = g_playerEntNamesKnown[i - 1];

		for (int k = 0; k < knownCnt; k++) {
			dstring_t dstr = g_updateEntNames[k];

			if (known[dstr]) {
				continue; // already know this name
			}

			string_t namet = g_knownStrings[dstr];

			const int max_name_len = 180;
			static char buffer[max_name_len];
			strcpy_safe(buffer, STRING(namet), max_name_len);

			MESSAGE_BEGIN(MSG_ONE, gmsgStringIdx, NULL, plr->pev);
			WRITE_SHORT(dstr);
			WRITE_STRING(buffer);
			MESSAGE_END();
			known[dstr] = true;

			break; // only one new string per update to prevent overflows
		}
	}

	memset(g_visibleEntNames, 0, sizeof(g_visibleEntNames));
}