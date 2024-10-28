#pragma once
#include <extdll.h>
#include <string>
#include <vector>
#include <stdint.h>

struct CommandArgs {
	bool isConsoleCmd;
	
	// gets current globally defined args
	EXPORT CommandArgs();

	EXPORT void loadArgs();

	// returns empty string if idx is out of bounds
	EXPORT std::string ArgV(int idx);

	// return number of args
	EXPORT int ArgC();

	// return entire command string
	EXPORT std::string getFullCommand();

private:
	std::vector<std::string> args;
};
