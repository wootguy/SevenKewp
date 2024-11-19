#pragma once

// call every server frame to save world state and player pings
void lagcomp_update();

// begin lag compensation for this player
// call this before doing any traces from the given player's perspective
// this will rewind the world state to match what the player saw X ms ago (X = player ping)
void lagcomp_begin(CBasePlayer* plr);

// must be called after lagcomp_begin to restore the current world state after a rewind
void lagcomp_end();
