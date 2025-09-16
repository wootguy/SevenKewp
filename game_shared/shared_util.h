/*
* util functions that both the serverand client need
*/
#pragma once
#include "Platform.h"
#include <string.h>
#include <string>
#include <vector>
#include <stdint.h>

// In case this ever changes
#define M_PI			3.14159265358979323846

#define WEP_FLAG_DYNAMIC_ACCURACY	1 // weapon accuracy changes when moving/crouching
#define WEP_FLAG_SECONDARY_ACCURACY	2 // weapon has a separate accuracy value for secondary fire
#define WEP_FLAG_VERTICAL_ACCURACY	4 // weapon has a separate accuracy value(s) for the vertical axis
#define WEP_FLAG_USE_ZOOM_CROSSHAIR	8 // use the sprite-based crosshair while zoomed

EXPORT extern const std::vector<std::string> g_emptyCurlHeaders;
EXPORT extern const std::string g_emptyCurlPostData;

// same as strncpy except it ensures the destination is null terminated, even if the buffer is too small
EXPORT char* strcpy_safe(char* dest, const char* src, size_t size);

// same as strncat except it ensures the destination is null terminated, even if the buffer is too small
EXPORT char* strcat_safe(char* dest, const char* src, size_t size);

// hash client data files for auto-updates in bulk
const char* UTIL_HashClientDataFiles();

// called by the client only to delete sevenkewp files to prepare for an update
void UTIL_DeleteClientDataFiles();

EXPORT float normalizeRangef(const float value, const float start, const float end);

EXPORT std::vector<std::string> splitString(std::string str, const char* delimiters);

EXPORT std::string toLowerCase(std::string str);

EXPORT std::string toUpperCase(std::string str);

EXPORT std::string trimSpaces(std::string s);

EXPORT std::string replaceString(std::string subject, std::string search, std::string replace);

EXPORT bool dirExists(const std::string& path);

EXPORT bool createDir(const std::string& path);

EXPORT std::string getAbsolutePath(const std::string& relpath);

// escapes a string for use with curl resquests (e.g. query parameter with spaces in it)
EXPORT std::string UTIL_UrlEncode(const std::string& decoded);

// returns the HTTP response code:
//     100-199 = information
//     200-299 = success
//     300-399 = redirect
//     400-499 = client error
//     500-599 = server error
// response_string = response data from the server
// postFields = data to send for POST request
// headers = HTTP headers
EXPORT int UTIL_CurlRequest(std::string url, std::string& response_string,
	const std::string& postData = g_emptyCurlPostData,
	const std::vector<std::string>& headers = g_emptyCurlHeaders);

// returns the HTTP response code, or -1 if the file failed to open for writing
EXPORT int UTIL_CurlDownload(std::string url, std::string fpath);

// get function address in a loaded library
EXPORT void* GetFunctionAddress(void* libHandle, const char* funcName);

// 0.0.0 or "SevenKewp 0.0.0" if including the mod name
EXPORT const char* UTIL_SevenKewpClientString(int version, bool includeModName = true);

// versions are compatible if their major.minor versions are the same.
// patch versions can differ. Incompatible clients have most features disabled.
EXPORT bool UTIL_AreSevenKewpVersionsCompatible(int clientVersion, int serverVersion);

EXPORT uint64_t getEpochMillis();

EXPORT double TimeDifference(uint64_t start, uint64_t end);

// returns 0 if the file doesn't exist
EXPORT uint64_t getFileModifiedTime(const char* path);

EXPORT std::string normalize_path(std::string s);

// searches game directories in order (e.g. valve/path, valve_downloads/path)
// returns an empty string if the file can't be found
EXPORT std::string getGameFilePath(const char* path, bool matchCase = true);

// relative to a content directory. Will search all suffixed directores (valve, valve_addon, valve_downloads)
EXPORT uint8_t* UTIL_LoadFile(const char* fpath, int* size = NULL);

// load file relative to the Half-Life/ folder
EXPORT uint8_t* UTIL_LoadFileRoot(const char* fpath, int* size = NULL);