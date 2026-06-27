#include "extdll.h"
#include "util.h"
#include "shake.h"
#include "user_messages.h"
#include "PluginManager.h"
#include "wc_params.h"

UserMessageIds g_umsg;

std::vector<UserMessage> g_userMessages;

int REG_USER_MSG(const char* name, int size) {
	CALL_HOOKS(int, pfnRegUserMsg, name, size);

	if (strlen(name) > 11) {
		ALERT(at_error, "User message name %s is too long (11 chars max)\n", name);
		return 0;
	}

	UserMessage msg;
	msg.name = name;
	msg.size = size;

	msg.id = g_engfuncs.pfnRegUserMsg(name, size);

	g_userMessages.push_back(msg);

	return msg.id;
}

UserMessage* GetUserMsg(int id) {
	for (int i = 0; i < (int)g_userMessages.size(); i++) {
		UserMessage& msg = g_userMessages[i];

		if (msg.id == id) {
			return &msg;
		}
	}

	return NULL;
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
	UserMessage* msg = GetUserMsg(id);

	if (msg) {
		if (size)
			*size = msg->size;
		return msg->id;
	}

	return 0;
}

void LinkUserMessages(void)
{
	// Already taken care of?
	if (g_umsg.SelAmmo)
	{
		return;
	}

	INIT_USER_MSG(SelAmmo, sizeof(SelAmmo));
	INIT_USER_MSG(CurWeapon, 3);
	INIT_USER_MSG(CurWeaponX, 4);
	INIT_USER_MSG(Geiger, 1);
	INIT_USER_MSG(Flashlight, 2);
	INIT_USER_MSG(FlashBat, 1);
	INIT_USER_MSG(Health, 1);
	INIT_USER_MSG(Damage, 12);
	INIT_USER_MSG(Battery, 2);
	INIT_USER_MSG(Train, 1);
	//INIT_USER_MSG(HudTextPro, -1);
	INIT_USER_MSG(HudText, -1);
	INIT_USER_MSG(SayText, -1);
	INIT_USER_MSG(TextMsg, -1);
	INIT_USER_MSG(WeaponList, -1);
	INIT_USER_MSG(WeaponListX, -1);
	INIT_USER_MSG(ResetHUD, 1);
	INIT_USER_MSG(InitHUD, 0);
	INIT_USER_MSG(GameTitle, 1);
	INIT_USER_MSG(DeathMsg, -1);
	INIT_USER_MSG(ScoreInfo, 9);
	INIT_USER_MSG(TeamInfo, -1);
	INIT_USER_MSG(TeamScore, -1);
	INIT_USER_MSG(GameMode, 1);
	INIT_USER_MSG(MOTD, -1);
	INIT_USER_MSG(ServerName, -1);
	INIT_USER_MSG(AmmoPickup, 2);
	INIT_USER_MSG(WeapPickup, 1);
	INIT_USER_MSG(ItemPickup, -1);
	INIT_USER_MSG(HideWeapon, 1);
	INIT_USER_MSG(SetFOV, 1);
	INIT_USER_MSG(ShowMenu, -1);
	INIT_USER_MSG(ScreenShake, sizeof(ScreenShake));
	INIT_USER_MSG(ScreenFade, sizeof(ScreenFade));
	INIT_USER_MSG(AmmoX, 2);
	INIT_USER_MSG(AmmoXX, 3);
	INIT_USER_MSG(TeamNames, -1);

	INIT_USER_MSG(StatusText, -1);
	INIT_USER_MSG(StatusValue, 3);

	INIT_USER_MSG(CustomWep, -1);
	INIT_USER_MSG(CustomWepAk, -1);
	INIT_USER_MSG(CustomWepEv, -1);
	INIT_USER_MSG(NextMap, -1);
	INIT_USER_MSG(TimeLeft, 4);
	INIT_USER_MSG(Fog, 8);
	INIT_USER_MSG(ToxicCloud, 4);
	INIT_USER_MSG(PmodelAnim, 2);
	INIT_USER_MSG(WeaponBits, 8);
	INIT_USER_MSG(TagInfo, 8);
	INIT_USER_MSG(PlayerPos, -1);
	INIT_USER_MSG(SpriteAdv, -1);
	INIT_USER_MSG(WaterSplash, -1);
	INIT_USER_MSG(PredFiles, -1);
	INIT_USER_MSG(PredCvars, 2);
	INIT_USER_MSG(PredMove, -1);
	INIT_USER_MSG(MatsPath, -1);
	INIT_USER_MSG(CustomHud, -1);
	INIT_USER_MSG(HudColor, 3);
	INIT_USER_MSG(HudConPrint, -1);
	INIT_USER_MSG(StringIdx, -1);
	INIT_USER_MSG(HudSprite, -1);
	INIT_USER_MSG(HudSprTogl, 1);
	INIT_USER_MSG(HudUpdNum, 5);
	INIT_USER_MSG(Tracer2, 13);
	INIT_USER_MSG(BloodSpr2, 12);
	INIT_USER_MSG(TempFx, -1);
}