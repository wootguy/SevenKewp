#include "extdll.h"
#include "util.h"
#include "shake.h"
#include "user_messages.h"
#include "PluginManager.h"

int giPrecacheGrunt = 0;
int gmsgShake = 0;
int gmsgFade = 0;
int gmsgSelAmmo = 0;
int gmsgFlashlight = 0;
int gmsgFlashBattery = 0;
int gmsgResetHUD = 0;
int gmsgInitHUD = 0;
int gmsgShowGameTitle = 0;
int gmsgCurWeapon = 0;
int gmsgHealth = 0;
int gmsgDamage = 0;
int gmsgBattery = 0;
int gmsgTrain = 0;
int gmsgLogo = 0;
int gmsgWeaponList = 0;
int gmsgAmmoX = 0;
int gmsgHudText = 0;
int gmsgDeathMsg = 0;
int gmsgScoreInfo = 0;
int gmsgTeamInfo = 0;
int gmsgTeamScore = 0;
int gmsgGameMode = 0;
int gmsgMOTD = 0;
int gmsgServerName = 0;
int gmsgAmmoPickup = 0;
int gmsgWeapPickup = 0;
int gmsgItemPickup = 0;
int gmsgHideWeapon = 0;
int gmsgSetCurWeap = 0;
int gmsgSayText = 0;
int gmsgTextMsg = 0;
int gmsgSetFOV = 0;
int gmsgShowMenu = 0;
int gmsgGeigerRange = 0;
int gmsgTeamNames = 0;

int gmsgStatusText = 0;
int gmsgStatusValue = 0;

int gmsgToxicCloud = 0;

int gmsgAllowSpec_cs16;
int gmsgForceCam_cs16;
int gmsgRadar_cs16;

std::vector<UserMessage> g_userMessages;

int REG_USER_MSG(const char* name, int size) {
	CALL_HOOKS(int, pfnRegUserMsg, name, size);

	UserMessage msg;
	msg.name = name;
	msg.size = size;

	msg.id = g_engfuncs.pfnRegUserMsg(name, size);

	g_userMessages.push_back(msg);

	return msg.id;
}

int GetUserMsgInfo(const char* msgname, int* size) {
	for (int i = 0; i < (int)g_userMessages.size(); i++) {
		UserMessage& msg = g_userMessages[i];

		if (!strcmp(msg.name, msgname)) {
			if (size)
				*size = msg.size;
			return msg.id;
		}
	}

	return 0;
}

int GetUserMsgInfo(int id, int* size) {
	for (int i = 0; i < (int)g_userMessages.size(); i++) {
		UserMessage& msg = g_userMessages[i];

		if (msg.id == id) {
			if (size)
				*size = msg.size;
			return msg.id;
		}
	}

	return 0;
}

void LinkUserMessages(void)
{
	// Already taken care of?
	if (gmsgSelAmmo)
	{
		return;
	}

	gmsgSelAmmo = REG_USER_MSG("SelAmmo", sizeof(SelAmmo));
	gmsgCurWeapon = REG_USER_MSG("CurWeapon", 3);
	gmsgGeigerRange = REG_USER_MSG("Geiger", 1);
	gmsgFlashlight = REG_USER_MSG("Flashlight", 2);
	gmsgFlashBattery = REG_USER_MSG("FlashBat", 1);
	gmsgHealth = REG_USER_MSG("Health", 1);
	gmsgDamage = REG_USER_MSG("Damage", 12);
	gmsgBattery = REG_USER_MSG("Battery", 2);
	gmsgTrain = REG_USER_MSG("Train", 1);
	//gmsgHudText = REG_USER_MSG( "HudTextPro", -1 );
	gmsgHudText = REG_USER_MSG("HudText", -1); // we don't use the message but 3rd party addons may!
	gmsgSayText = REG_USER_MSG("SayText", -1);
	gmsgTextMsg = REG_USER_MSG("TextMsg", -1);
	gmsgWeaponList = REG_USER_MSG("WeaponList", -1);
	gmsgResetHUD = REG_USER_MSG("ResetHUD", 1);		// called every respawn
	gmsgInitHUD = REG_USER_MSG("InitHUD", 0);		// called every time a new player joins the server
	gmsgShowGameTitle = REG_USER_MSG("GameTitle", 1);
	gmsgDeathMsg = REG_USER_MSG("DeathMsg", -1);
	gmsgScoreInfo = REG_USER_MSG("ScoreInfo", 9);
	gmsgTeamInfo = REG_USER_MSG("TeamInfo", -1);  // sets the name of a player's team
	gmsgTeamScore = REG_USER_MSG("TeamScore", -1);  // sets the score of a team on the scoreboard
	gmsgGameMode = REG_USER_MSG("GameMode", 1);
	gmsgMOTD = REG_USER_MSG("MOTD", -1);
	gmsgServerName = REG_USER_MSG("ServerName", -1);
	gmsgAmmoPickup = REG_USER_MSG("AmmoPickup", 2);
	gmsgWeapPickup = REG_USER_MSG("WeapPickup", 1);
	gmsgItemPickup = REG_USER_MSG("ItemPickup", -1);
	gmsgHideWeapon = REG_USER_MSG("HideWeapon", 1);
	gmsgSetFOV = REG_USER_MSG("SetFOV", 1);
	gmsgShowMenu = REG_USER_MSG("ShowMenu", -1);
	gmsgShake = REG_USER_MSG("ScreenShake", sizeof(ScreenShake));
	gmsgFade = REG_USER_MSG("ScreenFade", sizeof(ScreenFade));
	gmsgAmmoX = REG_USER_MSG("AmmoX", 2);
	gmsgTeamNames = REG_USER_MSG("TeamNames", -1);

	gmsgStatusText = REG_USER_MSG("StatusText", -1);
	gmsgStatusValue = REG_USER_MSG("StatusValue", 3);

	// Sven Co-op messages (unused)
	gmsgToxicCloud = REG_USER_MSG("ToxicCloud", 4);

	// CS 1.6 messages
	//gmsgAllowSpec_cs16 = REG_USER_MSG("AllowSpec", 1);
	//gmsgForceCam_cs16 = REG_USER_MSG("ForceCam", 3);
	//gmsgRadar_cs16 = REG_USER_MSG("Radar", 7);
}