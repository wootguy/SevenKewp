class CBasePlayer;

typedef uint16_t dstring_t; // index in cache, which gets you the string_t, which then gets the string

extern bool g_visibleEntNames[8192]; // entities that should have their names broadcast if needed

void BroadcastEntNames();

void InitStringDeltasForPlayer(CBasePlayer* plr);

void InitStringDeltas();