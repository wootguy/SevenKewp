enum player_status {
	PLAYER_STATUS_NONE,		// playing the game
	PLAYER_STATUS_LOAD,		// not fully loaded into the game yet
	PLAYER_STATUS_CHAT,		// typing in chat
	PLAYER_STATUS_CONSOLE,	// console or menus opened
	PLAYER_STATUS_TABBED,	// another window has focus
};

#define UNPACK_PLAYER_STATUS(val) (val & 0xF) 