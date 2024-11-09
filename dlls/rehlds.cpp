#include "rehlds.h"
#include "util.h"

IRehldsApi* g_RehldsApi;
const RehldsFuncs_t* g_RehldsFuncs;
IRehldsServerData* g_RehldsData;
IRehldsHookchains* g_RehldsHookchains;
IRehldsServerStatic* g_RehldsSvs;

bool RehldsApi_Init()
{
#ifdef WIN32
	// Find the most appropriate module handle from a list of DLL candidates
	// Notes:
	// - "swds.dll" is the library Dedicated Server
	//
	//    Let's also attempt to locate the ReHLDS API in the client's library
	// - "sw.dll" is the client library for Software render, with a built-in listenserver
	// - "hw.dll" is the client library for Hardware render, with a built-in listenserver
	const char* dllNames[] = { "swds.dll", "sw.dll", "hw.dll" }; // List of DLL candidates to lookup for the ReHLDS API
	CSysModule* engineModule = NULL; // The module handle of the selected DLL
	for (const char* dllName : dllNames)
	{
		if (engineModule = Sys_LoadModule(dllName))
			break; // gotcha
	}

#else
	CSysModule* engineModule = Sys_LoadModule("engine_i486.so");
#endif

	if (!engineModule)
		return false;

	CreateInterfaceFn ifaceFactory = Sys_GetFactory(engineModule);
	if (!ifaceFactory)
		return false;

	int retCode = 0;
	g_RehldsApi = (IRehldsApi*)ifaceFactory(VREHLDS_HLDS_API_VERSION, &retCode);

	if (!g_RehldsApi)
		return false;

	int majorVersion = g_RehldsApi->GetMajorVersion();
	int minorVersion = g_RehldsApi->GetMinorVersion();

	if (majorVersion != REHLDS_API_VERSION_MAJOR)
	{
		ALERT(at_error, "ReHLDS API major version mismatch; expected %d, real %d\n", REHLDS_API_VERSION_MAJOR, majorVersion);

		// need to notify that it is necessary to update the ReHLDS.
		if (majorVersion < REHLDS_API_VERSION_MAJOR)
		{
			ALERT(at_error, "Please update the ReHLDS up to a major version API >= %d\n", REHLDS_API_VERSION_MAJOR);
		}

		// need to notify that it is necessary to update the module.
		else if (majorVersion > REHLDS_API_VERSION_MAJOR)
		{
			ALERT(at_error, "Please update the mod up to a major version API >= %d\n", majorVersion);
		}

		return false;
	}

	if (minorVersion < REHLDS_API_VERSION_MINOR)
	{
		ALERT(at_error, "ReHLDS API minor version mismatch; expected at least %d, real %d\n", REHLDS_API_VERSION_MINOR, minorVersion);
		ALERT(at_error, "Please update the ReHLDS up to a minor version API >= %d\n", REHLDS_API_VERSION_MINOR);
		return false;
	}

	g_RehldsFuncs = g_RehldsApi->GetFuncs();
	g_RehldsData = g_RehldsApi->GetServerData();
	g_RehldsHookchains = g_RehldsApi->GetHookchains();
	g_RehldsSvs = g_RehldsApi->GetServerStatic();

	g_engfuncs.pfnServerPrint(UTIL_VarArgs("ReHLDS API version %d.%d initialized\n",
		REHLDS_API_VERSION_MAJOR, REHLDS_API_VERSION_MINOR));

	return true;
}
