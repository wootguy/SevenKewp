#include "extdll.h"
#include "util.h"
#include "debug.h"
#include "user_messages.h"
#include <queue>
#include <fstream>

msg_info g_lastMsg;
char g_msgStrPool[512];
int g_nextStrOffset = 0;
std::string g_debugLogName;
std::string g_errorLogName;
std::ofstream g_debugFile;
std::ofstream g_errorFile;
std::queue<msg_hist_item> g_messageHistory;
float g_messageHistoryCutoff = 1.0f; // don't keep more than X seconds of history

void add_msg_part(int mtype, int iValue) {
	if (!mp_debugmsg.value) {
		return;
	}
	int idx = g_lastMsg.numMsgParts++ % 512;
	msg_part& part = g_lastMsg.parts[idx];
	part.type = mtype;
	part.iValue = iValue;
}
void add_msg_part(int mtype, float fValue) {
	if (!mp_debugmsg.value) {
		return;
	}
	int idx = g_lastMsg.numMsgParts++ % 512;
	msg_part& part = g_lastMsg.parts[idx];
	part.type = mtype;
	part.iValue = fValue;
}
void add_msg_part(const char* sValue) {
	if (!mp_debugmsg.value) {
		return;
	}
	int idx = g_lastMsg.numMsgParts++ % 512;
	msg_part& part = g_lastMsg.parts[idx];
	part.type = MFUNC_STRING;

	int strLen = sValue ? strlen(sValue) + 1 : 1;
	if (g_nextStrOffset + strLen < 512) {
		if (sValue) {
			memcpy(g_msgStrPool + g_nextStrOffset, sValue, strLen);
			part.sValue = g_msgStrPool + g_nextStrOffset;
			g_nextStrOffset += strLen;
		}
		else {
			part.sValue = g_msgStrPool + g_nextStrOffset;
			g_msgStrPool[g_nextStrOffset++] = 0;
		}
	}
	else {
		ALERT(at_logged, "ERROR: WRITE_STRING exceeded 512 bytes of message\n");
		part.sValue = 0;
	}
}

std::string getLogTimeStr() {
	time_t rawtime;
	struct tm* timeinfo;
	static char buffer[256];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "L %d/%m/%Y - %H:%M:%S: ", timeinfo);

	return buffer;
}

void writeDebugLog(std::ofstream& outFile, std::string lastLogName, std::string prefix, std::string line) {
	std::string curLogName;

	{
		time_t rawtime;
		struct tm* timeinfo;
		static char buffer[256];

		time(&rawtime);
		timeinfo = localtime(&rawtime);

		strftime(buffer, sizeof(buffer), "%Y-%m-%d.log", timeinfo);
		std::string fname = "logs_dbg/" + prefix + "_" + std::string(buffer);

		curLogName = fname;
	}

	if (curLogName != lastLogName) {
		lastLogName = curLogName;
		outFile.close();
		outFile.open(lastLogName, std::ios_base::app);
	}

	outFile << getLogTimeStr() << line << "\n";
}

const char* msgDestStr(int msg_dest) {
	const char* sdst = "";
	switch (msg_dest) {
	case MSG_BROADCAST:
		sdst = "MSG_BROADCAST";
		break;
	case MSG_ONE:
		sdst = "MSG_ONE";
		break;
	case MSG_ALL:
		sdst = "MSG_ALL";
		break;
	case MSG_INIT:
		sdst = "MSG_INIT";
		break;
	case MSG_PVS:
		sdst = "MSG_PVS";
		break;
	case MSG_PAS:
		sdst = "MSG_PAS";
		break;
	case MSG_PVS_R:
		sdst = "MSG_PVS_R";
		break;
	case MSG_PAS_R:
		sdst = "MSG_PAS_R";
		break;
	case MSG_ONE_UNRELIABLE:
		sdst = "MSG_ONE_UNRELIABLE";
		break;
	case MSG_SPEC:
		sdst = "MSG_SPEC";
		break;
	default:
		sdst = UTIL_VarArgs("%d (unkown)", msg_dest);
		break;
	}

	return sdst;
}

const char* msgTypeStr(int msg_type) {
	const char* sdst = "";

	switch (msg_type) {
	case SVC_BAD: sdst = "SVC_BAD"; break;
	case SVC_NOP: sdst = "SVC_NOP"; break;
	case SVC_DISCONNECT: sdst = "SVC_DISCONNECT"; break;
	case SVC_EVENT: sdst = "SVC_EVENT"; break;
	case SVC_VERSION: sdst = "SVC_VERSION"; break;
	case SVC_SETVIEW: sdst = "SVC_SETVIEW"; break;
	case SVC_SOUND: sdst = "SVC_SOUND"; break;
	case SVC_TIME: sdst = "SVC_TIME"; break;
	case SVC_PRINT: sdst = "SVC_PRINT"; break;
	case SVC_STUFFTEXT: sdst = "SVC_STUFFTEXT"; break;
	case SVC_SETANGLE: sdst = "SVC_SETANGLE"; break;
	case SVC_SERVERINFO: sdst = "SVC_SERVERINFO"; break;
	case SVC_LIGHTSTYLE: sdst = "SVC_LIGHTSTYLE"; break;
	case SVC_UPDATEUSERINFO: sdst = "SVC_UPDATEUSERINFO"; break;
	case SVC_DELTADESCRIPTION: sdst = "SVC_DELTADESCRIPTION"; break;
	case SVC_CLIENTDATA: sdst = "SVC_CLIENTDATA"; break;
	case SVC_STOPSOUND: sdst = "SVC_STOPSOUND"; break;
	case SVC_PINGS: sdst = "SVC_PINGS"; break;
	case SVC_PARTICLE: sdst = "SVC_PARTICLE"; break;
	case SVC_DAMAGE: sdst = "SVC_DAMAGE"; break;
	case SVC_SPAWNSTATIC: sdst = "SVC_SPAWNSTATIC"; break;
	case SVC_EVENT_RELIABLE: sdst = "SVC_EVENT_RELIABLE"; break;
	case SVC_SPAWNBASELINE: sdst = "SVC_SPAWNBASELINE"; break;
	case SVC_TEMPENTITY: sdst = "SVC_TEMPENTITY"; break;
	case SVC_SETPAUSE: sdst = "SVC_SETPAUSE"; break;
	case SVC_SIGNONNUM: sdst = "SVC_SIGNONNUM"; break;
	case SVC_CENTERPRINT: sdst = "SVC_CENTERPRINT"; break;
	case SVC_KILLEDMONSTER: sdst = "SVC_KILLEDMONSTER"; break;
	case SVC_FOUNDSECRET: sdst = "SVC_FOUNDSECRET"; break;
	case SVC_SPAWNSTATICSOUND: sdst = "SVC_SPAWNSTATICSOUND"; break;
	case SVC_INTERMISSION: sdst = "SVC_INTERMISSION"; break;
	case SVC_FINALE: sdst = "SVC_FINALE"; break;
	case SVC_CDTRACK: sdst = "SVC_CDTRACK"; break;
	case SVC_RESTORE: sdst = "SVC_RESTORE"; break;
	case SVC_CUTSCENE: sdst = "SVC_CUTSCENE"; break;
	case SVC_WEAPONANIM: sdst = "SVC_WEAPONANIM"; break;
	case SVC_DECALNAME: sdst = "SVC_DECALNAME"; break;
	case SVC_ROOMTYPE: sdst = "SVC_ROOMTYPE"; break;
	case SVC_ADDANGLE: sdst = "SVC_ADDANGLE"; break;
	case SVC_NEWUSERMSG: sdst = "SVC_NEWUSERMSG"; break;
	case SVC_PACKETENTITIES: sdst = "SVC_PACKETENTITIES"; break;
	case SVC_DELTAPACKETENTITIES: sdst = "SVC_DELTAPACKETENTITIES"; break;
	case SVC_CHOKE: sdst = "SVC_CHOKE"; break;
	case SVC_RESOURCELIST: sdst = "SVC_RESOURCELIST"; break;
	case SVC_NEWMOVEVARS: sdst = "SVC_NEWMOVEVARS"; break;
	case SVC_RESOURCEREQUEST: sdst = "SVC_RESOURCEREQUEST"; break;
	case SVC_CUSTOMIZATION: sdst = "SVC_CUSTOMIZATION"; break;
	case SVC_CROSSHAIRANGLE: sdst = "SVC_CROSSHAIRANGLE"; break;
	case SVC_SOUNDFADE: sdst = "SVC_SOUNDFADE"; break;
	case SVC_FILETXFERFAILED: sdst = "SVC_FILETXFERFAILED"; break;
	case SVC_HLTV: sdst = "SVC_HLTV"; break;
	case SVC_DIRECTOR: sdst = "SVC_DIRECTOR"; break;
	case SVC_VOICEINIT: sdst = "SVC_VOICEINIT"; break;
	case SVC_VOICEDATA: sdst = "SVC_VOICEDATA"; break;
	case SVC_SENDEXTRAINFO: sdst = "SVC_SENDEXTRAINFO"; break;
	case SVC_TIMESCALE: sdst = "SVC_TIMESCALE"; break;
	case SVC_RESOURCELOCATION: sdst = "SVC_RESOURCELOCATION"; break;
	case SVC_SENDCVARVALUE: sdst = "SVC_SENDCVARVALUE"; break;
	case SVC_SENDCVARVALUE2: sdst = "SVC_SENDCVARVALUE2"; break;
	default:
		if (msg_type == giPrecacheGrunt) sdst = "giPrecacheGrunt";
		else if (msg_type == giPrecacheGrunt) sdst = "giPrecacheGrunt";
		else if (msg_type == gmsgShake) sdst = "gmsgShake";
		else if (msg_type == gmsgFade) sdst = "gmsgFade";
		else if (msg_type == gmsgSelAmmo) sdst = "gmsgSelAmmo";
		else if (msg_type == gmsgFlashlight) sdst = "gmsgFlashlight";
		else if (msg_type == gmsgFlashBattery) sdst = "gmsgFlashBattery";
		else if (msg_type == gmsgResetHUD) sdst = "gmsgResetHUD";
		else if (msg_type == gmsgInitHUD) sdst = "gmsgInitHUD";
		else if (msg_type == gmsgShowGameTitle) sdst = "gmsgShowGameTitle";
		else if (msg_type == gmsgCurWeapon) sdst = "gmsgCurWeapon";
		else if (msg_type == gmsgHealth) sdst = "gmsgHealth";
		else if (msg_type == gmsgDamage) sdst = "gmsgDamage";
		else if (msg_type == gmsgBattery) sdst = "gmsgBattery";
		else if (msg_type == gmsgTrain) sdst = "gmsgTrain";
		else if (msg_type == gmsgLogo) sdst = "gmsgLogo";
		else if (msg_type == gmsgWeaponList) sdst = "gmsgWeaponList";
		else if (msg_type == gmsgAmmoX) sdst = "gmsgAmmoX";
		else if (msg_type == gmsgHudText) sdst = "gmsgHudText";
		else if (msg_type == gmsgDeathMsg) sdst = "gmsgDeathMsg";
		else if (msg_type == gmsgScoreInfo) sdst = "gmsgScoreInfo";
		else if (msg_type == gmsgTeamInfo) sdst = "gmsgTeamInfo";
		else if (msg_type == gmsgTeamScore) sdst = "gmsgTeamScore";
		else if (msg_type == gmsgGameMode) sdst = "gmsgGameMode";
		else if (msg_type == gmsgMOTD) sdst = "gmsgMOTD";
		else if (msg_type == gmsgServerName) sdst = "gmsgServerName";
		else if (msg_type == gmsgAmmoPickup) sdst = "gmsgAmmoPickup";
		else if (msg_type == gmsgWeapPickup) sdst = "gmsgWeapPickup";
		else if (msg_type == gmsgItemPickup) sdst = "gmsgItemPickup";
		else if (msg_type == gmsgHideWeapon) sdst = "gmsgHideWeapon";
		else if (msg_type == gmsgSetCurWeap) sdst = "gmsgSetCurWeap";
		else if (msg_type == gmsgSayText) sdst = "gmsgSayText";
		else if (msg_type == gmsgTextMsg) sdst = "gmsgTextMsg";
		else if (msg_type == gmsgSetFOV) sdst = "gmsgSetFOV";
		else if (msg_type == gmsgShowMenu) sdst = "gmsgShowMenu";
		else if (msg_type == gmsgGeigerRange) sdst = "gmsgGeigerRange";
		else if (msg_type == gmsgTeamNames) sdst = "gmsgTeamNames";
		else if (msg_type == gmsgStatusText) sdst = "gmsgStatusText";
		else if (msg_type == gmsgStatusValue) sdst = "gmsgStatusValue";
		else if (msg_type == gmsgToxicCloud) sdst = "gmsgToxicCloud";
		else sdst = UTIL_VarArgs("%d", msg_type);
		break;
	}

	return sdst;
}

void log_msg(msg_info& msg) {
	if (!mp_debugmsg.value) {
		return;
	}

	std::string originStr = msg.hasOrigin ? UTIL_VarArgs("(%X %X %X)",
		*(int*)&msg.pOrigin[0], *(int*)&msg.pOrigin[1], *(int*)&msg.pOrigin[2]) : "NULL";
	std::string entStr = msg.name[0] != 0 ? std::string(msg.name) : "NULL";
	std::string argStr = "";
	for (int i = 0; i < msg.numMsgParts; i++) {
		switch (msg.parts[i].type) {
		case MFUNC_BYTE:
			argStr += UTIL_VarArgs(" B-%X", msg.parts[i].iValue);
			break;
		case MFUNC_CHAR:
			argStr += UTIL_VarArgs(" C-%X", msg.parts[i].iValue);
			break;
		case MFUNC_SHORT:
			argStr += UTIL_VarArgs(" S-%X", msg.parts[i].iValue);
			break;
		case MFUNC_LONG:
			argStr += UTIL_VarArgs(" L-%X", msg.parts[i].iValue);
			break;
		case MFUNC_ANGLE:
			argStr += UTIL_VarArgs(" A-%X", *(int*)&msg.parts[i].fValue);
			break;
		case MFUNC_COORD:
			argStr += UTIL_VarArgs(" F-%X", *(int*)&msg.parts[i].fValue);
			break;
		case MFUNC_STRING:
			if (msg.parts[i].sValue >= g_msgStrPool && msg.parts[i].sValue < g_msgStrPool + 512) {
				argStr += UTIL_VarArgs(" \"%s\"", msg.parts[i].sValue);
			}
			else {
				argStr += UTIL_VarArgs(" \"\"");
			}
			break;
		case MFUNC_ENTITY:
			argStr += UTIL_VarArgs(" E-%X", msg.parts[i].iValue);
			break;
		default:
			break;
		}
	}

	float now = g_engfuncs.pfnTime();

	std::string log = UTIL_VarArgs("T%.2f MSG(%s, %s, %s, %s)%s",
		now, msgDestStr(msg.msg_dest), msgTypeStr(msg.msg_type),
		originStr.c_str(), entStr.c_str(), argStr.c_str());

	// forget old messages
	while (!g_messageHistory.empty() && now - g_messageHistory.front().time > g_messageHistoryCutoff) {
		g_messageHistory.pop();
	}

	msg_hist_item item;
	item.time = now;
	item.msg = log;
	g_messageHistory.push(item);
}

void writeNetworkMessageHistory(std::string reason) {
	float now = g_engfuncs.pfnTime();
	while (!g_messageHistory.empty() && now - g_messageHistory.front().time > g_messageHistoryCutoff) {
		g_messageHistory.pop();
	}

	while (!g_messageHistory.empty()) {
		msg_hist_item item = g_messageHistory.front();
		g_messageHistory.pop();
		writeDebugLog(g_debugFile, g_debugLogName, "debug", item.msg);
	}
	writeDebugLog(g_debugFile, g_debugLogName, "debug", UTIL_VarArgs("T%.2f ", g_engfuncs.pfnTime()) + reason + "\n");
	g_debugFile.flush();
}

void clearNetworkMessageHistory() {
	while (!g_messageHistory.empty()) {
		g_messageHistory.pop();
	}
}

#ifdef DEBUG
EXPORT edict_t* DBG_EntOfVars(const entvars_t* pev)
{
	if (pev->pContainingEntity != NULL)
		return pev->pContainingEntity;
	ALERT(at_console, "entvars_t pContainingEntity is NULL, calling into engine");
	edict_t* pent = (*g_engfuncs.pfnFindEntityByVars)((entvars_t*)pev);
	if (pent == NULL)
		ALERT(at_console, "DAMN!  Even the engine couldn't FindEntityByVars!");
	((entvars_t*)pev)->pContainingEntity = pent;
	return pent;
}

void
DBG_AssertFunction(
	BOOL		fExpr,
	const char* szExpr,
	const char* szFile,
	int			szLine,
	const char* szMessage)
{
	if (fExpr)
		return;
	char szOut[512];
	if (szMessage != NULL)
		snprintf(szOut, 512, "ASSERT FAILED:\n %s \n(%s@%d)\n%s", szExpr, szFile, szLine, szMessage);
	else
		snprintf(szOut, 512, "ASSERT FAILED:\n %s \n(%s@%d)", szExpr, szFile, szLine);
	ALERT(at_console, szOut);
}
#endif	// DEBUG

bool ModelIsValid(entvars_t* edict, studiohdr_t* header) {
	// basic corruption detection
	if (header->id != 1414743113 || header->version != 10) {
		ALERT(at_error, "Model corruption! Model: %s, ID: %d, Version: %d\n",
			STRING(edict->model), header->version, header->id);
		return false;
	}

	return true;
}

const char* cstr(string_t s) {
	return STRING(s);
}