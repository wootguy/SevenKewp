#include "shared_util.h"
#include "md5.h"

#ifdef CLIENT_DLL
#include "../cl_dll/hud.h"
#include "../cl_dll/cl_util.h"
#else
#include "extdll.h"
#include "util.h"
#endif

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

#define NUM_AUTO_UPDATE_FILES 1
const char* g_autoUpdateFiles[NUM_AUTO_UPDATE_FILES] = {
	"sprites/hlcoop/hud.txt"
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