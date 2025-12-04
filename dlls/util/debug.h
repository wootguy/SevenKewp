#pragma once
#include <string>

//
// How did I ever live without ASSERT?
//
#ifdef	DEBUG
void DBG_AssertFunction(BOOL fExpr, const char* szExpr, const char* szFile, int szLine, const char* szMessage);
#define ASSERT(f)		DBG_AssertFunction(f, #f, __FILE__, __LINE__, NULL)
#define ASSERTSZ(f, sz)	DBG_AssertFunction(f, #f, __FILE__, __LINE__, sz)
#else	// !DEBUG
#define ASSERT(f)
#define ASSERTSZ(f, sz)
#endif	// !DEBUG

enum msg_func_types {
	MFUNC_BYTE,
	MFUNC_CHAR,
	MFUNC_SHORT,
	MFUNC_LONG,
	MFUNC_ANGLE,
	MFUNC_COORD,
	MFUNC_STRING,
	MFUNC_ENTITY,
};

struct msg_part {
	uint16_t type;

	union {
		int iValue;
		float fValue;
	};
	const char* sValue;
};

struct msg_info {
	int msg_dest;
	int msg_type;
	bool hasOrigin;
	float pOrigin[3];
	int entIdx;
	char name[32];
	int sz; // always valid, even ithout mp_debugmsg

	int numMsgParts;
	msg_part parts[512];
};

struct msg_hist_item {
	float time;
	std::string msg;
};

extern msg_info g_lastMsg; // updated when mp_debugmsg is enabled
extern int g_nextStrOffset;
extern std::string g_debugLogName;
extern std::string g_errorLogName;
extern std::ofstream g_debugFile;
extern std::ofstream g_errorFile;

void add_msg_part(int mtype, int iValue);
void add_msg_part(int mtype, float fValue);
void add_msg_part(const char* sValue);

std::string getLogTimeStr();

void writeDebugLog(std::ofstream& outFile, std::string lastLogName, std::string prefix, std::string line);

EXPORT void writeDebugLog(std::string prefix, std::string line);

EXPORT const char* msgDestStr(int msg_dest);

EXPORT const char* msgTypeStr(int msg_type);

void log_msg(msg_info& msg);

// write the most recent X seconds of message history for debugging client disconnects 
// due to malformed network messages.
// reason = reason for writing the message history
EXPORT void writeNetworkMessageHistory(std::string reason);
EXPORT void clearNetworkMessageHistory();

EXPORT int LastMsgSize();

// for debugging
bool ModelIsValid(entvars_t* edict, studiohdr_t* header);