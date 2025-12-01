#pragma once
#include <stdint.h>

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
EXPORT extern int gmsgCurWeaponX; // Larger clip size
EXPORT extern int gmsgHealth;
EXPORT extern int gmsgDamage;
EXPORT extern int gmsgBattery;
EXPORT extern int gmsgTrain;
EXPORT extern int gmsgLogo;
EXPORT extern int gmsgWeaponList;
EXPORT extern int gmsgWeaponListX; // more parameters
EXPORT extern int gmsgAmmoX;
EXPORT extern int gmsgAmmoXX; // Extra-Extra-Large ammo count <:)
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

EXPORT extern int gmsgCustomWeapon; // custom weapon prediction parameters
EXPORT extern int gmsgCustomWeaponEvents; // custom weapon prediction parameters (events)
EXPORT extern int gmsgSoundIdx; // mapping of sound indexes to file paths
EXPORT extern int gmsgNextMap; // next map name
EXPORT extern int gmsgTimeLeft; // seconds left before map ends (0 = infinite)
EXPORT extern int gmsgFog; // fog rendering parameters
EXPORT extern int gmsgPmodelAnim; // third person weapon animation
EXPORT extern int gmsgWeaponBits; // which weapons does this client have (for weapon IDs >= 32)
EXPORT extern int gmsgTagInfo; // data for name tags
EXPORT extern int gmsgPlayerPos; // tag positions for players outside of the PVS
EXPORT extern int gmsgToxicCloud; // chumtoad attack
EXPORT extern int gmsgSpriteAdv; // chumtoad attack
EXPORT extern int gmsgWaterSplash; // chumtoad attack

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