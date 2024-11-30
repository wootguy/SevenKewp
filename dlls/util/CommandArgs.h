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
	EXPORT std::string ArgV(int idx) const;

	// return number of args
	EXPORT int ArgC() const;

	// return entire command string
	EXPORT std::string getFullCommand() const;

private:
	std::vector<std::string> args;
};
