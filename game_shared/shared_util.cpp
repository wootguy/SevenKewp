#include "shared_util.h"
#include "md5.h"
#include <string>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <chrono>

#ifdef ENABLE_CURL
#include <curl/curl.h>
#endif

#ifdef CLIENT_DLL
#include "../cl_dll/hud.h"
#include "../cl_dll/cl_util.h"
#define PRINTERR(fmt, ...) PRINTF(fmt, ##__VA_ARGS__) 
#else
#include "extdll.h"
#include "util.h"
#define PRINTERR(fmt, ...) ALERT(at_error, fmt, ##__VA_ARGS__) 
#endif

#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#include <direct.h>
#include <sys/stat.h>
#define GetCurrentDir _getcwd
#ifdef WIN32
#include <io.h>
#define F_OK 0
#define access _access
#endif
#else
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>
#define GetCurrentDir getcwd
#define GetProcAddress dlsym
#define HMODULE void*
#endif

using namespace std::chrono;

char* strcpy_safe(char* dest, const char* src, size_t size) {
	if (size > 0) {
		size_t i;
		for (i = 0; i < size - 1 && src[i]; i++) {
			dest[i] = src[i];
		}
		dest[i] = '\0';
	}
	return dest;
}

char* strcat_safe(char* dest, const char* src, size_t size) {
	size_t dstLen = strlen(dest);
	char* writeDst = dest + dstLen;
	
	if (dstLen > size) {
		return dest;
	}
	size -= dstLen;

	if (size > 0) {
		size_t i;
		for (i = 0; i < size - 1 && src[i]; i++) {
			writeDst[i] = src[i];
		}
		writeDst[i] = '\0';
	}
	return dest;
}

uint8_t* LoadFile(const char* fpath, int* len) {
#ifdef CLIENT_DLL
	return gEngfuncs.COM_LoadFile(fpath, 5, len);
#else
	return LOAD_FILE_FOR_ME(fpath, len);
#endif
}

void FreeFile(uint8_t* buffer) {
#ifdef CLIENT_DLL
	gEngfuncs.COM_FreeFile(buffer);
#else
	FREE_FILE(buffer);
#endif
}

uint8_t* UTIL_AppendFileData(const char* fpath, uint8_t* existingData, int& len) {
	int appendLen;
	uint8_t* appendData = LoadFile(fpath, &appendLen);
	if (!appendData) {
		//ALERT(at_console, "Hash file failed to load '%s'\n", fpath);
		return existingData;
	}

	uint8_t* newData = new uint8_t[len + appendLen];

	if (existingData) {
		memcpy(newData, existingData, len);
		delete[] existingData;
	}

	memcpy(newData + len, appendData, appendLen);

	FreeFile(appendData);

	len += appendLen;
	return newData;
}

#define NUM_AUTO_UPDATE_FILES 2
const char* g_autoUpdateFiles[NUM_AUTO_UPDATE_FILES] = {
	"sprites/hlcoop/hud.txt",
	"commandmenu_sk.txt",
};

const char* UTIL_HashClientDataFiles() {
	// compute the hash result for client files which should be reacquired from the server if changed.
	// The client must duplicate this hashing logic.
	int len = 0;
	uint8_t* totalFileData = NULL;

	for (int i = 0; i < NUM_AUTO_UPDATE_FILES; i++) {
		totalFileData = UTIL_AppendFileData(g_autoUpdateFiles[i], totalFileData, len);
	}
	
	uint8_t digest[16];
	MD5Context mdc;
	MD5Init(&mdc);
	MD5Update(&mdc, totalFileData, len);
	MD5Final(digest, &mdc);

	return UTIL_VarArgs("%08X", *(uint32_t*)digest);
}

void UTIL_DeleteClientDataFiles() {
#ifndef CLIENT_DLL
	return; // don't delete the update files!
#else

	static char downloadsDir[256];
	const char* gamedir = gEngfuncs.pfnGetGameDirectory();
	strcpy_safe(downloadsDir, UTIL_VarArgs("%s_downloads/", gamedir), 256);

	for (int i = 0; i < NUM_AUTO_UPDATE_FILES; i++) {
		const char* path = FindGameFile(g_autoUpdateFiles[i]);
		
		if (!path)
			continue;
		if (strstr(path, downloadsDir) != path) {
			// only delete files downloaded from servers.
			// The client may be overriding files in valve_addon intentionally
			PRINTF("File causing mismatch: %s\n", path);
			continue;
		}
		
		remove(path);
		PRINTF("Deleted file: %s\n", path);
	}
#endif
}

// https://stackoverflow.com/questions/1628386/normalise-orientation-between-0-and-360
float normalizeRangef(const float value, const float start, const float end)
{
	const float width = end - start;
	const float offsetValue = value - start;   // value relative to 0

	return (offsetValue - (floorf(offsetValue / width) * width)) + start;
	// + start to reset back to start of original range
}

std::vector<std::string> splitString(std::string str, const char* delimiters)
{
	std::vector<std::string> split;

	const char* c_str = str.c_str();
	size_t str_len = strlen(c_str);

	size_t start = strspn(c_str, delimiters);

	while (start < str_len)
	{
		size_t end = strcspn(c_str + start, delimiters);
		split.push_back(str.substr(start, end));
		start += end + strspn(c_str + start + end, delimiters);
	}

	return split;
}

std::string toLowerCase(std::string str) {
	std::string out = str;

	for (int i = 0; str[i]; i++) {
		out[i] = tolower(str[i]);
	}

	return out;
}

std::string toUpperCase(std::string str) {
	std::string out = str;

	for (int i = 0; str[i]; i++) {
		out[i] = toupper(str[i]);
	}

	return out;
}

std::string trimSpaces(std::string s) {
	size_t start = s.find_first_not_of(" \t\n\r");
	size_t end = s.find_last_not_of(" \t\n\r");
	return (start == std::string::npos) ? "" : s.substr(start, end - start + 1);
}

std::string replaceString(std::string subject, std::string search, std::string replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}

std::string UTIL_UrlEncode(const std::string& decoded)
{
#ifdef ENABLE_CURL
	const auto encoded_value = curl_easy_escape(nullptr, decoded.c_str(), static_cast<int>(decoded.length()));
	std::string result(encoded_value);
	curl_free(encoded_value);
	return result;
#else
	PRINTERR("UTIL_UrlEncode requires ENABLE_CURL build flag in cmake\n");
	return "";
#endif
}

bool dirExists(const std::string& path)
{
	struct stat info;

	if (stat(path.c_str(), &info) != 0)
		return false;
	else if (info.st_mode & S_IFDIR)
		return true;
	return false;
}

bool createDir(const std::string& path) {
	if (dirExists(path))
		return true;

	int nError = 0;
#if defined(_WIN32)
	nError = _mkdir(path.c_str());
#else 
	nError = mkdir(path.c_str(), 0733);
#endif
	if (nError != 0) {
		printf("Failed to create directory (error %d)\n", nError);
	}
	return nError == 0;
}

bool isAbsolutePath(const std::string& path) {
	if (path.length() > 1 && path[1] == ':') {
		return true;
	}
	if (path.length() > 1 && (path[0] == '\\' || path[0] == '/' || path[0] == '~')) {
		return true;
	}
	return false;
}

std::string getAbsolutePath(const std::string& relpath) {
	if (isAbsolutePath(relpath)) {
		return relpath;
	}

	char buffer[256];
	if (GetCurrentDir(buffer, sizeof(buffer)) != NULL) {
		std::string abspath = std::string(buffer) + "/" + relpath;

#if defined(WIN32)	
		replace(abspath.begin(), abspath.end(), '/', '\\'); // convert to windows slashes
#endif

		return abspath;
	}
	else {
		return "Error: Unable to get current working directory";
	}

	return relpath;
}

size_t curlWriteString(void* ptr, size_t size, size_t nmemb, void* data) {
	if (!ptr || !data) {
		return 0;
	}
	((std::string*)data)->append((char*)ptr, size * nmemb);
	return size * nmemb;
}

size_t curlWriteFile(void* ptr, size_t size, size_t nmemb, void* stream) {
	return fwrite(ptr, size, nmemb, (FILE*)stream);
}

#ifdef WIN32
#define USER_AGENT_SYSTEM "(Windows)"
#else
#define USER_AGENT_SYSTEM "(Linux)"
#endif

#ifdef CLIENT_DLL
#define USER_AGENT_APP "SevenKewp Client"
#else
#define USER_AGENT_APP "SevenKewp Server"
#endif

int UTIL_CurlRequest_internal(std::string url, void* writeData, bool isString) {
#ifdef ENABLE_CURL
	auto curl = curl_easy_init();

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 50L);
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

		std::string verString = UTIL_SevenKewpClientString(SEVENKEWP_VERSION);
		std::string agent = UTIL_VarArgs("%s %s %s", USER_AGENT_APP, verString.c_str(), USER_AGENT_SYSTEM);

		curl_easy_setopt(curl, CURLOPT_USERAGENT, agent.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, isString ? curlWriteString : curlWriteFile);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, writeData);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L); // follow redirects

		// disables HTTPS verifications. No certs needed. Big security hole.
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		CURLcode res = curl_easy_perform(curl);

		if (res != CURLE_OK && isString) {
			*((std::string*)writeData) = std::string("CURL Error: ") + curl_easy_strerror(res);
		}

		long response_code;
		double elapsed;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
		curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &elapsed);

		curl_easy_cleanup(curl);
		curl = NULL;

		return response_code;
	}
#else
	PRINTERR("UTIL_Curl* requires ENABLE_CURL build flag in cmake\n");
#endif
	return 0;
}

int UTIL_CurlRequest(std::string url, std::string& response_string) {
	response_string = "";
	return UTIL_CurlRequest_internal(url, &response_string, true);
}

int UTIL_CurlDownload(std::string url, std::string fpath) {
	FILE* fp = fopen(fpath.c_str(), "wb");
	
	if (!fp)
		return -1;

	int ret = UTIL_CurlRequest_internal(url, fp, false);

	fclose(fp);

	return ret;
}

void* GetFunctionAddress(void* libHandle, const char* funcName) {
	return (void*)GetProcAddress((HMODULE)libHandle, funcName);
}

const char* UTIL_SevenKewpClientString(int version, bool includeModName) {
	int patch = version % 10000;			// minor bugfix / optional update
	int minor = (version / 10000) % 10000;	// required update
	int major = version / (10000*10000);	// 0 = pre-release, 1 = post-release
	return UTIL_VarArgs("%s%d.%d.%d", includeModName ? "SevenKewp " : "", major, minor, patch);
}

bool UTIL_AreSevenKewpVersionsCompatible(int clientVersion, int serverVersion) {
	return (clientVersion / 10000) == (serverVersion / 10000) && clientVersion > 0 && serverVersion > 0;
}

uint64_t getEpochMillis() {
	return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

double TimeDifference(uint64_t start, uint64_t end) {
	if (end > start) {
		return (end - start) / 1000.0;
	}
	else {
		return -((start - end) / 1000.0);
	}
}

uint64_t getFileModifiedTime(const char* path) {
	struct stat result;
	if (stat(path, &result) == 0) {
		return result.st_mtime;
	}

	return 0;
}