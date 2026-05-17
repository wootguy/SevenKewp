#pragma once

#ifdef _WIN32
#include "windows.h"
#define LOADLIB_FUNC_NAME ""
#define PLUGIN_EXT ".dll"
#define LibError() (std::string("error code ") + std::to_string(GetLastError()))
#else
#include <dlfcn.h>
#define PLUGIN_EXT ".so"
#define LOADLIB_FUNC_NAME "dlopen"
#define GetProcAddress dlsym
#define GetLastError dlerror
#define FreeLibrary !dlclose
#define HMODULE void *
#define LibError() std::string(dlerror())
#endif

// get handle to the server DLL
extern "C" DLLEXPORT HMODULE GetServerModule();