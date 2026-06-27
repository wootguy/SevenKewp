/*
* util functions that both the serverand client need
*/
#pragma once
#include "Platform.h"
#include <string.h>
#include <string>
#include <vector>
#include <stdint.h>
#include "vector.h"
#include "rgb.h"
#include "../game_shared/shared_effects.h"
#include "const.h"
#include "studio.h"
#include "HashMap.h"
#include "progdefs.h"

#undef min
#undef max

// In case this ever changes
#define M_PI			3.14159265358979323846

#define ITEM_FLAG_SELECTONEMPTY		1
#define ITEM_FLAG_NOAUTORELOAD		2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY	4
#define ITEM_FLAG_LIMITINWORLD		8
#define ITEM_FLAG_EXHAUSTIBLE		16 // A player can totally exhaust their ammo supply and lose this weapon
#define ITEM_FLAG_NOAUTOSWITCHTO	32

#define WEP_FLAG_DYNAMIC_ACCURACY	1	// weapon accuracy changes when moving/crouching
#define WEP_FLAG_SECONDARY_ACCURACY	2	// weapon has a separate accuracy value for secondary fire
#define WEP_FLAG_VERTICAL_ACCURACY	4	// weapon has a separate accuracy value(s) for the vertical axis
#define WEP_FLAG_USE_ZOOM_CROSSHAIR	8	// use the sprite-based crosshair while zoomed
#define WEP_FLAG_NO_ZOOM_CROSSHAIR	16	// remove the crosshair while zoomed (for iron sights)

// Makes these more explicit, and easier to find
#define FILE_GLOBAL static
#define DLL_GLOBAL

// In case it's not alread defined
typedef int BOOL;

typedef enum { ignore_monsters = 1, dont_ignore_monsters = 0, missile = 2 } IGNORE_MONSTERS;

#define ARRAY_SZ(array) (int)(sizeof(array) / sizeof(array[0]))

enum distant_sound_types {
	DISTANT_NONE,
	DISTANT_9MM, // light tapping noise
	DISTANT_357, // deeper tap
	DISTANT_556, // deep tap / small explosion
	DISTANT_BOOM // big explosion
};

EXPORT extern const Vector g_vecZero;
EXPORT extern globalvars_t* gpGlobals;

EXPORT extern const std::vector<std::string> g_emptyCurlHeaders;
EXPORT extern const std::string g_emptyCurlPostData;
EXPORT extern StringSet g_default_weapon_hud_icon_names; // ammo, weapon, weapon_s, ...

// same as strncpy except it ensures the destination is null terminated, even if the buffer is too small
EXPORT char* strcpy_safe(char* dest, const char* src, size_t size);

// same as strncat except it ensures the destination is null terminated, even if the buffer is too small
EXPORT char* strcat_safe(char* dest, const char* src, size_t size);

// same as sprintf except it ensures the destination is null terminated, even if the buffer is too small
EXPORT int sprintf_safe(char* dst, int len_dst, const char* format, ...);

// hash client data files for auto-updates in bulk
const char* UTIL_HashClientDataFiles(bool& overrideDetected);

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
// returns NULL if file couldn't be loaded
EXPORT uint8_t* UTIL_LoadFile(const char* fpath, int* size = NULL);

// load file relative to the Half-Life/ folder
EXPORT uint8_t* UTIL_LoadFileRoot(const char* fpath, int* size = NULL);

// loads muzzle flash details from file on the first call, then returns cached results
EXPORT custom_muzzle_flash_t loadCustomMuzzleFlash(const char* path);

EXPORT void UnloadCustomMuzzleFlashes();

EXPORT int UTIL_PointContents(const Vector& vec);

EXPORT bool UTIL_PointInLiquid(const Vector& vec);

EXPORT bool UTIL_PointInSplashable(const Vector& vec);

EXPORT bool UTIL_IsLiquidContents(int contents);

// find the water intersection point, if one exists
EXPORT bool UTIL_WaterTrace(Vector from, Vector to, Vector& isect);

EXPORT float clampf(float val, float min, float max);

EXPORT int clampi(int val, int min, int max);

EXPORT char* UTIL_VarArgs(const char* format, ...);

EXPORT void UTIL_WaterSplashParams(float scale, int playSound, float& ratio, float& sz, float& fps, float& vol, int& pitch, const char*& sample);

EXPORT void te_debug_box(Vector mins, Vector maxs, float life=50, RGBA c = RGBA(255, 0, 0, 255), int msgType = MSG_BROADCAST, edict_t* dest = NULL);

EXPORT void te_debug_beam(Vector start, Vector end, float life=50, RGBA c=RGBA(255,0,0,255), int msgType = MSG_BROADCAST, edict_t* dest = NULL);

// create a splash if trace intersects water
// playSound: 1 = slow moving object, 2 = bullet, 3 = obnoxious player
// skipEnt: don't send message to this player (for predicted splashes - server only)
EXPORT void	UTIL_WaterSplashTrace(Vector from, Vector to, float scale, int playSound, edict_t* skipEnt);

// parse a 3 digit hex color string into its components ("80F" -> purple)
// return false if string is an invalid length
EXPORT bool UTIL_ParseHexColor(const char* hex, int& r, int& g, int& b);

// compress to a 16 bit "float" that can display all uint values
// but with suffixes to hide precision (20k instead of 20321)
uint16_t UTIL_CompressUint(uint32_t v);
uint32_t UTIL_DecompressUint(uint16_t v);

// linear interpolation
EXPORT float UTIL_Lerp(float start, float end, float t);

// convert degrees of accuracy to an accuracy cone used with FireBullets
EXPORT Vector UTIL_ConeFromDegrees(float degrees);

// convert degrees of accuracy to an accuracy cone used with FireBullets
EXPORT Vector UTIL_ConeFromDegrees(float degreesX, float degreesY);

// open a file using a path relative to the game dir (valve/)
EXPORT FILE* UTIL_OpenFile(const char* path, const char* mode);

// get set bit position in mask
EXPORT int BitIndex(uint32_t mask);

EXPORT float GetSequenceDuration(studiohdr_t* pstudiohdr, int iseq); // animation time in seconds

EXPORT bool UTIL_ModelIsSprite(int modelidx);

EXPORT int UTIL_GetRenderFxOpacity(int renderfx, int renderamt, float t);

EXPORT float UTIL_WeaponTimeBase(void);

// Convert floats to/from signed fixed-point integers.
// Use this when the number of bits needed in the fixed-point representation is less than a standard type.
// For example, if you want 24 bits instead of 32 (part of int32), or 12 instead of 16 (part of int16).
// Simpler conversion logic can be used otherwise.
// result will be sign-extended to fit an int32_t.
// values that don't fit in the specified number of bits will be clamped.
inline int32_t FLOAT_TO_FIXED(float x, int whole_bits, int frac_bits) {
	int32_t maxVal = ((1 << (whole_bits + frac_bits - 1)) - 1);
	int32_t minVal = -(maxVal + 1);
	int32_t r = clampf(x, minVal, maxVal) * (1 << frac_bits);
	return r;
}
// sign extend a fixed point value that was stored in an unsigned integer, which may have 0 in its higher bits
inline int32_t SIGN_EXTEND_FIXED(uint32_t x, int total_bits) {
	uint32_t b = total_bits;
	int m = 1U << (b - 1); // sign bit mask
	x = x & ((1U << b) - 1); // remove sign bit
	return (x ^ m) - m; // sign extended version of x
}
// x should not be sign extended to fit the int32_t it was stored in. That will be done in this function
inline float FIXED_TO_FLOAT(int x, int whole_bits, int frac_bits) {
	// https://graphics.stanford.edu/~seander/bithacks.html#VariableSignExtend
	uint32_t b = whole_bits + frac_bits;
	int m = 1U << (b - 1); // sign bit mask

	x = x & ((1U << b) - 1); // remove sign bit
	int r = (x ^ m) - m; // sign extended version of x

	return r / (float)(1 << frac_bits);
}

// compares two fixed point integers for equality, where each may or may not be sign extended
inline bool FIXED_EQUALS(int x, int y, int totalBits) {
	int m = (1 << totalBits) - 1; // mask of value without extended sign
	return (x & m) == (y & m);
}