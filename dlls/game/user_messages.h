#pragma once

EXPORT extern int giPrecacheGrunt;
EXPORT extern int gmsgShake;
EXPORT extern int gmsgFade;
EXPORT extern int gmsgSelAmmo;
EXPORT extern int gmsgFlashlight;
EXPORT extern int gmsgFlashBattery;
EXPORT extern int gmsgResetHUD;
EXPORT extern int gmsgInitHUD;
EXPORT extern int gmsgShowGameTitle;
EXPORT extern int gmsgCurWeapon;
EXPORT extern int gmsgHealth;
EXPORT extern int gmsgDamage;
EXPORT extern int gmsgBattery;
EXPORT extern int gmsgTrain;
EXPORT extern int gmsgLogo;
EXPORT extern int gmsgWeaponList;
EXPORT extern int gmsgAmmoX;
EXPORT extern int gmsgHudText;
EXPORT extern int gmsgDeathMsg;
EXPORT extern int gmsgScoreInfo;
EXPORT extern int gmsgTeamInfo;
EXPORT extern int gmsgTeamScore;
EXPORT extern int gmsgGameMode;
EXPORT extern int gmsgMOTD;
EXPORT extern int gmsgServerName;
EXPORT extern int gmsgAmmoPickup;
EXPORT extern int gmsgWeapPickup;
EXPORT extern int gmsgItemPickup;
EXPORT extern int gmsgHideWeapon;
EXPORT extern int gmsgSetCurWeap;
EXPORT extern int gmsgSayText;
EXPORT extern int gmsgTextMsg;
EXPORT extern int gmsgSetFOV;
EXPORT extern int gmsgShowMenu;
EXPORT extern int gmsgGeigerRange;
EXPORT extern int gmsgTeamNames;

EXPORT extern int gmsgStatusText;
EXPORT extern int gmsgStatusValue;

EXPORT extern int gmsgToxicCloud;

// If a message in another mod shares the same name as an HL message, then you can reuse the existing ID.
// Otherwise you need to register it so the client can hook it and associate the server's ID.
// If messages from different mods use the same name but have different data lengths, then you'll need
// to use the ReHlds function to end the message with the correct writer or else get SVC_BAD.
// The following are CS 1.6 client messages that don't exist in Half-Life.
EXPORT extern int gmsgAllowSpec_cs16;
EXPORT extern int gmsgForceCam_cs16;
EXPORT extern int gmsgRadar_cs16;

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

void LinkUserMessages(void);