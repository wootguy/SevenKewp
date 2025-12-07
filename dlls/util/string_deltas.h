#include "const.h"
#include <stdint.h>

class CBasePlayer;

typedef uint16_t dstring_t; // index in cache, which gets you the string_t, which then gets the string

extern bool g_visibleEntNames[8192]; // entities that should have their names broadcast if needed

void BroadcastEntNames();

void InitStringDeltasForPlayer(CBasePlayer* plr);

void InitStringDeltas();

// queue a string delta that will eventually be synced to the client.
// return an existing index if the string was already queued, else the new index
dstring_t QueueDeltaString(string_t newString);