#pragma once
#include "CBaseEntity.h"
#include <string>

#define MAX_MENU_OPTIONS 128
#define MAX_PAGE_OPTIONS 5
#define ITEMS_PER_PAGE (MAX_PAGE_OPTIONS-3) // leave room for next, more, and exit options
#define BACK_OPTION_IDX (ITEMS_PER_PAGE)
#define MORE_OPTION_IDX (ITEMS_PER_PAGE+1)
#define MAX_ITEMS_NO_PAGES (ITEMS_PER_PAGE+2)
#define MAX_PLAYERS 32

class CTriggerVote;
class CBasePlayer;

// this must be called as part of a MessageBegin hook for text menus to know when they are active
void TextMenuMessageBeginHook(int msg_dest, int msg_type, const float* pOrigin, edict_t* ed);

// this must be called as part of a DLL ClientCommand hook for option selections to work
bool TextMenuClientCommandHook(CBasePlayer* pEntity);

struct TextMenuItem {
	std::string displayText;
	std::string data;
};

typedef void (CTriggerVote::*EntityTextMenuCallback)(class TextMenu* menu, CBasePlayer* player, int itemNumber, TextMenuItem& item);
typedef void (*TextMenuCallback)(class TextMenu* menu, CBasePlayer* player, int itemNumber, TextMenuItem& item);

// Do not create new TextMenus. Only use initMenuForPlayer
class TextMenu {
public:
	TextMenu();

	// use this to create menus for each player.
	// When creating a menu for all players, pass NULL for player.
	EXPORT static TextMenu* init(CBasePlayer* player, TextMenuCallback callback);

	EXPORT static TextMenu* init(CBasePlayer* player, EntityTextMenuCallback callback, CBaseEntity* ent);

	EXPORT void SetTitle(std::string title);

	EXPORT void AddItem(std::string displayText, std::string optionData);

	// set player to NULL to send to all players.
	// This should be the same target as was used with initMenuForPlayer
	// paging not supported yet
	EXPORT void Open(uint8_t duration, uint8_t page, CBasePlayer* player);

	// don't call directly. This is triggered by global hook functions
	EXPORT void handleMenuMessage(int msg_dest, edict_t* ed);

	// don't call directly. This is triggered by global hook functions
	EXPORT void handleMenuselectCmd(CBasePlayer* pEntity, int selection);

private:
	void initAnon(TextMenuCallback callback);
	void initEnt(EntityTextMenuCallback callback, CBaseEntity* ent);
	void initCommon();

private:
	EntityTextMenuCallback entCallback = NULL;
	TextMenuCallback anonCallback = NULL;
	EHANDLE h_ent; // entity which started the vote
	float openTime = 0; // time when the menu was opened
	uint32_t viewers; // bitfield indicating who can see the menu
	std::string title;
	TextMenuItem options[MAX_MENU_OPTIONS];
	int numOptions = 0;
	int8_t lastPage = 0;
	int8_t lastDuration = 0;

	bool isActive = false;

	bool isPaginated();
};


extern TextMenu g_textMenus[MAX_PLAYERS];