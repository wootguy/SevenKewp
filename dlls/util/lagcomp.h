#pragma once

EXPORT float get_smoothed_ping(CBasePlayer* plr);

// call every server frame to save world state and player pings
void lagcomp_update();

// begin lag compensation for this player
// call this before doing any traces from the given player's perspective
// this will rewind the world state to match what the player saw X ms ago (X = player ping)
EXPORT void lagcomp_begin(CBasePlayer* plr);

// must be called after lagcomp_begin to restore the current world state after a rewind
EXPORT void lagcomp_end();

// returns difference between lag compensated position and server's position
EXPORT Vector get_lagcomp_offset(int entindex);