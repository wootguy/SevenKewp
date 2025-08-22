#include <string>
#include <fstream>
#include <thread>
#include <atomic>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include "miniz.h"

#include "shared_util.h"
#include "hud.h"
#include "cl_util.h"
#include "net_api.h"

#define EXT_FUNC
#include "interface.h"

#define UPDATE_URL "https://api.github.com/repos/wootguy/SevenKewp/releases/latest"
#define MANUAL_UPDATE_URL "https://github.com/wootguy/SevenKewp/releases/latest"
#define DOWNLOAD_FOLDER "sevenkewp_updater_temp" // relative to gamedir_addon
#define RESTART_DELAY 3.0f // time before restarting the game after an update

enum UPDATE_THREAD_STATUS {
	CUPDATE_STATUS_RUNNING = -2,
	CUPDATE_STATUS_ERROR = -1,
	CUPDATE_STATUS_NO_ACTION = 0, // thread finished. no action was taken/necessary
	CUPDATE_STATUS_SUCCESS = 1, // thread finished successfully
};

uint64_t g_lastUpdateTime;

std::string g_updateUrl;
std::string g_updateFileName;
std::string g_updateThreadError;
std::string g_update_dir;
std::string g_update_zip;
std::string g_update_tag_name;
uint32_t g_update_tag_vernum;

std::thread g_update_thread;
std::atomic<int> g_updateThreadStatus(CUPDATE_STATUS_RUNNING);

// function parameters for the init func
typedef void(*CLIENT_DLL_INIT_FUNCTION)(void*);

void UpdateFinish() {
	g_lastUpdateTime = getEpochMillis();
	gHUD.m_ClientUpdater.m_updateState = CUPDATE_NONE;
}

void AbortUpdate() {
	PRINTF("\n-------------------------\nUPDATE FAILED!\n-------------------------\n");
	PRINTF("Consider updating your client manually here:\n");
	PRINTF("%s\n\n", MANUAL_UPDATE_URL);
	gEngfuncs.pfnCenterPrint("Update failed!\nCheck your console for errors.");
	UpdateFinish();
}

// return true on success
bool ExtractZipFile(std::string zipPath, std::string& fileInZip, std::string& outFilePath) {
	// Open the ZIP archive
	mz_zip_archive zip_archive;
	memset(&zip_archive, 0, sizeof(zip_archive));

	if (!mz_zip_reader_init_file(&zip_archive, zipPath.c_str(), 0)) {
		PRINTF("ERROR: Failed to open zip file: %s\n", zipPath.c_str());
		return false;
	}

	// Locate the file index in the archive
	int file_index = mz_zip_reader_locate_file(&zip_archive, fileInZip.c_str(), nullptr, 0);
	if (file_index < 0) {
		PRINTF("ERROR: File not found in zip: %s\n", fileInZip.c_str());
		mz_zip_reader_end(&zip_archive);
		return false;
	}

	// Get the uncompressed size
	mz_zip_archive_file_stat file_stat;
	if (!mz_zip_reader_file_stat(&zip_archive, file_index, &file_stat)) {
		PRINTF("ERROR: Failed to get file info for: %s\n", fileInZip.c_str());
		mz_zip_reader_end(&zip_archive);
		return false;
	}

	// Extract the file to memory
	std::vector<unsigned char> buffer(file_stat.m_uncomp_size);
	if (!mz_zip_reader_extract_to_mem(&zip_archive, file_index, buffer.data(), buffer.size(), 0)) {
		PRINTF("ERROR: Failed to extract file: %s\n", fileInZip.c_str());
		mz_zip_reader_end(&zip_archive);
		return false;
	}

	// Write it to disk
	std::ofstream out(outFilePath, std::ios::binary);

	if (!out.is_open()) {
		PRINTF("ERROR: Failed to open file for writing: %s\n", outFilePath.c_str());
		return false;
	}

	out.write(reinterpret_cast<char*>(buffer.data()), buffer.size());
	out.close();

	mz_zip_reader_end(&zip_archive);
	return true;
}

bool TestLibrary(std::string libPath) {
	CSysModule* lib = Sys_LoadModule(libPath.c_str());

	if (!lib) {
		PRINTF("ERROR: Failed to load library: %s\n", libPath.c_str());
		return false;
	}

	// call the exported function that the engine uses to set up callbacks.
	// Yes, the function is simply called "F" (see cdll_int.cpp)
	CLIENT_DLL_INIT_FUNCTION apiFunc = (CLIENT_DLL_INIT_FUNCTION)Sys_GetProcAddress(lib, "F");

	if (!apiFunc) {
		PRINTF("ERROR: Invalid client library.\n");
		Sys_UnloadModule(lib);
		return false;
	}

	// call the loaded function
	static cldll_func_t veryRealData;
	apiFunc(&veryRealData);

	// no crash? Guess we're good to go.

	Sys_UnloadModule(lib);
	return true;
}

bool IsLanIP(uint8_t* ip) {
	if (ip[0] == 10)
		return true; // 10.0.0.0/8
	if (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31)
		return true; // 172.16.0.0/12
	if (ip[0] == 192 && ip[1] == 168)
		return true; // 192.168.0.0/16
	if (ip[0] == 127)
		return true; // loopback

	return false;
}

void RestartGame() {
	net_status_t netstatus;
	gEngfuncs.pNetAPI->Status(&netstatus);

	uint8_t* ip = netstatus.remote_address.ip;
	uint16_t port = netstatus.remote_address.port;
	port = (port >> 8) | (port << 8); // swap endian

	std::string restartCmd;

	if (netstatus.connected && !IsLanIP(ip)) {
		restartCmd = UTIL_VarArgs("steam://connect/%d.%d.%d.%d:%d",
			(int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3], (int)port);
	}
	else {
		restartCmd = UTIL_VarArgs("steam://rungameid/%d", gEngfuncs.pfnGetAppID());
	}

	EngineClientCmd("quit\n");

	// run restart command in a detached shell and with a delay to give the game time to close
#ifdef WIN32
	restartCmd = "cmd /C start /b cmd /C \"timeout /t 3 /nobreak && start " + restartCmd + "\"";
#else
	restartCmd = "sh -c \"sleep 3 && xdg-open " + restartCmd + "\" &";
#endif

	PRINTF("Restarting game with shell command:\n%s", restartCmd.c_str());
	system(restartCmd.c_str());
}

// sets thread status code: -1 = error, 0 = no update neeeded, 1 = update found
void CheckForUpdate() {
	std::string response_string;
	int code = UTIL_CurlRequest(UPDATE_URL, response_string);

	if (code != 200) {
		if (code <= 0) {
			g_updateThreadError = "ERROR: Curl responded with:\n" + response_string;
		}
		else {
			g_updateThreadError = "ERROR: Unexpected HTTP response code " + std::to_string(code) + "\n"
				+ "Response data from server:\n" + response_string;
		}
		
		g_updateThreadStatus = CUPDATE_STATUS_ERROR;
		return;
	}

	rapidjson::Document json;
	json.Parse(response_string.c_str());

	if (!json.IsObject()) {
		g_updateThreadError = "ERROR: Unexpected response(not a json object).\n";
		g_updateThreadStatus = CUPDATE_STATUS_ERROR;
		return;
	}
	if (!json.HasMember("tag_name")) {
		g_updateThreadError = "ERROR: Unexpected JSON response (missing tag_name).\n";
		g_updateThreadStatus = CUPDATE_STATUS_ERROR;
		return;
	}
	if (!json.HasMember("assets") || !json["assets"].IsArray()) {
		g_updateThreadError = "ERROR: Unexpected JSON response (missing assets).\n";
		g_updateThreadStatus = CUPDATE_STATUS_ERROR;
		return;
	}

	rapidjson::Value& resp = json["tag_name"];
	g_update_tag_name = resp.GetString();

	// strip non-numeric characters
	std::string tagnum;
	for (int i = 0; i < g_update_tag_name.size(); i++) {
		if (isdigit(g_update_tag_name[i]))
			tagnum += g_update_tag_name[i];
	}

	if (tagnum.empty()) {
		g_updateThreadError = "ERROR: tag_name has unexpected format: '" + g_update_tag_name + "'\n";
		g_updateThreadStatus = CUPDATE_STATUS_ERROR;
		return;
	}

	g_update_tag_vernum = atoi(tagnum.c_str());

	if (g_update_tag_vernum <= SEVENKEWP_VERSION) {
		g_updateThreadError = "";
		g_updateThreadStatus = CUPDATE_STATUS_NO_ACTION;
		return;
	}

	rapidjson::Value& assets = json["assets"];
	const std::string updateNamePrefix = "sevenkewp_client_";

	int bestAsset = -1;
	for (int i = 0; i < assets.Size(); i++) {
		rapidjson::Value& asset = assets[i];
		if (asset.IsObject() && asset.HasMember("name") && asset.HasMember("browser_download_url")) {
			std::string name = asset["name"].GetString();
			std::string lowerName = toLowerCase(name);
			if (lowerName.find(updateNamePrefix) == 0 && lowerName.find(".zip") == name.size()-4) {
				//PRINTF("Found asset: %s\n", name.c_str());
				if (bestAsset != -1) {
					g_updateThreadError = "Multiple files found with prefix '" + updateNamePrefix + "'. Don't know what to do.\n";
					g_updateThreadStatus = CUPDATE_STATUS_ERROR;
					return;
				}
				bestAsset = i;
			}
		}
	}

	if (bestAsset == -1) {
		g_updateThreadError = "ERROR: No release ZIPs found.\n";
		g_updateThreadStatus = CUPDATE_STATUS_ERROR;
		return;
	}

	rapidjson::Value& asset = assets[bestAsset];
	g_updateUrl = asset["browser_download_url"].GetString();
	g_updateFileName = asset["name"].GetString();

	// just in case, don't ever break out of the update folder
	g_updateFileName = replaceString(g_updateFileName, "/", "");
	g_updateFileName = replaceString(g_updateFileName, "\\", "");
	g_updateFileName = replaceString(g_updateFileName, "..", "");

	g_updateThreadError = "";
	g_updateThreadStatus = CUPDATE_STATUS_SUCCESS;
}

void DownloadUpdateFile() {
	if (!createDir(g_update_dir)) {
		g_updateThreadError = "ERROR: Failed to create update folder: " + g_update_dir + "\n";
		g_updateThreadStatus = CUPDATE_STATUS_ERROR;
		return;
	}

	g_update_zip = UTIL_VarArgs("%s/%s", g_update_dir.c_str(), g_updateFileName.c_str());

	int ret = UTIL_CurlDownload(g_updateUrl, g_update_zip.c_str());
	if (ret != 200) {
		g_updateThreadError = "ERROR: Failed to download file (Error code " + std::to_string(ret) + ").\n";
		g_updateThreadStatus = CUPDATE_STATUS_ERROR;
		return;
	}

	g_updateThreadError = "";
	g_updateThreadStatus = CUPDATE_STATUS_SUCCESS;
}

bool ApplyUpdate() {
#ifdef WIN32
	const char* libName = "client.dll";
#else
	const char* libName = "client.so";
#endif

	std::string zipLibPath = UTIL_VarArgs("cl_dlls/%s", libName);
	std::string tempLibPath = UTIL_VarArgs("%s/%s", g_update_dir.c_str(), libName);

	if (fileExists(tempLibPath.c_str())) {
		remove(tempLibPath.c_str());
	}

	if (!ExtractZipFile(g_update_zip, zipLibPath, tempLibPath)) {
		return false;
	}

	if (!TestLibrary(tempLibPath)) {
		remove(tempLibPath.c_str());
		return false;
	}

	// replace the library
	std::string addonDllDir = UTIL_VarArgs("%s_addon/cl_dlls", gEngfuncs.pfnGetGameDirectory());
	std::string currentLib = UTIL_VarArgs("%s/%s", addonDllDir.c_str(), libName);
	std::string backupLibName = UTIL_VarArgs("%s/%s.backup", addonDllDir.c_str(), libName);

	if (fileExists(backupLibName.c_str())) {
		remove(backupLibName.c_str());
	}

	rename(currentLib.c_str(), backupLibName.c_str());
	rename(tempLibPath.c_str(), currentLib.c_str());

	std::string libAbsPath = getAbsolutePath(currentLib);
	PRINTF("Updated file: %s\n", libAbsPath.c_str());

	return true;
}

void UpdateClientCommand() {
	if (gHUD.m_ClientUpdater.m_updateState != CUPDATE_NONE) {
		PRINTF("\nAn update is already in progress.\n");
		gEngfuncs.pfnCenterPrint("An update is already in progress.");
		return;
	}
	if (getEpochMillis() - g_lastUpdateTime < 3000) {
		PRINTF("Wait a few seconds before attempting to update again.\n");
		gEngfuncs.pfnCenterPrint("Wait a few seconds before attempting\nto update again.");
		return;
	}

	if ((int)CVAR_GET_FLOAT("hlcoop_version") == gHUD.m_sevenkewpVersion) {
		gEngfuncs.pfnCenterPrint("Your client is up-to-date");
		return;
	}

	PRINTF("\n-------------------------\nBeginning client update\n-------------------------\n");

	g_update_dir = UTIL_VarArgs("%s_addon/%s", gEngfuncs.pfnGetGameDirectory(), DOWNLOAD_FOLDER);

	PRINTF("Checking update url:\n%s\n", UPDATE_URL);
	gHUD.m_ClientUpdater.m_updateState = CUPDATE_CHECK;

	g_updateThreadStatus = CUPDATE_STATUS_RUNNING;
	g_update_thread = std::thread(CheckForUpdate);
	g_update_thread.detach();
}

int CHudClientUpdater::Init() {
	gHUD.AddHudElem(this);

	cvar_t* ver_cvar = CVAR_GET_PTR("hlcoop_version");

	m_iFlags |= HUD_INTERMISSION | HUD_ACTIVE;

	return 1;
}

void CHudClientUpdater::Think() {
	if (m_updateState == CUPDATE_NONE || g_updateThreadStatus == CUPDATE_STATUS_RUNNING)
		return;

	if (m_updateState == CUPDATE_CHECK) {
		PRINTF("\nLatest release version: %4d (%s)\n",
			g_update_tag_vernum, g_update_tag_name.c_str());
		PRINTF("Installed version:      %4d (v%s)\n",
			SEVENKEWP_VERSION, UTIL_SevenKewpClientString(SEVENKEWP_VERSION, false));

		if (g_updateThreadStatus != CUPDATE_STATUS_SUCCESS) {
			if (g_updateThreadStatus == CUPDATE_STATUS_ERROR) {
				PRINTF("%s", g_updateThreadError.c_str());
				AbortUpdate();
			}
			else {
				PRINTF("\nYour client is up-to-date.\n\n-------------------------\n");
				gEngfuncs.pfnCenterPrint("Your client is up-to-date");
				UpdateFinish();
			}
			return;
		}

		PRINTF("Using update archive:   %s\n", g_updateFileName.c_str());
		PRINTF("\nDownloading:\n%s\n", g_updateUrl.c_str());
		m_updateState = CUPDATE_DOWNLOAD;
		g_updateThreadStatus = CUPDATE_STATUS_RUNNING;
		g_update_thread = std::thread(DownloadUpdateFile);
		g_update_thread.detach();
	}
	else if (m_updateState == CUPDATE_DOWNLOAD) {
		if (g_updateThreadStatus != CUPDATE_STATUS_SUCCESS) {
			PRINTF("%s", g_updateThreadError.c_str());
			AbortUpdate();
			return;
		}

		PRINTF("\nDownload finished. Applying update.\n");
		if (ApplyUpdate()) {
			UpdateFinish();
			m_updateState = CUPDATE_FINISH;
			g_lastUpdateTime = getEpochMillis();

			PRINTF("\n-------------------------\nUpdate complete!\n-------------------------\n");
			PRINTF("Restarting game in %d seconds...\n", (int)RESTART_DELAY);
		}
		else {
			AbortUpdate();
		}
	}
	else if (m_updateState == CUPDATE_FINISH) {
		float timeLeft = RESTART_DELAY - TimeDifference(g_lastUpdateTime, getEpochMillis());
		gEngfuncs.pfnCenterPrint(UTIL_VarArgs("Update complete!\nRestarting game in %.1fs\n", timeLeft));

		if (timeLeft <= 0) {
			UpdateFinish();
			RestartGame();
		}
	}
}

int CHudClientUpdater::Draw(float flTime) {
	if (!ver_cvar) {
		ver_cvar = CVAR_GET_PTR("hlcoop_version");
		return 1;
	}

	int clientVerNum = ver_cvar->value;
	if (clientVerNum == gHUD.m_sevenkewpVersion) {
		return 1;
	}

	if (clientVerNum < 0) {
		return 1; // in case you don't want to update, you can set the cvar to -1 to hide the notice
	}

	const char* commandBtn = gEngfuncs.Key_LookupBinding("commandmenu");
	std::string commandBtnTip = commandBtn ? std::string(" with [") + commandBtn + "]" : "";
	const char* additionalTip = commandBtn ? "" : " (type or bind \"+commandmenu\" to open the menu)";
	const char* updateMessage = UTIL_VarArgs(
		"Open the command menu%s and choose \"Update Client\"%s",
		commandBtnTip.c_str(), additionalTip);

	if (m_updateState == CUPDATE_CHECK) {
		updateMessage = "Checking for update...\n";
	}
	else if (m_updateState == CUPDATE_DOWNLOAD) {
		updateMessage = "Downloading update...\n";
	}
	else if (m_updateState == CUPDATE_FINISH) {
		updateMessage = "Update finished!\n";
	}
	else if (clientVerNum > gHUD.m_sevenkewpVersion) {
		updateMessage = "This server needs an update for your client to work.";
	}
	

	int w, h;
	GetConsoleStringSize(updateMessage, &w, &h);

	int x = (ScreenWidth - w) / 2;
	int y = ScreenHeight - h;

	DrawConsoleString(x, y, updateMessage, RGB(0, 255, 255));

	// version comparison message
	std::string clientVer = UTIL_SevenKewpClientString(clientVerNum, false);
	std::string serverVer = UTIL_SevenKewpClientString(gHUD.m_sevenkewpVersion, false);
	const char* versionMessage = UTIL_VarArgs("Client v%s  |  Server v%s",
		clientVer.c_str(), serverVer.c_str());
	
	GetConsoleStringSize(versionMessage, &w, &h);
	x = (ScreenWidth - w) / 2;

	if (m_updateState == CUPDATE_NONE)
		DrawConsoleString(x, y - h, versionMessage, RGB(0, 255, 255));

	const char* warnMessage = "Your client is out-of-date!";
	if (clientVerNum > gHUD.m_sevenkewpVersion) {
		warnMessage = "Your client is too new!";
	}

	GetConsoleStringSize(warnMessage, &w, &h);
	x = (ScreenWidth - w) / 2;

	if (m_updateState == CUPDATE_NONE)
		DrawConsoleString(x, y - h*2, warnMessage, RGB(0, 255, 255));

	return 1;
}