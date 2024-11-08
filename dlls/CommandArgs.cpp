#include "CommandArgs.h"
#include "util.h"

CommandArgs::CommandArgs() {
	
}

void CommandArgs::loadArgs() {
	std::string firstArgLower = toLowerCase(CMD_ARGV(0));
	isConsoleCmd = firstArgLower != "say" && firstArgLower != "say_team";

	std::string argStr = CMD_ARGC() > 1 ? CMD_ARGS() : "";

	if (isConsoleCmd) {
		argStr = CMD_ARGV(0) + std::string(" ") + argStr;
	}

	if (!isConsoleCmd && argStr.length() > 2 && argStr[0] == '\"' && argStr[argStr.length() - 1] == '\"') {
		argStr = argStr.substr(1, argStr.length() - 2); // strip surrounding quotes
	}

	while (!argStr.empty()) {
		// strip spaces
		argStr = trimSpaces(argStr);


		if (argStr[0] == '\"') { // quoted argument (include the spaces between quotes)
			argStr = argStr.substr(1);
			int endQuote = argStr.find("\"");

			if (endQuote == -1) {
				args.push_back(argStr);
				break;
			}

			args.push_back(argStr.substr(0, endQuote));
			argStr = argStr.substr(endQuote + 1);
		}
		else {
			// normal argument, separate by space
			int nextSpace = argStr.find(" ");

			if (nextSpace == -1) {
				args.push_back(argStr);
				break;
			}

			args.push_back(argStr.substr(0, nextSpace));
			argStr = argStr.substr(nextSpace + 1);
		}
	}
}

std::string CommandArgs::ArgV(int idx) {
	if (idx >= 0 && idx < (int)args.size()) {
		return args[idx];
	}

	return "";
}

int CommandArgs::ArgC() {
	return args.size();
}

std::string CommandArgs::getFullCommand() {
	std::string str = ArgV(0);

	for (int i = 1; i < (int)args.size(); i++) {
		str += " " + args[i];
	}

	return str;
}
