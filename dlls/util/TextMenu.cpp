#include "extdll.h"
#include "util.h"
#include "TextMenu.h"
#include "user_messages.h"
#include "CTriggerVote.h"
#include <cstdint>
#include "CBasePlayer.h"

class CTriggerVote;

TextMenu g_textMenus[MAX_PLAYERS];
int g_exitOptionNum = 5;

// listen for any other functions/plugins opening menus, so that TextMenu knows if it's the active menu
void TextMenuMessageBeginHook(int msg_dest, int msg_type, const float* pOrigin, edict_t* ed) {
	if (msg_type != gmsgShowMenu) {
		return;
	}

	for (int i = 0; i < MAX_PLAYERS; i++) {
		g_textMenus[i].handleMenuMessage(msg_dest, ed);
	}
}

// handle player selections
bool TextMenuClientCommandHook(CBasePlayer* pPlayer) {
	if (toLowerCase(CMD_ARGV(0)) == "menuselect") {
		int selection = atoi(CMD_ARGV(1)) - 1;
		if (selection < 0 || selection >= MAX_MENU_OPTIONS) {
			return true;
		}

		for (int i = 0; i < MAX_PLAYERS; i++) {
			g_textMenus[i].handleMenuselectCmd(pPlayer, selection);
		}

		return true;
	}

	return false;
}

TextMenu* TextMenu::init(CBasePlayer* player, TextMenuCallback callback) {
	int idx = player ? player->entindex() % MAX_PLAYERS : 0;
	TextMenu* menu = &g_textMenus[idx];
	menu->initAnon(callback);
	return menu;
}

TextMenu* TextMenu::init(CBasePlayer* player, EntityTextMenuCallback callback, CBaseEntity* ent) {
	int idx = player ? player->entindex() % MAX_PLAYERS : 0;
	TextMenu* menu = &g_textMenus[idx];
	menu->initEnt(callback, ent);
	return menu;
}

TextMenu::TextMenu() {
	initAnon(NULL);
}

void TextMenu::initCommon() {
	viewers = 0;
	numOptions = 0;
	m_strings.clear();

	for (int i = 0; i < MAX_MENU_OPTIONS - 1; i++) {
		options[i].displayText.clear();
		options[i].data.clear();
	}
	TextMenuItem_internal exitItem;
	exitItem.displayText = m_strings.alloc("Exit");
	exitItem.data = m_strings.alloc("exit");
	options[MAX_MENU_OPTIONS - 1] = exitItem;
	backText = m_strings.alloc("Back");
	moreText = m_strings.alloc("More");
	extraText = m_strings.alloc("");
	noexit = false;
}

void TextMenu::initAnon(TextMenuCallback newCallback) {
	initCommon();
	this->anonCallback = newCallback;
	this->entCallback = NULL;
	h_ent = NULL;
}

void TextMenu::initEnt(EntityTextMenuCallback newCallback, CBaseEntity* ent) {
	initCommon();
	this->entCallback = newCallback;
	this->anonCallback = NULL;
	h_ent = ent;
}

void TextMenu::handleMenuMessage(int msg_dest, edict_t* ed) {

	// Another text menu has been opened for one or more players, so this menu
	// is no longer visible and should not handle menuselect commands

	// If this message is in fact triggered by this object, then the viewer flags should be set
	// after this func finishes.

	if (!viewers) {
		return;
	}

	if ((msg_dest == MSG_ONE || msg_dest == MSG_ONE_UNRELIABLE) && ed) {
		//ALERT(at_console, "New menu opened for %s", STRING(ed->v.netname));
		viewers &= ~(PLRBIT(ed));
	}
	else if (msg_dest == MSG_ALL || msg_dest == MSG_ALL) {
		//ALERT(at_console, "New menu opened for all players");
		viewers = 0;
	}
	else {
		//ALERT(at_console, "Unhandled text menu message dest: %d", msg_dest);
	}
}

void TextMenu::handleMenuselectCmd(CBasePlayer* pPlayer, int selection) {
	if (!viewers || !pPlayer) {
		return;
	}

	int playerbit = PLRBIT(pPlayer->edict());

	if (viewers & playerbit) {
		if (!noexit && selection == g_exitOptionNum-1) {
			// exit menu
			viewers &= ~playerbit;
		}
		else if (isPaginated() && selection == BackOptionIdx()) {
			Open(lastDuration, lastPage - 1, pPlayer);
		}
		else if (isPaginated() && selection == MoreOptionIdx()) {
			Open(lastDuration, lastPage + 1, pPlayer);
		}
		else if (selection < numOptions && IsValidPlayer(pPlayer->edict())) {
			viewers &= ~playerbit;
			
			if (anonCallback) {
				TextMenuItem callbackItem = options[lastPage * ItemsPerPage() + selection].publicItem();
				anonCallback(this, pPlayer, selection, callbackItem);
			}

			if (entCallback) {
				if (h_ent) {
					TextMenuItem callbackItem = options[lastPage * ItemsPerPage() + selection].publicItem();
					(((CTriggerVote*)h_ent.GetEntity())->*entCallback)(this, pPlayer, selection, callbackItem);
				}
			}
		}
	}
	else {
		//ALERT(at_console, "%s is not viewing the '%s' menu", STRING(pEntity->v.netname), title.c_str());
	}
}

bool TextMenu::isPaginated() {
	return numOptions > MaxItemsNoPages();
}

void TextMenu::SetTitle(const char* newTitle) {
	this->title = m_strings.alloc(newTitle);
}

void TextMenu::SetPaginationText(const char* backText, const char* moreText) {
	this->backText = m_strings.alloc(backText);
	this->moreText = m_strings.alloc(moreText);
}

void TextMenu::SetExtraText(const char* extraText) {
	this->extraText = m_strings.alloc(extraText);
}

void TextMenu::RemoveExit() {
	noexit = true;
}

int TextMenu::ItemsPerPage() {
	return noexit ? MAX_PAGE_OPTIONS - 2 : MAX_PAGE_OPTIONS - 3;
}

int TextMenu::BackOptionIdx() {
	return ItemsPerPage();
}

int TextMenu::MoreOptionIdx() {
	return ItemsPerPage() + 1;
}

int TextMenu::MaxItemsNoPages() {
	return ItemsPerPage() + 2;
}

void TextMenu::AddItem(const char* displayText, const char* optionData) {
	if (numOptions >= MAX_MENU_OPTIONS) {
		ALERT(at_console, "Maximum menu options reached! Failed to add: %s\n", optionData);
		return;
	}

	TextMenuItem_internal item;
	item.displayText = m_strings.alloc(displayText);
	item.data = m_strings.alloc(optionData);
	options[numOptions] = item;

	numOptions++;
}

void TextMenu::Open(uint8_t duration, uint8_t page, CBasePlayer* player) {
	uint16_t validSlots = 0;
	
	if (player && !IsValidPlayer(player->edict())) {
		return; // can happen when using EHANDLE and the player leaves the game
	}

	if (!noexit)
		validSlots = (1 << (g_exitOptionNum - 1)); // exit option always valid

	lastPage = page;
	lastDuration = duration;
	
	int maxItemsWithoutPagination = MAX_PAGE_OPTIONS - (noexit ? 0 : 1);
	int limitPerPage = isPaginated() ? ItemsPerPage() : maxItemsWithoutPagination;
	int itemOffset = page * ItemsPerPage();
	int totalPages = (numOptions+(ItemsPerPage() -1)) / ItemsPerPage();

	std::string pageSuffix = "";
	if (isPaginated()) {
		pageSuffix = UTIL_VarArgs(" (%d/%d)", page + 1, totalPages);
	}

	std::string menuText = std::string("\\y") + title.str() + pageSuffix + "\n\n";

	int addedOptions = 0;
	for (int i = itemOffset, k = 0; i < itemOffset+limitPerPage && i < numOptions; i++, k++) {
		addedOptions++;

		if (!strcmp(options[i].displayText.str(), "") && !strcmp(options[i].data.str(), ""))
			continue; // don't display blank options (so plugins can control option numbers)

		validSlots |= (1 << k);
		menuText += "\\y" + std::to_string(k + 1) + ":\\w " + options[i].displayText.str() + "\n";
	}

	while (isPaginated() && addedOptions < ItemsPerPage()) {
		menuText += "\n";
		addedOptions++;
	}

	menuText += "\n";

	if (isPaginated()) {
		if (page > 0) {
			menuText += "\\y" + std::to_string(BackOptionIdx() + 1) + ":\\w " + backText.str() + "\n";
			validSlots |= (1 << (ItemsPerPage()));
		}
		else {
			menuText += "\n";
		}
		if (page < totalPages - 1) {
			menuText += "\\y" + std::to_string(MoreOptionIdx() + 1) + ":\\w " + moreText.str() + "\n";
			validSlots |= (1 << (ItemsPerPage() + 1));
		}
		else {
			menuText += "\n";
		}
	}

	if (!noexit)
		menuText += "\\y" + std::to_string(g_exitOptionNum % 10) + ":\\w Exit";

	menuText += extraText.str();

	if (menuText.size() > 187)
		menuText = menuText.substr(0, 187);

	if (player) {
		MESSAGE_BEGIN(MSG_ONE, gmsgShowMenu, NULL, player->edict());
		WRITE_SHORT(validSlots);
		WRITE_CHAR(duration);
		WRITE_BYTE(FALSE); // "need more" (?)
		WRITE_STRING(menuText.c_str());
		MESSAGE_END();

		viewers |= PLRBIT(player->edict());
	}
	else {
		ALERT(at_console, "WARNING: pagination is broken for menus that don't have a destination player\n");
		MESSAGE_BEGIN(MSG_ALL, gmsgShowMenu);
		WRITE_SHORT(validSlots);
		WRITE_CHAR(duration);
		WRITE_BYTE(FALSE); // "need more" (?)
		WRITE_STRING(menuText.c_str());
		MESSAGE_END();

		viewers = 0xffffffff;
	}
}

TextMenuItem TextMenu::TextMenuItem_internal::publicItem() {
	TextMenuItem ret;
	ret.displayText = displayText.str();
	ret.data = data.str();

	if (!ret.displayText) {
		ret.displayText = "";
	}
	if (!ret.data) {
		ret.data = "";
	}

	return ret;
}