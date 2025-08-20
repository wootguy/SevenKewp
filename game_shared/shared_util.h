/*
* util functions that both the serverand client need
*/
#pragma once
#include "Platform.h"
#include <string.h>
#include <string>
#include <vector>
#include <stdint.h>

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
EXPORT int UTIL_CurlRequest(std::string url, std::string& response_string);

// returns the HTTP response code, or -1 if the file failed to open for writing
EXPORT int UTIL_CurlDownload(std::string url, std::string fpath);

// get function address in a loaded library
EXPORT void* GetFunctionAddress(void* libHandle, const char* funcName);

EXPORT const char* UTIL_SevenKewpClientString(int version, bool includeModName = true);

EXPORT uint64_t getEpochMillis();

EXPORT double TimeDifference(uint64_t start, uint64_t end);