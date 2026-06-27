#pragma once
#define PLAYER_FATAL_FALL_SPEED		1024// approx 60 feet
#define PLAYER_MAX_SAFE_FALL_SPEED	580// approx 20 feet
#define DAMAGE_FOR_FALL_SPEED		(float) 100 / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED )// damage per unit per second.
#define PLAYER_MIN_BOUNCE_SPEED		200
#define PLAYER_FALL_PUNCH_THRESHHOLD (float)350 // won't punch player's screen/make scrape noise unless player falling at least this fast.

//
// Player PHYSICS FLAGS bits
//
#define		PFLAG_ONLADDER		( 1<<0 )
#define		PFLAG_ONSWING		( 1<<0 )
#define		PFLAG_ONTRAIN		( 1<<1 )
#define		PFLAG_ONBARNACLE	( 1<<2 )
#define		PFLAG_DUCKING		( 1<<3 )		// In the process of ducking, but totally squatted yet
#define		PFLAG_USING			( 1<<4 )		// Using a continuous entity
#define		PFLAG_OBSERVER		( 1<<5 )		// player is locked in stationary cam mode. Spectators can move, observers can't.

//
// generic player
//
//-----------------------------------------------------
//This is Half-Life player entity
//-----------------------------------------------------
#define CSUITPLAYLIST	4		// max of 4 suit sentences queued up at any time

#define SUIT_GROUP			TRUE
#define	SUIT_SENTENCE		FALSE

#define	SUIT_REPEAT_OK		0
#define SUIT_NEXT_IN_30SEC	30
#define SUIT_NEXT_IN_1MIN	60
#define SUIT_NEXT_IN_5MIN	300
#define SUIT_NEXT_IN_10MIN	600
#define SUIT_NEXT_IN_30MIN	1800
#define SUIT_NEXT_IN_1HOUR	3600

#define CSUITNOREPEAT		32

#define	SOUND_FLASHLIGHT_ON		"items/flashlight1.wav"
#define	SOUND_FLASHLIGHT_OFF	"items/flashlight1.wav"
#define	SOUND_NIGHTVISION_ON	"items/nightvision1.wav"
#define	SOUND_NIGHTVISION_OFF	"items/nightvision2.wav"

#define TEAM_NAME_LENGTH	16

#define FAM_NOTHING 0
#define FAM_FORCEVIEWANGLES 1
#define FAM_ADDAVELOCITY 2

#define PLR_IDLE_STATE_TIME 20 // number of idle seconds until considered IDLE in the scoreboard
#define PLR_IDLE_STATE_TIME2 60 // time for big idle
#define PLR_IDLE_STATE_TIME3 120 // time for super idle

#define MAX_MAP_NAME 32

typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
	PLAYER_ATTACK2, // left hand fire
	PLAYER_ATTACK3, // right hand fire
	PLAYER_ATTACK_GL, // m16 grenade launcher
	PLAYER_RELOAD,
	PLAYER_RELOAD2, // left hand reload
	PLAYER_RELOAD3, // right hand reload
	PLAYER_DROP_ITEM,
	PLAYER_USE,
	PLAYER_DEPLOY_WEAPON,
	PLAYER_COCK_WEAPON,
	PLAYER_BARNACLE_HIT,
	PLAYER_BARNACLE_CRUNCH,
} PLAYER_ANIM;

#define MAX_ID_RANGE 4096
#define SBAR_STRING_SIZE 128

enum sbar_data
{
	SBAR_ID_TARGETNAME = 1,
	SBAR_ID_TARGETHEALTH,
	SBAR_ID_TARGETARMOR,
	SBAR_END,
};

#define MAX_CLIENT_ENTS 1665 // default for the latest HL client from steam
#define MAX_LEGACY_CLIENT_ENTS 1365 // default when using the steam_legacy beta

// This was increased to 1024 for HL25 but client limits make most of those new slots useless.
// Sending more than ~800 crashes the client
// Sending more than 512 makes temporary effects disappear (explosions)
// Sending more than 500 makes temporary effects flicker and sometimes disappear
#define MAX_PACKET_ENTITIES 500

// unlike HL25, temporary effects don't start disappearing near the packet entity limit
#define MAX_LEGACY_PACKET_ENTITIES 256

// max number of new entities sent at once when entering a new area.
// too many new entities makes clients freeze with "datagram overflow" which is not
// always recoverable.
#define MAX_NEW_PACKET_ENTITIES_BURST 40

// minimum number of new packet entities per update
#define MIN_NEW_PACKET_ENTITIES 8

enum HL_CLIENT_SYSTEM {
	CLIENT_SYSTEM_NOT_CHECKED, // player hasn't responded to cvar queries yet
	CLIENT_SYSTEM_WINDOWS,
	CLIENT_SYSTEM_LINUX
};

enum HL_CLIENT_ENGINE_VERSION {
	CLIENT_ENGINE_NOT_CHECKED,	// player hasn't responded to cvar queries yet
	CLIENT_ENGINE_HL_LATEST,	// the latest version of the HL client from steam
	CLIENT_ENGINE_HL_LEGACY,	// the legacy version of HL from steam
	CLIENT_ENGINE_BOT,			// bots don't use a client
};

enum HL_CLIENT_MOD_VERSION {
	CLIENT_MOD_NOT_CHECKED,	// player hasn't responded to cvar queries yet
	CLIENT_MOD_HL,			// the vanilla half-life mod from steam (or an undetected custom client)
	CLIENT_MOD_SEVENKEWP,	// The client for this mod
	CLIENT_MOD_HLBUGFIXED,	// a popular custom client (for cheating!!! but also cool stuff...)
	CLIENT_MOD_ADRENALINE,	// another popular custom client (players with names like "^1J^2E^3F^4F")
	CLIENT_MOD_BOT,			// bots don't use mods
};

enum HL_CLIENT_RENDERER {
	CLIENT_RENDERER_NOT_CHECKED, // player hasn't responded to cvar queries yet
	CLIENT_RENDERER_OPENGL,
	CLIENT_RENDERER_SOFTWARE
};

struct client_info_t {
	int engine_version;
	int mod_version;
	int renderer;
	int max_edicts;
	int max_packet_entities;
};

enum PLAYER_CLASS {
	PCLASS_DEFAULT, // give player the default weapons for their client
	PCLASS_HL,		// give player vanilla HL weapons when possible
};

// debug flags
#define DF_NODES 1 // display nearby nodes and connections

enum PLAYER_MODEL_ANIM_SET {
	PMODEL_ANIMS_HALF_LIFE,			// standard valve player model animations
	PMODEL_ANIMS_HALF_LIFE_COOP,	// sven co-op 5.x model ported to this mod (preserves HL anim order)
	PMODEL_ANIMS_SVEN_COOP_5,		// standard sven co-op 5.x player model animations
};