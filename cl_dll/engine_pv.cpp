#include "engine_pv.h"
#include "hud.h"
#include "cl_util.h"

#ifdef WIN32
#include "windows.h"
#endif

char dummyData[16];
EnginePv g_enginepv;
extern bool is_steam_legacy_engine;
extern bool is_software_renderer;

char** g_sound_precache; // list of precached sounds (for mapping an index to a file path)

uintptr_t GetModuleBase(const char* moduleName) {
	static char fullModuleName[MAX_PATH];

#ifdef WIN32
	safe_sprintf(fullModuleName, MAX_PATH, "%s.dll", moduleName);
	return (uintptr_t)GetModuleHandleA(fullModuleName);
#else
	safe_sprintf(fullModuleName, MAX_PATH, "%s.so", moduleName);
	FILE* fp = fopen("/proc/self/maps", "r");
	if (!fp) return 0;

	char line[512];
	long unsigned int base = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (strstr(line, fullModuleName)) {
			sscanf(line, "%lx-", &base);
			break;
		}
	}
	fclose(fp);
	return base;
#endif
}

void InitEnginePv() {
	g_enginepv.r_wpoly = (int32_t*)dummyData;
	g_enginepv.r_epoly = (int32_t*)dummyData;
	g_sound_precache = NULL;

	uintptr_t engineModuleBase = GetModuleBase(is_software_renderer ? "sw" : "hw");
	
	if (!engineModuleBase)
		return;

	// offsets found by checking arguments passed to the Con_Printf call for r_speeds 1.
	// Look for what uses this string in a decompiler to get the wpoly/epoly globals:
	//		"%3ifps %3i ms  %4i wpoly %4i epoly\n"  (hw.*)
	//		"%5.1f ms %3i/%3i/%3i poly %3i surf\n"  (sw.*)
	// 
	// Look for this to see where the sound precache list is (PlaySoundByIndex API):
	//      "invalid sound %i"
	// The final function call indexes into an array of char*
	// 
	// Remember to subtract the default module base address from the .data addresses you find (0x1000000)
	// 
	// TODO: this code might violate the terms of the Steam subscriber agreement. Maybe there's
	// a way to hook Con_Printf instead... or would that be even more blatant reverse engineering?

#ifdef WIN32
	if (is_software_renderer) {
		g_enginepv.r_wpoly = (int32_t*)(engineModuleBase + 0x6FD330);
		g_sound_precache = (char**)(engineModuleBase + 0xD26ED4);
	}
	else {
		g_enginepv.r_wpoly = (int32_t*)(engineModuleBase + 0xDC55B0);
		g_enginepv.r_epoly = (int32_t*)(engineModuleBase + 0xDC55AC);
		g_sound_precache = (char**)(engineModuleBase + 0x13F6C14);
	}
#else
	if (is_software_renderer) {
		g_enginepv.r_wpoly = (int32_t*)(engineModuleBase + 0xBE7908);
		g_sound_precache = (char**)(engineModuleBase + 0xBA0950);
	}
	else {
		g_enginepv.r_wpoly = (int32_t*)(engineModuleBase + 0xF7D804);
		g_enginepv.r_epoly = (int32_t*)(engineModuleBase + 0xF7D7F4);
		g_sound_precache = (char**)(engineModuleBase + 0xDCE730);
	}
#endif
}

// hard to believe there's no engine API for this. Without it you would need to transfer
// the entire sound list over again so the client can play by index. Adding string arguments
// to the network messages that need custom sound playback would be even worse.
const char* GetSoundByIndex(int idx) {
	InitEnginePv();
	
	if (!g_sound_precache) {
		PRINTF("EnginePv sound list not initialized!\n");
		return "";
	}

	if (idx < 0 || idx >= 512) {
		PRINTF("Invalid sound index %d\n", idx);
		return "";
	}

	return g_sound_precache[idx];
}