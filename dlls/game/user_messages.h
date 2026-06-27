#pragma once
#include <stdint.h>

#define INIT_USER_MSG(name, size) \
	g_umsg.name = REG_USER_MSG(#name, size)

// Custom user messages.
// Field names match the hook functions names in the client code.
// 11 characters max.
struct UserMessageIds {
	// HLDM messages:
	int ScreenShake;
	int ScreenFade;
	int SelAmmo;		// unused
	int Flashlight;
	int FlashBat;
	int ResetHUD;		// called every respawn
	int InitHUD;		// called every time a new player joins the server
	int GameTitle;
	int CurWeapon;
	int CurWeaponX;		// Larger clip size
	int Health;
	int Damage;
	int Battery;
	int Train;
	int WeaponList;
	int WeaponListX;	// more parameters
	int AmmoX;
	int AmmoXX;			// Extra-Extra-Large ammo count <:)
	int HudText;		// we don't use the message but 3rd party addons may!
	int DeathMsg;
	int ScoreInfo;
	int TeamInfo;		// sets the name of a player's team
	int TeamScore;		// sets the score of a team on the scoreboard
	int GameMode;
	int MOTD;
	int ServerName;
	int AmmoPickup;
	int WeapPickup;
	int ItemPickup;
	int HideWeapon;
	int SetCurWeap;
	int SayText;
	int TextMsg;
	int SetFOV;
	int ShowMenu;
	int Geiger;
	int TeamNames;
	int Logo;			// initialized later
	int StatusText;
	int StatusValue;

	// SevenKewp messages:
	int CustomWep;		// custom weapon prediction parameters
	int CustomWepAk;	// custom weapon prediction parameters for a single attack
	int CustomWepEv;	// custom weapon prediction parameters (events)
	int NextMap;		// next map name
	int TimeLeft;		// seconds left before map ends (0 = infinite)
	int Fog;			// fog rendering parameters
	int PmodelAnim;		// third person weapon animation
	int WeaponBits;		// which weapons does this client have (for weapon IDs >= 32)
	int TagInfo;		// data for name tags
	int PlayerPos;		// tag positions for players outside of the PVS
	int ToxicCloud;		// chumtoad attack
	int SpriteAdv;		// chumtoad attack
	int WaterSplash;	// chumtoad attack
	int PredFiles;		// prediction related file indexes
	int PredCvars;		// prediction related cvar values
	int PredMove;		// prediction related movement values
	int MatsPath;		// path to a custom materials file
	int CustomHud;		// custom hud dir to load for a weapon
	int HudColor;		// Set default HUD color
	int HudConPrint;	// Dispaly text using the console/chat font
	int StringIdx;		// send a string_t to the client so it can look it up by index
	int HudSprite;		// Display a sprite on the client HUD
	int HudSprTogl;		// Toggle HUD sprite visibility
	int HudUpdNum;		// Update HUD numeric display value
	int Tracer2;		// Tracer with increased range (+/-4k -> +/-32k) and a customizable color
	int BloodSpr2;		// Blood sprite effect with increased range (+/-4k -> +/-32k)
	int TempFx;			// SVC_TEMPENTITY effect with increased range (+/-4k -> +/-32k)
};

EXPORT extern UserMessageIds g_umsg;

// Note: also update msgTypeStr() in util.cpp when adding new messages

typedef struct _SelAmmo
{
	BYTE	Ammo1Type;
	BYTE	Ammo1;
	BYTE	Ammo2Type;
	BYTE	Ammo2;
} SelAmmo;

struct UserMessage {
	const char* name;
	int id;
	int size;
};

extern std::vector<UserMessage> g_userMessages;

#ifdef CLIENT_DLL
#define REG_USER_MSG				(*g_engfuncs.pfnRegUserMsg)
#else
EXPORT int REG_USER_MSG(const char* name, int size);
#endif

// Find a usermsg, registered by the gamedll, with the corresponding
// msgname, and return remaining info about it (msgid, size). -metamod
EXPORT int GetUserMsgInfo(const char* msgname, int* size);
EXPORT int GetUserMsgInfo(int id, int* size);

EXPORT UserMessage* GetUserMsg(int id); // returns NULL if not found

void LinkUserMessages(void);